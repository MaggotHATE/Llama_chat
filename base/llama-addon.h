#pragma once
#include "llama.h"

/*
void llama_set_time(struct llama_context * ctx, const int64_t t_start_sample_us) {
    llama_set_time_impl(ctx ? &ctx->sampling : nullptr, t_start_sample_us);
}
LLAMA_API void llama_set_time(struct llama_context * ctx, const int64_t t_start_sample_us);
-------------------
void llama_set_time_impl(struct llama_sampling * smpl, const int64_t t_start_sample_us) {
    if (smpl) {
        smpl->t_sample_us += ggml_time_us() - t_start_sample_us;
    }
}
void llama_set_time_impl(struct llama_sampling * smpl, const int64_t t_start_sample_us);
*/

    /// https://github.com/oobabooga/text-generation-webui/pull/6335
    LLAMA_API void llama_sample_xtc_addon(
            struct llama_context * ctx,
          llama_token_data_array * candidates,
                           float   xtc_probability,
                           float   xtc_threshold,
                           float   xtc_threshold_max,
                           bool    xtc_probability_once,
                           int     xtc_min,
                          size_t   min_keep);

    /// https://github.com/oobabooga/text-generation-webui/pull/6335
    LLAMA_API void llama_sample_xtc_addon2(
            struct llama_context * ctx,
          llama_token_data_array * candidates,
                           float   xtc_probability,
                           float   xtc_threshold,
                           float   xtc_threshold_max,
                           bool    xtc_probability_once,
                           int     xtc_min,
                          size_t   min_keep);


    /// @details P-Step sampling as described in [THIS PR]
    LLAMA_API void llama_sample_p_step_addon(
            struct llama_context * ctx,
          llama_token_data_array * candidates,
                           float   step,
                          size_t   min_keep);

    LLAMA_API void llama_sample_min_p_addon(
            struct llama_context * ctx,
          llama_token_data_array * candidates,
                           float   p,
                          size_t   min_keep);

    LLAMA_API void llama_sample_repetition_penalties_addon(
            struct llama_context * ctx,
          llama_token_data_array * candidates,
               const llama_token * last_tokens,
                          size_t   penalty_last_n,
                           float   penalty_repeat,
                           float   penalty_freq,
                           float   penalty_present,
                           float   penalty_threshold);

    LLAMA_API void llama_sample_entropy_addon(
            struct llama_context * ctx,
          llama_token_data_array * candidates_p,
                           float   min_temp,
                           float   max_temp,
                           float   exponent_val,
                           float   smoothing_factor,
                           float   smoothing_curve);

    LLAMA_API void llama_sample_temp_addon(
            struct llama_context * ctx,
          llama_token_data_array * candidates,
                           float   temp,
                           float   smoothing_factor,
                           float   smoothing_curve);

    ///  @details DRY sampler as described in: https://github.com/oobabooga/text-generation-webui/pull/5677
    LLAMA_API void llama_sample_dry(
          llama_token_data_array * candidates,
               const llama_token * last_tokens,
                          size_t   last_tokens_size,
                           float   dry_base,
                           float   dry_multiplier,
                             int   dry_allowed_length,
               const llama_token * dry_seq_breakers,
                          size_t   dry_seq_breakers_size);