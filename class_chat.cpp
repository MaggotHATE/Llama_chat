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
    int cycles = -1;
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
            promptsDB.push_back(filename); // assuming that we passed the prompt itself instead of filename;
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
        
        if (wildcardsDB.contains("preset")) {
            preset = wildcardsDB["preset"];
            size_t slash = preset.rfind('/');
            if (slash != preset.npos) subname = preset.substr(slash+1);
            else subname = preset;
        }
        
        if (wildcardsDB.contains("prompts")) promptsDB = wildcardsDB["prompts"];
        else {
            std::string input2;
            std::getline(std::cin, input2);
            
            promptsDB.push_back(input2);
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

struct presetTest{
    nlohmann::json testParams;
    std::string subname;
    std::string saveFolder = "R";
    unsigned int seed = 0;
    int cycle = 0;
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
    
};

int main(int argc, char ** argv) {
    
    wildcard Card;
    presetTest Test;
    std::string inputPrompt;
    bool busy = false;
    
    
    
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
    
    if (argc > 1){
        std::string filename = argv[1];
        if (filename.rfind(".json") != filename.npos){
            auto instantJson = getJson(filename);
            if (instantJson.contains("presets")){
                Test.init(instantJson);
                busy = true;
            } else if (instantJson.contains("cycles")){
                Card.init(instantJson);
                Card.getPrompt();
                if (!Card.preset.empty()) settings.modelConfig["card"] = Card.preset;
                busy = true;
            } 
        } else if (filename.rfind(".txt") != filename.npos) {
            std::cout << "Opening text file " << filename << std::endl;
            inputPrompt = getText(filename);
            busy = true;
        }
    }
    
    std::cout << settings.modelConfig.dump(3) << std::endl; 
    
    modelThread threadedChat;
    
    threadedChat.jsonConfig = settings.modelConfig;
    threadedChat.load();
    
    
    int latency = 30;
    if (settings.localConfig.contains("latency")) latency = settings.localConfig["latency"];
    
    // cycle for automated generation with wildcards
    std::string input1;
    bool cycling = 0;
    
    
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
                if (!busy){
                    std::getline(std::cin, input);
                } else {
                    if (Card.cycles >= 0) {
                        std::cout << "Card cycles " << std::to_string(Card.cycles) << std::endl;
                        if(cycling == 1){
                            threadedChat.writeTextFileFull(Card.saveFolder + '/', std::to_string(Card.seed) + "-" + Card.subname + "-" + std::to_string(Card.cycles));
                            if (Card.cycles == 0) {
                                return 0;
                            } else {
                                cycling = 0;
                                input = "restart";
                                settings.modelConfig["card"] = Card.preset;
                                //threadedChat.unload();
                                //threadedChat.load(settings.modelConfig, false);
                            }
                        } else {
                            --Card.cycles;
                            Card.getPrompt();
                            input = Card.prompt;
                            cycling = 1;
                        }
                    } else if (Test.cycle < Test.presetsNames.size()) {
                        std::cout << "Test cycle " << std::to_string(Test.cycle) << std::endl;
                        if(cycling == 1){
                            std::string name = Test.presetsNames[Test.cycle];
                            
                            size_t slash = Test.presetsNames[Test.cycle].rfind('/');
                            if (slash != Test.presetsNames[Test.cycle].npos) name = Test.presetsNames[Test.cycle].substr(slash+1);
                            
                            name = std::to_string(Test.seed) + "-" + name;
                            std::cout << "Writing into " << name << std::endl;
                            
                            threadedChat.writeTextFileFull(Test.saveFolder + '/', name);
                            Test.cycle++;
                            
                            if (Test.cycle == Test.presetsNames.size()) {
                                return 0;
                            } else {
                                cycling = 0;
                                input = "restart";
                                //threadedChat.unload();
                                settings.modelConfig["card"] = Test.presetsNames[Test.cycle];
                                settings.modelConfig["seed"] = Test.seed;
                                //threadedChat.load(settings.modelConfig, false);
                            }
                        } else {
                            input = Test.prompt;
                            cycling = 1;
                        }
                    } else if (!inputPrompt.empty()) {
                        input = inputPrompt;
                        inputPrompt.clear();
                        busy = false;
                    }
                }
                
                if (input == "restart"){
                    threadedChat.unload();
                    threadedChat.load(settings.modelConfig, false);
                } else if (input == "save"){
                    threadedChat.writeTextFile();
                } else if (input == "cycle"){
                     std::cout << "Write a filename for .json or a prompt: " << std::endl; 
                     std::string input2;
                     std::getline(std::cin, input2);
                    
                     Card.init(input2);
                     Card.getPrompt();
                     
                     if (Card.preset.empty()){
                         threadedChat.appendQuestion(Card.prompt);
                         threadedChat.display();
                         threadedChat.startGen();
                         threadedChat.getResultAsyncStringFull2(true, true);
                         cycling = 1;
                         --Card.cycles;
                         
                     } else {
                         threadedChat.unload();
                         settings.modelConfig["card"] = Card.preset;
                         threadedChat.load(settings.modelConfig, false);
                     }
                     
                     busy = true;
                } else if (input == "test"){
                     std::cout << "Write a filename for test .json or a prompt, or skip to use default presetsTest.json: " << std::endl; 
                     std::string input2;
                     std::getline(std::cin, input2);
                    
                     Test.init(input2);
                     threadedChat.unload();
                     settings.modelConfig["card"] = Test.presetsNames[Test.cycle];
                     settings.modelConfig["seed"] = Test.seed;
                     threadedChat.load(settings.modelConfig, false);
                     busy = true;
                } else {
                    threadedChat.appendQuestion(input);
                    threadedChat.display();
                    threadedChat.startGen();
                    threadedChat.getResultAsyncStringFull2(true, true);

                    
                    
                }
        }
        
        
      }
    
    return 0;

}
