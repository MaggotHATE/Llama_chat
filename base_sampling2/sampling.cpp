#include "sampling.h"

#include "common.h"

#include <cmath>
#include <unordered_map>

struct common_sampler {
    common_sampler_params params;

    struct llama_sampler * grmr;
    struct llama_sampler * chain;

    ring_buffer<llama_token> prev;

    std::vector<llama_token_data> cur;

    llama_token_data_array cur_p;

    void set_logits(struct llama_context * ctx, int idx) {
        const auto * logits = llama_get_logits_ith(ctx, idx);

        const int n_vocab = llama_n_vocab(llama_get_model(ctx));

        cur.resize(n_vocab);

        for (llama_token token_id = 0; token_id < n_vocab; token_id++) {
            cur[token_id] = llama_token_data{token_id, logits[token_id], 0.0f};
        }

        cur_p = { cur.data(), cur.size(), -1, false };
    }
};

std::string common_sampler_params::print() const {
    char result[1024];

    snprintf(result, sizeof(result),
            "\trepeat_last_n = %d, repeat_penalty = %.3f, frequency_penalty = %.3f, presence_penalty = %.3f\n"
            "\ttop_k = %d, tfs_z = %.3f, top_p = %.3f, min_p = %.3f, typical_p = %.3f, temp = %.3f\n"
            "\tmirostat = %d, mirostat_lr = %.3f, mirostat_ent = %.3f",
            penalty_last_n, penalty_repeat, penalty_freq, penalty_present,
            top_k, tfs_z, top_p, min_p, typical_p, temp,
            mirostat, mirostat_eta, mirostat_tau);

    return std::string(result);
}

struct common_sampler * common_sampler_init(const struct llama_model * model, const struct common_sampler_params & params) {
    llama_sampler_chain_params lparams = llama_sampler_chain_default_params();

    lparams.no_perf = params.no_perf;

    auto * result = new common_sampler {
        /* .params = */ params,
        /* .grmr   = */ llama_sampler_init_grammar(model, params.grammar.c_str(), "root"),
        /* .chain  = */ llama_sampler_chain_init(lparams),
        /* .prev   = */ ring_buffer<llama_token>(std::max(32, params.n_prev)),
        /* .cur    = */ {},
        /* .cur_p  = */ {},
    };

    llama_sampler_chain_add(result->chain,
            llama_sampler_init_logit_bias(
                llama_n_vocab(model),
                params.logit_bias.size(),
                params.logit_bias.data()));

    llama_sampler_chain_add(result->chain,
            llama_sampler_init_penalties(
                llama_n_vocab  (model),
                llama_token_eos(model),
                llama_token_nl (model),
                params.penalty_last_n,
                params.penalty_repeat,
                params.penalty_freq,
                params.penalty_present,
                params.penalize_nl,
                params.ignore_eos));

    //if (params.temp > 0.0f) {
        if (params.mirostat == 0) {
            for (const auto & cnstr : params.samplers_sequence) {
                switch (cnstr) {
                    case 'k':
                        llama_sampler_chain_add(result->chain, llama_sampler_init_top_k    (params.top_k));
                        break;
                    case 'p':
                        llama_sampler_chain_add(result->chain, llama_sampler_init_top_p    (params.top_p, params.min_keep));
                        break;
                    case 'm':
                        llama_sampler_chain_add(result->chain, llama_sampler_init_min_p_addon (params.min_p, params.min_p_rand, params.min_keep));
                        break;
                    case 'f':
                        llama_sampler_chain_add(result->chain, llama_sampler_init_tail_free(params.tfs_z, params.min_keep));
                        break;
                    case 'y':
                        llama_sampler_chain_add(result->chain, llama_sampler_init_typical  (params.typical_p, params.min_keep));
                        break;
                    case 't':
                        llama_sampler_chain_add(result->chain, llama_sampler_init_temp_ext_addon (params.temp, params.dynatemp_range, params.dynatemp_exponent, params.smoothing_factor, params.smoothing_curve));
                        break;
                    case 'T':
                        llama_sampler_chain_add(result->chain, llama_sampler_init_temp_ext (params.temp, params.dynatemp_range, params.dynatemp_exponent));
                        break;
                    case 's':
                        llama_sampler_chain_add(result->chain, llama_sampler_init_p_step_addon (params.p_step, params.min_keep));
                        break;
                    case 'o':
                        llama_sampler_chain_add(result->chain, llama_sampler_init_noise_addon (params.noise_min, params.noise_max, params.seed));
                        break;
                    case 'r':
                        llama_sampler_chain_add(result->chain, llama_sampler_init_rx_addon (params.range_max, params.range_min, params.min_keep));
                        break;
                    case 'x':
                        llama_sampler_chain_add(result->chain, llama_sampler_init_xtc_addon (params.xtc_probability, params.xtc_threshold, params.xtc_threshold_max, params.xtc_probability_once, params.xtc_min, params.min_keep, params.seed));
                        break;
                    case 'i':
                        llama_sampler_chain_add(result->chain, llama_sampler_init_infill   (model));
                        break;
                    default:
                        break;
                }
            }
            llama_sampler_chain_add(result->chain, llama_sampler_init_dist(params.seed));
        } else if (params.mirostat == 1) {
            llama_sampler_chain_add(result->chain, llama_sampler_init_temp(params.temp));
            llama_sampler_chain_add(result->chain, llama_sampler_init_mirostat(llama_n_vocab(model), params.seed, params.mirostat_tau, params.mirostat_eta, 100));
        } else if (params.mirostat == 2) {
            llama_sampler_chain_add(result->chain, llama_sampler_init_temp(params.temp));
            llama_sampler_chain_add(result->chain, llama_sampler_init_mirostat_v2(params.seed, params.mirostat_tau, params.mirostat_eta));
        } else {
            GGML_ASSERT(false && "unknown mirostat version");
        }
    //} else {
    //    if (params.n_probs > 0) {
            // some use cases require to sample greedily, but still obtain the probabilities of the top tokens
            // ref: https://github.com/ggerganov/llama.cpp/pull/9605
            //
            // the following will not produce exactly the same probs as applyging softmax to the full vocabulary, but
            // it is much faster, since we avoid sorting all tokens and should give a good approximation
    //        llama_sampler_chain_add(result->chain, llama_sampler_init_top_k(params.n_probs));
    //    }

        // if (params.k_limit > 0) llama_sampler_chain_add(result->chain, llama_sampler_init_limit_k (params.k_limit));

        // if (params.range_min < 1.0f && params.range_max == 1.0f) llama_sampler_chain_add(result->chain, llama_sampler_init_rx_addon (params.range_max, params.range_min, 2));

        // if (params.noise_min > 0.0f) llama_sampler_chain_add(result->chain, llama_sampler_init_noise_addon (params.noise_min, params.noise_max, params.seed));

    //    llama_sampler_chain_add(result->chain, llama_sampler_init_greedy());
    //}

    return result;
}

void common_sampler_free(struct common_sampler * gsmpl) {
    if (gsmpl) {
        llama_sampler_free(gsmpl->grmr);

        llama_sampler_free(gsmpl->chain);

        delete gsmpl;
    }
}

void common_sampler_accept(struct common_sampler * gsmpl, llama_token token, bool accept_grammar) {
    if (accept_grammar) {
        llama_sampler_accept(gsmpl->grmr, token);
    }

    llama_sampler_accept(gsmpl->chain, token);

    gsmpl->prev.push_back(token);
}

void common_sampler_reset(struct common_sampler * gsmpl) {
    llama_sampler_reset(gsmpl->grmr);

    llama_sampler_reset(gsmpl->chain);
}

struct common_sampler * common_sampler_clone(common_sampler * gsmpl) {
    return new common_sampler {
        /* .params = */ gsmpl->params,
        /* .grmr   = */ llama_sampler_clone(gsmpl->grmr),
        /* .chain  = */ llama_sampler_clone(gsmpl->chain),
        /* .prev   = */ gsmpl->prev,
        /* .cur    = */ gsmpl->cur,
        /* .cur_p  = */ gsmpl->cur_p,
    };
}

void common_perf_print(const struct llama_context * ctx, const struct common_sampler * gsmpl) {
    // TODO: measure grammar performance

    if (gsmpl) {
        llama_perf_sampler_print(gsmpl->chain);
    }
    if (ctx) {
        llama_perf_context_print(ctx);
    }
}

llama_token common_sampler_sample(struct common_sampler * gsmpl, struct llama_context * ctx, int idx, bool grammar_first) {
    gsmpl->set_logits(ctx, idx);

    auto & grmr  = gsmpl->grmr;
    auto & chain = gsmpl->chain;
    auto & cur_p = gsmpl->cur_p; // initialized by set_logits

    if (grammar_first) {
        llama_sampler_apply(grmr, &cur_p);
    }

    llama_sampler_apply(chain, &cur_p);

    GGML_ASSERT(cur_p.selected != -1 && "no selected token during sampling - check your sampling configuration");

    const llama_token id = cur_p.data[cur_p.selected].id;

    if (grammar_first) {
        return id;
    }

    // check if it the sampled token fits the grammar
    {
        llama_token_data       single_token_data       = { id, 1.0f, 0.0f };
        llama_token_data_array single_token_data_array = { &single_token_data, 1, -1, false };

        llama_sampler_apply(grmr, &single_token_data_array);

        const bool is_valid = single_token_data_array.data[0].logit != -INFINITY;
        if (is_valid) {
            return id;
        }
    }

    // resampling:
    // if the token is not valid, sample again, but first apply the grammar sampler and then the sampling chain
    gsmpl->set_logits(ctx, idx);

    llama_sampler_apply(grmr,  &cur_p);
    llama_sampler_apply(chain, &cur_p);

    GGML_ASSERT(cur_p.selected != -1 && "no selected token during re-sampling - check your sampling configuration");

    return cur_p.data[cur_p.selected].id;
}

uint32_t common_sampler_get_seed(const struct common_sampler * gsmpl) {
    return llama_sampler_get_seed(gsmpl->chain);
}

// helpers

llama_token_data_array * common_sampler_get_candidates(struct common_sampler * gsmpl) {
    return &gsmpl->cur_p;
}

llama_token common_sampler_last(const struct common_sampler * gsmpl) {
    return gsmpl->prev.rat(0);
}

std::string common_sampler_print(const struct common_sampler * gsmpl) {
    std::string result = "\tlogits ";

    for (int i = 0; i < llama_sampler_chain_n(gsmpl->chain); i++) {
        const auto * smpl = llama_sampler_chain_get(gsmpl->chain, i);
        result += std::string("-> ") + llama_sampler_name(smpl) + " ";
    }

    return result;
}

std::string common_sampler_prev_str(common_sampler * gsmpl, llama_context * ctx_main, int n) {
    n = std::min(n, (int) gsmpl->prev.size());

    if (n <= 0) {
        return "";
    }

    std::string result;
    result.reserve(8*n); // 8 is the average length of a token [citation needed], TODO: compute this from the vocab

    for (int i = n - 1; i >= 0; i--) {
        const llama_token id = gsmpl->prev.rat(i);

        GGML_ASSERT(id != LLAMA_TOKEN_NULL && "null token in the sampling history - should not happen");

        result += common_token_to_piece(ctx_main, id);
    }

    return result;
}

ring_buffer<llama_token> llama_sampling_get_prev(common_sampler * gsmpl) {
    return gsmpl->prev;
}

void llama_sampling_set_prev(ring_buffer<llama_token> prev_state, common_sampler * gsmpl) {
    gsmpl->prev = prev_state;
}

