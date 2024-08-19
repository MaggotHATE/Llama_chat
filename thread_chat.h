#pragma once

#include <future>
#include <chrono>
#include <thread>
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

#ifdef GGML_USE_CLBLAST
    extern std::string GGML_OPENCL_RESULT_DEVICE_NAME;
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

static void remove_last_nl(std::string& msg) {
    if (msg.back() == '\n' || msg.back() == '\r') {
        msg.pop_back();
    }
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
    int remain_tokens = 0;
    int consumed_tokens = 0;
    int past_tokens = 0;
    int left_tokens = 0;
    
    bool penalize_nl = false;
    
    std::string lastTimings = "Not yet calculated...";
    float lastSpeed = 0.0f;
    float lastSpeedPrompt = 0.0f;
    std::string lastResult = "";
    std::string shortModelName = "";
    std::string sparamsList = "";
    std::string sparamsListShort = "";
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
        sparamsListShort = newChat.getSparamsChanged(false);
    }
    
    void appendFirstPrompt(){
        std::string context;

        if (newChat.params.prompt.length() > 0) {
            context = newChat.params.prompt;

            if (newChat.params.antiprompt.size()) {
                if (newChat.params.prompt != newChat.params.antiprompt[0]){
                    int cutAntiPos = context.rfind(newChat.params.antiprompt[0]);
                    if (cutAntiPos != std::string::npos){
                        context.erase(cutAntiPos);
                    }

                }
            }
        } else {
            
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
        //if (penalize_nl) remove_last_nl(input);
        resultsStringPairs.emplace_back(std::pair(newChat.params.antiprompt[0],input));
        //else resultsStringPairs.emplace_back(std::pair(newChat.params.input_prefix,input));
    }
    
    void removeLastAnswer(){
        resultsStringPairs.pop_back();
    }
    
    void switch_debug() {
        newChat.switch_debug();
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
    
    std::string display(){
        Clear();
        getTimigsPre();
        getSparamsList();
        std::string summary = "-";
        std::cout << "----------------------------------------"<< std::endl;
        std::cout << "Model: " << shortModelName << std::endl;
        std::cout << "Seed: " << std::to_string(newChat.params.seed) << std::endl;
        std::cout << "ctx size: " << std::to_string(newChat.params.n_ctx) << std::endl;
        std::cout << "batch size: " << std::to_string(newChat.params.n_batch) << std::endl;
        std::cout << "ubatch size: " << std::to_string(newChat.params.n_ubatch) << std::endl;
#ifdef GGML_USE_CLBLAST
        std::cout << "GPU: " << GGML_OPENCL_RESULT_DEVICE_NAME << std::endl;
#endif
        std::cout << "----------------------------------------" << std::endl;
        if (newChat.params.antiprompt.size()) {
            for (auto antiprompt : newChat.params.antiprompt) std::cout << "Antiprompt: " << antiprompt << std::endl;
        }
        std::cout << "input_prefix: " << newChat.params.input_prefix << std::endl;
        std::cout << "input_suffix: " << newChat.params.input_suffix << "\n----------------------------------------" << std::endl;
        //std::cout << newChat.formatRepresentation << std::endl;
        std::cout << externalData << std::endl;
        std::cout << sparamsList << std::endl;
        std::cout << "\nSTATUS       : " << (newChat.finished ? "READY" : "BUSY") << std::endl;
        std::cout << "WAITING      : " << (is_interacting ? "YES" : "NO") << std::endl;
        std::cout << "isContinue   : " << isContinue << std::endl;
        std::cout << "Past         : " << past_tokens << std::endl;
        std::cout << "Consumed     : " << consumed_tokens << std::endl;
        std::cout << "Remain       : " << remain_tokens << std::endl;
        std::cout << "Last         : " << last_tokens << std::endl;
        std::cout << "Past-Last    : " << past_tokens - last_tokens << std::endl;
        std::cout << "embd_inp.size: " << newChat.getEmbInpSize() << '\n' << std::endl;
        std::cout << "n_past_last  : " << newChat.n_past_last << std::endl;
        std::cout << "n_embd_inp_last  : " << newChat.n_embd_inp_last << std::endl;
        std::cout << "n_consumed_last  : " << newChat.n_consumed_last << '\n' << std::endl;
        
        //#ifdef GGML_EXPERIMENTAL1
        std::cout << "Threads: " << newChat.params.n_threads << "/" << newChat.params.n_threads_batch << '\n' << std::endl;
        if (penalize_nl) std::cout << "penalize_nl = true" << std::endl;
        if (newChat.params.use_mmap) std::cout << "use_mmap = true" << std::endl;
        if (newChat.params.no_kv_offload) std::cout << "no_kv_offload = true" << std::endl;
        if (newChat.params.flash_attn) std::cout << "flash_attn = true" << '\n' << std::endl;
        //#endif
        //std::cout << lastTimings << std::endl;
        std::cout << std::format("Eval speed: {:.3f} t/s | Gen speed: {:.3f} t/s", lastSpeedPrompt, lastSpeed) << std::endl;
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
            
            //if (r.second.back() != '\n') std::cout<< DELIMINER;
        }
        if (isContinue == 'w') {
#ifdef GGML_USE_VULKAN
            summary += std::format("VK{}|",newChat.params.n_gpu_layers);
#elif defined(GGML_USE_CLBLAST)
            summary += std::format("CL{}|",newChat.params.n_gpu_layers);
#endif
            summary += std::format("{}|Msg: {}t|Left: {}t|in {:.3f}t/s|out {:.3f}t/s", sparamsListShort, last_tokens, left_tokens, lastSpeedPrompt, lastSpeed);
            std::cout << lastResult;
            std::cout << "\n----------------------------------------------------------------------------------------------\n" << summary;
        }
        //if (last_tokens > 0) std::cout << "Generated: " << last_tokens << std::endl;
        //std::cout<< DELIMINER;
        
        return summary;
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
        std::string path1 = path + std::to_string(newChat.params.seed) + "-" + name  + "-" + shortModelName + ".txt";
        std::ofstream file(path1, std::ios::app);
        if (file.is_open()) {
            file << shortModelName << DELIMINER;
            file << sparamsList << DELIMINER;
            file << "Threads: " << newChat.params.n_threads << "/" << newChat.params.n_threads_batch << std::endl;
            file << lastTimings << std::endl;
            file << "Prompt:" << consumed_tokens << std::endl;
            file << "Result: " << last_tokens << std::endl;
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

    bool writeTextFileDivided(std::string path, std::string name){
        std::string path1 = path + std::to_string(newChat.params.seed) + "-" + name  + "-" + shortModelName + "-I.txt";
        std::string path2 = path + std::to_string(newChat.params.seed) + "-" + name  + "-" + shortModelName + "-O.txt";
        std::ofstream file_i(path1, std::ios::app);
        std::ofstream file_o(path2, std::ios::app);
        if (file_i.is_open() && file_o.is_open()) {
            file_i << shortModelName << DELIMINER;
            file_i << sparamsList << DELIMINER;
            file_i << "Threads: " << newChat.params.n_threads << "/" << newChat.params.n_threads_batch << std::endl;
            file_i << lastTimings << std::endl;
            file_i << "Prompt:" << consumed_tokens << std::endl;
            file_i << "Result: " << last_tokens << std::endl;
            file_i << "----------------------------------------\n"<< std::endl;
            for (auto r : resultsStringPairs) {
                // file << r.first << DELIMINER;
                // file << ' ' << r.second << DELIMINER;
                file_i << '\n' << r.second << DELIMINER;
            }

            file_i.close();
            file_o << '\n' << resultsStringPairs.back().second << DELIMINER;
            file_o.close();
            return true;
        } else {
            return false;
        }
    }

    bool writeTextFileUnified(std::string path, std::string name, bool first_time){
        std::string path1 = path + std::to_string(newChat.params.seed) + "-INPUT-" + shortModelName + ".txt";
        std::string path2 = path + std::to_string(newChat.params.seed) + "-" + name  + "-" + shortModelName + "-O.txt";
        std::ofstream file_o(path2, std::ios::app);
        if (file_o.is_open()) {
            if (first_time) {
                std::ofstream file_i(path1, std::ios::app);
                if (file_i.is_open()) {
                    file_i << shortModelName << DELIMINER;
                    file_i << sparamsList << DELIMINER;
                    file_i << "Threads: " << newChat.params.n_threads << "/" << newChat.params.n_threads_batch << std::endl;
                    file_i << lastTimings << std::endl;
                    file_i << "Prompt:" << consumed_tokens << std::endl;
                    file_i << "Result: " << last_tokens << std::endl;
                    file_i << "----------------------------------------\n"<< std::endl;
                    for (auto r : resultsStringPairs) {
                        // file << r.first << DELIMINER;
                        // file << ' ' << r.second << DELIMINER;
                        file_i << '\n' << r.second << DELIMINER;
                    }

                    file_i.close();
                }
            }
            std::string summary = "";
#ifdef GGML_USE_VULKAN
            summary += "vk|";
#elif defined(GGML_USE_CLBLAST)
            summary += "cl|";
#endif
            summary += std::format("{}|Msg: {}t|Left: {}t|in {:.3f}t/s|out {:.3f}t/s", sparamsListShort, last_tokens, left_tokens, lastSpeedPrompt, lastSpeed);

            file_o << summary << DELIMINER;
            file_o << '\n' << resultsStringPairs.back().second << DELIMINER;
            file_o.close();
            return true;
        } else {
            return false;
        }
    }

    bool writeTextFile(){
        std::string path = std::to_string(newChat.params.seed) + "-" + shortModelName + "-" + ".txt";
        std::ofstream file(path, std::ios::app);
        if (file.is_open()) {
            file << shortModelName << DELIMINER;
            file << std::to_string(newChat.params.seed) << DELIMINER;
            file << sparamsList << DELIMINER;
            file << "Threads: " << newChat.params.n_threads << "/" << newChat.params.n_threads_batch << std::endl;
            file << lastTimings << std::endl;
            file << "Prompt:" << consumed_tokens << std::endl;
            file << "Result: " << last_tokens << std::endl;
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
        lastResult = "";
        resultsStringPairs.clear();
        isContinue = '_';
        isPregen = '_';
        isLoading = '_';
        isUnload = '_';
    }
    
    void unloadSoft(){
        newChat.clearSoft();
        lastResult = "";
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
        left_tokens = newChat.params.n_ctx - past_tokens;
        remain_tokens = newChat.getRemainTokens();
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
                newChat.capture_lasts();

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
                newChat.capture_lasts();

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

            loaded = newChat.initialize(localConfig, false, soft);

            appendFirstPrompt();
            getTimigsBoth();
            //remain_tokens = newChat.getRemainTokens();
            consumed_tokens = newChat.getConsumedTokens();
            past_tokens = newChat.getPastTokens();

            penalize_nl = newChat.params.sparams.penalize_nl;

            //std::cout << "Loaded, you can type now! " << std::endl;
            getShortName();
            getSparamsList();
            
            if (isUnload == 'y') {
                unload();
            } else isContinue = 'i';

            getTimigsPre();

            return loaded;
        });
    }

    void load() {

        totalResult = std::async(std::launch::async, [this] mutable { 

            loaded = newChat.initialize(jsonConfig, false);

            appendFirstPrompt();
            //getTimigsSimple();
            //remain_tokens = newChat.getRemainTokens();
            consumed_tokens = newChat.getConsumedTokens();
            past_tokens = newChat.getPastTokens();

            penalize_nl = newChat.params.sparams.penalize_nl;

            //std::cout << "Loaded, you can type now! " << std::endl;
            getTimigsBoth();
            getShortName();
            getSparamsList();
            getTimigsPre();
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

            newChat.process_prompt();

            appendFirstPrompt();
            //getTimigsSimple();
            getTimigsBoth();
            //remain_tokens = newChat.getRemainTokens();
            consumed_tokens = newChat.getConsumedTokens();
            past_tokens = newChat.getPastTokens();

            penalize_nl = newChat.params.sparams.penalize_nl;

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
        aChat.params.sparams.p_step = params.sparams.p_step;
        aChat.params.sparams.penalty_repeat = params.sparams.penalty_repeat;
        aChat.params.sparams.penalty_threshold = params.sparams.penalty_threshold;
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
        std::cout << "fillLocalJson... " << std::endl;
        
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
        if (params.flash_attn != paramsDefault.flash_attn) modelConfig[model]["flash_attn"] = params.flash_attn;
        if (params.no_kv_offload != paramsDefault.no_kv_offload) modelConfig[model]["no_kv_offload"] = params.no_kv_offload;
        if (!params.lora_adapter.empty()) modelConfig[model]["lora"] = std::get<0>(params.lora_adapter[0]);
        
        if (params.sparams.temp_func != paramsDefault.sparams.temp_func) {
            std::cout << "Need to create an object: temp" << std::endl;
            modelConfig[model]["temp"]["value"] = params.sparams.temp;
            modelConfig[model]["temp"]["p_min"] = params.sparams.temp_func.p_min;
            modelConfig[model]["temp"]["p_max"] = params.sparams.temp_func.p_max;
            modelConfig[model]["temp"]["p_add"] = params.sparams.temp_func.p_add;
            modelConfig[model]["temp"]["p_mul"] = params.sparams.temp_func.p_mul;
            std::cout << "Object created! " << std::endl;
        } else if (params.sparams.temp != paramsDefault.sparams.temp) modelConfig[model]["temp"] = params.sparams.temp;
        
        if (params.sparams.dynatemp_range_func != paramsDefault.sparams.dynatemp_range_func) {
            std::cout << "Need to create an object: dynatemp_range" << std::endl;
            modelConfig[model]["dynatemp_range"]["value"] = params.sparams.dynatemp_range;
            modelConfig[model]["dynatemp_range"]["p_min"] = params.sparams.dynatemp_range_func.p_min;
            modelConfig[model]["dynatemp_range"]["p_max"] = params.sparams.dynatemp_range_func.p_max;
            modelConfig[model]["dynatemp_range"]["p_add"] = params.sparams.dynatemp_range_func.p_add;
            modelConfig[model]["dynatemp_range"]["p_mul"] = params.sparams.dynatemp_range_func.p_mul;
            std::cout << "Object created! " << std::endl;
        } else if (params.sparams.dynatemp_range != paramsDefault.sparams.dynatemp_range) modelConfig[model]["dynatemp_range"] = params.sparams.dynatemp_range;
        if (params.sparams.smoothing_factor != paramsDefault.sparams.smoothing_factor) modelConfig[model]["temp_smoothing"] = params.sparams.smoothing_factor;
        if (params.sparams.smoothing_factor != paramsDefault.sparams.smoothing_factor) modelConfig[model]["smoothing_factor"] = params.sparams.smoothing_factor;
        if (params.sparams.smoothing_curve != paramsDefault.sparams.smoothing_curve) modelConfig[model]["smoothing_curve"] = params.sparams.smoothing_curve;
        if (params.sparams.top_k != paramsDefault.sparams.top_k) modelConfig[model]["top_k"] = params.sparams.top_k;
        if (params.sparams.top_p != paramsDefault.sparams.top_p) modelConfig[model]["top_p"] = params.sparams.top_p;
        if (params.sparams.min_p != paramsDefault.sparams.min_p) modelConfig[model]["min_p"] = params.sparams.min_p;
        if (params.sparams.tfs_z != paramsDefault.sparams.tfs_z) modelConfig[model]["tfs_z"] = params.sparams.tfs_z;
        
        if (params.sparams.p_step_func != paramsDefault.sparams.p_step_func) {
            std::cout << "Need to create an object: p_step" << std::endl;
            modelConfig[model]["p_step"]["value"] = params.sparams.p_step;
            modelConfig[model]["p_step"]["p_min"] = params.sparams.p_step_func.p_min;
            modelConfig[model]["p_step"]["p_max"] = params.sparams.p_step_func.p_max;
            modelConfig[model]["p_step"]["p_add"] = params.sparams.p_step_func.p_add;
            modelConfig[model]["p_step"]["p_mul"] = params.sparams.p_step_func.p_mul;
            std::cout << "Object created! " << std::endl;
        } else if (params.sparams.p_step != paramsDefault.sparams.p_step) modelConfig[model]["p_step"] = params.sparams.p_step;
        if (params.sparams.xtc_probability != paramsDefault.sparams.xtc_probability) modelConfig[model]["xtc_probability"] = params.sparams.xtc_probability;
        if (params.sparams.xtc_threshold != paramsDefault.sparams.xtc_threshold) modelConfig[model]["xtc_threshold"] = params.sparams.xtc_threshold;
        // penalties
        if (params.sparams.penalty_repeat != paramsDefault.sparams.penalty_repeat) modelConfig[model]["repeat_penalty"] = params.sparams.penalty_repeat;
        if (params.sparams.penalty_threshold != paramsDefault.sparams.penalty_threshold) modelConfig[model]["penalty_threshold"] = params.sparams.penalty_threshold;
        if (params.sparams.penalty_freq != paramsDefault.sparams.penalty_freq) modelConfig[model]["frequency_penalty"] = params.sparams.penalty_freq;
        if (params.sparams.penalty_present != paramsDefault.sparams.penalty_present) modelConfig[model]["presence_penalty"] = params.sparams.penalty_present;
        if (params.sparams.penalty_last_n != paramsDefault.sparams.penalty_last_n) modelConfig[model]["penalty_last_n"] = params.sparams.penalty_last_n;
        // DRY
        if (params.sparams.dry_multiplier != paramsDefault.sparams.dry_multiplier) modelConfig[model]["dry_multiplier"] = params.sparams.dry_multiplier;
        if (params.sparams.dry_base != paramsDefault.sparams.dry_base) modelConfig[model]["dry_base"] = params.sparams.dry_base;
        if (params.sparams.dry_allowed_length != paramsDefault.sparams.dry_allowed_length) modelConfig[model]["dry_allowed_length"] = params.sparams.dry_allowed_length;
        if (params.sparams.dry_penalty_last_n != paramsDefault.sparams.dry_penalty_last_n) modelConfig[model]["dry_penalty_last_n"] = params.sparams.dry_penalty_last_n;
        //mirostat
        if (params.sparams.mirostat != paramsDefault.sparams.mirostat) modelConfig[model]["mirostat"] = params.sparams.mirostat;
        if (params.sparams.mirostat_tau != paramsDefault.sparams.mirostat_tau) modelConfig[model]["mirostat_tau"] = params.sparams.mirostat_tau;
        if (params.sparams.mirostat_eta != paramsDefault.sparams.mirostat_eta) modelConfig[model]["mirostat_eta"] = params.sparams.mirostat_eta;
        if (params.sparams.cfg_scale != paramsDefault.sparams.cfg_scale) modelConfig[model]["cfg-scale"] = params.sparams.cfg_scale;
        if (params.sparams.min_keep != paramsDefault.sparams.min_keep) modelConfig[model]["min_keep"] = params.sparams.min_keep;
        if (params.n_ctx != paramsDefault.n_ctx) modelConfig[model]["ctx-size"] = params.n_ctx;
        if (params.grp_attn_n != paramsDefault.grp_attn_n) modelConfig[model]["grp_attn_n"] = params.grp_attn_n;
        if (params.grp_attn_w != paramsDefault.grp_attn_w) modelConfig[model]["grp_attn_w"] = params.grp_attn_w;
        if (params.n_keep != paramsDefault.n_keep) modelConfig[model]["n_keep"] = params.n_keep;
        if (params.n_batch != paramsDefault.n_batch) modelConfig[model]["n_batch"] = params.n_batch;
        if (params.n_ubatch != paramsDefault.n_ubatch) modelConfig[model]["n_ubatch"] = params.n_ubatch;
        if (params.n_threads != paramsDefault.n_threads) modelConfig[model]["n_threads"] = params.n_threads;
        if (params.clblast_platform_id != paramsDefault.clblast_platform_id) modelConfig[model]["clblast_platform_id"] = params.clblast_platform_id;
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
            if (params.rope_scaling_type == LLAMA_ROPE_SCALING_TYPE_NONE) modelConfig[model]["rope_scaling_type"] = "none";
            else if (params.rope_scaling_type == LLAMA_ROPE_SCALING_TYPE_LINEAR) modelConfig[model]["rope_scaling_type"] = "linear";
            else if (params.rope_scaling_type == LLAMA_ROPE_SCALING_TYPE_YARN) modelConfig[model]["rope_scaling_type"] = "yarn";
        }
        
        modelConfig[model]["cfg-negative-prompt"] = params.sparams.cfg_negative_prompt;
        if (params.prompt.size()) modelConfig[model]["prompt"] = params.prompt;
        // else {
            // modelConfig[model]["prompt"] = "### Instruction:";
            // params.prompt = "### Instruction:";
        // }
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
        
        
        if (params.sparams.temp_func != paramsDefault.sparams.temp_func) {
            std::cout << "Need to create an object: temp" << std::endl;
            newCard["temp"]["value"] = params.sparams.temp;
            newCard["temp"]["p_min"] = params.sparams.temp_func.p_min;
            newCard["temp"]["p_max"] = params.sparams.temp_func.p_max;
            newCard["temp"]["p_add"] = params.sparams.temp_func.p_add;
            newCard["temp"]["p_mul"] = params.sparams.temp_func.p_mul;
            std::cout << "Object created! " << std::endl;
        } else if (params.sparams.temp != paramsDefault.sparams.temp) newCard["temp"] = params.sparams.temp;
        if (params.sparams.dynatemp_range != paramsDefault.sparams.dynatemp_range) newCard["dynatemp_range"] = params.sparams.dynatemp_range;
        if (params.sparams.smoothing_factor != paramsDefault.sparams.smoothing_factor) newCard["temp_smoothing"] = params.sparams.smoothing_factor;
        if (params.sparams.smoothing_factor != paramsDefault.sparams.smoothing_factor) newCard["smoothing_factor"] = params.sparams.smoothing_factor;
        if (params.sparams.smoothing_curve != paramsDefault.sparams.smoothing_curve) newCard["smoothing_curve"] = params.sparams.smoothing_curve;
        if (params.sparams.top_k != paramsDefault.sparams.top_k) newCard["top_k"] = params.sparams.top_k;
        if (params.sparams.top_p != paramsDefault.sparams.top_p) newCard["top_p"] = params.sparams.top_p;
        if (params.sparams.min_p != paramsDefault.sparams.min_p) newCard["min_p"] = params.sparams.min_p;
        if (params.sparams.tfs_z != paramsDefault.sparams.tfs_z) newCard["tfs_z"] = params.sparams.tfs_z;
        if (params.sparams.typical_p != paramsDefault.sparams.typical_p) newCard["typical_p"] = params.sparams.typical_p;
        if (params.sparams.p_step != paramsDefault.sparams.p_step) newCard["p_step"] = params.sparams.p_step;
        if (params.sparams.xtc_probability != paramsDefault.sparams.xtc_probability) newCard["xtc_probability"] = params.sparams.xtc_probability;
        if (params.sparams.xtc_threshold != paramsDefault.sparams.xtc_threshold) newCard["xtc_threshold"] = params.sparams.xtc_threshold;
        //penalties
        if (params.sparams.penalty_threshold != paramsDefault.sparams.penalty_threshold) newCard["penalty_threshold"] = params.sparams.penalty_threshold;
        if (params.sparams.penalty_repeat != paramsDefault.sparams.penalty_repeat) newCard["repeat_penalty"] = params.sparams.penalty_repeat;
        if (params.sparams.penalty_freq != paramsDefault.sparams.penalty_freq) newCard["frequency_penalty"] = params.sparams.penalty_freq;
        if (params.sparams.penalty_present != paramsDefault.sparams.penalty_present) newCard["presence_penalty"] = params.sparams.penalty_present;
        if (params.sparams.penalty_last_n != paramsDefault.sparams.penalty_last_n) newCard["penalty_last_n"] = params.sparams.penalty_last_n;
        //DRY
        if (params.sparams.dry_multiplier != paramsDefault.sparams.dry_multiplier) newCard["dry_multiplier"] = params.sparams.dry_multiplier;
        if (params.sparams.dry_base != paramsDefault.sparams.dry_base) newCard["dry_base"] = params.sparams.dry_base;
        if (params.sparams.dry_allowed_length != paramsDefault.sparams.dry_allowed_length) newCard["frequency_penalty"] = params.sparams.dry_allowed_length;
        if (params.sparams.dry_penalty_last_n != paramsDefault.sparams.dry_penalty_last_n) newCard["dry_penalty_last_n"] = params.sparams.dry_penalty_last_n;
        //mirostat
        if (params.sparams.mirostat != paramsDefault.sparams.mirostat) newCard["mirostat"] = params.sparams.mirostat;
        if (params.sparams.mirostat_tau != paramsDefault.sparams.mirostat_tau) newCard["mirostat_tau"] = params.sparams.mirostat_tau;
        if (params.sparams.mirostat_eta != paramsDefault.sparams.mirostat_eta) newCard["mirostat_eta"] = params.sparams.mirostat_eta;
        if (params.sparams.cfg_scale != paramsDefault.sparams.cfg_scale) newCard["cfg-scale"] = params.sparams.cfg_scale;
        if (params.sparams.min_keep != paramsDefault.sparams.min_keep) newCard["min_keep"] = params.sparams.min_keep;
        if (params.n_ctx != paramsDefault.n_ctx) newCard["ctx-size"] = params.n_ctx;
        if (params.n_keep != paramsDefault.n_keep) newCard["n_keep"] = params.n_keep;
        if (params.n_batch != paramsDefault.n_batch) newCard["n_batch"] = params.n_batch;
        if (params.n_ubatch != paramsDefault.n_ubatch) newCard["n_ubatch"] = params.n_ubatch;
        if (params.n_threads != paramsDefault.n_threads) newCard["n_threads"] = params.n_threads;
        if (params.clblast_platform_id != paramsDefault.clblast_platform_id) newCard["clblast_platform_id"] = params.clblast_platform_id;

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
            if (params.rope_scaling_type == LLAMA_ROPE_SCALING_TYPE_NONE) newCard["rope_scaling_type"] = "none";
            else if (params.rope_scaling_type == LLAMA_ROPE_SCALING_TYPE_LINEAR) newCard["rope_scaling_type"] = "linear";
            else if (params.rope_scaling_type == LLAMA_ROPE_SCALING_TYPE_YARN) newCard["rope_scaling_type"] = "yarn";
        }
        
        newCard["cfg-negative-prompt"] = params.sparams.cfg_negative_prompt;
        if (params.prompt.size()) newCard["prompt"] = params.prompt;
        // else {
            // newCard["prompt"] = "### Instruction:";
            // params.prompt = "### Instruction:";
        // }
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

