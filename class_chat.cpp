#include <thread_chat.h>

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

int main(int argc, char ** argv) {
    
    //std::setlocale(LC_CTYPE, ".UTF8");
    
    
    std::setlocale(LC_ALL, "en_US.utf8");
    SetConsoleOutputCP(CP_UTF8);
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
    
    int cycles = 10;
    int latency = 30;
    std::string input1;
    bool cycling = 0;
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
            
            if (cycles <= 0) running = false;
            
            if (input1.empty()){
                std::getline(std::cin, input);
                
                if (input == "timings"){
                    threadedChat.newChat.print_timings();
                    std::this_thread::sleep_for(std::chrono::milliseconds(5000));
                    
                    
                } else if (input == "restart"){
                    threadedChat.unload();
                    threadedChat.load(settings.modelConfig, false);
                } else if (input == "write"){
                    threadedChat.writeTextFile();
                } else if (input == "cycle"){
                     std::getline(std::cin, input1);
                     threadedChat.appendQuestion(input1);
                     threadedChat.display();
                     threadedChat.startGen();
                     threadedChat.getResultAsyncStringFull2(true, true);
                     cycling = 1;
                     --cycles;
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
            } else {
                
                if (cycles > 0) {
                    
                    if(cycling == 1){
                        threadedChat.writeTextFile("R");
                        if (cycles <= 0) running = false;
                        else {
                            threadedChat.unload();
                            threadedChat.load(settings.modelConfig, false);
                            cycling = 0;
                        }
                    }
                    
                    if (threadedChat.isContinue == 'i'){
                        if (cycles <= 0) running = false;
                        --cycles;
                        threadedChat.appendQuestion(input1);
                        threadedChat.display();
                        threadedChat.startGen();
                        threadedChat.getResultAsyncStringFull2(true, true);
                        cycling = 1;
                    }
                
                }
                
                if (cycles <= 0) running = false;
            }
        }
        
        
      }
    
    return 0;

}
