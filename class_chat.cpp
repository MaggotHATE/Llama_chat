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

struct Lineup {
    bool run = false;
    
    char process(std::string& choice) {
        if (choice == "restart") {
            return 't';
        } else if (choice == "regen") {
            return 'r';
        } else if (choice == "regens") {
            run = true;
            return 'R';
        } else if (choice == "units") {
            return 'u';
        } else if (choice == "save") {
            return 's';
        } else if (choice == "cycle") {
            run = true;
            return 'c';
        } else if (choice == "unicycle") {
            run = true;
            return 'i';
        } else if (choice == "test") {
            run = true;
            return 'T';
        } else {
            return 'I';
        }

        return 0;
    }
    
};

int main(int argc, char ** argv) {
    std::string test_char_descr = "";
    wildcardGen Card;
    presetTest Test;
    std::string inputPrompt;
    std::string filename;
    //bool busy = false;
    int regens = 0;
    bool regen = false;
    
    
    nlohmann::json queue;
    queue["choices"] = {"stop", "input"};
    Lineup queue_logic;
    
    std::setlocale(LC_CTYPE, ".UTF8");
    //SetConsoleCP(CP_UTF8);
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
    //threadedChat.switch_debug();
    
    std::string window_title = "ChatTest";
   
    
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
        window_title = "ChatTest: TXT ";
        inputPrompt = getText(filename);
        //busy = true;
        queue_logic.run = true;
        if (filename.rfind("regens.txt") != filename.npos) {
            regens = 20;
            queue["choices"].push_back("regens");
            //queue["choices"].push_back("input1");
            //threadedChat.externalData = "Cycles left: " + std::to_string(regens);
        }
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
    
    // if (threadedChat.isContinue != 'i') {
        // SetConsoleTitle("Ready...");
        // std::cout << "Loading finished!" << filename << std::endl;
    // }
    
    // std::getline(std::cin, input1);
    
    SetConsoleTitle(window_title.c_str());
    
    bool running = true;
      while (running) {
        //std::cout << "Loaded " << std::to_string(loaded) << std::endl;
        if (threadedChat.isContinue != 'i') {
            //if (threadedChat.isContinue == 'w') threadedChat.getResultAsyncString(true);
            //else 
            std::this_thread::sleep_for(std::chrono::milliseconds(latency));
            if (threadedChat.isContinue == 'w') {
			SetConsoleTitle((window_title + " | " + std::to_string(regens) + " | " + threadedChat.display()).c_str());
            }
            //if (threadedChat.loaded == 9) threadedChat.display();
        } else {
            
            
            
            
                threadedChat.display();
                //if (threadedChat.newChat.params.input_prefix.empty()) std::cout << threadedChat.newChat.params.antiprompt[0];
                if (threadedChat.newChat.params.antiprompt.size() && 
                    threadedChat.newChat.params.antiprompt[0] != threadedChat.newChat.params.input_prefix) 
                     std::cout << threadedChat.newChat.params.antiprompt[0] << threadedChat.newChat.params.input_prefix;
                else std::cout << threadedChat.newChat.params.input_prefix;
                
                std::string input;
                
                // if (queue_logic.run == false){
                    // if (filename.rfind("regens.txt") != filename.npos) {
                        // window_title = "ChatTest: Regens TXT ";
                        // input = "regens";
                    // } else 
                        // std::getline(std::cin, input);
                // } else {
                    // if (regens > 0) {
                        // threadedChat.writeTextFile("tales/",std::to_string(regens));
                        // --regens;
                        // threadedChat.externalData = "Cycles left: " + std::to_string(regens);
                        // input = "regen";
                        // if (regens == 1) {
                            // queue_logic.run = false;
                        // }
                    // } else if (!inputPrompt.empty()) {
                        // input = inputPrompt;
                        // inputPrompt.clear();
                        // queue_logic.run = false;
                    // }
                // }
                if (queue["choices"].back() == "regens") {
                    if (regens >= 1) {
                        if (regens < 20) {
                            threadedChat.writeTextFileUnified("tales/",std::to_string(regens), regens == 19);
                            input = "regen";
                            threadedChat.externalData = "Cycles left: " + std::to_string(regens);
                        }
                        --regens;
                        threadedChat.externalData = "Cycles left: " + std::to_string(regens);
                    } else queue["choices"].erase("regens");
                } else if (queue["choices"].back() != "input") input = queue["choices"].back();
                else std::getline(std::cin, input);
                
                
                
                char choice = queue_logic.process(input);
                
                switch (choice) {
                    case 't': {
                        threadedChat.unload();
                        threadedChat.load(settings.modelConfig, false);
                        break;
                    }
                    case 'r': {
                        window_title = "ChatTest: Regen ";
                        threadedChat.clear_last();
                        threadedChat.display();
                        threadedChat.startGen();
                        //threadedChat.getResultAsyncStringFull2(true, false);
                        threadedChat.getResultAsyncStringFull3();
                        break;
                    }
                    case 'R': {
                        threadedChat.writeTextFileDivided("tales/",std::to_string(regens));
                        window_title = "ChatTest: Regens ";
                        regens = 19;
                        queue["choices"].push_back("regens");
                        threadedChat.externalData = "Cycles left: " + std::to_string(regens);
                        break;
                    }
                    case 'u': {
                        window_title = "ChatTest: Units ";
                        std::cout << test_char_descr << std::endl; 
                        std::string input2;
                        std::getline(std::cin, input2);
                        input2 = test_char_descr + "\n" + input2;
                        threadedChat.appendQuestion(input2);
                        threadedChat.display();
                        threadedChat.startGen();
                        threadedChat.getResultAsyncStringFull2(true, true);
                        break;
                    }
                    case 's': {
                        threadedChat.writeTextFile();
                        break;
                    }
                    case 'c': {
                         window_title = "ChatTest: Cycle ";
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
                        break;
                    }
                    case 'i': {
                         window_title = "ChatTest: Unicycle ";
                         threadedChat.unload();
                         auto wildcardsDB = getJson(test_char_descr);
                         
                         if (wildcardsDB.contains("prompts")) Card.cycle(wildcardsDB, settings, threadedChat);
                         else {
                            Card.promptsDB.push_back(test_char_descr); 
                            auto wildcardsDB = getJson("wildcards.json");
                            Card.cycle(wildcardsDB, settings, threadedChat);
                         }
                        break;
                    }
                    case 'T': {
                         window_title = "ChatTest: Test ";
                         std::cout << "Write a filename for test .json or a prompt, or skip to use default presetsTest.json: " << std::endl; 
                         std::string input2;
                         std::getline(std::cin, input2);
                        
                         Test.init(input2);
                         threadedChat.unload();
                         settings.modelConfig["card"] = Test.presetsNames[Test.cycle];
                         settings.modelConfig["seed"] = Test.seed;
                         threadedChat.load(settings.modelConfig, false);
                        break;
                    }
                    case 'I': {
                        window_title = "ChatTest: Input ";
                        if (!input.empty()) inputPrompt = input;
                        remove_last_nl(inputPrompt);
                        threadedChat.appendQuestion(inputPrompt);
                        threadedChat.display();
                        threadedChat.startGen();
                        //threadedChat.getResultAsyncStringFull2(true, true);
                        threadedChat.getResultAsyncStringFull3();
                        break;
                    }
                    default: break;
                }

        }
        
        
      }
    
    return 0;

}
