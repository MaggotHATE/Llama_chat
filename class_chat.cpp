#include <thread_chat.h>
#include "include/units.hpp"

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
    std::string test_char_descr = "";
    wildcardGen Card;
    presetTest Test;
    std::string inputPrompt;
    std::string filename;
    bool busy = false;
    int regens = 0;
    bool regen = false;
    
    std::setlocale(LC_CTYPE, ".UTF8");
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
    //SetConsoleOutputCP(1251);
    //SetConsoleCP(1251);
    
    std::cout << argv[0] << std::endl;
    
    auto configName = getFileWithSameName(argv[0], ".json");
    
    if (std::filesystem::exists(configName)){
        std::cout << "Loading " << configName << std::endl;
    } else {
        std::cout << "Can't find " << configName << ", loading default config.json" << std::endl;
        configName = "config.json";
    }
    
    if (argc > 1){
        filename = argv[1];
    }
    
    //std::setlocale(LC_ALL, "en_US.utf8");
    
    //chat newChat;
    //std::vector<std::string> results;
    //task_lambda();
    
    
    configurableChat settings;
    SetConsoleTitle("Loading json...");
    settings.localConfig = getJson(configName);
    
    if (filename.rfind(".gguf") != filename.npos) {
        std::cout << "Opening a model " << filename << std::endl;
        SetConsoleTitle("Opening a model...");
        settings.localConfig["model"] = filename;
    }
    
    SetConsoleTitle("Getting settings for the model...");
    settings.getSettingsFull();
    settings.fillLocalJson(settings.params.model);
    modelThread threadedChat;
    
    
    if (filename.rfind(".json") != filename.npos){
        SetConsoleTitle("Loading a json file...");
        auto instantJson = getJson(filename);
        if (instantJson.contains("presets")){
            //Test.init(instantJson);
            //settings.modelConfig["card"] = Test.presetsNames[0];
            //settings.modelConfig["seed"] = Test.seed;
            //threadedChat.externalData = Test.presetsNames[0];
            //busy = true;
            Test.startTest(instantJson, settings, threadedChat);
        } else if (instantJson.contains("cycles")){
            // Card.init(instantJson);
            // Card.getPrompt();
            // if (!Card.preset.empty()) settings.modelConfig["card"] = Card.preset;
            // threadedChat.externalData = Card.preset + "\nCycles left: " + std::to_string(Card.cycles);
            // busy = true;
            Card.cycle(instantJson, settings, threadedChat);
        } else {
            settings.readJsonToParams(instantJson);
        }
    } else if (filename.rfind(".txt") != filename.npos) {
        SetConsoleTitle("Loading a text file...");
        std::cout << "Opening text file " << filename << std::endl;
        inputPrompt = getText(filename);
        busy = true;
    }
    
    std::cout << settings.modelConfig.dump(3) << std::endl; 
    
    
    SetConsoleTitle("Loading...");
    threadedChat.jsonConfig = settings.modelConfig;
    threadedChat.load();
    
    
    int latency = 30;
    if (settings.localConfig.contains("latency")) latency = settings.localConfig["latency"];
    
    // cycle for automated generation with wildcards
    std::string input1;
    bool cycling = 0;
    
    SetConsoleTitle("ChatTest ");
    bool running = true;
      while (running) {
        //std::cout << "Loaded " << std::to_string(loaded) << std::endl;
        if (threadedChat.isContinue != 'i') {
            //if (threadedChat.isContinue == 'w') threadedChat.getResultAsyncString(true);
            //else 
            std::this_thread::sleep_for(std::chrono::milliseconds(latency));
            if (threadedChat.isContinue == 'w') {
                SetConsoleTitle(threadedChat.display().c_str());
            }
            //if (threadedChat.loaded == 9) threadedChat.display();
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
                    if (filename.rfind("regens.txt") != filename.npos) {
                        input = "regens";
                    } else 
                        std::getline(std::cin, input);
                } else {
                    if (regens > 0) {
                        threadedChat.writeTextFile("tales/",std::to_string(regens));
                        --regens;
                        threadedChat.externalData = "Cycles left: " + std::to_string(regens);
                        input = "regen";
                        if (regens == 1) {
                            busy = false;
                        }
                    } else if (Card.cycles >= 0) {
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
                            threadedChat.externalData = Card.preset + "\nCycles left: " + std::to_string(Card.cycles);
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
                                threadedChat.externalData = Test.presetsNames[Test.cycle] + "(" + std::to_string(Test.cycle + 1) + "/" + std::to_string(Test.presetsNames.size()) + ")";
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
                
                if (input == "restart") {
                    threadedChat.unload();
                    threadedChat.load(settings.modelConfig, false);
                } else if (input == "regen") {
                    threadedChat.clear_last();
                    threadedChat.display();
                    threadedChat.startGen();
                    //threadedChat.getResultAsyncStringFull2(true, false);
                    threadedChat.getResultAsyncStringFull3();
                } else if (input == "regens") {
                    regens = 20;
                    threadedChat.externalData = "Cycles left: " + std::to_string(regens);
                    busy = true;
                } else if (input == "units") {
                    std::cout << test_char_descr << std::endl; 
                    std::string input2;
                    std::getline(std::cin, input2);
                    input2 = test_char_descr + "\n" + input2;
                    threadedChat.appendQuestion(input2);
                    threadedChat.display();
                    threadedChat.startGen();
                    threadedChat.getResultAsyncStringFull2(true, true);
                } else if (input == "save") {
                    threadedChat.writeTextFile();
                } else if (input == "cycle") {
                     threadedChat.unload();
                     std::cout << "Write a filename for .json or a prompt: " << std::endl; 
                     std::string input2;
                     std::getline(std::cin, input2);
                     auto wildcardsDB = getJson(input2);
                     
                     if (wildcardsDB.contains("prompts")) Card.cycle(wildcardsDB, settings, threadedChat);
                     else {
                        Card.promptsDB.push_back(input2); 
                        auto wildcardsDB = getJson("wildcards.json");
                        Card.cycle(wildcardsDB, settings, threadedChat);
                     }
                     
                     //Card.init(input2);
                     //Card.getPrompt();
                     
                     // if (Card.preset.empty()){
                         // threadedChat.appendQuestion(Card.prompt);
                         // threadedChat.display();
                         // threadedChat.startGen();
                         // threadedChat.getResultAsyncStringFull2(true, true);
                         // cycling = 1;
                         // --Card.cycles;
                         
                     // } else {
                         // threadedChat.unload();
                         // settings.modelConfig["card"] = Card.preset;
                         // threadedChat.load(settings.modelConfig, false);
                     // }
                     
                     busy = true;
                } else if (input == "unicycle") {
                     threadedChat.unload();
                     auto wildcardsDB = getJson(test_char_descr);
                     
                     if (wildcardsDB.contains("prompts")) Card.cycle(wildcardsDB, settings, threadedChat);
                     else {
                        Card.promptsDB.push_back(test_char_descr); 
                        auto wildcardsDB = getJson("wildcards.json");
                        Card.cycle(wildcardsDB, settings, threadedChat);
                     }
                     
                     busy = true;
                } else if (input == "test") {
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
                    //threadedChat.getResultAsyncStringFull2(true, true);
                    threadedChat.getResultAsyncStringFull3();

                    
                    
                }
        }
        
        
      }
    
    return 0;

}
