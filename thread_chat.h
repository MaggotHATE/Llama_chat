#pragma once

#include <future>
#include <chrono>
#include <thread>
#include <locale>
#include <codecvt>


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
    volatile char isUnload = '_';
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
    std::string sparamsList = "";
    std::string externalData = "";

    // ~modelThread(){
        // ~newChat;
    // }
    
    void getShortName(){
        //from slashes till dot of extension
        shortModelName = newChat.params.model;
        int slash = shortModelName.rfind('/');
        if (slash == shortModelName.npos) slash = 0;
        int rslash = shortModelName.rfind('\\');
        if (rslash == shortModelName.npos) rslash = 0;
        
        if (slash > rslash) shortModelName = shortModelName.substr(slash+1);
        else shortModelName = shortModelName.substr(rslash+1, shortModelName.rfind('.'));
        
        shortModelName = shortModelName.substr(0, shortModelName.rfind('.'));
    }
    
    void getSparamsList(){
        //sparamsList = newChat.getSparams();
        sparamsList = newChat.getSparamsChanged();
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
        
        resultsStringPairs.emplace_back(std::pair("AI",context));
    }
    
    void appendAnswer(std::string input){
        //resultsString.emplace_back(input);
        //resultsString.emplace_back(input);
        if (newChat.params.antiprompt.size()) {
            int cutAntiPos = input.rfind(newChat.params.antiprompt[0]);
            if (cutAntiPos != std::string::npos){
                input.erase(cutAntiPos);
            }
        }
        
        resultsStringPairs.emplace_back(std::pair("AI",input));
    }
    
    void applySuffix(std::string& suffix){
        newChat.params.input_suffix = suffix;
    }
    
    bool isNewSuffix(std::string& suffix){
        return newChat.params.input_suffix != suffix;
    }
    
    void appendQuestion(std::string& input){
        //if (input.back() == DELIMINER ) input.pop_back();
        //resultsString.emplace_back(input);
        //if(newChat.params.input_prefix.empty()) 
            resultsStringPairs.emplace_back(std::pair(newChat.params.antiprompt[0],input));
        //else resultsStringPairs.emplace_back(std::pair(newChat.params.input_prefix,input));
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
        
        std::cout << "----------------------------------------"<< std::endl;
        std::cout << "Model: " << shortModelName << std::endl;
        std::cout << "Seed: " << std::to_string(newChat.params.seed) << std::endl;
        std::cout << "ctx size: " << std::to_string(newChat.params.n_ctx) << "\n----------------------------------------" << std::endl;
        if (newChat.params.antiprompt.size()) std::cout << "Antiprompt: " << newChat.params.antiprompt[0] << std::endl;
        std::cout << "input_prefix: " << newChat.params.input_prefix << std::endl;
        std::cout << "input_suffix: " << newChat.params.input_suffix << "\n----------------------------------------" << std::endl;
        //std::cout << newChat.formatRepresentation << std::endl;
        std::cout << externalData << std::endl;
        std::cout << sparamsList << std::endl;
        std::cout << '\n' << "STATUS     : " << (newChat.finished ? "READY" : "BUSY") << std::endl;
        std::cout << "WAITING    : " << (is_interacting ? "YES" : "NO") << std::endl;
        std::cout << "isContinue : " << isContinue << std::endl;
        std::cout << "Generated  : " << past_tokens << std::endl;
        std::cout << "Consumed   : " << consumed_tokens << std::endl;
        std::cout << "Last       : " << last_tokens << std::endl;
        std::cout << "Past-Last  : " << past_tokens - last_tokens << '\n' << std::endl;
        std::cout << "embd_inp.size: " << newChat.getEmbInpSize() << '\n' << std::endl;
        
        //#ifdef GGML_EXPERIMENTAL1
        std::cout << "Threads: " << newChat.params.n_threads << "/" << newChat.params.n_threads_batch << '\n' << std::endl;
        //#endif
        //std::cout << lastTimings << std::endl;
        std::cout << "Eval speed: " << std::to_string(lastSpeedPrompt) << "\n Gen speed: " << std::to_string(lastSpeed) << std::endl;
        std::cout << "----------------------------------------\n"<< std::endl;
        
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
        if (isContinue == 'w') std::cout << lastResult;
        //if (last_tokens > 0) std::cout << "Generated: " << last_tokens << std::endl;
        //std::cout<< DELIMINER;
    }
    
    bool writeTextFileSimple(std::string path){
        std::ofstream file(path, std::ios::app);
        if (file.is_open()) {
            file << shortModelName << DELIMINER;
            file << lastTimings << DELIMINER;
            file << '\n' << "Generated: " << past_tokens << '\n' << std::endl;
            file << "----------------------------------------\n"<< std::endl;
            for (auto r : resultsStringPairs){
                //file << r.first << DELIMINER;
                //file << ' ' << r.second << DELIMINER;
                file << '\n' << r.second << DELIMINER;
            }
            
            file.close();
            return true;
        } else {
            return false;
        }
    }
    
    bool writeTextFile(std::string path){
        std::string path1 = path + std::to_string(newChat.params.seed) + ".txt";
        std::ofstream file(path1, std::ios::app);
        if (file.is_open()) {
            file << shortModelName << DELIMINER;
            file << lastTimings << DELIMINER;
            file << '\n' << "Generated: " << past_tokens << '\n' << std::endl;
            file << "----------------------------------------\n"<< std::endl;
            for (auto r : resultsStringPairs){
                // file << r.first << DELIMINER;
                // file << ' ' << r.second << DELIMINER;
                file << '\n' << r.second << DELIMINER;
            }
            
            file.close();
            return true;
        } else {
            return false;
        }
    }
    
    bool writeTextFile(std::string path, std::string name){
        std::string path1 = path + std::to_string(newChat.params.seed) + "-" + name + ".txt";
        std::ofstream file(path1, std::ios::app);
        if (file.is_open()) {
            file << shortModelName << DELIMINER;
            file << lastTimings << DELIMINER;
            file << '\n' << "Generated: " << past_tokens << '\n' << std::endl;
            file << "----------------------------------------\n"<< std::endl;
            for (auto r : resultsStringPairs){
                // file << r.first << DELIMINER;
                // file << ' ' << r.second << DELIMINER;
                file << '\n' << r.second << DELIMINER;
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
            file << std::to_string(newChat.params.seed) << DELIMINER;
            file << lastTimings << DELIMINER;
            file << sparamsList << DELIMINER;
            file << '\n' << "Generated: " << past_tokens << '\n' << std::endl;
            file << "----------------------------------------\n"<< std::endl;
            for (auto r : resultsStringPairs){
                //file << r.first << DELIMINER;
                //file << ' ' << r.second << DELIMINER;
                file << '\n' << r.second << DELIMINER;
            }
            
            file.close();
            return true;
        } else {
            return false;
        }
    }
    
    bool writeTextFileFull(std::string path, std::string name){
        std::string path1 = path + name + "-" + shortModelName + ".txt";
        std::ofstream file(path1, std::ios::app);
        if (file.is_open()) {
            file << shortModelName << DELIMINER;
            file << "Threads: " << newChat.params.n_threads << "/" << newChat.params.n_threads_batch << '\n' << DELIMINER;
            file << lastTimings << DELIMINER;
            file << "Seed = " << std::to_string(newChat.params.seed) << DELIMINER;
            file << sparamsList << DELIMINER;
            file << '\n' << "Generated: " << past_tokens << '\n' << std::endl;
            file << "----------------------------------------\n"<< std::endl;
            for (auto r : resultsStringPairs){
                // file << r.first << DELIMINER;
                // file << ' ' << r.second << DELIMINER;
                file << '\n' << r.second << DELIMINER;
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
    
    void clear_last() {
        resultsStringPairs.pop_back();
        newChat.clear_last();
    }
    
    void unload(){
        newChat.clear();
        resultsStringPairs.clear();
        isContinue = '_';
        isPregen = '_';
        isLoading = '_';
        isUnload = '_';
    }
    
    void unloadSoft(){
        newChat.clearSoft();
        resultsStringPairs.clear();
        isContinue = '_';
        isPregen = '_';
        isLoading = '_';
        isUnload = '_';
    }
    
    void reset(){
        newChat.resetCTX();
        newChat.clearLastEmbd();
        resultsStringPairs.clear();
        isContinue = '_';
        isPregen = '_';
        isLoading = '_';
        isUnload = '_';
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
    
    void continueGen(){
        newChat.finished = false;
        isContinue = 'w';
        isPregen = 'w';
    }
    
    void stop(){
        //newChat.finished = true;
        isContinue = 'i';
        resultsStringPairs.emplace_back(std::pair("AI",lastResult));
        //getTimigsSimple();
        getTimigsBoth();
    }
    
    void pause(){
        //newChat.finished = true;
        isContinue = 'i';
        //getTimigsSimple();
        getTimigsBoth();
    }
    
    // loading, preloading and generating threads
    
/// CURRENT

void checkFinished() {
    if (newChat.finished){
        //newChat.eraseAntiprompt(lastResult);
        newChat.eraseAntiprompt(lastResult);
        newChat.eraseLast(lastResult, newChat.params.input_prefix);
        
        resultsStringPairs.emplace_back(std::pair("AI",lastResult));
        lastResult = "";
        isContinue = 'i';
        isPregen = 'i';
        
        getTimigsBoth();
        newChat.clear_speed();
    }
}

void getStats() {
    last_tokens = newChat.getLastTokens();
    consumed_tokens = newChat.getConsumedTokens();
    past_tokens = newChat.getPastTokens();
}

void getResultAsyncStringFull2(bool streaming = false, bool full = true) {
        //isContinue = 'w';
        //std::cout << " ** " << input << std::endl;
        newChat.clearLastTokens();
        
        
        futureTextString = std::async(std::launch::async, [this, &streaming, &full] mutable {
            
            std::string input = resultsStringPairs.back().second;
                
            getTimigsPre();
            //newChat.inputOnly(input);
            if (full == true) {
                isPregen = 'w';
                newChat.inputOnlyNew(input);
            }
            
            isPregen = 'i';
            consumed_tokens = newChat.getConsumedTokens();
            past_tokens = newChat.getPastTokens();
            last_tokens = newChat.getLastTokens();
            
            getTimigsPre();
            if (streaming == true) display();
            
            while (isContinue != 'i'){
        

                std::string output = newChat.cycleStringsOnly(false);
                lastResult += output;
                //if (streaming == true) display();
                if (streaming == true) std::cout << output;
                getTimigsGen();
                
                // if (full == true) {
                    // if (streaming == true) {
                        // getTimigsGen();
                        // display();
                        // std::cout << lastResult;
                    // } else getTimigsGen();
                // } else if (streaming == true) std::cout << output;
                
                //getTimigsSimple();
                
                checkFinished();
                getStats();
                
            }
            
            if (streaming == true) display();
            
            std::string result = "ready";
                
            return result;
        });
        
    }
    
// for UI
void getResultAsyncStringFull3() {
        //isContinue = 'w';
        //std::cout << " ** " << input << std::endl;
        newChat.clearLastTokens();
        
        
        futureTextString = std::async(std::launch::async, [this] mutable {
            isPregen = 'w';
            getTimigsPre();
            
            std::string input = resultsStringPairs.back().second;
            newChat.inputOnlyNew(input);
            
            isPregen = 'i';
            consumed_tokens = newChat.getConsumedTokens();
            past_tokens = newChat.getPastTokens();
            
            getTimigsPre();
            
            
            while (isContinue != 'i'){
        

                std::string output = newChat.cycleStringsOnly(false);
                if (isContinue == 'i') {
                    if (isUnload == 'y') {
                        unload();
                    }
                    return (std::string) "stopped";
                }
                lastResult += output;
                
                getTimigsGen();
                
                //getTimigsSimple();
                
                checkFinished();
                getStats();
                if (isContinue == 'i') {
                    if (isUnload == 'y') {
                        unload();
                    }
                    return (std::string) "stopped";
                }
            
            }
            
            std::string result = "ready";
                
            return result;
        });
        
    }
    
    void getResultAsyncStringRepeat(bool streaming = false, bool full = false) {
        
        futureTextString = std::async(std::launch::async, [this, &streaming, &full] mutable {
                
            getTimigsPre();
            isPregen = 'i';
            consumed_tokens = newChat.getConsumedTokens();
            past_tokens = newChat.getPastTokens();
            last_tokens = newChat.getLastTokens();
            
            getTimigsPre();
            
            
            while (isContinue != 'i') {
        

                std::string output = newChat.cycleStringsOnly(false);
                if (isContinue == 'i') {
                    if (isUnload == 'y') {
                        unload();
                    }
                    return (std::string) "stopped";
                }
                
                lastResult += output;
                
                // if (streaming) {
                    // if (full) {
                        // getTimigsGen();
                        // display();
                        // std::cout << lastResult;
                    // } else std::cout << output;
                // } else {
                    // if (full) {
                        // getTimigsGen();
                    // }
                // }
                // there's no repeat/continue in chatTest anyway, UI-only
                //getTimigsSimple();
                getTimigsGen();
                
                checkFinished();
                getStats();
                if (isContinue == 'i') {
                    if (isUnload == 'y') {
                        unload();
                    }
                    return (std::string) "stopped";
                }
            }
            
            std::string result = "ready";
                
            return result;
        });
        
    }
    
    void getResultAsyncStringRepeat3() {
        //isContinue = 'w';
        //std::cout << " ** " << input << std::endl;
        newChat.clearLastTokens();
        
        
        futureTextString = std::async(std::launch::async, [this] mutable {
            isPregen = 'i';
            consumed_tokens = newChat.getConsumedTokens();
            past_tokens = newChat.getPastTokens();
            
            getTimigsPre();
            
            
            while (isContinue != 'i'){
        

                std::string output = newChat.cycleStringsOnly(false);
                if (isContinue == 'i') {
                    if (isUnload == 'y') {
                        unload();
                    }
                    return (std::string) "stopped";
                }
                lastResult += output;
                
                getTimigsGen();
                
                //getTimigsSimple();
                
                checkFinished();
                getStats();
                if (isContinue == 'i') {
                    if (isUnload == 'y') {
                        unload();
                    }
                    return (std::string) "stopped";
                }
            
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

            //std::cout << "Loaded, you can type now! " << std::endl;
            getShortName();
            getSparamsList();
            
            if (isUnload == 'y') {
                unload();
            } else isContinue = 'i';
            
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

            //std::cout << "Loaded, you can type now! " << std::endl;
            getShortName();
            getSparamsList();
            //isContinue = 'i';
            
            if (isUnload == 'y') {
                unload();
            } else isContinue = 'i';
            
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
            getSparamsList();
            isContinue = 'i';
            
            loaded = 9;
            return loaded;
        });
    }
    
    
};

static bool exists( const std::filesystem::path& p) noexcept {
    try {
        return std::filesystem::exists(p);
    } catch (std::filesystem::filesystem_error const& ex) {
        std::cout << "what():  " << ex.what() << '\n'
                  << "path1(): " << ex.path1() << '\n'
                  << "path2(): " << ex.path2() << '\n'
                  << "code().value():    " << ex.code().value() << '\n'
                  << "code().message():  " << ex.code().message() << '\n'
                  << "code().category(): " << ex.code().category().name() << '\n';
    }
    
    return false;
}

struct model_card {
    std::string name;
    std::string path;
    std::string format;
};

struct configurableChat{
    gpt_params params;
    //gpt_params paramsDefault;
    
    nlohmann::json localConfig;
    nlohmann::json modelConfig;
    bool tempFirst = true;
    std::string charName = "";
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
    std::string inputSuffix = "";
    std::string inputPrefix = "";
    std::string inputAntiprompt = "NULL";
    std::string inputAntiCFG = "";
    std::string inputInstructFile = "NULL";
    std::string modelName = "NULL";
    std::string modelShortName = "NULL";
    std::string grammarFile = "";
    std::string localJsonDump;
    
    void getModelsList() noexcept{
        modelsFromConfig.clear();
        for (auto& [key, value] : localConfig.items() ){
            if (value.is_object() && exists(key)) {
                std::string fullName = key;
                size_t lastSlash = fullName.rfind('/');
                size_t lastRslash = fullName.rfind('\\');
                if (lastSlash == std::string::npos) lastSlash = -1;
                if (lastRslash == std::string::npos) lastRslash = -1;
                size_t result = (lastSlash > lastRslash ? lastRslash : lastSlash);
                
                std::string onlyName = fullName.substr(result + 1);
                
                // printf("\n%d vs %d = %d: result in %s\n", lastSlash, 
                                                          // lastRslash, 
                                                          // result, 
                                                          // onlyName.c_str()
                // );
                
                modelsFromConfig.emplace_back( std::pair(fullName,onlyName) );
                    //std::cout << "Adding " << fullName << std::endl;
            }
        }
        
        std::cout << "getModelsList finished" << std::endl;
    }
    
    void getModelsList(nlohmann::json& configFile) noexcept{
        modelsFromConfig.clear();
        for (auto& [key, value] : configFile.items() ){
            if (value.is_object() && exists(key)) {
                std::string fullName = key;
                size_t lastSlash = fullName.rfind('/');
                size_t lastRslash = fullName.rfind('\\');
                if (lastSlash == std::string::npos) lastSlash = -1;
                if (lastRslash == std::string::npos) lastRslash = -1;
                size_t result = (lastSlash > lastRslash ? lastRslash : lastSlash);
                
                std::string onlyName = fullName.substr(result + 1);
                
                // printf("\n%d vs %d = %d: result in %s\n", lastSlash, 
                                                          // lastRslash, 
                                                          // result, 
                                                          // onlyName.c_str()
                // );
                
                modelsFromConfig.emplace_back( std::pair(fullName,onlyName) );
                    //std::cout << "Adding " << fullName << std::endl;
            }
        }
        
        std::cout << "getModelsList finished" << std::endl;
    }
    
    void getFilesList(){
        promptFiles.clear();
        
        if ( std::filesystem::exists(promptFilesFolder) ){
            for (auto const& file : std::filesystem::directory_iterator{promptFilesFolder}) 
            {
                if (file.path().extension() == ".txt") promptFiles.emplace_back( file.path());
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
            std::string prompt;
            std::string tail;
            std::copy(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>(), back_inserter(prompt));
            inputPromptExt = prompt;
            
            while (prompt.back() == '\n') {
                tail += prompt.back();
                prompt.pop_back();
            }
            if (!headless){
                int antiPos = prompt.rfind('\n');
                if (antiPos != prompt.npos) {
                    inputAntipromptExt = prompt.substr(antiPos + 1) + tail;
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
    
    void getSettingsFromJson(nlohmann::json& config){
        readParamsFromJson(config, params);
    }

    void getSettingsFull(){
        params = paramsDefault;
        readParamsFromJson(localConfig, params);
        if (localConfig.contains("name")) charName = localConfig["name"];
        else charName.clear();
        //readParamsFromJson(modelConfig, params);
        modelName = params.model;
        syncInputs();
    }
    
    void syncInputs(){
        inputPrompt = params.prompt;
        inputSuffix = params.input_suffix;
        inputPrefix = params.input_prefix;
        if(params.antiprompt.size()) inputAntiprompt = params.antiprompt[0];
        if(params.sparams.cfg_negative_prompt.size()) inputAntiCFG = params.sparams.cfg_negative_prompt;
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
    
    bool checkInputSuffix(){
        if (!inputSuffix.empty() && inputSuffix != modelConfig[modelName]["input_suffix"].get<std::string>()) return false;
        
        return true;
    }
    
    bool checkInputPrefix(){
        if (!inputPrefix.empty() && inputPrefix != modelConfig[modelName]["input_prefix"].get<std::string>()) return false;
        
        return true;
    }
    
    bool checkInputPrompt(){
        if (inputPrompt != "NULL" && inputPrompt != modelConfig[modelName]["prompt"].get<std::string>()) return false;
        
        return true;
    }
    
    void cancelPromt(){
        inputPrompt = modelConfig[modelName]["prompt"].get<std::string>();
    }
    
    bool checkInputAntiprompt(){
        if (inputAntiprompt != "NULL" && inputAntiprompt != modelConfig[modelName]["reverse-prompt"].get<std::string>()) return false;
        
        return true;
    }
    
    void cancelAntipromt(){
        inputAntiprompt = modelConfig[modelName]["reverse-prompt"].get<std::string>();
    }
    
    bool checkInputAntiCFG(){
        if (!inputAntiCFG.empty() && inputAntiCFG != modelConfig[modelName]["cfg-negative-prompt"].get<std::string>()) return false;
        
        return true;
    }
    
    void cancelAntiCFG(){
        inputAntiCFG = modelConfig[modelName]["cfg-negative-prompt"].get<std::string>();
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
    
    void fillLocalJson(std::string& model, bool clear = true){
        if (clear) modelConfig.clear();
        
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
        if (!params.lora_adapter.empty()) modelConfig[model]["lora"] = std::get<0>(params.lora_adapter[0]);
        
        
        if (params.sparams.temp != paramsDefault.sparams.temp) modelConfig[model]["temp"] = params.sparams.temp;
        if (params.sparams.dynatemp_range != paramsDefault.sparams.dynatemp_range) modelConfig[model]["dynatemp_range"] = params.sparams.dynatemp_range;
        if (params.sparams.temp_smoothing != paramsDefault.sparams.temp_smoothing) modelConfig[model]["temp_smoothing"] = params.sparams.temp_smoothing;
        if (params.sparams.top_k != paramsDefault.sparams.top_k) modelConfig[model]["top_k"] = params.sparams.top_k;
        if (params.sparams.top_p != paramsDefault.sparams.top_p) modelConfig[model]["top_p"] = params.sparams.top_p;
        if (params.sparams.min_p != paramsDefault.sparams.min_p) modelConfig[model]["min_p"] = params.sparams.min_p;
        if (params.sparams.tfs_z != paramsDefault.sparams.tfs_z) modelConfig[model]["tfs_z"] = params.sparams.tfs_z;
        if (params.sparams.typical_p != paramsDefault.sparams.typical_p) modelConfig[model]["typical_p"] = params.sparams.typical_p;
        if (params.sparams.penalty_repeat != paramsDefault.sparams.penalty_repeat) modelConfig[model]["repeat_penalty"] = params.sparams.penalty_repeat;
        if (params.sparams.penalty_freq != paramsDefault.sparams.penalty_freq) modelConfig[model]["frequency_penalty"] = params.sparams.penalty_freq;
        if (params.sparams.penalty_present != paramsDefault.sparams.penalty_present) modelConfig[model]["presence_penalty"] = params.sparams.penalty_present;
        if (params.sparams.mirostat != paramsDefault.sparams.mirostat) modelConfig[model]["mirostat"] = params.sparams.mirostat;
        if (params.sparams.mirostat_tau != paramsDefault.sparams.mirostat_tau) modelConfig[model]["mirostat_tau"] = params.sparams.mirostat_tau;
        if (params.sparams.mirostat_eta != paramsDefault.sparams.mirostat_eta) modelConfig[model]["mirostat_eta"] = params.sparams.mirostat_eta;
        if (params.sparams.cfg_scale != paramsDefault.sparams.cfg_scale) modelConfig[model]["cfg-scale"] = params.sparams.cfg_scale;
        if (params.n_ctx != paramsDefault.n_ctx) modelConfig[model]["ctx-size"] = params.n_ctx;
        if (params.grp_attn_n != paramsDefault.grp_attn_n) modelConfig[model]["grp_attn_n"] = params.grp_attn_n;
        if (params.grp_attn_w != paramsDefault.grp_attn_w) modelConfig[model]["grp_attn_w"] = params.grp_attn_w;
        if (params.n_keep != paramsDefault.n_keep) modelConfig[model]["n_keep"] = params.n_keep;
        if (params.n_batch != paramsDefault.n_batch) modelConfig[model]["n_batch"] = params.n_batch;
        if (params.n_threads != paramsDefault.n_threads) modelConfig[model]["n_threads"] = params.n_threads;
        
        if (params.format_instruct != paramsDefault.format_instruct) modelConfig[model]["format_instruct"] = params.format_instruct;
        if (params.format_dialog != paramsDefault.format_dialog) modelConfig[model]["format_dialog"] = params.format_dialog;
        if (params.bos != paramsDefault.bos) modelConfig[model]["bos"] = params.bos;
        if (params.eos != paramsDefault.eos) modelConfig[model]["eos"] = params.eos;
        if (params.sparams.samplers_sequence != paramsDefault.sparams.samplers_sequence) modelConfig[model]["samplers_sequence"] = params.sparams.samplers_sequence;
        
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
        if (params.yarn_beta_slow != paramsDefault.yarn_beta_slow) modelConfig[model]["yarn_beta_slow"] = params.yarn_beta_slow;
        if (params.yarn_orig_ctx != paramsDefault.yarn_orig_ctx) modelConfig[model]["yarn_orig_ctx"] = params.yarn_orig_ctx;
        if (params.yarn_attn_factor != paramsDefault.yarn_attn_factor) modelConfig[model]["yarn_attn_factor"] = params.yarn_attn_factor;
        
        if (params.rope_scaling_type != paramsDefault.rope_scaling_type) {
            if (params.rope_scaling_type == LLAMA_ROPE_SCALING_NONE) modelConfig[model]["rope_scaling_type"] = "none";
            else if (params.rope_scaling_type == LLAMA_ROPE_SCALING_LINEAR) modelConfig[model]["rope_scaling_type"] = "linear";
            else if (params.rope_scaling_type == LLAMA_ROPE_SCALING_YARN) modelConfig[model]["rope_scaling_type"] = "yarn";
        }
        
        modelConfig[model]["cfg-negative-prompt"] = params.sparams.cfg_negative_prompt;
        if (params.prompt.size()) modelConfig[model]["prompt"] = params.prompt;
        else {
            modelConfig[model]["prompt"] = "### Instruction:";
            params.prompt = "### Instruction:";
        }
        if(params.antiprompt.size()) modelConfig[model]["reverse-prompt"] = params.antiprompt[0];
        else {
            modelConfig[model]["reverse-prompt"] = "### Instruction:";
            params.antiprompt.emplace_back("### Instruction:");
        }
        
        if(grammarFile != "") modelConfig[model]["grammar-file"] = grammarFile;
        
        //modelConfig["temp_first"] = tempFirst;
        if (!charName.empty()) modelConfig["name"] = charName;
        
        modelConfig[model]["seed"] = params.seed;
        
    }
    
    nlohmann::json createNewCard(){
        
        nlohmann::json newCard;
        
        if (!params.input_suffix.empty()) newCard["input_suffix"] = params.input_suffix;
        if (!params.input_prefix.empty()) newCard["input_prefix"] = params.input_prefix;
        if (params.input_prefix_bos != paramsDefault.input_prefix_bos) newCard["input_prefix_bos"] = params.input_prefix_bos;
        
        if (params.sparams.penalize_nl != paramsDefault.sparams.penalize_nl) newCard["penalize_nl"] = params.sparams.penalize_nl;
        if (params.use_mmap != paramsDefault.use_mmap) newCard["use_mmap"] = params.use_mmap;
        if (!params.lora_adapter.empty()) newCard["lora"] = std::get<0>(params.lora_adapter[0]);
        
        
        if (params.sparams.temp != paramsDefault.sparams.temp) newCard["temp"] = params.sparams.temp;
        if (params.sparams.dynatemp_range != paramsDefault.sparams.dynatemp_range) newCard["dynatemp_range"] = params.sparams.dynatemp_range;
        if (params.sparams.temp_smoothing != paramsDefault.sparams.temp_smoothing) newCard["temp_smoothing"] = params.sparams.temp_smoothing;
        if (params.sparams.top_k != paramsDefault.sparams.top_k) newCard["top_k"] = params.sparams.top_k;
        if (params.sparams.top_p != paramsDefault.sparams.top_p) newCard["top_p"] = params.sparams.top_p;
        if (params.sparams.min_p != paramsDefault.sparams.min_p) newCard["min_p"] = params.sparams.min_p;
        if (params.sparams.tfs_z != paramsDefault.sparams.tfs_z) newCard["tfs_z"] = params.sparams.tfs_z;
        if (params.sparams.typical_p != paramsDefault.sparams.typical_p) newCard["typical_p"] = params.sparams.typical_p;
        if (params.sparams.penalty_repeat != paramsDefault.sparams.penalty_repeat) newCard["repeat_penalty"] = params.sparams.penalty_repeat;
        if (params.sparams.penalty_freq != paramsDefault.sparams.penalty_freq) newCard["frequency_penalty"] = params.sparams.penalty_freq;
        if (params.sparams.penalty_present != paramsDefault.sparams.penalty_present) newCard["presence_penalty"] = params.sparams.penalty_present;
        if (params.sparams.mirostat != paramsDefault.sparams.mirostat) newCard["mirostat"] = params.sparams.mirostat;
        if (params.sparams.mirostat_tau != paramsDefault.sparams.mirostat_tau) newCard["mirostat_tau"] = params.sparams.mirostat_tau;
        if (params.sparams.mirostat_eta != paramsDefault.sparams.mirostat_eta) newCard["mirostat_eta"] = params.sparams.mirostat_eta;
        if (params.sparams.cfg_scale != paramsDefault.sparams.cfg_scale) newCard["cfg-scale"] = params.sparams.cfg_scale;
        if (params.n_ctx != paramsDefault.n_ctx) newCard["ctx-size"] = params.n_ctx;
        if (params.n_keep != paramsDefault.n_keep) newCard["n_keep"] = params.n_keep;
        if (params.n_batch != paramsDefault.n_batch) newCard["n_batch"] = params.n_batch;
        if (params.n_threads != paramsDefault.n_threads) newCard["n_threads"] = params.n_threads;
        
        //#if GGML_EXPERIMENTAL1
        if (params.n_threads_batch != paramsDefault.n_threads_batch) newCard["n_threads_batch"] = params.n_threads_batch;
        //#endif
        if (params.n_gpu_layers != paramsDefault.n_gpu_layers) newCard["n_gpu_layers"] = params.n_gpu_layers;
        
        #if GGML_OLD_FORMAT
        if (params.rms_norm_eps != paramsDefault.rms_norm_eps) newCard["rms-norm-eps"] = params.rms_norm_eps;
        #else 
        if (params.n_threads_batch != paramsDefault.n_threads_batch) newCard["n_threads_batch"] = params.n_threads_batch;
        #endif
        
        if (params.rope_freq_base != paramsDefault.rope_freq_base) newCard["rope_freq_base"] = params.rope_freq_base;
        if (params.rope_freq_scale != paramsDefault.rope_freq_scale) newCard["rope_freq_scale"] = params.rope_freq_scale;
        if (params.yarn_beta_slow != paramsDefault.yarn_beta_slow) newCard["yarn_beta_slow"] = params.yarn_beta_slow;
        if (params.yarn_orig_ctx != paramsDefault.yarn_orig_ctx) newCard["yarn_orig_ctx"] = params.yarn_orig_ctx;
        if (params.yarn_attn_factor != paramsDefault.yarn_attn_factor) newCard["yarn_attn_factor"] = params.yarn_attn_factor;
        
        if (params.rope_scaling_type != paramsDefault.rope_scaling_type) {
            if (params.rope_scaling_type == LLAMA_ROPE_SCALING_NONE) newCard["rope_scaling_type"] = "none";
            else if (params.rope_scaling_type == LLAMA_ROPE_SCALING_LINEAR) newCard["rope_scaling_type"] = "linear";
            else if (params.rope_scaling_type == LLAMA_ROPE_SCALING_YARN) newCard["rope_scaling_type"] = "yarn";
        }
        
        newCard["cfg-negative-prompt"] = params.sparams.cfg_negative_prompt;
        if (params.prompt.size()) newCard["prompt"] = params.prompt;
        else {
            newCard["prompt"] = "### Instruction:";
            params.prompt = "### Instruction:";
        }
        if(params.antiprompt.size()) newCard["reverse-prompt"] = params.antiprompt[0];
        else {
            newCard["reverse-prompt"] = "### Instruction:";
            params.antiprompt.emplace_back("### Instruction:");
        }
        
        if(grammarFile != "") newCard["grammar-file"] = grammarFile;
        
        return newCard;
    }
    
    void fillLocalJson(bool clear = true){
        fillLocalJson(modelName, clear);
    }
    
    void applySuffix(chat& aChat){
        aChat.params.input_suffix = params.input_suffix;
    }
    
    bool isNewSuffix(chat& aChat){
        return aChat.params.input_suffix != params.input_suffix;
    }
    
    void updateInput(){
        if (inputPrompt != "NULL") params.prompt = inputPrompt;
        params.input_suffix = inputSuffix;
        params.input_prefix = inputPrefix;
        if (inputAntiprompt != "NULL") {
            if(!params.antiprompt.size()) 
                params.antiprompt.emplace_back(inputAntiprompt);
            else
                params.antiprompt[0] = inputAntiprompt;
        }
        if (!inputAntiCFG.empty()) params.sparams.cfg_negative_prompt = inputAntiCFG;
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
    
    void readJsonToParams(nlohmann::json& file){
        readParamsFromJson(file, params);
        
        if (!file.contains("model") && file.contains(params.model)){
            nlohmann::json subJson = file[params.model];
            readParamsFromJson(subJson, params);
        }
        
        fillLocalJson(params.model);
    }
    
    void checkLocalConfig(){
        if (noConfig == true && modelName != "NULL" && inputPrompt != "NULL"){
            noConfig = false;
        }
    }
    
    void resetConfig(std::string fileName){
        localConfig.clear();
        modelConfig.clear();
        params = paramsDefault;
        getFromJson(fileName);
        readParamsFromJson(localConfig, modelName, params);
        fillLocalJson();
        updateDump();
    }
};

struct presetTest{
    nlohmann::json testParams;
    std::string subname;
    std::string saveFolder = "R";
    unsigned int seed = 0;
    int cycle = 0;
    bool cycling = 0;
    std::vector<std::string> presetsNames;
    std::string prompt;
    
    void init(std::string filename){
        std::cout << "Initializing presets test..." << std::endl;
        
        seed = getRand();
        
        if (filename.empty()) testParams = getJson("presetsTest.json");
        else {
            testParams = getJson(filename);
        }
        
        if (testParams.contains("prompt")) {
            prompt = testParams["prompt"];
            
        } else {
            prompt = filename;
            testParams = getJson("presetsTest.json");
        }
        
        if (testParams.contains("presets")){
            
            presetsNames = testParams["presets"];
                    
            if (testParams.contains("folder")) saveFolder = testParams["folder"];
        
        }
        
        
        
    }
    
    void init(nlohmann::json& file){
        std::cout << "Initializing presets test..." << std::endl;
        
        seed = getRand();
        
        testParams = file;
        
        if (testParams.contains("prompt")) {
            prompt = testParams["prompt"];
        } else {
            std::string input2;
            std::getline(std::cin, input2);
            
            prompt = input2;
            
        }
            
        presetsNames = testParams["presets"];
                    
        if (testParams.contains("folder")) saveFolder = testParams["folder"];
    }
    
    std::string writeName(){
        std::string name = presetsNames[cycle];
                    
        size_t slash = presetsNames[cycle].rfind('/');
        if (slash != presetsNames[cycle].npos) name = presetsNames[cycle].substr(slash+1);
        
        name = std::to_string(seed) + "-" + name;
        std::cout << "Writing into " << name << std::endl;
        
        return name;
    }
    
    int startTest(nlohmann::json& jsonFile, configurableChat& settings, modelThread& threadedChat, bool writeExternal = true, bool streaming = true, bool waiting = true, int latency = 20){
    
        
        
        init(jsonFile);
        if (jsonFile.contains("config")){
            std::cout << " Found config inside presets test file..." << std::endl;
            if (jsonFile["config"].is_object()){
                //readParamsFromJson(jsonFile["config"], settings.params);
                nlohmann::json config = jsonFile["config"];
                settings.readJsonToParams(config);
            }
        }
        settings.modelConfig["card"] = presetsNames[cycle];
        settings.modelConfig["seed"] = seed;
        if (writeExternal) threadedChat.externalData = "Preset: " + presetsNames[cycle] + "(" + std::to_string(cycle + 1) + "/" + std::to_string(presetsNames.size()) + ")";
        threadedChat.load(settings.modelConfig, false);
        
        while(cycle != presetsNames.size()){
            if (threadedChat.isContinue != 'i') {
                if (waiting) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(latency));
                    //if (threadedChat.loaded == 9 && streaming) threadedChat.display();
                }
            } else {
                //if (streaming) threadedChat.display();
                std::cout << "Test cycle " << std::to_string(cycle) << std::endl;
                if(cycling == 1){
                    threadedChat.writeTextFileFull(saveFolder + '/', writeName());
                    cycle++;
                    
                    if (cycle == presetsNames.size()) {
                        threadedChat.unload();
                        if (writeExternal) threadedChat.externalData = "Test finished!";
                        return 0;
                    } else {
                        cycling = 0;
                        threadedChat.unload();
                        settings.modelConfig["card"] = presetsNames[cycle];
                        settings.modelConfig["seed"] = seed;
                        if (writeExternal) threadedChat.externalData = "Preset: " + presetsNames[cycle] + "(" + std::to_string(cycle + 1) + "/" + std::to_string(presetsNames.size()) + ")";
                        threadedChat.load(settings.modelConfig, false);
                    }
                } else {
                    threadedChat.appendQuestion(prompt);
                    //if (streaming) threadedChat.display();
                    threadedChat.startGen();
                    threadedChat.getResultAsyncStringFull2(streaming, true);
                    //threadedChat.getResultAsyncStringFull3();
                    cycling = 1;
                }
            }
        }
        
        return 1;
    }
};

struct wildcardGen {
    nlohmann::json wildcardsDB;
    std::string subname;
    std::string saveFolder = "R";
    int cycles = -1;
    bool cycling = 0;
    unsigned int seed = 0;
    std::vector<std::string> promptsDB;
    std::string prompt0;
    std::string prompt;
    std::string preset;
    
    void init(std::string filename){
        std::cout << "Initializing wildcarded generator..." << std::endl;
        
        seed = getRand();
        
        if (filename.empty()) wildcardsDB = getJson("wildcards.json");
        else {
            wildcardsDB = getJson(filename);
        }
        
        if (wildcardsDB.contains("cycles")) cycles = wildcardsDB["cycles"];
        else cycles = 10;
                    
        if (wildcardsDB.contains("folder")) saveFolder = wildcardsDB["folder"];
        
        if (wildcardsDB.contains("preset")) preset = wildcardsDB["preset"];
        
        if (wildcardsDB.contains("prompts")) promptsDB = wildcardsDB["prompts"];
        else {
            promptsDB.emplace_back(filename); // assuming that we passed the prompt itself instead of filename;
            std::string cNum;
            std::cout << " Enter the number of cycles: " << std::endl;
            std::getline(std::cin, cNum);
            cycles = std::stoi(cNum);
        }
        
    }
    
    void init(nlohmann::json& file){
        std::cout << "Initializing wildcarded generator..." << std::endl;
        
        seed = getRand();
        
        wildcardsDB = file;
        
        cycles = wildcardsDB["cycles"];
                    
        if (wildcardsDB.contains("folder")) saveFolder = wildcardsDB["folder"];
        
        subname = std::to_string(seed);
        
        if (wildcardsDB.contains("preset")) {
            preset = wildcardsDB["preset"];
            size_t slash = preset.rfind('/');
            if (slash != preset.npos) subname += "-" + preset.substr(slash+1);
            else subname += "-" + preset;
        }
        
        if (wildcardsDB.contains("prompts")) promptsDB = wildcardsDB["prompts"];
        else if (promptsDB.empty()) {
            std::string input2;
            std::getline(std::cin, input2);
            
            promptsDB.emplace_back(input2);
        }
        
    }
    
    void getPrompt(){
        unsigned int x = getRand() % promptsDB.size();
        prompt0 = promptsDB[x];
        
        prompt = findWildcard(prompt0);
    }
    
    std::string findWildcard(std::string inputString){
    
        size_t last = inputString.rfind("__");
        if (last != inputString.npos){
            
            std::string subInputString = inputString.substr(0,last);
            std::cout << " subInputString: " << subInputString << std::endl;
            size_t first = subInputString.rfind("__");
            if (first != subInputString.npos){
                std::string wildcard = subInputString.substr(first + 2);
                
                
                if(wildcardsDB.contains(wildcard)) {
                    
                    std::vector<std::string> wildcardsArr = wildcardsDB[wildcard];
                    unsigned int i = getRand() % wildcardsArr.size();
                    std::string choice = wildcardsArr[i];
                    inputString.replace(first,last - first + 2,choice);
                    std::cout << "Replaced: " << inputString << std::endl;
                    
                    if (wildcardsDB.contains("subname")){
                        if (wildcard == wildcardsDB["subname"].get<std::string>()){
                            subname += "-" + choice;
                        }
                    }
                    
                    return findWildcard(inputString);
                    
                } else std::cout << " no wildcard: " << wildcard << std::endl;
            } else std::cout << " no first __" << std::endl;
        } else std::cout << " no last __" << std::endl;
            
        return inputString;
    }
    
    int cycle(nlohmann::json& jsonFile, configurableChat& settings, modelThread& threadedChat, bool writeExternal = true, bool streaming = true, bool waiting = true, int latency = 20){
        
        
        init(jsonFile);
        if (jsonFile.contains("config")){
            std::cout << " Found config inside cycle test file..." << std::endl;
            if (jsonFile["config"].is_object()){
                //readParamsFromJson(jsonFile["config"], settings.params);
                nlohmann::json config = jsonFile["config"];
                if (config.contains("card")) preset = config["card"];
                if (config.contains("preset")) preset = config["preset"];
                settings.readJsonToParams(config);
            }
        }
        //if (!preset.empty()) settings.modelConfig["card"] = preset;
        //if (writeExternal) threadedChat.externalData = "-Preset: " + preset + "\n-Cycles left: " + std::to_string(cycles) + "\n-File name: " + subname + "-"+ std::to_string(threadedChat.newChat.params.seed) + "\n";
        threadedChat.load(settings.modelConfig, false);
        
        while(cycles >= 0){
            if (threadedChat.isContinue != 'i') {
                if (waiting) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(latency));
                    //if (threadedChat.loaded == 9 && streaming) threadedChat.display();
                }
            } else {
                if(cycling == 1){
                    threadedChat.writeTextFileFull(saveFolder + '/', subname + "-" + std::to_string(cycles) + "-"+ std::to_string(threadedChat.newChat.params.seed));
                    if (cycles == 0) {
                        threadedChat.unload();
                        if (writeExternal) threadedChat.externalData = "Test finished!";
                        return 0;
                    } else {
                        cycling = 0;
                        //input = "restart";
                        settings.modelConfig["card"] = preset;
                        threadedChat.unload();
                        threadedChat.load(settings.modelConfig, false);
                    }
                } else {
                    --cycles;
                    if (writeExternal) threadedChat.externalData = "-Preset: " + preset + "\n-Cycles left: " + std::to_string(cycles) + "\n-File subname: " + subname + "-" + std::to_string(cycles) + "-"+ std::to_string(threadedChat.newChat.params.seed) + "\n";
                    getPrompt();
                    //input = Card.prompt;
                    processPrompt(prompt);
                    threadedChat.appendQuestion(prompt);
                    //if (streaming) threadedChat.display();
                    threadedChat.startGen();
                    threadedChat.getResultAsyncStringFull2(streaming, true);
                    //threadedChat.getResultAsyncStringFull3();
                    cycling = 1;
                }
            }
        }
    } 
};

