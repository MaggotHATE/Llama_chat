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

#include "common.h"
//#include "sampling_plus.h"
#include "llama.h"

#include "jsonParams.h"

#include <cassert>
#include <cinttypes>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <format>

#if defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
#include <signal.h>
#include <unistd.h>
#define DELIMINER '\r'
#elif defined (_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define DELIMINER '\n'
#include <windows.h>
#include <signal.h>
#endif

#define SESSIONS_FOLDER "sessions/"

static gpt_params paramsDefault;

static bool is_interacting = false;

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
    int last_tokens_count     = 0;
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
    
    std::string formatRepresentation;
    
    //std::map<std::string,std::string> stats;

	chat(int argc, char ** argv){
		init(argc, argv);
	}
    
    chat(){}
    
    chat(bool useJson){
        init(useJson);
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
        }
        
    }
    
    void print_timings(){
        //llama_print_timings(ctx);
        std::cout << llama_string_timings(ctx) << std::endl;
    }
    
    int getLastTokens(){
        return last_tokens_count;
    }
    
    int getRemainTokens(){
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
    
    std::string getSparamsChanged(){
        std::string result = "logits ";
        
        if (params.sparams.penalty_repeat != paramsDefault.sparams.penalty_repeat) result += "-> penalty_repeat = " + std::to_string(params.sparams.penalty_repeat); 
        if (params.sparams.penalty_freq != paramsDefault.sparams.penalty_freq) result += "-> penalty_freq = " + std::to_string(params.sparams.penalty_freq); 
        if (params.sparams.penalty_present != paramsDefault.sparams.penalty_present) result += "-> penalty_present = " + std::to_string(params.sparams.penalty_present); 
        
        
        // mirostat is special 
        if (params.sparams.mirostat != paramsDefault.sparams.mirostat) {
            if (params.sparams.dynatemp_range > 0) {
                result += std::format("dynatemp_range ({:.2f} - {:.2f})",params.sparams.temp > params.sparams.dynatemp_range ? params.sparams.temp - params.sparams.dynatemp_range : 0, params.sparams.temp + params.sparams.dynatemp_range); 
            } else { 
                result += "temp "; 
                if (params.sparams.temp != paramsDefault.sparams.temp) result += std::format("= {:.2f}",params.sparams.temp); 
                result += std::format("/{:.2f}", params.sparams.temp_smoothing); 
            }
            result += "-> mirostat = " + std::to_string(params.sparams.mirostat); 
            result += std::format("; mirostat_tau =  {:.2f}", params.sparams.mirostat_tau); 
            result += std::format("; mirostat_eta = {:.2f}", params.sparams.mirostat_eta);
        } else {
            // if (params.sparams.top_k != paramsDefault.sparams.top_k) result += "\n top_k = " + std::to_string(params.sparams.top_k);
            // if (params.sparams.tfs_z != paramsDefault.sparams.tfs_z) result +="\n tfs_z = " + std::to_string(params.sparams.tfs_z); 
            // if (params.sparams.typical_p != paramsDefault.sparams.typical_p) result += "\n typical_p = " + std::to_string(params.sparams.typical_p); 
            // if (params.sparams.top_p != paramsDefault.sparams.top_p) result += "\n top_p = " + std::to_string(params.sparams.top_p);
            // if (params.sparams.min_p != paramsDefault.sparams.min_p) result += "\n min_p = " + std::to_string(params.sparams.min_p);
            for (auto s : params.sparams.samplers_sequence){
                result += "-> ";
                switch (s){
                    case 'k': result += "top_k "; if (params.sparams.top_k != paramsDefault.sparams.top_k) result += "= " + std::to_string(params.sparams.top_k); break;
                    case 'f': result += "tfs_z "; if (params.sparams.tfs_z != paramsDefault.sparams.tfs_z) result += std::format("= {:.2f}",params.sparams.tfs_z); break;
                    case 'y': result += "typical_p "; if (params.sparams.typical_p != paramsDefault.sparams.typical_p) result += std::format("= {:.2f}",params.sparams.typical_p); break;
                    case 'p': result += "top_p "; if (params.sparams.top_p != paramsDefault.sparams.top_p) result += std::format("= {:.2f}",params.sparams.top_p); break;
                    case 'm': result += "min_p "; if (params.sparams.min_p != paramsDefault.sparams.min_p) result += std::format("= {:.2f}",params.sparams.min_p); break;
                    case 't': {
                            if (params.sparams.dynatemp_range > 0) {
                                result += std::format("dynatemp_range ({:.2f} - {:.2f})",params.sparams.temp > params.sparams.dynatemp_range ? params.sparams.temp - params.sparams.dynatemp_range : 0, params.sparams.temp + params.sparams.dynatemp_range); 
                            } else { 
                                result += "temp "; 
                                if (params.sparams.temp != paramsDefault.sparams.temp) result += std::format("= {:.2f}",params.sparams.temp); 
                                result += std::format("/{:.2f}", params.sparams.temp_smoothing); 
                            }
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
                params.antiprompt.push_back(antiprompt);
            else
                params.antiprompt[0] = antiprompt;
        }
        
        return load();
    }
    
    int initGenerate(nlohmann::json configJson, bool streamed = true, bool soft = false){
        
        //int result = init(configJson);
        readParamsFromJson(configJson, params);
        if (configJson.contains("temp_first")) {
            tempFirst = configJson["temp_first"];
            printf("Temperature sampling first = %d\n", tempFirst);
        }
        
        
        //chatFormat.sequence = params.format;
        //printf("Format = %s\n", params.format.c_str());
        formatRepresentation = "Format:\n";
        
        int result = load(soft);
        
        generate(streamed);
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
                    n_remain -= line_inp.size();
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
    
    int loadModel(){

        //llama_init_backend(params.numa);
        llama_backend_init(params.numa);
        printf("..............Backend initialized................\n");

        // load the model and apply lora adapter, if any
        //ctx = llama_init_from_gpt_params(params);
        std::tie(model, ctx) = llama_init_from_gpt_params(params);
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
        llama_backend_init(params.numa);
        printf("..............Backend initialized................\n");

        // load the model and apply lora adapter, if any
        //ctx = llama_init_from_gpt_params(params);
        std::tie(model, ctx) = llama_init_from_gpt_params(params);
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
            llama_backend_init(params.numa);
            printf("..............Backend initialized................\n");

            // load the model and apply lora adapter, if any
            std::tie(model, ctx) = llama_init_from_gpt_params(params);
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
        //fprintf(stderr, "%s: build = %d (%s)\n", __func__, BUILD_NUMBER, BUILD_COMMIT);

        
        //printf("%s: Model: successful load\n");
        // print system information
        //{
            //fprintf(stderr, "\n");
            //fprintf(stderr, "system_info: n_threads = %d / %d | %s\n",
            //        params.n_threads, std::thread::hardware_concurrency(), llama_print_system_info());
        //}

        // determine the maximum memory usage needed to do inference for the given n_batch and n_ctx parameters
        // uncomment the "used_mem" line in llama.cpp to see the results
        /* if (params.mem_test) {
            {
                // const std::vector<llama_token> tmp(params.n_batch, llama_token_bos());
                // llama_eval(ctx, tmp.data(), tmp.size(), 0, params.n_threads);
            // }
            
                fprintf(stderr, "%s: testing memory usage for n_batch = %d, n_ctx = %d\n", __func__, params.n_batch, params.n_ctx);

            //{
                // const std::vector<llama_token> tmp = { 0, };
                // llama_eval(ctx, tmp.data(), tmp.size(), params.n_predict - 1, params.n_threads);
                const std::vector<llama_token> tmp(params.n_batch, llama_token_bos(ctx));
                llama_eval(ctx, tmp.data(), tmp.size(), params.n_ctx, params.n_threads);
            }

            llama_print_timings(ctx);
            llama_free(ctx);
            llama_free_model(model);

            return 0;
        } */
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
                if (!llama_load_session_file(ctx, path_session.c_str(), session_tokens.data(), session_tokens.capacity(), &n_token_count_out)) {
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
        
        if (!params.antiprompt.size()) params.antiprompt.push_back("You:");
        if (params.prompt.empty()) params.prompt = params.antiprompt.back();
        
        if (params.instruct) {
            params.antiprompt.push_back("### Instruction:");
            params.prompt = "### Instruction:";
        }

        // Add a space in front of the first character to match OG llama tokenizer behavior
        //removed after GGUF
        //params.prompt.insert(0, 1, ' ');
        
        //const bool is_spm = llama_vocab_type(ctx) == LLAMA_VOCAB_TYPE_SPM;
        // Add BOS if SPM tokenizer
        //const bool add_bos = llama_vocab_type(model) == LLAMA_VOCAB_TYPE_SPM;
        const bool add_bos = llama_should_add_bos_token(model);
        printf("add_bos: %d\n", add_bos);
        
        /* // tokenize the prompt
        if (llama_vocab_type(ctx) == LLAMA_VOCAB_TYPE_SPM) {
            // Add a space in front of the first character to match OG llama tokenizer behavior
            params.prompt.insert(0, 1, ' ');
        } */
        
        // if (!params.prompt.empty() || session_tokens.empty()) {
            // // Add a space in front of the first character to match OG llama tokenizer behavior
            // params.prompt.insert(0, 1, ' ');
            
        
        
        if (params.interactive_first || params.instruct || !params.prompt.empty() || session_tokens.empty()) {
            //this is the first problem we have
            // there is no formatting for the initial prompt
            //embd_inp = ::llama_tokenize(ctx, params.prompt, add_bos, true);
            formatInput(params.format_instruct, params.prompt);
        } else {
            embd_inp = session_tokens;
        }
        
        if (embd_inp.empty()) {
            embd_inp.push_back(llama_token_bos(model));
        }
        
        //if (embd_inp.empty()) {
        //    embd_inp.push_back(llama_token_bos(ctx));
        //}

        // Tokenize negative prompt

        if (ctx_guidance) {
            //params.cfg_negative_prompt.insert(0, 1, ' ');
            guidance_inp = ::llama_tokenize(ctx_guidance, sparams.cfg_negative_prompt, add_bos, true);

            std::vector<llama_token> original_inp = ::llama_tokenize(ctx, params.prompt, add_bos, true);
            original_prompt_len = original_inp.size();
            guidance_offset = (int)guidance_inp.size() - original_prompt_len;
        }
        
        //n_ctx = llama_n_ctx(ctx);

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
        if (params.n_keep < 0 || params.n_keep > (int) embd_inp.size() || params.instruct) {
            params.n_keep = (int)embd_inp.size();
        }

        // prefix & suffix for instruct mode
        inp_pfx = ::llama_tokenize(ctx, "\n\n### Instruction:\n\n", add_bos, true);
        inp_sfx = ::llama_tokenize(ctx, "\n\n### Response:\n\n", false, true);

        
        fprintf(stderr, "\nsampling: repeat_last_n = %d, penalty_repeat = %f, penalty_present = %f, penalty_freq = %f, top_k = %d, tfs_z = %f, top_p = %f, typical_p = %f, temp = %f, mirostat = %d, mirostat_lr = %f, mirostat_ent = %f\n",
                sparams.penalty_last_n, sparams.penalty_repeat, sparams.penalty_present, sparams.penalty_freq, sparams.top_k, sparams.tfs_z, sparams.top_p, sparams.typical_p, sparams.temp, sparams.mirostat, sparams.mirostat_eta, sparams.mirostat_tau);
        
        //stats["Params"] = paramsPrint;
        //fprintf(stderr, "generate: n_ctx = %d, n_batch = %d, n_predict = %d, n_keep = %d\n", n_ctx, params.n_batch, params.n_predict, params.n_keep);
        //fprintf(stderr, "\n");


        /* if (!params.grammar.empty()) {
            parsed_grammar = grammar_parser::parse(params.grammar.c_str());
            // will be empty (default) if there are parse errors
            if (parsed_grammar.rules.empty()) {
                return 1;
            }
            fprintf(stderr, "%s: grammar:\n", __func__);
            grammar_parser::print_grammar(stderr, parsed_grammar);
            fprintf(stderr, "\n");

            {
                auto it = sparams.logit_bias.find(llama_token_eos(ctx));
                if (it != sparams.logit_bias.end() && it->second == -INFINITY) {
                    fprintf(stderr, "%s: warning: EOS token is disabled, which will cause most grammars to fail\n", __func__);
                }
            }

            std::vector<const llama_grammar_element *> grammar_rules(parsed_grammar.c_rules());
            grammar = llama_grammar_init(
                grammar_rules.data(), grammar_rules.size(), parsed_grammar.symbol_ids.at("root"));
        } */
        
        // TODO: replace with ring-buffer
        //last_n_tokens = std::vector<llama_token>(n_ctx);
        //std::fill(last_n_tokens.begin(), last_n_tokens.end(), 0);
        //std::vector<llama_token> last_tokens(n_ctx);
        //last_tokens = std::vector<llama_token>(n_ctx);
        //std::fill(last_tokens.begin(), last_tokens.end(), 0);

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
       

        // the first thing we will do is to output the prompt, so set color accordingly


        // moved
        // {
            // const std::vector<llama_token> tmp = { llama_token_bos(ctx), };
            // llama_eval(ctx, tmp.data(), tmp.size(), 0, params.n_threads);
            // llama_reset_timings(ctx);
        // }
        
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
        if (n_past + (int) embd.size() + std::max<int>(0, guidance_offset) > n_ctx) {
            printf("....");
            const int n_left    = n_past - params.n_keep - 1;
            const int n_discard = n_left/2;

            // always keep the first token - BOS
            //n_past = std::max(1, params.n_keep);
            //n_past_guidance = std::max(1, params.n_keep + guidance_offset);
            llama_kv_cache_seq_rm   (ctx, 0, params.n_keep + 1            , params.n_keep + n_discard + 1);
            llama_kv_cache_seq_shift(ctx, 0, params.n_keep + 1 + n_discard, n_past, -n_discard);

            // insert n_left/2 tokens at the start of embd from last_n_tokens
            //embd.insert(embd.begin(), last_n_tokens.begin() + n_ctx - n_left/2 - embd.size(), last_n_tokens.end() - embd.size());
            //embd.insert(embd.begin(), last_tokens.begin() + n_ctx - n_left/2 - embd.size(), last_tokens.end() - embd.size());
            n_past -= n_discard;

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
            if (n_past + (int) embd.size() + std::max<int>(0, guidance_offset) > n_ctx) {
                // if (params.n_predict == -2) {
                    // printf("\n\n%s: context full and n_predict == -%d => stopping\n", __func__, params.n_predict);
                    // break;
                // }
                
                const int n_left    = n_past - params.n_keep - 1;
                const int n_discard = n_left/2;
                
                llama_kv_cache_seq_rm   (ctx, 0, params.n_keep + 1            , params.n_keep + n_discard + 1);
                llama_kv_cache_seq_shift(ctx, 0, params.n_keep + 1 + n_discard, n_past, -n_discard);
                
                n_past -= n_discard;
                
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
                
                llama_kv_cache_seq_shift(ctx, 0, ga_i,                n_past,              ib*bd);
                llama_kv_cache_seq_div  (ctx, 0, ga_i + ib*bd,        ga_i + ib*bd + ga_w, ga_n);
                llama_kv_cache_seq_shift(ctx, 0, ga_i + ib*bd + ga_w, n_past + ib*bd,      dd);
                
                n_past -= bd;

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
    
    int evaluate_main(){
        for (int i = 0; i < (int) embd.size(); i += params.n_batch) {
            int n_eval = (int) embd.size() - i;
            if (n_eval > params.n_batch) {
                n_eval = params.n_batch;
            }
            if (llama_decode(ctx, llama_batch_get_one(&embd[i], n_eval, n_past, 0))) {
                fprintf(stderr, "%s : failed to eval\n", __func__);
                return 0;
            }
            n_past += n_eval;
        }
        
        return 1;
    }
    void evaluate_session(){
        if (embd.size() > 0 && !path_session.empty()) {
            session_tokens.insert(session_tokens.end(), embd.begin(), embd.end());
            n_session_consumed = session_tokens.size();
        }
    }
    
    //checking already existing contex
    int checkEmb(){
        if (debug) fprintf(stderr, "1");
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
        if (debug) fprintf(stderr, "2");

        // optionally save the session on first sample (for faster prompt loading next time)
        if (!path_session.empty() && need_to_save_session && !params.prompt_cache_ro) {
            need_to_save_session = false;
            llama_save_session_file(ctx, path_session.c_str(), session_tokens.data(), session_tokens.size());
        }
        
        
        
        // new generation function, moved to common 
        const llama_token id = llama_sampling_sample(ctx_sampling, ctx, ctx_guidance);
        //const llama_token id = llama_sampling_sample_choice(ctx_sampling, ctx, ctx_guidance, tempFirst);
        
        //last_tokens.erase(last_tokens.begin());
        //last_tokens.push_back(id);
        llama_sampling_accept(ctx_sampling, ctx, id, true);

        // add it to the context
        //embd.push_back(id);
        embd.push_back(id);
        ++last_tokens_count;

        // echo this to console
        input_echo = true;

        // decrement remaining sampling budget
        --n_remain;
        
        return 1;
    }
    
// main generation end////////////////////////////////////////////////////////////////

    void clear_last() {
        llama_kv_cache_seq_rm(ctx, 0, n_past - last_tokens_count, -1);
        embd_inp.erase(embd_inp.begin() + n_consumed, embd_inp.end());
        n_remain += last_tokens_count;
        n_past -= last_tokens_count;
        
        last_tokens_count = 0;
        
        if (n_past > 0) {
            resetGrammar();
            is_interacting = false;
        }
    }

// additional functions
    void clearLastEmbd(){
        embd.erase(embd.begin()+last_tokens_count);
        n_remain += last_tokens_count;
        last_tokens_count = 0;
    }
        
    void clearLastTokens(){
        last_tokens_count = 0;
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
    int checkAntiprompt(){
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
                    if (debug) fprintf(stderr, "6");
                    is_antiprompt = true;
                    fflush(stdout);
                    break;
                }
            }
        }
        
        return 1;
    }
    
    int setEOS(){
        // deal with end of text token in interactive mode
        //if (last_n_tokens.back() == llama_token_eos(ctx)) {
        //if (last_tokens.back() == llama_token_eos(ctx)) {
        //if (ctx_sampling->prev.back() == llama_token_eos(ctx)) {
        if (llama_sampling_last(ctx_sampling) == llama_token_eos(model)) {
            if (params.interactive) {
                if (params.antiprompt.size() != 0) {
                    // tokenize and inject first reverse prompt
                    const auto first_antiprompt = ::llama_tokenize(ctx, params.antiprompt.front(), false, true);
                    embd_inp.insert(embd_inp.end(), first_antiprompt.begin(), first_antiprompt.end());
                    is_antiprompt = true;
                }

                is_interacting = true;
                //printf("\n");
            } else if (params.instruct) {
                is_interacting = true;
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
            if (debug) fprintf(stderr, "3");
            fastStop = true;
            // some user input remains from prompt or interaction, forward it to processing
            while ((int) embd_inp.size() > n_consumed) {
                //fprintf(stderr, ">");
                embd.push_back(embd_inp[n_consumed]);
                //last_n_tokens.erase(last_n_tokens.begin());
                //last_n_tokens.push_back(embd_inp[n_consumed]);
                //last_tokens.erase(last_tokens.begin());
                //last_tokens.push_back(embd_inp[n_consumed]);
                
                // push the prompt in the sampling context in order to apply repetition penalties later
                // for the prompt, we don't apply grammar rules
                llama_sampling_accept(ctx_sampling, ctx, embd_inp[n_consumed], false);
                
                
                ++n_consumed;
                if ((int) embd.size() >= params.n_batch) {
                    break;
                }
            }
        }

        // display text
        
        
        if (input_echo) {
            if (debug) fprintf(stderr, "4");
            for (auto id : embd) { 
                //std::string tknStr = llama_token_to_string(ctx, id); 
                const std::string tknStr = llama_token_to_piece(ctx, id); 
                //result += (std::string) tknStr;
                result += tknStr;
                //if (streaming) printf("%s", tknStr);
                if (streaming) std::cout<<tknStr;
            }

        }
        
        // if ((int) embd_inp.size() <= n_consumed) {
            // checkAntiprompt();
        // }
        
        return 1;
    }
    
    
    
    // input assembly, DELIMINER is important for proper model functioning since we use getline
    int subInput(std::string& buffer, std::string line){
        buffer += line + DELIMINER;
        //if (debug) fprintf(stderr, "7");
        //std::cout << " *** *** " << line << std::endl;
        // Add tokens to embd only if the input buffer is non-empty
        // Entering a empty line lets the user pass control back
        if (params.input_prefix_bos) {
            embd_inp.push_back(llama_token_bos(model));
        }
        
        
        
        if (buffer.length() > 1) {
            if (debug) fprintf(stderr, "<");
            
            appendPrefix(buffer);
            
            // append input suffix if any
            appendSuffix(buffer);
            
            const size_t original_size = embd_inp.size();

            // instruct mode: insert instruction prefix
            if (params.instruct && !is_antiprompt) {
                n_consumed = embd_inp.size();
                if (debug) printf("|");
                embd_inp.insert(embd_inp.end(), inp_pfx.begin(), inp_pfx.end());
            }
            
            if (params.escape) {
                process_escapes(buffer);
            }

            //auto line_inp = ::llama_tokenize(ctx, buffer, false);
            //const auto line_inp = ::llama_tokenize(ctx, buffer, false);
            const auto line_pfx = ::llama_tokenize(ctx, params.input_prefix, false, true);
            const auto line_inp = ::llama_tokenize(ctx, buffer, false, false);
            const auto line_sfx = ::llama_tokenize(ctx, params.input_suffix, false, true);
            //embd_inp.insert(embd_inp.end(), line_inp.begin(), line_inp.end());
            embd_inp.insert(embd_inp.end(), line_pfx.begin(), line_pfx.end());
            embd_inp.insert(embd_inp.end(), line_inp.begin(), line_inp.end());
            embd_inp.insert(embd_inp.end(), line_sfx.begin(), line_sfx.end());
            
            for (size_t i = original_size; i < embd_inp.size(); ++i) {
                const llama_token token = embd_inp[i];
                output_tokens.push_back(token);
                output_ss << llama_token_to_piece(ctx, token);
            }

            // instruct mode: insert response suffix
            if (params.instruct) {
                if (debug) printf("^");
                embd_inp.insert(embd_inp.end(), inp_sfx.begin(), inp_sfx.end());
            }

            n_remain -= line_inp.size();
        }

        //is_antiprompt = false;
        
        return 1;
    }
    
    // the logics should be:
    // 3 4! 5 6 1 3 4! 5 6 > < 3 [5 1 2 4!] 1 3 4! 5 6
    // forwarding, print, preInput, postInput, check Emb, forwarding, print, preInput, postIput (waiting for input), postInput(getting input ), forwarding, [preInput, check Emb, generate, print], check, forwarding, print Anti, preInput, postIput (waiting for input)    
    
    //input, which requires preemptive processing (checking, adding prompt leftovers, antiprompt processing)
    
    int inputOnly(std::string input){
        //std::cout << " ***** " << input << std::endl;
        if (n_past > 0 && is_interacting) {

            std::string buffer;
            
            //std::cout << " ***** " << input << std::endl;
            
            subInput(buffer, input);     
            input_echo = false; // do not echo this again        
        }
        
        if (n_past > 0) {
            resetGrammar();
            is_interacting = false;
        }
        
        //checkInfinite();

        return 1;
    }
    
// NEW //////////////////////////////////////////////////////////////////////////////////////////
    
    void resetCTX(){
        llama_sampling_reset(ctx_sampling);
    }


    int applyEmbNew(bool fastStop = false){
        if (debug) fprintf(stderr, "2");

        // optionally save the session on first sample (for faster prompt loading next time)
        if (!path_session.empty() && need_to_save_session && !params.prompt_cache_ro) {
            need_to_save_session = false;
            llama_save_session_file(ctx, path_session.c_str(), session_tokens.data(), session_tokens.size());
        }
        
        //llama_sampling_context ctx_sampling = llama_sampling_context_init(params, grammar);
        
        // new generation function, moved to common 
        const llama_token id = llama_sampling_sample(ctx_sampling, ctx, ctx_guidance);
        //const llama_token id = llama_sampling_sample_choice(ctx_sampling, ctx, ctx_guidance, tempFirst);
        
        //last_tokens.erase(last_tokens.begin());
        //last_tokens.push_back(id);
        
        llama_sampling_accept(ctx_sampling, ctx, id, true);

        // add it to the context
        //embd.push_back(id);
        embd_msg.push_back(id);
        //++last_tokens_count;

        // echo this to console
        input_echo = true;

        // decrement remaining sampling budget
        --n_remain;
        
        return 1;
    }
    
    void appendPrefixBos(){
        if (params.input_prefix_bos) {
            embd_inp.push_back(llama_token_bos(model));
        }
    }
    
    
    void classicProcessing(std::string& buffer){
        const auto line_pfx = ::llama_tokenize(ctx, params.input_prefix, false, true);
        const auto line_inp = ::llama_tokenize(ctx, buffer, false, false);
        const auto line_sfx = ::llama_tokenize(ctx, params.input_suffix, false, true);
        
        embd_inp.insert(embd_inp.end(), line_pfx.begin(), line_pfx.end());
        embd_inp.insert(embd_inp.end(), line_inp.begin(), line_inp.end());
        embd_inp.insert(embd_inp.end(), line_sfx.begin(), line_sfx.end());
        
        n_remain -= line_inp.size();
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
            if (params.instruct && !is_antiprompt) {
                n_consumed = embd_inp.size();
                if (debug) printf("|");
                embd_inp.insert(embd_inp.end(), inp_pfx.begin(), inp_pfx.end());
            }
            
            if (params.escape) {
                process_escapes(buffer);
            }

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
           
            //classicProcessing(buffer);
            formatInput(params.format_dialog, buffer);
            //formatInput(buffer);
            
            
            
            for (size_t i = original_size; i < embd_inp.size(); ++i) {
                const llama_token token = embd_inp[i];
                output_tokens.push_back(token);
                output_ss << llama_token_to_piece(ctx, token);
            }

            // instruct mode: insert response suffix
            if (params.instruct) {
                if (debug) printf("^");
                embd_inp.insert(embd_inp.end(), inp_sfx.begin(), inp_sfx.end());
            }
            
            

            // n_remain -= line_inp.size();
        }

        //is_antiprompt = false;
        
        return 1;
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
        
        formatRepresentation += "{output}";
        
        //checkInfinite();

        return 1;
    }

// initial (instruct) processing
    std::string generate(bool consoleOutput = true, bool verbose = false) {
        
        printf("Starting initial prompt processing...\n");
        
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
        
        while ((n_remain != 0 && !is_antiprompt) || params.interactive) {
            
            if (consoleOutput) preEmb(result, fastStop, streaming); // includes checking, generation and adding prompt leftovers
            else preEmb(result, fastStop, consoleOutput);
            
            if ((int) embd_inp.size() <= n_consumed) {
                if (debug) fprintf(stderr, "5");
                
                //checkAntiprompt();
                
                if (n_past > 0 && is_interacting) {
                    checkAntiprompt();
                    
                    if (verbose) {
                        if (!streaming) std::cout << result << " ";
                        
                        printf("Return generate: prompt processed\n");
                    }
                    
                    return result;
                }
                
            }
            
        }
        
        

        return "cycle broken!";
    }

    const std::string getPrinted(){
        if (debug) fprintf(stderr, "4");
        
        for (auto id : embd) { 
            //return llama_token_to_string(ctx, id); 
            return llama_token_to_piece(ctx, id); 
        }
    }
    
    const std::string getPrinted(std::vector<llama_token>& embd_msg){
        if (debug) fprintf(stderr, "4");
        
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
    
    std::string preEmbNew(bool& fastStop){ // 1 2 3 4
        //std::cout << " ** " << std::endl;
        
        if (embd.size() > 0) {
            checkEmb(); // 1
        }

        embd.clear();
        embd_guidance.clear();

        if ((int) embd_inp.size() <= n_consumed && !is_interacting) {
            applyEmb(); // 2
        } else {
            if (debug) fprintf(stderr, "3");
            fastStop = true;
            // some user input remains from prompt or interaction, forward it to processing
            while ((int) embd_inp.size() > n_consumed) {
                //fprintf(stderr, ">");
                embd.push_back(embd_inp[n_consumed]);
                //last_n_tokens.erase(last_n_tokens.begin());
                //last_n_tokens.push_back(embd_inp[n_consumed]);
                //last_tokens.erase(last_tokens.begin());
                //last_tokens.push_back(embd_inp[n_consumed]);
                
                // GG: I'm not sure it's a good idea to push the prompt tokens into the sampling context
                //     Most likely will remove this in the future to avoid exposing "prev"
                //     Same thing is done in "server". If we stop pushing the prompt tokens, then the repetition
                //     penalty will be applied only based on the tokens generated by the model.
                ctx_sampling->prev.erase(ctx_sampling->prev.begin());
                ctx_sampling->prev.push_back(embd_inp[n_consumed]);
                
                
                ++n_consumed;
                if ((int) embd.size() >= params.n_batch) {
                    break;
                }
            }
        }

        // display text
        
        
        if (input_echo) {
            return getPrinted();
        }
        
        // if ((int) embd_inp.size() <= n_consumed) {
            // checkAntiprompt();
        // }
        
        return "";
    }
    
    // token by token generation and pushing
    std::string cycleStringsOnly(bool stream = false) {
        //std::cout << " * " << input << std::endl;
        
        std::string result;
        bool fastStop = false;
        
    
        //generate(false);  // do not forget to include it elsewhere after loading the model  
        //inputOnly(input); // MOVED
        
        std::string bit = preEmbNew(fastStop);
            
        //if (stream || streaming) std::cout << bit;
    
        result += bit;
        
        if ((int) embd_inp.size() <= n_consumed) {
            if (debug) fprintf(stderr, "5");
            //fprintf(stderr, "5");
            
            checkAntiprompt();
            
            setEOS();
            
            if (n_past > 0 && is_interacting) {
                //fprintf(stderr, "6");
                
                //eraseAntiprompt(result);
                finished = true;
                
                //checkAntiprompt();
                //setEOS();
                
                return result;
            }
            
        }
        
        // if ((n_remain != 0 && !is_antiprompt) || params.interactive) {
        
            // result += cycleStringsOnly(input, stream);
        
        // }

        return result;
    }
   
    int save(std::string fileName, std::vector<char*>& results){
        if (saveToFile){
            writeTextFile(SESSIONS_FOLDER + fileName, results);
        }
        
        return 1;
    }

    
};

