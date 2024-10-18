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

//statistics
int temp_total = 0;
int min_p_total = 0;
int p_step_total = 0;
int xtc_total = 0;
int xtc_removed = 0;
float xtc_percent = 0.0;
int candidates_max = 0;
std::string last_candidates = "";

int rx_total = 0;
int rx_removed = 0;
float rx_percent = 0.0;

int k_total = 99;

bool k_set = false;

static bool writeToFile(std::string path, std::string text){
    std::ofstream file(path, std::ios::app);
    if (file.is_open()) {
        file << text;
        file.close();
        return true;
    } else {
        return false;
    }
}

static bool writeFloatToFile(std::string path, float data, std::string add){
    std::string text = add + std::to_string(data);
    
    std::ofstream file(path, std::ios::app);
    if (file.is_open()) {
        file << text;
        file.close();
        return true;
    } else {
        return false;
    }
}

static bool writeCandidatesToFile(std::string path, llama_token_data_array * candidates, std::string add){
    std::string text = add + "(" + std::to_string(candidates->size) + ")";
    int zeroes = 0;
    for (size_t i = 0; i < candidates->size; ++i) {
        int chance = candidates->data[i].p * 100;
        if (chance > 0 || candidates->size == 1) { 
            text += "\n[" + std::to_string(i) + "] p=" + std::to_string(chance) + "% (l=" + std::to_string(candidates->data[i].logit) + ");"; 
        } else ++zeroes;
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

static bool writeCandidatesToFile2(std::string path, llama_token_data_array * candidates, std::string add){
    std::string text = add + "(" + std::to_string(candidates->size) + "): ";
    int zeroes = 0;
    for (size_t i = 0; i < candidates->size; ++i) {
        int chance = candidates->data[i].p * 100;
        int logit = candidates->data[i].logit;
        if (chance > 0 || candidates->size == 1) { 
            text += " #" + std::to_string(i) +"[" + std::to_string(chance) + "%|" + std::to_string(logit) + "]"; 
        } else ++zeroes;
    }
    //if (zeroes > 0) text += " Zeroes: " + std::to_string(zeroes);
    //text += "\n";
    std::ofstream file(path, std::ios::app);
    if (file.is_open()) {
        file << text;
        file.close();
        return true;
    } else {
        return false;
    }
}

static bool writeCandidatesToFile2vec(std::string path, std::vector<llama_token_data> & candidates, std::string add){
    std::string text = add + "(" + std::to_string(candidates.size()) + "): ";
    int zeroes = 0;
    for (size_t i = 0; i < candidates.size(); ++i) {
        int chance = candidates[i].p * 100;
        int logit = candidates[i].logit;
        if (chance > 0 || candidates.size() == 1) { 
            text += " #" + std::to_string(i) +"[" + std::to_string(chance) + "%|" + std::to_string(logit) + "]"; 
        } else ++zeroes;
    }
    //if (zeroes > 0) text += " Zeroes: " + std::to_string(zeroes);
    //text += "\n";
    std::ofstream file(path, std::ios::app);
    if (file.is_open()) {
        file << text;
        file.close();
        return true;
    } else {
        return false;
    }
}

static std::string getFormattedCandidates(llama_token_data_array * candidates){
    std::string text = "(" + std::to_string(candidates->size) + "): ";
    int zeroes = 0;
    for (size_t i = 0; i < candidates->size; ++i) {
        int chance = candidates->data[i].p * 100;
        int logit = candidates->data[i].logit;
        if (chance > 0 || candidates->size == 1) { 
            text += " #" + std::to_string(i) +"[" + std::to_string(chance) + "%|" + std::to_string(logit) + "]"; 
        } else ++zeroes;
    }
    //if (zeroes > 0) text += "~" + std::to_string(zeroes);

    return text;
}

static std::string getFormattedCandidatesFull(llama_token_data_array * candidates){
    std::string text = "(" + std::to_string(candidates->size) + "): ";
    for (size_t i = 0; i < candidates->size; ++i) {
        int chance = candidates->data[i].p * 100;
        int logit = candidates->data[i].logit;
        if (logit > 0 || candidates->size == 1) { 
            text += " #" + std::to_string(i) +"[" + std::to_string(chance) + "%|" + std::to_string(logit) + "]"; 
        }
    }
    //if (zeroes > 0) text += "~" + std::to_string(zeroes);

    return text;
}

static std::string getFormattedCandidatesVec(std::vector<llama_token_data> & candidates){
    std::string text = "(" + std::to_string(candidates.size()) + "): ";
    int zeroes = 0;
    for (size_t i = 0; i < candidates.size(); ++i) {
        int chance = candidates[i].p * 100;
        int logit = candidates[i].logit;
        if (chance > 0 || candidates.size() == 1) { 
            text += " #" + std::to_string(i) +"[" + std::to_string(chance) + "%|" + std::to_string(logit) + "]"; 
        } else ++zeroes;
    }
    //if (zeroes > 0) text += "~" + std::to_string(zeroes);

    return text;
}

static bool writeCandidatesToFileLogitsOnly(std::string path, llama_token_data_array * candidates, std::string add){
    if (candidates->size <= 1) return false;

    std::string text = add + "(" + std::to_string(candidates->size) + "): ";
    int zeroes = 0;
    for (size_t i = 0; i < candidates->size; ++i) {
        int logit = candidates->data[i].logit;
        text += "[" + std::to_string(logit) + "] "; 
    }
    //if (zeroes > 0) text += " Zeroes: " + std::to_string(zeroes);
    text += "\n";
    std::ofstream file(path, std::ios::app);
    if (file.is_open()) {
        file << text;
        file.close();
        return true;
    } else {
        return false;
    }
}

//-------------------common sampling functions------------------------

static uint32_t get_rng_seed(uint32_t seed) {
    if (seed == LLAMA_DEFAULT_SEED) {
        // use system clock if std::random_device is not a true RNG
        static bool is_rd_prng = std::random_device().entropy() == 0;
        if (is_rd_prng) {
            return (uint32_t) std::chrono::system_clock::now().time_since_epoch().count();
        }
        std::random_device rd;
        return rd();
    }
    return seed;
}

static void llama_sampler_k_token_impl(llama_token_data_array * cur_p, size_t k) {
    cur_p->data += k;
    cur_p->size = 1;
}

static void llama_sampler_shift_only_impl(llama_token_data_array * cur_p, size_t pos_front, int n_remove) {
    size_t pos_end = cur_p->size - n_remove;
    for (size_t i = pos_front; i <= pos_end; ++i) {
        cur_p->data[i] = cur_p->data[i + n_remove];
    }
}


static void llama_sampler_shift_only_impl(llama_token_data_array * cur_p, size_t pos_front, size_t pos_end) {
    int n_remove = cur_p->size - pos_end;
    for (size_t i = pos_front; i <= pos_end; ++i) {
        cur_p->data[i] = cur_p->data[i + n_remove];
    }
}

static void llama_sampler_sort_only_impl(llama_token_data_array * cur_p) {
    GGML_ASSERT(cur_p->size > 0);

    std::sort(cur_p->data, cur_p->data + cur_p->size, [](const llama_token_data & a, const llama_token_data & b) {
        return a.logit > b.logit;
    });
}

static void llama_sampler_calculate_impl(llama_token_data_array * cur_p) {
    GGML_ASSERT(cur_p->size > 0);

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

static void llama_sampler_softmax_impl(llama_token_data_array * cur_p) {
    GGML_ASSERT(cur_p->size > 0);

    // Sort the logits in descending order
    if (!cur_p->sorted) {
        llama_sampler_sort_only_impl(cur_p);
        cur_p->sorted = true;
    }

    llama_sampler_calculate_impl(cur_p);
}

static void llama_sampler_noise_impl(llama_token_data_array * cur_p, size_t range, float randomizationFactor = 1.0f, bool isTrueRNG = true, unsigned int rngSeed = 123456789) {
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
std::string cur_p_disp = "\n" + getFormattedCandidatesFull(cur_p);
    // Apply Gaussian noise to each logit
    for (size_t i = 0; i < range; ++i) {
        // Add Gaussian noise to the logit
        cur_p->data[i].logit += distribution(generator);
    }

    cur_p->sorted = false;

    llama_sampler_softmax_impl(cur_p);

cur_p_disp += "\n" + getFormattedCandidatesFull(cur_p) + "\n";
writeToFile("randomization.txt", cur_p_disp);
    // cur_p->sorted = false;
}

//-------------------LIMIT K------------------------

struct llama_sampler_limit_k {
    const int32_t k;
    bool k_set = false;
};

static const char * llama_sampler_limit_k_name(const struct llama_sampler * /*smpl*/) {
    return "limit-k";
}

static void llama_sampler_limit_k_apply(struct llama_sampler * smpl, llama_token_data_array * cur_p) {
    auto * ctx = (llama_sampler_limit_k *) smpl->ctx;

    // if (ctx->k_set == false) {
        // llama_sampler_softmax_impl(cur_p);
        // llama_sampler_k_token_impl(cur_p, ctx->k);
        // ctx->k_set = true;
    // }

    if (ctx->k_set == false) {
        llama_sampler_softmax_impl(cur_p);
        cur_p->data += ctx->k;
        cur_p->size -= ctx->k;
        llama_sampler_softmax_impl(cur_p);

        std::string conf_disp = "\n";
        int n = 10;
        int idx = 0;
        float top_confidence = 0;
        //std::map<int, float> top_confidence;
        if (n + 1 > cur_p->size) n = cur_p->size;
        for (int i = 1; i <= n; ++i) {
            float conf = cur_p->data[i].p - cur_p->data[i+1].p;
            if (top_confidence < conf) {
                top_confidence = conf;
                idx = i;
            }

            conf_disp += "\n #" + std::to_string(i) + " [" + std::to_string(conf) + "/" + std::to_string(cur_p->data[i].p) + " - " + std::to_string(cur_p->data[i+1].p) + "]";
        }

        cur_p->data[0].p = cur_p->data[idx].p;
        cur_p->size = 1;

        conf_disp += "\n !" + std::to_string(idx) + " [" + std::to_string(top_confidence) + "]";
        writeToFile("k_addon.txt", conf_disp);

        k_total = cur_p->size;

        ctx->k_set = true;
    }
}

static struct llama_sampler * llama_sampler_limit_k_clone(const struct llama_sampler * smpl) {
    auto * ctx = (const llama_sampler_limit_k *) smpl->ctx;

    return llama_sampler_init_limit_k(ctx->k);
}

static void llama_sampler_limit_k_free(struct llama_sampler * smpl) {
    delete (llama_sampler_limit_k *) smpl->ctx;
}

// static void llama_sampler_limit_k_reset(struct llama_sampler * smpl) {
    // auto * ctx = (llama_sampler_limit_k *) smpl->ctx;
    // ctx->k_set = false;
// }

static struct llama_sampler_i llama_sampler_limit_k_i = {
    /* .name   = */ llama_sampler_limit_k_name,
    /* .accept = */ nullptr,
    /* .apply  = */ llama_sampler_limit_k_apply,
    /* .reset  = */ nullptr,
    /* .clone  = */ llama_sampler_limit_k_clone,
    /* .free   = */ llama_sampler_limit_k_free,
};

struct llama_sampler * llama_sampler_init_limit_k(int32_t k) {
    return new llama_sampler {
        /* .iface = */ &llama_sampler_limit_k_i,
        /* .ctx   = */ new llama_sampler_limit_k {
            /* .k = */ k,
        },
    };
}

//-------------------MIN-P WITH NOISE------------------------

struct llama_sampler_min_p_addon {
    const float  p;
    const float  rand;
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
    if (ctx->rand > 0.1f) {
        llama_sampler_softmax_impl(cur_p);
        llama_sampler_noise_impl(cur_p, cur_p->size, ctx->rand);
    }

    // if the cur_p are sorted or the unsorted implementation failed, use this implementation
    if (!min_p_applied) {
        // Sort the logits in descending order
        llama_sampler_sort_only_impl(cur_p);
        cur_p->sorted = true;

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
    min_p_total = cur_p->size;

    if (candidates_max < cur_p->size) candidates_max = cur_p->size;
}

static struct llama_sampler * llama_sampler_min_p_addon_clone(const struct llama_sampler * smpl) {
    const auto * ctx = (const llama_sampler_min_p_addon *) smpl->ctx;
    return llama_sampler_init_min_p_addon(ctx->p, ctx->rand, ctx->min_keep);
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

struct llama_sampler * llama_sampler_init_min_p_addon(float p, float rand, size_t min_keep) {
    return new llama_sampler {
        /* .iface = */ &llama_sampler_min_p_addon_i,
        /* .ctx   = */ new llama_sampler_min_p_addon {
            /* .p        = */ p,
            /* .rand     = */ rand,
            /* .min_keep = */ min_keep,
        },
    };
}

//------------------------Range eXclusion---------------------------------

struct llama_sampler_rx_addon {
    const float max;
    const float min;
    const size_t min_keep;
};

void llama_sample_rx_addon_apply(struct llama_sampler * smpl, llama_token_data_array * cur_p) {
    rx_total += cur_p->size;
    rx_percent = ((float)rx_removed / (float)rx_total) * 100;

    const auto * ctx = (llama_sampler_rx_addon *) smpl->ctx;
    if (ctx->min >= 1.0f || ctx->min >= ctx->max || cur_p->size < 2) {
        return;
    }

    llama_sampler_softmax_impl(cur_p);

    int pos_first = -1;
    int to_remove = 0;

    for (size_t i = 0; i < cur_p->size; ++i) {
        if (cur_p->data[i].p >= ctx->min) {
            if (cur_p->data[i].p > ctx->max) pos_first = i;
            else ++to_remove;
        } else break;
    }

// std::string data = "\n\nINPUT : from " + std::to_string(pos_first) + "; remove " + std::to_string(to_remove) + ". " + getFormattedCandidates(cur_p);

    if (cur_p->size - to_remove < 1) to_remove = cur_p->size - 1;

    if (cur_p->size - to_remove >= ctx->min_keep && to_remove > 0) {

        for (size_t i = pos_first + 1; i <= cur_p->size - (to_remove + 1); ++i) {
            cur_p->data[i] = cur_p->data[i + to_remove];
        }

rx_removed = rx_removed + to_remove;
rx_percent = ((float)rx_removed / (float)rx_total) * 100;

        cur_p->size -= to_remove;
    }

// data += "\nRESULT: " + std::to_string(to_remove) + " to_remove; ";
// writeCandidatesToFile2("rx_addon.txt", cur_p, data);
}

static const char * llama_sampler_rx_addon_name(const struct llama_sampler * /*smpl*/) {
    return "rx";
}

static struct llama_sampler * llama_sampler_rx_addon_clone(const struct llama_sampler * smpl) {
    const auto * ctx = (const llama_sampler_rx_addon *) smpl->ctx;
    return llama_sampler_init_rx_addon(ctx->max, ctx->min, ctx->min_keep);
}

static void llama_sampler_rx_addon_free(struct llama_sampler * smpl) {
    delete (llama_sampler_rx_addon *) smpl->ctx;
}

static struct llama_sampler_i llama_sampler_rx_addon_i = {
    /* .name   = */ llama_sampler_rx_addon_name,
    /* .accept = */ nullptr,
    /* .apply  = */ llama_sample_rx_addon_apply,
    /* .reset  = */ nullptr,
    /* .clone  = */ llama_sampler_rx_addon_clone,
    /* .free   = */ llama_sampler_rx_addon_free,
};

struct llama_sampler * llama_sampler_init_rx_addon(float max, float min, size_t min_keep) {
    return new llama_sampler {
        /* .iface = */ &llama_sampler_rx_addon_i,
        /* .ctx   = */ new llama_sampler_rx_addon {
            /* .front     = */ max,
            /* .end     = */   min,
            /* .min_keep = */  min_keep,
        },
    };
}

//-------------------NOISE SAMPLING--------------------------

struct llama_sampler_noise_addon {
    const float  min;
    const float  max;

    const uint32_t seed;
    uint32_t       seed_cur;

    std::mt19937 rng;
};

static void llama_sampler_noise_addon_apply(struct llama_sampler * smpl, llama_token_data_array * cur_p) {
    auto * ctx = (llama_sampler_noise_addon *) smpl->ctx;

    // Check the range
    if (ctx->min > ctx->max) return;

    // Create a Gaussian distribution
    std::normal_distribution<float> distribution(ctx->min, ctx->max);

// std::string data = "\n\nINPUT : " + getFormattedCandidates(cur_p);

    // Apply Gaussian noise to each logit
    for (size_t i = 0; i < cur_p->size; ++i) {
        cur_p->data[i].logit += distribution(ctx->rng);
    }

// data += "\nRESULT: ";
// writeCandidatesToFile2("noise_addon.txt", cur_p, data);

    if (candidates_max < cur_p->size) candidates_max = cur_p->size;

    cur_p->sorted = false;
}

static const char * llama_sampler_noise_addon_name(const struct llama_sampler * /*smpl*/) {
    return "noise";
}

static struct llama_sampler * llama_sampler_noise_addon_clone(const struct llama_sampler * smpl) {
    const auto * ctx = (const llama_sampler_noise_addon *) smpl->ctx;
    auto * result = llama_sampler_init_noise_addon(ctx->min, ctx->max, ctx->seed);

    // copy the state
    {
        auto * result_ctx = (llama_sampler_noise_addon *) result->ctx;

        result_ctx->rng = ctx->rng;
    }

    return result;
}

static void llama_sampler_noise_addon_free(struct llama_sampler * smpl) {
    delete (llama_sampler_noise_addon *) smpl->ctx;
}

static void llama_sampler_noise_addon_reset(struct llama_sampler * smpl) {
    auto * ctx = (llama_sampler_noise_addon *) smpl->ctx;
    ctx->seed_cur = get_rng_seed(ctx->seed);
    ctx->rng.seed(ctx->seed_cur);
}

static struct llama_sampler_i llama_sampler_noise_addon_i = {
    /* .name   = */ llama_sampler_noise_addon_name,
    /* .accept = */ nullptr,
    /* .apply  = */ llama_sampler_noise_addon_apply,
    /* .reset  = */ llama_sampler_noise_addon_reset,
    /* .clone  = */ llama_sampler_noise_addon_clone,
    /* .free   = */ llama_sampler_noise_addon_free,
};

struct llama_sampler * llama_sampler_init_noise_addon(float min, float max, uint32_t seed) {
    auto seed_cur = get_rng_seed(seed);
    return new llama_sampler {
        /* .iface = */ &llama_sampler_noise_addon_i,
        /* .ctx   = */ new llama_sampler_noise_addon {
            /* .min        = */ min,
            /* .max = */ max,
            /* .seed = */ seed,
            /* .seed_cur = */ seed_cur,
            /* .rng = */ std::mt19937(seed_cur),
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

    const uint32_t seed;
    uint32_t       seed_cur;

    std::mt19937 rng;
};

void llama_sample_xtc_addon_apply(struct llama_sampler * smpl, llama_token_data_array * cur_p) {
    auto * ctx = (llama_sampler_xtc_addon *) smpl->ctx;

    xtc_total += cur_p->size;
    xtc_percent = ((float)xtc_removed / (float)xtc_total) * 100;

    if (ctx->probability <= 0.0f
        || ctx->threshold_max <= 0.0f
        || ctx->threshold_max <= ctx->threshold
        || cur_p->size <= 2) {
        return;
    }

    std::uniform_real_distribution<float> distance(0.0f, 1.0f);
    float chance = distance(ctx->rng);
    if (chance > ctx->probability) return;

    // in case it's not sorted/recalculated yet
    llama_sampler_softmax_impl(cur_p);

    int pos_first = -1;
    int pos_last = 0;
    int to_keep = 1;
    int to_remove = 0;

    if (ctx->threshold > 0.5f && cur_p->data[0].p >= ctx->threshold && cur_p->data[0].p <= ctx->threshold_max) {
        to_keep = 0;
        pos_last = 0;
        to_remove = 1;
    } else if (ctx->threshold <= 0.5f) {
        for (size_t i = 0; i < cur_p->size; ++i) {
            if (cur_p->data[i].p - ctx->threshold >= -1e-5) {
                if (cur_p->data[i].p - ctx->threshold_max > 1e-3) pos_first = i;
                pos_last = i;
            } else break;
        }

        to_remove = pos_last - (to_keep + pos_first);
    }

    if (cur_p->size - to_remove >= ctx->min_keep && to_remove > 0) {

// std::string data = "\n\nINPUT : " + std::to_string(to_remove) + " to_remove; " + std::to_string(pos_first) + " pos_first; " + getFormattedCandidates(cur_p);

        last_candidates = getFormattedCandidates(cur_p) + "\n";

        llama_sampler_shift_only_impl(cur_p, pos_first + 1, to_remove);

xtc_removed = xtc_removed + to_remove;
xtc_percent = ((float)xtc_removed / (float)xtc_total) * 100;

        cur_p->size = cur_p->size - to_remove;

// data += "\nRESULT: " + std::to_string(to_remove) + " to_remove; ";
// writeCandidatesToFile2("xtc_addon.txt", cur_p, data);

last_candidates += getFormattedCandidates(cur_p);
    }
}

static const char * llama_sampler_xtc_addon_name(const struct llama_sampler * /*smpl*/) {
    return "xtc";
}

static struct llama_sampler * llama_sampler_xtc_addon_clone(const struct llama_sampler * smpl) {
    const auto * ctx = (const llama_sampler_xtc_addon *) smpl->ctx;
    auto * result = llama_sampler_init_xtc_addon(ctx->probability, ctx->threshold, ctx->threshold_max, ctx->probability_once, ctx->min, ctx->min_keep, ctx->seed);

    // copy the state
    {
        auto * result_ctx = (llama_sampler_xtc_addon *) result->ctx;

        result_ctx->rng = ctx->rng;
    }

    return result;
}

static void llama_sampler_xtc_addon_free(struct llama_sampler * smpl) {
    delete (llama_sampler_xtc_addon *) smpl->ctx;
}

static void llama_sampler_xtc_addon_reset(struct llama_sampler * smpl) {
    auto * ctx = (llama_sampler_xtc_addon *) smpl->ctx;
    ctx->seed_cur = get_rng_seed(ctx->seed);
    ctx->rng.seed(ctx->seed_cur);
}

static struct llama_sampler_i llama_sampler_xtc_addon_i = {
    /* .name   = */ llama_sampler_xtc_addon_name,
    /* .accept = */ nullptr,
    /* .apply  = */ llama_sample_xtc_addon_apply,
    /* .reset  = */ llama_sampler_xtc_addon_reset,
    /* .clone  = */ llama_sampler_xtc_addon_clone,
    /* .free   = */ llama_sampler_xtc_addon_free,
};

struct llama_sampler * llama_sampler_init_xtc_addon(float probability, float threshold, float threshold_max, bool probability_once, int min, size_t min_keep, uint32_t seed) {
    auto seed_cur = get_rng_seed(seed);
    return new llama_sampler {
        /* .iface = */ &llama_sampler_xtc_addon_i,
        /* .ctx   = */ new llama_sampler_xtc_addon {
            /* .p        = */ probability,
            /* .threshold = */ threshold,
            /* .threshold_max = */ threshold_max,
            /* .probability_once = */ probability_once,
            /* .min = */ min,
            /* .min_keep = */ min_keep,
            /* .seed = */ seed,
            /* .seed_cur = */ seed_cur,
            /* .rng = */ std::mt19937(seed_cur),
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

    llama_sampler_noise_impl(candidates, candidates->size, 1.0f);

    llama_sampler_softmax_impl(candidates);

    bool step_found = false;

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
    p_step_total = candidates->size;
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

        llama_sampler_softmax_impl(cur_p);
        // Apply smoothing if smoothing_factor is > 0. Do not change base implementation otherwise.
        if (smoothing_factor > 0 && cur_p->size > 1) {
            //llama_sampler_softmax_impl(cur_p);
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

        //llama_sampler_softmax_impl(cur_p);

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

    } else {
        for (size_t i = 0; i < cur_p->size; ++i) {
            cur_p->data[i].logit /= ctx->temp;
        }
    }
    temp_total = cur_p->size;
    //writeCandidatesToFile("candidates_after.txt", cur_p, "\nAFTER TEMP:");
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