#pragma once

#include "llama.h"
#include "llama-addon.h"

#include <string>
#include <vector>

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

enum gpt_sampler_type {
    GPT_SAMPLER_TYPE_NONE        = 0,
    GPT_SAMPLER_TYPE_TOP_K       = 1,
    GPT_SAMPLER_TYPE_TOP_P       = 2,
    GPT_SAMPLER_TYPE_MIN_P       = 3,
    GPT_SAMPLER_TYPE_TFS_Z       = 4,
    GPT_SAMPLER_TYPE_TYPICAL_P   = 5,
    GPT_SAMPLER_TYPE_TEMPERATURE = 6,
};

// sampling parameters
struct gpt_sampler_params {
    uint32_t seed = LLAMA_DEFAULT_SEED; // the seed used to initialize llama_sampler

    int32_t n_prev            = 64;    // number of previous tokens to remember
    int32_t n_probs           = 0;     // if greater than 0, output the probabilities of top n_probs tokens.
    int32_t min_keep          = 0;     // 0 = disabled, otherwise samplers should return at least min_keep tokens
    int32_t top_k             = 40;    // <= 0 to use vocab size
    float   top_p             = 0.95f; // 1.0 = disabled
    float   min_p             = 0.05f; // 0.0 = disabled
    float   tfs_z             = 1.00f; // 1.0 = disabled
    float   typical_p         = 1.00f; // typical_p, 1.0 = disabled
    float   p_step            = 0.00f;    // 0.0 = disabled
    float   temp              = 0.80f; // <= 0.0 to sample greedily, 0.0 to not output probabilities
    float   dynatemp_range    = 0.00f; // 0.0 = disabled
    float   dynatemp_exponent = 1.00f; // controls how entropy maps to temperature in dynamic temperature sampler
    float   smoothing_factor  = 0.00f;    // 0.0 = disabled
    float   smoothing_curve   = 1.00f;    // 1.0 = flat
    int32_t penalty_last_n    = 64;    // last n tokens to penalize (0 = disable penalty, -1 = context size)
    float   penalty_repeat    = 1.00f; // 1.0 = disabled
    float   penalty_freq      = 0.00f; // 0.0 = disabled
    float   penalty_present   = 0.00f; // 0.0 = disabled
    float   penalty_threshold = 0.00f; // 0.0 = disabled
    int32_t mirostat          = 0;     // 0 = disabled, 1 = mirostat, 2 = mirostat 2.0
    float   mirostat_tau      = 5.00f; // target entropy
    float   mirostat_eta      = 0.10f; // learning rate
    llama_sampling_param_func temp_func;
    llama_sampling_param_func dynatemp_range_func;
    llama_sampling_param_func p_step_func;
    float    dry_multiplier        = 0.0f; // 0.0f = disabled, recommended value: 0.8f
    float    dry_base              = 1.75f;
    uint32_t dry_allowed_length    = 2;
    int32_t  dry_penalty_last_n    = -1; // DRY last n tokens to penalize (0 = disable penalty, -1 = context size)
    float    xtc_probability       = 0.5; // probability of removing a top token
    float    xtc_threshold         = 0.1; // minimum tokens probablitity for this to run
    float    xtc_threshold_max     = 1.0; // maximum tokens probablitity for this to run
    bool     xtc_probability_once  = false; // should we calculate chances one or for each token
    int      xtc_min               = 2; // minimum number of penalizeable tokens
    bool    penalize_nl       = false; // consider newlines as a repeatable token
    bool    ignore_eos        = false;
    std::string samplers_sequence     = "kfypmts"; // top_k, tail_free, typical_p, top_p, min_p, temp, p_step

    std::string grammar; // optional BNF-like grammar to constrain sampling

    std::vector<llama_logit_bias> logit_bias; // logit biases to apply

    // print the parameters into a string
    std::string print() const;
};

// gpt_sampler extends llama_sampler with additional functionality:
//
//  - grammar support
//  - custom sampler logic based on the parameters
//  - history of the last accepted tokens
//  - performance metrics
//
// This goal is to have a common implementation of the sampling logic shared across the examples.
// For example, depending on the temperature, the sampling chain can be very simple (greedy) or more
// complex (top-k, top-p, etc).
//
// Another example is related to the grammar. In general, the grammar constraints applied on the full
// vocabulary can be very taxing. To improve performance, the grammar can be applied only to the sampled
// token in order to verify if it fits the grammar. And only if the token doesn't fit the grammar, the
// grammar constraints are applied to the full vocabulary and the token is resampled.
//
// The gpt_sampler also maintains a container with the last accepted tokens. In the future, this can
// be moved into the core llama library.
//
// For convenience, the gpt_sampler also maintains a container with the current candidate tokens.
// This can be used to access the probabilities of the rest of the non-sampled tokens.
//
// TODO: measure grammar performance
//

struct gpt_sampler;

// llama_sampler API overloads

struct gpt_sampler * gpt_sampler_init(const struct llama_model * model, const struct gpt_sampler_params & params);

void gpt_sampler_free(struct gpt_sampler * gsmpl);

// if accept_grammar is true, the token is accepted both by the sampling chain and the grammar
void                 gpt_sampler_accept(struct gpt_sampler * gsmpl, llama_token token, bool accept_grammar);
void                 gpt_sampler_reset (struct gpt_sampler * gsmpl);
struct gpt_sampler * gpt_sampler_clone (struct gpt_sampler * gsmpl);

// arguments can be nullptr to skip printing
void gpt_perf_print(const struct llama_context * ctx, const struct gpt_sampler * gsmpl);

// extended sampling implementation:
//
// - set logits
// - apply the configured sampler chain
// - check if the token fits the grammar (if any)
// - if not: resample by first applying the grammar constraints and then sampling again (slower path)
//
// if grammar_first is true, the grammar is applied before the samplers (slower)
// useful in cases where all the resulting candidates (not just the sampled one) must fit the grammar
//
llama_token gpt_sampler_sample(struct gpt_sampler * gsmpl, struct llama_context * ctx, int idx, bool grammar_first = false);

// helpers

// access the internal list of current candidate tokens
llama_token_data_array * gpt_sampler_get_candidates(struct gpt_sampler * gsmpl);

// get the last accepted token
llama_token gpt_sampler_last(const struct gpt_sampler * gsmpl);

// print the sampler chain into a string
std::string gpt_sampler_print(const struct gpt_sampler * gsmpl);

// get a string representation of the last accepted tokens
std::string gpt_sampler_prev_str(gpt_sampler * gsmpl, llama_context * ctx, int n);

char        gpt_sampler_type_to_chr(enum gpt_sampler_type cnstr);
std::string gpt_sampler_type_to_str(enum gpt_sampler_type cnstr);

std::vector<enum gpt_sampler_type> gpt_sampler_types_from_names(const std::vector<std::string> & names, bool allow_alt_names);
std::vector<enum gpt_sampler_type> gpt_sampler_types_from_chars(const std::string & chars);

// this performs rollback of the latest sampling operation by "rollback_num" tokens;
// it simply strikes the latest "rollback_num" tokens from the "prev" vector
// in general, the rollback is "imperfect", meaning the "forgotten tokens" which were dropped when the length of "prev" exceeded "n_prev" cannot be recalled after rollback
// however, if `sampling_params.n_prev` >= `sampling_params.penalty_last_n` + `rollback_num`, then it becomes "perfect" rollback
void llama_sampling_rollback(gpt_sampler * gsmpl, int rollback_num);
