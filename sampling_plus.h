#pragma once
#include <sampling.h>

// this is a temporary experiment, since sampling with temp showed some interesting results
// but it stil needs testing

// temperature first
llama_token llama_sampling_sample_temp_first(
                  struct llama_sampling_context * ctx_sampling,
                  struct llama_context * ctx_main,
                  struct llama_context * ctx_cfg,
                  const int idx) {
    const llama_sampling_params & params = ctx_sampling->params;

    const int n_vocab = llama_n_vocab(llama_get_model(ctx_main));

    const float   temp            = params.temp;
    const int32_t top_k           = params.top_k <= 0 ? n_vocab : params.top_k;
    const float   top_p           = params.top_p;
    const float   min_p           = params.min_p;
    const float   tfs_z           = params.tfs_z;
    const float   typical_p       = params.typical_p;
    const int32_t penalty_last_n  = params.penalty_last_n < 0 ? params.n_prev : params.penalty_last_n;
    const float   penalty_repeat  = params.penalty_repeat;
    const float   penalty_freq    = params.penalty_freq;
    const float   penalty_present = params.penalty_present;
    const int     mirostat        = params.mirostat;
    const float   mirostat_tau    = params.mirostat_tau;
    const float   mirostat_eta    = params.mirostat_eta;
    const bool    penalize_nl     = params.penalize_nl;

    auto & prev = ctx_sampling->prev;
    auto & cur  = ctx_sampling->cur;

    llama_token id = 0;

    float * logits = llama_get_logits_ith(ctx_main, idx);

    // apply params.logit_bias map
    for (auto it = params.logit_bias.begin(); it != params.logit_bias.end(); it++) {
        logits[it->first] += it->second;
    }

    cur.clear();

    for (llama_token token_id = 0; token_id < n_vocab; token_id++) {
        cur.emplace_back(llama_token_data{token_id, logits[token_id], 0.0f});
    }

    llama_token_data_array cur_p = { cur.data(), cur.size(), false };

    if (ctx_cfg) {
        llama_sample_classifier_free_guidance(ctx_main, &cur_p, ctx_cfg, params.cfg_scale);
    }

    // apply penalties
    if (!prev.empty()) {
        const float nl_logit = logits[llama_token_nl(llama_get_model(ctx_main))];

        llama_sample_repetition_penalties(ctx_main, &cur_p,
                prev.data() + prev.size() - penalty_last_n,
                penalty_last_n, penalty_repeat, penalty_freq, penalty_present);

        if (!penalize_nl) {
            for (size_t idx = 0; idx < cur_p.size; idx++) {
                if (cur_p.data[idx].id == llama_token_nl(llama_get_model(ctx_main))) {
                    cur_p.data[idx].logit = nl_logit;
                    break;
                }
            }
        }
    }

    if (ctx_sampling->grammar != NULL) {
        llama_sample_grammar(ctx_main, &cur_p, ctx_sampling->grammar);
    }

    if (temp < 0.0) {
        // greedy sampling, with probs
        llama_sample_softmax(ctx_main, &cur_p);
        id = cur_p.data[0].id;
    } else if (temp == 0.0) {
        // greedy sampling, no probs
        id = llama_sample_token_greedy(ctx_main, &cur_p);
    } else {
        if (mirostat == 1) {
            const int mirostat_m = 100;
            llama_sample_temp(ctx_main, &cur_p, temp);
            id = llama_sample_token_mirostat(ctx_main, &cur_p, mirostat_tau, mirostat_eta, mirostat_m, &ctx_sampling->mirostat_mu);
        } else if (mirostat == 2) {
            llama_sample_temp(ctx_main, &cur_p, temp);
            id = llama_sample_token_mirostat_v2(ctx_main, &cur_p, mirostat_tau, mirostat_eta, &ctx_sampling->mirostat_mu);
        } else {
            // temperature sampling
            size_t min_keep = std::max(1, params.n_probs);

            llama_sample_temp     (ctx_main, &cur_p, temp);
            llama_sample_top_k    (ctx_main, &cur_p, top_k,     min_keep);
            llama_sample_tail_free(ctx_main, &cur_p, tfs_z,     min_keep);
            llama_sample_typical  (ctx_main, &cur_p, typical_p, min_keep);
            llama_sample_top_p    (ctx_main, &cur_p, top_p,     min_keep);
            llama_sample_min_p    (ctx_main, &cur_p, min_p,     min_keep);
            

            id = llama_sample_token(ctx_main, &cur_p);

            //{
            //    const int n_top = 10;
            //    LOG("top %d candidates:\n", n_top);

            //    for (int i = 0; i < n_top; i++) {
            //        const llama_token id = cur_p.data[i].id;
            //        (void)id; // To avoid a warning that id is unused when logging is disabled.
            //        LOG(" - %5d: '%12s' (%.3f)\n", id, llama_token_to_piece(ctx_main, id).c_str(), cur_p.data[i].p);
            //    }
            //}

            //LOG("sampled token: %5d: '%s'\n", id, llama_token_to_piece(ctx_main, id).c_str());
        }
    }

    return id;
}

// choosing which one to use
llama_token llama_sampling_sample_choice(
                  struct llama_sampling_context * ctx_sampling,
                  struct llama_context * ctx_main,
                  struct llama_context * ctx_cfg,
                  bool temp_first,
                  int idx  = 0) {
                      
    if (temp_first) return llama_sampling_sample_temp_first(ctx_sampling, ctx_main, ctx_cfg,idx);
    else return llama_sampling_sample(ctx_sampling, ctx_main, ctx_cfg, idx);
}