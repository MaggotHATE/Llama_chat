#include "common.h"
#include "llama.h"
#include "grammar-parser.h"
#include "json.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#pragma once

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

nlohmann::json getJson(std::string fileName){
    if (fileName.find(".json") == fileName.npos) fileName += ".json";
    
    nlohmann::json config;
    std::fstream o1(fileName);
    if (o1.is_open()) {
        
        o1 >> std::setw(4) >> config;
        o1.close();
        
        std::cout << "Loaded: " << fileName << std::endl;
    } else {
        config["error"] = "is_open";
        
        std::cout << "Failed to load: " << fileName  << std::endl;
    }
    
    return config;
}

void getParamsFromJson(nlohmann::json& config, gpt_params& params, bool hasFile = false, bool headless = false){
    
    if (checkJString(config, "card")) {
        std::string cardPath = loadNpring(config,"card");
        
        nlohmann::json card = getJson(cardPath);
        
        getParamsFromJson(card, params, hasFile, headless);
    }
    
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
    if (checkJString(config, "input_prefix")) params.input_prefix = loadNpring(config,"input_prefix", true);
    if (checkJString(config, "input_suffix")) params.input_suffix = loadNpring(config,"input_suffix", true);
    if (checkJNum(config, "n_threads")) params.n_threads = loadNpring(config,"n_threads", true);
    if (checkJNum(config, "n_threads_batch")) params.n_threads_batch = loadNpring(config,"n_threads_batch", true);
    if (checkJNum(config, "n_gpu_layers")) params.n_gpu_layers = loadNpring(config,"n_gpu_layers", true);
    if (checkJNum(config, "ctx-size")) params.n_ctx = loadNpring(config,"ctx-size", true);
    if (checkJNum(config, "n_keep")) params.n_keep = loadNpring(config,"n_keep", true);
    if (checkJNum(config, "n_batch")) params.n_batch = loadNpring(config,"n_batch", true);
    if (checkJNum(config, "temp")) params.temp = loadNpring(config,"temp", true);
    if (checkJNum(config, "top_k")) params.top_k = loadNpring(config,"top_k", true);
    if (checkJNum(config, "top_p")) params.top_p = loadNpring(config,"top_p", true);
    if (checkJNum(config, "min_p")) params.min_p = loadNpring(config,"min_p", true);
    if (checkJNum(config, "typical_p")) params.typical_p = loadNpring(config,"typical_p", true);
    if (checkJNum(config, "tfs_z")) params.tfs_z = loadNpring(config,"tfs_z", true);
    if (checkJNum(config, "repeat_penalty")) params.repeat_penalty = loadNpring(config,"repeat_penalty", true);
    if (checkJNum(config, "frequency_penalty")) params.frequency_penalty = loadNpring(config,"frequency_penalty", true);
    if (checkJNum(config, "presence_penalty")) params.presence_penalty = loadNpring(config,"presence_penalty", true);
    if (checkJNum(config, "mirostat")) params.mirostat = loadNpring(config,"mirostat", true);
    if (checkJNum(config, "mirostat_tau")) params.mirostat_tau = loadNpring(config,"mirostat_tau", true);
    if (checkJNum(config, "mirostat_eta")) params.mirostat_eta = loadNpring(config,"mirostat_eta", true);
    //if (config["color"].is_boolean()) params.use_color = loadNpring(config,"color", false);
    if (config["penalize_nl"].is_boolean()) params.penalize_nl = loadNpring(config,"penalize_nl", false);
    if (config["input_prefix_bos"].is_boolean()) params.input_prefix_bos = loadNpring(config,"input_prefix_bos", false);
    
    if (checkJString(config, "grammar")) params.grammar = config["grammar"];
    if (checkJString(config, "grammar-file")) readGrammarFile(params, config["grammar-file"]);
    
    #ifdef GGML_OLD_FORMAT
    if (checkJNum(config, "rms-norm-eps")) params.rms_norm_eps = loadNpring(config,"rms-norm-eps", true);
    #endif
    
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
    
    if(config["model"].is_string() && config[params.model].is_object()){
        std::cout << "Found settings for model " << params.model << std::endl;
        nlohmann::json modelConfig = config[params.model];
        bool hasFiles = modelConfig["file"].is_string() || config["file"].is_string();
        getParamsFromJson(modelConfig, params, hasFiles, headless);
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

unsigned int getRand(){
    std::srand(std::time(nullptr));
    if (std::rand() > std::rand()){
        return std::rand()*std::rand();
    } else return std::rand();
}

std::string processByWildcards(nlohmann::json& config, std::string inputString){
    
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