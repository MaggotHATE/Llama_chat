// Defines sigaction on msys:
// #ifndef _GNU_SOURCE
// #define _GNU_SOURCE
// #endif

/* #ifdef GGML_USE_CLBLAST
#   include "base_cl/common.h"
#   include "base_cl/llama.h"
//#   include "base_cl/build-info.h"
#   include "base_cl/grammar-parser.h"
#else
#   include "base/common.h"
#   include "base/llama.h"
//#   include "base/build-info.h"
#   include "base/grammar-parser.h"
#endif */

#pragma once

//#include "common.h"
//#include "sampling_plus.h"
//#include "llama.h"

#include "jsonParams.h"

#include <cassert>
#include <cinttypes>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <algorithm>


#if defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
#include <signal.h>
#include <unistd.h>
//#define DELIMINER '\r'
#elif defined (_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
//#define DELIMINER '\n'
#include <windows.h>
#include <signal.h>
#endif

#ifdef GGML_USE_CLBLAST
    //#include "ggml-opencl.h"
    extern int GGML_OPENCL_DEFAULT_PLATFORM_ID;
    extern std::string GGML_OPENCL_RESULT_DEVICE_NAME;
#endif

extern int temp_total;
extern int min_p_total;
extern int candidates_max;
extern int candidates_max_min_p;
extern int p_step_total;
extern int xtc_total;
extern int xtc_removed;
extern float xtc_percent;

extern int rx_total;
extern int rx_removed;
extern float rx_percent;

extern int k_total;

extern bool k_set;

extern int num_probs_tops;

extern int num_probs_bottoms;

extern float confidence_total;

#define SESSIONS_FOLDER "sessions/"

static common_params paramsDefault;

static bool is_interacting = false;

static void log_down(std::string text, uint32_t seed) {
    writeTextFile(std::to_string(seed) + ".log", text);
}

#ifdef GGML_USE_BLAS
        std::string backend_name = "blas";
        std::string backend_name_short = "b";
#elif defined GGML_USE_CLBLAST
        std::string backend_name = "Clblast";
        std::string backend_name_short = "c";
#elif defined GGML_USE_VULKAN
        std::string backend_name = "Vulkan";
        std::string backend_name_short = "v";
#else
        std::string backend_name = "CPU";
        std::string backend_name_short = "u";
#endif

//STRUCT/////////////////////////////////////////

/* struct modelData{
    llama_context * ctx;
    llama_model * model;
    
    int n_ctx;
    
    size_t n_matching_session_tokens = 0;
    
    llama_context * ctx_guidance = NULL;
    std::vector<llama_token> session_tokens;
    std::vector<llama_token> embd;
    std::vector<llama_token> embd_inp;
    std::vector<llama_token> last_tokens;
    std::vector<llama_token> guidance_inp;
    std::vector<llama_token> embd_guidance;
    std::vector<llama_token_data> candidates;
    
    grammar_parser::parse_state parsed_grammar;
    struct llama_grammar * grammar = NULL;
    
    int n_vocab;
    int n_past                = 0;
    int n_remain              = 0;
    int n_consumed            = 0;
    int n_session_consumed    = 0;
    int n_past_guidance       = 0;
    int guidance_offset       = 0;
    int original_prompt_len   = 0;
    int last_tokens_count     = 0;

    std::vector<int> inp_pfx;
    std::vector<int> inp_sfx;
       
    std::vector<int> llama_token_newline;
    
    std::vector<int>   input_tokens;
    std::vector<int>   output_tokens;
}; */
/* 
typedef struct format{
    //position, definition
    std::string bos;
    std::string eos;
    // i for input
    // o for output
    // p for prefix
    // s for suffix
    // b for bos
    // e for eos
    // d for \n or \r
    std::string sequence;
}; 
 */
 
typedef struct chat_state {
    int kv_cache_pos   = 0;
    int embd_inp_size   = 0;
    int n_past_size     = 0;
    int n_consumed_size = 0;

    void capture_kv_cache(int value) {
        kv_cache_pos = value;
    }

    void capture_embd_inp(int value) {
        embd_inp_size = value;
    }

    void capture_n_past(int value) {
        n_past_size = value;
    }

    void capture_n_consumed(int value) {
        n_consumed_size = value;
    }

    void clear() {
        kv_cache_pos = 0;
        embd_inp_size = 0;
        n_past_size = 0;
        n_consumed_size = 0;
    }
};

//CLASS////////////////////////////////////////////////////////////////////////////////////////////

class chat
{
private:
	
    llama_context * ctx = nullptr;
    llama_model * model = nullptr;
    common_sampler * smpl = nullptr;
    const llama_vocab * vocab = nullptr;
    // common_init_result llama_init;


    int n_ctx;
    std::vector<llama_token> session_tokens;
    size_t n_matching_session_tokens = 0;
    std::vector<llama_token> embd_inp;
    std::string path_session = "";
    //std::vector<llama_token> last_n_tokens;
    std::vector<llama_token> last_tokens;

    //grammar_parser::parse_state parsed_grammar;
    //struct llama_grammar * grammar = NULL;
    std::vector<llama_token_data> candidates;
    //llama_sampling_context ctx_sampling;
    //llama_sampling_params & sparams;
    std::vector<std::vector<llama_token>> antiprompt_ids;
    int n_vocab;
    bool cleared              = true;
        
    
    bool input_echo           = true;
    bool need_to_save_session = false;
    bool debug                = false;

    bool streaming            = true;
    bool saveToFile           = false;
    
    
    

    int n_past                = 0;
    int n_remain              = 0;
    int n_consumed            = 0;
    int n_session_consumed    = 0;
    int original_prompt_len   = 0;
    int t_eval_ms             = 0;
    int n_eval                = 0;
    int t_p_eval_ms           = 0;
    int n_p_eval              = 0;
    // since params.n_keep changes
    int n_keep                = 0;

    int ga_i = 0;
    int ga_n = 1;
    int ga_w;

    std::vector<llama_token> embd;
    // std::vector<llama_token> embd_msg;

    std::vector<int> inp_pfx;
    std::vector<int> inp_sfx;
       
    std::vector<int> llama_token_newline;

    std::vector<int>   input_tokens;
    std::vector<int>   output_tokens;
    std::ostringstream output_ss; 
    
    struct ggml_threadpool * threadpool;
    struct ggml_threadpool * threadpool_batch = NULL;

public:
    common_params params;
    bool is_antiprompt   = false;
    bool need_antiprompt = false;
    bool tempFirst       = true;
    bool finished        = true;
    bool add_bos         = false;
    bool add_eos         = false;

    int n_past_last     = 0;
    int n_remain_last   = 0;
    int n_consumed_last = 0;
    int n_embd_inp_last = 0;
    int c_empty_msgs    = 0;

    // experimenting with simple dynamic paramenters
    float d_temp_min = 1.8;
    float d_temp_max = 5;
    float d_temp_add = 0.1f;
    float d_temp_mul = 1.0f;
    bool d_temp_up = true;

    chat_state rewind_state;
    std::string has_antiprompt = "";

    std::string formatRepresentation = "";
    std::string txt_vocab_bos = "";
    std::string txt_vocab_eos = "";

    struct llama_perf_context_data ctx_performance_data;

    //std::map<std::string,std::string> stats;

    ring_buffer<llama_token> prev_state = ring_buffer<llama_token>(std::max(32, params.sparams.n_prev));

    chat(int argc, char ** argv){
        init(argc, argv);
    }
    
    chat(){}
    
    chat(bool useJson){
        init_default(useJson);
    }

    std::string get_state_descr() {
        return std::format("SMPL = {}; kv_cache_pos = {}; embd_inp_size = {}; n_past_size = {}; n_consumed_size = {}", get_buffer_data(), rewind_state.kv_cache_pos, rewind_state.embd_inp_size, rewind_state.n_past_size, rewind_state.n_consumed_size);
    }

    void capture_smpl() {
        prev_state = llama_sampling_get_prev(smpl);
    }

    void restore_smpl() {
        //smpl_size = value;
        llama_sampling_set_prev(prev_state, smpl);
    }

    void dynamic_params(float & parameter, float & p_min, float & p_max, float & p_add, float & p_mul, bool & p_dir) {
        if (p_min != -1.0f && p_max != -1.0f) {
            if (p_max < parameter) {
                p_dir = false;
            } else if (p_min > parameter) {
                p_dir = true;
            }

            if (p_dir) parameter = parameter * p_mul + p_add;
            else parameter = parameter / p_mul - p_add;
        }
    }

    void dynamic_params(float & parameter, llama_sampling_param_func & parameter_func) {
        if (parameter_func.p_min != -1.0f && parameter_func.p_max != -1.0f) {
            if (parameter_func.p_max < parameter) {
                parameter_func.p_dir = false;
            } else if (parameter_func.p_min > parameter) {
                parameter_func.p_dir = true;
            }

            if (parameter_func.p_dir) parameter = parameter * parameter_func.p_mul + parameter_func.p_add;
            else parameter = parameter / parameter_func.p_mul - parameter_func.p_add;
        }
    }

    void dynamicParamsPrepare() {
        dynamic_params(params.sparams.temp, params.sparams.temp_func);
        dynamic_params(params.sparams.dynatemp_range, params.sparams.dynatemp_range_func);
        dynamic_params(params.sparams.p_step, params.sparams.p_step_func);
    }

	// ~chat(){
        // if (params.model != "empty" ) {
            
            // if (!path_session.empty() && params.prompt_cache_all && !params.prompt_cache_ro) {
                // fprintf(stderr, "\n%s: saving final output to session file '%s'\n", __func__, path_session.c_str());
                // llama_save_session_file(ctx, path_session.c_str(), session_tokens.data(), session_tokens.size());
            // }
            
            // //llama_print_timings(ctx);
            
            
        // }
        
        // if (embd.size()) clear();
	// }

    void clearSoft(){
        session_tokens.clear();
        n_matching_session_tokens = 0;
        embd_inp.clear();
        //last_n_tokens.clear();
        last_tokens.clear();
        embd.clear();
        inp_pfx.clear();
        inp_sfx.clear();
        llama_token_newline.clear();
        candidates.clear();
        formatRepresentation = "";
        //resetCTX();

        n_past                = 0;
        n_remain              = 0;
        n_consumed            = 0;
        n_session_consumed    = 0;
        original_prompt_len   = 0;
        t_eval_ms             = 0;
        n_eval                = 0;
        t_p_eval_ms           = 0;
        n_p_eval              = 0;
        n_keep                = 0;
        n_past_last           = 0;
        n_remain_last         = 0;
        c_empty_msgs          = 0;

        xtc_total = 0;
        xtc_removed = 0;
        xtc_percent = 0;

        params = paramsDefault;

        rewind_state.clear();
    }

    void clear(){
        if (!cleared){
            cleared = true;

            params.n_keep = n_keep;
            common_sampler_free(smpl);
            llama_perf_context_reset(ctx);
            llama_free(ctx);
            llama_model_free(model);
            // llama_init = {};
            //if (grammar != NULL) {
            //    llama_grammar_free(grammar);
            //}
            llama_backend_free();
            ggml_threadpool_free(threadpool);
            ggml_threadpool_free(threadpool_batch);

            clearSoft();
        }
        
    }

    void switch_debug() {
        debug = !debug;
    }

    // std::string getBOS() {
        // return (std::string) llama_token_get_text(vocab, llama_vocab_bos(vocab));
    // }

    // std::string getEOS() {
        // return (std::string) llama_token_get_text(vocab, llama_vocab_eos(vocab));
    // }

    int getRemainTokens() {
        return n_remain;
    }

    int getConsumedTokens(){
        return n_consumed;
    }

    int getPastTokens(){
        return n_past;
    }

    int getEmbSize() {
        return std::size(embd);
    }

    int getEmbInpSize() {
        return std::size(embd_inp);
    }

    void perf_update() {
        ctx_performance_data = llama_perf_context(ctx); 
    }

    std::string get_buffer_data() {
        return std::format("Size: {}; Pos: {}; Capacity: {}; First: {}",(int)prev_state.sz, (int)prev_state.pos, (int)prev_state.capacity, (int)prev_state.first);
    }

    float get_speed() {
        // const llama_timings timings = llama_get_timings(ctx);

        // int t_eval_ms_curr = timings.t_eval_ms - t_eval_ms;
        // int n_eval_curr = timings.n_eval - n_eval;

        // t_eval_ms = timings.t_eval_ms;
        // n_eval = timings.n_eval;

        // if (t_eval_ms_curr > 0 && n_eval_curr > 0) {
            // return (1e3 / t_eval_ms_curr * n_eval_curr);
        // }

        // return (1e3 / t_eval_ms * n_eval);
        float speed_ts = 0;
        if (ctx_performance_data.t_eval_ms > 0 && ctx_performance_data.n_eval > 0)
            speed_ts = (1e3 / ctx_performance_data.t_eval_ms * ctx_performance_data.n_eval);

        return speed_ts;
    }

    float get_speed_p(){
        // const llama_timings timings = llama_get_timings(ctx);
        
        // int t_p_eval_ms_curr = timings.t_p_eval_ms - t_p_eval_ms;
        // int n_p_eval_curr = timings.n_p_eval - n_p_eval;

        // t_p_eval_ms = timings.t_p_eval_ms;
        // n_p_eval = timings.n_p_eval;

        // if (t_p_eval_ms_curr > 0 && n_p_eval_curr > 0) {
            // return (1e3 / t_p_eval_ms_curr * n_p_eval_curr);
        // }

        // return (1e3 / t_p_eval_ms * n_p_eval);
        float speed_pp = 0;
        if (ctx_performance_data.t_p_eval_ms > 0 && ctx_performance_data.n_p_eval > 0)
            speed_pp = (1e3 / ctx_performance_data.t_p_eval_ms * ctx_performance_data.n_p_eval);

        return speed_pp;
    }

    void clear_speed(){
        t_eval_ms = 0;
        n_eval = 0;
    }

    // unsafe to run in threads;
    int getArgs(int argc, char ** argv){
        for (int i = 1; i < argc; i++) {
            std::string arg = argv[i];
            std::string blank = "-i";
            if (arg == "-sv" || arg == "--save") {
                saveToFile = true;
                argv[i] = const_cast<char*>(blank.c_str());
            }
            if (arg == "-d" || arg == "--debug") {
                debug = true;
                argv[i] = const_cast<char*>(blank.c_str());
            }
            if (arg == "-strm" || arg == "--streaming") {
                streaming = false;
                argv[i] = const_cast<char*>(blank.c_str());
            }
        }

        // if (gpt_params_parse(argc, argv, params) == false) {
            // return -1;
        // }
    }
    
    std::string getSparams() {
        return "\n top_k = " + std::to_string(params.sparams.top_k) + "\n top_p = " + std::to_string(params.sparams.top_p) + "\n min_p = " + std::to_string(params.sparams.min_p) + "\n tfs_z = " + std::to_string(params.sparams.tfs_z) + "\n typical_p = " + std::to_string(params.sparams.typical_p) + "\n temp = " + std::to_string(params.sparams.temp) + "\n penalty_repeat = " + std::to_string(params.sparams.penalty_repeat) + "\n penalty_freq = " + std::to_string(params.sparams.penalty_freq) + "\n penalty_present = " + std::to_string(params.sparams.penalty_present) + "\n mirostat = " + std::to_string(params.sparams.mirostat) + "\n mirostat_tau = " + std::to_string(params.sparams.mirostat_tau) + "\n mirostat_eta = " + std::to_string(params.sparams.mirostat_eta);
    }
    
    std::string getSparamsChanged(bool fullnames = true) {

        std::string result = fullnames ? backend_name + "->logits" : backend_name_short + "@";

        std::string name_top_k = fullnames ? "top_k" : "K";
        std::string name_rx = fullnames ? "range" : "R";

        //if (params.sparams.temp <= 0) {
            // if (params.sparams.n_probs > 0) {
                // result += std::format("-> {}={}", name_top_k, params.sparams.n_probs);
            // }
            // if (params.sparams.range_min < 1.0f && params.sparams.range_max == 1.0f) {
                // result += std::format("-> {}={:.2f}-{:.2f}:{}/{}({:.2f}%)",name_rx, params.sparams.range_min, params.sparams.range_max, rx_removed, rx_total, rx_percent);
            // }
            // if (params.sparams.range_min < 1.0f) {
                // result += std::format("-> k={}/{}", params.sparams.k_limit, k_total);
                // if (k_set) result += " set!";
            // }
            // if (params.sparams.noise_min > 0.0f) {
                // result += std::format("-> noise={:.2f}-{:.2f}", params.sparams.noise_min, params.sparams.noise_max);
            // }

        //    result += std::format("-> {:.2f} greedy",params.sparams.temp);
        //} else {
            std::string name_penalty_repeat = fullnames ? "penalty_repeat" : "p_r";
            std::string name_penalty_threshold = fullnames ? "penalty_threshold" : "p_t";
            std::string name_penalty_freq = fullnames ? "penalty_freq" : "p_f";
            std::string name_penalty_present = fullnames ? "penalty_present" : "p_p";
            //DRY
            std::string name_dry = fullnames ? "DRY" : "D";

            std::string name_temp = fullnames ? "temp" : "T";
            std::string name_dynatemp_range = fullnames ? "dynatemp_range" : "dT";

            std::string name_mirostat = fullnames ? "mirostat" : "I";
            std::string name_mirostat_tau = fullnames ? "mirostat_tau" : "I_t";
            std::string name_mirostat_eta = fullnames ? "mirostat_eta" : "I_e";

            std::string name_top_n_sigma = fullnames ? "top_n_sigma" : "tns";

            std::string name_tfs_z = fullnames ? "tfs_z" : "F";
            std::string name_typical_p = fullnames ? "typical_p" : "Y";
            std::string name_p_step = fullnames ? "p_step" : "S";
            std::string name_xtc_probability = fullnames ? "xtc_probability" : "x_p";
            std::string name_xtc_threshold = fullnames ? "xtc_threshold" : "x_t";
            std::string name_top_p = fullnames ? "top_p" : "P";
            std::string name_min_p = fullnames ? "min_p" : "M";
            std::string name_noise = fullnames ? "noise" : "O";
            std::string name_k_shift = fullnames ? "k_shift" : "k_s";

            // mirostat is special 
            if (params.sparams.mirostat != paramsDefault.sparams.mirostat) {
                if (params.sparams.dynatemp_range > 0) {
                    result += std::format("->{}({:.2f}-{:.2f})",name_dynatemp_range,params.sparams.temp > params.sparams.dynatemp_range ? params.sparams.temp - params.sparams.dynatemp_range : 0, params.sparams.temp + params.sparams.dynatemp_range);
                    if (params.sparams.smoothing_factor != paramsDefault.sparams.smoothing_factor) result += std::format("/{:.2f}*{:.2f}", params.sparams.smoothing_factor, params.sparams.smoothing_curve);
                } else {
                    result += name_temp; 
                    if (params.sparams.temp != paramsDefault.sparams.temp) result += std::format("={:.2f}",params.sparams.temp); 
                    result += std::format("/{:.2f}*{:.2f}", params.sparams.smoothing_factor, params.sparams.smoothing_curve);
                }
                result += "-> " + name_mirostat + " = " + std::to_string(params.sparams.mirostat); 
                result += std::format(";{}={:.2f}", name_mirostat_tau, params.sparams.mirostat_tau); 
                result += std::format(";{}={:.2f}", name_mirostat_eta, params.sparams.mirostat_eta);
            } else if (params.sparams.top_n_sigma >= 0) {
                result += std::format("->{}={}",name_top_k,params.sparams.top_k);
                if (params.sparams.penalty_repeat > 1.0f) {
                    result += std::format("->{}={:.2f}", name_penalty_repeat, params.sparams.penalty_repeat);
                    if (params.sparams.penalty_threshold != paramsDefault.sparams.penalty_threshold) result += std::format(";{}={:.2f}", name_penalty_threshold, params.sparams.penalty_threshold); 
                    if (params.sparams.penalty_freq != paramsDefault.sparams.penalty_freq) result += std::format(";{}={:.2f}", name_penalty_freq, params.sparams.penalty_freq);
                    if (params.sparams.penalty_present != paramsDefault.sparams.penalty_present) result += std::format(";{}={:.2f}", name_penalty_present, params.sparams.penalty_present);
                }
                //DRY
                if (params.sparams.dry_multiplier != paramsDefault.sparams.dry_multiplier) {
                    result += std::format("->{}={:.2f}*{:.2f}L{}N{}", name_dry, params.sparams.dry_base, params.sparams.dry_multiplier, params.sparams.dry_allowed_length, params.sparams.dry_penalty_last_n);
                }
                result += std::format("->{}={:.2f}", name_temp, params.sparams.temp);
                // result += std::format("->{}={:.2f}-{:.2f}", name_noise, params.sparams.noise_min, params.sparams.noise_max);
                result += std::format("->{}={:.2f}", name_top_n_sigma, params.sparams.top_n_sigma);
                result += std::format("->{}={:.2f}-{:.2f}", name_noise, params.sparams.noise_min, params.sparams.noise_max);
            } else {
                //DRY
                if (params.sparams.dry_multiplier != paramsDefault.sparams.dry_multiplier) {
                    result += std::format("->{}={:.2f}*{:.2f}L{}N{}", name_dry, params.sparams.dry_base, params.sparams.dry_multiplier, params.sparams.dry_allowed_length, params.sparams.dry_penalty_last_n);
                }
                for (auto s : params.sparams.samplers_sequence){
                    result += "->";
                    switch (s) {
                        case 'e': {
                            result += std::format("{}={:.2f}", name_penalty_repeat, params.sparams.penalty_repeat);
                            if (params.sparams.penalty_threshold != paramsDefault.sparams.penalty_threshold) result += std::format(";{}={:.2f}", name_penalty_threshold, params.sparams.penalty_threshold); 
                            if (params.sparams.penalty_freq != paramsDefault.sparams.penalty_freq) result += std::format(";{}={:.2f}", name_penalty_freq, params.sparams.penalty_freq);
                            if (params.sparams.penalty_present != paramsDefault.sparams.penalty_present) result += std::format(";{}={:.2f}", name_penalty_present, params.sparams.penalty_present);
                        }
                        break;
                        case 'k': result += name_top_k; if (params.sparams.top_k != paramsDefault.sparams.top_k) result += std::format("={}",params.sparams.top_k); break;
                        case 'f': result += name_tfs_z; if (params.sparams.tfs_z != paramsDefault.sparams.tfs_z) result += std::format("={:.2f}",params.sparams.tfs_z); break;
                        case 'y': result += name_typical_p; if (params.sparams.typical_p != paramsDefault.sparams.typical_p) result += std::format("={:.2f}",params.sparams.typical_p); break;
                        case 's': result += name_p_step; if (params.sparams.p_step != paramsDefault.sparams.p_step) result += std::format("={:.2f}",params.sparams.p_step); result += std::format("({})", p_step_total); break;
                        // case 'x': result += std::format("xtc={:.2f}-{:.2f}({}%/{})",params.sparams.xtc_threshold,params.sparams.xtc_threshold_max,(params.sparams.xtc_probability*100),params.sparams.xtc_min); if (params.sparams.xtc_probability_once) result += "once"; else result += "each"; result += std::format("-{}/{}({:.2f}%)", xtc_removed, xtc_total, xtc_percent); break;
                        case 'x': result += std::format("xtc={:.2f}-{:.2f}/{:.2f}%",params.sparams.xtc_threshold,params.sparams.xtc_threshold_max,params.sparams.xtc_probability*100); result += std::format("-{}/{}({:.2f}%)", xtc_removed, xtc_total, xtc_percent); break;
                        case 'p': result += name_top_p; if (params.sparams.top_p != paramsDefault.sparams.top_p) result += std::format("={:.2f}",params.sparams.top_p); break;
                        case 'o': result += std::format("{}={:.2f}-{:.2f}", name_noise, params.sparams.noise_min, params.sparams.noise_max); break;
                        case 'm': result += name_min_p; if (params.sparams.min_p != paramsDefault.sparams.min_p) result += std::format("={:.3f}%{:.2f}",params.sparams.min_p,params.sparams.min_p_rand); result += std::format("({}/{})", min_p_total, candidates_max_min_p); break;
                        case 'l': result += std::format("{}:{}({} max)", name_k_shift, params.sparams.confidence_shift, params.sparams.k_shift); break;
                        case 'r': result += name_rx; if (params.sparams.range_min != paramsDefault.sparams.range_min || params.sparams.range_max != paramsDefault.sparams.range_max) result += std::format("={:.2f}-{:.2f}:{}/{}({:.2f}%)",params.sparams.range_min, params.sparams.range_max, rx_removed, rx_total, rx_percent); break;
                        case 't': {
                                if (params.sparams.temp_adaptive == true) {
                                    result += name_temp + "-ADP";
                                } else {
                                    if (params.sparams.dynatemp_range > 0) {
                                        result += std::format("{}({:.2f}-{:.2f})",name_dynatemp_range, params.sparams.temp > params.sparams.dynatemp_range ? params.sparams.temp - params.sparams.dynatemp_range : 0, params.sparams.temp + params.sparams.dynatemp_range);
                                        if (params.sparams.smoothing_curve != paramsDefault.sparams.smoothing_curve) result += std::format("*{:.2f}", params.sparams.smoothing_curve);
                                    } else {
                                        result += name_temp;
                                        if (params.sparams.temp != paramsDefault.sparams.temp) result += std::format("={:.2f}",params.sparams.temp);
                                        // smoothing_curve doesn't work without dynatemp anyway
                                    }
                                    if (params.sparams.smoothing_factor != paramsDefault.sparams.smoothing_factor) result += std::format("^{:.2f}", params.sparams.smoothing_factor);
                                }
                                //result += std::format("({})", temp_total);
                                break;
                            }
                        case 'T': {
                                result += std::format("{}={:.2f}", name_temp, params.sparams.temp);
                                if (params.sparams.temp <= 0) result += "(greedy)";
                                break;
                            }
                        default : result += std::format("\"{}\"??",s); break;
                    }
                }
            }
        //}
        result += std::format(":{}({}vs{}={:.2f})", candidates_max, num_probs_tops, num_probs_bottoms, confidence_total);
        if (params.sparams.confidence_top > 0) result += std::format(" Cnf[{:.2f}-{:.2f}]", params.sparams.confidence_top, params.sparams.confidence_bottom);
        return result;
    }

    int load_and_process(bool streamed = true, bool soft = false) {
        int result = load(soft);

        process_prompt(streamed);
        return result;
    }

    int init(int argc, char ** argv){
        // this is unsafe to run in threads;
        getArgs(argc, argv);
        
        printf("\n LOADING \n");
        
        return init_default();
    }

    int init_default(bool useJson = true, bool headless = false){

        if (useJson) readParamsFromFile("config.json", params, headless);

        return load_and_process();
    }

    int init_custom(std::string modelName, std::string prompt = "NULL", std::string antiprompt = "NULL"){

        if (modelName != "NULL") readParamsFromFile("config.json", modelName, params);
        else readParamsFromFile("config.json", params);
        
        if (prompt != "NULL") params.prompt = prompt;
        if (antiprompt != "NULL") {
            if(!std::size(params.antiprompt)) 
                params.antiprompt.emplace_back(antiprompt);
            else
                params.antiprompt[0] = antiprompt;
        }

        return load_and_process();
    }

    int initialize(nlohmann::json configJson, bool streamed = true, bool soft = false) {

        readParamsFromJson(configJson, params);

        return load_and_process(streamed, soft);
    }

    void formatInput(std::string format, std::string& buffer) {
        // printf("%s: FORMAT = %s\n", __func__, format.c_str());
        for (auto s : format) {
            switch (s){
                case 'a':{
                    // printf("%s: found antiprompt\n", __func__);
                    if (std::size(params.antiprompt)){
                        formatRepresentation += params.antiprompt[0];
                        // const auto line_antiprompt_format = common_tokenize(ctx, params.antiprompt[0], false, true);
                        // embd_inp.insert(embd_inp.end(), line_antiprompt_format.begin(), line_antiprompt_format.end());
                        tokenizeAntiprompt();
                    }
                    break;
                }
                case 'b':{
                    if (!params.bos.empty()) {
                        // printf("%s: found custom bos \n", __func__);
                        formatRepresentation += params.bos;
                        const auto line_bos_format = common_tokenize(ctx, params.bos, false, true);
                        embd_inp.insert(embd_inp.end(), line_bos_format.begin(), line_bos_format.end());
                    } else {
                        // printf("%s: found model bos \n", __func__);
                        formatRepresentation += llama_token_get_text(vocab, llama_vocab_bos(vocab));
                        embd_inp.emplace_back(llama_vocab_bos(vocab));
                    }
                    break;
                }
                case 'e':{
                    if (!params.eos.empty()) {
                        // printf("%s: found custom eos \n", __func__);
                        formatRepresentation += params.eos;
                        const auto line_eos_format = common_tokenize(ctx, params.eos, false, true);
                        embd_inp.insert(embd_inp.end(), line_eos_format.begin(), line_eos_format.end());
                    } else {
                        // printf("%s: found model eos \n", __func__);
                        formatRepresentation += llama_token_get_text(vocab, llama_vocab_eos(vocab));
                        embd_inp.emplace_back(llama_vocab_eos(vocab));
                    }
                    break;
                }
                case 'p':{
                    // printf("%s: found prefix \n", __func__);
                    formatRepresentation += params.input_prefix;
                    const auto line_pfx = common_tokenize(ctx, params.input_prefix, false, true);
                    embd_inp.insert(embd_inp.end(), line_pfx.begin(), line_pfx.end());
                    break;
                }
                case 'i':{
                    // printf("%s: found input \n", __func__);
                    /* if (format == params.format_instruct) */ formatRepresentation += buffer;
                    //else formatRepresentation += "{input}";
                    const auto line_inp = common_tokenize(ctx, buffer, false, false);
                    embd_inp.insert(embd_inp.end(), line_inp.begin(), line_inp.end());
                    if (n_remain > 0) n_remain -= std::size(line_inp);
                    break;
                }
                case 's':{
                    // printf("%s: found suffix \n", __func__);
                    formatRepresentation += params.input_suffix;
                    const auto line_sfx = common_tokenize(ctx, params.input_suffix, false, true);
                    embd_inp.insert(embd_inp.end(), line_sfx.begin(), line_sfx.end());
                    break;
                }
                case 'd':{
                    // printf("%s: found deliminer \n", __func__);
                    formatRepresentation += "\n";
                    const auto line_delim = common_tokenize(ctx, "\n", false, true);
                    embd_inp.insert(embd_inp.end(), line_delim.begin(), line_delim.end());
                    break;
                }
            }

        }

    }
    
    int checkPreLoad(){
    
        
    
        if (params.embedding) {
            printf("\n************\n");
            printf("%s: please use the 'embedding' tool for embedding calculations\n", __func__);
            printf("************\n\n");

            return 0;
        }
        
        if (params.rope_freq_scale != 0.0) {
            fprintf(stderr, "%s: warning: scaling RoPE frequency by %g (default 1.0)\n", __func__, params.rope_freq_scale);
        }
        
        //const int n_ctx_train = llama_n_ctx_train(model);
        //n_ctx = llama_n_ctx(ctx);

        //fprintf(stderr, "Context size %zu \n", params.n_ctx);
        // if (params.n_ctx > 2048) {
            // fprintf(stderr, "%s: warning: model might not support context sizes greater than 2048 tokens (%d specified);"
                    // "expect poor results\n", __func__, params.n_ctx);
        // const int n_ctx_train = llama_n_ctx_train(ctx);
        // if (params.n_ctx > n_ctx_train) {
            // fprintf(stderr, "%s: warning: model was trained on only %d context tokens (%d specified)\n",
                    // __func__, n_ctx_train, params.n_ctx);
        // } else 
        if (params.n_ctx < 8) {
            fprintf(stderr, "%s: warning: minimum context size is 8, using minimum size.\n", __func__);
            params.n_ctx = 8;
        }
        
        return 1;
    }
    
    int setRandomSeed(){
        if (params.sparams.seed == LLAMA_DEFAULT_SEED) {
            //params.seed = time(NULL);
            // std::srand(std::time(nullptr));
            // if (std::rand() > std::rand()){
                // params.seed = std::rand()*std::rand();
            // } else params.seed = std::rand();
            
            params.sparams.seed = getRand();
        }

        //fprintf(stderr, "%s: seed  = %d\n", __func__, params.seed);
        return 2;
    }
    
    void tokenize_antiprompt() {
        // tokenized antiprompts

        antiprompt_ids.reserve(std::size(params.antiprompt));
        for (const std::string & antiprompt : params.antiprompt) {
            antiprompt_ids.emplace_back(common_tokenize(ctx, antiprompt, false, true));
        }
    }

    int check_encoder() {
        if (llama_model_has_encoder(model)) {
            int enc_input_size = std::size(embd_inp);
            llama_token * enc_input_buf = embd_inp.data();

            if (llama_encode(ctx, llama_batch_get_one(enc_input_buf, enc_input_size))) {
                return 1;
            }

            llama_token decoder_start_token_id = llama_model_decoder_start_token(model);
            if (decoder_start_token_id == LLAMA_TOKEN_NULL) {
                decoder_start_token_id = llama_vocab_bos(vocab);
            }

            embd_inp.clear();
            //embd_inp.push_back(decoder_start_token_id);
            embd_inp.emplace_back(decoder_start_token_id);
        }

        return 0;
    }

    int assignThreads() {
        printf("%s: llama threadpool init = n_threads = %d\n",
            __func__,
            (int) params.cpuparams.n_threads
        );

        // auto * reg = ggml_backend_dev_backend_reg(ggml_backend_dev_by_type(GGML_BACKEND_DEVICE_TYPE_CPU));
        // this is somewhat redundant since we don't use dynamic backens
        // however, it's better than no check at all
        auto * cpu_dev = ggml_backend_dev_by_type(GGML_BACKEND_DEVICE_TYPE_CPU);
        if (!cpu_dev) {
            printf("%s: no CPU backend found\n", __func__);
            return 1;
        }
        auto * reg = ggml_backend_dev_backend_reg(cpu_dev);

        auto * ggml_threadpool_new_fn = (decltype(ggml_threadpool_new) *) ggml_backend_reg_get_proc_address(reg, "ggml_threadpool_new");
        auto * ggml_threadpool_free_fn = (decltype(ggml_threadpool_free) *) ggml_backend_reg_get_proc_address(reg, "ggml_threadpool_free");

        struct ggml_threadpool_params tpp_batch =
            ggml_threadpool_params_from_cpu_params(params.cpuparams_batch);
        struct ggml_threadpool_params tpp =
                ggml_threadpool_params_from_cpu_params(params.cpuparams);

        set_process_priority(params.cpuparams.priority);

        if (!ggml_threadpool_params_match(&tpp, &tpp_batch)) {
            threadpool_batch = ggml_threadpool_new_fn(&tpp_batch);
            if (!threadpool_batch) {
                printf("%s: batch threadpool create failed : n_threads %d\n", __func__, tpp_batch.n_threads);
                exit(1);
            }

            // Start the non-batch threadpool in the paused state
            tpp.paused = true;
        }

        threadpool = ggml_threadpool_new_fn(&tpp);
        if (!threadpool) {
            printf("%s: threadpool create failed : n_threads %d\n", __func__, tpp.n_threads);
            exit(1);
        }

        llama_attach_threadpool(ctx, threadpool, threadpool_batch);

        return 1;
    }

    int load(bool soft = false){
        
        printf("Load start \n");
        
        auto & sparams = params.sparams;
        // this function is only needed if backends are compiled as dynamic libraries
        // there might be buffer problems for now
        ggml_backend_load_all();
        printf("..............Loaded dynamic backends.(%s)................\n", __func__);
        
        // if (!soft){
            // int status = 0;
            
            // printf("strictLoad \n");
            // status = strictLoad();
            // if (status == 0) return 0;
        // }
        
        if (!soft){
            int status = 0;
            
            status += checkPreLoad(); // 1
            printf("checkPreLoad = %d\n", status);
            if (status == 0) return 0;
            
            //loading the model itself; uses llama_backend, ctx, ctx_guidance
            // LET'S TRY THIS
            //llama_init_backend(params.numa);

//#define GGML_OPENCL_DEFAULT_PLATFORM_ID params.clblast_platform_id
#ifdef GGML_USE_CLBLAST
// #undef GGML_OPENCL_DEFAULT_PLATFORM_ID
// #define GGML_OPENCL_DEFAULT_PLATFORM_ID params.clblast_platform_id
            GGML_OPENCL_DEFAULT_PLATFORM_ID = params.clblast_platform_id;
            printf("..GGML_OPENCL_DEFAULT_PLATFORM_ID = %d..\n", GGML_OPENCL_DEFAULT_PLATFORM_ID);
#endif

            llama_backend_init();
#ifdef GGML_USE_CLBLAST
            printf("..............Backend initialized common: %s................\n", GGML_OPENCL_RESULT_DEVICE_NAME);
#else
            printf("..............Backend initialized common (%s)................\n", __func__);
#endif

            // load the model and apply lora adapter, if any
            common_init_result llama_init = common_init_from_params(params);
            printf("..............common_init_from_params (%s)................\n", __func__);

            // model = llama_init.model.release();
            // model = llama_init.model.get();
            model = llama_init.model.release();
            vocab = llama_model_get_vocab(model);
            printf("..............Model initialized (%s)................\n", __func__);

            // ctx = llama_init.context.release();
            // ctx = llama_init.context.get();
            ctx = llama_init.context.release();
            printf("..............Context initialized (%s)................\n", __func__);

            assignThreads();
            printf("..............Threads assigned (%s)................\n", __func__);

            if (model == NULL) {
                fprintf(stderr, "%s: error: unable to load model\n", __func__);
                return 0;
            }

            status += 3; // +3


            printf("%s: loadModel = %d\n", __func__, status);
            if (status < 3) return 0;

            // setting seed
            status += setRandomSeed();
            //common_sampler_get_seed(smpl);
            printf("%s: setRandomSeed = %d\n", __func__, params.sparams.seed);

            if (status == 0) return 0;
        }
        
        cleared = false;
        n_keep = params.n_keep;

        n_ctx = llama_n_ctx(ctx);
        printf("%s: llama_n_ctx = %d\n", __func__, n_ctx);

        smpl = common_sampler_init(model, sparams);
        printf("%s: common_sampler_init\n", __func__);
        if (!smpl) {
            fprintf(stderr, "%s: failed to initialize sampling subsystem\n", __func__);
            exit(1);
        }

        common_sampler_get_seed(smpl);
        printf("%s: common_sampler_get_seed\n", __func__);

        ga_n = params.grp_attn_n;
        ga_w = params.grp_attn_w;

        if (ga_n != 1) {
            GGML_ASSERT(ga_n > 0                    && "grp_attn_n must be positive");                     // NOLINT
            GGML_ASSERT(ga_w % ga_n == 0            && "grp_attn_w must be a multiple of grp_attn_n");     // NOLINT
        }

        printf("%s: checking for a saved session...\n", __func__);
        path_session = params.path_prompt_cache;

        if (!path_session.empty()) {
            fprintf(stderr, "%s: attempting to load saved session from '%s'\n", __func__, path_session.c_str());

            // fopen to check for existing session
            FILE * fp = std::fopen(path_session.c_str(), "rb");
            if (fp != NULL) {
                std::fclose(fp);
                
                session_tokens.resize(n_ctx);
                size_t n_token_count_out = 0;
                if (!llama_state_load_file(ctx, path_session.c_str(), session_tokens.data(), session_tokens.capacity(), &n_token_count_out)) {
                    fprintf(stderr, "%s: error: failed to load session file '%s'\n", __func__, path_session.c_str());
                    return 1;
                }
                session_tokens.resize(n_token_count_out);
                //llama_set_rng_seed(ctx, params.sparams.seed);

                fprintf(stderr, "%s: loaded a session with prompt size of %d tokens\n", __func__, (int) std::size(session_tokens));
            } else {
                fprintf(stderr, "%s: session file does not exist, will create\n", __func__);
            }
        }

        // in instruct mode, we inject a prefix and a suffix to each input by the user
        // we are in a permanent dialog mode 
        params.interactive_first = true;
        params.interactive = true;

        if (!std::size(params.antiprompt)) params.antiprompt.emplace_back("You:");
        //if (params.prompt.empty()) params.prompt = params.antiprompt.back();
        // instruct mode was removed since its format is not universal enough

        // std::string pause = "";
        // std::getline(std::cin, pause);

        add_bos = llama_vocab_get_add_bos(vocab);
        printf("%s: add_bos: %d\n", __func__, add_bos);

        if (!llama_model_has_encoder(model)) {
            GGML_ASSERT(!llama_vocab_get_add_eos(vocab));
        }

        add_eos = llama_vocab_get_add_eos(vocab);
        printf("%s: add_eos: %d\n", __func__, add_eos);

        if (add_bos == true) txt_vocab_bos = llama_token_get_text(vocab, llama_vocab_bos(vocab));
        txt_vocab_eos = llama_token_get_text(vocab, llama_vocab_eos(vocab));
        printf("%s: txt_vocab_bos: %s\n", __func__, txt_vocab_bos.c_str());
        printf("%s: txt_vocab_eos: %s\n", __func__, txt_vocab_eos.c_str());

        if (params.interactive_first || !params.prompt.empty() || session_tokens.empty()) {
            //this is the first problem we have
            // there is no formatting for the initial prompt
            formatInput(params.format_instruct, params.prompt);
            printf("%s: init formatting finished\n", __func__);
        } else {
            embd_inp = session_tokens;
        }

        if (embd_inp.empty() && add_bos == true) {
            embd_inp.emplace_back(llama_vocab_bos(vocab));
            //embd_inp.push_back(llama_token_bos(model));
        }

        // Tokenize negative prompt
        // removed here by sampling refactor v2

        if ((int) std::size(embd_inp) > n_ctx - 4) {
            fprintf(stderr, "%s: error: prompt is too long (%d tokens, max %d)\n", __func__, (int) std::size(embd_inp), n_ctx - 4);
            return 1;
        }
        printf("%s: embd_inp filled\n", __func__, add_eos);

        // debug message about similarity of saved session, if applicable
        n_matching_session_tokens = 0;
        if (std::size(session_tokens)) {
            for (llama_token id : session_tokens) {
                if (n_matching_session_tokens >= std::size(embd_inp) || id != embd_inp[n_matching_session_tokens]) {
                    break;
                }
                n_matching_session_tokens++;
            }
            if (params.prompt.empty() && n_matching_session_tokens == std::size(embd_inp)) {
                fprintf(stderr, "%s: using full prompt from session file\n", __func__);
            } else if (n_matching_session_tokens >= std::size(embd_inp)) {
                fprintf(stderr, "%s: session file has exact match for prompt!\n", __func__);
            } else if (n_matching_session_tokens < (std::size(embd_inp) / 2)) {
                fprintf(stderr, "%s: warning: session file has low similarity to prompt (%zu / %zu tokens); will mostly be reevaluated\n",
                    __func__, n_matching_session_tokens, std::size(embd_inp));
            } else {
                fprintf(stderr, "%s: session file matches %zu / %zu tokens of prompt\n",
                    __func__, n_matching_session_tokens, std::size(embd_inp));
            }

            // remove any "future" tokens that we might have inherited from the previous session
            //llama_kv_cache_tokens_rm(ctx, n_matching_session_tokens, -1);
            llama_kv_self_seq_rm(ctx, -1, n_matching_session_tokens, -1);
        }

        // if we will use the cache for the full prompt without reaching the end of the cache, force
        // reevaluation of the last token token to recalculate the cached logits
        if (!embd_inp.empty() && n_matching_session_tokens == std::size(embd_inp) &&
                std::size(session_tokens) > std::size(embd_inp)) {
            session_tokens.resize(std::size(embd_inp) - 1);
        }

        // number of tokens to keep when resetting context
        if (params.n_keep < 0 || params.n_keep > (int) std::size(embd_inp)) {
            params.n_keep = (int)std::size(embd_inp);
        } else {
            params.n_keep += add_bos; // always keep the BOS token
        }

        
        fprintf(stderr, "\nsampling:\n repeat_last_n = %d,\n penalty_repeat = %f,\n penalty_present = %f,\n penalty_freq = %f,\n top_k = %d,\n tfs_z = %f,\n top_p = %f,\n typical_p = %f,\n temp = %f,\n mirostat = %d,\n mirostat_lr = %f,\n mirostat_ent = %f\n",
                sparams.penalty_last_n, sparams.penalty_repeat, sparams.penalty_present, sparams.penalty_freq, sparams.top_k, sparams.tfs_z, sparams.top_p, sparams.typical_p, sparams.temp, sparams.mirostat, sparams.mirostat_eta, sparams.mirostat_tau);

        if (params.interactive) {
            const char *control_message;

            control_message = " - Press Return to return control to LLaMa.\n"
                              " - To return control without starting a new line, end your input with '/'.\n"
                              " - If you want to submit another line, end your input with '\\'.\n";

            is_interacting = params.interactive_first;
        }

        //ctx_sampling = llama_sampling_init(params);

        bool need_to_save_session = !path_session.empty() && n_matching_session_tokens < std::size(embd_inp);

        n_remain           = params.n_predict;
        n_remain_last      = std::size(embd_inp);

        printf("%s: Load finished\n", __func__);

        return 9;
    }
//-----------------------------------------MAIN CYCLES------------------------------------------- 
//-----------------------------------------MAIN CYCLES------------------------------------------- 
//-----------------------------------------MAIN CYCLES-------------------------------------------     
    
    void checkSize(){
        // Note: n_ctx - 4 here is to match the logic for commandline prompt handling via
        // --prompt or --file which uses the same value.
        int max_embd_size = n_ctx - 4;
        // Ensure the input doesn't exceed the context size by truncating embd if necessary.
        if ((int)std::size(embd) > max_embd_size) {
            const int skipped_tokens = (int) std::size(embd) - max_embd_size;
            embd.resize(max_embd_size);
            printf("<<input too long: skipped %d token%s>>", skipped_tokens, skipped_tokens != 1 ? "s" : "");
            fflush(stdout);
            
        }
    }
    
    void resetContext() {
        // infinite text generation via context swapping
        // if we run out of context:
        // - take the n_keep first tokens from the original prompt (via n_past)
        // - take half of the last (n_ctx - n_keep) tokens and recompute the logits in batches
        if (n_past + (int) std::size(embd) >= n_ctx) {
            printf("....");
            const int n_left    = n_past - params.n_keep;
            const int n_discard = n_left/2;

            // always keep the first token - BOS
            //n_past = std::max(1, params.n_keep);
            //n_past_guidance = std::max(1, params.n_keep + guidance_offset);
            llama_kv_self_seq_rm (ctx, 0, params.n_keep            , params.n_keep + n_discard);
            llama_kv_self_seq_add(ctx, 0, params.n_keep + n_discard, n_past, -n_discard);

            // insert n_left/2 tokens at the start of embd from last_n_tokens
            //embd.insert(embd.begin(), last_n_tokens.begin() + n_ctx - n_left/2 - embd.size(), last_n_tokens.end() - embd.size());
            //embd.insert(embd.begin(), last_tokens.begin() + n_ctx - n_left/2 - embd.size(), last_tokens.end() - embd.size());
            n_past -= n_discard;

            // stop saving session if we run out of context
            path_session.clear();

        }
    }
    
    void extendContext() {
        if (ga_n == 1) {
            // infinite text generation via context shifting
            // if we run out of context:
            // - take the n_keep first tokens from the original prompt (via n_past)
            // - take half of the last (n_ctx - n_keep) tokens and recompute the logits in batches

            if (n_past + (int) std::size(embd) >= n_ctx) {
                if (!params.ctx_shift) {
                    printf("\n\n%s: context full and context shift is disabled => stopping\n", __func__);
                    return;
                }

                if (params.n_predict == -2) {
                    printf("\n\n%s: context full and n_predict == -%d => stopping\n", __func__, params.n_predict);
                    return;
                }

                const int n_left    = n_past - params.n_keep;
                const int n_discard = n_left/2;

                llama_kv_self_seq_rm (ctx, 0, params.n_keep            , params.n_keep + n_discard);
                llama_kv_self_seq_add(ctx, 0, params.n_keep + n_discard, n_past, -n_discard);

                n_past -= n_discard;

                path_session.clear();
            }
        } else {
            // context extension via Self-Extend
            while (n_past >= ga_i + ga_w) {
                const int ib = (ga_n*ga_i)/ga_w;
                const int bd = (ga_w/ga_n)*(ga_n - 1);
                const int dd = (ga_w/ga_n) - ib*bd - ga_w;
                
                llama_kv_self_seq_add(ctx, 0, ga_i,                n_past,              ib*bd);
                llama_kv_self_seq_div  (ctx, 0, ga_i + ib*bd,        ga_i + ib*bd + ga_w, ga_n);
                llama_kv_self_seq_add(ctx, 0, ga_i + ib*bd + ga_w, n_past + ib*bd,      dd);
                
                n_past -= bd;

                ga_i += ga_w/ga_n;
            }
        }
    }

    int reuse() {
        // try to reuse a matching prefix from the loaded session instead of re-eval (via n_past)
        if (n_session_consumed < (int) std::size(session_tokens)) {
            size_t i = 0;
            for ( ; i < std::size(embd); i++) {
                if (embd[i] != session_tokens[n_session_consumed]) {
                    session_tokens.resize(n_session_consumed);
                    return 0;
                }

                n_past++;
                //n_past_last = n_past;
                n_session_consumed++;

                if (n_session_consumed >= (int) std::size(session_tokens)) {
                    ++i;
                    return 0;
                }
            }
            if (i > 0) {
                embd.erase(embd.begin(), embd.begin() + i);
            }
            
            // remove any "future" tokens that we might have inherited from the session from the KV cache
            //llama_kv_cache_tokens_rm(ctx, n_past, -1);
        }
        
        return 1;
    }

    int evaluateEmbd() {
        for (int i = 0; i < (int) std::size(embd); i += params.n_batch) {
            int n_eval = (int) std::size(embd) - i;
            if (n_eval > params.n_batch) {
                n_eval = params.n_batch;
            }
            if (llama_decode(ctx, llama_batch_get_one(&embd[i], n_eval))) {
                fprintf(stderr, "%s : failed to eval\n", __func__);
                std::string report = std::format("\n{}(): {} failed to eval: {}", __func__, params.model, embd[i]);
                writeTextFile(std::to_string(params.sparams.seed) + ".txt", report);
                return 0;
            }
            n_past += n_eval;
            //n_past_last = n_past;
        }
        
        return 1;
    }

    void evaluateSession() {
        if (std::size(embd) > 0 && !path_session.empty()) {
            session_tokens.insert(session_tokens.end(), embd.begin(), embd.end());
            n_session_consumed = std::size(session_tokens);
        }
    }

    void check_antiprompt_tkns() {
        // check for reverse prompt using special tokens
        llama_token last_token = common_sampler_last(smpl);
        for (std::vector<llama_token> ids : antiprompt_ids) {
            if (std::size(ids) == 1 && last_token == ids[0]) {
                if (params.interactive) {
                    is_interacting = true;
                    has_antiprompt = std::format("{}: already has antiprompt", __func__);
                }
                is_antiprompt = true;
                break;
            }
        }
    }

    //checking already existing contex
    int checkEmbd(){
        if (debug) printf("-ce");

        // Note: n_ctx - 4 here is to match the logic for commandline prompt handling via
        // --prompt or --file which uses the same value.
        checkSize();

        // infinite text generation via context swapping
        // if we run out of context:
        // - take the n_keep first tokens from the original prompt (via n_past)
        // - take half of the last (n_ctx - n_keep) tokens and recompute the logits in batches
        //resetContext();
        extendContext();

        // try to reuse a matching prefix from the loaded session instead of re-eval (via n_past)
        if (reuse() == 0) return 0;

        // evaluate tokens in batches
        // embd is typically prepared beforehand to fit within a batch, but not always
        // the first call processes the prompt
        if (evaluateEmbd() == 0) return 0;

        if (!path_session.empty()) evaluateSession();

        captureStateOnce();

        return 1;
    }

    // main generation, includes adding antiprompt at the end
    int sampleTknIntoEmbd(bool emptyMessage = false) {
        if (debug) printf("-ae");

        // optionally save the session on first sample (for faster prompt loading next time)
        if (!path_session.empty() && need_to_save_session && !params.prompt_cache_ro) {
            need_to_save_session = false;
            llama_state_save_file(ctx, path_session.c_str(), session_tokens.data(), std::size(session_tokens));
        }

        // new generation function, moved to common 
        // const llama_token id = common_sampler_sample(smpl, ctx, -1);
        llama_token id = common_sampler_sample(smpl, ctx, -1);

        // try to sample a different token to avoid empty messages
        while (emptyMessage == true && llama_token_is_eog(vocab, id)) {
            ++c_empty_msgs;
            common_sampler_reset(smpl);
            id = common_sampler_sample(smpl, ctx, -1);
        }

        // accept the result
        common_sampler_accept(smpl, id, /* apply_grammar= */ true);

        // add it to the contexts
        embd.emplace_back(id);

        // echo this to console
        // no longer used
        // input_echo = true;

        // decrement remaining sampling budget
        if (n_remain > 0) --n_remain;

        return 1;
    }

// main generation end////////////////////////////////////////////////////////////////

// additional functions
    void captureStateOnce() {
        if (rewind_state.kv_cache_pos == 0) {
            capture_smpl();
            // rewind_state.capture_kv_cache(llama_kv_cache_seq_pos_max(ctx, 0));
            // rewind_state.capture_kv_cache(llama_kv_self_seq_pos_max(ctx, -1));
            rewind_state.capture_kv_cache(llama_kv_self_seq_pos_max(ctx, 0));
            rewind_state.capture_embd_inp(embd_inp.size());
            rewind_state.capture_n_past(n_past);
            rewind_state.capture_n_consumed(n_consumed);
        }
    }

    int get_kv_cache_seq_pos_max() {
        // return llama_kv_cache_seq_pos_max(ctx, 0);
        // return llama_kv_self_seq_pos_max(ctx, -1);
        return llama_kv_self_seq_pos_max(ctx, 0);
    }

    void clearStates2() {
        rewind_state.kv_cache_pos = 0;
        rewind_state.embd_inp_size = 0;
        rewind_state.n_past_size = 0;
        rewind_state.n_consumed_size = 0;
    }

    void rewindBack() {
    // sampling
        common_sampler_reset(smpl);
        restore_smpl();
        //common_sampler_reset(smpl);
    // context
        llama_kv_self_seq_rm(ctx, 0, rewind_state.kv_cache_pos, -1);
        // llama_kv_self_seq_rm(ctx, -1, rewind_state.kv_cache_pos, -1);
        // llama_kv_cache_update(ctx);
    // chat parameters
        embd_inp.erase(embd_inp.begin() + rewind_state.embd_inp_size, embd_inp.end());
        n_past = rewind_state.n_past_size - 1;
        n_consumed = rewind_state.n_consumed_size;
    }

    int resetGrammar(){
        if (is_interacting) {
            // reset grammar state if we're restarting generation
            // if (grammar != NULL) {
                // llama_grammar_free(grammar);

                // std::vector<const llama_grammar_element *> grammar_rules(
                    // parsed_grammar.c_rules());
                // grammar = llama_grammar_init(
                    // grammar_rules.data(), grammar_rules.size(),
                    // parsed_grammar.symbol_ids.at("root"));
            // }
            common_sampler_reset(smpl);
        }

        //is_interacting = false;

        return 1;
    }

    int checkInfinite() {
        // In interactive mode, respect the maximum number of tokens and drop back to user input when reached.
        // We skip this logic when n_predict == -1 (infinite) or -2 (stop at context size).
        if (n_remain <= 0 && params.n_predict >= 0) {
            n_remain = params.n_predict;
            is_interacting = true;
        }
    }

    // checking antiprompts 
    int checkAntiprompt() {
        // check for reverse prompt in the last n_prev tokens
        if (std::size(params.antiprompt)) {
            const int n_prev = 32;
            const std::string last_output = common_sampler_prev_str(smpl, ctx, n_prev);

            is_antiprompt = false;
            // Check if each of the reverse prompts appears at the end of the output.
            // If we're not running interactively, the reverse prompt might be tokenized with some following characters
            // so we'll compensate for that by widening the search window a bit.
            for (std::string & antiprompt : params.antiprompt) {
                size_t extra_padding = params.interactive ? 0 : 2;
                size_t search_start_pos = last_output.length() > static_cast<size_t>(antiprompt.length() + extra_padding)
                    ? last_output.length() - static_cast<size_t>(antiprompt.length() + extra_padding)
                    : 0;

                if (last_output.find(antiprompt, search_start_pos) != std::string::npos) {
                    //formatRepresentation += params.antiprompt[0];
                    has_antiprompt = std::format("{}: already has antiprompt", __func__);
                    if (params.interactive) {
                        is_interacting = true;
                    }
                    if (debug) printf("-ca");
                    is_antiprompt = true;
                    fflush(stdout);
                    break;
                }
            }
        }

        check_antiprompt_tkns();

        return 1;
    }

    void tokenizeAntiprompt() {
        if (std::size(params.antiprompt) != 0) {
            // tokenize and inject first reverse prompt
            std::string end_string = params.antiprompt.front();
            const auto first_antiprompt = common_tokenize(ctx, end_string, false, true);
            embd_inp.insert(embd_inp.end(), first_antiprompt.begin(), first_antiprompt.end());

            has_antiprompt = std::format("{}: TRUE, + antiprompt: '''{}'''", __func__, end_string);

            is_antiprompt = true;
        }
    }

    // inserts antiprompt after EOS/EOG
    bool checkEOS() {
        // deal with end of text token in interactive mode
        if (llama_token_is_eog(vocab, common_sampler_last(smpl))) {
            if (params.interactive) {
                if (!is_antiprompt) tokenizeAntiprompt();

                is_interacting = true;
                return true;
            }
        }

        return false;
    }

    void appendSuffix(std::string& buffer){
        if (!params.input_suffix.empty()) {
            buffer += params.input_suffix;
        }
    }

    void appendPrefix(std::string& buffer){
        if (!params.input_prefix.empty()) {
            buffer = params.input_prefix + buffer;
            printf("%s", buffer.c_str());
        }
    }

// assembled work on context
    int InitEmbdProcessing(std::string& result, bool& fastStop, bool streaming = false){ // 1 2 3 4
        //std::cout << " ** " << std::endl;
        
        if (std::size(embd) > 0) {
            if (checkEmbd() == 0) return 0; // 1
        }

        embd.clear();

        if ((int) std::size(embd_inp) <= n_consumed && !is_interacting) {
            sampleTknIntoEmbd(); // 2
        } else {
            if (debug) printf("-pes");
            fastStop = true;
            // some user input remains from prompt or interaction, forward it to processing
            while ((int) std::size(embd_inp) > n_consumed) {
                //fprintf(stderr, ">");
                embd.emplace_back(embd_inp[n_consumed]);
                //embd.push_back(embd_inp[n_consumed]);
                //last_n_tokens.erase(last_n_tokens.begin());
                //last_n_tokens.emplace_back(embd_inp[n_consumed]);
                //last_tokens.erase(last_tokens.begin());
                //last_tokens.emplace_back(embd_inp[n_consumed]);
                
                // push the prompt in the sampling context in order to apply repetition penalties later
                // for the prompt, we don't apply grammar rules
                common_sampler_accept(smpl, embd_inp[n_consumed], /* apply_grammar= */ false);
                
                
                ++n_consumed;
                if ((int) std::size(embd) >= params.n_batch) {
                    break;
                }
            }
        }

        // display and return text
        
        
        if (input_echo) {
            //printf("-pei");
            for (auto id : embd) { 
                //std::string tknStr = llama_token_to_string(ctx, id); 
                const std::string tknStr = common_token_to_piece(ctx, id); 
                //result += (std::string) tknStr;
                result += tknStr;
                //if (streaming) printf("%s", tknStr);
                //std::cout<<tknStr;
            }

        }
        
        // if ((int) embd_inp.size() <= n_consumed) {
            // checkAntiprompt();
        // }
        
        return 1;
    }

// NEW //////////////////////////////////////////////////////////////////////////////////////////
    void resetCTX(){
        common_sampler_reset(smpl);
    }

    void appendPrefixBos(){
        if (params.input_prefix_bos) {
            embd_inp.emplace_back(llama_vocab_bos(vocab));
            //embd_inp.push_back(llama_token_bos(model));
        }
    }

    int getFormattedInput(std::string& line){
        //rewind_state.capture_embd_inp(embd_inp.size());
        //std::string buffer = line + DELIMINER;
        
        if (line.length() > 1) {
            if (debug) fprintf(stderr, "<");
            
            // Add tokens to embd only if the input buffer is non-empty
            // Entering a empty line lets the user pass control back
            //appendPrefixBos();
            
            //given the latest commit for chatml prompt, I feel like it's time for formats
            // from jsons or smth else
            //std::string buffer = params.input_prefix + line + DELIMINER + params.input_suffix;
            // since we have format now, all the rest will be added according to it
            std::string buffer = line;
            
            const size_t original_size = std::size(embd_inp);

            // format and process input
            //if (rewind_state.smpl_size == 0) 
                formatInput(params.format_dialog, buffer);
            //formatInput(buffer);
            // if (rewind_state.smpl_size == 0) 
                // capture_states2();

            // n_remain -= line_inp.size();
        }

        //is_antiprompt = false;
        
        return 1;
    }

// initial (instruct) processing
    std::string process_prompt(bool consoleOutput = true, bool verbose = false) {

        if (debug) printf("Starting initial prompt processing...\n");

        std::string result;
        //std::cout << " * " << std::endl;
        bool fastStop = false;

        //appendPrefixBos();

        // TECHNICALLY this is an overshoot, since we force antiprompt on user
        // formats are very different, so are models, some do not need this
        /*
        if (hasAntiprompt(params.prompt) == -1 && params.input_prefix.empty())  {
            params.prompt += DELIMINER + params.antiprompt[0];
            //formatRepresentation += DELIMINER + params.antiprompt[0];
        }
         */


        //ctx_sampling = llama_sampling_context_init(params, grammar);

        //n_vocab = llama_n_vocab(model);

        //candidates.reserve(n_vocab);

        //ctx_sampling = llama_sampling_init(params.sparams);

        tokenize_antiprompt();
        
        check_encoder();

        while ((n_remain != 0 && !is_antiprompt) || params.interactive) {

            if (consoleOutput) InitEmbdProcessing(result, fastStop, streaming); // includes checking, generation and adding prompt leftovers
            else InitEmbdProcessing(result, fastStop, consoleOutput);

            if ((int) std::size(embd_inp) <= n_consumed) {
                if (debug) printf("-g");

                //checkAntiprompt();

                if (n_past > 0 && is_interacting) {
                    checkAntiprompt();

                    if (verbose) {
                        if (!streaming) std::cout << result << " ";

                        if (debug) printf("Return generate: prompt processed\n");
                    }

                    // get_speed();
                    // get_speed_p();

                    return result;
                }

            }

        }

        return "cycle broken!";
    }

    const std::string getTknFromEmbd(){
        if (debug) printf("-gp");
        
        for (auto id : embd) { 
            //return llama_token_to_string(ctx, id); 
            return common_token_to_piece(ctx, id); 
        }
    }

    int hasAntiprompt(std::string& result){
        if (std::size(params.antiprompt)) {
            int cutAntiPos = result.rfind(params.antiprompt[0]);
            if (cutAntiPos != std::string::npos){
                return cutAntiPos;
            }
        }
        return -1;
    }

    int hasLast(std::string& result, std::string& stop){
        int cutAntiPos = result.rfind(stop);
        if (cutAntiPos != std::string::npos){
            return cutAntiPos;
        }
        return -1;
    }

    void eraseAntiprompt(std::string& result){
        if (std::size(params.antiprompt)) {
            // int cutAntiPos = result.rfind(params.antiprompt[0]);
            // if (cutAntiPos != std::string::npos){
                // result.erase(cutAntiPos);
            // }
            std::string anti = params.antiprompt[0];
            bool replaced = replace_string_mod(result, anti, "");
            while (replaced == true) {
                replaced = replace_string_mod(result, anti, "");
            }
        }
    }

    void eraseLast(std::string& result, std::string& stop){
        int cutAntiPos = result.rfind(stop);
        if (cutAntiPos != std::string::npos){
            result.erase(cutAntiPos);
        }
    }

    std::string getTokenOld(){ // 1 2 3 4
        if (std::size(embd) > 0) {
            if (checkEmbd() == 0) return txt_vocab_eos;  // 1, plus safety measure
        }

        embd.clear();

        if ((int) std::size(embd_inp) <= n_consumed && !is_interacting) {
            sampleTknIntoEmbd(); // 2
        } else {
            fromInpToEmbd();
            // n_consumed_last = n_consumed;
            // n_embd_inp_last = embd_inp.size();
        }

        // output text
        if (input_echo) {
            return getTknFromEmbd();
        }

        return "";
    }

    void fromInpToEmbd() {
        if (debug) printf("-pen");

        // some user input remains from prompt or interaction, forward it to processing
        while ((int) std::size(embd_inp) > n_consumed) {
            //embd.push_back(embd_inp[n_consumed]);
            embd.emplace_back(embd_inp[n_consumed]);

            // GG: I'm not sure it's a good idea to push the prompt tokens into the sampling context
            //     Most likely will remove this in the future to avoid exposing "prev"
            //     Same thing is done in "server". If we stop pushing the prompt tokens, then the repetition
            //     penalty will be applied only based on the tokens generated by the model.
            //ctx_sampling->prev.erase(ctx_sampling->prev.begin());
            //ctx_sampling->prev.push_back(embd_inp[n_consumed]);
            //ctx_sampling->prev.emplace_back(embd_inp[n_consumed]);

            ++n_consumed;
            if ((int) std::size(embd) >= params.n_batch) {
                break;
            }
        }

    }

    void skipInput() {
        is_interacting = false;
    }

    int checkAndClearEmbd() {
        if (std::size(embd) > 0) {
            if (checkEmbd() == 0) return 0; // 1
        }

        embd.clear();

        return 1;
    }

//input processing, which requires preemptive checking, adding prompt leftovers, antiprompt processing
    int inputProcessing(std::string& input){
        //std::cout << " ***** " << input << std::endl;
        formatRepresentation = "";
        has_antiprompt = "";

        // embd_msg.clear();

        if (n_past > 0 && is_interacting) {
            getFormattedInput(input);
            input_echo = false; // do not echo this again
        }

        if (n_past > 0) {
            resetGrammar();
            is_interacting = false;
        }

        fromInpToEmbd();

        if (checkAndClearEmbd() == 0) return 0;

        return 1;
    }

// this is an attempt to strictly separate all input-based preparations
// however, it assumes conditions (see in getTokenOld())
// fromInpToEmbd() and capture_states() should be done elsewhere
    std::string getBit(bool emptyMessage = false) { // 1 2 3 4
        //std::cout << " ** " << std::endl;
        //log_down(std::format("processEmb: {} vs {}\n", embd_inp.size(), n_consumed), params.seed);

        // if (checkAndClearEmbd() == 0) return txt_vocab_eos;
        if (checkAndClearEmbd() == 0) {
            finished = true;
            return txt_vocab_eos;
        }

        if (!is_interacting) sampleTknIntoEmbd(emptyMessage); // 2

        return getTknFromEmbd();
    }

    // token by token generation and pushing
    std::string cycleStringsOnly(bool stream = false, bool emptyMessage = false) {

        dynamicParamsPrepare();
        //process_prompt(false);  // do not forget to include it elsewhere after loading the model  
        //inputOnly(input); // MOVED

        std::string bit = getBit(emptyMessage);

        if ((int) std::size(embd_inp) <= n_consumed) {
            if (debug) printf("-cso");

            if (checkEOS() == false) checkAntiprompt();

            // if (!is_antiprompt) checkEOS();

            if (n_past > 0 && is_interacting) {

                finished = true;

                return bit;
            }

        }

        return bit;
    }

    int save(std::string fileName, std::vector<char*>& results){
        if (saveToFile){
            writeTextFile(SESSIONS_FOLDER + fileName, results);
        }
        
        return 1;
    }

};

