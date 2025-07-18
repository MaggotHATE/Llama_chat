// Various helper functions and utilities

#pragma once

#include "llama-cpp.h"

#include <string>
#include <vector>
#include <map>

#ifdef _WIN32
#define DIRECTORY_SEPARATOR '\\'
#else
#define DIRECTORY_SEPARATOR '/'
#endif // _WIN32

#define die(msg)          do { fputs("error: " msg "\n", stderr);                exit(1); } while (0)
#define die_fmt(fmt, ...) do { fprintf(stderr, "error: " fmt "\n", __VA_ARGS__); exit(1); } while (0)

#define print_build_info() do {                                                                     \
    fprintf(stderr, "%s: build = %d (%s)\n",      __func__, LLAMA_BUILD_NUMBER, LLAMA_COMMIT);      \
    fprintf(stderr, "%s: built with %s for %s\n", __func__, LLAMA_COMPILER, LLAMA_BUILD_TARGET);    \
} while(0)

#define DEFAULT_MODEL_PATH "models/7B/ggml-model-f16.gguf"

struct common_adapter_lora_info {
    std::string path;
    float scale;

    struct llama_adapter_lora * ptr;
};

using llama_tokens = std::vector<llama_token>;

// build info
extern int LLAMA_BUILD_NUMBER;
extern const char * LLAMA_COMMIT;
extern const char * LLAMA_COMPILER;
extern const char * LLAMA_BUILD_TARGET;

struct common_control_vector_load_info;

//
// CPU utils
//

struct cpu_params {
    int      n_threads                   = -1;
    bool     cpumask[GGML_MAX_N_THREADS] = {false}; // CPU affinity mask.
    bool     mask_valid                  = false;   // Default: any CPU
    enum ggml_sched_priority  priority   = GGML_SCHED_PRIO_NORMAL;  // Scheduling prio : (0 - normal, 1 - medium, 2 - high, 3 - realtime)
    bool     strict_cpu                  = false;   // Use strict CPU placement
    uint32_t poll                        = 50;      // Polling (busywait) level (0 - no polling, 100 - mostly polling)
};

int32_t cpu_get_num_physical_cores();
int32_t cpu_get_num_math();

//
// Common params
//

enum llama_example {
    LLAMA_EXAMPLE_COMMON,
    LLAMA_EXAMPLE_SPECULATIVE,
    LLAMA_EXAMPLE_MAIN,
    LLAMA_EXAMPLE_INFILL,
    LLAMA_EXAMPLE_EMBEDDING,
    LLAMA_EXAMPLE_PERPLEXITY,
    LLAMA_EXAMPLE_RETRIEVAL,
    LLAMA_EXAMPLE_PASSKEY,
    LLAMA_EXAMPLE_IMATRIX,
    LLAMA_EXAMPLE_BENCH,
    LLAMA_EXAMPLE_SERVER,
    LLAMA_EXAMPLE_CVECTOR_GENERATOR,
    LLAMA_EXAMPLE_EXPORT_LORA,
    LLAMA_EXAMPLE_LLAVA,
    LLAMA_EXAMPLE_LOOKUP,
    LLAMA_EXAMPLE_PARALLEL,

    LLAMA_EXAMPLE_COUNT,
};

enum common_sampler_type {
    COMMON_SAMPLER_TYPE_NONE        = 0,
    COMMON_SAMPLER_TYPE_TOP_K       = 1,
    COMMON_SAMPLER_TYPE_TOP_P       = 2,
    COMMON_SAMPLER_TYPE_MIN_P       = 3,
    COMMON_SAMPLER_TYPE_TFS_Z       = 4,
    COMMON_SAMPLER_TYPE_TYPICAL_P   = 5,
    COMMON_SAMPLER_TYPE_TEMPERATURE = 6,
};

// dimensionality reduction methods, used by cvector-generator
enum dimre_method {
    DIMRE_METHOD_PCA,
    DIMRE_METHOD_MEAN,
};

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

// vocoder params
struct common_params_vocoder {
    std::string hf_repo = ""; // HF repo                                                     // NOLINT
    std::string hf_file = ""; // HF file                                                     // NOLINT
    std::string model     = ""; // model path                                                // NOLINT
    std::string model_url = ""; // model url to download                                     // NOLINT
};

struct common_grammar_trigger {
    std::string word;
    bool at_start;
};

// sampling parameters
struct common_params_sampling {
    uint32_t seed = LLAMA_DEFAULT_SEED; // the seed used to initialize llama_sampler

    int32_t n_prev            = 64;    // number of previous tokens to remember
    int32_t n_probs           = 0;     // if greater than 0, output the probabilities of top n_probs tokens.
    int32_t min_keep          = 0;     // 0 = disabled, otherwise samplers should return at least min_keep tokens
    int32_t top_k             = 40;    // <= 0 to use vocab size
    float   top_p             = 1.00f; // 1.0 = disabled
    float   min_p             = 0.05f; // 0.0 = disabled
    float   min_p_rand        = 1.00f; // randomization factor for noise
    float   tfs_z             = 1.00f; // 1.0 = disabled
    float   typical_p         = 1.00f; // typical_p, 1.0 = disabled
    float   p_step            = 0.00f; // 0.0 = disabled
    float   p_step_rand       = 0.00f; // 0.0 = disabled
    float   temp              = 0.80f; // <= 0.0 to sample greedily, 0.0 to not output probabilities
    float   dynatemp_range    = 0.00f; // 0.0 = disabled
    float   dynatemp_exponent = 1.00f; // controls how entropy maps to temperature in dynamic temperature sampler
    float   smoothing_factor  = 0.00f; // 0.0 = disabled
    float   smoothing_curve   = 1.00f; // 1.0 = flat
    bool    temp_adaptive     = false; // enables automatic adaptive setting of temperature
    int32_t penalty_last_n    = 64;    // last n tokens to penalize (0 = disable penalty, -1 = context size)
    float   penalty_repeat    = 1.00f; // 1.0 = disabled
    float   penalty_freq      = 0.00f; // 0.0 = disabled
    float   penalty_present   = 0.00f; // 0.0 = disabled
    float   penalty_threshold = 0.00f; // 0.0 = disabled
    int32_t mirostat          = 0;     // 0 = disabled, 1 = mirostat, 2 = mirostat 2.0
    float   mirostat_tau      = 5.00f; // target entropy
    float   mirostat_eta      = 0.10f; // learning rate
    float   top_n_sigma       = -1;    // -1 = disabled
    llama_sampling_param_func temp_func;
    llama_sampling_param_func dynatemp_range_func;
    llama_sampling_param_func p_step_func;
    float   dry_multiplier     = 0.0f;  // 0.0 = disabled;      DRY repetition penalty for tokens extending repetition:
    float   dry_base           = 1.75f; // 0.0 = disabled;      multiplier * base ^ (length of sequence before token - allowed length)
    int32_t dry_allowed_length = 2;     // tokens extending repetitions beyond this receive penalty
    int32_t dry_penalty_last_n = -1;    // how many tokens to scan for repetitions (0 = disable penalty, -1 = context size)
    float    xtc_probability       = 0.5; // probability of removing a top token
    float    xtc_threshold         = 0.1; // minimum tokens probablitity for this to run
    float    xtc_threshold_max     = 1.0; // maximum tokens probablitity for this to run
    bool     xtc_probability_once  = false; // should we calculate chances one or for each token
    int      xtc_min               = 2; // minimum number of penalizeable tokens
    float    noise_min             = 0.0f; // minimum in randomization range
    float    noise_max             = 1.0f; // maximum in randomization range
    float    range_max             = 1.0; // maximum tokens probablitity in range
    float    range_min             = 1.0; // minimum tokens probablitity in range
    int32_t  k_shift               = 0; // token shift for the first greedy sampling
    float    confidence_shift      = 14.0f; // difference between neighbouring logits

    float    confidence_top            = 0.0f; // desired maximum confidence of the chosen logit
    float    confidence_bottom         = 0.0f; // desired minimum confidence of the chosen logit

    bool    penalize_nl       = false; // consider newlines as a repeatable token
    bool    ignore_eos        = false;
    bool    no_perf           = false; // disable performance metrics

    std::string samplers_sequence     = "kfypmts"; // top_k, tail_free, typical_p, top_p, min_p, temp, p_step

    std::vector<std::string> dry_sequence_breakers = {"\n", ":", "\"", "*"};     // default sequence breakers for DRY

    std::string                         grammar; // optional BNF-like grammar to constrain sampling
    bool                                grammar_lazy = false;
    std::vector<common_grammar_trigger> grammar_trigger_words;  // optional trigger words to trigger lazy grammar
    std::vector<llama_token>            grammar_trigger_tokens; // optional trigger tokens to trigger lazy grammar and print trigger special tokens.

    std::vector<llama_logit_bias> logit_bias; // logit biases to apply
    std::vector<llama_logit_bias> logit_bias_eog; // pre-calculated logit biases for EOG tokens

    std::vector<std::string> logit_bias_strings; // words for logit biases, all matches
    std::vector<std::string> logit_bias_strings_exact; // words for logit biases, exact matches
    std::vector<std::string> logit_bias_strings_beginning; // words for logit biases, beginning of the word matches
    std::vector<std::string> logit_bias_strings_ending; // words for logit biases, ending of the word matches

    std::vector<llama_token> logit_bias_tokens_manual; // tokens for manual restricting
    std::vector<std::string> logit_bias_strings_manual; // words for manual restricting


    std::map<std::string, float> logit_bias_strings_ext; // words for logit biases, but with extra configuration
    std::vector<std::string> logit_bias_strings_start; // restricted beginnings of messages

    // print the parameters into a string
    std::string print() const;
};

struct common_params_speculative {
    int32_t n_ctx        =     0; // draft context size
    int32_t n_max        =    16; // maximum number of tokens to draft during speculative decoding
    int32_t n_min        =     5; // minimum number of draft tokens to use for speculative decoding
    int32_t n_gpu_layers =    -1; // number of layers to store in VRAM for the draft model (-1 - use default)
    float   p_split      =  0.1f; // speculative decoding split probability
    float   p_min        =  0.9f; // minimum speculative decoding probability (greedy)
    struct cpu_params cpuparams;
    struct cpu_params cpuparams_batch;
    std::string model = ""; // draft model for speculative decoding                          // NOLINT
};




struct common_params_diffusion {
    int32_t steps       = 64;     // number of diffusion steps
    float   eps         = 1e-3f;  // epsilon for timesteps
    int32_t algorithm   = 0;      // diffusion algorithm (0=ORIGIN, 1=MASKGIT_PLUS, 2=TOPK_MARGIN, 3=ENTROPY)
    float   alg_temp    = 0.0f;   // algorithm temperature
    bool    visual_mode = false;  // show progressive diffusion on screen
};

struct common_params {
    int32_t n_predict             =    -1; // new tokens to predict
    int32_t n_ctx                 =  2048; // context size
    int32_t n_batch               =  2048; // logical batch size for prompt processing (must be >=32 to use BLAS)
    int32_t n_ubatch              =   512; // physical batch size for prompt processing (must be >=32 to use BLAS)
    int32_t n_keep                =     0; // number of tokens to keep from initial prompt
    int32_t n_draft               =     5; // number of tokens to draft during speculative decoding
    int32_t n_chunks              =    -1; // max number of chunks to process (-1 = unlimited)
    int32_t n_parallel            =     1; // number of parallel sequences to decode
    int32_t n_sequences           =     1; // number of sequences to decode
    float   p_split               =  0.1f; // speculative decoding split probability
    int32_t grp_attn_n            =     1; // group-attention factor
    int32_t grp_attn_w            =   512; // group-attention width
    int32_t n_print               =    -1; // print token count every n tokens (-1 = disabled)
    float   rope_freq_base        =  0.0f; // RoPE base frequency
    float   rope_freq_scale       =  0.0f; // RoPE frequency scaling factor
    float   yarn_ext_factor       = -1.0f; // YaRN extrapolation mix factor
    float   yarn_attn_factor      =  1.0f; // YaRN magnitude scaling factor
    float   yarn_beta_fast        = 32.0f; // YaRN low correction dim
    float   yarn_beta_slow        =  1.0f; // YaRN high correction dim
    int32_t yarn_orig_ctx         =     0; // YaRN original context length
    float   defrag_thold          = -1.0f; // KV cache defragmentation threshold

    int32_t clblast_platform_id = -1;

    // offload params
    std::vector<ggml_backend_dev_t> devices;         // devices to use for offloading
    int32_t n_gpu_layers                    =    -1; // number of layers to store in VRAM (-1 - use default)
    int32_t main_gpu                        =     0; // the GPU that is used for scratch and small tensors
    float   tensor_split[128]               =   {0}; // how split tensors should be distributed across GPUs
    enum llama_split_mode        split_mode = LLAMA_SPLIT_MODE_LAYER; // how to split the model across GPUs

    struct cpu_params cpuparams;
    struct cpu_params cpuparams_batch;
    struct cpu_params draft_cpuparams;
    struct cpu_params draft_cpuparams_batch;

    ggml_backend_sched_eval_callback cb_eval = nullptr;
    void * cb_eval_user_data                 = nullptr;

    ggml_numa_strategy numa = GGML_NUMA_STRATEGY_DISABLED;

    enum llama_rope_scaling_type rope_scaling_type = LLAMA_ROPE_SCALING_TYPE_UNSPECIFIED;
    enum llama_pooling_type      pooling_type      = LLAMA_POOLING_TYPE_UNSPECIFIED; // pooling type for embeddings
    enum llama_attention_type    attention_type    = LLAMA_ATTENTION_TYPE_UNSPECIFIED; // attention type for embeddings

    struct common_params_sampling    sparams;
    struct common_params_vocoder     vocoder;
    struct common_params_speculative speculative;

    std::string model                = ""; // model path                                                    // NOLINT
    std::string model_draft          = ""; // draft model for speculative decoding                          // NOLINT
    std::string model_alias          = "unknown"; // model alias                                            // NOLINT
    std::string model_url            = ""; // model url to download                                         // NOLINT
    std::string hf_token             = ""; // HF token                                                      // NOLINT
    std::string hf_repo              = ""; // HF repo                                                       // NOLINT
    std::string hf_file              = ""; // HF file                                                       // NOLINT
    std::string prompt               = "";                                                                  // NOLINT
    std::string prompt_file          = ""; // store the external prompt file name                           // NOLINT
    std::string path_prompt_cache    = ""; // path to file for saving/loading prompt eval state             // NOLINT
    std::string input_prefix         = ""; // string to prefix user inputs with                             // NOLINT
    std::string input_suffix         = ""; // string to suffix user inputs with                             // NOLINT
    std::string logdir               = ""; // directory in which to save YAML log files                     // NOLINT
    std::string lookup_cache_static  = ""; // path of static ngram cache file for lookup decoding           // NOLINT
    std::string lookup_cache_dynamic = ""; // path of dynamic ngram cache file for lookup decoding          // NOLINT
    std::string logits_file          = ""; // file for saving *all* logits                                  // NOLINT

    std::string format_instruct   = "bi";  // format for the first prompt
    std::string format_dialog     = "pis"; // format for interacive
    std::string bos               = "";
    std::string eos               = "";

    std::vector<std::string> in_files;   // all input files
    std::vector<std::string> antiprompt; // strings upon which more user input is prompted (a.k.a. reverse prompts)
    std::vector<llama_model_kv_override> kv_overrides;
    std::string kv_overrides_pair;
    std::vector<llama_model_tensor_buft_override> tensor_buft_overrides;
    // to simplify the process
    std::map<std::string, std::string> tensor_override_pairs;

    bool lora_init_without_apply = false; // only load lora to memory, but do not apply it to ctx (user can manually apply lora later using llama_lora_adapter_apply)
    std::vector<common_adapter_lora_info> lora_adapters; // lora adapter path with user defined scale

    std::vector<common_control_vector_load_info> control_vectors; // control vector with user defined scale

    int32_t verbosity                  = 0;
    int32_t control_vector_layer_start = -1; // layer range for control vector
    int32_t control_vector_layer_end   = -1; // layer range for control vector

    int32_t ppl_stride      = 0;     // stride for perplexity calculations. If left at 0, the pre-existing approach will be used.
    int32_t ppl_output_type = 0;     // = 0 -> ppl output is as usual, = 1 -> ppl output is num_tokens, ppl, one per line
                                     //                                       (which is more convenient to use for plotting)
                                     //
    bool   hellaswag        = false; // compute HellaSwag score over random tasks from datafile supplied in prompt
    size_t hellaswag_tasks  = 400;   // number of tasks to use when computing the HellaSwag score

    bool   winogrande       = false; // compute Winogrande score over random tasks from datafile supplied in prompt
    size_t winogrande_tasks = 0;     // number of tasks to use when computing the Winogrande score. If 0, all tasks will be computed

    bool   multiple_choice  = false;  // compute TruthfulQA score over random tasks from datafile supplied in prompt
    size_t multiple_choice_tasks = 0; // number of tasks to use when computing the TruthfulQA score. If 0, all tasks will be computed

    bool   kl_divergence    = false; // compute KL divergence

    bool usage             = false; // print usage
    bool use_color         = false; // use color to distinguish generations and inputs
    bool special           = false; // enable special token output
    bool interactive       = false; // interactive mode
    bool interactive_first = false; // wait for user input immediately
    bool conversation      = false; // conversation mode (does not print special tokens and suffix/prefix)
    bool prompt_cache_all  = false; // save user input and generations to prompt cache
    bool prompt_cache_ro   = false; // open the prompt cache read-only and do not update it

    bool escape            = true;  // escape "\n", "\r", "\t", "\'", "\"", and "\\"
    bool multiline_input   = false; // reverse the usage of `\`
    bool simple_io         = false; // improves compatibility with subprocesses and limited consoles
    bool cont_batching     = true;  // insert new sequences for decoding on-the-fly
    bool flash_attn        = false; // flash attention
    bool no_perf           = false; // disable performance metrics
    bool swa_full          = false; // use full-size SWA cache (https://github.com/ggml-org/llama.cpp/pull/13194#issuecomment-2868343055)
    bool kv_unified        = false; // enable unified KV cache
    bool input_prefix_bos  = false; // prefix BOS to user inputs, preceding input_prefix
    bool logits_all        = false; // return logits for all tokens in the batch
    bool use_mmap          = true;  // use mmap for faster loads
    bool use_mlock         = false; // use mlock to keep model in memory
    bool verbose_prompt    = false; // print prompt tokens before generation
    bool display_prompt    = true;  // print prompt before generation
    bool infill            = false; // use infill mode
    // bool dump_kv_cache     = false; // dump the KV cache contents for debugging purposes
    bool no_kv_offload     = false; // disable KV offloading
    bool warmup            = true;  // warmup run
    bool check_tensors     = false; // validate tensor data

    std::string cache_type_k = "f16"; // KV cache data type for the K
    std::string cache_type_v = "f16"; // KV cache data type for the V

    // multimodal models (see examples/llava)
    std::string mmproj = "";        // path to multimodal projector                                         // NOLINT
    std::vector<std::string> image; // path to image file(s)

    // embedding
    bool embedding         = false; // get only sentence embedding
    int32_t embd_normalize = 2;     // normalisation for embendings (-1=none, 0=max absolute int16, 1=taxicab, 2=euclidean, >2=p-norm)
    std::string embd_out   = "";    // empty = default, "array" = [[],[]...], "json" = openai style, "json+" = same "json" + cosine similarity matrix
    std::string embd_sep   = "\n";  // separator of embendings

    // server params
    int32_t port           = 8080;         // server listens on this network port
    int32_t timeout_read   = 600;          // http read timeout in seconds
    int32_t timeout_write  = timeout_read; // http write timeout in seconds
    int32_t n_threads_http = -1;           // number of threads to process HTTP requests (TODO: support threadpool)
    int32_t n_cache_reuse  = 0;            // min chunk size to reuse from the cache via KV shifting

    std::string hostname      = "127.0.0.1";
    std::string public_path   = "";                                                                         // NOLINT
    std::string chat_template = "";                                                                         // NOLINT
    std::string system_prompt = "";                                                                         // NOLINT
    bool enable_chat_template = true;

    std::vector<std::string> api_keys;

    std::string ssl_file_key  = "";                                                                         // NOLINT
    std::string ssl_file_cert = "";                                                                         // NOLINT

    // "advanced" endpoints are disabled by default for better security
    bool webui            = true;
    bool endpoint_slots   = false;
    bool endpoint_props   = false; // only control POST requests, not GET
    bool endpoint_metrics = false;

    bool log_json = false;

    std::string slot_save_path;

    float slot_prompt_similarity = 0.5f;

    // batched-bench params
    bool is_pp_shared = false;

    std::vector<int32_t> n_pp;
    std::vector<int32_t> n_tg;
    std::vector<int32_t> n_pl;

    // retrieval params
    std::vector<std::string> context_files; // context files to embed

    int32_t chunk_size = 64; // chunk size for context embedding

    std::string chunk_separator = "\n"; // chunk separator for context embedding

    // passkey params
    int32_t n_junk = 250; // number of times to repeat the junk text
    int32_t i_pos  = -1;  // position of the passkey in the junk text

    // imatrix params
    std::string out_file = "imatrix.dat"; // save the resulting imatrix to this file

    int32_t n_out_freq  = 10; // output the imatrix every n_out_freq iterations
    int32_t n_save_freq =  0; // save the imatrix every n_save_freq iterations
    int32_t i_chunk     =  0; // start processing from this chunk

    bool process_output = false; // collect data for the output tensor
    bool compute_ppl    = true;  // whether to compute perplexity

    // cvector-generator params
    int n_pca_batch = 100;
    int n_pca_iterations = 1000;
    dimre_method cvector_dimre_method = DIMRE_METHOD_PCA;
    std::string cvector_outfile       = "control_vector.gguf";
    std::string cvector_positive_file = "examples/cvector-generator/positive.txt";
    std::string cvector_negative_file = "examples/cvector-generator/negative.txt";

    bool spm_infill = false; // suffix/prefix/middle pattern for infill

    std::string lora_outfile = "ggml-lora-merged-f16.gguf";

    // batched-bench params
    bool batched_bench_output_jsonl = false;

    bool ctx_shift = true;

    // optional callback for model loading progress and cancellation:
    // called with a progress value between 0.0 and 1.0.
    // return false from callback to abort model loading or true to continue
    llama_progress_callback load_progress_callback = NULL;
    void *                  load_progress_callback_user_data = NULL;
};

std::string common_params_get_system_info(const common_params & params);

bool parse_cpu_range(const std::string& range, bool(&boolmask)[GGML_MAX_N_THREADS]);
bool parse_cpu_mask(const std::string& mask, bool(&boolmask)[GGML_MAX_N_THREADS]);
void postprocess_cpu_params(cpu_params& cpuparams, const cpu_params* role_model = nullptr);
bool set_process_priority(enum ggml_sched_priority prio);

//
// String utils
//

std::string string_strip(const std::string & str);
std::string string_get_sortable_timestamp();

std::string string_join(const std::vector<std::string> & values, const std::string & separator);
std::vector<std::string> string_split(const std::string & str, const std::string & delimiter);
std::string string_repeat(const std::string & str, size_t n);

void string_replace_all(std::string & s, const std::string & search, const std::string & replace);

static bool string_starts_with(const std::string & str,
                               const std::string & prefix) {  // While we wait for C++20's std::string::starts_with...
    return str.rfind(prefix, 0) == 0;
}

bool string_parse_kv_override(const char * data, std::vector<llama_model_kv_override> & overrides);
void string_process_escapes(std::string & input);

//
// Filesystem utils
//

bool fs_validate_filename(const std::string & filename);
bool fs_create_directory_with_parents(const std::string & path);

std::string fs_get_cache_directory();
std::string fs_get_cache_file(const std::string & filename);

//
// Model utils
//

// note: defines object's lifetime
struct common_init_result {
    llama_model_ptr   model;
    llama_context_ptr context;

    std::vector<llama_adapter_lora_ptr> lora;
};

struct common_init_result     common_init_from_params(common_params & params);

struct llama_model_params     common_model_params_to_llama    (common_params & params);
struct llama_context_params   common_context_params_to_llama  (const common_params & params);
struct ggml_threadpool_params ggml_threadpool_params_from_cpu_params(const cpu_params & params);

struct llama_model * common_load_model_from_url(const char * model_url, const char * path_model, const char * hf_token, const struct llama_model_params & params);
struct llama_model * common_load_model_from_hf(const char * repo, const char * file, const char * path_model, const char * hf_token, const struct llama_model_params & params);

// clear LoRA adapters from context, then apply new list of adapters
void common_set_adapter_lora(struct llama_context * ctx, std::vector<common_adapter_lora_info> & lora);

// copied from arg
void common_process_override_tensors(common_params & params, const std::map<std::string, std::string> & values);
void common_process_override_tensors(common_params & params);

// Batch utils

void common_batch_clear(struct llama_batch & batch);

void common_batch_add(
                 struct llama_batch & batch,
                        llama_token   id,
                          llama_pos   pos,
    const std::vector<llama_seq_id> & seq_ids,
                               bool   logits);

//
// Token utils
//
// longest common prefix
size_t common_lcp(const llama_tokens & a, const llama_tokens & b);
// longet common subsequence
size_t common_lcs(const llama_tokens & a, const llama_tokens & b);

//
// Vocab utils
//

// tokenizes a string into a vector of tokens
// should work similar to Python's `tokenizer.encode`
std::vector<llama_token> common_tokenize(
  const struct llama_context * ctx,
           const std::string & text,
                        bool   add_special,
                        bool   parse_special = false);

std::vector<llama_token> common_tokenize(
    const struct llama_vocab * vocab,
           const std::string & text,
                        bool   add_special,
                        bool   parse_special = false);

// tokenizes a token into a piece, optionally renders special/control tokens
// should work similar to Python's `tokenizer.id_to_piece`
std::string common_token_to_piece(
        const struct llama_context * ctx,
                       llama_token   token,
                       bool          special = true);

std::string common_token_to_piece(
          const struct llama_vocab * vocab,
                       llama_token   token,
                       bool          special = true);

// detokenizes a vector of tokens into a string
// should work similar to Python's `tokenizer.decode`
// optionally renders special/control tokens
std::string common_detokenize(
           const struct  llama_context * ctx,
        const std::vector<llama_token> & tokens,
                                  bool   special = true);

std::string common_detokenize(
              const struct llama_vocab * vocab,
        const std::vector<llama_token> & tokens,
                                  bool   special = true);

//
// Chat template utils
//

// same with llama_chat_message, but uses std::string
struct common_chat_msg {
    std::string role;
    std::string content;
};

// Get the built-in chat template for the model.
// Removed since they are unsued now
// Since there were no chages to this part, you can simply copy it from llama.cpp

//
// KV cache utils
//

// removed

//
// Embedding utils
//

void common_embd_normalize(const float * inp, float * out, int n, int embd_norm = 2);

float common_embd_similarity_cos(const float * embd1, const float * embd2, int n);

//
// Control vector utils
//

struct common_control_vector_data {
    int n_embd;

    // stores data for layers [1, n_layer] where n_layer = data.size() / n_embd
    std::vector<float> data;
};

struct common_control_vector_load_info {
    float strength;

    std::string fname;
};

// Load control vectors, scale each by strength, and add them together.
// On error, returns {-1, empty}
common_control_vector_data common_control_vector_load(const std::vector<common_control_vector_load_info> & load_infos);

//
// Split utils
//

namespace {
const char * const LLM_KV_SPLIT_NO            = "split.no";
const char * const LLM_KV_SPLIT_COUNT         = "split.count";
const char * const LLM_KV_SPLIT_TENSORS_COUNT = "split.tensors.count";
}

//
// YAML utils
//

void yaml_dump_vector_float    (FILE * stream, const char * prop_name, const std::vector<float> & data);
void yaml_dump_vector_int      (FILE * stream, const char * prop_name, const std::vector<int> & data);
void yaml_dump_string_multiline(FILE * stream, const char * prop_name, const char * data);
