#pragma once

#include "common.h"
#include "llama.h"
#include "grammar-parser.h"
#include "json.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <random>

static void readGrammarFile(gpt_params& params, std::string fileName){
    std::ifstream file(fileName);
    if (!file) {
        fprintf(stderr, "error: failed to open file '%s'\n", fileName);
    } else {
        std::copy(
            std::istreambuf_iterator<char>(file),
            std::istreambuf_iterator<char>(),
            std::back_inserter(params.sparams.grammar)
        );
    }
}

static void processInstructFile(std::string filename, gpt_params& params, bool headless = false){
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

static std::string getInlineFileText(std::string filename) {
    std::string result = "";
    std::ifstream file(filename);
    if (!file) {
        fprintf(stderr, "error: failed to open file '%s'\n", filename.c_str());
    } else {
        fprintf(stderr, "Opening file '%s'...\n", filename.c_str());
        std::copy(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>(), back_inserter(result));
    }
    
    return result;
}

static void processPrompt(std::string& prompt, char open = '<', char close = '>') {
    int fileClose = prompt.rfind(close);
    int fileOpen = prompt.rfind(open);
    int len = fileClose - fileOpen;
    //fprintf(stderr, "Between %d and %d, len %d\n", fileOpen, fileClose, len);
    if(fileClose != prompt.npos && fileOpen != prompt.npos && len > 1){
        std::string filename = prompt.substr(fileOpen+1,len-1);
        //fprintf(stderr, "Looking for file '%s'\n", filename.c_str());
        std::string result = getInlineFileText(filename);
        processPrompt(result);
        if (!result.empty()) prompt.replace(fileOpen,len+1,result);
    }
}

static bool checkJString(nlohmann::json& config, std::string name){
    if(config.contains(name)){
        if(config[name].is_string()) return true;
    }
    
    return false;
}

static bool checkJNum(nlohmann::json& config, std::string name){
    if(config.contains(name)){
        if(config[name].is_number()) return true;
    }
    
    return false;
}

static nlohmann::json getJson(std::string fileName){
    if (fileName.find(".json") == fileName.npos) fileName += ".json";
    std::cout << "Opening json " << fileName << std::endl;
    nlohmann::json config;
    std::fstream o1(fileName);
    if (o1.is_open()) {
        o1 >> std::setw(4) >> config;
        o1.close();
    } else {
        std::cout << fileName << " not found!" << std::endl;
        config["error"] = "is_open";
    }
    
    return config;
}

static std::string getText(std::string filename){
    std::string result;
    
    std::ifstream file(filename);
    if (file) {
        std::copy(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>(), back_inserter(result));
    }
    
    return result;
}

static std::string formatPrompt(nlohmann::json& config) {
    std::string format = "";
    if (checkJString(config, "format")) {
        format = config["format"];
        size_t systemPos = format.find("system_message");
        if (systemPos != format.npos && checkJString(config, "system_message")){
            std::string system_message = config["system_message"];
            processPrompt(system_message);
            format.replace(systemPos,14,system_message);
        }
        
        // size_t promptPos = format.find("prompt");
        // if (promptPos != format.npos && checkJString(config, "prompt")){
            // std::string prompt = config["prompt"];
            // format.replace(promptPos,6,prompt);
        // }
    }
    
    return format;
}

static void getParamsFromJson(nlohmann::json& config, gpt_params& params, bool hasFile = false, bool headless = false){
    
    if (checkJString(config, "file")) {
        processInstructFile(config["file"], params, headless);
        processPrompt(params.prompt);
    } else if (!hasFile) {
        if (checkJString(config, "format")) {
            params.prompt = formatPrompt(config);
        } else if (checkJString(config, "prompt")) {
            params.prompt = config["prompt"];
            processPrompt(params.prompt);
        }
        
        if (checkJString(config, "reverse-prompt")){
            if(!params.antiprompt.size()) 
                params.antiprompt.push_back(config["reverse-prompt"]);
            else
                params.antiprompt[0] = config["reverse-prompt"];
        }
    }
    
    if (checkJString(config, "model")) params.model = config["model"];
    if (checkJString(config, "cfg-negative-prompt")) params.sparams.cfg_negative_prompt = config["cfg-negative-prompt"];
    if (checkJNum(config, "cfg-scale")) params.sparams.cfg_scale = config["cfg-scale"];
    //if (config["cfg-smooth-factor"].is_number()) params.cfg_smooth_factor = config["cfg-smooth-factor", false);
    if (checkJString(config, "lora")) {
        params.lora_adapter.push_back(std::tuple<std::string, float>(config["lora"], 1.0));
        params.use_mmap = false;
    }
    if (checkJString(config, "input_prefix")) params.input_prefix = config["input_prefix"];
    if (checkJString(config, "input_suffix")) params.input_suffix = config["input_suffix"];
    if (checkJString(config, "format_instruct")) params.format_instruct = config["format_instruct"];
    if (checkJString(config, "format_dialog")) params.format_dialog = config["format_dialog"];
    if (checkJString(config, "samplers_sequence")) params.sparams.samplers_sequence = config["samplers_sequence"];
    if (checkJString(config, "bos")) params.bos = config["bos"];
    if (checkJString(config, "eos")) params.eos = config["eos"];
    
    if (checkJNum(config, "seed")) params.seed = config["seed"];
    if (checkJNum(config, "n_threads")) params.n_threads = config["n_threads"];
    if (checkJNum(config, "n_threads_batch")) params.n_threads_batch = config["n_threads_batch"];
    if (checkJNum(config, "n_gpu_layers")) params.n_gpu_layers = config["n_gpu_layers"];
    if (checkJNum(config, "ctx-size")) params.n_ctx = config["ctx-size"];
    if (checkJNum(config, "grp_attn_n")) params.grp_attn_n = config["grp_attn_n"];
    if (checkJNum(config, "grp_attn_w")) params.grp_attn_w = config["grp_attn_w"];
    if (checkJNum(config, "n_keep")) params.n_keep = config["n_keep"];
    if (checkJNum(config, "n_batch")) params.n_batch = config["n_batch"];
    if (checkJNum(config, "temp")) params.sparams.temp = config["temp"];
    if (checkJNum(config, "top_k")) params.sparams.top_k = config["top_k"];
    if (checkJNum(config, "top_p")) params.sparams.top_p = config["top_p"];
    if (checkJNum(config, "min_p")) params.sparams.min_p = config["min_p"];
    if (checkJNum(config, "typical_p")) params.sparams.typical_p = config["typical_p"];
    if (checkJNum(config, "tfs_z")) params.sparams.tfs_z = config["tfs_z"];
    if (checkJNum(config, "repeat_penalty")) params.sparams.penalty_repeat = config["repeat_penalty"];
    if (checkJNum(config, "frequency_penalty")) params.sparams.penalty_freq = config["frequency_penalty"];
    if (checkJNum(config, "presence_penalty")) params.sparams.penalty_present = config["presence_penalty"];
    if (checkJNum(config, "mirostat")) params.sparams.mirostat = config["mirostat"];
    if (checkJNum(config, "mirostat_tau")) params.sparams.mirostat_tau = config["mirostat_tau"];
    if (checkJNum(config, "mirostat_eta")) params.sparams.mirostat_eta = config["mirostat_eta"];
    //if (config["color"].is_boolean()) params.use_color = config["color"];
    if (config["penalize_nl"].is_boolean()) params.sparams.penalize_nl = config["penalize_nl"];
    if (config["use_mmap"].is_boolean()) params.use_mmap = config["use_mmap"];
    if (config["input_prefix_bos"].is_boolean()) params.input_prefix_bos = config["input_prefix_bos"];
    
    if (checkJString(config, "grammar")) params.sparams.grammar = config["grammar"];
    if (checkJString(config, "grammar-file")) readGrammarFile(params, config["grammar-file"]);
    
    #ifdef GGML_OLD_FORMAT
    if (checkJNum(config, "rms-norm-eps")) params.rms_norm_eps = config["rms-norm-eps"];
    #endif
    
    if (checkJNum(config, "rope_freq_base")) params.rope_freq_base = config["rope_freq_base"];
    if (checkJNum(config, "rope_freq_scale")) params.rope_freq_scale = config["rope_freq_scale"];
    
    if (checkJNum(config, "yarn_orig_ctx")) params.yarn_orig_ctx = config["yarn_orig_ctx"];
    if (checkJNum(config, "yarn_attn_factor")) params.yarn_attn_factor = config["yarn_attn_factor"];
    if (checkJNum(config, "yarn_beta_slow")) params.yarn_beta_slow = config["yarn_beta_slow"];
    
    if (checkJString(config, "rope_scaling_type")) {
        if (config["rope_scaling_type"] == "none")   { params.rope_scaling_type = LLAMA_ROPE_SCALING_NONE; }
        else if (config["rope_scaling_type"] == "linear") { params.rope_scaling_type = LLAMA_ROPE_SCALING_LINEAR; }
        else if (config["rope_scaling_type"] == "yarn")   { params.rope_scaling_type = LLAMA_ROPE_SCALING_YARN; }
    }
    
    std::cout << "headless: " << headless << std::endl;
    
    // separated for better compatibility and less redundancy
    // if (checkJString(config, "card")) {
        // std::string cardPath = config["card"];
        
        // nlohmann::json card = getJson(cardPath);
        
        // getParamsFromJson(card, params, hasFile, headless);
    // }
    
    // if (checkJString(config, "preset")) {
        // std::string presetPath = config["preset"];
        
        // nlohmann::json preset = getJson(presetPath);
        
        // getParamsFromJson(preset, params, hasFile, headless);
    // }
}

static void getParamsFromPreset(nlohmann::json& config, gpt_params& params, bool hasFile = false, bool headless = false){
    
    if (checkJString(config, "card")) {
        std::string cardPath = config["card"];
        
        nlohmann::json card = getJson(cardPath);
        
        getParamsFromJson(card, params, hasFile, headless);
    }
    
    if (checkJString(config, "preset")) {
        std::string presetPath = config["preset"];
        
        nlohmann::json preset = getJson(presetPath);
        
        getParamsFromJson(preset, params, hasFile, headless);
    }
}

static void readParamsFromJson(nlohmann::json& config, gpt_params& params, bool headless = false){
    
    getParamsFromJson(config, params, false, headless);
    getParamsFromPreset(config, params, false, headless);
    
    if(config.contains(params.model) && config[params.model].is_object()){
        std::cout << "Found settings for model " << params.model << std::endl;
        nlohmann::json modelConfig = config[params.model];
        bool hasFiles = modelConfig["file"].is_string() || config["file"].is_string();
        getParamsFromJson(modelConfig, params, hasFiles, headless);
        getParamsFromPreset(modelConfig, params, false, headless);
    }
    
}

static void readParamsFromJson(nlohmann::json& config, std::string modelName, gpt_params& params, bool headless = false){
    
    getParamsFromJson(config, params, false, headless);
    getParamsFromPreset(config, params, false, headless);
    params.model = modelName;
    
    if(config.contains("model")){
        if(config["model"].is_string() && config[params.model].is_object()){
            std::cout << "Found settings for model " << params.model << std::endl;
            nlohmann::json modelConfig = config[params.model];
            bool hasFiles = modelConfig["file"].is_string() || config["file"].is_string();
            getParamsFromJson(modelConfig, params, hasFiles, headless);
            getParamsFromPreset(modelConfig, params, false, headless);
        }
    }
    

}

static void readParamsFromFile(std::string fimeName, gpt_params& params, bool headless = false){
    std::fstream o1(fimeName);
    if (o1.is_open()) {
        nlohmann::json config;
        o1 >> std::setw(4) >> config;
        
        getParamsFromJson(config, params, false, headless);
        getParamsFromPreset(config, params, false, headless);
        
        if(config.contains("model")){
            if(config["model"].is_string() && config[params.model].is_object()){
                std::cout << "Found settings for model " << params.model << std::endl;
                nlohmann::json modelConfig = config[params.model];
                bool hasFiles = modelConfig["file"].is_string() || config["file"].is_string();
                getParamsFromJson(modelConfig, params, hasFiles, headless);
                getParamsFromPreset(modelConfig, params, false, headless);
            }
        }
        
        
        o1.close();
    }
}

static void readParamsFromFile(std::string fimeName, std::string modelName, gpt_params& params, bool headless = false ){
    std::fstream o1(fimeName);
    if (o1.is_open()) {
        nlohmann::json config;
        o1 >> std::setw(4) >> config;
        
        getParamsFromJson(config, params, false, headless);
        getParamsFromPreset(config, params, false, headless);
        params.model = modelName;
        
        if(config.contains(modelName)){
            if(config[modelName].is_object()){
                std::cout << "Found settings for model " << params.model << std::endl;
                nlohmann::json modelConfig = config[params.model];
                bool hasFiles = modelConfig["file"].is_string() || config["file"].is_string();
                getParamsFromJson(modelConfig, params, hasFiles, headless);
                getParamsFromPreset(modelConfig, params, false, headless);
            }
        }
        
        
        o1.close();
    }
}

static bool writeTextFile(std::string path, std::string text){
    std::ofstream file(path, std::ios::app);
    if (file.is_open()) {
        file << text;
        file.close();
        return true;
    } else {
        return false;
    }
}

static bool writeTextFile(std::string path, std::vector<char*>& texts){
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

static bool writeTextFileOver(std::string path, std::string text){
    std::ofstream file(path);
    if (file.is_open()) {
        file << text;
        file.close();
        return true;
    } else {
        return false;
    }
}

static bool writeTextFileOver(std::string path, std::vector<char*>& texts){
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

static void writeJson(nlohmann::json config, std::string filename) {
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



static unsigned int getRand(){
    // std::srand(std::time(nullptr));
    // if (std::rand() > std::rand()){
        // return std::rand()*std::rand();
    // } else return std::rand();
    //std::random_device rd;
    //return rd();
    
    std::random_device rd;
    std::seed_seq sseq ({rd()%1000,rd()%10000,rd()%100000});
    std::knuth_b kr(sseq);
    
    return kr();
}

static std::string processByWildcards(nlohmann::json& config, std::string inputString){
    
    size_t last = inputString.rfind("__");
    if (last != inputString.npos){
        
        std::string subInputString = inputString.substr(0,last);
        std::cout << " subInputString: " << subInputString << std::endl;
        size_t first = subInputString.rfind("__");
        if (first != subInputString.npos){
            std::string wildcard = subInputString.substr(first + 2);
            
            if(config.contains(wildcard)) {
                
                std::vector<std::string> wildcardsArr = config[wildcard];
                unsigned int i = getRand() % wildcardsArr.size();
                std::string choice = wildcardsArr[i];
                inputString.replace(first,last - first + 2,choice);
                std::cout << "Replaced: " << inputString << std::endl;
                
                return processByWildcards(config, inputString);
                
            } else std::cout << " no wildcard: " << wildcard << std::endl;
        } else std::cout << " no first __" << std::endl;
    } else std::cout << " no last __" << std::endl;
        
    return inputString;
}

static void sanitizePath(std::string& path){
    int slashes = path.rfind("\\");
    while (slashes != path.npos){
        path.replace(slashes,1,"/");
        slashes = path.rfind("\\");
    }
}

static std::string getFileWithSameName(std::string input, std::string type){
    std::string result;

    sanitizePath(input);
    size_t lastDot = input.rfind('.');
    size_t lastSlash = input.rfind('/');
    if (lastDot != input.npos){
        result = input.substr(lastSlash + 1, lastDot - lastSlash - 1) + type;
    }
    
    return result;
}