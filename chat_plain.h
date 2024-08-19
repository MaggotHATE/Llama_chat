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

#define SESSIONS_FOLDER "sessions/"

static gpt_params paramsDefault;

static bool is_interacting = false;

static void log_down(std::string text, uint32_t seed) {
    writeTextFile(std::to_string(seed) + ".log", text);
}

std::string llama_string_timings(struct llama_context * ctx) {
    const llama_timings timings = llama_get_timings(ctx);
    std::string result = __func__;
    result += " \n";
    char* buf = new char[1000];
    
    sprintf(buf, "load time = %8.2f ms\n", timings.t_load_ms);
    result += buf;
    
    sprintf(buf, "sample time = %8.2f ms / %5d runs   (%8.2f tokens per second)\n",timings.t_sample_ms, timings.n_sample, 1e3 / timings.t_sample_ms * timings.n_sample);
    result += buf;
    
    sprintf(buf, "prompt eval time = %8.2f ms / %5d tokens (%8.2f tokens per second)\n", timings.t_p_eval_ms, timings.n_p_eval, 1e3 / timings.t_p_eval_ms * timings.n_p_eval);
    result += buf;
    
    sprintf(buf, "eval time = %8.2f ms / %5d runs   (%8.2f tokens per second)\n",timings.t_eval_ms, timings.n_eval, 1e3 / timings.t_eval_ms * timings.n_eval);
    result += buf;
    
    sprintf(buf, "total time = %8.2f ms\n", (timings.t_end_ms - timings.t_start_ms));
    result += buf;
    
    return result;
}

std::string llama_string_timings_simple(struct llama_context * ctx) {
    const llama_timings timings = llama_get_timings(ctx);
    std::string result = __func__;
    result += " \n";
    char* buf = new char[1000];
    
    sprintf(buf, "load time = %8.2f ms\n", timings.t_load_ms);
    result += buf;
    
    sprintf(buf, "sampling: %8.2f tokens per second\n", 1e3 / timings.t_sample_ms * timings.n_sample);
    result += buf;
    
    sprintf(buf, "prompt eval:%8.2f tokens per second\n", 1e3 / timings.t_p_eval_ms * timings.n_p_eval);
    result += buf;
    
    sprintf(buf, "eval: %8.2f tokens per second\n", 1e3 / timings.t_eval_ms * timings.n_eval);
    result += buf;
    
    return result;
}

float llama_string_eval(struct llama_context * ctx) {
    const llama_timings timings = llama_get_timings(ctx);
    return (1e3 / timings.t_eval_ms * timings.n_eval);
}

float llama_string_evalPrompt(struct llama_context * ctx) {
    const llama_timings timings = llama_get_timings(ctx);
    return (1e3 / timings.t_p_eval_ms * timings.n_p_eval);
}

///

float llama_string_eval(const llama_timings & timings) {
    if (timings.t_eval_ms != 0 && timings.n_eval != 0) return (1e3 / timings.t_eval_ms * timings.n_eval);
    else return 0;
}

float llama_string_evalPrompt(const llama_timings & timings) {
    return (1e3 / timings.t_p_eval_ms * timings.n_p_eval);
}

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
//CLASS////////////////////////////////////////////////////////////////////////////////////////////

class chat
{
private:
	
    llama_context * ctx;
    llama_model * model;
    llama_context * ctx_guidance = NULL;
    int n_ctx;
    std::vector<llama_token> session_tokens;
    size_t n_matching_session_tokens = 0;
    std::vector<llama_token> embd_inp;
    std::string path_session = "";
    //std::vector<llama_token> last_n_tokens;
    std::vector<llama_token> last_tokens;
    std::vector<llama_token> guidance_inp;
    std::vector<llama_token> embd_guidance;
    //grammar_parser::parse_state parsed_grammar;
    //struct llama_grammar * grammar = NULL;
    struct llama_sampling_context * ctx_sampling;
    std::vector<llama_token_data> candidates;
    //llama_sampling_context ctx_sampling;
    //llama_sampling_params & sparams;
    std::vector<std::vector<llama_token>> antiprompt_ids;
    int n_vocab;
    bool cleared              = true;
        
    
    bool input_echo           = true;
    bool need_to_save_session = false;
    bool debug                = false;
    bool is_antiprompt        = false;
    bool streaming            = true;
    bool saveToFile           = false;
    
    
    

    int n_past                = 0;
    int n_remain              = 0;
    int n_consumed            = 0;
    int n_session_consumed    = 0;
    int n_past_guidance       = 0;
    int guidance_offset       = 0;
    int original_prompt_len   = 0;
    int n_last_message        = 0;
    int n_last_message_past   = 0;
    int t_eval_ms             = 0;
    int n_eval                = 0;
    int t_p_eval_ms           = 0;
    int n_p_eval              = 0;
    // since params.n_keep changes
    int n_keep                = 0;
    
    int ga_i = 0;

    int ga_n;
    int ga_w;
    
    std::vector<llama_token> embd;
    std::vector<llama_token> embd_msg;
    
    std::vector<int> inp_pfx;
    std::vector<int> inp_sfx;
       
    std::vector<int> llama_token_newline;
    
    std::vector<int>   input_tokens;
    std::vector<int>   output_tokens;
    std::ostringstream output_ss; 
    
public:
    gpt_params params;
    bool tempFirst           = true;
    bool finished            = true;
    int n_past_last           = 0;
    int n_remain_last         = 0;
    int n_consumed_last       = 0;
    int n_embd_inp_last       = 0;

    // experimenting with simple dynamic paramenters
    float d_temp_min = 1.8;
    float d_temp_max = 5;
    float d_temp_add = 0.1f;
    float d_temp_mul = 1.0f;
    bool d_temp_up = true;

    std::string formatRepresentation;

    //std::map<std::string,std::string> stats;

    chat(int argc, char ** argv){
        init(argc, argv);
    }
    
    chat(){}
    
    chat(bool useJson){
        init(useJson);
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
        guidance_inp.clear();
        embd_guidance.clear();
        embd.clear();
        inp_pfx.clear();
        inp_sfx.clear();
        llama_token_newline.clear();
        candidates.clear();
        //resetCTX();
        
        n_past                = 0;
        n_remain              = 0;
        n_consumed            = 0;
        n_session_consumed    = 0;
        n_past_guidance       = 0;
        guidance_offset       = 0;
        original_prompt_len   = 0;
        t_eval_ms             = 0;
        n_eval                = 0;
        t_p_eval_ms           = 0;
        n_p_eval              = 0;
        n_keep                = 0;
        n_past_last           = 0;
        n_remain_last         = 0;
        
       
        
        params = paramsDefault;
    }
    
    void clear(){
        if (!cleared){
            cleared = true;
            
            params.n_keep = n_keep;
            
            llama_free(ctx);
            if (ctx_guidance) { llama_free(ctx_guidance); }
            llama_free_model(model);
            //if (grammar != NULL) {
            //    llama_grammar_free(grammar);
            //}
            llama_sampling_free(ctx_sampling);
            llama_backend_free();
            
            clearSoft();
            
// #ifdef GGML_USE_VULKAN
            // ggml_vk_free_cpu_assist();
// #endif
        }
        
    }
    
    void switch_debug() {
        debug = !debug;
    }
    
    void print_timings() {
        //llama_print_timings(ctx);
        std::cout << llama_string_timings(ctx) << std::endl;
    }
    
    int getLastTokens() {
        return n_last_message;
    }
    
    int getRemainTokens() {
        return n_remain;
    } 

    int getConsumedTokens(){
        return n_consumed;
    }

    int getPastTokens(){
        return n_past;
    }
    
    int getEmbInpSize() {
        return embd_inp.size();
    }
    
    std::string get_timings(){
        //llama_print_timings(ctx);
        return llama_string_timings(ctx);
    }
    
    std::string get_timings_simple(){
        //llama_print_timings(ctx);
        return llama_string_timings_simple(ctx);
    }
    
    std::string get_eval(){
        //llama_print_timings(ctx);
        return std::to_string(llama_string_eval(ctx)) + " t/s";
    }
    
    float get_speed() {
        const llama_timings timings = llama_get_timings(ctx);
        
        t_eval_ms = timings.t_eval_ms - t_eval_ms;
        n_eval = timings.n_eval - n_eval;
        
        if (t_eval_ms != 0 && n_eval != 0) return (1e3 / t_eval_ms * n_eval);
        else return 0;
        //return llama_string_eval(ctx);
    }
    
    float get_speed_p(){
        const llama_timings timings = llama_get_timings(ctx);
        
        t_p_eval_ms = timings.t_p_eval_ms - t_p_eval_ms;
        n_p_eval = timings.n_p_eval - n_p_eval;
        
        if (t_p_eval_ms != 0 && n_p_eval != 0) return (1e3 / t_p_eval_ms * n_p_eval);
        else return 0;
        //return llama_string_eval(ctx);
    }
    
    void clear_speed(){
        t_eval_ms = 0;
        n_eval = 0;
    }
    
    std::string get_evalPrompt(){
        //llama_print_timings(ctx);
        return std::to_string(llama_string_evalPrompt(ctx)) + " t/s";
    }
    
    std::string get_ts(){
        std::string result = "Evals: \n";
        const llama_timings timings = llama_get_timings(ctx);
        result += "Prompts: " + std::to_string(llama_string_evalPrompt(timings)) + " t/s; \n";
        result += "Generation: " + std::to_string(llama_string_eval(timings)) + " t/s; \n";
        
        return result;
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
        
        if (gpt_params_parse(argc, argv, params) == false) {
            return -1;
        }
    }
    
    std::string getSparams(){
        return "\n top_k = " + std::to_string(params.sparams.top_k) + "\n top_p = " + std::to_string(params.sparams.top_p) + "\n min_p = " + std::to_string(params.sparams.min_p) + "\n tfs_z = " + std::to_string(params.sparams.tfs_z) + "\n typical_p = " + std::to_string(params.sparams.typical_p) + "\n temp = " + std::to_string(params.sparams.temp) + "\n penalty_repeat = " + std::to_string(params.sparams.penalty_repeat) + "\n penalty_freq = " + std::to_string(params.sparams.penalty_freq) + "\n penalty_present = " + std::to_string(params.sparams.penalty_present) + "\n mirostat = " + std::to_string(params.sparams.mirostat) + "\n mirostat_tau = " + std::to_string(params.sparams.mirostat_tau) + "\n mirostat_eta = " + std::to_string(params.sparams.mirostat_eta);
    }
    
    std::string getSparamsChanged(bool fullnames = true){
        std::string result = fullnames ? "logits" : "@";

        std::string name_penalty_repeat = fullnames ? "penalty_repeat" : "p_r";
        std::string name_penalty_threshold = fullnames ? "penalty_threshold" : "p_t";
        std::string name_penalty_freq = fullnames ? "penalty_freq" : "p_f";
        std::string name_penalty_present = fullnames ? "penalty_present" : "p_p";
        //DRY
        std::string name_dry_multiplier = fullnames ? "dry_multiplier" : "d_m";
        std::string name_dry_base = fullnames ? "dry_base" : "d_b";
        std::string name_dry_allowed_length = fullnames ? "dry_allowed_length" : "d_l";
        std::string name_dry_penalty_last_n = fullnames ? "dry_penalty_last_n" : "d_n";

        std::string name_temp = fullnames ? "temp" : "T";
        std::string name_dynatemp_range = fullnames ? "dynatemp_range" : "dT";

        std::string name_mirostat = fullnames ? "mirostat" : "I";
        std::string name_mirostat_tau = fullnames ? "mirostat_tau" : "I_t";
        std::string name_mirostat_eta = fullnames ? "mirostat_eta" : "I_e";

        std::string name_top_k = fullnames ? "top_k" : "K";
        std::string name_tfs_z = fullnames ? "tfs_z" : "F";
        std::string name_typical_p = fullnames ? "typical_p" : "Y";
        std::string name_p_step = fullnames ? "p_step" : "S";
        std::string name_xtc_probability = fullnames ? "xtc_probability" : "x_p";
        std::string name_xtc_threshold = fullnames ? "xtc_threshold" : "x_t";
        std::string name_top_p = fullnames ? "top_p" : "P";
        std::string name_min_p = fullnames ? "min_p" : "M";

        if (params.sparams.penalty_repeat != paramsDefault.sparams.penalty_repeat) result += std::format("-> {}={:.3f}", name_penalty_repeat, params.sparams.penalty_repeat);
        if (params.sparams.penalty_threshold != paramsDefault.sparams.penalty_threshold) result += std::format("-> {}={:.3f}", name_penalty_threshold, params.sparams.penalty_threshold); 
        if (params.sparams.penalty_freq != paramsDefault.sparams.penalty_freq) result += std::format("-> {}={:.3f}", name_penalty_freq, params.sparams.penalty_freq);
        if (params.sparams.penalty_present != paramsDefault.sparams.penalty_present) result += std::format("-> {}={:.3f}", name_penalty_present, params.sparams.penalty_present);
        //DRY
        if (params.sparams.dry_multiplier != paramsDefault.sparams.dry_multiplier) result += std::format("-> {}={:.3f}", name_dry_multiplier, params.sparams.dry_multiplier);
        if (params.sparams.dry_base != paramsDefault.sparams.dry_base) result += std::format("-> {}={:.3f}", name_dry_base, params.sparams.dry_base); 
        if (params.sparams.dry_allowed_length != paramsDefault.sparams.dry_allowed_length) result += std::format("-> {}={}", name_dry_allowed_length, params.sparams.dry_allowed_length);
        if (params.sparams.dry_penalty_last_n != paramsDefault.sparams.dry_penalty_last_n) result += std::format("-> {}={}", name_dry_penalty_last_n, params.sparams.dry_penalty_last_n);
        // mirostat is special 
        if (params.sparams.mirostat != paramsDefault.sparams.mirostat) {
            if (params.sparams.dynatemp_range > 0) {
                result += std::format("->{}({:.3f}-{:.3f})",name_dynatemp_range,params.sparams.temp > params.sparams.dynatemp_range ? params.sparams.temp - params.sparams.dynatemp_range : 0, params.sparams.temp + params.sparams.dynatemp_range);
                if (params.sparams.smoothing_factor != paramsDefault.sparams.smoothing_factor) result += std::format("/{:.2f}*{:.2f}", params.sparams.smoothing_factor, params.sparams.smoothing_curve);
            } else {
                result += name_temp; 
                if (params.sparams.temp != paramsDefault.sparams.temp) result += std::format("={:.3f}",params.sparams.temp); 
                result += std::format("/{:.3f}*{:.3f}", params.sparams.smoothing_factor, params.sparams.smoothing_curve);
            }
            result += "-> " + name_mirostat + " = " + std::to_string(params.sparams.mirostat); 
            result += std::format(";{}={:.3f}", name_mirostat_tau, params.sparams.mirostat_tau); 
            result += std::format(";{}={:.3f}", name_mirostat_eta, params.sparams.mirostat_eta);
        } else {
            for (auto s : params.sparams.samplers_sequence){
                result += "->";
                switch (s){
                    case 'k': result += name_top_k; if (params.sparams.top_k != paramsDefault.sparams.top_k) result += std::format("={}",params.sparams.top_k); break;
                    case 'f': result += name_tfs_z; if (params.sparams.tfs_z != paramsDefault.sparams.tfs_z) result += std::format("={:.3f}",params.sparams.tfs_z); break;
                    case 'y': result += name_typical_p; if (params.sparams.typical_p != paramsDefault.sparams.typical_p) result += std::format("={:.3f}",params.sparams.typical_p); break;
                    case 's': result += name_p_step; if (params.sparams.p_step != paramsDefault.sparams.p_step) result += std::format("={:.3f}",params.sparams.p_step); break;
                    case 'x': result += std::format("xtc={:.3f}-{:.03f}%",params.sparams.xtc_threshold,params.sparams.xtc_probability); break;
                    case 'p': result += name_top_p; if (params.sparams.top_p != paramsDefault.sparams.top_p) result += std::format("={:.3f}",params.sparams.top_p); break;
                    case 'm': result += name_min_p; if (params.sparams.min_p != paramsDefault.sparams.min_p) result += std::format("={:.3f}",params.sparams.min_p); break;
                    case 't': {
                            if (params.sparams.dynatemp_range > 0) {
                                result += std::format("{}({:.3f}-{:.3f})",name_dynatemp_range, params.sparams.temp > params.sparams.dynatemp_range ? params.sparams.temp - params.sparams.dynatemp_range : 0, params.sparams.temp + params.sparams.dynatemp_range);
                                if (params.sparams.smoothing_curve != paramsDefault.sparams.smoothing_curve) result += std::format("*{:.3f}", params.sparams.smoothing_curve);
                            } else {
                                result += name_temp;
                                if (params.sparams.temp != paramsDefault.sparams.temp) result += std::format("={:.3f}",params.sparams.temp);
                                // smoothing_curve doesn't work without dynatemp anyway
                            }
                            if (params.sparams.smoothing_factor != paramsDefault.sparams.smoothing_factor) result += std::format("^{:.3f}", params.sparams.smoothing_factor);
                            break;
                        }
                    default : break;
                }
            }
        }

        return result;
    }

    int init(int argc, char ** argv){
        // this is unsafe to run in threads;
        getArgs(argc, argv);
        
        printf("\n LOADING \n");
        
        return init();
    } 

    int init(bool useJson = true, bool headless = false){

        if (useJson) readParamsFromFile("config.json", params, headless);
        
        return load();
    }

    int init(nlohmann::json configJson){

        readParamsFromJson(configJson, params);
        
        return load();
    }   

    int init(std::string modelName, std::string prompt = "NULL", std::string antiprompt = "NULL"){

        if (modelName != "NULL") readParamsFromFile("config.json", modelName, params);
        else readParamsFromFile("config.json", params);
        
        if (prompt != "NULL") params.prompt = prompt;
        if (antiprompt != "NULL") {
            if(!params.antiprompt.size()) 
                params.antiprompt.emplace_back(antiprompt);
            else
                params.antiprompt[0] = antiprompt;
        }
        
        return load();
    }

    int initialize(nlohmann::json configJson, bool streamed = true, bool soft = false){

        //int result = init(configJson);
        readParamsFromJson(configJson, params);
        // if (configJson.contains("temp_first")) {
            // tempFirst = configJson["temp_first"];
            // printf("Temperature sampling first = %d\n", tempFirst);
        // }


        //chatFormat.sequence = params.format;
        //printf("Format = %s\n", params.format.c_str());
        formatRepresentation = "Format:\n";

        int result = load(soft);

        process_prompt(streamed);
        return result;
    }

    void formatInput(std::string format, std::string& buffer) {
        
        for (auto s : format){
            switch (s){
                case 'a':{
                    if (params.antiprompt.size()){
                        formatRepresentation += params.antiprompt[0];
                        const auto line_antiprompt_format = ::llama_tokenize(ctx, params.antiprompt[0], false, true);
                        embd_inp.insert(embd_inp.end(), line_antiprompt_format.begin(), line_antiprompt_format.end());
                    }
                    break;
                }
                case 'b':{
                    formatRepresentation += params.bos;
                    const auto line_bos_format = ::llama_tokenize(ctx, params.bos, false, true);
                    embd_inp.insert(embd_inp.end(), line_bos_format.begin(), line_bos_format.end());
                    break;
                }
                case 'e':{
                    formatRepresentation += params.eos;
                    const auto line_eos_format = ::llama_tokenize(ctx, params.eos, false, true);
                    embd_inp.insert(embd_inp.end(), line_eos_format.begin(), line_eos_format.end());
                    break;
                }
                case 'p':{
                    formatRepresentation += params.input_prefix;
                    const auto line_pfx = ::llama_tokenize(ctx, params.input_prefix, false, true);
                    embd_inp.insert(embd_inp.end(), line_pfx.begin(), line_pfx.end());
                    break;
                }
                case 'i':{
                    /* if (format == params.format_instruct) */ formatRepresentation += buffer;
                    //else formatRepresentation += "{input}";
                    const auto line_inp = ::llama_tokenize(ctx, buffer, false, false);
                    embd_inp.insert(embd_inp.end(), line_inp.begin(), line_inp.end());
                    if (n_remain > 0) n_remain -= line_inp.size();
                    break;
                }
                case 's':{
                    formatRepresentation += params.input_suffix;
                    const auto line_sfx = ::llama_tokenize(ctx, params.input_suffix, false, true);
                    embd_inp.insert(embd_inp.end(), line_sfx.begin(), line_sfx.end());
                    break;
                }
                case 'd':{
                    formatRepresentation += "\n";
                    const auto line_delim = ::llama_tokenize(ctx, "\n", false, true);
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
        if (params.seed == LLAMA_DEFAULT_SEED) {
            //params.seed = time(NULL);
            // std::srand(std::time(nullptr));
            // if (std::rand() > std::rand()){
                // params.seed = std::rand()*std::rand();
            // } else params.seed = std::rand();
            
            params.seed = getRand();
        }

        //fprintf(stderr, "%s: seed  = %d\n", __func__, params.seed);

        std::mt19937 rng(params.seed);
        if (params.random_prompt) {
            params.prompt = gpt_random_prompt(rng);
        }
        
        return 2;
    }
    
    void tokenize_antiprompt() {
        // tokenized antiprompts

        antiprompt_ids.reserve(params.antiprompt.size());
        for (const std::string & antiprompt : params.antiprompt) {
            antiprompt_ids.emplace_back(::llama_tokenize(ctx, antiprompt, false, true));
        }
    }

    int check_encoder() {
        if (llama_model_has_encoder(model)) {
            int enc_input_size = embd_inp.size();
            llama_token * enc_input_buf = embd_inp.data();

            if (llama_encode(ctx, llama_batch_get_one(enc_input_buf, enc_input_size, 0, 0))) {
                return 1;
            }

            llama_token decoder_start_token_id = llama_model_decoder_start_token(model);
            if (decoder_start_token_id == -1) {
                decoder_start_token_id = llama_token_bos(model);
            }

            embd_inp.clear();
            //embd_inp.push_back(decoder_start_token_id);
            embd_inp.emplace_back(decoder_start_token_id);
        }

        return 0;
    }

    int loadModel(){

        //llama_init_backend(params.numa);
        llama_backend_init();
        printf("..............Backend initialized simple................\n");

        // load the model and apply lora adapter, if any
        //ctx = llama_init_from_gpt_params(params);
        llama_init_result llama_init = llama_init_from_gpt_params(params);

        model = llama_init.model;
        ctx = llama_init.context;
        printf("..............Model initialized................\n");
        
        
        if (model == NULL) {
            fprintf(stderr, "%s: error: unable to load model\n", __func__);
            return 0;
        }
        
        return 3;
    }
    
    // this part is necessary for proper functioning
    int strictLoad(){
        int status = 0;
        
        //checking values 
        status += checkPreLoad(); // 1
        printf("checkPreLoad = %d\n", status);
        if (status == 0) return 0;
        
        //loading the model itself; uses llama_backend, ctx, ctx_guidance
        //status += loadModel(); // +3
        // LET'S TRY THIS
        //llama_init_backend(params.numa);
        llama_backend_init();
        printf("..............Backend initialized strict................\n");

        // load the model and apply lora adapter, if any
        //ctx = llama_init_from_gpt_params(params);
        llama_init_result llama_init = llama_init_from_gpt_params(params);

        model = llama_init.model;
        ctx = llama_init.context;
        printf("..............Model initialized................\n");
        
        
        if (model == NULL) {
            fprintf(stderr, "%s: error: unable to load model\n", __func__);
            return 0;
        }
        
        status += 3; // +3
        
        
        printf("loadModel = %d\n", status);
        if (status < 3) return 0;

        // setting seed
        status += setRandomSeed(); // 6
        printf("setRandomSeed = %d\n", status);
        
        return status;
    }
    
    int load(bool soft = false){
        
        printf("Load start \n");
        
        llama_sampling_params & sparams = params.sparams;
        
        
        
        
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
            printf("..............Backend initialized common................\n");
#endif
            // load the model and apply lora adapter, if any
            llama_init_result llama_init = llama_init_from_gpt_params(params);

            model = llama_init.model;
            ctx = llama_init.context;
            printf("..............Model initialized................\n");
            if (sparams.cfg_scale > 1.f) {
                struct llama_context_params lparams = llama_context_params_from_gpt_params(params);
                ctx_guidance = llama_new_context_with_model(model, lparams);
            }
            
            
            if (model == NULL) {
                fprintf(stderr, "%s: error: unable to load model\n", __func__);
                return 0;
            }
            
            status += 3; // +3
            
            
            printf("loadModel = %d\n", status);
            if (status < 3) return 0;

            // setting seed
            status += setRandomSeed();
            
            if (status == 0) return 0;
        }
        
        cleared = false;
        n_keep = params.n_keep;

        printf("llama_n_ctx \n");
        
        n_ctx = llama_n_ctx(ctx);
        
        ga_n = params.grp_attn_n;
        ga_w = params.grp_attn_w;

        if (ga_n != 1) {
            GGML_ASSERT(ga_n > 0                    && "grp_attn_n must be positive");                     // NOLINT
            GGML_ASSERT(ga_w % ga_n == 0            && "grp_attn_w must be a multiple of grp_attn_n");     // NOLINT
        }


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
                llama_set_rng_seed(ctx, params.seed);

                fprintf(stderr, "%s: loaded a session with prompt size of %d tokens\n", __func__, (int) session_tokens.size());
            } else {
                fprintf(stderr, "%s: session file does not exist, will create\n", __func__);
            }
        }

        // in instruct mode, we inject a prefix and a suffix to each input by the user
        // we are in a permanent dialog mode 
        params.interactive_first = true;
        params.interactive = true;

        if (!params.antiprompt.size()) params.antiprompt.emplace_back("You:");
        //if (params.prompt.empty()) params.prompt = params.antiprompt.back();
        // instruct mode was removed since its format is not universal enough

        const bool add_bos = llama_add_bos_token(model);
        if (!llama_model_has_encoder(model)) {
            GGML_ASSERT(!llama_add_eos_token(model));
        }
        printf("add_bos: %d\n", add_bos);

        if (params.interactive_first || !params.prompt.empty() || session_tokens.empty()) {
            //this is the first problem we have
            // there is no formatting for the initial prompt
            //embd_inp = ::llama_tokenize(ctx, params.prompt, add_bos, true);
            formatInput(params.format_instruct, params.prompt);
        } else {
            embd_inp = session_tokens;
        }

        if (embd_inp.empty()) {
            embd_inp.emplace_back(llama_token_bos(model));
            //embd_inp.push_back(llama_token_bos(model));
        }

        // Tokenize negative prompt

        if (ctx_guidance) {
            //params.cfg_negative_prompt.insert(0, 1, ' ');
            guidance_inp = ::llama_tokenize(ctx_guidance, sparams.cfg_negative_prompt, add_bos, true);

            std::vector<llama_token> original_inp = ::llama_tokenize(ctx, params.prompt, add_bos, true);
            original_prompt_len = original_inp.size();
            guidance_offset = (int)guidance_inp.size() - original_prompt_len;
        }

        if ((int) embd_inp.size() > n_ctx - 4) {
            fprintf(stderr, "%s: error: prompt is too long (%d tokens, max %d)\n", __func__, (int) embd_inp.size(), n_ctx - 4);
            return 1;
        }

        // debug message about similarity of saved session, if applicable
        n_matching_session_tokens = 0;
        if (session_tokens.size()) {
            for (llama_token id : session_tokens) {
                if (n_matching_session_tokens >= embd_inp.size() || id != embd_inp[n_matching_session_tokens]) {
                    break;
                }
                n_matching_session_tokens++;
            }
            if (params.prompt.empty() && n_matching_session_tokens == embd_inp.size()) {
                fprintf(stderr, "%s: using full prompt from session file\n", __func__);
            } else if (n_matching_session_tokens >= embd_inp.size()) {
                fprintf(stderr, "%s: session file has exact match for prompt!\n", __func__);
            } else if (n_matching_session_tokens < (embd_inp.size() / 2)) {
                fprintf(stderr, "%s: warning: session file has low similarity to prompt (%zu / %zu tokens); will mostly be reevaluated\n",
                    __func__, n_matching_session_tokens, embd_inp.size());
            } else {
                fprintf(stderr, "%s: session file matches %zu / %zu tokens of prompt\n",
                    __func__, n_matching_session_tokens, embd_inp.size());
            }

            // remove any "future" tokens that we might have inherited from the previous session
            //llama_kv_cache_tokens_rm(ctx, n_matching_session_tokens, -1);
            llama_kv_cache_seq_rm(ctx, -1, n_matching_session_tokens, -1);
        }

        // if we will use the cache for the full prompt without reaching the end of the cache, force
        // reevaluation of the last token token to recalculate the cached logits
        if (!embd_inp.empty() && n_matching_session_tokens == embd_inp.size() &&
                session_tokens.size() > embd_inp.size()) {
            session_tokens.resize(embd_inp.size() - 1);
        }

        // number of tokens to keep when resetting context
        if (params.n_keep < 0 || params.n_keep > (int) embd_inp.size()) {
            params.n_keep = (int)embd_inp.size();
        } else {
            params.n_keep += add_bos; // always keep the BOS token
        }

        
        fprintf(stderr, "\nsampling: repeat_last_n = %d, penalty_repeat = %f, penalty_present = %f, penalty_freq = %f, top_k = %d, tfs_z = %f, top_p = %f, typical_p = %f, temp = %f, mirostat = %d, mirostat_lr = %f, mirostat_ent = %f\n",
                sparams.penalty_last_n, sparams.penalty_repeat, sparams.penalty_present, sparams.penalty_freq, sparams.top_k, sparams.tfs_z, sparams.top_p, sparams.typical_p, sparams.temp, sparams.mirostat, sparams.mirostat_eta, sparams.mirostat_tau);

        if (params.interactive) {
            const char *control_message;

            control_message = " - Press Return to return control to LLaMa.\n"
                              " - To return control without starting a new line, end your input with '/'.\n"
                              " - If you want to submit another line, end your input with '\\'.\n";

            is_interacting = params.interactive_first;
        }

        //ctx_sampling = llama_sampling_init(params);

        bool need_to_save_session = !path_session.empty() && n_matching_session_tokens < embd_inp.size();

        n_remain           = params.n_predict;
        n_remain_last      = embd_inp.size();


        printf("ctx_sampling init\n");

        ctx_sampling = llama_sampling_init(params.sparams);

        printf("Load finished\n");

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
        if ((int)embd.size() > max_embd_size) {
            const int skipped_tokens = (int) embd.size() - max_embd_size;
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
        if (n_past + (int) embd.size() + std::max<int>(0, guidance_offset) >= n_ctx) {
            printf("....");
            const int n_left    = n_past - params.n_keep;
            const int n_discard = n_left/2;

            // always keep the first token - BOS
            //n_past = std::max(1, params.n_keep);
            //n_past_guidance = std::max(1, params.n_keep + guidance_offset);
            llama_kv_cache_seq_rm   (ctx, 0, params.n_keep            , params.n_keep + n_discard);
            llama_kv_cache_seq_add(ctx, 0, params.n_keep + n_discard, n_past, -n_discard);

            // insert n_left/2 tokens at the start of embd from last_n_tokens
            //embd.insert(embd.begin(), last_n_tokens.begin() + n_ctx - n_left/2 - embd.size(), last_n_tokens.end() - embd.size());
            //embd.insert(embd.begin(), last_tokens.begin() + n_ctx - n_left/2 - embd.size(), last_tokens.end() - embd.size());
            n_past -= n_discard;
            n_last_message_past -= n_discard;

            if (ctx_guidance) {
                n_past_guidance -= n_discard;
            }

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
            if (n_past + (int) embd.size() + std::max<int>(0, guidance_offset) >= n_ctx) {
                // if (params.n_predict == -2) {
                    // printf("\n\n%s: context full and n_predict == -%d => stopping\n", __func__, params.n_predict);
                    // break;
                // }
                
                const int n_left    = n_past - params.n_keep;
                const int n_discard = n_left/2;
                
                llama_kv_cache_seq_rm   (ctx, 0, params.n_keep            , params.n_keep + n_discard);
                llama_kv_cache_seq_add(ctx, 0, params.n_keep + n_discard, n_past, -n_discard);
                
                n_past -= n_discard;
                n_last_message_past -= n_discard;
                
                if (ctx_guidance) {
                        n_past_guidance -= n_discard;
                    }

                path_session.clear();
            }
        } else {
            // context extension via Self-Extend
            while (n_past >= ga_i + ga_w) {
                const int ib = (ga_n*ga_i)/ga_w;
                const int bd = (ga_w/ga_n)*(ga_n - 1);
                const int dd = (ga_w/ga_n) - ib*bd - ga_w;
                
                llama_kv_cache_seq_add(ctx, 0, ga_i,                n_past,              ib*bd);
                llama_kv_cache_seq_div  (ctx, 0, ga_i + ib*bd,        ga_i + ib*bd + ga_w, ga_n);
                llama_kv_cache_seq_add(ctx, 0, ga_i + ib*bd + ga_w, n_past + ib*bd,      dd);
                
                n_past -= bd;
                n_last_message_past -= bd;

                ga_i += ga_w/ga_n;
            }
        }
    }
    
    int reuse() {
        // try to reuse a matching prefix from the loaded session instead of re-eval (via n_past)
        if (n_session_consumed < (int) session_tokens.size()) {
            size_t i = 0;
            for ( ; i < embd.size(); i++) {
                if (embd[i] != session_tokens[n_session_consumed]) {
                    session_tokens.resize(n_session_consumed);
                    return 0;
                }

                n_past++;
                //n_past_last = n_past;
                n_session_consumed++;

                if (n_session_consumed >= (int) session_tokens.size()) {
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
    
    int evaluate_guidance(){
        if (ctx_guidance) {
            int input_size = 0;
            llama_token* input_buf = NULL;

            if (n_past_guidance < (int) guidance_inp.size()) {
                // Guidance context should have the same data with these modifications:
                //
                // * Replace the initial prompt
                // * Shift everything by guidance_offset
                embd_guidance = guidance_inp;
                if (embd.begin() + original_prompt_len < embd.end()) {
                    embd_guidance.insert(
                        embd_guidance.end(),
                        embd.begin() + original_prompt_len,
                        embd.end()
                    );
                }

                input_buf = embd_guidance.data();
                input_size = embd_guidance.size();

            } else {
                input_buf = embd.data();
                input_size = embd.size();
            }

            for (int i = 0; i < input_size; i += params.n_batch) {
                int n_eval = std::min(input_size - i, params.n_batch);
                if (llama_decode(ctx_guidance, llama_batch_get_one(input_buf + i, n_eval, n_past_guidance, 0))) {
                    fprintf(stderr, "%s : failed to eval\n", __func__);
                    return 0;
                }

                n_past_guidance += n_eval;
            }
        }
        
        return 1;
    }
    
    int evaluate_main() {
        for (int i = 0; i < (int) embd.size(); i += params.n_batch) {
            int n_eval = (int) embd.size() - i;
            if (n_eval > params.n_batch) {
                n_eval = params.n_batch;
            }
            if (llama_decode(ctx, llama_batch_get_one(&embd[i], n_eval, n_past, 0))) {
                fprintf(stderr, "%s : failed to eval\n", __func__);
                std::string report = std::format("evaluate_main(): failed to eval: {}\n{}\n Failed: {}", params.model, params.prompt,embd[i]);
                writeTextFile(std::to_string(getRand()) + ".txt", report);
                return 1;
            }
            n_past += n_eval;
            //n_past_last = n_past;
        }
        
        return 1;
    }
    
    void evaluate_session() {
        if (embd.size() > 0 && !path_session.empty()) {
            session_tokens.insert(session_tokens.end(), embd.begin(), embd.end());
            n_session_consumed = session_tokens.size();
        }
    }
    
    void check_antiprompt_tkns() {
        // check for reverse prompt using special tokens
        llama_token last_token = llama_sampling_last(ctx_sampling);
        for (std::vector<llama_token> ids : antiprompt_ids) {
            if (ids.size() == 1 && last_token == ids[0]) {
                if (params.interactive) {
                    is_interacting = true;
                }
                is_antiprompt = true;
                break;
            }
        }
    }
    
    //checking already existing contex
    int checkEmb(){
        if (debug) printf("-ce");
        //log_down("checkEmb\n", params.seed);
        //fprintf(stderr, "1");
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
        
        if (evaluate_guidance() == 0) return 0;
        
        if (evaluate_main() == 0) return 0;

        evaluate_session();
        
        return 1;
    }
    
    // main generation, includes adding antiprompt at the end
    int applyEmb(bool fastStop = false) {
        if (debug) printf("-ae");
        //log_down("applyEmb\n", params.seed);
        //fprintf(stderr, "2");

        // optionally save the session on first sample (for faster prompt loading next time)
        if (!path_session.empty() && need_to_save_session && !params.prompt_cache_ro) {
            need_to_save_session = false;
            llama_state_save_file(ctx, path_session.c_str(), session_tokens.data(), session_tokens.size());
        }

        // new generation function, moved to common 
        const llama_token id = llama_sampling_sample(ctx_sampling, ctx, ctx_guidance);
        //const llama_token id = llama_sampling_sample_choice(ctx_sampling, ctx, ctx_guidance, tempFirst);

        //last_tokens.erase(last_tokens.begin());
        //last_tokens.emplace_back(id);
        llama_sampling_accept(ctx_sampling, ctx, id, true);

        // add it to the context
        //embd.emplace_back(id);
        embd.emplace_back(id);
        //embd.push_back(id);
        ++n_last_message;
        ++n_last_message_past;

        // echo this to console
        input_echo = true;

        // decrement remaining sampling budget
        if (n_remain > 0) --n_remain;
        
        return 1;
    }
    
// main generation end////////////////////////////////////////////////////////////////

// additional functions
    void clearLastTokens() {
        n_last_message = 0;
        n_last_message_past = 0;
    }
    
    void capture_states() {
        n_past_last = n_past;
        n_embd_inp_last = embd_inp.size();
        n_consumed_last = n_consumed;
    }


    void clear_last() {
        // n_past = n_past_last + 1;
        // llama_kv_cache_seq_rm(ctx, 0, n_past, -1);
        // embd_inp.erase(embd_inp.begin() + n_consumed, embd_inp.end());
        //int comp = -1;
        int comp = 1;
        llama_sampling_rollback(ctx_sampling, n_last_message_past + comp);
        llama_kv_cache_seq_rm(ctx, 0, n_past - n_last_message_past, -1);
        embd_inp.erase(embd_inp.begin() + n_embd_inp_last + comp, embd_inp.end());
        //n_remain += last_tokens_count;
        n_past -= n_last_message_past;
        //n_past = n_past_last;
        //n_consumed = n_consumed_last;
        //embd.clear();
        //embd_guidance.clear();

        clearLastTokens();

        if (n_past > 0) {
            resetGrammar();
            is_interacting = false;
        }
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
            llama_sampling_reset(ctx_sampling);
        }
        
        //is_interacting = false;
        
        return 1;
    }
    
    int checkInfinite(){
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
        if (params.antiprompt.size()) {
            //std::string last_output;
            //for (auto id : last_n_tokens) {
            //for (auto id : last_tokens) {
            //for (auto id : ctx_sampling->prev) {
            //    last_output += llama_token_to_piece(ctx, id);
            //}
            const int n_prev = 32;
            const std::string last_output = llama_sampling_prev_str(ctx_sampling, ctx, n_prev);

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
    
    int setEOS(){
        // deal with end of text token in interactive mode
        //if (last_n_tokens.back() == llama_token_eos(ctx)) {
        //if (last_tokens.back() == llama_token_eos(ctx)) {
        //if (ctx_sampling->prev.back() == llama_token_eos(ctx)) {
        //if (llama_sampling_last(ctx_sampling) == llama_token_eos(model)) {
        if (llama_token_is_eog(model, llama_sampling_last(ctx_sampling))) {
            if (params.interactive) {
                if (params.antiprompt.size() != 0) {
                    // tokenize and inject first reverse prompt
                    const auto first_antiprompt = ::llama_tokenize(ctx, params.antiprompt.front(), false, true);
                    embd_inp.insert(embd_inp.end(), first_antiprompt.begin(), first_antiprompt.end());
                    is_antiprompt = true;
                }

                is_interacting = true;
                //printf("\n");
            }
        }
        
        return 1;
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
    
    
// assembled work on context with writing into a "result"////////////////////    
    int preEmb(std::string& result, bool& fastStop, bool streaming = false){ // 1 2 3 4
        //std::cout << " ** " << std::endl;
        
        if (embd.size() > 0) {
            checkEmb(); // 1
        }

        embd.clear();
        embd_guidance.clear();

        if ((int) embd_inp.size() <= n_consumed && !is_interacting) {
            applyEmb(); // 2
        } else {
            if (debug) printf("-pes");
            fastStop = true;
            // some user input remains from prompt or interaction, forward it to processing
            while ((int) embd_inp.size() > n_consumed) {
                //fprintf(stderr, ">");
                embd.emplace_back(embd_inp[n_consumed]);
                //embd.push_back(embd_inp[n_consumed]);
                //last_n_tokens.erase(last_n_tokens.begin());
                //last_n_tokens.emplace_back(embd_inp[n_consumed]);
                //last_tokens.erase(last_tokens.begin());
                //last_tokens.emplace_back(embd_inp[n_consumed]);
                
                // push the prompt in the sampling context in order to apply repetition penalties later
                // for the prompt, we don't apply grammar rules
                llama_sampling_accept(ctx_sampling, ctx, embd_inp[n_consumed], false);
                
                
                ++n_consumed;
                if ((int) embd.size() >= params.n_batch) {
                    break;
                }
            }
        }

        // display and return text
        
        
        if (input_echo) {
            //printf("-pei");
            for (auto id : embd) { 
                //std::string tknStr = llama_token_to_string(ctx, id); 
                const std::string tknStr = llama_token_to_piece(ctx, id); 
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
        llama_sampling_reset(ctx_sampling);
    }
    
    void appendPrefixBos(){
        if (params.input_prefix_bos) {
            embd_inp.emplace_back(llama_token_bos(model));
            //embd_inp.push_back(llama_token_bos(model));
        }
    }
    
    
    void classicProcessing(std::string& buffer) {
         //auto line_inp = ::llama_tokenize(ctx, buffer, false);
        //const auto line_inp = ::llama_tokenize(ctx, buffer, false);
        // const auto line_pfx = ::llama_tokenize(ctx, params.input_prefix, false, true);
        // const auto line_inp = ::llama_tokenize(ctx, buffer, false, false);
        // const auto line_sfx = ::llama_tokenize(ctx, params.input_suffix, false, true);
        //embd_inp.insert(embd_inp.end(), line_inp.begin(), line_inp.end());
        // embd_inp.insert(embd_inp.end(), line_pfx.begin(), line_pfx.end());
        
        // if (chatFormat.eos.first == "answer") embd_inp.insert(embd_inp.end(), line_eos_format.begin(), line_eos_format.end());
        // if (chatFormat.bos.first == "prompt") embd_inp.insert(embd_inp.end(), line_eos_format.begin(), line_bos_format.end());
        
        // embd_inp.insert(embd_inp.end(), line_inp.begin(), line_inp.end());
        
        // if (chatFormat.eos.first == "prompt") embd_inp.insert(embd_inp.end(), line_eos_format.begin(), line_eos_format.end());
        // if (chatFormat.bos.first == "answer") embd_inp.insert(embd_inp.end(), line_eos_format.begin(), line_bos_format.end());
        
       // embd_inp.insert(embd_inp.end(), line_sfx.begin(), line_sfx.end());
           
        const auto line_pfx = ::llama_tokenize(ctx, params.input_prefix, false, true);
        const auto line_inp = ::llama_tokenize(ctx, buffer, false, false);
        const auto line_sfx = ::llama_tokenize(ctx, params.input_suffix, false, true);
        
        embd_inp.insert(embd_inp.end(), line_pfx.begin(), line_pfx.end());
        embd_inp.insert(embd_inp.end(), line_inp.begin(), line_inp.end());
        embd_inp.insert(embd_inp.end(), line_sfx.begin(), line_sfx.end());
        
        if (n_remain > 0) n_remain -= line_inp.size();
    }
    
    int subInputNew(std::string& line){
        
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
            
            const size_t original_size = embd_inp.size();

            // instruct mode: insert instruction prefix
            // if (params.instruct && !is_antiprompt) {
                // n_consumed = embd_inp.size();
                // if (debug) printf("|");
                // embd_inp.insert(embd_inp.end(), inp_pfx.begin(), inp_pfx.end());
            // }

            if (params.escape) {
                process_escapes(buffer);
            }

            //classicProcessing(buffer);
            formatInput(params.format_dialog, buffer);
            //formatInput(buffer);

            for (size_t i = original_size; i < embd_inp.size(); ++i) {
                const llama_token token = embd_inp[i];
                output_tokens.emplace_back(token);
                //output_tokens.push_back(token);
                //output_ss << llama_token_to_piece(ctx, token);
                //std::cout << "tkns = " << embd_inp.size() << std::endl;
            }

            // n_remain -= line_inp.size();
        }

        //is_antiprompt = false;
        
        return 1;
    }
    
    void capture_lasts() {
        n_past_last = embd_inp.size();
        //n_remain_last = n_remain;
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

            if (consoleOutput) preEmb(result, fastStop, streaming); // includes checking, generation and adding prompt leftovers
            else preEmb(result, fastStop, consoleOutput);

            if ((int) embd_inp.size() <= n_consumed) {
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

    const std::string getToken(){
        if (debug) printf("-gp");
        
        for (auto id : embd) { 
            //return llama_token_to_string(ctx, id); 
            return llama_token_to_piece(ctx, id); 
        }
    }
    
    const std::string getToken(std::vector<llama_token>& embd_msg){
        if (debug) printf("-gp");
        
        for (auto id : embd_msg) { 
            //return llama_token_to_string(ctx, id); 
            return llama_token_to_piece(ctx, id); 
        }
    }
    
    int hasAntiprompt(std::string& result){
        if (params.antiprompt.size()) {
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
        if (params.antiprompt.size()) {
            int cutAntiPos = result.rfind(params.antiprompt[0]);
            if (cutAntiPos != std::string::npos){
                result.erase(cutAntiPos);
            }
        }
    }
    
    void eraseLast(std::string& result, std::string& stop){
        int cutAntiPos = result.rfind(stop);
        if (cutAntiPos != std::string::npos){
            result.erase(cutAntiPos);
        }
    }

    std::string getBitOld(){ // 1 2 3 4
        //std::cout << " ** " << std::endl;
        //log_down(std::format("processEmb: {} vs {}\n", embd_inp.size(), n_consumed), params.seed);
        if (embd.size() > 0) {
            checkEmb(); // 1
        }

        embd.clear();
        embd_guidance.clear();

        if ((int) embd_inp.size() <= n_consumed && !is_interacting) {
            applyEmb(); // 2
        } else {
            prepareEmb();
            // n_consumed_last = n_consumed;
            // n_embd_inp_last = embd_inp.size();
            capture_states();
        }

        // output text
        if (input_echo) {
            return getToken();
        }

        // if ((int) embd_inp.size() <= n_consumed) {
            // checkAntiprompt();
        // }

        return "";
    }

    void prepareEmb() {
        if (debug) printf("-pen");
        //log_down("preEmbSeparate \n", params.seed);
        //fastStop = true;
        
        // some user input remains from prompt or interaction, forward it to processing
        while ((int) embd_inp.size() > n_consumed) {
            //embd.push_back(embd_inp[n_consumed]);
            embd.emplace_back(embd_inp[n_consumed]);

            // GG: I'm not sure it's a good idea to push the prompt tokens into the sampling context
            //     Most likely will remove this in the future to avoid exposing "prev"
            //     Same thing is done in "server". If we stop pushing the prompt tokens, then the repetition
            //     penalty will be applied only based on the tokens generated by the model.
            ctx_sampling->prev.erase(ctx_sampling->prev.begin());
            //ctx_sampling->prev.push_back(embd_inp[n_consumed]);
            ctx_sampling->prev.emplace_back(embd_inp[n_consumed]);

            ++n_consumed;
            if ((int) embd.size() >= params.n_batch) {
                break;
            }
        }

        capture_lasts();
    }

    void prepareBeforeAnswer() {
        prepareEmb();
        capture_states();
    }

//input processing, which requires preemptive checking, adding prompt leftovers, antiprompt processing
    int inputOnlyNew(std::string& input){
        //std::cout << " ***** " << input << std::endl;
        embd_msg.clear();

        if (n_past > 0 && is_interacting) {

            subInputNew(input);
            input_echo = false; // do not echo this again        
        }

        if (n_past > 0) {
            resetGrammar();
            is_interacting = false;
            
        }

        // prepareEmb();
        // capture_states();
        prepareBeforeAnswer();

        formatRepresentation += "{output}";

        //checkInfinite();

        return 1;
    }

// this is an attempt to strictly separate all input-based preparations
// however, it assumes conditions (see in getBitOld())
// prepareEmb() and capture_states() should be done elsewhere
    std::string getBit() { // 1 2 3 4
        //std::cout << " ** " << std::endl;
        //log_down(std::format("processEmb: {} vs {}\n", embd_inp.size(), n_consumed), params.seed);

        if (embd.size() > 0) {
            checkEmb(); // 1
        }

        embd.clear();
        embd_guidance.clear();

        if (!is_interacting) applyEmb(); // 2

        return getToken();
    }

    // token by token generation and pushing
    std::string cycleStringsOnly(bool stream = false) {
        //std::cout << " * " << input << std::endl;
        //log_down("cycleStringsOnly\n", params.seed);
        std::string result;
        //bool fastStop = false;

        dynamic_params(params.sparams.temp, params.sparams.temp_func);
        dynamic_params(params.sparams.dynatemp_range, params.sparams.dynatemp_range_func);
        dynamic_params(params.sparams.p_step, params.sparams.p_step_func);
        //process_prompt(false);  // do not forget to include it elsewhere after loading the model  
        //inputOnly(input); // MOVED

        std::string bit = getBit();

        //if (stream || streaming) std::cout << bit;

        result += bit;

        if ((int) embd_inp.size() <= n_consumed) {
            if (debug) printf("-cso");
            //fprintf(stderr, "5");

            checkAntiprompt();

            setEOS();

            if (n_past > 0 && is_interacting) {
                //fprintf(stderr, "6");

                //eraseAntiprompt(result);
                finished = true;
                //log_down("cycleStringsOnly FINISHED\n", params.seed);
                //checkAntiprompt();
                //setEOS();

                return result;
            }

        }

        return result;
    }

    int save(std::string fileName, std::vector<char*>& results){
        if (saveToFile){
            writeTextFile(SESSIONS_FOLDER + fileName, results);
        }
        
        return 1;
    }

    
};

