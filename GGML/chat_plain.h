// Defines sigaction on msys:
// #ifndef _GNU_SOURCE
// #define _GNU_SOURCE
// #endif

#include "common.h"
#include "llama.h"
#include "build-info.h"
#include "grammar-parser.h"
#include "json.hpp"

#include <cassert>
#include <cinttypes>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

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

static llama_context ** g_ctx;
static gpt_params paramsDefault;

static bool is_interacting = false;

void readGrammarFile(gpt_params& params, std::string fileName){
    std::ifstream file(fileName);
    if (!file) {
        fprintf(stderr, "error: failed to open file '%s'\n", fileName);
    } else {
        std::copy(
            std::istreambuf_iterator<char>(file),
            std::istreambuf_iterator<char>(),
            std::back_inserter(params.grammar)
        );
    }
}

auto loadNpring(nlohmann::json& config, std::string confName, bool verbose = true){
    if (verbose) std::cout << "load: " << confName << " = "  << config[confName] << std::endl;
    return config[confName];
}

void processInstructFile(std::string filename, gpt_params& params, bool headless = false){
    std::ifstream file(filename);
    if (!file) {
        fprintf(stderr, "error: failed to open file '%s'\n", filename);
    } else {
        params.prompt.clear();
        //fprintf(stderr, "opening file '%s'...\n", config["file"]);
        std::copy(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>(), back_inserter(params.prompt));
        if (params.prompt.back() == '\n') {
            params.prompt.pop_back();
        }
        if (!headless){
            int antiPos = params.prompt.rfind('\n');
            if (antiPos != params.prompt.npos) {
                if(!params.antiprompt.size()) 
                    params.antiprompt.push_back(params.prompt.substr(antiPos));
                else
                    params.antiprompt[0] = params.prompt.substr(antiPos);
            }
        }
    }
}

bool checkJString(nlohmann::json& config, std::string name){
    if(config.contains(name)){
        if(config[name].is_string()) return true;
    }
    
    return false;
}

bool checkJNum(nlohmann::json& config, std::string name){
    if(config.contains(name)){
        if(config[name].is_number()) return true;
    }
    
    return false;
}

void getParamsFromJson(nlohmann::json& config, gpt_params& params, bool hasFile = false, bool headless = false){
    
    if (checkJString(config, "file")) {
        processInstructFile(config["file"], params, headless);
    } else if (!hasFile) {
        if (checkJString(config, "prompt")) params.prompt = config["prompt"];
        
        if (checkJString(config, "reverse-prompt")){
            if(!params.antiprompt.size()) 
                params.antiprompt.push_back(config["reverse-prompt"]);
            else
                params.antiprompt[0] = config["reverse-prompt"];
        }
    }
    
    if (checkJString(config, "model")) params.model = config["model"];
    if (checkJString(config, "cfg-negative-prompt")) params.cfg_negative_prompt = loadNpring(config,"cfg-negative-prompt", true);
    if (checkJNum(config, "cfg-scale")) params.cfg_scale = loadNpring(config,"cfg-scale", true);
    //if (config["cfg-smooth-factor"].is_number()) params.cfg_smooth_factor = loadNpring(config,"cfg-smooth-factor", false);
    if (checkJString(config, "lora")) {
        params.lora_adapter = config["lora"];
        //params.use_mmap = false;
    }
    if (checkJNum(config, "n_threads")) params.n_threads = loadNpring(config,"n_threads", true);
    if (checkJNum(config, "e_threads")) params.e_threads = loadNpring(config,"e_threads", true);
    if (checkJNum(config, "ctx-size")) params.n_ctx = loadNpring(config,"ctx-size", true);
    if (checkJNum(config, "n_keep")) params.n_keep = loadNpring(config,"n_keep", true);
    if (checkJNum(config, "temp")) params.temp = loadNpring(config,"temp", true);
    if (checkJNum(config, "n_gpu_layers")) params.n_gpu_layers = loadNpring(config,"n_gpu_layers", true);
    if (checkJNum(config, "top_k")) params.top_k = loadNpring(config,"top_k", true);
    if (checkJNum(config, "top_p")) params.top_p = loadNpring(config,"top_p", true);
    if (checkJNum(config, "typical_p")) params.typical_p = loadNpring(config,"typical_p", true);
    if (checkJNum(config, "tfs_z")) params.tfs_z = loadNpring(config,"tfs_z", true);
    if (checkJNum(config, "repeat_penalty")) params.repeat_penalty = loadNpring(config,"repeat_penalty", true);
    if (checkJNum(config, "frequency_penalty")) params.frequency_penalty = loadNpring(config,"frequency_penalty", true);
    if (checkJNum(config, "presence_penalty")) params.presence_penalty = loadNpring(config,"presence_penalty", true);
    if (checkJNum(config, "mirostat")) params.mirostat = loadNpring(config,"mirostat", true);
    if (checkJNum(config, "mirostat_tau")) params.mirostat_tau = loadNpring(config,"mirostat_tau", true);
    if (checkJNum(config, "mirostat_eta")) params.mirostat_eta = loadNpring(config,"mirostat_eta", true);
    
    if (checkJString(config, "grammar")) params.grammar = config["grammar"];
    if (checkJString(config, "grammar-file")) readGrammarFile(params, config["grammar-file"]);
    if (checkJNum(config, "rms-norm-eps")) params.rms_norm_eps = loadNpring(config,"rms-norm-eps", true);
    if (checkJNum(config, "rope_freq_base")) params.rope_freq_base = loadNpring(config,"rope_freq_base", true);
    if (checkJNum(config, "rope_freq_scale")) params.rope_freq_scale = loadNpring(config,"rope_freq_scale", true);
    
    std::cout << "headless: " << headless << std::endl;
}

void readParamsFromJson(nlohmann::json& config, gpt_params& params, bool headless = false){
    
    getParamsFromJson(config, params, false, headless);
    
    if(config["model"].is_string() && config[params.model].is_object()){
        std::cout << "Found settings for model " << params.model << std::endl;
        nlohmann::json modelConfig = config[params.model];
        bool hasFiles = modelConfig["file"].is_string() || config["file"].is_string();
        getParamsFromJson(modelConfig, params, hasFiles, headless);
    }

}

void readParamsFromJson(nlohmann::json& config, std::string modelName, gpt_params& params, bool headless = false){
    
    getParamsFromJson(config, params, false, headless);
    params.model = modelName;
    
    if (config.contains(params.model)){
        if(config["model"].is_string() && config[params.model].is_object()){
            std::cout << "Found settings for model " << params.model << std::endl;
            nlohmann::json modelConfig = config[params.model];
            bool hasFiles = modelConfig["file"].is_string() || config["file"].is_string();
            getParamsFromJson(modelConfig, params, hasFiles, headless);
        }
    }
}

void readParamsFromFile(std::string fimeName, gpt_params& params, bool headless = false){
    std::fstream o1(fimeName);
    if (o1.is_open()) {
        nlohmann::json config;
        o1 >> std::setw(4) >> config;
        
        getParamsFromJson(config, params, false, headless);
        
        if(config["model"].is_string() && config[params.model].is_object()){
            std::cout << "Found settings for model " << params.model << std::endl;
            nlohmann::json modelConfig = config[params.model];
            bool hasFiles = modelConfig["file"].is_string() || config["file"].is_string();
            getParamsFromJson(modelConfig, params, hasFiles, headless);
        }
        
        o1.close();
    }
}

void readParamsFromFile(std::string fimeName, std::string modelName, gpt_params& params, bool headless = false ){
    std::fstream o1(fimeName);
    if (o1.is_open()) {
        nlohmann::json config;
        o1 >> std::setw(4) >> config;
        
        getParamsFromJson(config, params, false, headless);
        params.model = modelName;
        
        if(config[modelName].is_object()){
            std::cout << "Found settings for model " << params.model << std::endl;
            nlohmann::json modelConfig = config[params.model];
            bool hasFiles = modelConfig["file"].is_string() || config["file"].is_string();
            getParamsFromJson(modelConfig, params, hasFiles, headless);
        }
        
        o1.close();
    }
}

bool writeTextFile(std::string path, std::string text){
    std::ofstream file(path, std::ios::app);
    if (file.is_open()) {
        file << text;
        file.close();
        return true;
    } else {
        return false;
    }
}

bool writeTextFile(std::string path, std::vector<char*>& texts){
    std::ofstream file(path, std::ios::app);
    if (file.is_open()) {
        
        for (auto text : texts){
            file << text;
        }
        
        file.close();
        return true;
    } else {
        return false;
    }
}

bool writeTextFileOver(std::string path, std::string text){
    std::ofstream file(path);
    if (file.is_open()) {
        file << text;
        file.close();
        return true;
    } else {
        return false;
    }
}

bool writeTextFileOver(std::string path, std::vector<char*>& texts){
    std::ofstream file(path);
    if (file.is_open()) {
        
        for (auto text : texts){
            file << text;
        }
        
        file.close();
        return true;
    } else {
        return false;
    }
}

void writeJson(nlohmann::json config, std::string filename) {
    if (filename.find(".json") == filename.npos)
        filename += ".json";

    std::ofstream o(filename, std::ofstream::out);
    //std::cout << config << std::endl;
    if (config.contains("error")) config.erase("error");
    if (o.is_open()) {
        o << std::setw(4) << config << std::endl;
        //std::cout << "Json written to " << filename << std::endl;
    }
    o.close();
    o.flush();

}

nlohmann::json getJson(std::string fimeName){
    nlohmann::json config;
    std::fstream o1(fimeName);
    if (o1.is_open()) {
        
        o1 >> std::setw(4) >> config;
        o1.close();
    } else {
        config["error"] = "is_open";
    }
    
    return config;
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

float llama_string_eval(const llama_timings timings) {
    return (1e3 / timings.t_eval_ms * timings.n_eval);
}

float llama_string_evalPrompt(const llama_timings timings) {
    return (1e3 / timings.t_p_eval_ms * timings.n_p_eval);
}

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
    std::vector<llama_token> last_n_tokens;
    std::vector<llama_token> guidance_inp;
    std::vector<llama_token> embd_guidance;
    grammar_parser::parse_state parsed_grammar;
    llama_grammar * grammar = NULL;
    bool cleared = true;
        
    
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
    int last_tokens           = 0;
    int t_eval_ms             = 0;
    int n_eval                = 0;
    int t_p_eval_ms           = 0;
    int n_p_eval              = 0;
    
    
    std::vector<llama_token> embd;
    
    std::vector<int> inp_pfx;
    std::vector<int> inp_sfx;
       
    std::vector<int> llama_token_newline;
    
public:
    gpt_params params;
    bool finished = true;
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
        last_n_tokens.clear();
        guidance_inp.clear();
        embd_guidance.clear();
        embd.clear();
        llama_token_newline.clear();
        
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
        
        
        params = paramsDefault;
    }
    
    void clear(){
        if (!cleared){
            cleared = true;
            
            clearSoft();
            
            llama_free(ctx);
            if (ctx_guidance) { llama_free(ctx_guidance); }
            llama_free_model(model);
            if (grammar != NULL) {
                llama_grammar_free(grammar);
            }
            llama_backend_free();
        }
    }
    
    void print_timings(){
        //llama_print_timings(ctx);
        std::cout << llama_string_timings(ctx) << std::endl;
    }
    
    int getLastTokens(){
        return last_tokens;
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
        return std::to_string(llama_string_eval(ctx)) + " tokens per second";
    }
    
    std::string get_evalPrompt(){
        //llama_print_timings(ctx);
        return std::to_string(llama_string_evalPrompt(ctx)) + " tokens per second";
    }
    
    std::string get_ts(){
        std::string result = "Evals: \n";
        const llama_timings timings = llama_get_timings(ctx);
        result += "Prompts: " + std::to_string(llama_string_evalPrompt(timings)) + " t/s; \n";
        result += "Generation: " + std::to_string(llama_string_eval(timings)) + " t/s; \n";
        
        return result;
    }
    
    float get_speed(){
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
        
        int result = load(soft);
        
        generate(streamed);
        return result;
    }
    
    int checkPreLoad(){

        if (params.perplexity) {
            printf("\n************\n");
            printf("%s: please use the 'perplexity' tool for perplexity calculations\n", __func__);
            printf("************\n\n");

            return 0;
        }

        if (params.embedding) {
            printf("\n************\n");
            printf("%s: please use the 'embedding' tool for embedding calculations\n", __func__);
            printf("************\n\n");

            return 0;
        }
        
        if (params.rope_freq_base != 10000.0) {
            fprintf(stderr, "%s: warning: changing RoPE frequency base to %g (default 10000.0)\n", __func__, params.rope_freq_base);
        }

        if (params.rope_freq_scale != 1.0) {
            fprintf(stderr, "%s: warning: scaling RoPE frequency by %g (default 1.0)\n", __func__, params.rope_freq_scale);
        }

        //fprintf(stderr, "Context size %zu \n", params.n_ctx);
        if (params.n_ctx > 2048) {
            fprintf(stderr, "%s: warning: model might not support context sizes greater than 2048 tokens (%d specified);"
                    "expect poor results\n", __func__, params.n_ctx);
        } else if (params.n_ctx < 8) {
            fprintf(stderr, "%s: warning: minimum context size is 8, using minimum size.\n", __func__);
            params.n_ctx = 8;
        }
        
        return 1;
    }
    
    int setRandom(){
        if (params.seed == LLAMA_DEFAULT_SEED) {
            params.seed = time(NULL);
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

        g_ctx = &ctx;

        // load the model and apply lora adapter, if any
        //ctx = llama_init_from_gpt_params(params);
        std::tie(model, ctx) = llama_init_from_gpt_params(params);
        
        if (params.cfg_scale > 1.f) {
            struct llama_context_params lparams = llama_context_params_from_gpt_params(params);
            ctx_guidance = llama_new_context_with_model(model, lparams);
        }
        
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
        if (status == 0) return 0;
        
        //loading the model itself; uses llama_backend, g_ctx, ctx, ctx_guidance
        status += loadModel(); // 3
        if (status < 3) return 0;

        // setting seed
        status += setRandom(); // 6
        
        return status;
    }
    
    int load(bool soft = false){
        
        if (!soft){
            int status = 0;
            
            status = strictLoad();
            if (status == 0) return 0;
        }
        
        cleared = false;
        
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
        if (params.mem_test) {
            {
                // const std::vector<llama_token> tmp(params.n_batch, llama_token_bos());
                // llama_eval(ctx, tmp.data(), tmp.size(), 0, params.n_threads);
            // }
            
                fprintf(stderr, "%s: testing memory usage for n_batch = %d, n_ctx = %d\n", __func__, params.n_batch, params.n_ctx);

            //{
                // const std::vector<llama_token> tmp = { 0, };
                // llama_eval(ctx, tmp.data(), tmp.size(), params.n_predict - 1, params.n_threads);
                const std::vector<llama_token> tmp(params.n_batch, llama_token_bos());
                llama_eval(ctx, tmp.data(), tmp.size(), params.n_ctx, params.n_threads, params.e_threads);
            }

            llama_print_timings(ctx);
            llama_free(ctx);
            llama_free_model(model);

            return 0;
        }

        // export the cgraph and exit
        if (params.export_cgraph) {
            llama_eval_export(ctx, "llama.ggml");
            llama_free(ctx);
            llama_free_model(model);

            return 0;
        }

        path_session = params.path_prompt_cache;

        if (!path_session.empty()) {
            fprintf(stderr, "%s: attempting to load saved session from '%s'\n", __func__, path_session.c_str());

            // fopen to check for existing session
            FILE * fp = std::fopen(path_session.c_str(), "rb");
            if (fp != NULL) {
                std::fclose(fp);

                session_tokens.resize(params.n_ctx);
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

        // tokenize the prompt
        
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
        
        // if (!params.prompt.empty() || session_tokens.empty()) {
            // // Add a space in front of the first character to match OG llama tokenizer behavior
            // params.prompt.insert(0, 1, ' ');
        if (params.interactive_first || params.instruct || !params.prompt.empty() || session_tokens.empty()) {
            embd_inp = ::llama_tokenize(ctx, params.prompt, true);
        } else {
            embd_inp = session_tokens;
        }

        // Tokenize negative prompt

        if (ctx_guidance) {
            params.cfg_negative_prompt.insert(0, 1, ' ');
            guidance_inp = ::llama_tokenize(ctx_guidance, params.cfg_negative_prompt, true);

            std::vector<llama_token> original_inp = ::llama_tokenize(ctx, params.prompt, true);
            original_prompt_len = original_inp.size();
            guidance_offset = (int)guidance_inp.size() - original_prompt_len;
        }
        
        n_ctx = llama_n_ctx(ctx);

        if ((int) embd_inp.size() > n_ctx - 4) {
            fprintf(stderr, "%s: error: prompt is too long (%d tokens, max %d)\n", __func__, (int) embd_inp.size(), n_ctx - 4);
            return 1;
        }

        // debug message about similarity of saved session, if applicable
        size_t n_matching_session_tokens = 0;
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
        inp_pfx = ::llama_tokenize(ctx, "\n\n### Instruction:\n\n", true);
        inp_sfx = ::llama_tokenize(ctx, "\n\n### Response:\n\n", false);

        // determine newline token
        //llama_token_newline = ::llama_tokenize(ctx, "\n", false);

        // if (params.interactive) {


            // //fprintf(stderr, "%s: interactive mode on.\n", __func__);

            // if (params.antiprompt.size()) {
                // for (auto antiprompt : params.antiprompt) {
                    // //fprintf(stderr, "Reverse prompt: '%s'\n", antiprompt.c_str());
                    // fprintf(stderr, "Starting input from: '%s'\n", antiprompt.c_str());
                // }
            // }

            // if (!params.input_prefix.empty()) {
                // fprintf(stderr, "Input prefix: '%s'\n", params.input_prefix.c_str());
            // }

            // if (!params.input_suffix.empty()) {
                // fprintf(stderr, "Input suffix: '%s'\n", params.input_suffix.c_str());
            // }
        // }
        //char* paramsPrint = new char[200];
        fprintf(stderr, "\nsampling: repeat_last_n = %d, repeat_penalty = %f, presence_penalty = %f, frequency_penalty = %f, top_k = %d, tfs_z = %f, top_p = %f, typical_p = %f, temp = %f, mirostat = %d, mirostat_lr = %f, mirostat_ent = %f\n",
                params.repeat_last_n, params.repeat_penalty, params.presence_penalty, params.frequency_penalty, params.top_k, params.tfs_z, params.top_p, params.typical_p, params.temp, params.mirostat, params.mirostat_eta, params.mirostat_tau);
        
        //stats["Params"] = paramsPrint;
        //fprintf(stderr, "generate: n_ctx = %d, n_batch = %d, n_predict = %d, n_keep = %d\n", n_ctx, params.n_batch, params.n_predict, params.n_keep);
        //fprintf(stderr, "\n");


        if (!params.grammar.empty()) {
            parsed_grammar = grammar_parser::parse(params.grammar.c_str());
            // will be empty (default) if there are parse errors
            if (parsed_grammar.rules.empty()) {
                return 1;
            }
            fprintf(stderr, "%s: grammar:\n", __func__);
            grammar_parser::print_grammar(stderr, parsed_grammar);
            fprintf(stderr, "\n");

            {
                auto it = params.logit_bias.find(llama_token_eos());
                if (it != params.logit_bias.end() && it->second == -INFINITY) {
                    fprintf(stderr, "%s: warning: EOS token is disabled, which will cause most grammars to fail\n", __func__);
                }
            }

            std::vector<const llama_grammar_element *> grammar_rules(parsed_grammar.c_rules());
            grammar = llama_grammar_init(
                grammar_rules.data(), grammar_rules.size(), parsed_grammar.symbol_ids.at("root"));
        }
        
        // TODO: replace with ring-buffer
        last_n_tokens = std::vector<llama_token>(n_ctx);
        std::fill(last_n_tokens.begin(), last_n_tokens.end(), 0);

        if (params.interactive) {
            const char *control_message;

            control_message = " - Press Return to return control to LLaMa.\n"
                              " - To return control without starting a new line, end your input with '/'.\n"
                              " - If you want to submit another line, end your input with '\\'.\n";

            /*fprintf(stderr, "== Running in interactive mode. ==\n"
    #if defined (__unix__) || (defined (__APPLE__) && defined (__MACH__)) || defined (_WIN32)
                   " - Press Ctrl+C to interject at any time.\n"
    #endif
                   "%s\n", control_message);
        */
            is_interacting = params.interactive_first;
        }

        bool need_to_save_session = !path_session.empty() && n_matching_session_tokens < embd_inp.size();

        n_remain           = params.n_predict;

        // the first thing we will do is to output the prompt, so set color accordingly


        
        {
            const std::vector<llama_token> tmp = { llama_token_bos(), };
            llama_eval(ctx, tmp.data(), tmp.size(), 0, params.n_threads, params.e_threads);
            llama_reset_timings(ctx);
        }
        
        return 9;
    }      
//-----------------------------------------MAIN CYCLES------------------------------------------- 
//-----------------------------------------MAIN CYCLES------------------------------------------- 
//-----------------------------------------MAIN CYCLES-------------------------------------------     
    
    //checking already existing contex
    int checkEmb(){
        if (debug) fprintf(stderr, "1");
        // Note: n_ctx - 4 here is to match the logic for commandline prompt handling via
        // --prompt or --file which uses the same value.
        auto max_embd_size = n_ctx - 4;
        // Ensure the input doesn't exceed the context size by truncating embd if necessary.
        if ((int)embd.size() > max_embd_size) {
            auto skipped_tokens = embd.size() - max_embd_size;
            printf("<<input too long: skipped %ld token%s>>", skipped_tokens, skipped_tokens != 1 ? "s" : "");
            fflush(stdout);
            embd.resize(max_embd_size);
        }

        // infinite text generation via context swapping
        // if we run out of context:
        // - take the n_keep first tokens from the original prompt (via n_past)
        // - take half of the last (n_ctx - n_keep) tokens and recompute the logits in batches
        if (n_past + (int) embd.size() + std::max<int>(0, guidance_offset) > n_ctx) {
            printf("....");
            const int n_left = n_past - params.n_keep;

            // always keep the first token - BOS
            n_past = std::max(1, params.n_keep);
            n_past_guidance = std::max(1, params.n_keep + guidance_offset);

            // insert n_left/2 tokens at the start of embd from last_n_tokens
            embd.insert(embd.begin(), last_n_tokens.begin() + n_ctx - n_left/2 - embd.size(), last_n_tokens.end() - embd.size());

            // stop saving session if we run out of context
            path_session.clear();

            //printf("\n---\n");
            //printf("resetting: '");
            //for (int i = 0; i < (int) embd.size(); i++) {
            //    printf("%s", llama_token_to_str(ctx, embd[i]));
            //}
            //printf("'\n");
            //printf("\n---\n");
        }

        // try to reuse a matching prefix from the loaded session instead of re-eval (via n_past)
        if (n_session_consumed < (int) session_tokens.size()) {
            size_t i = 0;
            for ( ; i < embd.size(); i++) {
                if (embd[i] != session_tokens[n_session_consumed]) {
                    session_tokens.resize(n_session_consumed);
                    break;
                }

                n_past++;
                n_session_consumed++;

                if (n_session_consumed >= (int) session_tokens.size()) {
                    ++i;
                    break;
                }
            }
            if (i > 0) {
                embd.erase(embd.begin(), embd.begin() + i);
            }
        }

        // evaluate tokens in batches
        // embd is typically prepared beforehand to fit within a batch, but not always
        
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
                //fprintf(stderr, "\n---------------------\n");
                //for (int i = 0; i < (int) embd_guidance.size(); i++) {
                    //fprintf(stderr, "%s", llama_token_to_str(ctx, embd_guidance[i]));
                //}
                //fprintf(stderr, "\n---------------------\n");
            } else {
                input_buf = embd.data();
                input_size = embd.size();
            }

            for (int i = 0; i < input_size; i += params.n_batch) {
                int n_eval = std::min(input_size - i, params.n_batch);
                if (llama_eval(ctx_guidance, input_buf + i, n_eval, n_past_guidance, params.n_threads, params.e_threads)) {
                    fprintf(stderr, "%s : failed to eval\n", __func__);
                    return 1;
                }

                n_past_guidance += n_eval;
            }
        }
        
        for (int i = 0; i < (int) embd.size(); i += params.n_batch) {
            int n_eval = (int) embd.size() - i;
            if (n_eval > params.n_batch) {
                n_eval = params.n_batch;
            }
            if (llama_eval(ctx, &embd[i], n_eval, n_past, params.n_threads, params.e_threads)) {
                fprintf(stderr, "%s : failed to eval\n", __func__);
                return 1;
            }
            n_past += n_eval;
        }

        if (embd.size() > 0 && !path_session.empty()) {
            session_tokens.insert(session_tokens.end(), embd.begin(), embd.end());
            n_session_consumed = session_tokens.size();
        }
        
        return 1;
    }
    
    // main generation, includes adding antiprompt at the end
    int applyEmb(bool fastStop = false){
        if (debug) fprintf(stderr, "2");
        // out of user input, sample next token
        const float   temp            = params.temp;
        const int32_t top_k           = params.top_k <= 0 ? llama_n_vocab(ctx) : params.top_k;
        const float   top_p           = params.top_p;
        const float   tfs_z           = params.tfs_z;
        const float   typical_p       = params.typical_p;
        const int32_t repeat_last_n   = params.repeat_last_n < 0 ? n_ctx : params.repeat_last_n;
        const float   repeat_penalty  = params.repeat_penalty;
        const float   alpha_presence  = params.presence_penalty;
        const float   alpha_frequency = params.frequency_penalty;
        const int     mirostat        = params.mirostat;
        const float   mirostat_tau    = params.mirostat_tau;
        const float   mirostat_eta    = params.mirostat_eta;
        const bool    penalize_nl     = params.penalize_nl;

        // optionally save the session on first sample (for faster prompt loading next time)
        if (!path_session.empty() && need_to_save_session && !params.prompt_cache_ro) {
            need_to_save_session = false;
            llama_save_session_file(ctx, path_session.c_str(), session_tokens.data(), session_tokens.size());
        }

        llama_token id = 0;

        {
            auto logits  = llama_get_logits(ctx);
            auto n_vocab = llama_n_vocab(ctx);

            // Apply params.logit_bias map
            for (auto it = params.logit_bias.begin(); it != params.logit_bias.end(); it++) {
                logits[it->first] += it->second;
            }

            std::vector<llama_token_data> candidates;
            candidates.reserve(n_vocab);
            for (llama_token token_id = 0; token_id < n_vocab; token_id++) {
                candidates.emplace_back(llama_token_data{token_id, logits[token_id], 0.0f});
            }

            llama_token_data_array candidates_p = { candidates.data(), candidates.size(), false };
            
            if (ctx_guidance) {
                llama_sample_classifier_free_guidance(ctx, &candidates_p, ctx_guidance, params.cfg_scale);
            }

            // Apply penalties
            float nl_logit = logits[llama_token_nl()];
            auto last_n_repeat = std::min(std::min((int)last_n_tokens.size(), repeat_last_n), n_ctx);
            llama_sample_repetition_penalty(ctx, &candidates_p,
                last_n_tokens.data() + last_n_tokens.size() - last_n_repeat,
                last_n_repeat, repeat_penalty);
            llama_sample_frequency_and_presence_penalties(ctx, &candidates_p,
                last_n_tokens.data() + last_n_tokens.size() - last_n_repeat,
                last_n_repeat, alpha_frequency, alpha_presence);
            if (!penalize_nl) {
                logits[llama_token_nl()] = nl_logit;
            }
            
            if (grammar != NULL) {
                llama_sample_grammar(ctx, &candidates_p, grammar);
            }

            if (temp <= 0) {
                // Greedy sampling
                id = llama_sample_token_greedy(ctx, &candidates_p);
            } else {
                if (mirostat == 1) {
                    static float mirostat_mu = 2.0f * mirostat_tau;
                    const int mirostat_m = 100;
                    llama_sample_temperature(ctx, &candidates_p, temp);
                    id = llama_sample_token_mirostat(ctx, &candidates_p, mirostat_tau, mirostat_eta, mirostat_m, &mirostat_mu);
                } else if (mirostat == 2) {
                    static float mirostat_mu = 2.0f * mirostat_tau;
                    llama_sample_temperature(ctx, &candidates_p, temp);
                    id = llama_sample_token_mirostat_v2(ctx, &candidates_p, mirostat_tau, mirostat_eta, &mirostat_mu);
                } else {
                    // Temperature sampling
                    llama_sample_top_k(ctx, &candidates_p, top_k, 1);
                    llama_sample_tail_free(ctx, &candidates_p, tfs_z, 1);
                    llama_sample_typical(ctx, &candidates_p, typical_p, 1);
                    llama_sample_top_p(ctx, &candidates_p, top_p, 1);
                    llama_sample_temperature(ctx, &candidates_p, temp);
                    id = llama_sample_token(ctx, &candidates_p);
                }
            }
            // printf("`%d`", candidates_p.size);
            if (grammar != NULL) {
                llama_grammar_accept_token(ctx, grammar, id);
            }
            
            last_n_tokens.erase(last_n_tokens.begin());
            last_n_tokens.push_back(id);
        }

        // replace end of text token with newline token when in interactive mode
        // if (id == llama_token_eos() && params.interactive && !params.instruct) {
            // id = llama_token_newline.front();
            // if (params.antiprompt.size() != 0 && !fastStop) {
                // // tokenize and inject first reverse prompt
                // const auto first_antiprompt = ::llama_tokenize(ctx, params.antiprompt.front(), false);
                // embd_inp.insert(embd_inp.end(), first_antiprompt.begin(), first_antiprompt.end());
            // }
        // }

        // add it to the context
        embd.push_back(id);
        ++last_tokens;

        // echo this to console
        input_echo = true;

        // decrement remaining sampling budget
        --n_remain;
        
        return 1;
    }
// main generation end////////////////////////////////////////////////////////////////

    void clearLastEmbd(){
        embd.erase(embd.begin()+last_tokens);
        n_remain += last_tokens;
        last_tokens = 0;
    }
        
    void clearLastTokens(){
        last_tokens = 0;
    }
    

    int inputEmb(){
        
        if (debug) fprintf(stderr, "5");
        
        // checkAntiprompt
        // check for reverse prompt
        if (params.antiprompt.size()) {
            std::string last_output;
            for (auto id : last_n_tokens) {
                last_output += llama_token_to_string(ctx, id);
            }

            is_antiprompt = false;
            // Check if each of the reverse prompts appears at the end of the output.
            // If we're not running interactively, the reverse prompt might be tokenized with some following characters
            // so we'll compensate for that by widening the search window a bit.
            for (std::string & antiprompt : params.antiprompt) {
                size_t extra_padding = params.interactive ? 0 : 2;
                size_t search_start_pos = last_output.length() > static_cast<size_t>(antiprompt.length() + extra_padding)
                    ? last_output.length() - static_cast<size_t>(antiprompt.length() + extra_padding)
                    : 0;

                if (last_output.find(antiprompt.c_str(), search_start_pos) != std::string::npos) {
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
        
        // deal with end of text token in interactive mode
        if (last_n_tokens.back() == llama_token_eos()) {
            if (params.interactive) {
                if (params.antiprompt.size() != 0) {
                    // tokenize and inject first reverse prompt
                    const auto first_antiprompt = ::llama_tokenize(ctx, params.antiprompt.front(), false);
                    embd_inp.insert(embd_inp.end(), first_antiprompt.begin(), first_antiprompt.end());
                    is_antiprompt = true;
                }

                is_interacting = true;
                printf("\n");
            } else if (params.instruct) {
                is_interacting = true;
            }
        }

        // preInputEmb
        if (n_past > 0 && is_interacting) {
            if (debug) fprintf(stderr, ">");
            if (params.instruct) {
                printf("\n> ");
            }
            
            if (params.input_prefix_bos) {
                embd_inp.push_back(llama_token_bos());
            }

            std::string buffer;
            if (!params.input_prefix.empty()) {
                buffer += params.input_prefix;
                printf("%s", buffer.c_str());
            }

            std::string line;

            std::getline(std::cin, line);
            buffer += line + DELIMINER;

            // Add tokens to embd only if the input buffer is non-empty
            // Entering a empty line lets the user pass control back
            if (buffer.length() > 1) {
                if (debug) fprintf(stderr, "<");
                // append input suffix if any
                if (!params.input_suffix.empty()) {
                    buffer += params.input_suffix;
                    printf("%s", params.input_suffix.c_str());
                }

                // instruct mode: insert instruction prefix
                if (params.instruct && !is_antiprompt) {
                    n_consumed = embd_inp.size();
                    printf("|");
                    embd_inp.insert(embd_inp.end(), inp_pfx.begin(), inp_pfx.end());
                }

                auto line_inp = ::llama_tokenize(ctx, buffer, false);
                embd_inp.insert(embd_inp.end(), line_inp.begin(), line_inp.end());

                // instruct mode: insert response suffix
                if (params.instruct) {
                    printf("^");
                    embd_inp.insert(embd_inp.end(), inp_sfx.begin(), inp_sfx.end());
                }

                n_remain -= line_inp.size();
            }

            input_echo = false; // do not echo this again
        }

        if (n_past > 0) {
            if (is_interacting) {
                // reset grammar state if we're restarting generation
                if (grammar != NULL) {
                    llama_grammar_free(grammar);

                    std::vector<const llama_grammar_element *> grammar_rules( parsed_grammar.c_rules());
                    grammar = llama_grammar_init(
                        grammar_rules.data(), grammar_rules.size(),
                        parsed_grammar.symbol_ids.at("root"));
                }
            }
            is_interacting = false;
        }
        
        return 1;
    }
    
    int resetGrammar(){
        if (is_interacting) {
            // reset grammar state if we're restarting generation
            if (grammar != NULL) {
                llama_grammar_free(grammar);

                std::vector<const llama_grammar_element *> grammar_rules(
                    parsed_grammar.c_rules());
                grammar = llama_grammar_init(
                    grammar_rules.data(), grammar_rules.size(),
                    parsed_grammar.symbol_ids.at("root"));
            }
        }
        
        //is_interacting = false;
        
        return 1;
    }
    
    
    // assembled work on context
    int preEmb(bool& fastStop){ // 1 2 3 4
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
                last_n_tokens.erase(last_n_tokens.begin());
                last_n_tokens.push_back(embd_inp[n_consumed]);
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
                //const char* tknChars = llama_token_to_str(ctx, id);
                //const size_t sizeTkn = strlen(tknChars) + 1;
                //wchar_t* tknW = new wchar_t[sizeTkn];
                //mbstowcs(tknW, tknChars, sizeTkn);
                
                //printf("%s", tknW);
                //std::cout << tknChars;
                std::cout << llama_token_to_string(ctx, id);
            }

            fflush(stdout);

        }
        
        // if ((int) embd_inp.size() <= n_consumed) {
            // checkAntiprompt();
        // }
        
        return 1;
    }
    
    // assembled work on context, but with writing into a "results" database of dialog    
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
                last_n_tokens.erase(last_n_tokens.begin());
                last_n_tokens.push_back(embd_inp[n_consumed]);
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
                std::string tknStr = llama_token_to_string(ctx, id); 
                //std::string tknStr = llama_token_to_str(ctx, id).c_str(); 
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
    
    // checking antiprompts 
    int checkAntiprompt(){
        // check for reverse prompt
        if (params.antiprompt.size()) {
            std::string last_output;
            for (auto id : last_n_tokens) {
                last_output += llama_token_to_string(ctx, id);
            }

            is_antiprompt = false;
            // Check if each of the reverse prompts appears at the end of the output.
            // If we're not running interactively, the reverse prompt might be tokenized with some following characters
            // so we'll compensate for that by widening the search window a bit.
            for (std::string & antiprompt : params.antiprompt) {
                size_t extra_padding = params.interactive ? 0 : 2;
                size_t search_start_pos = last_output.length() > static_cast<size_t>(antiprompt.length() + extra_padding)
                    ? last_output.length() - static_cast<size_t>(antiprompt.length() + extra_padding)
                    : 0;

                if (last_output.find(antiprompt.c_str(), search_start_pos) != std::string::npos) {
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
        if (last_n_tokens.back() == llama_token_eos()) {
            if (params.interactive) {
                if (params.antiprompt.size() != 0) {
                    // tokenize and inject first reverse prompt
                    const auto first_antiprompt = ::llama_tokenize(ctx, params.antiprompt.front(), false);
                    embd_inp.insert(embd_inp.end(), first_antiprompt.begin(), first_antiprompt.end());
                    is_antiprompt = true;
                }

                is_interacting = true;
                printf("\n");
            } else if (params.instruct) {
                is_interacting = true;
            }
        }
        
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
            embd_inp.push_back(llama_token_bos());
        }
        
        if (buffer.length() > 1) {
            if (debug) fprintf(stderr, "<");
            // append input suffix if any
            if (!params.input_suffix.empty()) {
                buffer += params.input_suffix;
                printf("%s", params.input_suffix.c_str());
            }

            // instruct mode: insert instruction prefix
            if (params.instruct && !is_antiprompt) {
                n_consumed = embd_inp.size();
                if (debug) printf("|");
                embd_inp.insert(embd_inp.end(), inp_pfx.begin(), inp_pfx.end());
            }

            auto line_inp = ::llama_tokenize(ctx, buffer, false);
            embd_inp.insert(embd_inp.end(), line_inp.begin(), line_inp.end());

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

        return 1;
    }
    
// NEW //////////////////////////////////////////////////////////////////////////////////////////

    std::string generate(bool consoleOutput = true){
        std::string result;
        //std::cout << " * " << std::endl;
        //char* result = new char[2048];
        bool fastStop = false;
        while ((n_remain != 0 && !is_antiprompt) || params.interactive) {
            
            if (consoleOutput) preEmb(result, fastStop, streaming); // includes checking, generation and adding prompt leftovers
            else preEmb(result, fastStop, consoleOutput);
            
            if ((int) embd_inp.size() <= n_consumed) {
                if (debug) fprintf(stderr, "5");
                
                checkAntiprompt();
                
                if (n_past > 0 && is_interacting) {
                    if (!streaming) 
                        std::cout << result << " ";
                    
                    return result;
                }
                
            }
            
        }
        
        

        return "cycle broken!";
    }

    std::string getPrinted(){
        if (debug) fprintf(stderr, "4");
        
        for (auto id : embd) { 
            return llama_token_to_string(ctx, id); 
            //return llama_token_to_str(ctx, id).c_str(); 
        }
    }
    
    void eraseAntiprompt(std::string& result){
        if (params.antiprompt.size()) {
            int cutAntiPos = result.rfind(params.antiprompt[0]);
            if (cutAntiPos != std::string::npos){
                result.erase(cutAntiPos);
            }
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
                last_n_tokens.erase(last_n_tokens.begin());
                last_n_tokens.push_back(embd_inp[n_consumed]);
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
    
    std::string cycleStringsOnly(bool stream = false){
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
                
                checkAntiprompt();
                setEOS();
                
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

