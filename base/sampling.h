#pragma once

#include "llama-addon.h"

#include "grammar-parser.h"

#include <string>
#include <vector>
#include <unordered_map>

// sampling function
typedef struct llama_sampling_param_func {
    float p_min = -1.0f;
    float p_max = -1.0f;
    float p_add = 0.0f;
    float p_mul = 1.0f;
    bool p_dir = true;

    bool operator==(const llama_sampling_param_func& other) const {
        return p_min == other.p_min && p_max == other.p_max && p_add == other.p_add && p_mul == other.p_mul;
    }

    bool operator!=(const llama_sampling_param_func& other) const {
        return p_min != other.p_min || p_max != other.p_max || p_add != other.p_add || p_mul != other.p_mul;
    }

} llama_sampling_param_func;

// sampling parameters
typedef struct llama_sampling_params {
    int32_t     n_prev                = 64;       // number of previous tokens to remember
    int32_t     n_probs               = 0;        // if greater than 0, output the probabilities of top n_probs tokens.
    int32_t     min_keep              = 0;        // 0 = disabled, otherwise samplers should return at least min_keep tokens
    int32_t     top_k                 = 0;       // <= 0 to use vocab size
    float       top_p                 = 1.0f;    // 1.0 = disabled
    float       min_p                 = 0.05f;    // 0.0 = disabled
    float       tfs_z                 = 1.00f;    // 1.0 = disabled
    float       typical_p             = 1.00f;    // 1.0 = disabled
    float       temp                  = 0.80f;    // 1.0 = disabled
    llama_sampling_param_func temp_func;
    float       smoothing_factor      = 0.00f;    // 0.0 = disabled
    float       smoothing_curve       = 1.00f;    // 1.0 = flat
    float       dynatemp_range        = 0.00f;    // 0.0 = disabled
    llama_sampling_param_func dynatemp_range_func;
    float       p_step                = 0.00f;    // 0.0 = disabled
    llama_sampling_param_func p_step_func;
    int32_t     penalty_last_n        = 64;       // last n tokens to penalize (0 = disable penalty, -1 = context size)
    float       penalty_repeat        = 1.00f;    // 1.0 = disabled
    float       penalty_freq          = 0.00f;    // 0.0 = disabled
    float       penalty_present       = 0.00f;    // 0.0 = disabled
    float       penalty_threshold     = 1.00f;    // 0.0 = disabled
    int32_t     mirostat              = 0;        // 0 = disabled, 1 = mirostat, 2 = mirostat 2.0
    float       mirostat_tau          = 5.00f;    // target entropy
    float       mirostat_eta          = 0.10f;    // learning rate
    bool        penalize_nl           = true;     // consider newlines as a repeatable token
    float       dry_multiplier        = 0.0f;               // 0.0f = disabled, recommended value: 0.8f
    float       dry_base              = 1.75f;
    uint32_t    dry_allowed_length    = 2;
    int32_t     dry_penalty_last_n    = -1;                 // DRY last n tokens to penalize (0 = disable penalty, -1 = context size)
    float       xtc_probability       = 0.5; // probability of removing a top token
    float       xtc_threshold         = 0.1; // minimum tokens probablitity for this to run
    bool        xtc_probability_once  = false; // should we calculate chances one or for each token
    std::string samplers_sequence     = "kfypmts"; // top_k, tail_free, typical_p, top_p, min_p, temp, p_step

    std::string grammar;  // optional BNF-like grammar to constrain sampling

    // Classifier-Free Guidance
    // https://arxiv.org/abs/2306.17806
    std::string cfg_negative_prompt; // string to help guidance
    float       cfg_scale     = 1.f; // how strong is guidance

    std::unordered_map<llama_token, float> logit_bias; // logit bias for specific tokens
    std::vector<llama_token> dry_seq_breakers; // sequence breakers for the DRY sampler
} llama_sampling_params;

// general sampler context
// TODO: move to llama.h
struct llama_sampling_context {
    // parameters that will be used for sampling
    llama_sampling_params params;

    // mirostat sampler state
    float mirostat_mu;

    llama_grammar * grammar;

    // internal
    grammar_parser::parse_state parsed_grammar;

    // TODO: replace with ring-buffer
    std::vector<llama_token>      prev;
    std::vector<llama_token_data> cur;
};

#include "common.h"

// Create a new sampling context instance.
struct llama_sampling_context * llama_sampling_init(const struct llama_sampling_params & params);

void llama_sampling_free(struct llama_sampling_context * ctx);

// Reset the sampler context
// - clear prev tokens
// - reset grammar
void llama_sampling_reset(llama_sampling_context * ctx);

// Copy the sampler context
void llama_sampling_cp(llama_sampling_context * src, llama_sampling_context * dst);

// Get the last sampled token
llama_token llama_sampling_last(llama_sampling_context * ctx);

// Get a string representation of the last sampled tokens
std::string llama_sampling_prev_str(llama_sampling_context * ctx_sampling, llama_context * ctx_main, int n);

// Print sampling parameters into a string
std::string llama_sampling_print(const llama_sampling_params & params);

// Print sampling order into a string
std::string llama_sampling_order_print(const llama_sampling_params & params);

// this is a common sampling function used across the examples for convenience
// it can serve as a starting point for implementing your own sampling function
// Note: When using multiple sequences, it is the caller's responsibility to call
//       llama_sampling_reset when a sequence ends
//
// required:
//  - ctx_main:     context to use for sampling
//  - ctx_sampling: sampling-specific context
//
// optional:
//  - ctx_cfg:      context to use for classifier-free guidance
//  - idx:          sample from llama_get_logits_ith(ctx, idx)
//
// returns:
//  - token:      sampled token
//  - candidates: vector of candidate tokens
//
llama_token llama_sampling_sample(
        struct llama_sampling_context * ctx_sampling,
        struct llama_context * ctx_main,
        struct llama_context * ctx_cfg,
        int idx = 0);

void llama_sampling_accept(
        struct llama_sampling_context * ctx_sampling,
        struct llama_context * ctx_main,
        llama_token id,
        bool apply_grammar);

// this performs rollback of the latest sampling operation by "rollback_num" tokens;
// it simply strikes the latest "rollback_num" tokens from the "prev" vector
// in general, the rollback is "imperfect", meaning the "forgotten tokens" which were dropped when the length of "prev" exceeded "n_prev" cannot be recalled after rollback
// however, if `sampling_params.n_prev` >= `sampling_params.penalty_last_n` + `rollback_num`, then it becomes "perfect" rollback
void llama_sampling_rollback(
        struct llama_sampling_context * ctx_sampling,
        int rollback_num);
