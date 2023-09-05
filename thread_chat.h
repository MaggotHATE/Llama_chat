#include <future>
#include <chrono>
#include <thread>
#include <locale>
#include <codecvt>


#ifdef GGML_OLD_FORMAT
#   include "GGML/chat_plain.h"
#elif defined(GGML_OLD_FORMAT)
#   include "GGML/chat_plain.h"
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
    std::string lastResult = "";

    // ~modelThread(){
        // ~newChat;
    // }
    
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
    
    void appendQuestion(std::string input){
        //resultsString.push_back(input);
        resultsStringPairs.push_back(std::pair(newChat.params.antiprompt[0],input));
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
        
        std::cout << "Generated: " << past_tokens << '\n' << std::endl;
        std::cout << lastTimings << std::endl;
        
        for (auto r : resultsStringPairs){
            if (r.first == "AI"){
                std::cout << r.second;
                //wstring converted;
                //fromUTF8(r.second, converted);
                //std::wcout << converted;
            } else {
                std::cout << r.first << r.second;
            }
            
            if (r.second.back() != '\n') std::cout<< DELIMINER;
        }
        
        //if (last_tokens > 0) std::cout << "Generated: " << last_tokens << std::endl;
        //std::cout<< DELIMINER;
    }
    
    bool writeTextFile(std::string path){
        std::ofstream file(path, std::ios::app);
        if (file.is_open()) {
            
            for (auto r : resultsStringPairs){
                file << r.first;
                file << r.second;
                file << DELIMINER;
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
            
            for (auto r : resultsStringPairs){
                file << r.first;
                file << r.second;
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
    
    void getTimigs(){
        lastTimings = newChat.get_timings();
    }
    
    void getTimigsSimple(){
        //lastTimings = newChat.get_timings_simple();
        lastTimings = newChat.get_ts();
    }
    
    void startGen(){
        newChat.finished = false;
        isContinue = 'w';
        lastResult = "";
    }
    
    void stop(){
        //newChat.finished = true;
        isContinue = 'i';
        resultsStringPairs.push_back(std::pair("AI",lastResult));
        getTimigsSimple();
    }
    
    // loading, preloading and generating threads  
    void getResultAsyncStringFull(bool streaming = false, bool full = false) {
        //isContinue = 'w';
        //std::cout << " ** " << input << std::endl;
        newChat.clearLastTokens();
        
        futureTextString = std::async(std::launch::async, [this, &streaming, &full] mutable {
            
            std::string input = resultsStringPairs.back().second;
                
            newChat.inputOnly(input);
            
            consumed_tokens = newChat.getConsumedTokens();
            past_tokens = newChat.getPastTokens();
            
            while (isContinue != 'i'){
        
                futureTextSubString = std::async(std::launch::async, [this, &streaming, &full] mutable {
                    //char* result = newChat.cycleInputSingleOutputString(input); 
                    
                    //std::string result = newChat.cycleStringsOnly(resultsString.back()); 
                    //resultsString.push_back(result);
                    //std::string input = resultsStringPairs.back().second;
                    std::string output = newChat.cycleStringsOnly(false);
                    lastResult += output;
                    
                    if (streaming) {
                        if (full) {
                            getTimigsSimple();
                            display();
                            std::cout << lastResult;
                        } else std::cout << output;
                    }
                    //getTimigsSimple();
                    
                    if (newChat.finished){
                        newChat.eraseAntiprompt(lastResult);
                        resultsStringPairs.push_back(std::pair("AI",lastResult));
                        isContinue = 'i';
                        isPregen = 'i';
                        getTimigsSimple();
                    }
                    
                    return lastResult;
                });
                
                //futureTextString.get();
                futureTextSubString.wait();
                last_tokens = newChat.getLastTokens();
                //remain_tokens = newChat.getRemainTokens();
                consumed_tokens = newChat.getConsumedTokens();
                past_tokens = newChat.getPastTokens();
            
            }
            
            std::string result = "ready";
                
            return result;
        });
        
    }
    
    void getResultAsyncStringRepeat(bool streaming = false) {
        //isContinue = 'w';
        //std::cout << " ** " << input << std::endl;
        
        
        newChat.clearLastEmbd();
        
        futureTextString = std::async(std::launch::async, [this, &streaming] mutable {
            
            while (isContinue != 'i'){
        
                futureTextSubString = std::async(std::launch::async, [this, &streaming] mutable {
                    //char* result = newChat.cycleInputSingleOutputString(input); 
                    
                    //std::string result = newChat.cycleStringsOnly(resultsString.back()); 
                    //resultsString.push_back(result);
                    //std::string input = resultsStringPairs.back().second;
                    std::string output = newChat.cycleStringsOnly(false);
                    if (streaming) std::cout << output;
                    lastResult += output;
                    //getTimigsSimple();
                    
                    if (newChat.finished){
                        newChat.eraseAntiprompt(lastResult);
                        resultsStringPairs.push_back(std::pair("AI",lastResult));
                        isContinue = 'i';
                        isPregen = 'i';
                        getTimigsSimple();
                        //last_tokens = newChat.getLastTokens();
                    }
                    
                    return lastResult;
                });
                
                //futureTextString.get();
                futureTextSubString.wait();
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
            getTimigsSimple();
            //remain_tokens = newChat.getRemainTokens();
            consumed_tokens = newChat.getConsumedTokens();
            past_tokens = newChat.getPastTokens();

            std::cout << "Loaded, you can type now! " << std::endl;
            isContinue = 'i';
            
            return loaded;
        });
    }
    
    void load() {
    
        totalResult = std::async(std::launch::async, [this] mutable { 

            loaded = newChat.initGenerate(jsonConfig, false);

            appendFirstPrompt();
            getTimigsSimple();
            //remain_tokens = newChat.getRemainTokens();
            consumed_tokens = newChat.getConsumedTokens();
            past_tokens = newChat.getPastTokens();

            std::cout << "Loaded, you can type now! " << std::endl;
            isContinue = 'i';
            
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
        params = paramsDefault;
        params = aChat.params;
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
        if(params.cfg_negative_prompt.size()) inputAntiCFG = params.cfg_negative_prompt;
    }
    
    int checkChangedInputs(){
        int result = 0;
        if (inputPrompt != "NULL" && inputPrompt != params.prompt) result += 1;
        if (inputAntiprompt != "NULL" && params.antiprompt.size() && inputAntiprompt != params.antiprompt[0]) result += 2;
        if (inputAntiCFG != "NULL" && inputAntiCFG != params.cfg_negative_prompt) result += 4;
        
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
        aChat.params.temp = params.temp;
        aChat.params.top_k = params.top_k;
        aChat.params.top_p = params.top_p;
        aChat.params.tfs_z = params.tfs_z;
        aChat.params.typical_p = params.typical_p;
        aChat.params.repeat_penalty = params.repeat_penalty;
        aChat.params.frequency_penalty = params.frequency_penalty;
        aChat.params.presence_penalty = params.presence_penalty;
        aChat.params.mirostat = params.mirostat;
        aChat.params.mirostat_tau = params.mirostat_tau;
        aChat.params.mirostat_eta = params.mirostat_eta;
        aChat.params.cfg_scale = params.cfg_scale;
        //aChat.params.cfg_smooth_factor = cfg_smooth_factor;
    }
    
    // void pushSettings(){
        // //aChat.params.n_threads = n_threads;
        // aChat.params.temp = params.temp;
        // aChat.params.top_k = params.top_k;
        // aChat.params.top_p = params.top_p;
        // aChat.params.tfs_z = params.tfs_z;
        // aChat.params.typical_p = params.typical_p;
        // aChat.params.repeat_penalty = params.repeat_penalty;
        // aChat.params.frequency_penalty = params.frequency_penalty;
        // aChat.params.presence_penalty = params.presence_penalty;
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
        
        if (params.temp != paramsDefault.temp) modelConfig[model]["temp"] = params.temp;
        if (params.top_k != paramsDefault.top_k) modelConfig[model]["top_k"] = params.top_k;
        if (params.top_p != paramsDefault.top_p) modelConfig[model]["top_p"] = params.top_p;
        if (params.tfs_z != paramsDefault.tfs_z) modelConfig[model]["tfs_z"] = params.tfs_z;
        if (params.typical_p != paramsDefault.typical_p) modelConfig[model]["typical_p"] = params.typical_p;
        if (params.repeat_penalty != paramsDefault.repeat_penalty) modelConfig[model]["repeat_penalty"] = params.repeat_penalty;
        if (params.frequency_penalty != paramsDefault.frequency_penalty) modelConfig[model]["frequency_penalty"] = params.frequency_penalty;
        if (params.presence_penalty != paramsDefault.presence_penalty) modelConfig[model]["presence_penalty"] = params.presence_penalty;
        if (params.mirostat != paramsDefault.mirostat) modelConfig[model]["mirostat"] = params.mirostat;
        if (params.mirostat_tau != paramsDefault.mirostat_tau) modelConfig[model]["mirostat_tau"] = params.mirostat_tau;
        if (params.mirostat_eta != paramsDefault.mirostat_eta) modelConfig[model]["mirostat_eta"] = params.mirostat_eta;
        if (params.cfg_scale != paramsDefault.cfg_scale) modelConfig[model]["cfg-scale"] = params.cfg_scale;
        if (params.n_ctx != paramsDefault.n_ctx) modelConfig[model]["ctx-size"] = params.n_ctx;
        if (params.n_threads != paramsDefault.n_threads) modelConfig[model]["n_threads"] = params.n_threads;
        if (params.n_gpu_layers != paramsDefault.n_gpu_layers) modelConfig[model]["n_gpu_layers"] = params.n_gpu_layers;
        
        #if GGML_OLD_FORMAT
        if (params.rms_norm_eps != paramsDefault.rms_norm_eps) modelConfig[model]["rms-norm-eps"] = params.rms_norm_eps;
        #endif
        
        if (params.rope_freq_base != paramsDefault.rope_freq_base) modelConfig[model]["rope_freq_base"] = params.rope_freq_base;
        if (params.rope_freq_scale != paramsDefault.rope_freq_scale) modelConfig[model]["rope_freq_scale"] = params.rope_freq_scale;
        
        modelConfig[model]["cfg-negative-prompt"] = params.cfg_negative_prompt;
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
    }
    
    void fillLocalJson(){
        modelConfig.clear();
        
        modelConfig["model"] = modelName;
        
        if (inputInstructFile != "NULL") //modelConfig[modelName]["file"] = inputInstructFile;
            processInstructFile(inputInstructFile, params);
        else if (instructFileFromJson != "NULL") //modelConfig[modelName]["file"] = instructFileFromJson;
            processInstructFile(instructFileFromJson, params);
        
        if (params.temp != paramsDefault.temp) modelConfig[modelName]["temp"] = params.temp;
        if (params.top_k != paramsDefault.top_k) modelConfig[modelName]["top_k"] = params.top_k;
        if (params.top_p != paramsDefault.top_p) modelConfig[modelName]["top_p"] = params.top_p;
        if (params.tfs_z != paramsDefault.tfs_z) modelConfig[modelName]["tfs_z"] = params.tfs_z;
        if (params.typical_p != paramsDefault.typical_p) modelConfig[modelName]["typical_p"] = params.typical_p;
        if (params.repeat_penalty != paramsDefault.repeat_penalty) modelConfig[modelName]["repeat_penalty"] = params.repeat_penalty;
        if (params.frequency_penalty != paramsDefault.frequency_penalty) modelConfig[modelName]["frequency_penalty"] = params.frequency_penalty;
        if (params.presence_penalty != paramsDefault.presence_penalty) modelConfig[modelName]["presence_penalty"] = params.presence_penalty;
        if (params.mirostat != paramsDefault.mirostat) modelConfig[modelName]["mirostat"] = params.mirostat;
        if (params.mirostat_tau != paramsDefault.mirostat_tau) modelConfig[modelName]["mirostat_tau"] = params.mirostat_tau;
        if (params.mirostat_eta != paramsDefault.mirostat_eta) modelConfig[modelName]["mirostat_eta"] = params.mirostat_eta;
        if (params.cfg_scale != paramsDefault.cfg_scale) modelConfig[modelName]["cfg-scale"] = params.cfg_scale;
        if (params.n_ctx != paramsDefault.n_ctx) modelConfig[modelName]["ctx-size"] = params.n_ctx;
        if (params.n_threads != paramsDefault.n_threads) modelConfig[modelName]["n_threads"] = params.n_threads;
        if (params.n_gpu_layers != paramsDefault.n_gpu_layers) modelConfig[modelName]["n_gpu_layers"] = params.n_gpu_layers;
        
        #if GGML_OLD_FORMAT
        if (params.rms_norm_eps != paramsDefault.rms_norm_eps) modelConfig[modelName]["rms-norm-eps"] = params.rms_norm_eps;
        #endif
        
        if (params.rope_freq_base != paramsDefault.rope_freq_base) modelConfig[modelName]["rope_freq_base"] = params.rope_freq_base;
        if (params.rope_freq_scale != paramsDefault.rope_freq_scale) modelConfig[modelName]["rope_freq_scale"] = params.rope_freq_scale;

        modelConfig[modelName]["cfg-negative-prompt"] = params.cfg_negative_prompt;
        if (params.prompt.size()) modelConfig[modelName]["prompt"] = params.prompt;
        else {
            modelConfig[modelName]["prompt"] = "### Instruction:";
            params.prompt = "### Instruction:";
        }
        if(params.antiprompt.size()) modelConfig[modelName]["reverse-prompt"] = params.antiprompt[0];
        else {
            modelConfig[modelName]["reverse-prompt"] = "### Instruction:";
            params.antiprompt.push_back("### Instruction:");
        }
    }
    
    void updateInput(){
        if (inputPrompt != "NULL") params.prompt = inputPrompt;
        if (inputAntiprompt != "NULL") {
            if(!params.antiprompt.size()) 
                params.antiprompt.push_back(inputAntiprompt);
            else
                params.antiprompt[0] = inputAntiprompt;
        }
        if (inputAntiCFG != "NULL") params.cfg_negative_prompt = inputAntiCFG;
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