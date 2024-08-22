#include "ggml.h"
#include "ggml-alloc.h"
#include "ggml-backend.h"
#include "llama.h"

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

void llama_sample_xtc_addon(struct llama_context * ctx, llama_token_data_array * candidates, float xtc_probability, float xtc_threshold, bool xtc_probability_once, int xtc_min, size_t min_keep) {
    if (xtc_probability <= 0.0f || xtc_threshold <= 0.0f || candidates->size <= 1) {
        return;
    }

    std::random_device rd;
    float chance = (float)(rd()%100)/100;
    if (xtc_probability_once && chance > xtc_probability) return;

    llama_sample_softmax(nullptr, candidates);

    const int64_t t_start_sample_us = ggml_time_us();
    int id_first = -1;
    size_t removed = 0;
    for (size_t i = 0; i < (candidates->size - 1); ++i) {
        if (candidates->data[i].p >= xtc_threshold) {
                if (id_first == -1) {
                    id_first = i;
                    ++removed;
                } else if (xtc_probability_once || chance <= xtc_probability) {
                    // .logits are used for sorting and calculating .p in llama_sample_softmax_impl
                    candidates->data[i].logit = -999.0f;
                    if (!xtc_probability_once) chance = (float)(rd()%100)/100;
                    ++removed;
                }
        }
    }

    if (removed >= xtc_min) {
        // penalizing by first id
        if (xtc_probability_once || chance <= xtc_probability) candidates->data[id_first].logit = -999.0f;
        // sorting with new logits
        std::sort(candidates->data, candidates->data + candidates->size, [](const llama_token_data & a, const llama_token_data & b) {
            return a.logit > b.logit;
        });
        //resizing now that penalized tokens are at the back
        candidates->size = candidates->size - removed;
    }
    llama_set_time(ctx, t_start_sample_us);
}

void llama_sample_p_step_addon(struct llama_context * ctx, llama_token_data_array * candidates, float step, size_t min_keep) {
    if (step <= 0.0f || candidates->size <= 1) {
        return;
    }

    llama_sample_softmax(nullptr, candidates);

    const int64_t t_start_sample_us = ggml_time_us();

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
        llama_sample_softmax(ctx, candidates);

    }

    for (size_t i = 1; i < candidates->size; ++i) {
        if (!step_found && candidates->data[i].p < step * candidates->data[i - 1].p) {
            step_found = true;
        }

        if (step_found && i >= min_keep) {
            // Resize the output vector to keep only the tokens before the step
            candidates->size = i;
            break;
        }
    }

    llama_set_time(ctx, t_start_sample_us);
}

void llama_sample_min_p_addon(struct llama_context * ctx, llama_token_data_array * candidates, float p, size_t min_keep) {
    if (p <= 0.0f || !candidates->size) {
        return;
    }

    const int64_t t_start_sample_us = ggml_time_us();

    //llama_sample_softmax(ctx, candidates);
    
    bool min_p_applied = false;

    // if the candidates aren't sorted, try the unsorted implementation first
    if (!candidates->sorted) {
        std::vector<llama_token_data> filtered_tokens;

        float max_logit = -FLT_MAX;
        for (size_t i = 0; i < candidates->size; ++i) {
            max_logit = std::max(max_logit, candidates->data[i].logit);
        }
        const float min_logit = max_logit + logf(p); // min logit for p_i >= p * p_max

        for (size_t i = 0; i < candidates->size; ++i) {
            if (candidates->data[i].logit >= min_logit) {
                filtered_tokens.push_back(candidates->data[i]);
            }
        }
        
        // if we have enough values the operation was a success
        if (filtered_tokens.size() >= min_keep) {
            memcpy(candidates->data, filtered_tokens.data(), filtered_tokens.size()*sizeof(llama_token_data));
            candidates->size = filtered_tokens.size();
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
        for (size_t i = 0; i < candidates->size; ++i) {
            // Add Gaussian noise to the logit
            candidates->data[i].logit += distribution(generator);
        }

        candidates->sorted = false;
    }

    // Store original top probability
    float original_top_prob = candidates->data[0].p;
    
    // if the candidates are sorted or the unsorted implementation failed, use this implementation
    if (!min_p_applied) {
        // Sort the logits in descending order
        if (!candidates->sorted) {
            std::sort(candidates->data, candidates->data + candidates->size, [](const llama_token_data & a, const llama_token_data & b) {
                return a.logit > b.logit;
            });
            candidates->sorted = true;
        }

        const float min_logit = candidates->data[0].logit + logf(p); // min logit for p_i >= p * p_max
        size_t i = 1; // first token always matches

        for (; i < candidates->size; ++i) {
            if (candidates->data[i].logit < min_logit && i >= min_keep) {
                break; // prob too small
            }
        }

        // Resize the output vector to keep only the matching tokens
        candidates->size = i;
    }

    llama_set_time(ctx, t_start_sample_us);
}

void llama_sample_repetition_penalties_addon(
            struct llama_context * ctx,
          llama_token_data_array * candidates,
               const llama_token * last_tokens,
                          size_t   penalty_last_n,
                           float   penalty_repeat,
                           float   penalty_freq,
                           float   penalty_present,
                           float   penalty_threshold) {
    if (penalty_last_n == 0 || penalty_threshold == 0.0f || (penalty_repeat == 1.0f && penalty_freq == 0.0f && penalty_present == 0.0f)) {
        return;
    }

    const int64_t t_start_sample_us = ggml_time_us();

    // Create a frequency map to count occurrences of each token in last_tokens
    std::unordered_map<llama_token, int> token_count;
    for (size_t i = 0; i < penalty_last_n; ++i) {
        token_count[last_tokens[i]]++;
    }

    // Apply frequency and presence penalties to the candidates
    for (size_t i = 0; i < candidates->size; ++i) {
        const auto token_iter = token_count.find(candidates->data[i].id);
        if (token_iter == token_count.end()) {
            continue;
        }

        const int count = token_iter->second;
        
        if (float(count) / float(penalty_last_n) > penalty_threshold) {
            continue;
        }

        // The academic publication that described this technique actually just only divided, but that would cause tokens with negative logits to become more likely, which is obviously wrong.
        // This is common fix for this problem, which is to multiply by the penalty instead of dividing.
        if (candidates->data[i].logit <= 0) {
            candidates->data[i].logit *= penalty_repeat;
        } else {
            candidates->data[i].logit /= penalty_repeat;
        }

        candidates->data[i].logit -= float(count) * penalty_freq + float(count > 0) * penalty_present;
    }

    candidates->sorted = false;

    llama_set_time(ctx, t_start_sample_us);
}

void llama_sample_temp_addon(struct llama_context * ctx, llama_token_data_array * candidates, float temp, float smoothing_factor, float smoothing_curve) {
    // Get current time
    const int64_t t_start_sample_us = ggml_time_us();

    // Apply temperature scaling
    for (size_t i = 0; i < candidates->size; ++i) {
        candidates->data[i].logit /= temp;
    }
    
    llama_sample_softmax(ctx, candidates);


    // Find min and max logits for normalization
    float min_logit = candidates->data[0].logit;
    float max_logit = candidates->data[0].logit;
    for (size_t i = 1; i < candidates->size; ++i) {
        if (candidates->data[i].logit < min_logit) min_logit = candidates->data[i].logit;
        if (candidates->data[i].logit > max_logit) max_logit = candidates->data[i].logit;
    }

    // Only apply smoothing if smoothing_factor is not 0
    if (smoothing_factor != 0) {
        // Apply the remapping and sigmoid function
        float new_min_logit = FLT_MAX;
        float new_max_logit = FLT_MIN;
        for (size_t i = 0; i < candidates->size; ++i) {
            // Normalize the logits to the [0,1] range
            float normalized_logit = (candidates->data[i].logit - min_logit) / (max_logit - min_logit);

            // Apply the sigmoid function to the normalized logits
            float sigmoid_logit = smoothing_curve / (smoothing_curve + expf(-smoothing_factor * (normalized_logit - 0.5f)));

            // Update the logits with the smoothed values
            candidates->data[i].logit = sigmoid_logit * (max_logit - min_logit) + min_logit;

            // Find new min and max logits after smoothing
            if (candidates->data[i].logit < new_min_logit) new_min_logit = candidates->data[i].logit;
            if (candidates->data[i].logit > new_max_logit) new_max_logit = candidates->data[i].logit;
        }

        // Rescale logits again so that new min and max logits match original min and max logits
        for (size_t i = 0; i < candidates->size; ++i) {
            candidates->data[i].logit = (candidates->data[i].logit - new_min_logit) / (new_max_logit - new_min_logit) * (max_logit - min_logit) + min_logit;
        }

        llama_sample_softmax(ctx, candidates);

    } 
    //else {
    //    // Print message indicating skipping of the smoothing
    //    printf("--------\nSkipping smoothing as smoothing_factor is 0.\n--------");
    //}

    // Update timing in context if ctx is available
    llama_set_time(ctx, t_start_sample_us);
}

void llama_sample_entropy_addon(struct llama_context * ctx, llama_token_data_array * candidates_p, float min_temp, float max_temp, float exponent_val, float smoothing_factor, float smoothing_curve) {
    const int64_t t_start_sample_us = ggml_time_us();

    // no need to do anything if there is only one (or zero) candidates
    if (candidates_p->size <= 1) {
        return;
    }

    // Apply smoothing if smoothing_factor is > 0. Do not change base implementation otherwise.
    if (smoothing_factor > 0 && candidates_p->size > 1) {
        llama_sample_softmax(ctx, candidates_p);
        float h = candidates_p->data[0].logit; // Find the maximum logit for h to be added after the transformation

        // Apply the modified quadratic transformation using the smoothing_factor and smoothing_curve
        for (size_t i = 0; i < candidates_p->size; ++i) {
            float logit_shifted = candidates_p->data[i].logit - h;
            float k = (3 - smoothing_curve) / 2;
            float s = (smoothing_curve - 1) / 2;
            candidates_p->data[i].logit = -(k * smoothing_factor * logit_shifted * logit_shifted) + (s * smoothing_factor * logit_shifted * logit_shifted * logit_shifted) + h;
        }
        llama_sample_softmax(ctx, candidates_p);
    }


    // Calculate maximum possible entropy
    float max_entropy = -logf(1.0f / candidates_p->size);

    llama_sample_softmax(nullptr, candidates_p);

    // Calculate entropy of the softmax probabilities
    float entropy = 0.0f;
    for (size_t i = 0; i < candidates_p->size; ++i) {
        float prob = candidates_p->data[i].p;
        if (prob > 0.0f) { // Ensure no log(0)
            entropy -= prob * logf(prob);
        }
    }

    // Normalize the entropy (max_entropy cannot be 0 here because we checked candidates_p->size != 1 above)
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
    for (size_t i = 0; i < candidates_p->size; ++i) {
        candidates_p->data[i].logit /= dyn_temp;
    }

    // Re-compute softmax probabilities after scaling logits with dynamic temperature
    double max_l_double = candidates_p->data[0].logit;
    double cum_sum_double = 0.0;
    for (size_t i = 0; i < candidates_p->size; ++i) {
        double p = exp(candidates_p->data[i].logit - max_l_double);
        candidates_p->data[i].p = p; // Store the scaled probability
        cum_sum_double += p;
    }
    for (size_t i = 0; i < candidates_p->size; ++i) {
        candidates_p->data[i].p /= cum_sum_double; // Re-normalize the probabilities
    }

#ifdef DEBUG
    // Print the updated top 25 probabilities after temperature scaling
    LLAMA_LOG_INFO("\nUpdated Top 25 Probabilities After Dynamic Temperature Scaling (in percentages):\n");
    for (size_t i = 0; i < 25 && i < candidates_p->size; ++i) {
        LLAMA_LOG_INFO("Token %zu: %f%%\n", i + 1, candidates_p->data[i].p * 100.0f);
    }
#endif

    llama_set_time(ctx, t_start_sample_us);
}

void llama_sample_dry(llama_token_data_array * candidates, const llama_token * last_tokens, size_t last_tokens_size, float dry_base, float dry_multiplier, int dry_allowed_length, const llama_token * dry_seq_breakers, size_t dry_seq_breakers_size) {
    // skip dry sampler if we don't have a previous token
    if (last_tokens_size < 1) return;

    // get the last token
    auto last_token = last_tokens[last_tokens_size - 1];

    // if last token is part of the sequence breakers, skip whole sampler
    if (std::find(dry_seq_breakers, dry_seq_breakers + dry_seq_breakers_size, last_token) != dry_seq_breakers + dry_seq_breakers_size) {
        return;
    }

    // create an unordered map of "next tokens" <-> max match length
    std::unordered_map<llama_token, size_t> match_lengths;

    // loop through each previous token (exclude the last token)
    for (size_t i = 0; i < last_tokens_size - 1; ++i) {
        // skip if the compare token is not the same as the last token
        if (last_tokens[i] != last_token) {
            continue;
        }

        // get the next token (i + 1 is always less than last_tokens_size)
        auto next_token = last_tokens[i + 1];

        // if next token is part of the sequence breakers, skip
        if (std::find(dry_seq_breakers, dry_seq_breakers + dry_seq_breakers_size, next_token) != dry_seq_breakers + dry_seq_breakers_size) {
            continue;
        }

        // try to extend the match backwards (match length starts at 1 because last token is already matched)
        size_t match_length = 1;

        // loop through the previous tokens
        for (;; match_length++) {
            // if we have reached the start of our last tokens, break
            if (i < match_length) break;

            // compare token starts at our prev index, going backwards by match length
            auto compare_token = last_tokens[i - match_length];

            // head token starts at the end of last tokens, going backwards by match length, minus 1 because we start at the last token itself
            auto head_token = last_tokens[last_tokens_size - 1 - match_length];

            // break out of the match if any tokens don't match
            if (compare_token != head_token) {
                break;
            }

            // if compare token is part of the sequence breakers, break out of the match
            if (std::find(dry_seq_breakers, dry_seq_breakers + dry_seq_breakers_size, compare_token) != dry_seq_breakers + dry_seq_breakers_size) {
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
                    float penalty = dry_multiplier * pow(dry_base, match_length - dry_allowed_length);

                    // apply the dry penalty
                    candidates->data[i].logit -= penalty;
                    break;
                }
            }
        }
    }
}