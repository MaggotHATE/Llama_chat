#pragma once

#include <string>
#include <vector>
#include <stdexcept>
#include "llama-cpp.h"

// the ring buffer works similarly to std::deque, but with a fixed capacity
// TODO: deduplicate with llama-impl.h
template<typename T>
struct ring_buffer {
    ring_buffer(size_t cap) : capacity(cap), data(cap) {}

    T & front() {
        if (sz == 0) {
            throw std::runtime_error("ring buffer is empty");
        }
        return data[first];
    }

    const T & front() const {
        if (sz == 0) {
            throw std::runtime_error("ring buffer is empty");
        }
        return data[first];
    }

    T & back() {
        if (sz == 0) {
            throw std::runtime_error("ring buffer is empty");
        }
        return data[pos];
    }

    const T & back() const {
        if (sz == 0) {
            throw std::runtime_error("ring buffer is empty");
        }
        return data[pos];
    }

    void push_back(const T & value) {
        if (sz == capacity) {
            // advance the start when buffer is full
            first = (first + 1) % capacity;
        } else {
            sz++;
        }
        data[pos] = value;
        pos = (pos + 1) % capacity;
    }

    T pop_front() {
        if (sz == 0) {
            throw std::runtime_error("ring buffer is empty");
        }
        T value = data[first];
        first = (first + 1) % capacity;
        sz--;
        return value;
    }

    const T & rat(size_t i) const {
        if (i >= sz) {
            throw std::runtime_error("ring buffer: index out of bounds");
        }
        return data[(first + sz - i - 1) % capacity];
    }

    std::vector<T> to_vector() const {
        std::vector<T> result;
        result.reserve(sz);
        for (size_t i = 0; i < sz; i++) {
            result.push_back(data[(first + i) % capacity]);
        }
        return result;
    }

    void clear() {
        // here only reset the status of the buffer
        sz = 0;
        first = 0;
        pos = 0;
    }

    bool empty() const {
        return sz == 0;
    }

    size_t size() const {
        return sz;
    }

    size_t capacity = 0;
    size_t sz = 0;
    size_t first = 0;
    size_t pos = 0;
    std::vector<T> data;
};

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

    LLAMA_API struct llama_sampler * llama_sampler_init_limit_k (float confidence_shift, int32_t k);
    LLAMA_API struct llama_sampler * llama_sampler_init_min_p_addon (float p, float rand, size_t min_keep);
    LLAMA_API struct llama_sampler * llama_sampler_init_xtc_addon (float probability, float threshold, float threshold_max, bool probability_once, int min, size_t min_keep, uint32_t seed);
    LLAMA_API struct llama_sampler * llama_sampler_init_noise_addon (float min, float max, uint32_t seed);
    LLAMA_API struct llama_sampler * llama_sampler_init_rx_addon (float max, float min, size_t min_keep);
    LLAMA_API struct llama_sampler * llama_sampler_init_p_step_addon (float step, float rand, size_t min_keep);
    LLAMA_API struct llama_sampler * llama_sampler_init_temp_ext_addon (float t, float delta, float exponent, float smoothing_factor, float smoothing_curve, bool temp_adaptive);

    LLAMA_API struct llama_sampler * llama_sampler_init_post_addon (uint32_t seed, float probability, float threshold);
    LLAMA_API struct llama_sampler * llama_sampler_init_dist_plus  (uint32_t seed, float confidence_top, float confidence_bottom);
    LLAMA_API struct llama_sampler * llama_sampler_init_dist_plus1 (uint32_t seed, float confidence_top, float confidence_bottom, const std::vector<llama_logit_bias> logit_bias);

    LLAMA_API struct llama_sampler * llama_sampler_init_penalties_addon(
                             int32_t   n_vocab,           // llama_n_vocab()
                         llama_token   special_eos_id,    // llama_token_eos()
                         llama_token   linefeed_id,       // llama_token_nl()
                             int32_t   penalty_last_n,    // last n tokens to penalize (0 = disable penalty, -1 = context size)
                               float   penalty_repeat,    // 1.0 = disabled
                               float   penalty_freq,      // 0.0 = disabled
                               float   penalty_present,   // 0.0 = disabled
                               float   penalty_threshold, // 0.0 = disabled
                                bool   penalize_nl,       // consider newlines as a repeatable token
                                bool   ignore_eos);       // ignore the end-of-sequence token

    LLAMA_API struct llama_sampler * llama_sampler_init_dry_addon(
                            const float       dry_base,
                            const float       dry_multiplier,
                            const int         dry_allowed_length,
                            const size_t      dry_seq_breakers_size,
                            const llama_token last_tokens,
                            size_t            last_tokens_size,
                            const llama_token dry_seq_breakers);

    /// @details Tail Free Sampling described in https://www.trentonbricken.com/Tail-Free-Sampling/.
    LLAMA_API struct llama_sampler * llama_sampler_init_tail_free  (float   z, size_t min_keep);

    /// @details Top n sigma sampling as described in academic paper "Top-nσ: Not All Logits Are You Need" https://arxiv.org/pdf/2411.07641
    LLAMA_API struct llama_sampler * llama_sampler_init_top_n_sigma_addon(float n);

    LLAMA_API struct llama_sampler * llama_sampler_init_logit_bias_addon(
                             int32_t   n_vocab,
                             int32_t   n_logit_bias,
              const llama_logit_bias * logit_bias);

              LLAMA_API struct llama_sampler * llama_sampler_init_logit_bias_start_addon(
                             int32_t   n_vocab,
                             int32_t   n_logit_bias,
              const llama_logit_bias * logit_bias);