#include "ggml.h"
#include "ggml-alloc.h"
#include "ggml-backend.h"
#include "llama.h"
#include "llama-impl.h"
#include "llama-addon.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <cctype>
#include <cfloat>
#include <cinttypes>
#include <climits>
#include <cmath>
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <forward_list>
#include <fstream>
#include <functional>
#include <future>
#include <initializer_list>
#include <locale>
#include <map>
#include <memory>
#include <mutex>
#include <numeric>
#include <queue>
#include <random>
#include <regex>
#include <set>
#include <sstream>
#include <thread>
#include <type_traits>
#include <unordered_map>

//xtc statistics
int xtc_total = 0;
int xtc_removed = 0;
float xtc_percent = 0.0;

//-------------------common functions------------------------

static void llama_sampler_softmax_impl(llama_token_data_array * cur_p) {
    GGML_ASSERT(cur_p->size > 0);

    // Sort the logits in descending order
    if (!cur_p->sorted) {
        std::sort(cur_p->data, cur_p->data + cur_p->size, [](const llama_token_data & a, const llama_token_data & b) {
            return a.logit > b.logit;
        });
        cur_p->sorted = true;
    }

    float max_l = cur_p->data[0].logit;
    float cum_sum = 0.0f;

    for (size_t i = 0; i < cur_p->size; ++i) {
        float p = expf(cur_p->data[i].logit - max_l);
        cur_p->data[i].p = p;
        cum_sum += p;
    }

    for (size_t i = 0; i < cur_p->size; ++i) {
        cur_p->data[i].p /= cum_sum;
    }
}

static bool writeCandidatesToFile(std::string path, llama_token_data_array * candidates, std::string add){
    std::string text = add + "(" + std::to_string(candidates->size) + ")";
    int zeroes = 0;
    for (size_t i = 0; i < candidates->size; ++i) {
        if (candidates->data[i].p > 0) { text += "\n[" + std::to_string(i) + "] p=" + std::to_string(candidates->data[i].p) + "(l=" + std::to_string(candidates->data[i].logit) + ");"; } else ++zeroes;
    }
    if (zeroes > 0) text += "\n Zeroes: " + std::to_string(zeroes);
    std::ofstream file(path, std::ios::app);
    if (file.is_open()) {
        file << text;
        file.close();
        return true;
    } else {
        return false;
    }
}

//-------------------MIN-P WITH NOISE------------------------

struct llama_sampler_min_p_addon {
    const float  p;
    const size_t min_keep;
};

static const char * llama_sampler_min_p_addon_name(const struct llama_sampler * /*smpl*/) {
    return "min-p";
}

static void llama_sampler_min_p_addon_apply(struct llama_sampler * smpl, llama_token_data_array * cur_p) {
    const auto * ctx = (llama_sampler_min_p_addon *) smpl->ctx;

    if (ctx->p <= 0.0f || !cur_p->size) {
        return;
    }

    bool min_p_applied = false;

    // if the cur_p aren't sorted, try the unsorted implementation first
    if (!cur_p->sorted) {
        std::vector<llama_token_data> filtered_tokens;

        float max_logit = -FLT_MAX;
        for (size_t i = 0; i < cur_p->size; ++i) {
            max_logit = std::max(max_logit, cur_p->data[i].logit);
        }
        const float min_logit = max_logit + logf(ctx->p); // min logit for p_i >= p * p_max

        for (size_t i = 0; i < cur_p->size; ++i) {
            if (cur_p->data[i].logit >= min_logit) {
                filtered_tokens.push_back(cur_p->data[i]);
            }
        }

        // if we have enough values the operation was a success
        if (filtered_tokens.size() >= ctx->min_keep) {
            memcpy(cur_p->data, filtered_tokens.data(), filtered_tokens.size()*sizeof(llama_token_data));
            cur_p->size = filtered_tokens.size();
            min_p_applied = true;
        }
    }

    // Variables to hold the external values
    bool worstToken = false; // unused from earlier experiment, disregard
    float randomizationFactor = 1.0f; // Default value of the randomization factor
    bool isTrueRNG = true; // Default value for RNG type, set to true for true randomness
    unsigned int rngSeed = 123456789; // Default seed value for deterministic RNG

    // Check if the randomizationFactor value is above 0 and apply Gaussian noise if so
    if (randomizationFactor > 0.0) {        
        // Create a random number generator
        std::default_random_engine generator;
        if (isTrueRNG) {
            // Seed with a real random value, if available
            std::random_device rd;
            generator.seed(rd());
        } else {
            // Use a fixed seed for deterministic behavior
            generator.seed(rngSeed);
        }

        // Create a Gaussian distribution with mean 0 and standard deviation of your choice
        std::normal_distribution<float> distribution(0.0f, randomizationFactor); // Replace 1.0f with the desired standard deviation

        // Apply Gaussian noise to each logit
        for (size_t i = 0; i < cur_p->size; ++i) {
            // Add Gaussian noise to the logit
            cur_p->data[i].logit += distribution(generator);
        }

        cur_p->sorted = false;
    }

    // if the cur_p are sorted or the unsorted implementation failed, use this implementation
    if (!min_p_applied) {
        // Sort the logits in descending order
        if (!cur_p->sorted) {
            std::sort(cur_p->data, cur_p->data + cur_p->size, [](const llama_token_data & a, const llama_token_data & b) {
                return a.logit > b.logit;
            });
            cur_p->sorted = true;
        }

        const float min_logit = cur_p->data[0].logit + logf(ctx->p); // min logit for p_i >= p * p_max
        size_t i = 1; // first token always matches

        for (; i < cur_p->size; ++i) {
            if (cur_p->data[i].logit < min_logit && i >= ctx->min_keep) {
                break; // prob too small
            }
        }

        // Resize the output vector to keep only the matching tokens
        cur_p->size = i;
    }
}

static struct llama_sampler * llama_sampler_min_p_addon_clone(const struct llama_sampler * smpl) {
    const auto * ctx = (const llama_sampler_min_p_addon *) smpl->ctx;
    return llama_sampler_init_min_p_addon(ctx->p, ctx->min_keep);
}

static void llama_sampler_min_p_addon_free(struct llama_sampler * smpl) {
    delete (llama_sampler_min_p_addon *) smpl->ctx;
}

static struct llama_sampler_i llama_sampler_min_p_addon_i = {
    /* .name   = */ llama_sampler_min_p_addon_name,
    /* .accept = */ nullptr,
    /* .apply  = */ llama_sampler_min_p_addon_apply,
    /* .reset  = */ nullptr,
    /* .clone  = */ llama_sampler_min_p_addon_clone,
    /* .free   = */ llama_sampler_min_p_addon_free,
};

struct llama_sampler * llama_sampler_init_min_p_addon(float p, size_t min_keep) {
    return new llama_sampler {
        /* .iface = */ &llama_sampler_min_p_addon_i,
        /* .ctx   = */ new llama_sampler_min_p_addon {
            /* .p        = */ p,
            /* .min_keep = */ min_keep,
        },
    };
}

//------------------------XTC---------------------------------

struct llama_sampler_xtc_addon {
    const float  probability;
    const float  threshold;
    const float  threshold_max;
    const bool   probability_once;
    const int    min;
    const size_t min_keep;
};

void llama_sample_xtc_addon_apply(struct llama_sampler * smpl, llama_token_data_array * candidates) {
    const auto * ctx = (llama_sampler_xtc_addon *) smpl->ctx;

    if (ctx->probability <= 0.0f || ctx->threshold <= 0.0f || ctx->min < 1 || candidates->size <= 1) {
        return;
    }

    xtc_total += candidates->size;
    xtc_percent = ((float)xtc_removed / (float)xtc_total) * 100;

    std::random_device rd;
    float chance = (float)(rd()%100)/100;
    if (ctx->probability_once && chance > ctx->probability) return;

    llama_sampler_softmax_impl(candidates);

    int removed = 0;
    // going through all candidates from back to front, easier to keep the last of probables
    for (int i = (candidates->size - 1); i >= 0; --i) {
        if (candidates->data[i].p >= ctx->threshold && candidates->data[i].p <= ctx->threshold_max) {
            if (removed == 0 || ctx->probability_once || chance <= ctx->probability) {
                ++removed;
                if (removed >= ctx->min) {
                    // .logits are used for sorting and calculating .p in llama_sample_softmax_impl
                    candidates->data[i].logit = -999.0f;
                    if (!ctx->probability_once) chance = (float)(rd()%100)/100;

                    ++xtc_removed;
                    xtc_percent = ((float)xtc_removed / (float)xtc_total) * 100;
                }
            }
        }
    }

    // still need this check
    if (removed >= ctx->min) {
        //writeCandidatesToFile("xtc_addon.txt", candidates, "\nPROCESSED:");

        // sorting with new logits, ex-last probable will be the first anyway
        std::sort(candidates->data, candidates->data + candidates->size, [](const llama_token_data & a, const llama_token_data & b) {
            return a.logit > b.logit;
        });

        // resizing now that penalized tokens are at the back
        candidates->size = candidates->size - removed + 1;

        // std::ofstream file("xtc_test2.txt", std::ios::app);
        // if (file.is_open()) {
            // file << xtc_log;
            // file.close();
        // }
    }
}

static const char * llama_sampler_xtc_addon_name(const struct llama_sampler * /*smpl*/) {
    return "xtc";
}

static struct llama_sampler * llama_sampler_xtc_addon_clone(const struct llama_sampler * smpl) {
    const auto * ctx = (const llama_sampler_xtc_addon *) smpl->ctx;
    return llama_sampler_init_xtc_addon(ctx->probability, ctx->threshold, ctx->threshold_max, ctx->probability_once, ctx->min, ctx->min_keep);
}

static void llama_sampler_xtc_addon_free(struct llama_sampler * smpl) {
    delete (llama_sampler_xtc_addon *) smpl->ctx;
}

static struct llama_sampler_i llama_sampler_xtc_addon_i = {
    /* .name   = */ llama_sampler_xtc_addon_name,
    /* .accept = */ nullptr,
    /* .apply  = */ llama_sample_xtc_addon_apply,
    /* .reset  = */ nullptr,
    /* .clone  = */ llama_sampler_xtc_addon_clone,
    /* .free   = */ llama_sampler_xtc_addon_free,
};

struct llama_sampler * llama_sampler_init_xtc_addon(float probability, float threshold, float threshold_max, bool probability_once, int min, size_t min_keep) {
    return new llama_sampler {
        /* .iface = */ &llama_sampler_xtc_addon_i,
        /* .ctx   = */ new llama_sampler_xtc_addon {
            /* .p        = */ probability,
            /* .min_keep = */ threshold,
            /* .min_keep = */ threshold_max,
            /* .min_keep = */ probability_once,
            /* .min_keep = */ min,
            /* .min_keep = */ min_keep,
        },
    };
}

//------------------------P-STEP---------------------------------

struct llama_sampler_p_step_addon {
    const float  step;
    const size_t min_keep;
};

void llama_sample_p_step_addon_apply(struct llama_sampler * smpl, llama_token_data_array * candidates) {
    const auto * ctx = (llama_sampler_p_step_addon *) smpl->ctx;

    if (ctx->step <= 0.0f || candidates->size <= 1) {
        return;
    }

    llama_sampler_softmax_impl(candidates);

    bool step_found = false;
    
    // Variables to hold the external values
    float randomizationFactor = 1.0f; // Default value of the randomization factor
    bool isTrueRNG = true; // Default value for RNG type, set to true for true randomness
    unsigned int rngSeed = 123456789; // Default seed value for deterministic RNG

    // Check if the randomizationFactor value is above 0 and apply Gaussian noise if so
    if (randomizationFactor > 0.0) {

        // Create a random number generator
        std::default_random_engine generator;
        if (isTrueRNG) {
            // Seed with a real random value, if available
            std::random_device rd;
            generator.seed(rd());
        } else {
            // Use a fixed seed for deterministic behavior
            generator.seed(rngSeed);
        }

        // Create a Gaussian distribution with mean 0 and standard deviation of your choice
        std::normal_distribution<float> distribution(0.0f, randomizationFactor); // Replace 1.0f with the desired standard deviation

        // Apply Gaussian noise to each logit
        for (size_t i = 0; i < candidates->size; ++i) {
            // Add Gaussian noise to the logit
            candidates->data[i].logit += distribution(generator);
        }

        candidates->sorted = false;

        // Re-normalize probabilities if necessary
        llama_sampler_softmax_impl(candidates);

    }

    for (size_t i = 1; i < candidates->size; ++i) {
        if (!step_found && candidates->data[i].p < ctx->step * candidates->data[i - 1].p) {
            step_found = true;
        }

        if (step_found && i >= ctx->min_keep) {
            // Resize the output vector to keep only the tokens before the step
            candidates->size = i;
            break;
        }
    }
}

static const char * llama_sampler_p_step_addon_name(const struct llama_sampler * /*smpl*/) {
    return "p_step";
}

static struct llama_sampler * llama_sampler_p_step_addon_clone(const struct llama_sampler * smpl) {
    const auto * ctx = (const llama_sampler_p_step_addon *) smpl->ctx;
    return llama_sampler_init_p_step_addon(ctx->step, ctx->min_keep);
}

static void llama_sampler_p_step_addon_free(struct llama_sampler * smpl) {
    delete (llama_sampler_p_step_addon *) smpl->ctx;
}

static struct llama_sampler_i llama_sampler_p_step_addon_i = {
    /* .name   = */ llama_sampler_p_step_addon_name,
    /* .accept = */ nullptr,
    /* .apply  = */ llama_sample_p_step_addon_apply,
    /* .reset  = */ nullptr,
    /* .clone  = */ llama_sampler_p_step_addon_clone,
    /* .free   = */ llama_sampler_p_step_addon_free,
};

struct llama_sampler * llama_sampler_init_p_step_addon(float step, size_t min_keep) {
    return new llama_sampler {
        /* .iface = */ &llama_sampler_p_step_addon_i,
        /* .ctx   = */ new llama_sampler_p_step_addon {
            /* .step     = */ step,
            /* .min_keep = */ min_keep,
        },
    };
}

//------------------------TEMP WITH SMOOTHING---------------------------------

struct llama_sampler_temp_ext_addon {
    const float temp;
    const float delta;
    const float exponent;
    const float smoothing_factor;
    const float smoothing_curve;
};

static const char * llama_sampler_temp_ext_addon_name(const struct llama_sampler * /*smpl*/) {
    return "temp-ext";
}

static void llama_sampler_temp_ext_addon_apply(struct llama_sampler * smpl, llama_token_data_array * cur_p) {
    const auto * ctx = (llama_sampler_temp_ext_addon *) smpl->ctx;
    if (ctx->delta > 0) {
        const float min_temp = std::max(0.0f, ctx->temp - ctx->delta);
        const float max_temp = ctx->temp + ctx->delta;
        float exponent_val = ctx->exponent;
        float smoothing_factor = ctx->smoothing_factor;
        float smoothing_curve = ctx->smoothing_curve;

        // no need to do anything if there is only one (or zero) candidates
        if (cur_p->size <= 1) {
            return;
        }

        // Apply smoothing if smoothing_factor is > 0. Do not change base implementation otherwise.
        if (smoothing_factor > 0 && cur_p->size > 1) {
            llama_sampler_softmax_impl(cur_p);
            float h = cur_p->data[0].logit; // Find the maximum logit for h to be added after the transformation

            // Apply the modified quadratic transformation using the smoothing_factor and smoothing_curve
            for (size_t i = 0; i < cur_p->size; ++i) {
                float logit_shifted = cur_p->data[i].logit - h;
                float k = (3 - smoothing_curve) / 2;
                float s = (smoothing_curve - 1) / 2;
                cur_p->data[i].logit = -(k * smoothing_factor * logit_shifted * logit_shifted) + (s * smoothing_factor * logit_shifted * logit_shifted * logit_shifted) + h;
            }
            llama_sampler_softmax_impl(cur_p);
        }

        // Calculate maximum possible entropy
        float max_entropy = -logf(1.0f / cur_p->size);

        llama_sampler_softmax_impl(cur_p);

        // Calculate entropy of the softmax probabilities
        float entropy = 0.0f;
        for (size_t i = 0; i < cur_p->size; ++i) {
            float prob = cur_p->data[i].p;
            if (prob > 0.0f) { // Ensure no log(0)
                entropy -= prob * logf(prob);
            }
        }

        // Normalize the entropy (max_entropy cannot be 0 here because we checked cur_p->size != 1 above)
        float normalized_entropy = entropy / max_entropy;

        // Map the normalized entropy to the desired temperature range using the power function
        float dyn_temp = min_temp + (max_temp - min_temp) * powf(normalized_entropy, exponent_val);

    #ifdef DEBUG
        LLAMA_LOG_INFO("Your text maxtemp value is: %f\n", max_temp);
        LLAMA_LOG_INFO("Entropy: %f\n", entropy);
        LLAMA_LOG_INFO("Max Possible Entropy: %f\n", max_entropy);
        LLAMA_LOG_INFO("Normalized Entropy: %f\n", normalized_entropy);
        LLAMA_LOG_INFO("Exponent: %f\n", exponent_val);
        LLAMA_LOG_INFO("Dynamic Temperature (dyn_temp): %f\n", dyn_temp);
    #endif

        // Apply the dynamically calculated temperature scaling
        for (size_t i = 0; i < cur_p->size; ++i) {
            cur_p->data[i].logit /= dyn_temp;
        }

        // Re-compute softmax probabilities after scaling logits with dynamic temperature
        const double max_l_double = cur_p->data[0].logit;

        double cum_sum_double = 0.0;
        for (size_t i = 0; i < cur_p->size; ++i) {
            double p = exp(cur_p->data[i].logit - max_l_double);
            cur_p->data[i].p = p; // Store the scaled probability
            cum_sum_double += p;
        }

        for (size_t i = 0; i < cur_p->size; ++i) {
            cur_p->data[i].p /= cum_sum_double; // Re-normalize the probabilities
        }

    #ifdef DEBUG
        // Print the updated top 25 probabilities after temperature scaling
        LLAMA_LOG_INFO("\nUpdated Top 25 Probabilities After Dynamic Temperature Scaling (in percentages):\n");
        for (size_t i = 0; i < 25 && i < cur_p->size; ++i) {
            LLAMA_LOG_INFO("Token %zu: %f%%\n", i + 1, cur_p->data[i].p * 100.0f);
        }
    #endif
    } else {
        for (size_t i = 0; i < cur_p->size; ++i) {
            cur_p->data[i].logit /= ctx->temp;
        }
    }
}

static struct llama_sampler * llama_sampler_temp_ext_addon_clone(const struct llama_sampler * smpl) {
    const auto * ctx = (const llama_sampler_temp_ext_addon *) smpl->ctx;
    return llama_sampler_init_temp_ext_addon(ctx->temp, ctx->delta, ctx->exponent, ctx->smoothing_factor, ctx->smoothing_curve);
}

static void llama_sampler_temp_ext_addon_free(struct llama_sampler * smpl) {
    delete (llama_sampler_temp_ext_addon *) smpl->ctx;
}

static struct llama_sampler_i llama_sampler_temp_ext_addon_i = {
    /* .name   = */ llama_sampler_temp_ext_addon_name,
    /* .accept = */ nullptr,
    /* .apply  = */ llama_sampler_temp_ext_addon_apply,
    /* .reset  = */ nullptr,
    /* .clone  = */ llama_sampler_temp_ext_addon_clone,
    /* .free   = */ llama_sampler_temp_ext_addon_free,
};

struct llama_sampler * llama_sampler_init_temp_ext_addon(float temp, float delta, float exponent, float smoothing_factor, float smoothing_curve) {
    return new llama_sampler {
        /* .iface = */ &llama_sampler_temp_ext_addon_i,
        /* .ctx   = */ new llama_sampler_temp_ext_addon {
            /* .temp             = */ temp,
            /* .delta            = */ delta,
            /* .exponent         = */ exponent,
            /* .smoothing_factor = */ smoothing_factor,
            /* .smoothing_curve  = */ smoothing_curve,
        },
    };
}

//------------------------PENALTIES WITH THRESHOLD---------------------------------

struct llama_sampler_penalties_addon {
    const int32_t     n_vocab;
    const llama_token special_eos_id;
    const llama_token linefeed_id;

    const int32_t penalty_last_n;
    const float   penalty_repeat;
    const float   penalty_freq;
    const float   penalty_present;
    const float   penalty_threshold;

    const bool    penalize_nl;
    const bool    ignore_eos;

    ring_buffer<llama_token> prev;
};

static const char * llama_sampler_penalties_addon_name(const struct llama_sampler * /*smpl*/) {
    return "penalties";
}

static void llama_sampler_penalties_addon_accept(struct llama_sampler * smpl, llama_token token) {
    auto * ctx = (llama_sampler_penalties_addon *) smpl->ctx;
    if (ctx->penalty_last_n == 0) {
        return;
    }

    ctx->prev.push_back(token);
}

static void llama_sampler_penalties_addon_apply(struct llama_sampler * smpl, llama_token_data_array * cur_p) {
    auto * ctx = (llama_sampler_penalties_addon *) smpl->ctx;

    if (ctx->ignore_eos) {
        assert(ctx->special_eos_id >= 0);

        // optimistically check if the candidates are not yet sorted/shuffled/truncated
        if (cur_p->size > (size_t) ctx->special_eos_id && cur_p->data[ctx->special_eos_id].id == ctx->special_eos_id) {
            cur_p->data[ctx->special_eos_id].logit = -INFINITY;
        } else {
            // else, search for the special EOS token
            for (size_t i = 0; i < cur_p->size; ++i) {
                if (cur_p->data[i].id == ctx->special_eos_id) {
                    cur_p->data[i].logit = -INFINITY;
                    break;
                }
            }
        }
    }

    if ((ctx->penalty_last_n == 0) ||
        (ctx->penalty_repeat == 1.0f && ctx->penalty_freq == 0.0f && ctx->penalty_present == 0.0f && ctx->penalty_threshold == 0.0f)) {
        return;
    }

    bool nl_found = false;
    size_t nl_idx = 0;
    float nl_logit = -INFINITY;
    if (!ctx->penalize_nl) {
        assert(ctx->linefeed_id >= 0);

        // optimistically check if the candidates are not yet sorted/shuffled/truncated
        if (cur_p->size > (size_t) ctx->linefeed_id && cur_p->data[ctx->linefeed_id].id == ctx->linefeed_id) {
            nl_found = true;
            nl_idx = ctx->linefeed_id;
            nl_logit = cur_p->data[ctx->linefeed_id].logit;
        } else {
            // else, search for the linefeed token
            for (size_t i = 0; i < cur_p->size; ++i) {
                if (cur_p->data[i].id == ctx->linefeed_id) {
                    nl_found = true;
                    nl_idx = i;
                    nl_logit = cur_p->data[i].logit;
                    break;
                }
            }
        }
    }

    // Create a frequency map to count occurrences of each token in last_tokens
    // TODO: optimize this by maintaining the token count in the sampler context
    using llama_token_cnt = std::unordered_map<llama_token, int>;
    llama_token_cnt token_count;

    for (int i = 0; i < std::min<int>(ctx->penalty_last_n, ctx->prev.size()); ++i) {
        token_count[ctx->prev.rat(i)]++;
    }

    // Apply frequency and presence penalties to the cur_p
    for (size_t i = 0; i < cur_p->size; ++i) {
        const auto token_iter = token_count.find(cur_p->data[i].id);
        if (token_iter == token_count.end()) {
            continue;
        }

        const int count = token_iter->second;

        if (float(count) / float(ctx->penalty_last_n) > ctx->penalty_threshold) {
            continue;
        }

        // The academic publication that described this technique actually just only divided, but that would cause tokens with negative logits to become more likely, which is obviously wrong.
        // This is common fix for this problem, which is to multiply by the penalty instead of dividing.
        if (cur_p->data[i].logit <= 0) {
            cur_p->data[i].logit *= ctx->penalty_repeat;
        } else {
            cur_p->data[i].logit /= ctx->penalty_repeat;
        }

        cur_p->data[i].logit -= float(count) * ctx->penalty_freq + float(count > 0) * ctx->penalty_present;
    }

    cur_p->sorted = false;

    if (!ctx->penalize_nl && nl_found) {
        // restore the logit of the newline token if it was penalized
        cur_p->data[nl_idx].logit = nl_logit;
    }
}

static void llama_sampler_penalties_addon_reset(struct llama_sampler * smpl) {
    auto * ctx = (llama_sampler_penalties_addon *) smpl->ctx;
    ctx->prev.clear();
}

static struct llama_sampler * llama_sampler_penalties_addon_clone(const struct llama_sampler * smpl) {
    const auto * ctx = (const llama_sampler_penalties_addon *) smpl->ctx;
    auto * result = llama_sampler_init_penalties_addon(
            ctx->n_vocab,
            ctx->special_eos_id,
            ctx->linefeed_id,
            ctx->penalty_last_n,
            ctx->penalty_repeat,
            ctx->penalty_freq,
            ctx->penalty_present,
            ctx->penalty_threshold,
            ctx->penalize_nl,
            ctx->ignore_eos);

    // copy the state
    {
        auto * result_ctx = (llama_sampler_penalties_addon *) result->ctx;

        result_ctx->prev = ctx->prev;
    }

    return result;
}

static void llama_sampler_penalties_addon_free(struct llama_sampler * smpl) {
    delete (llama_sampler_penalties_addon *) smpl->ctx;
}

static struct llama_sampler_i llama_sampler_penalties_addon_i = {
    /* .name   = */ llama_sampler_penalties_addon_name,
    /* .accept = */ llama_sampler_penalties_addon_accept,
    /* .apply  = */ llama_sampler_penalties_addon_apply,
    /* .reset  = */ llama_sampler_penalties_addon_reset,
    /* .clone  = */ llama_sampler_penalties_addon_clone,
    /* .free   = */ llama_sampler_penalties_addon_free,
};

struct llama_sampler * llama_sampler_init_penalties_addon(
        int32_t n_vocab,
        llama_token special_eos_id,
        llama_token linefeed_id,
        int32_t penalty_last_n,
        float penalty_repeat,
        float penalty_freq,
        float penalty_present,
        float penalty_threshold,
        bool penalize_nl,
        bool ignore_eos) {
    if (linefeed_id == LLAMA_TOKEN_NULL) {
        penalize_nl = true;
    }

    if (special_eos_id == LLAMA_TOKEN_NULL) {
        ignore_eos = false;
    }

    return new llama_sampler {
        /* .iface = */ &llama_sampler_penalties_addon_i,
        /* .ctx   = */ new llama_sampler_penalties_addon {
            /* .n_vocab         = */ n_vocab,
            /* .special_eos_id  = */ special_eos_id,
            /* .linefeed_id     = */ linefeed_id,
            /* .penalty_last_n  = */ penalty_last_n,
            /* .penalty_repeat  = */ penalty_repeat,
            /* .penalty_freq    = */ penalty_freq,
            /* .penalty_present = */ penalty_present,
            /* .penalty_threshold = */ penalty_threshold,
            /* .penalize_nl     = */ penalize_nl,
            /* .ignore_eos      = */ ignore_eos,
            /* .prev            = */ ring_buffer<llama_token>(penalty_last_n),
        },
    };
}

//------------------------DRY---------------------------------

/*

struct llama_sampler_dry_addon {
    const float dry_base;
    const float dry_multiplier;
    const int dry_allowed_length;
    const size_t dry_seq_breakers_size;

    const llama_token last_tokens;
    size_t last_tokens_size;
    const llama_token dry_seq_breakers;

    //ring_buffer<llama_token> prev;
};

static const char * llama_sampler_dry_addon_name(const struct llama_sampler * ) {
    return "DRY";
}

void llama_sampler_dry_addon_apply(struct llama_sampler * smpl, llama_token_data_array * candidates) {
    auto * ctx = (llama_sampler_dry_addon *) smpl->ctx;

    // skip dry sampler if we don't have a previous token
    if (ctx->last_tokens_size < 1) return;

    // get the last token
    auto last_token = ctx->last_tokens[ctx->last_tokens_size - 1];

    // if last token is part of the sequence breakers, skip whole sampler
    if (std::find(ctx->dry_seq_breakers, ctx->dry_seq_breakers + ctx->dry_seq_breakers_size, last_token) != ctx->dry_seq_breakers + ctx->dry_seq_breakers_size) {
        return;
    }

    // create an unordered map of "next tokens" <-> max match length
    std::unordered_map<llama_token, size_t> match_lengths;

    // loop through each previous token (exclude the last token)
    for (size_t i = 0; i < ctx->last_tokens_size - 1; ++i) {
        // skip if the compare token is not the same as the last token
        if (ctx->last_tokens[i] != last_token) {
            continue;
        }

        // get the next token (i + 1 is always less than last_tokens_size)
        auto next_token = ctx->last_tokens[i + 1];

        // if next token is part of the sequence breakers, skip
        if (std::find(ctx->dry_seq_breakers, ctx->dry_seq_breakers + ctx->dry_seq_breakers_size, next_token) != ctx->dry_seq_breakers + ctx->dry_seq_breakers_size) {
            continue;
        }

        // try to extend the match backwards (match length starts at 1 because last token is already matched)
        size_t match_length = 1;

        // loop through the previous tokens
        for (;; match_length++) {
            // if we have reached the start of our last tokens, break
            if (i < match_length) break;

            // compare token starts at our prev index, going backwards by match length
            auto compare_token = ctx->last_tokens[i - match_length];

            // head token starts at the end of last tokens, going backwards by match length, minus 1 because we start at the last token itself
            auto head_token = ctx->last_tokens[ctx->last_tokens_size - 1 - match_length];

            // break out of the match if any tokens don't match
            if (compare_token != head_token) {
                break;
            }

            // if compare token is part of the sequence breakers, break out of the match
            if (std::find(ctx->dry_seq_breakers, ctx->dry_seq_breakers + ctx->dry_seq_breakers_size, compare_token) != ctx->dry_seq_breakers + ctx->dry_seq_breakers_size) {
                break;
            }
        }

        // Check if the next token exists in the map
        auto it = match_lengths.find(next_token);

        if (it == match_lengths.end()) {
            // Key does not exist, insert the new value
            match_lengths[next_token] = match_length;
        } else {
            // Key exists, update it with the max of the new value or the existing value
            it->second = std::max(it->second, match_length);
        }
    }

    // apply penalties
    for (const auto& pair : match_lengths) {
        auto next_token = pair.first;
        auto match_length = pair.second;

        // if the match length is greater than or equal to our allowed length in config, we apply penalities
        if (match_length >= (size_t)dry_allowed_length) {

            // find our next token in the candidates->data
            for (size_t i = 0; i < candidates->size; ++i) {
                if (candidates->data[i].id == next_token) {
                    // calculate the penalty
                    float penalty = ctx->dry_multiplier * pow(ctx->dry_base, match_length - ctx->dry_allowed_length);

                    // apply the dry penalty
                    candidates->data[i].logit -= penalty;
                    break;
                }
            }
        }
    }
}


static void llama_sampler_dry_addon_accept(struct llama_sampler * smpl, llama_token token) {
    auto * ctx = (llama_sampler_dry_addon_apply *) smpl->ctx;
    if (ctx->penalty_last_n == 0) {
        return;
    }

    ctx->prev.push_back(token);
}

static void llama_sampler_dry_addon_reset(struct llama_sampler * smpl) {
    auto * ctx = (llama_sampler_dry_addon *) smpl->ctx;
    ctx->prev.clear();
}

static struct llama_sampler * llama_sampler_dry_addon_clone(const struct llama_sampler * smpl) {
    const auto * ctx = (const llama_sampler_dry_addon *) smpl->ctx;
    auto * result = llama_sampler_init_dry_addon(
            ctx->dry_base,
            ctx->dry_multiplier,
            ctx->dry_allowed_length,
            ctx->dry_seq_breakers_size,
            ctx->last_tokens,
            ctx->last_tokens_size,
            ctx->dry_seq_breakers);

    // copy the state
    {
        auto * result_ctx = (llama_sampler_dry_addon *) result->ctx;

        result_ctx->prev = ctx->prev;
    }

    return result;
}

static void llama_sampler_dry_addon_free(struct llama_sampler * smpl) {
    delete (llama_sampler_dry_addon *) smpl->ctx;
}

static struct llama_sampler_i llama_sampler_dry_addon_i = {
    llama_sampler_dry_addon_name,
    llama_sampler_dry_addon_accept,
    llama_sampler_dry_addon_apply,
    llama_sampler_dry_addon_reset,
    llama_sampler_dry_addon_clone,
    llama_sampler_dry_addon_free,
};

struct llama_sampler * llama_sampler_init_dry_addon(
        const float       dry_base,
        const float       dry_multiplier,
        const int         dry_allowed_length,
        const size_t      dry_seq_breakers_size,
        const llama_token last_tokens,
        size_t            last_tokens_size,
        const llama_token dry_seq_breakers) {

    return new llama_sampler {
        &llama_sampler_dry_addon_i,
        new llama_sampler_dry_addon {
            dry_base,
            dry_multiplier,
            dry_allowed_length,
            dry_seq_breakers_size,
            last_tokens,
            last_tokens_size,
            dry_seq_breakers,
        },
    };
}

*/