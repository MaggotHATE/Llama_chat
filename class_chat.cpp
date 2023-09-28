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

int main(int argc, char ** argv) {
    SetConsoleOutputCP(CP_UTF8);
    
    //std::setlocale(LC_ALL, "en_US.utf8");
    //chat newChat;
    //std::vector<std::string> results;
    //task_lambda();
    configurableChat settings;
    
    settings.localConfig = getJson("config.json");
    settings.getSettingsFull();
    
    settings.fillLocalJson(settings.params.model);
    
    std::cout << settings.modelConfig.dump(3) << std::endl; 
    
    modelThread threadedChat;
    
    // trying to load and generate in background threads
    //allTest(newChat, argc, argv);
    //threadedChat.newChat.getArgs(argc, argv); // avoiding segfault whed calling --help
    //load(threadedChat);
    threadedChat.jsonConfig = settings.modelConfig;
    threadedChat.load();
    
    int latency = 30;
    if (settings.localConfig.contains("latency")) latency = settings.localConfig["latency"];
    //task_lambda(threadedChat);
    
    
    bool running = true;
      while (running) {
        //std::cout << "Loaded " << std::to_string(loaded) << std::endl;
        if (threadedChat.isContinue != 'i') {
            //if (threadedChat.isContinue == 'w') threadedChat.getResultAsyncString(true);
            //else 
            std::this_thread::sleep_for(std::chrono::milliseconds(latency));
        } else {
            
            threadedChat.display();
            //if (threadedChat.newChat.params.input_prefix.empty()) std::cout << threadedChat.newChat.params.antiprompt[0];
            if (threadedChat.newChat.params.antiprompt.size() && 
                threadedChat.newChat.params.antiprompt[0] != threadedChat.newChat.params.input_prefix) 
                 std::cout << threadedChat.newChat.params.antiprompt[0] << threadedChat.newChat.params.input_prefix;
            else std::cout << threadedChat.newChat.params.input_prefix;
            
            //std::cout << threadedChat.newChat.params.input_prefix << ">";
            
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
                threadedChat.getResultAsyncStringFull2(true, true);
                
                 // char input[2048];
                // fflush(stdout);
                // fgets(input, 2048, stdin);

                
                // threadedChat.getResultAsyncCombined(input); 
                
                
            }
        }
        
        
      }
    
    return 0;

}
