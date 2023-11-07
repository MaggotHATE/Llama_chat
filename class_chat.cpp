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

struct wildcard{
    nlohmann::json wildcardsDB;
    std::string subname;
    std::string saveFolder = "R";
    int cycles = 10;
    unsigned int seed = 0;
    std::vector<std::string> promptsDB;
    std::string prompt0;
    std::string prompt;
    
    void init(std::string filename){
        seed = getRand();
        
        if (filename.empty()) wildcardsDB = getJson("wildcards.json");
        else {
            wildcardsDB = getJson(filename);
        }
        
        if (wildcardsDB.contains("cycles")) cycles = wildcardsDB["cycles"];
                    
        if (wildcardsDB.contains("folder")) saveFolder = wildcardsDB["folder"];
        
        
        
        if (wildcardsDB.contains("prompts")) promptsDB = wildcardsDB["prompts"];
        else {
            promptsDB.push_back(filename); // assuming that we passed the prompt itself instead of filename;
            std::string cNum;
            std::cout << " Enter the number of cycles: " << std::endl;
            std::getline(std::cin, cNum);
            cycles = std::stoi(cNum);
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
                            subname = choice;
                        }
                    }
                    
                    return findWildcard(inputString);
                    
                } else std::cout << " no wildcard: " << wildcard << std::endl;
            } else std::cout << " no first __" << std::endl;
        } else std::cout << " no last __" << std::endl;
            
        return inputString;
    }
};



int main(int argc, char ** argv) {
    
    std::setlocale(LC_CTYPE, ".UTF8");
    
    
    //std::setlocale(LC_ALL, "en_US.utf8");
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
    
    
    int latency = 30;
    if (settings.localConfig.contains("latency")) latency = settings.localConfig["latency"];
    
    // cycle for automated generation with wildcards
    std::string input1;
    bool cycling = 0;
    
    int cycles = 10;
    //nlohmann::json wildcardsDB;
    //std::string saveFolder = "R";
    
    wildcard Card;
    
    //std::srand(std::time(nullptr));
    //unsigned int cycleIDX = std::rand() * std::rand() / std::rand();
    
    //task_lambda(threadedChat);
    
    
    bool running = true;
      while (running) {
        //std::cout << "Loaded " << std::to_string(loaded) << std::endl;
        if (threadedChat.isContinue != 'i') {
            //if (threadedChat.isContinue == 'w') threadedChat.getResultAsyncString(true);
            //else 
                
            std::this_thread::sleep_for(std::chrono::milliseconds(latency));
        } else {
            
            
            
            if (Card.prompt.empty()){
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
                } else if (input == "save"){
                    threadedChat.writeTextFile();
                } else if (input == "cycle"){
                     std::cout << "Write a filename for .json or a prompt: " << std::endl; 
                     std::string input2;
                     std::getline(std::cin, input2);
                    
                     Card.init(input2);
                     Card.getPrompt();
                     //if (input2 == "y"){
                         threadedChat.appendQuestion(Card.prompt);
                         threadedChat.display();
                         threadedChat.startGen();
                         threadedChat.getResultAsyncStringFull2(true, true);
                         cycling = 1;
                         --Card.cycles;
                     //}
                } else {
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
            } else if (Card.cycles >= 0) {
                    
                if(cycling == 1){
                    threadedChat.writeTextFile(Card.saveFolder + '/', std::to_string(Card.seed) + "-" + std::to_string(Card.cycles) + "-" + Card.subname);
                    if (Card.cycles == 0) {
                        return 0;
                    } else {
                        threadedChat.unload();
                        threadedChat.load(settings.modelConfig, false);
                        cycling = 0;
                    }
                } else if (threadedChat.isContinue == 'i'){
                    //if (cycles <= 0) running = false;
                    --Card.cycles;
                    //auto result = findWildcard(wildcardsDB, input1);
                    Card.getPrompt();
                    
                    threadedChat.appendQuestion(Card.prompt);
                    threadedChat.display();
                    threadedChat.startGen();
                    threadedChat.getResultAsyncStringFull2(true, true);
                    cycling = 1;
                }
            }
        }
        
        
      }
    
    return 0;

}
