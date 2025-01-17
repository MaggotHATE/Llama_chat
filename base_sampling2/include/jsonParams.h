#pragma once

#include "common.h"
#include "sampling.h"
//#include "llama.h"
#define JSON_ASSERT GGML_ASSERT
#include "json.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <random>
#include <format>
#include <chrono>

#if defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
#define DELIMINER '\r'
#elif defined (_WIN32)
#define DELIMINER '\n'
#endif

static std::string extract_string(std::string& text, std::string open, std::string close) {
    size_t open_pos = text.rfind(open);
    size_t close_pos = text.rfind(close);
    if (open_pos != text.npos && close_pos != text.npos) {
        size_t diff = close_pos - open_pos - open.length();
        return text.substr(open_pos + open.length(), diff);
    }

    return "NULL";
}

static std::string extract_string_mod(std::string& text, std::string open, std::string close) {
    size_t open_pos = text.rfind(open);
    size_t close_pos = text.rfind(close);
    if (open_pos != text.npos && close_pos != text.npos) {
        size_t diff = close_pos - open_pos - open.length();
        std::string extract = text.substr(open_pos + open.length(), diff);
        std::cout << "Extracting: " << extract << std::endl;
        text.replace(open_pos,diff + open.length() + close.length(),"");
        return extract;
    }

    return "NULL";
}

static bool replace_string_mod(std::string& text, std::string target, std::string replacement) {
    size_t target_pos = text.rfind(target);

    if (target_pos != text.npos) {
        text.replace(target_pos,target.length(), replacement);
        return true;
    }

    return false;
}

static void readGrammarFile(common_params& params, std::string fileName){
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

static void processInstructFile(std::string filename, common_params& params, bool headless = false){
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
                    params.antiprompt.emplace_back(params.prompt.substr(antiPos));
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

static void processFormat(std::string& prompt, std::string& replacement, char open = '<', char close = '>') {
    int stringClose = prompt.rfind(close);
    int stringOpen = prompt.rfind(open);
    int len = stringClose - stringOpen;
    //fprintf(stderr, "Between %d and %d, len %d\n", stringOpen, stringClose, len);
    if(stringClose != prompt.npos && stringOpen != prompt.npos && len > 1){
        processPrompt(replacement);
        prompt.replace(stringOpen,len+1,replacement);
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

static bool checkJBool(nlohmann::json& config, std::string name){
    if(config.contains(name)){
        if(config[name].is_boolean()) return true;
    }
    
    return false;
}

static bool checkJObj(nlohmann::json& config, std::string name){
    if(config.contains(name)){
        if(config[name].is_object()) {
            std::cout << name << " object found!" << std::endl;
            return true;
        }
    }
    
    return false;
}

static bool checkJArr(nlohmann::json& config, std::string name){
    if(config.contains(name)){
        if(config[name].is_array()) {
            std::cout << name << " array found!" << std::endl;
            return true;
        }
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

static std::string getText(std::string filename) {
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

static void text_fill(std::string& text, std::string tag, std::string fill, std::string open, std::string close) {
    // example: {{char}} would require "{{" and "}}"
    std::string text_tmp = text;
    size_t open_pos = text_tmp.rfind(open);
    size_t close_pos = text_tmp.rfind(close);
    
    while (open_pos != text_tmp.npos && close_pos != text_tmp.npos) {
        size_t diff = close_pos - open_pos - open.length();
        if (diff > 0) {
            std::string result_tag = text_tmp.substr(open_pos + open.length(),diff);
            if (result_tag == tag) { 
                std::cout << std::format("FOUND: {} - {} = {}, {} to {}", open_pos, close_pos, diff, result_tag, tag) << std::endl;
                text.replace(open_pos,diff + open.length() + close.length(),fill);
                //std::cout << text << std::endl;
            } 
            // else {
                // std::cout << std::format("SKIPPED: {} - {} = {}, {} to {}", open_pos, close_pos, diff, result_tag, tag) << std::endl;
                
            // }
        }

        text_tmp.replace(open_pos,open.length(),"{}");
        text_tmp.replace(close_pos,close.length(),"{}");
        open_pos = text_tmp.rfind(open);
        close_pos = text_tmp.rfind(close);
    }
}

static void sanitize_deliminers(std::string& text) {
    size_t delim_pair = text.find("\r\n");
    while (delim_pair != text.npos) {
        text.replace(delim_pair,2,""+DELIMINER);
        delim_pair = text.find("\r\n");
    }
}

static void process_STcard(std::string& text, std::string card_path, std::string user, std::string char_name = "Character"){
    nlohmann::json st_card = getJson(card_path);

    if (st_card.contains("data")) {

        std::vector<std::string> data_tags = {"personality", "post_history_instructions", "scenario","description", "first_mes", "mes_example"}; //temp solution until I'm used to the chaos of SillyTavern cards

        nlohmann::json st_card_data = st_card["data"];
        std::cout << "Data found" << std::endl;

        text_fill(text, "user", user, "{{", "}}");

        if (checkJString(st_card_data, "name")) {
            std::cout << "name found" << std::endl;
            char_name = st_card_data["name"];
            text_fill(text, "char", char_name, "{{", "}}");
        }
        
        for (auto tag : data_tags) {
            std::cout << "\n Checking " << tag << std::endl;
            
            if (checkJString(st_card_data, tag)) {
                std::cout << tag << " found" << std::endl;
                std::string tag_val = st_card_data[tag];

                std::cout << "\n " << tag << ":\n" << tag_val << std::endl;

                text_fill(tag_val, "original", " ", "{{", "}}");
                text_fill(tag_val, "char", char_name, "{{", "}}");
                text_fill(tag_val, "user", user, "{{", "}}");
                std::cout << "\n  " << tag << ":\n" << tag_val << std::endl;

                text_fill(text, tag, tag_val, "{{", "}}");
            }
        }

        std::cout << '\n' << text << std::endl;

    }

}

static void process_STcard_Anytag(std::string& text, std::string card_path, std::string user, std::string char_name = "Character"){
    nlohmann::json st_card = getJson(card_path);

    if (st_card.contains("data")) {

        nlohmann::json st_card_data = st_card["data"];
        std::cout << "Data found" << std::endl;

        text_fill(text, "user", user, "{{", "}}");

        if (checkJString(st_card_data, "name")) {
            std::cout << "name found" << std::endl;
            char_name = st_card_data["name"];
            text_fill(text, "char", char_name, "{{", "}}");
        }
        
        for (auto tag : st_card_data.items()) {
            std::cout << "\n Found " << tag.key() << std::endl;

            std::string tag_key = tag.key();
            std::string tag_val = tag.value();

            std::cout << "\n " << tag_key << ":\n" << tag_val << std::endl;

            text_fill(tag_val, "original", " ", "{{", "}}");
            text_fill(tag_val, "char", char_name, "{{", "}}");
            text_fill(tag_val, "user", user, "{{", "}}");
            std::cout << "\n  " << tag_key << ":\n" << tag_val << std::endl;

            text_fill(text, tag_key, tag_val, "{{", "}}");
        }

        std::cout << '\n' << text << std::endl;

    }

}

static void processFormatTemplate(nlohmann::json& config, common_params& params) {
    std::string format_template = "";
    if (checkJString(config, "format_file")) {
        format_template = getText(config["format_file"]);
        
        if (checkJString(config, "st_card") && checkJString(config, "user")) process_STcard(format_template, config["st_card"], config["user"]);
        
        size_t systemPos = format_template.find("{system_message}");
        size_t promptPos = format_template.find("{prompt}");
        
        if (promptPos != format_template.npos) {
            
            std::string antiprompt;
            if (systemPos != format_template.npos) antiprompt = format_template.substr(systemPos + 16, promptPos - systemPos - 16);
            else { 
                antiprompt = format_template.substr(0, promptPos);
            }
            
            if (!antiprompt.empty()) {
                // additional safeguard
                size_t delimPos = antiprompt.find('\n');
                if (delimPos != antiprompt.npos && delimPos != (antiprompt.size() - 1) ) antiprompt = antiprompt.substr(delimPos);
                
                if(!params.antiprompt.size())
                    params.antiprompt.emplace_back(antiprompt);
                else
                    params.antiprompt[0] = antiprompt;
            }
            
            params.prompt = format_template.substr(0,promptPos);
            params.input_suffix = format_template.substr(promptPos+8);
            
            if (checkJString(config, "system_message")) {
                std::string system_message = config["system_message"];
                processFormat(params.prompt, system_message, '{','}');
            }
        }
        
    }
}

static void promptFileLink(std::string& text, std::string open, std::string close) {
    std::string text_tmp = text;
    size_t open_pos = text_tmp.rfind(open);
    size_t close_pos = text_tmp.rfind(close);
    
    while (open_pos != text_tmp.npos && close_pos != text_tmp.npos) {
        size_t diff = close_pos - open_pos - open.length();
        if (diff > 0) {
            std::string result_tag = text_tmp.substr(open_pos + open.length(),diff);
            std::string result_text = getText(result_tag);
            if (result_text.length() > 0) {
                text.replace(open_pos,diff + open.length() + close.length(),result_text);
            }

        }

        text_tmp.replace(open_pos,open.length(),"{}");
        text_tmp.replace(close_pos,close.length(),"{}");
        open_pos = text_tmp.rfind(open);
        close_pos = text_tmp.rfind(close);
    }
}

static void load_param_func(nlohmann::json& config, std::string param_name, float& param, llama_sampling_param_func& param_func) {
    if (checkJObj(config, param_name)) {
        std::cout << "Found object: " << param_name << std::endl;
        if (checkJNum(config[param_name], "value")) param = config[param_name]["value"];
        if (checkJNum(config[param_name], "p_min")) param_func.p_min = config[param_name]["p_min"];
        if (checkJNum(config[param_name], "p_max")) param_func.p_max = config[param_name]["p_max"];
        if (checkJNum(config[param_name], "p_add")) param_func.p_add = config[param_name]["p_add"];
        if (checkJNum(config[param_name], "p_mul")) param_func.p_mul = config[param_name]["p_mul"];
        std::cout << "Object loaded! " << std::endl;
    }
}

static void load_param_num(nlohmann::json& config, std::string param_name, float& param, llama_sampling_param_func& param_func) {
    if (checkJObj(config, param_name)) load_param_func(config, param_name, param, param_func);
    else if (checkJNum(config, param_name)) param = config[param_name];
}

static std::string msg_formatting(std::vector<std::pair<std::string, std::string>> messages, std::string order_sys, std::string order_msg, std::string presystem, std::string postsystem, std::string preinput, std::string postinput, std::string preoutput, std::string postoutput, int message_idx) {
    std::string formatted_result = "";
    int idx = 0;

    for (auto s : order_sys) {
        switch (s) {
            case '-': formatted_result += '\n'; break;
            case '!': formatted_result += messages[idx].second; ++idx; break;
            case '1': formatted_result += presystem; break;
            case '2': formatted_result += postsystem; break;
            default: break;
        }
    }

    while (idx < message_idx) {
        for (auto s : order_msg) {
            switch (s) {
                case '-': formatted_result += '\n'; break;
                case '!': formatted_result += messages[idx].second; ++idx; break;
                case '3': formatted_result += preinput; break;
                case '4': formatted_result += postinput; break;
                case '5': formatted_result += preoutput; break;
                case '6': formatted_result += postoutput; break;
                default: break;
            }
        }
    }

    return formatted_result;
}

static std::string get_last_formatted(std::vector<std::pair<std::string, std::string>> messages, std::string order_sys, std::string order_msg, std::string presystem, std::string postsystem, std::string preinput, std::string postinput, std::string preoutput, std::string postoutput) {
    std::string total = msg_formatting(messages, order_sys, order_msg, presystem, postsystem, preinput, postinput, preoutput, postoutput, messages.size());
    std::string previous = msg_formatting(messages, order_sys, order_msg, presystem, postsystem, preinput, postinput, preoutput, postoutput, messages.size() - 1);
    
    return total.substr(previous.size(), total.size() - previous.size());
}

static void getSamplingParamsFromJson(nlohmann::json& config, common_params& params) {
// general sampling
    if (checkJString(config, "samplers_sequence")) params.sparams.samplers_sequence = config["samplers_sequence"];
// confidence
    if (checkJNum(config, "confidence_shift")) params.sparams.confidence_shift = config["confidence_shift"];
    if (checkJNum(config, "confidence_top")) params.sparams.confidence_top = config["confidence_top"];
    if (checkJNum(config, "confidence_bottom")) params.sparams.confidence_bottom = config["confidence_bottom"];

// samplers
  // temp
    load_param_num(config, "temp", params.sparams.temp, params.sparams.temp_func);
    load_param_num(config, "dynatemp_range", params.sparams.dynatemp_range, params.sparams.dynatemp_range_func);
    if (checkJNum(config, "temp_smoothing")) params.sparams.smoothing_factor = config["temp_smoothing"];
    if (checkJNum(config, "smoothing_factor")) params.sparams.smoothing_factor = config["smoothing_factor"];
    if (checkJNum(config, "smoothing_curve")) params.sparams.smoothing_curve = config["smoothing_curve"];
    if (checkJBool(config, "temp_adaptive")) params.sparams.temp_adaptive = config["temp_adaptive"];
  // top-k
    if (checkJNum(config, "top_k")) params.sparams.top_k = config["top_k"];
    if (checkJNum(config, "k_shift")) params.sparams.k_shift = config["k_shift"];
  // top-p
    if (checkJNum(config, "top_p")) params.sparams.top_p = config["top_p"];
  // typical
     if (checkJNum(config, "typical_p")) params.sparams.typical_p = config["typical_p"];
  // tfs-z
    if (checkJNum(config, "tfs_z")) params.sparams.tfs_z = config["tfs_z"];
  // min-p
    if (checkJNum(config, "min_p")) params.sparams.min_p = config["min_p"];
    if (checkJNum(config, "min_p_rand")) params.sparams.min_p_rand = config["min_p_rand"];
  // noise
    if (checkJNum(config, "noise_min")) params.sparams.noise_min = config["noise_min"];
    if (checkJNum(config, "noise_max")) params.sparams.noise_max = config["noise_max"];
  // range-based exclusion
    if (checkJNum(config, "range_min")) params.sparams.range_min = config["range_min"];
    if (checkJNum(config, "range_max")) params.sparams.range_max = config["range_max"];
  // p-step
    //if (checkJNum(config, "p_step")) params.sparams.p_step = config["p_step"];
    load_param_num(config, "p_step", params.sparams.p_step, params.sparams.p_step_func);
  // xtc
    if (checkJNum(config, "xtc_probability")) params.sparams.xtc_probability = config["xtc_probability"];
    if (checkJNum(config, "xtc_threshold")) params.sparams.xtc_threshold = config["xtc_threshold"];
    if (checkJNum(config, "xtc_threshold_max")) params.sparams.xtc_threshold_max = config["xtc_threshold_max"];
    if (checkJNum(config, "xtc_min")) params.sparams.xtc_min = config["xtc_min"];
    if (checkJBool(config, "xtc_probability_once")) params.sparams.xtc_probability_once = config["xtc_probability_once"];
  // top_n_sigma
    if (checkJNum(config, "top_n_sigma")) params.sparams.top_n_sigma = config["top_n_sigma"];

//penalties
    if (checkJNum(config, "repeat_penalty")) params.sparams.penalty_repeat = config["repeat_penalty"];
    if (checkJNum(config, "penalty_repeat")) params.sparams.penalty_repeat = config["penalty_repeat"];
    if (checkJNum(config, "penalty_threshold")) params.sparams.penalty_threshold = config["penalty_threshold"];
    if (checkJNum(config, "frequency_penalty")) params.sparams.penalty_freq = config["frequency_penalty"];
    if (checkJNum(config, "presence_penalty")) params.sparams.penalty_present = config["presence_penalty"];
    if (checkJNum(config, "penalty_last_n")) params.sparams.penalty_last_n = config["penalty_last_n"];

//DRY
    if (checkJNum(config, "dry_multiplier")) params.sparams.dry_multiplier = config["dry_multiplier"];
    if (checkJNum(config, "dry_base")) params.sparams.dry_base = config["dry_base"];
    if (checkJNum(config, "dry_allowed_length")) params.sparams.dry_allowed_length = config["dry_allowed_length"];
    if (checkJNum(config, "dry_penalty_last_n")) params.sparams.dry_penalty_last_n = config["dry_penalty_last_n"];

//mirostat
    if (checkJNum(config, "mirostat")) params.sparams.mirostat = config["mirostat"];
    if (checkJNum(config, "mirostat_tau")) params.sparams.mirostat_tau = config["mirostat_tau"];
    if (checkJNum(config, "mirostat_eta")) params.sparams.mirostat_eta = config["mirostat_eta"];

}

static void getPromptingParamsFromJson(nlohmann::json& config, common_params& params, bool hasFile = false, bool headless = false) {
    if (checkJBool(config, "headless")) headless = true;
    if (checkJString(config, "file")) {
        processInstructFile(config["file"], params, headless);
        processPrompt(params.prompt);
    } else if (!hasFile) {
        if (checkJString(config, "format_file")) {
            processFormatTemplate(config, params);
            if (checkJString(config, "st_card") && checkJString(config, "user")) {
                process_STcard_Anytag(params.prompt, config["st_card"], config["user"]);
            } else if (checkJString(config, "char")) {
                text_fill(params.prompt, "char", config["char"], "{{", "}}");
            }
        } else {
            if (checkJString(config, "format")) {
                params.prompt = formatPrompt(config);
            } else if (checkJString(config, "prompt")) {
                params.prompt = config["prompt"];
                processPrompt(params.prompt);
                promptFileLink(params.prompt, "{{{", "}}}");

                if (checkJString(config, "st_card") && checkJString(config, "user")) {
                    process_STcard_Anytag(params.prompt, config["st_card"], config["user"]);
                } else if (checkJString(config, "char")) {
                    text_fill(params.prompt, "char", config["char"], "{{", "}}");
                } else promptFileLink(params.prompt, "{{", "}}");
            }

            if (checkJString(config, "reverse-prompt")){
                if(!params.antiprompt.size()) 
                    params.antiprompt.emplace_back(config["reverse-prompt"]);
                else
                    params.antiprompt[0] = config["reverse-prompt"];
                
                if (checkJString(config, "st_card") && checkJString(config, "user")) {
                    process_STcard_Anytag(params.antiprompt[0], config["st_card"], config["user"]);
                } else if (checkJString(config, "char")) {
                    text_fill(params.antiprompt[0], "char", config["char"], "{{", "}}");
                }
            }

            if (checkJString(config, "input_prefix")) {
                params.input_prefix = config["input_prefix"];
                if (checkJString(config, "st_card") && checkJString(config, "user")) {
                    process_STcard_Anytag(params.input_prefix, config["st_card"], config["user"]);
                } else if (checkJString(config, "char")) {
                    text_fill(params.input_prefix, "char", config["char"], "{{", "}}");
                }
            }

            if (checkJString(config, "input_suffix")) {
                params.input_suffix = config["input_suffix"];
                if (checkJString(config, "st_card") && checkJString(config, "user")) {
                    process_STcard_Anytag(params.input_suffix, config["st_card"], config["user"]);
                } else if (checkJString(config, "char")) {
                    text_fill(params.input_suffix, "char", config["char"], "{{", "}}");
                }
            }
        }
        
    }

    if (checkJString(config, "format_instruct")) params.format_instruct = config["format_instruct"];
    if (checkJString(config, "format_dialog")) params.format_dialog = config["format_dialog"];
    if (checkJString(config, "bos")) params.bos = config["bos"];
    if (checkJString(config, "eos")) params.eos = config["eos"];
    if (checkJNum(config, "seed")) params.sparams.seed = config["seed"];

    std::cout << "headless: " << headless << std::endl;

}

static void getPerformanceParamsFromJson(nlohmann::json& config, common_params& params) {
// threaded inference
    if (checkJNum(config, "n_threads")) params.cpuparams.n_threads = config["n_threads"];
    if (checkJNum(config, "n_threads_poll")) params.cpuparams.poll = config["n_threads_poll"];
    if (checkJNum(config, "n_threads_sched_priority")) {
        int sched_priority = config["n_threads_sched_priority"];
        switch (sched_priority){
            case 1: params.cpuparams.priority = GGML_SCHED_PRIO_MEDIUM; break;
            case 2: params.cpuparams.priority = GGML_SCHED_PRIO_HIGH; break;
            case 3: params.cpuparams.priority = GGML_SCHED_PRIO_REALTIME; break;
            default: params.cpuparams.priority = GGML_SCHED_PRIO_NORMAL; break;
        }
    }

// threading prompt processing
    if (checkJNum(config, "n_threads_batch")) params.cpuparams_batch.n_threads = config["n_threads_batch"];
    if (checkJNum(config, "n_threads_batch_poll")) params.cpuparams_batch.poll = config["n_threads_batch_poll"];
    if (checkJNum(config, "n_threads_batch_sched_priority")) {
        int sched_priority = config["n_threads_batch_sched_priority"];
        switch (sched_priority){
            case 1: params.cpuparams_batch.priority = GGML_SCHED_PRIO_MEDIUM; break;
            case 2: params.cpuparams_batch.priority = GGML_SCHED_PRIO_HIGH; break;
            case 3: params.cpuparams_batch.priority = GGML_SCHED_PRIO_REALTIME; break;
            default: params.cpuparams_batch.priority = GGML_SCHED_PRIO_NORMAL; break;
        }
    }

//gpu offload
    if (checkJNum(config, "n_gpu_layers")) params.n_gpu_layers = config["n_gpu_layers"];
    if (config["main_gpu"].is_boolean()) params.main_gpu = config["main_gpu"];

// backend-specific
#ifdef GGML_USE_VULKAN
    if (checkJNum(config, "n_gpu_layers_vk")) params.n_gpu_layers = config["n_gpu_layers_vk"];
    if (checkJNum(config, "n_threads_vk")) params.cpuparams.n_threads = config["n_threads_vk"];
    if (checkJNum(config, "n_threads_batch_vk")) params.cpuparams_batch.n_threads = config["n_threads_batch_vk"];
    if (config["use_mmap_vk"].is_boolean()) params.use_mmap = config["use_mmap_vk"];
    if (config["flash_attn_vk"].is_boolean()) params.flash_attn = config["flash_attn_vk"];
    if (config["no_kv_offload_vk"].is_boolean()) params.no_kv_offload = config["no_kv_offload_vk"];
#elif GGML_USE_CLBLAST
    if (checkJNum(config, "n_gpu_layers_clblast")) params.n_gpu_layers = config["n_gpu_layers_clblast"];
    if (checkJNum(config, "n_threads_clblast")) params.cpuparams.n_threads = config["n_threads_clblast"];
    if (checkJNum(config, "n_threads_batch_clblast")) params.cpuparams_batch.n_threads = config["n_threads_batch_clblast"];

    if (checkJNum(config, "clblast_platform_id")) params.clblast_platform_id = config["clblast_platform_id"];
#endif

// context-related
    if (checkJNum(config, "ctx-size")) params.n_ctx = config["ctx-size"];
    if (checkJNum(config, "grp_attn_n")) params.grp_attn_n = config["grp_attn_n"];
    if (checkJNum(config, "grp_attn_w")) params.grp_attn_w = config["grp_attn_w"];
    if (checkJNum(config, "n_keep")) params.n_keep = config["n_keep"];
    if (checkJNum(config, "min_keep")) params.sparams.min_keep = config["min_keep"];
    if (checkJNum(config, "n_batch")) params.n_batch = config["n_batch"];
    if (checkJNum(config, "n_ubatch")) params.n_ubatch = config["n_ubatch"];

// misc
    if (config["penalize_nl"].is_boolean()) params.sparams.penalize_nl = config["penalize_nl"];
    if (config["use_mmap"].is_boolean()) params.use_mmap = config["use_mmap"];
    if (config["flash_attn"].is_boolean()) params.flash_attn = config["flash_attn"];
    if (config["no_kv_offload"].is_boolean()) params.no_kv_offload = config["no_kv_offload"];
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
        if (config["rope_scaling_type"] == "none")   { params.rope_scaling_type = LLAMA_ROPE_SCALING_TYPE_NONE; }
        else if (config["rope_scaling_type"] == "linear") { params.rope_scaling_type = LLAMA_ROPE_SCALING_TYPE_LINEAR; }
        else if (config["rope_scaling_type"] == "yarn")   { params.rope_scaling_type = LLAMA_ROPE_SCALING_TYPE_YARN; }
    }

    if (checkJNum(config, "cache_type_k")) params.cache_type_k = config["cache_type_k"];
    if (checkJNum(config, "cache_type_v")) params.cache_type_v = config["cache_type_v"];

    if (checkJObj(config, "control_vectors")) {
        for (auto& el : config["control_vectors"].items()) {
            params.control_vectors.push_back({ el.value(), el.key(), });
        }
    }
}

static void getParamsFromJson(nlohmann::json& config, common_params& params, bool hasFile = false, bool headless = false){
    // the main parameter
    if (checkJString(config, "model")) params.model = config["model"];
    // prompt-related
    getPromptingParamsFromJson(config, params, hasFile, headless);
    // performance and misc
    getPerformanceParamsFromJson(config, params);
    //sampling
    getSamplingParamsFromJson(config, params);
}

static void getParamsFromPreset(nlohmann::json& config, common_params& params){
    
    if (checkJString(config, "card")) {
        std::string cardPath = config["card"];
        nlohmann::json card = getJson(cardPath);
        getSamplingParamsFromJson(card, params);
    } else if (checkJObj(config, "card")) {
        nlohmann::json card = config["card"];
        getSamplingParamsFromJson(card, params);
    }

    if (checkJString(config, "preset")) {
        std::string presetPath = config["preset"];
        nlohmann::json preset = getJson(presetPath);
        getSamplingParamsFromJson(preset, params);
    } else if (checkJObj(config, "preset")) {
        nlohmann::json preset = config["preset"];
        getSamplingParamsFromJson(preset, params);
    }
}

static void getParamsFromObject(std::string object_name, nlohmann::json& config, common_params& params, bool hasFile = false, bool headless = false){

    if(checkJObj(config, object_name)){
        std::cout << "Found object settings in " << params.model << std::endl;
        nlohmann::json modelConfig = config[object_name];
        bool hasFiles = modelConfig["file"].is_string() || config["file"].is_string();
        getParamsFromJson(modelConfig, params, hasFiles, headless);
        // sampling params only
        getParamsFromPreset(modelConfig, params);
        getParamsFromObject("group",modelConfig, params, false, headless);
    }
}

static void readParamsFromJson(nlohmann::json& config, common_params& params, bool headless = false){

    getParamsFromJson(config, params, false, headless);
    // sampling params only
    getParamsFromPreset(config, params);
    getParamsFromObject("group",config, params, false, headless);
    
    // if(config.contains(params.model) && config[params.model].is_object()){
    if(checkJObj(config, params.model)){
        std::cout << "Found settings for model " << params.model << std::endl;
        nlohmann::json modelConfig = config[params.model];
        bool hasFiles = modelConfig["file"].is_string() || config["file"].is_string();
        getParamsFromJson(modelConfig, params, hasFiles, headless);
        // sampling params only
        getParamsFromPreset(modelConfig, params);
        getParamsFromObject("group",modelConfig, params, false, headless);
    }

}

static void readParamsFromJson(nlohmann::json& config, std::string modelName, common_params& params, bool headless = false){
    
    getParamsFromJson(config, params, false, headless);
    getParamsFromPreset(config, params);
    getParamsFromObject("group",config, params, false, headless);
    params.model = modelName;
    
    if(config.contains("model")){
        if(config["model"].is_string() && config.contains(params.model) && config[params.model].is_object()){
            std::cout << "Found settings for model " << params.model << std::endl;
            nlohmann::json modelConfig = config[params.model];
            bool hasFiles = modelConfig["file"].is_string() || config["file"].is_string();
            getParamsFromJson(modelConfig, params, hasFiles, headless);
            getParamsFromPreset(modelConfig, params);
            getParamsFromObject("group",modelConfig, params, false, headless);
        }
    }

}

static void readParamsFromFile(std::string fimeName, common_params& params, bool headless = false){
    std::fstream o1(fimeName);
    if (o1.is_open()) {
        nlohmann::json config;
        o1 >> std::setw(4) >> config;
        
        getParamsFromJson(config, params, false, headless);
        getParamsFromPreset(config, params);
        getParamsFromObject("group",config, params, false, headless);
        
        if(config.contains("model")){
            if(config["model"].is_string() && config.contains(params.model) && config[params.model].is_object()){
                std::cout << "Found settings for model " << params.model << std::endl;
                nlohmann::json modelConfig = config[params.model];
                bool hasFiles = modelConfig["file"].is_string() || config["file"].is_string();
                getParamsFromJson(modelConfig, params, hasFiles, headless);
                getParamsFromPreset(modelConfig, params);
                getParamsFromObject("group",config, params, false, headless);
            }
        }
        
        
        o1.close();
    }
}

static void readParamsFromFile(std::string fimeName, std::string modelName, common_params& params, bool headless = false ){
    std::fstream o1(fimeName);
    if (o1.is_open()) {
        nlohmann::json config;
        o1 >> std::setw(4) >> config;
        
        getParamsFromJson(config, params, false, headless);
        getParamsFromPreset(config, params);
        getParamsFromObject("group",config, params, false, headless);
        params.model = modelName;
        
        if(config.contains(modelName)){
            if(config[modelName].is_object()){
                std::cout << "Found settings for model " << params.model << std::endl;
                nlohmann::json modelConfig = config[params.model];
                bool hasFiles = modelConfig["file"].is_string() || config["file"].is_string();
                getParamsFromJson(modelConfig, params, hasFiles, headless);
                getParamsFromPreset(modelConfig, params);
                getParamsFromObject("group",config, params, false, headless);
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
    
    uint32_t seed = static_cast<uint32_t>(std::chrono::system_clock::now().time_since_epoch().count());
    std::random_device rd;
    std::seed_seq sseq ({rd()%1000,rd()%10000,rd()%seed});
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