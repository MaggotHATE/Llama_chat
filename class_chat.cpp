#include "thread_chat.h"

#define SESSIONS_FOLDER "sessions/"

// #ifdef GGML_OLD_FORMAT
// #   include "GGML/thread_chat.h"
// #else
// #   include "thread_chat.h"  
// #endif

bool initial = false;
char waitGetline = 'i';
std::future<void> futureInput;
std::future<int> totalResult;
int loaded = 0;

void append_utf8(char32_t ch, std::string & out) {
    if (ch <= 0x7F) {
        out.push_back(static_cast<unsigned char>(ch));
    } else if (ch <= 0x7FF) {
        out.push_back(static_cast<unsigned char>(0xC0 | ((ch >> 6) & 0x1F)));
        out.push_back(static_cast<unsigned char>(0x80 | (ch & 0x3F)));
    } else if (ch <= 0xFFFF) {
        out.push_back(static_cast<unsigned char>(0xE0 | ((ch >> 12) & 0x0F)));
        out.push_back(static_cast<unsigned char>(0x80 | ((ch >> 6) & 0x3F)));
        out.push_back(static_cast<unsigned char>(0x80 | (ch & 0x3F)));
    } else if (ch <= 0x10FFFF) {
        out.push_back(static_cast<unsigned char>(0xF0 | ((ch >> 18) & 0x07)));
        out.push_back(static_cast<unsigned char>(0x80 | ((ch >> 12) & 0x3F)));
        out.push_back(static_cast<unsigned char>(0x80 | ((ch >> 6) & 0x3F)));
        out.push_back(static_cast<unsigned char>(0x80 | (ch & 0x3F)));
    } else {
        // Invalid Unicode code point
    }
}

struct configurables{
    gpt_params params;
    gpt_params paramsDefault;
    
    nlohmann::json localConfig;
    nlohmann::json modelConfig;
    std::string instructFileFromJson = "NULL";
    std::string inputPrompt = "NULL";
    std::string inputAntiprompt = "NULL";
    std::string inputAntiCFG = "NULL";
    std::string inputInstructFile = "NULL";
    std::string modelName = "NULL";
    
    
    void getSettings(chat& aChat){
        params = aChat.params;
    }
    
    void getSettings(){
        readParamsFromJson(localConfig, params);
        
        // if (localConfig.contains("model")){ // avoiding empty item created during checks
            // if (localConfig["model"].is_string()) modelName = localConfig["model"].get<std::string>();
            
            // if (localConfig.contains(modelName)){ // avoiding empty object created during checks
                // if (localConfig[modelName].is_object()){
                    // if (localConfig[modelName].contains("file")){ // avoiding empty item created during checks
                        // if (localConfig[modelName]["file"].is_string()) 
                            // instructFileFromJson = localConfig[modelName]["file"].get<std::string>();
                    // }
                // }
            // }
        // }
    }
    
    void getSettings(std::string newModelName){
        params = paramsDefault;
        readParamsFromJson(localConfig, newModelName, params);
        modelName = newModelName;
    }
    
    void updateSettings(){
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
        
        modelConfig[model]["temp"] = params.temp;
        modelConfig[model]["n_gpu_layers"] = params.n_gpu_layers;
        modelConfig[model]["top_k"] = params.top_k;
        modelConfig[model]["top_p"] = params.top_p;
        modelConfig[model]["tfs_z"] = params.tfs_z;
        modelConfig[model]["typical_p"] = params.typical_p;
        modelConfig[model]["repeat_penalty"] = params.repeat_penalty;
        modelConfig[model]["frequency_penalty"] = params.frequency_penalty;
        modelConfig[model]["presence_penalty"] = params.presence_penalty;
        modelConfig[model]["mirostat"] = params.mirostat;
        modelConfig[model]["mirostat_tau"] = params.mirostat_tau;
        modelConfig[model]["mirostat_eta"] = params.mirostat_eta;
        modelConfig[model]["cfg-scale"] = params.cfg_scale;
        modelConfig[model]["ctx-size"] = params.n_ctx;
        modelConfig[model]["n_threads"] = params.n_threads;
        //modelConfig[model]["rms-norm-eps"] = params.rms_norm_eps;
        modelConfig[model]["rope_freq_base"] = params.rope_freq_base;
        modelConfig[model]["rope_freq_scale"] = params.rope_freq_scale;
        
        modelConfig[model]["cfg-negative-prompt"] = params.cfg_negative_prompt;
        modelConfig[model]["prompt"] = params.prompt;
        modelConfig[model]["grammar"] = params.grammar;
        if(params.antiprompt.size()) modelConfig[model]["reverse-prompt"] = params.antiprompt[0];
    }
};

int main(int argc, char ** argv) {
    //std::setlocale(LC_ALL, "en_US.utf8");
    //chat newChat;
    //std::vector<std::string> results;
    //task_lambda();
    configurableChat settings;
    
    settings.localConfig = getJson("config.json");
    settings.getSettings();
    
    settings.fillLocalJson(settings.params.model);
    
    std::cout << settings.modelConfig.dump(3) << std::endl; 
    
    modelThread threadedChat;
    
    // trying to load and generate in background threads
    //allTest(newChat, argc, argv);
    //threadedChat.newChat.getArgs(argc, argv); // avoiding segfault whed calling --help
    //load(threadedChat);
    threadedChat.jsonConfig = settings.modelConfig;
    threadedChat.load();
    //task_lambda(threadedChat);
    
    
    bool running = true;
      while (running) {
        //std::cout << "Loaded " << std::to_string(loaded) << std::endl;
        if (threadedChat.isContinue != 'i') {
            //if (threadedChat.isContinue == 'w') threadedChat.getResultAsyncString(true);
            //else 
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        } else {
            
            threadedChat.display();
            std::cout << ">";
            
            std::string input;
            std::getline(std::cin, input);
            if (input == "timings"){
                threadedChat.newChat.print_timings();
                std::this_thread::sleep_for(std::chrono::milliseconds(5000));
                
                
            } else if (input == "restart"){
                threadedChat.unload();
                threadedChat.load(settings.modelConfig, false);
            } else if (input == "write"){
                threadedChat.writeTextFile();
            }else {
                threadedChat.appendQuestion(input);
                threadedChat.display();
                //threadedChat.isContinue = 'w';
                //threadedChat.lastResult = "";
                //threadedChat.getResultAsyncString(true);
                threadedChat.startGen();
                threadedChat.getResultAsyncStringFull(true);
                
                 // char input[2048];
                // fflush(stdout);
                // fgets(input, 2048, stdin);

                
                // threadedChat.getResultAsyncCombined(input); 
                
                
            }
        }
        
        
      }
    
    return 0;

}
