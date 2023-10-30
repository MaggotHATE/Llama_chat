#pragma once

#include <future>
#include <chrono>
#include <thread>
#include <locale>
#include <codecvt>
#include <format>


#ifdef GGML_OLD_FORMAT
#   include "GGML/chat_plain.h"
#elif defined(GGML_OLD_FORMAT)
#   include "GGML/chat_plain.h"
#elif defined(GGML_EXPERIMENTAL1)
#   include "experimental1/chat_plain.h"
#elif defined(GGML_EXPERIMENTAL)
#   include "experimental/chat_plain.h"
//#elif defined(GGML_USE_VULKAN)
//#   include "VULKAN/chat_plain.h"
#else
#   include "chat_plain.h"  
#endif

using namespace std;

template <typename T>
string toUTF8(const basic_string<T, char_traits<T>, allocator<T>>& source)
{
    string result;

    wstring_convert<codecvt_utf8_utf16<T>, T> convertor;
    result = convertor.to_bytes(source);

    return result;
}

template <typename T>
void fromUTF8(const string& source, basic_string<T, char_traits<T>, allocator<T>>& result)
{
    wstring_convert<codecvt_utf8_utf16<T>, T> convertor;
    result = convertor.from_bytes(source);
}

void Clear()
{
#if defined _WIN32
    system("cls");
    //clrscr(); // including header file : conio.h
#elif defined (__LINUX__) || defined(__gnu_linux__) || defined(__linux__)
    system("clear");
    //std::cout<< u8"\033[2J\033[1;1H"; //Using ANSI Escape Sequences 
#elif defined (__APPLE__)
    system("clear");
#endif
}

struct modelThread{
    //model class
    chat newChat;
    //vector for storing results
    std::vector<char*> results;
    std::vector<std::string> resultsString;
    std::vector<std::pair<std::string, std::string>> resultsStringPairs;
    nlohmann::json jsonConfig;
    
    // futures for loading, preloading and generating
    std::future<int> loader;
    std::future<int> textPreGen;
    std::future<char*> futureText;
    std::future<std::string> futureTextString;
    std::future<std::string> futureTextSubString;
    std::future<int> totalResult;
    // switches for loading, preloading and generating
    volatile char isLoading = '_';
    volatile char isPregen = '_';
    volatile char isContinue = '_';
    int loaded = 0;
    int last_tokens = 0;
    //int remain_tokens = 0;
    int consumed_tokens = 0;
    int past_tokens = 0;
    
    std::string lastTimings = "Not yet calculated...";
    float lastSpeed = 0.0f;
    float lastSpeedPrompt = 0.0f;
    std::string lastResult = "";
    std::string shortModelName = "";

    // ~modelThread(){
        // ~newChat;
    // }
    
    void getShortName(){
        shortModelName = newChat.params.model;
        int slash = shortModelName.rfind('/');
        if (slash == shortModelName.npos) slash = 0;
        int rslash = shortModelName.rfind('\\');
        if (rslash == shortModelName.npos) rslash = 0;
        
        if (slash > rslash) shortModelName = shortModelName.substr(slash+1);
        else shortModelName = shortModelName.substr(rslash+1);
    }
    
    void appendFirstPrompt(){
        std::string context = newChat.params.prompt;
        
        if (newChat.params.antiprompt.size()) {
            if (newChat.params.prompt != newChat.params.antiprompt[0]){
                int cutAntiPos = context.rfind(newChat.params.antiprompt[0]);
                if (cutAntiPos != std::string::npos){
                    context.erase(cutAntiPos);
                }
                
            }
        }
        
        resultsStringPairs.push_back(std::pair("AI",context));
    }
    
    void appendAnswer(std::string input){
        //resultsString.push_back(input);
        //resultsString.push_back(input);
        if (newChat.params.antiprompt.size()) {
            int cutAntiPos = input.rfind(newChat.params.antiprompt[0]);
            if (cutAntiPos != std::string::npos){
                input.erase(cutAntiPos);
            }
        }
        
        resultsStringPairs.push_back(std::pair("AI",input));
    }
    
    void applySuffix(std::string& suffix){
        newChat.params.input_suffix = suffix;
    }
    
    bool isNewSuffix(std::string& suffix){
        return newChat.params.input_suffix != suffix;
    }
    
    void appendQuestion(std::string input){
        //resultsString.push_back(input);
        //if(newChat.params.input_prefix.empty()) 
            resultsStringPairs.push_back(std::pair(newChat.params.antiprompt[0],input));
        //else resultsStringPairs.push_back(std::pair(newChat.params.input_prefix,input));
    }
    
    void removeLastAnswer(){
        resultsStringPairs.pop_back();
    }
    
    void displayResults(){
        Clear();
        
        for (auto r : resultsString){
            std::cout << r;
        }
        
        std::cout << "Generated: " << last_tokens << std::endl;
        std::cout << lastTimings << std::endl;
        
        std::cout<< DELIMINER;
    }
    
    void display(){
        Clear();
        
        std::cout << "Model: " << shortModelName << std::endl;
        std::cout << "Generated: " << past_tokens << '\n' << std::endl;
        std::cout << "input_prefix: " << newChat.params.input_prefix << '\n' << std::endl;
        std::cout << "input_suffix: " << newChat.params.input_suffix << '\n' << std::endl;
        
        //#ifdef GGML_EXPERIMENTAL1
        std::cout << "Threads: " << newChat.params.n_threads << "/" << newChat.params.n_threads_batch << '\n' << std::endl;
        //#endif
        //std::cout << lastTimings << std::endl;
        std::cout << "Eval speed: " << std::to_string(lastSpeedPrompt) << "\n Gen speed: " << std::to_string(lastSpeed) << std::endl;
        
        for (auto r : resultsStringPairs){
            if (r.first == "AI"){
                std::cout << r.second;
                //wstring converted;
                //fromUTF8(r.second, converted);
                //std::wcout << converted;
            } else {
                if (newChat.params.input_prefix.empty()) std::cout << r.first << r.second;
                else std::cout << newChat.params.input_prefix << r.second;
            }
            
            if (r.second.back() != '\n') std::cout<< DELIMINER;
        }
        
        //if (last_tokens > 0) std::cout << "Generated: " << last_tokens << std::endl;
        //std::cout<< DELIMINER;
    }
    
    bool writeTextFile(std::string path){
        std::string path1 = path + std::to_string(newChat.params.seed) + ".txt";
        std::ofstream file(path1, std::ios::app);
        if (file.is_open()) {
            file << shortModelName << DELIMINER;
            file << lastTimings << DELIMINER;
            for (auto r : resultsStringPairs){
                file << r.first << DELIMINER;
                file << ' ' << r.second << DELIMINER;
            }
            
            file.close();
            return true;
        } else {
            return false;
        }
    }
    
    bool writeTextFile(){
        std::string path = std::to_string(newChat.params.seed) + ".txt";
        std::ofstream file(path, std::ios::app);
        if (file.is_open()) {
            file << shortModelName << DELIMINER;
            file << lastTimings << DELIMINER;
            for (auto r : resultsStringPairs){
                file << r.first << DELIMINER;
                file << ' ' << r.second << DELIMINER;
            }
            
            file.close();
            return true;
        } else {
            return false;
        }
    }
    
    // void clearModel(){
        // ~newChat;
    // }
    
    void unload(){
        newChat.clear();
        resultsStringPairs.clear();
        isContinue = '_';
        isPregen = '_';
        isLoading = '_';
    }
    
    void unloadSoft(){
        newChat.clearSoft();
        resultsStringPairs.clear();
        isContinue = '_';
        isPregen = '_';
        isLoading = '_';
    }
    
    void reset(){
        newChat.resetCTX();
        newChat.clearLastEmbd();
        resultsStringPairs.clear();
        isContinue = '_';
        isPregen = '_';
        isLoading = '_';
    }
    
    void getTimigs(){
        lastTimings = newChat.get_timings();
    }
    
    void getTimigsSimple(){
        //lastTimings = newChat.get_timings_simple();
        lastTimings = newChat.get_ts();
    }
    
    
    
    void getTimigsPre(){
        //lastTimings = newChat.get_timings_simple();
        float tmp = newChat.get_speed_p();
        if (tmp != 0) lastSpeedPrompt = tmp;
    }
    
    void getTimigsGen(){
        //lastTimings = newChat.get_timings_simple();
        float tmp = newChat.get_speed();
        if (tmp != 0) lastSpeed = tmp;
    } 

    void getTimigsBoth(){
        getTimigsPre();
        getTimigsGen();
        
        lastTimings = "Eval speed: " + std::to_string(lastSpeedPrompt) + "\n Gen speed: " + std::to_string(lastSpeed) + '\n';
    }
    
    void startGen(){
        newChat.finished = false;
        isContinue = 'w';
        isPregen = 'w';
        lastResult = newChat.params.input_suffix;
    }
    
    void stop(){
        //newChat.finished = true;
        isContinue = 'i';
        resultsStringPairs.push_back(std::pair("AI",lastResult));
        //getTimigsSimple();
        getTimigsBoth();
    }
    
    // loading, preloading and generating threads
    
/// CURRENT
void getResultAsyncStringFull2(bool streaming = false, bool full = false) {
        //isContinue = 'w';
        //std::cout << " ** " << input << std::endl;
        newChat.clearLastTokens();
        
        
        futureTextString = std::async(std::launch::async, [this, &streaming, &full] mutable {
            
            std::string input = resultsStringPairs.back().second;
                
            getTimigsPre();
            //newChat.inputOnly(input);
            newChat.inputOnlyNew(input);
            isPregen = 'i';
            consumed_tokens = newChat.getConsumedTokens();
            past_tokens = newChat.getPastTokens();
            
            getTimigsPre();
            
            
            while (isContinue != 'i'){
        

                std::string output = newChat.cycleStringsOnly(false);
                lastResult += output;
                
                if (streaming) {
                    if (full) {
                        //getTimigsBoth();
                        //getTimigsPre();
                        getTimigsGen();
                        display();
                        std::cout << lastResult;
                    } else std::cout << output;
                } else {
                    if (full) {
                        getTimigsGen();
                        //getTimigsPre();
                    }
                }
                //getTimigsSimple();
                
                if (newChat.finished){
                    //newChat.eraseAntiprompt(lastResult);
                    newChat.eraseAntiprompt(lastResult);
                    newChat.eraseLast(lastResult, newChat.params.input_prefix);
                    
                    resultsStringPairs.push_back(std::pair("AI",lastResult));
                    isContinue = 'i';
                    
                    getTimigsBoth();
                    newChat.clear_speed();
                }
                    
                
                //futureTextString.get();

                last_tokens = newChat.getLastTokens();
                //remain_tokens = newChat.getRemainTokens();
                consumed_tokens = newChat.getConsumedTokens();
                past_tokens = newChat.getPastTokens();
            
            }
            
            std::string result = "ready";
                
            return result;
        });
        
    }
    
    void getResultAsyncStringRepeat(bool streaming = false, bool full = false) {
        //isContinue = 'w';
        //std::cout << " ** " << input << std::endl;
        removeLastAnswer();
        
        newChat.clearLastEmbd();
        
        futureTextString = std::async(std::launch::async, [this, &streaming, &full] mutable {
            
            //std::string input = resultsStringPairs.back().second;
                
            getTimigsPre();
            //newChat.inputOnly(input);
            //newChat.inputOnlyNew(input);
            isPregen = 'i';
            //consumed_tokens = newChat.getConsumedTokens();
            //past_tokens = newChat.getPastTokens();
            
            //getTimigsPre();
            
            
            while (isContinue != 'i'){
        

                std::string output = newChat.cycleStringsOnly(false);
                lastResult += output;
                
                if (streaming) {
                    if (full) {
                        //getTimigsBoth();
                        //getTimigsPre();
                        getTimigsGen();
                        display();
                        std::cout << lastResult;
                    } else std::cout << output;
                } else {
                    if (full) {
                        getTimigsGen();
                        //getTimigsPre();
                    }
                }
                //getTimigsSimple();
                
                if (newChat.finished){
                    //newChat.eraseAntiprompt(lastResult);
                    newChat.eraseAntiprompt(lastResult);
                    newChat.eraseLast(lastResult, newChat.params.input_prefix);
                    
                    resultsStringPairs.push_back(std::pair("AI",lastResult));
                    isContinue = 'i';
                    
                    getTimigsBoth();
                    newChat.clear_speed();
                }
                    
                
                //futureTextString.get();

                last_tokens = newChat.getLastTokens();
                //remain_tokens = newChat.getRemainTokens();
                consumed_tokens = newChat.getConsumedTokens();
                past_tokens = newChat.getPastTokens();
            
            }
            
            std::string result = "ready";
                
            return result;
        });
        
    }
    
    void load(nlohmann::json localConfig, bool soft = false) {
    
        totalResult = std::async(std::launch::async, [this, localConfig, soft] mutable { 

            loaded = newChat.initGenerate(localConfig, false, soft);

            appendFirstPrompt();
            getTimigsBoth();
            //remain_tokens = newChat.getRemainTokens();
            consumed_tokens = newChat.getConsumedTokens();
            past_tokens = newChat.getPastTokens();

            std::cout << "Loaded, you can type now! " << std::endl;
            getShortName();
            isContinue = 'i';
            
            return loaded;
        });
    }
    
    void load() {
    
        totalResult = std::async(std::launch::async, [this] mutable { 

            loaded = newChat.initGenerate(jsonConfig, false);

            appendFirstPrompt();
            //getTimigsSimple();
            getTimigsBoth();
            //remain_tokens = newChat.getRemainTokens();
            consumed_tokens = newChat.getConsumedTokens();
            past_tokens = newChat.getPastTokens();

            std::cout << "Loaded, you can type now! " << std::endl;
            getShortName();
            isContinue = 'i';
            
            return loaded;
        });
    }
    
    void microload() {
    
        loaded = 10;
        totalResult = std::async(std::launch::async, [this] mutable { 

            newChat.generate();

            appendFirstPrompt();
            //getTimigsSimple();
            getTimigsBoth();
            //remain_tokens = newChat.getRemainTokens();
            consumed_tokens = newChat.getConsumedTokens();
            past_tokens = newChat.getPastTokens();

            std::cout << "Reset complete!" << std::endl;
            getShortName();
            isContinue = 'i';
            
            loaded = 9;
            return loaded;
        });
    }
    
    
};

struct configurableChat{
    gpt_params params;
    //gpt_params paramsDefault;
    
    nlohmann::json localConfig;
    nlohmann::json modelConfig;
    std::vector<std::pair<std::string, std::string>> modelsFromConfig;
    
    std::string promptFilesFolder = "/prompts/";
    std::vector<std::filesystem::path> promptFiles;
    bool noConfig = false;
    //bool hasInstructFile = false;
    
    
    //float cfg_smooth_factor;
    std::string basePrompt;
    std::string modelFromJson = "NULL";
    std::string instructFileFromJson = "NULL";
    std::string cfgNegativePrompt;
    std::string inputPrompt = "NULL";
    std::string inputAntiprompt = "NULL";
    std::string inputAntiCFG = "NULL";
    std::string inputInstructFile = "NULL";
    std::string modelName = "NULL";
    std::string modelShortName = "NULL";
    std::string grammarFile = "";
    std::string localJsonDump;
    
    void getModelsList(){
        modelsFromConfig.clear();
        for (auto& [key, value] : localConfig.items() ){
            if (value.is_object()) {
                std::string fullName = key;
                auto lastSlash = fullName.rfind('/');
                auto lastRslash = fullName.rfind('\\');
                if (lastSlash == std::string::npos) lastSlash = -1;
                if (lastRslash == std::string::npos) lastRslash = -1;
                
                std::string onlyName = fullName.substr(
                    (lastRslash > lastSlash ? lastRslash : lastSlash) + 1  
                );
                modelsFromConfig.push_back( std::pair(fullName,onlyName) );
            }
        }
    }
    
    void getFilesList(){
        promptFiles.clear();
        
        if ( std::filesystem::exists(promptFilesFolder) ){
            for (auto const& file : std::filesystem::directory_iterator{promptFilesFolder}) 
            {
                if (file.path().extension() == ".txt") promptFiles.push_back( file.path());
            }
        }
    }
    
    void readInstructFile(std::string filename, bool headless = false){
        std::ifstream file(filename);
        if (!file) {
            fprintf(stderr, "error: failed to open file '%s'\n", filename);
        } else {
            inputPrompt.clear();
            std::copy(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>(), back_inserter(inputPrompt));
            if (inputPrompt.back() == '\n') {
                inputPrompt.pop_back();
            }
            if (!headless){
                int antiPos = inputPrompt.rfind('\n');
                if (antiPos != inputPrompt.npos) {
                    inputAntiprompt = inputPrompt.substr(antiPos);
                }
            }
        }
    }
    
    void readInstructFile(std::string filename, std::string& inputPromptExt, std::string& inputAntipromptExt, bool headless = false){
        std::ifstream file(filename);
        if (!file) {
            fprintf(stderr, "error: failed to open file '%s'\n", filename);
        } else {
            inputPromptExt.clear();
            std::copy(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>(), back_inserter(inputPromptExt));
            if (inputPromptExt.back() == '\n') {
                inputPromptExt.pop_back();
            }
            if (!headless){
                int antiPos = inputPromptExt.rfind('\n');
                if (antiPos != inputPromptExt.npos) {
                    inputAntipromptExt = inputPromptExt.substr(antiPos + 1);
                }
            }
        }
    }
    
    void updateDump(){
        localJsonDump = modelConfig.dump(3);
    }
    
    std::string getJStr(std::string param){
        if (modelFromJson != "NULL" && localConfig.contains(modelFromJson)) {
            if (localConfig[modelFromJson].contains(param)) {
                if (localConfig[modelFromJson][param].is_string()) return localConfig[modelFromJson][param].get<std::string>();
            } else if (localConfig.contains(param)) {
                if (localConfig[param].is_string()) return localConfig[param].get<std::string>();
            }
        } else if (localConfig.contains(param)) {
            if (localConfig[param].is_string()) return localConfig[param].get<std::string>();
        }
        
        return "NULL";
    }
    
    float getJFloat(std::string param){
        if (modelFromJson != "NULL" && localConfig.contains(modelFromJson)) {
            if (localConfig[modelFromJson].contains(param)) {
                if (localConfig[modelFromJson][param].is_number()) return localConfig[modelFromJson][param].get<float>();
            } else if (localConfig.contains(param)) {
                if (localConfig[param].is_number()) return localConfig[param].get<float>();
            }
        } else if (localConfig.contains(param)) {
            if (localConfig[param].is_number()) return localConfig[param].get<float>();
        }
        
        return 999.0f;
    }
    
    int getJInt(std::string param){
        if (modelFromJson != "NULL" && localConfig.contains(modelFromJson)) {
            if (localConfig[modelFromJson].contains(param)) {
                if (localConfig[modelFromJson][param].is_number()) return localConfig[modelFromJson][param].get<int>();
            } else if (localConfig.contains(param)) {
                if (localConfig[param].is_number()) return localConfig[param].get<int>();
            }
        } else if (localConfig.contains(param)) {
            if (localConfig[param].is_number()) return localConfig[param].get<int>();
        }
        
        return 999;
    }
    
    void getSettings(chat& aChat){
        int n_keep = params.n_keep;
        params = paramsDefault;
        params = aChat.params;
        params.n_keep = n_keep;
    }
    
    // void getSettings(){
        // params = paramsDefault;
        // params = aChat.params;
    // }
    
    

    void getSettingsFull(){
        params = paramsDefault;
        readParamsFromJson(localConfig, params);
        modelName = params.model;
        syncInputs();
    }
    
    void syncInputs(){
        inputPrompt = params.prompt;
        if(params.antiprompt.size()) inputAntiprompt = params.antiprompt[0];
        if(params.sparams.cfg_negative_prompt.size()) inputAntiCFG = params.sparams.cfg_negative_prompt;
    }
    
    int checkChangedInputs(){
        int result = 0;
        if (inputPrompt != "NULL" && inputPrompt != params.prompt) result += 1;
        if (inputAntiprompt != "NULL" && params.antiprompt.size() && inputAntiprompt != params.antiprompt[0]) result += 2;
        if (inputAntiCFG != "NULL" && inputAntiCFG != params.sparams.cfg_negative_prompt) result += 4;
        
        return result;
    }
    
    void getSettingsFromModelConfig(){
        params = paramsDefault;
        readParamsFromJson(modelConfig, params);
        modelName = params.model;
    }
    
    void getSettings(std::string newModelName){
        params = paramsDefault;
        readParamsFromJson(localConfig, newModelName, params);
        basePrompt = params.prompt;
        modelName = newModelName;
    }
    
    void updateSettings(){
        params = paramsDefault;
        readParamsFromJson(localConfig, modelName, params);
    }
    
    void pushSettings(chat& aChat){
        //aChat.params.n_threads = n_threads;
        aChat.params.sparams.temp = params.sparams.temp;
        //aChat.params.n_keep = params.n_keep;
        aChat.params.sparams.top_k = params.sparams.top_k;
        aChat.params.sparams.top_p = params.sparams.top_p;
        aChat.params.sparams.min_p = params.sparams.min_p;
        aChat.params.sparams.tfs_z = params.sparams.tfs_z;
        aChat.params.sparams.typical_p = params.sparams.typical_p;
        aChat.params.sparams.penalty_repeat = params.sparams.penalty_repeat;
        aChat.params.sparams.penalty_freq = params.sparams.penalty_freq;
        aChat.params.sparams.penalty_present = params.sparams.penalty_present;
        aChat.params.sparams.mirostat = params.sparams.mirostat;
        aChat.params.sparams.mirostat_tau = params.sparams.mirostat_tau;
        aChat.params.sparams.mirostat_eta = params.sparams.mirostat_eta;
        aChat.params.sparams.cfg_scale = params.sparams.cfg_scale;
        //aChat.params.cfg_smooth_factor = cfg_smooth_factor;
    }
    
    // void pushSettings(){
        // //aChat.params.n_threads = n_threads;
        // aChat.params.temp = params.temp;
        // aChat.params.top_k = params.top_k;
        // aChat.params.top_p = params.top_p;
        // aChat.params.tfs_z = params.tfs_z;
        // aChat.params.typical_p = params.typical_p;
        // aChat.params.penalty_repeat = params.penalty_repeat;
        // aChat.params.penalty_freq = params.penalty_freq;
        // aChat.params.penalty_present = params.penalty_present;
        // aChat.params.mirostat = params.mirostat;
        // aChat.params.mirostat_tau = params.mirostat_tau;
        // aChat.params.mirostat_eta = params.mirostat_eta;
        // aChat.params.cfg_scale = params.cfg_scale;
        // //aChat.params.cfg_smooth_factor = cfg_smooth_factor;
    // }
    
    
    void hasFile(){
        if (modelFromJson != "NULL" && localConfig.contains(modelFromJson)) {
            if (localConfig[modelFromJson]["file"].is_string()) {
                instructFileFromJson = localConfig[modelFromJson]["file"].get<std::string>();
            }
            
        } 
        
        if (instructFileFromJson == "NULL" && localConfig.contains("file")) {
            if (localConfig["file"].is_string()) instructFileFromJson = localConfig["file"].get<std::string>();
        }
        
    }
    
    bool checkInputPrompt(){
        if (inputPrompt != "NULL" && inputPrompt != modelConfig[modelName]["prompt"].get<std::string>()) return false;
        
        return true;
    }
    
    bool checkInputAntiprompt(){
        if (inputAntiprompt != "NULL" && inputAntiprompt != modelConfig[modelName]["reverse-prompt"].get<std::string>()) return false;
        
        return true;
    }
    
    bool checkInputAntiCFG(){
        if (inputAntiCFG != "NULL" && inputAntiCFG != modelConfig[modelName]["cfg-negative-prompt"].get<std::string>()) return false;
        
        return true;
    }
    
    void clearFile(){
        
        // if (modelName != "NULL") {
            // if (localConfig.contains(modelName)) localConfig[modelName].erase("file");
        // }
        
        // if (modelFromJson != "NULL") {
            // if (localConfig.contains(modelFromJson)) localConfig[modelFromJson].erase("file");
        // }
            
        // if (localConfig.contains("file")) localConfig.erase("file");
        
        instructFileFromJson = "NULL";
        
        params = paramsDefault;
        readParamsFromJson(localConfig, modelName, params);
    }
    
    void fillLocalJson(std::string& model){
        modelConfig.clear();
        
        modelConfig["model"] = model;
        
        if (inputInstructFile != "NULL") //modelConfig[modelName]["file"] = inputInstructFile;
            processInstructFile(inputInstructFile, params);
        else if (instructFileFromJson != "NULL") //modelConfig[modelName]["file"] = instructFileFromJson;
            processInstructFile(instructFileFromJson, params);
        
        if (!params.input_suffix.empty()) modelConfig[model]["input_suffix"] = params.input_suffix;
        if (!params.input_prefix.empty()) modelConfig[model]["input_prefix"] = params.input_prefix;
        if (params.input_prefix_bos != paramsDefault.input_prefix_bos) modelConfig[model]["input_prefix_bos"] = params.input_prefix_bos;
        
        if (params.sparams.penalize_nl != paramsDefault.sparams.penalize_nl) modelConfig[model]["penalize_nl"] = params.sparams.penalize_nl;
        if (params.use_mmap != paramsDefault.use_mmap) modelConfig[model]["use_mmap"] = params.use_mmap;
        
        
        if (params.sparams.temp != paramsDefault.sparams.temp) modelConfig[model]["temp"] = params.sparams.temp;
        if (params.sparams.top_k != paramsDefault.sparams.top_k) modelConfig[model]["top_k"] = params.sparams.top_k;
        if (params.sparams.top_p != paramsDefault.sparams.top_p) modelConfig[model]["top_p"] = params.sparams.top_p;
        if (params.sparams.min_p != paramsDefault.sparams.min_p) modelConfig[model]["min_p"] = params.sparams.min_p;
        if (params.sparams.tfs_z != paramsDefault.sparams.tfs_z) modelConfig[model]["tfs_z"] = params.sparams.tfs_z;
        if (params.sparams.typical_p != paramsDefault.sparams.typical_p) modelConfig[model]["typical_p"] = params.sparams.typical_p;
        if (params.sparams.penalty_repeat != paramsDefault.sparams.penalty_repeat) modelConfig[model]["repeat_penalty"] = params.sparams.penalty_repeat;
        if (params.sparams.penalty_freq != paramsDefault.sparams.penalty_freq) modelConfig[model]["frequency_penalty"] = params.sparams.penalty_freq;
        if (params.sparams.penalty_present != paramsDefault.sparams.penalty_present) modelConfig[model]["present_penalty"] = params.sparams.penalty_present;
        if (params.sparams.mirostat != paramsDefault.sparams.mirostat) modelConfig[model]["mirostat"] = params.sparams.mirostat;
        if (params.sparams.mirostat_tau != paramsDefault.sparams.mirostat_tau) modelConfig[model]["mirostat_tau"] = params.sparams.mirostat_tau;
        if (params.sparams.mirostat_eta != paramsDefault.sparams.mirostat_eta) modelConfig[model]["mirostat_eta"] = params.sparams.mirostat_eta;
        if (params.sparams.cfg_scale != paramsDefault.sparams.cfg_scale) modelConfig[model]["cfg-scale"] = params.sparams.cfg_scale;
        if (params.n_ctx != paramsDefault.n_ctx) modelConfig[model]["ctx-size"] = params.n_ctx;
        if (params.n_keep != paramsDefault.n_keep) modelConfig[model]["n_keep"] = params.n_keep;
        if (params.n_batch != paramsDefault.n_batch) modelConfig[model]["n_batch"] = params.n_batch;
        if (params.n_threads != paramsDefault.n_threads) modelConfig[model]["n_threads"] = params.n_threads;
        
        //#if GGML_EXPERIMENTAL1
        if (params.n_threads_batch != paramsDefault.n_threads_batch) modelConfig[model]["n_threads_batch"] = params.n_threads_batch;
        //#endif
        if (params.n_gpu_layers != paramsDefault.n_gpu_layers) modelConfig[model]["n_gpu_layers"] = params.n_gpu_layers;
        
        #if GGML_OLD_FORMAT
        if (params.rms_norm_eps != paramsDefault.rms_norm_eps) modelConfig[model]["rms-norm-eps"] = params.rms_norm_eps;
        #else 
        if (params.n_threads_batch != paramsDefault.n_threads_batch) modelConfig[model]["n_threads_batch"] = params.n_threads_batch;
        #endif
        
        if (params.rope_freq_base != paramsDefault.rope_freq_base) modelConfig[model]["rope_freq_base"] = params.rope_freq_base;
        if (params.rope_freq_scale != paramsDefault.rope_freq_scale) modelConfig[model]["rope_freq_scale"] = params.rope_freq_scale;
        
        modelConfig[model]["cfg-negative-prompt"] = params.sparams.cfg_negative_prompt;
        if (params.prompt.size()) modelConfig[model]["prompt"] = params.prompt;
        else {
            modelConfig[model]["prompt"] = "### Instruction:";
            params.prompt = "### Instruction:";
        }
        if(params.antiprompt.size()) modelConfig[model]["reverse-prompt"] = params.antiprompt[0];
        else {
            modelConfig[model]["reverse-prompt"] = "### Instruction:";
            params.antiprompt.push_back("### Instruction:");
        }
        
        if(grammarFile != "") modelConfig[model]["grammar-file"] = grammarFile;
    }
    
    void fillLocalJson(){
        fillLocalJson(modelName);
    }
    
    void applySuffix(chat& aChat){
        aChat.params.input_suffix = params.input_suffix;
    }
    
    bool isNewSuffix(chat& aChat){
        return aChat.params.input_suffix != params.input_suffix;
    }
    
    void updateInput(){
        if (inputPrompt != "NULL") params.prompt = inputPrompt;
        if (inputAntiprompt != "NULL") {
            if(!params.antiprompt.size()) 
                params.antiprompt.push_back(inputAntiprompt);
            else
                params.antiprompt[0] = inputAntiprompt;
        }
        if (inputAntiCFG != "NULL") params.sparams.cfg_negative_prompt = inputAntiCFG;
    }
    
    void pushToMainConfig(){
        for (auto& [key, value] : modelConfig.items() ){
            if (value != NULL) localConfig[key] = value;
        }
        
        if (localConfig.contains("error")) localConfig.erase("error");
    }
    
    void getFromJson(std::string fileName){
        localConfig = getJson(fileName);
        // we can use this for Welcome messages and initial configuration later 
        //if (localConfig["error"].is_string()) noConfig = true;
        if (localConfig.contains("error")) noConfig = true;
        else {
            noConfig = false;
            getModelsList();
            hasFile();
        }
    }
    
    void checkLocalConfig(){
        if (noConfig == true && modelName != "NULL" && inputPrompt != "NULL"){
            noConfig = false;
        }
    }
};