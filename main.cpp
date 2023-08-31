// Dear ImGui: standalone example application for SDL2 + OpenGL
// (SDL is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"
#include "misc/cpp/imgui_stdlib.h"
#include "tinyfiledialogs/tinyfiledialogs.h"
#include <stdio.h>
#include <SDL.h>

#include "thread_chat.h"

#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <SDL_opengles2.h>
#else
#include <SDL_opengl.h>
#endif

#if defined(GGML_USE_CLBLAST)
#   define clblast 1
#else 
#   define clblast 0
#endif

// This example can also compile and run with Emscripten! See 'Makefile.emscripten' for details.
#ifdef __EMSCRIPTEN__
#include "../libs/emscripten/emscripten_mainloop_stub.h"
#endif

#define U8(_S)    (const char*)u8##_S

// Helper from the main demo to display a little (?) mark which shows a tooltip when hovered.
// In your own code you may want to display an actual icon if you are using a merged icon fonts (see docs/FONTS.md)
static void HelpMarker(const char* desc)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::BeginItemTooltip())
    {
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

bool initial = false;
char waitGetline = 'i';
std::future<void> futureInput;
std::future<int> totalResult;
int loaded = 0;

void getInput(std::string& input) {
    //std::cout << "getInput create " << std::endl;
    futureInput = std::async(std::launch::async, [&input] { 
        std::getline(std::cin, input); 
        waitGetline = 'x';
        //std::cout << "_getline status: " << waitGetline << "_ ";
    });
    
}

int catchInput() {
    //std::cout << ".";
    
    if (waitGetline == 'x' && futureInput.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
        return 1;
    }
    
    return 0;
}

void timerGetline(){
    while(waitGetline){ 
        //std::cout << waitGetline << std::endl;
        
        if(waitGetline == 'w' || waitGetline == 'x'){ 
            if (waitGetline == 'x' && catchInput() == 1) waitGetline = 'r';
            //std::cout << "timerGetline result: " << input << std::endl;
        }
        
        //std::cout << waitGetline;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    }
    
    //std::cout << "timerGetline finished " << waitGetline << std::endl;
    
}

void startTimer(){
    //std::cout << "startTimer " << waitGetline << std::endl;
    
    std::thread timerGet([](){
        timerGetline();
        }

    );
    
    timerGet.detach();
    
}

void load(modelThread& model) {
    //std::cout << "getInput create " << std::endl;
    totalResult = std::async(std::launch::async, [&model, &loaded] { 
        //loaded = model.newChat.init();
        loaded = model.newChat.initGenerate(false);
        //model.appendQuestion(" ");

        std::cout << "Loaded, you can type now! " << std::endl;
        model.isContinue = 'i';
        
        return loaded;
    });
    
}

void sanitizePath(std::string& path){
    int slashes = path.rfind("\\");
    while (slashes != path.npos){
        path.replace(slashes,1,"/");
        slashes = path.rfind("\\");
    }
}

void load(modelThread& model, nlohmann::json localConfig, bool soft) {
    
    totalResult = std::async(std::launch::async, [&model, &loaded, localConfig, soft] { 

        loaded = model.newChat.initGenerate(localConfig, false, soft);

        model.appendFirstPrompt();
        model.getTimigsSimple();

        std::cout << "Loaded, you can type now! " << std::endl;
        model.isContinue = 'i';
        
        return loaded;
    });
    
}

void load_task(modelThread& model, configurableChat& localSettings, bool soft){
    
    std::packaged_task<int()> loader([&model, &loaded, &localSettings, &soft]() {
        loaded = model.newChat.initGenerate(localSettings.localConfig, false, soft);

        model.appendFirstPrompt();
        model.getTimigsSimple();

        std::cout << "Loaded, you can type now! " << std::endl;
        model.isContinue = 'i';
        
        return loaded;
    });
    
    totalResult = std::async(std::launch::async, [&loader] { 
            loader();
            return 1;
    });

}

std::string getStringFromJson(std::string fimeName, std::string stringName){
    nlohmann::json config;
    std::fstream o1(fimeName);
    if (o1.is_open()) {
        
        o1 >> std::setw(4) >> config;
        o1.close();
    } else {
        return "NULL";
    }
    
    if (config.contains(stringName)) {
        if(config[stringName].is_string()) return config[stringName].get<std::string>();
    }
    
    return "NULL";
}

void paramsPanel(gpt_params& params, int& totalThreads, ImVec2 size){
    ImGui::BeginChild("Params", size, false);
    
        //ImGui::SliderInt("n_threads", &localSettings.n_threads, 1, 4);
        ImGui::SliderFloat("temp", &params.temp, 0.0f, 2.0f); ImGui::SameLine(); HelpMarker("Adjust the randomness of the generated text. Lower means more robotic.");
        
        ImGui::SliderInt("top_k", &params.top_k, 0, 100); ImGui::SameLine(); HelpMarker("Top-k sampling. Selects the next token only from the top k most likely tokens predicted by the model. It helps reduce the risk of generating low-probability or nonsensical tokens, but it may also limit the diversity of the output.");
        ImGui::SliderFloat("top_p", &params.top_p, 0.5f, 1.0f); ImGui::SameLine(); HelpMarker("Top-p (Nucleus) sampling. Selects the next token from a subset of tokens that together have a cumulative probability of at least p. Higher values will lead to more diverse text, lower values will generate more focused and conservative text.");
        ImGui::SliderFloat("repeat_penalty", &params.repeat_penalty, 0.0f, 2.0f); ImGui::SameLine(); HelpMarker("Controls the repetition of token sequences in the generated text.");
        ImGui::SliderFloat("frequency_penalty", &params.frequency_penalty, 0.0f, 2.0f); ImGui::SameLine(); HelpMarker("Descreases the chance of repeating the same lines in the output. Affects the immediate result.");
        ImGui::SliderFloat("presence_penalty", &params.presence_penalty, 0.0f, 2.0f); ImGui::SameLine(); HelpMarker("Descreases the chance of repeating the same lines in the output. Affects the entrire dialog.");
        ImGui::SliderFloat("tfs_z", &params.tfs_z, 0.5f, 1.0f); ImGui::SameLine(); HelpMarker("Tail free sampling, parameter z. TFS filters out logits based on the second derivative of their probabilities. Adding tokens is stopped after the sum of the second derivatives reaches the parameter z. In short: TFS looks how quickly the probabilities of the tokens decrease and cuts off the tail of unlikely tokens using the parameter z.");
        ImGui::SliderFloat("typical_p", &params.typical_p, 0.5f, 1.0f); ImGui::SameLine(); HelpMarker("Locally typical sampling, parameter p. Promotes the generation of contextually coherent and diverse text by sampling tokens that are typical or expected based on the surrounding context");
        
        ImGui::SliderInt("mirostat", &params.mirostat, 0, 2); ImGui::SameLine(); HelpMarker("Uses Mirostat sampling. Top K, Nucleus (top_p), Tail Free (tfs_z) and Locally Typical (typical_p) samplers are ignored with this.");
        ImGui::SliderFloat("mirostat_tau", &params.mirostat_tau, 0.0f, 10.0f); ImGui::SameLine(); HelpMarker("Mirostat learning rate.");
        ImGui::SliderFloat("mirostat_eta", &params.mirostat_eta, 0.00f, 0.50f); ImGui::SameLine(); HelpMarker("Mirostat target entropy.");
        ImGui::SliderFloat("cfg_scale", &params.cfg_scale, 1.0f, 4.0f); ImGui::SameLine(); HelpMarker("How strong the cfg is. High values might result in no answer generated.");
        ImGui::SliderInt("n_threads", &params.n_threads, 0, totalThreads); ImGui::SameLine(); HelpMarker("Number of threads to use, recommended to leave at least one for the system.");
        // ImGui::SliderFloat("cfg_smooth_factor", &localSettings.cfg_smooth_factor, 0.0f, 2.0f); ImGui::SameLine(); HelpMarker("Desfines the mix between outpus with and without cfg.");

    ImGui::EndChild();
}

void paramsPanelNew(gpt_params& params, int& totalThreads, ImVec2 size){
    ImGui::BeginChild("Params", size, false);
    
        //ImGui::SliderInt("n_threads", &localSettings.n_threads, 1, 4);
        ImGui::SliderFloat("temp", &params.temp, 0.0f, 2.0f);
        if (ImGui::BeginPopupContextItem("temp"))
        {
            if (ImGui::Selectable("Reset to default")){
                params.temp = paramsDefault.temp;
            }
            ImGui::EndPopup();
        } ImGui::SameLine(); HelpMarker( ("Adjust the randomness of the generated text. Lower means more robotic. Default: " + std::to_string(paramsDefault.temp)).c_str());
        
        ImGui::SliderInt("top_k", &params.top_k, 0, 100);
        if (ImGui::BeginPopupContextItem("top_k"))
        {
            if (ImGui::Selectable("Reset to default")){
                params.top_k = paramsDefault.top_k;
            }
            ImGui::EndPopup();
        } ImGui::SameLine(); HelpMarker(("Top-k sampling. Selects the next token only from the top k most likely tokens predicted by the model. It helps reduce the risk of generating low-probability or nonsensical tokens, but it may also limit the diversity of the output. Default: " + std::to_string(paramsDefault.top_k)).c_str());
        
        ImGui::SliderFloat("top_p", &params.top_p, 0.5f, 1.0f);
        if (ImGui::BeginPopupContextItem("top_p"))
        {
            if (ImGui::Selectable("Reset to default")){
                params.top_p = paramsDefault.top_p;
            }
            ImGui::EndPopup();
        } ImGui::SameLine(); HelpMarker(("Top-p (Nucleus) sampling. Selects the next token from a subset of tokens that together have a cumulative probability of at least p. Higher values will lead to more diverse text, lower values will generate more focused and conservative text. Default: " + std::to_string(paramsDefault.top_p)).c_str());
        
        ImGui::SliderFloat("repeat_penalty", &params.repeat_penalty, 0.0f, 2.0f);
        if (ImGui::BeginPopupContextItem("repeat_penalty"))
        {
            if (ImGui::Selectable("Reset to default")){
                params.repeat_penalty = paramsDefault.repeat_penalty;
            }
            ImGui::EndPopup();
        } ImGui::SameLine(); HelpMarker(("Controls the repetition of token sequences in the generated text. Default: " + std::to_string(paramsDefault.repeat_penalty)).c_str());
        
        ImGui::SliderFloat("frequency_penalty", &params.frequency_penalty, 0.0f, 2.0f);
        if (ImGui::BeginPopupContextItem("frequency_penalty"))
        {
            if (ImGui::Selectable("Reset to default")){
                params.frequency_penalty = paramsDefault.frequency_penalty;
            }
            ImGui::EndPopup();
        } ImGui::SameLine(); HelpMarker(("Descreases the chance of repeating the same lines in the output. Affects the immediate result. Default: " + std::to_string(paramsDefault.frequency_penalty)).c_str());
        
        ImGui::SliderFloat("presence_penalty", &params.presence_penalty, 0.0f, 2.0f);
        if (ImGui::BeginPopupContextItem("presence_penalty"))
        {
            if (ImGui::Selectable("Reset to default")){
                params.presence_penalty = paramsDefault.presence_penalty;
            }
            ImGui::EndPopup();
        } ImGui::SameLine(); HelpMarker(("Descreases the chance of repeating the same lines in the output. Affects the entrire dialog. Default: " + std::to_string(paramsDefault.presence_penalty)).c_str());
        
        ImGui::SliderFloat("tfs_z", &params.tfs_z, 0.5f, 1.0f); ImGui::SameLine();
        if (ImGui::BeginPopupContextItem("tfs_z"))
        {
            if (ImGui::Selectable("Reset to default")){
                params.tfs_z = paramsDefault.tfs_z;
            }
            ImGui::EndPopup();
        } HelpMarker(("Tail free sampling, parameter z. TFS filters out logits based on the second derivative of their probabilities. Adding tokens is stopped after the sum of the second derivatives reaches the parameter z. In short: TFS looks how quickly the probabilities of the tokens decrease and cuts off the tail of unlikely tokens using the parameter z. Default: " + std::to_string(paramsDefault.tfs_z)).c_str());
        
        ImGui::SliderFloat("typical_p", &params.typical_p, 0.5f, 1.0f);
        if (ImGui::BeginPopupContextItem("typical_p"))
        {
            if (ImGui::Selectable("Reset to default")){
                params.typical_p = paramsDefault.typical_p;
            }
            ImGui::EndPopup();
        } ImGui::SameLine(); HelpMarker(("Locally typical sampling, parameter p. Promotes the generation of contextually coherent and diverse text by sampling tokens that are typical or expected based on the surrounding context. Default: " + std::to_string(paramsDefault.typical_p)).c_str());
        
        ImGui::SliderInt("mirostat", &params.mirostat, 0, 2); ImGui::SameLine();
        if (ImGui::BeginPopupContextItem("mirostat"))
        {
            if (ImGui::Selectable("Reset to default")){
                params.mirostat = paramsDefault.mirostat;
            }
            ImGui::EndPopup();
        } HelpMarker(("Uses Mirostat sampling. Top K, Nucleus, Tail Free and Locally Typical samplers are ignored with this. Default: " + std::to_string(paramsDefault.mirostat)).c_str());
        
         ImGui::SliderFloat("mirostat_tau", &params.mirostat_tau, 0.0f, 10.0f);
        if (ImGui::BeginPopupContextItem("mirostat_tau"))
        {
            if (ImGui::Selectable("Reset to default")){
                params.mirostat_tau = paramsDefault.mirostat_tau;
            }
            ImGui::EndPopup();
        } ImGui::SameLine(); HelpMarker(("Mirostat learning rate. Default: " + std::to_string(paramsDefault.mirostat_tau)).c_str());
        
        ImGui::SliderFloat("mirostat_eta", &params.mirostat_eta, 0.00f, 0.50f);
        if (ImGui::BeginPopupContextItem("mirostat_eta"))
        {
            if (ImGui::Selectable("Reset to default")){
                params.mirostat_eta = paramsDefault.mirostat_eta;
            }
            ImGui::EndPopup();
        } ImGui::SameLine(); HelpMarker(("Mirostat target entropy. Default: " + std::to_string(paramsDefault.mirostat_eta)).c_str());
        
        ImGui::SliderFloat("cfg_scale", &params.cfg_scale, 1.0f, 4.0f);
        if (ImGui::BeginPopupContextItem("cfg_scale"))
        {
            if (ImGui::Selectable("Reset to default")){
                params.cfg_scale = paramsDefault.cfg_scale;
            }
            ImGui::EndPopup();
        } ImGui::SameLine(); HelpMarker(("How strong the cfg is. High values might result in no answer generated. Default: " + std::to_string(paramsDefault.cfg_scale)).c_str());
        
        ImGui::SliderInt("n_threads", &params.n_threads, 0, totalThreads); ImGui::SameLine(); HelpMarker("Number of threads to use, recommended to leave at least one for the system.");
        // ImGui::SliderFloat("cfg_smooth_factor", &localSettings.cfg_smooth_factor, 0.0f, 2.0f); ImGui::SameLine(); HelpMarker("Desfines the mix between outpus with and without cfg.");

    ImGui::EndChild();
}

// this is probably the most degenerate part of imgui: not having clamps in input fields without a
// single reason, then giving a fake example of clamping that works through widgets that have clamps
void paramsPanelInputs(gpt_params& params, int& totalThreads, ImVec2 size){
    ImGui::BeginChild("Params", size, false);
    
        //ImGui::SliderInt("n_threads", &localSettings.n_threads, 1, 4);
        ImGui::InputFloat("temp", &params.temp, 0.001f, 0.01f, "%.3f");
        if (ImGui::BeginPopupContextItem("temp"))
        {
            if (ImGui::Selectable("Reset to default")){
                params.temp = paramsDefault.temp;
            }
            ImGui::EndPopup();
        } ImGui::SameLine(); HelpMarker( ("Adjust the randomness of the generated text. Lower means more robotic. Default: " + std::to_string(paramsDefault.temp)).c_str());
        
        ImGui::InputInt("top_k", &params.top_k, 1, 1);
        if (ImGui::BeginPopupContextItem("top_k"))
        {
            if (ImGui::Selectable("Reset to default")){
                params.top_k = paramsDefault.top_k;
            }
            ImGui::EndPopup();
        } ImGui::SameLine(); HelpMarker(("Top-k sampling. Selects the next token only from the top k most likely tokens predicted by the model. It helps reduce the risk of generating low-probability or nonsensical tokens, but it may also limit the diversity of the output. Default: " + std::to_string(paramsDefault.top_k)).c_str());
        
        ImGui::InputFloat("top_p", &params.top_p, 0.001f, 0.01f, "%.3f");
        if (ImGui::BeginPopupContextItem("top_p"))
        {
            if (ImGui::Selectable("Reset to default")){
                params.top_p = paramsDefault.top_p;
            }
            ImGui::EndPopup();
        } ImGui::SameLine(); HelpMarker(("Top-p (Nucleus) sampling. Selects the next token from a subset of tokens that together have a cumulative probability of at least p. Higher values will lead to more diverse text, lower values will generate more focused and conservative text. Default: " + std::to_string(paramsDefault.top_p)).c_str());
        
        ImGui::InputFloat("repeat_penalty", &params.repeat_penalty, 0.001f, 0.01f, "%.3f");
        if (ImGui::BeginPopupContextItem("repeat_penalty"))
        {
            if (ImGui::Selectable("Reset to default")){
                params.repeat_penalty = paramsDefault.repeat_penalty;
            }
            ImGui::EndPopup();
        } ImGui::SameLine(); HelpMarker(("Controls the repetition of token sequences in the generated text. Default: " + std::to_string(paramsDefault.repeat_penalty)).c_str());
        
        ImGui::InputFloat("frequency_penalty", &params.frequency_penalty, 0.001f, 0.01f, "%.3f");
        if (ImGui::BeginPopupContextItem("frequency_penalty"))
        {
            if (ImGui::Selectable("Reset to default")){
                params.frequency_penalty = paramsDefault.frequency_penalty;
            }
            ImGui::EndPopup();
        } ImGui::SameLine(); HelpMarker(("Descreases the chance of repeating the same lines in the output. Affects the immediate result. Default: " + std::to_string(paramsDefault.frequency_penalty)).c_str());
        
        ImGui::InputFloat("presence_penalty", &params.presence_penalty, 0.001f, 0.01f, "%.3f");
        if (ImGui::BeginPopupContextItem("presence_penalty"))
        {
            if (ImGui::Selectable("Reset to default")){
                params.presence_penalty = paramsDefault.presence_penalty;
            }
            ImGui::EndPopup();
        } ImGui::SameLine(); HelpMarker(("Descreases the chance of repeating the same lines in the output. Affects the entrire dialog. Default: " + std::to_string(paramsDefault.presence_penalty)).c_str());
        
        ImGui::InputFloat("tfs_z", &params.tfs_z, 0.001f, 0.01f, "%.3f"); ImGui::SameLine();
        if (ImGui::BeginPopupContextItem("tfs_z"))
        {
            if (ImGui::Selectable("Reset to default")){
                params.tfs_z = paramsDefault.tfs_z;
            }
            ImGui::EndPopup();
        } HelpMarker(("Tail free sampling, parameter z. TFS filters out logits based on the second derivative of their probabilities. Adding tokens is stopped after the sum of the second derivatives reaches the parameter z. In short: TFS looks how quickly the probabilities of the tokens decrease and cuts off the tail of unlikely tokens using the parameter z. Default: " + std::to_string(paramsDefault.tfs_z)).c_str());
        
        ImGui::InputFloat("typical_p", &params.typical_p, 0.001f, 0.01f, "%.3f");
        if (ImGui::BeginPopupContextItem("typical_p"))
        {
            if (ImGui::Selectable("Reset to default")){
                params.typical_p = paramsDefault.typical_p;
            }
            ImGui::EndPopup();
        } ImGui::SameLine(); HelpMarker(("Locally typical sampling, parameter p. Promotes the generation of contextually coherent and diverse text by sampling tokens that are typical or expected based on the surrounding context. Default: " + std::to_string(paramsDefault.typical_p)).c_str());
        
        ImGui::SliderInt("mirostat", &params.mirostat, 0, 2); ImGui::SameLine();
        if (ImGui::BeginPopupContextItem("mirostat"))
        {
            if (ImGui::Selectable("Reset to default")){
                params.mirostat = paramsDefault.mirostat;
            }
            ImGui::EndPopup();
        } HelpMarker(("Uses Mirostat sampling. Top K, Nucleus, Tail Free and Locally Typical samplers are ignored with this. Default: " + std::to_string(paramsDefault.mirostat)).c_str());
        
        ImGui::InputFloat("mirostat_tau", &params.mirostat_tau, 0.001f, 0.01f, "%.3f");
        if (ImGui::BeginPopupContextItem("mirostat_tau"))
        {
            if (ImGui::Selectable("Reset to default")){
                params.mirostat_tau = paramsDefault.mirostat_tau;
            }
            ImGui::EndPopup();
        } ImGui::SameLine(); HelpMarker(("Mirostat learning rate. Default: " + std::to_string(paramsDefault.mirostat_tau)).c_str());
        
        ImGui::InputFloat("mirostat_eta", &params.mirostat_eta, 0.001f, 0.01f, "%.3f");
        if (ImGui::BeginPopupContextItem("mirostat_eta"))
        {
            if (ImGui::Selectable("Reset to default")){
                params.mirostat_eta = paramsDefault.mirostat_eta;
            }
            ImGui::EndPopup();
        } ImGui::SameLine(); HelpMarker(("Mirostat target entropy. Default: " + std::to_string(paramsDefault.mirostat_eta)).c_str());
        
        ImGui::InputFloat("cfg_scale", &params.cfg_scale, 0.001f, 0.01f, "%.3f");
        if (ImGui::BeginPopupContextItem("cfg_scale"))
        {
            if (ImGui::Selectable("Reset to default")){
                params.cfg_scale = paramsDefault.cfg_scale;
            }
            ImGui::EndPopup();
        } ImGui::SameLine(); HelpMarker(("How strong the cfg is. High values might result in no answer generated. Default: " + std::to_string(paramsDefault.cfg_scale)).c_str());
        
        ImGui::SliderInt("n_threads", &params.n_threads, 0, totalThreads); ImGui::SameLine(); HelpMarker("Number of threads to use, recommended to leave at least one for the system.");
        // ImGui::SliderFloat("cfg_smooth_factor", &localSettings.cfg_smooth_factor, 0.0f, 2.0f); ImGui::SameLine(); HelpMarker("Desfines the mix between outpus with and without cfg.");

    ImGui::EndChild();
}

void templatesList(nlohmann::json& templatesJson, std::string& inputStr){
    ImGui::BeginChild("Templates from json file");
        int numTemplates = 0;
        for (auto& [key, value] : templatesJson.items() ){
            if (value.is_array()) {
                ImGui::SeparatorText(key.c_str());
                for (auto& element : value) {
                  
                  ImGui::TextWrapped(element.get<std::string>().c_str());
                  if (ImGui::BeginPopupContextItem(std::to_string(numTemplates).c_str()))
                  {
                    if (ImGui::Selectable("Use template")){
                        inputStr = element.get<std::string>();
                    }
                    
                    if (ImGui::Selectable("Copy")){
                        ImGui::LogToClipboard();
                        ImGui::LogText(element.get<std::string>().c_str());
                        ImGui::LogFinish();
                    }
                    
                    // if (ImGui::Selectable("Delete")){
                        // value.erase(numTemplates);
                    // }
                    
                    ImGui::EndPopup();
                  }
                  ++numTemplates;
                  ImGui::Separator();
                }
            }
        }
        
    ImGui::EndChild();
}

void templatesListSelect(nlohmann::json& templatesJson, std::string& inputStr){
    ImGui::BeginChild("Templates from json file");
        for (auto& [key, value] : templatesJson.items() ){
            if (value.is_array()) {
                ImGui::SeparatorText(key.c_str());
                for (auto& element : value) {
                  
                  HelpMarker(element.get<std::string>().c_str());
                  ImGui::SameLine();
                  if (ImGui::Selectable(element.get<std::string>().c_str()))
                      inputStr = element.get<std::string>();
                  // if (ImGui::BeginItemTooltip())
                    // {
                        // //ImGui::BeginChild("", ImVec2( 400.0f, 200.0f));
                            // ImGui::TextWrapped(element.get<std::string>().c_str());
                        // //ImGui::EndChild();
                        // ImGui::EndTooltip();
                    // }
                }
            }
        }
        
    ImGui::EndChild();
}

std::string openFile(const char* const* filterPatterns){
    char const* currPath = (filesystem::current_path().string() + '/').c_str();
    auto getFileName = tinyfd_openFileDialog("Select a file...", currPath,1, filterPatterns, NULL,0);
    if (getFileName) {
        std::string result = getFileName;
        
        
        
        return result;
    }
    return "NULL";
}

////////////////////////////////////////////////////////////////////////////////////////////////
// Main code ///////////////////////////////////////////////////////////////////////////////////
int main(int, char**)
{
    nlohmann::json configJson = getJson("chatConfig.json");
    int width = 600;
    int height = 800;
    float fontSize = 21.0f;
    std::string fontFile = "DroidSans.ttf";
    std::string modelsFolderName = "NULL";
    std::string promptsFolderName = "NULL";
    
    if (!configJson["error"].is_string()){
        if (configJson["width"].is_number()) width = configJson["width"].get<int>();
        if (configJson["height"].is_number()) height = configJson["height"].get<int>();
        if (configJson["font"].is_string()) fontFile = configJson["font"].get<std::string>();
        if (configJson["fontSize"].is_number()) fontSize = configJson["fontSize"].get<float>();
        if (configJson.contains("modelsFolder") && configJson["modelsFolder"].is_string()) modelsFolderName = configJson["modelsFolder"].get<std::string>();
        if (configJson.contains("promptsFolder") && configJson["promptsFolder"].is_string()) promptsFolderName = configJson["promptsFolder"].get<std::string>();
        //configJson.erase("error");
    }
    
    
    
    // Setup SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
        return -1;
    }

    // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100
    const char* glsl_version = "#version 100";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(__APPLE__)
    // GL 3.2 Core + GLSL 150
    const char* glsl_version = "#version 150";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

    // From 2.0.18: Enable native IME.
#ifdef SDL_HINT_IME_SHOW_UI
    SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif

    char* windowLable = "Llama.cpp chat (gguf format)";
    
    //if(clblast == 1) windowLable = "Llama.cpp chat (with CLBLAST)";
    
    #if GGML_OLD_FORMAT
    windowLable = "Llama.cpp chat (ggmlv3 format)";
    #elif defined(GGML_USE_CLBLAST)
    windowLable = "Llama.cpp chat (gguf format with CLBLAST)";
    #endif

    // Create window with graphics context
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Window* window = SDL_CreateWindow(windowLable, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, window_flags);
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    //ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return a nullptr. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    // - Our Emscripten build process allows embedding fonts to be accessible at runtime from the "fonts/" folder. See Makefile.emscripten for details.
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != nullptr);
    
    
    // Our state
    bool show_demo_window = false;
    bool show_another_window = true;
    bool my_tool_active = true;
    bool chatMode = true;
    bool scrolled = true;
    bool cancelled = false;
    bool copy = false;
    
    static bool use_work_area = true;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    ImFont* font = io.Fonts->AddFontFromFileTTF(fontFile.c_str(), fontSize, nullptr, io.Fonts->GetGlyphRangesCyrillic());
    // static ImWchar ranges[] = { 0x1, 0x1FFFF, 0 };
    // static ImFontConfig cfg;
    // cfg.OversampleH = cfg.OversampleV = 1;
    // cfg.MergeMode = true;
    ////cfg.FontBuilderFlags |= ImGuiFreeTypeBuilderFlags_LoadColor;
    // io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\seguiemj.ttf", 16.0f, &cfg, ranges);
    
    configurableChat localSettings;
    
    
    localSettings.modelFromJson = getStringFromJson("config.json","model");
    #if GGML_OLD_FORMAT
    char const *modelFilterPatterns[1]={"*.bin"};
    #else
    char const *modelFilterPatterns[2]={"*.gguf","*.bin"};
    #endif

    char const *instructFilterPatterns[1]={"*.txt"};
    char const *fontFilterPatterns[1]={"*.ttf"};
    char const* currPath = (filesystem::current_path().string() + '/').c_str();
    
    modelThread newChat;
    
    std::vector<std::pair<std::string, std::string>> localResultPairs;
    
    nlohmann::json templatesJson = getJson("templates.json");
    
    if (promptsFolderName != "NULL") localSettings.promptFilesFolder = promptsFolderName;
    
    localSettings.getFilesList();
    
    std::map<std::string, std::vector<std::string>> sessionHistory;
    
    
    bool copiedDialog = false;
    bool copiedSettings = false;
    bool initTokens = false;
    bool hasModel = false;
    int totalThreads = get_num_physical_cores();
    //newChat.isContinue = 'd';
    //static float wrap_width = 200.0f;
    //newChat.newChat.init();
    ImGui::StyleColorsClassic();
    
    
    localSettings.getFromJson("config.json");
    localSettings.getSettings();
    localSettings.fillLocalJson();
    int n_ctx_idx = sqrt( localSettings.params.n_ctx / 2048 );
    int tokens_this_session = 0;
    int consumed_this_session = 0;
    int past_this_session = 0;
    
    //std::string localJsonDump = localSettings.modelConfig.dump(3);
    localSettings.updateDump();
    
    std::string inputStr = "";
    std::string inputPrompt = localSettings.params.prompt;
    std::string inputAntiprompt = "";
    if(localSettings.params.antiprompt.size()) inputAntiprompt = localSettings.params.antiprompt[0];
    std::string inputAntiCFG = localSettings.params.cfg_negative_prompt;
    
    //static char inputChars[128 * 16];
    std::string helpLabel = "Your question";
    std::string aTiming = "Not calculated yet...";
    std::string output = "...";
    int maxString = 3000;
    
    
    
    ImGuiStyle& style = ImGui::GetStyle();
    
    style.AntiAliasedLines = false;
    style.AntiAliasedLinesUseTex = false;
    style.AntiAliasedFill = false;
   
    style.FrameBorderSize  = 0.0f;
    style.ChildBorderSize  = 1.0f;
    style.WindowPadding.x = 2.0f;
    style.CellPadding.x  = 3.0f;
    style.ItemSpacing.x  = 4.0f;
    
    style.ChildRounding = 6.0f;
    style.FrameRounding = 7.0f;
    style.PopupRounding = 8.0f;
    style.TabRounding = 9.0f;
    style.GrabRounding = 10.0f;
    
    style.FramePadding.x = 10.0f;
    
    //for the "full-frame" main window
    style.WindowRounding = 0.0f;
    
    
    ImGuiWindowFlags chat_window_flags = ImGuiWindowFlags_MenuBar|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoCollapse|ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoResize;
    //const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
    

    // Main loop
    bool done = false;
#ifdef __EMSCRIPTEN__
    // For an Emscripten build we are disabling file-system access, so let's not attempt to do a fopen() of the imgui.ini file.
    // You may manually call LoadIniSettingsFromMemory() to load settings from your own storage.
    io.IniFilename = nullptr;
    EMSCRIPTEN_MAINLOOP_BEGIN
#else
    while (!done)
#endif
    {        
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
                done = true;
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(use_work_area ? viewport->WorkPos : viewport->Pos);
        ImGui::SetNextWindowSize(use_work_area ? viewport->WorkSize : viewport->Size);
        //ImGui::SetNextWindowPos(ImVec2(0, 0));
        //ImGui::SetNextWindowSize(ImVec2(width, height));
        ImGui::Begin("Chat", NULL, chat_window_flags); 
        
        localSettings.checkLocalConfig();
        
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("Open..."))
            {
                if (ImGui::MenuItem("Open a model", "Ctrl+O")) {
                    if (modelsFolderName == "NULL"){
#if GGML_OLD_FORMAT
                        auto getModelName = tinyfd_openFileDialog("Select a model...", NULL,1, modelFilterPatterns, NULL,0);
#else 
                        auto getModelName = tinyfd_openFileDialog("Select a model...", NULL,2, modelFilterPatterns, NULL,0);
#endif
                        //auto getModelName = openFile(modelFilterPatterns);
                        if (getModelName) {
                        //if (getModelName != "NULL") {
                            std::string getModelNameStr = getModelName;
                            //sanitizePath(getModelNameStr);
                            localSettings.modelName = getModelNameStr;
                            localSettings.updateSettings();
                            localSettings.fillLocalJson();
                            //localJsonDump = localSettings.modelConfig.dump(3);
                            localSettings.updateDump();
                            n_ctx_idx = sqrt( localSettings.params.n_ctx / 2048 );
                            // if (!localSettings.noConfig) {
                                // localSettings.fillLocalJson();
                            // }
                            if (newChat.loaded == 9) {
                                newChat.loaded = 0;
                                copiedDialog = false;
                                copiedSettings = false;
                                newChat.unload();
                            }
                        }
                    } else {
                        //auto getModelName = tinyfd_openFileDialog("Select a model...", modelsFolderName.c_str(),1, modelFilterPatterns, NULL,0);
#if GGML_OLD_FORMAT
                        auto getModelName = tinyfd_openFileDialog("Select a model...", modelsFolderName.c_str(),1, modelFilterPatterns, NULL,0);
#else 
                        auto getModelName = tinyfd_openFileDialog("Select a model...", modelsFolderName.c_str(),2, modelFilterPatterns, NULL,0);
#endif
                        
                        if (getModelName) {
                            std::string getModelNameStr = getModelName;
                            //sanitizePath(getModelNameStr);
                            
                            localSettings.modelName = getModelNameStr;
                            sanitizePath(localSettings.modelName);
                            
                            localSettings.updateSettings();
                            localSettings.fillLocalJson();
                            //localJsonDump = localSettings.modelConfig.dump(3);
                            localSettings.updateDump();
                            
                            if (newChat.loaded == 9) {
                                newChat.loaded = 0;
                                copiedDialog = false;
                                copiedSettings = false;
                                newChat.unload();
                            }
                        }
                    }
                }
                if (ImGui::MenuItem("Select models folder", "Ctrl+S"))   {
                     auto getModelFolderName = tinyfd_selectFolderDialog("Select a model...", currPath);
                     if (getModelFolderName) {
                         modelsFolderName = getModelFolderName;
                         modelsFolderName += "\\";
                     }
                }
                
                if (ImGui::MenuItem("Select prompts folder", "Ctrl+S"))   { 
                     auto getPromptsFolderName = tinyfd_selectFolderDialog("Select a model...", currPath);
                     if (getPromptsFolderName) {
                         localSettings.promptFilesFolder = getPromptsFolderName;
                         localSettings.promptFilesFolder += "\\";
                         localSettings.getFilesList();
                     }
                }
                
                ImGui::Checkbox("Demo Window", &show_demo_window); 
                
                ImGui::EndMenu();
            }
            
            if (ImGui::BeginMenu("Save..."))
            {
                if (ImGui::MenuItem("Save settings", "Ctrl+S")) {
                    nlohmann::json settingsJson;
                    SDL_GL_GetDrawableSize(window, &width, &height);
                    
                    settingsJson["width"] = width;
                    settingsJson["height"] = height;
                    settingsJson["font"] = fontFile;
                    settingsJson["fontSize"] = fontSize;
                    if(modelsFolderName != "NULL") settingsJson["modelsFolder"] = modelsFolderName;
                    settingsJson["promptsFolder"] = localSettings.promptFilesFolder;
                    
                    writeJson(settingsJson, "chatConfig.json");
                    // if (modelsFolderName == "NULL"){
                        // auto getModelName = tinyfd_openFileDialog("Select a model...", NULL,1, modelFilterPatterns, NULL,0);
                        // if (getModelName) localSettings.modelName = getModelName;
                    // } else {
                        // auto getModelName = tinyfd_openFileDialog("Select a model...", modelsFolderName.c_str(),1, modelFilterPatterns, NULL,0);
                        // if (getModelName) localSettings.modelName = getModelName;
                    // }
                }
                if (ImGui::MenuItem("Save model config", "Ctrl+S"))   { 
                     // auto getModelFolderName = tinyfd_selectFolderDialog("Select a model...", NULL);
                     // if (getModelFolderName) {
                         // modelsFolderName = getModelFolderName;
                         // modelsFolderName += "\\";
                     // }
                    localSettings.pushToMainConfig();
                    writeJson(localSettings.localConfig, "config.json");
                     
                }

                
                ImGui::EndMenu();
            }
            
            if (!localSettings.noConfig){
                if (ImGui::BeginMenu("Models from config..."))
                {
                    for (auto& mdl : localSettings.modelsFromConfig){
                        if ( std::filesystem::exists(mdl.first)){
                            if (ImGui::MenuItem(mdl.second.c_str())) {
                               localSettings.modelName = mdl.first;
                               localSettings.updateSettings();
                               localSettings.fillLocalJson();
                               //localJsonDump = localSettings.modelConfig.dump(3);
                               localSettings.updateDump();
                               n_ctx_idx = sqrt( localSettings.params.n_ctx / 2048 );
                               
                               if (newChat.isContinue != 'w' && newChat.loaded == 9) {
                                   newChat.loaded = 0;
                                   copiedDialog = false;
                                   copiedSettings = false;

                                   hasModel = false;
                                   newChat.unload();
                               }
                            }
                        }
                    }
                    ImGui::EndMenu();
                }
            }
            
            if (ImGui::BeginMenu("UI..."))
            {
                if (ImGui::MenuItem("Open a font file", "Ctrl+S")) {
                    std::string fontFileOpened = openFile(fontFilterPatterns);
                    if (fontFileOpened != "NULL") {
                        sanitizePath(fontFileOpened);
                        //fontFile = fontFileOpened;
                        configJson = getJson("chatConfig.json");
                        configJson["font"] = fontFileOpened;
                        
                        writeJson(configJson, "chatConfig.json");
                        //font = io.Fonts->AddFontFromFileTTF(fontFile.c_str(), fontSize, nullptr, io.Fonts->GetGlyphRangesCyrillic());
                        
                        
                        //io.FontDefault = fontX;
                        //io.Fonts->Build();
                    }
                }
                
                ImGui::EndMenu();
            }
            // if (ImGui::BeginMenu("Restart")){ 
                // if (loaded == 9){
                    // loaded = 0;
                    // copied = false;
                    
                    // newChat.clear();
                // }
                // ImGui::EndMenu();
            // }
            
            ImGui::EndMenuBar();
        }
        
        //ImGui::BeginChild("Dialog", ImVec2( ImGui::GetContentRegionAvail().x * 0.5f, 24.0f), false);
        static int style_idx = 2;
        if (ImGui::Combo("", &style_idx, "Dark theme\0Light theme\0Classic theme\0", 50))
        {
            switch (style_idx)
            {
                case 0: ImGui::StyleColorsDark(); break;
                case 1: ImGui::StyleColorsLight(); break;
                case 2: ImGui::StyleColorsClassic(); break;
            }
        }
        //ImGui::EndChild();
        
        //ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
        ImGui::SameLine();
        
        ImGui::Checkbox("Chat mode", &chatMode); 
        
        
        if (newChat.loaded == 9){
            if (newChat.isContinue != 'w') {
                // ImGui::SameLine();
                // if (ImGui::Button("Soft restart")){ 
                    // newChat.loaded = 0;
                    // copiedDialog = false;
                    // copiedSettings = false;
                    // cancelled = false;
                    
                    // //newChat.clear();
                    // newChat.clearSoft();
                // }
                ImGui::SameLine();
                if (ImGui::Button("Restart")){ 
                    newChat.loaded = 0;
                    tokens_this_session = 0;
                    copiedDialog = false;
                    copiedSettings = false;
                    initTokens = false;
                    cancelled = false;

                    hasModel = false;
                    newChat.unload();
                }
                ImGui::SameLine();
                ImGui::TextWrapped(("Tokens: " + std::to_string(past_this_session)).c_str());
            }  else {
                ImGui::SameLine();
                if (ImGui::Button("Stop")){ 
                    newChat.stop();
                    tokens_this_session = newChat.last_tokens;
                    consumed_this_session = newChat.consumed_tokens;
                    past_this_session = newChat.past_tokens;
                    cancelled = true;
                }
                ImGui::SameLine();
                ImGui::TextWrapped(("Tokens: " + std::to_string(past_this_session)).c_str());
            }
        }
        
        
        
        //ImGui::Separator();
        //ImGui::Spacing();
        //ImGui::Text("Test");
        
        //ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
        if (ImGui::BeginTabBar("MyTabBar", ImGuiTabBarFlags_None))
        {
// Chat tab (initial info and loading, or chat display)/////////////////////////////////////////////
            if (ImGui::BeginTabItem("Chat"))
            {
// staring main loop once loaded the model//////////////////////////////////////////////////////////
                if (localSettings.noConfig){
                    ImGui::TextWrapped("No model config file found!");
                }
                
                if (newChat.loaded == 9) {
                    
                    if (!initTokens){
                        tokens_this_session = newChat.last_tokens;
                        consumed_this_session = newChat.consumed_tokens;
                        past_this_session = newChat.past_tokens;
                        
                        initTokens = true;
                    }
                    
                    if (!copiedSettings){
                        localSettings.getSettings(newChat.newChat);
                        copiedSettings = true;
                    }

                    {
                        ImGui::Indent();
                        ImGui::BeginChild("Dialog", ImVec2( ImGui::GetContentRegionAvail().x * 0.99f, ImGui::GetContentRegionAvail().y*0.80f), false);
                        
                            // to avoid mutexes we only use a copy of dialog DB for UI
                            if (newChat.isContinue == 'i') {
                                if (!copiedDialog){
                                    localResultPairs = newChat.resultsStringPairs;
                                    ImGui::SetScrollFromPosY(ImGui::GetCursorPos().y + ImGui::GetContentRegionAvail().y, 0.25f);
                                    copiedDialog = true;
                                    aTiming = newChat.lastTimings;
                                    helpLabel = " ";
                                }
                            }

/////// rendering dialogs ////////////////////////////////////////////////////////////////////////
                            int messageNum = 0;
                            for (auto& r : localResultPairs){
                                ++messageNum;
                                if (r.first == "AI"){
                                    //std::cout << r.second;
                                    //ImGui::TextWrapped(r.second.c_str());
                                    if (chatMode) ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + ImGui::GetContentRegionAvail().x * 0.5f);
                                    else ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + ImGui::GetContentRegionAvail().x);
                                    
                                    if (messageNum == 1){
                                        ImGui::SeparatorText("Context");
                                    }
                                    
                                    //auto idxPopup = std::to_string(messageNum).c_str();
                                    if (r.second.size() < maxString){
                                        ImGui::Text((r.second.c_str()));
                                        if (ImGui::BeginPopupContextItem(std::to_string(messageNum).c_str()))
                                        {
                                            if (ImGui::Selectable("Copy")){
                                                ImGui::LogToClipboard();
                                                ImGui::LogText(r.second.c_str());
                                                ImGui::LogFinish();
                                            }
                                            ImGui::EndPopup();
                                        }
                                    } else {
                                        std::string msg = r.second;
                                        while (msg.size() > maxString){
                                            std::string subMsg = msg.substr(0,maxString);
                                            int posDelim = subMsg.rfind('\n');
                                            if (posDelim == subMsg.npos){
                                                int posSpace = subMsg.rfind(". ");
                                                subMsg = msg.substr(0,posSpace);
                                                msg = msg.substr(posSpace);
                                            } else {
                                                subMsg = msg.substr(0,posDelim);
                                                msg = msg.substr(posDelim);
                                            }
                                            
                                            ImGui::Text((subMsg.c_str()));
                                            if (ImGui::BeginPopupContextItem(std::to_string(messageNum).c_str()))
                                            {
                                                if (ImGui::Selectable("Copy")){
                                                    ImGui::LogToClipboard();
                                                    ImGui::LogText(r.second.c_str());
                                                    ImGui::LogFinish();
                                                }
                                                ImGui::EndPopup();
                                            }
                                        }
                                        ImGui::Text((msg.c_str()));
                                        if (ImGui::BeginPopupContextItem(std::to_string(messageNum).c_str()))
                                        {
                                            if (ImGui::Selectable("Copy")){
                                                ImGui::LogToClipboard();
                                                ImGui::LogText(r.second.c_str());
                                                ImGui::LogFinish();
                                            }
                                            ImGui::EndPopup();
                                        }
                                    }
                                    ImGui::PopTextWrapPos();
                                    
                                    if (messageNum == 1){
                                        ImGui::SeparatorText("Dialog");
                                    }

                                    //ImGui::PushItemWidth(-ImGui::GetContentRegionAvail().x * 0.5f);
                                    //ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.5f);
                                    //ImGui::Text(r.second.c_str(), wrap_width);
                                    //ImGui::PopTextWrapPos();
                                    //ImGui::InputTextMultiline(" ", &r.second, ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 2), ImGuiInputTextFlags_ReadOnly);
                                } else {
                                    
                                    //if (chatMode) ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x * 0.35f);
                                    
                                    //std::cout << r.first << r.second;
                                    //std::string questionStr = r.first + r.second;
                                    //ImGui::SeparatorText(r.first.c_str());
                                    if (chatMode) ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x * 0.40f);
                                    ImGui::TextWrapped((r.first + r.second).c_str());
                                    if (ImGui::BeginPopupContextItem(std::to_string(messageNum).c_str()))
                                    {
                                        if (ImGui::Selectable("Copy")){
                                            ImGui::LogToClipboard();
                                            ImGui::LogText(r.second.c_str());
                                            ImGui::LogFinish();
                                        }
                                        
                                        if(templatesJson.contains("error")){
                                            if (ImGui::Selectable("Store as saved")){
                                                templatesJson["saved"].push_back(r.second);
                                                templatesJson.erase("error");
                                            }
                                        } else {
                                            for (auto& [key, value] : templatesJson.items() ){
                                                if (value.is_array()) {
                                                    if (ImGui::Selectable(("Store as " + key).c_str())){
                                                        templatesJson[key].push_back(r.second);
                                                    }
                                                }
                                            }
                                        }
                                        
                                        ImGui::EndPopup();
                                    }
                                    //ImGui::Separator();
                                    //ImGui::Text((r.first + r.second).c_str(), wrap_width);
                                    //ImGui::PopTextWrapPos();
                                    //ImGui::InputTextMultiline(" ", &questionStr, ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 2), ImGuiInputTextFlags_ReadOnly);
                                    
                                }
                                
                                ImGui::Spacing();
                                //if (r.second.back() != '\n') std::cout<< DELIMINER;
                            }
                            
                            // streaming dialog token-after-token
                            if (newChat.isContinue == 'w') {
                                //ImGui::TextWrapped("...");
                                if (chatMode) ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + ImGui::GetContentRegionAvail().x * 0.50f);
                                else ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + ImGui::GetContentRegionAvail().x);
                                output = newChat.lastResult;
                                
                                
                                if (output.size() < maxString){
                                    ImGui::TextWrapped(output.c_str());
                                } else {
                                    std::string msg = output;
                                    while (msg.size() > maxString){
                                        std::string subMsg = msg.substr(0,maxString);
                                        int posDelim = subMsg.rfind('\n');
                                        if (posDelim == subMsg.npos){
                                            int posSpace = subMsg.rfind(". ");
                                            subMsg = msg.substr(0,posSpace);
                                            msg = msg.substr(posSpace);
                                        } else {
                                            subMsg = msg.substr(0,posDelim);
                                            msg = msg.substr(posDelim);
                                        }
                                        ImGui::Text((subMsg.c_str()));
                                    }
                                    ImGui::Text((msg.c_str()));
                                }
                                
                                ImGui::PopTextWrapPos();
                                //newChat.getResultAsyncString();
                                scrolled = false;
                                if (!scrolled) {
                                    ImGui::SetScrollFromPosY(ImGui::GetCursorPos().y + ImGui::GetContentRegionAvail().y, 0.25f);
                                    scrolled = true;
                                }
                                
                                ImGui::SeparatorText("Generating");
                                
                                tokens_this_session = newChat.last_tokens;
                                consumed_this_session = newChat.consumed_tokens;
                                past_this_session = newChat.past_tokens;
                                //helpLabel = "Generating... "+std::to_string(tokens_this_session) + " tokens.";
                                //ImGui::SetScrollFromPosY(ImGui::GetCursorPos().y, 0.25f);
                                //ImGui::PushItemWidth(-ImGui::GetContentRegionAvail().x * 0.5f);
                                //ImGui::SetScrollFromPosY(ImGui::GetCursorStartPos().y + 200.0f, 0.25f);
                            } else {
                                if (messageNum > 1) ImGui::SeparatorText("");
                            }
                        
                        ImGui::EndChild();
                        ImGui::Unindent();
                    }
// INPUT ////////////////////////////////////////////////////////////////////////////////////////
                    //ImGui::InputTextMultiline("input text", inputStr, IM_ARRAYSIZE(inputStr));
                    
                    //static ImGuiInputTextFlags flags = ImGuiInputTextFlags_AllowTabInput;
                    //ImGui::InputTextMultiline(helpLabel.c_str(), &inputStr, ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16), flags);
                    
                    
                    //static char inputChars[24 * 16];
                    //ImGui::InputTextMultiline(helpLabel.c_str(), inputChars, IM_ARRAYSIZE(inputChars));
                    
                    // still need to make input field wrapping
                    //ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + ImGui::GetContentRegionAvail().x * 0.65f);
                    ImGui::InputTextMultiline(helpLabel.c_str(), &inputStr);
                    //ImGui::PopTextWrapPos();
                    
                    ImGui::SameLine();
                    
                    ImGui::BeginChild("");
                    // we don't need to show the button if generation is going
                    if (newChat.isContinue != 'w') {
                    
                        
                        if (ImGui::Button("Send")) {
                            if (inputStr.size()){
                                sessionHistory[localSettings.modelName].push_back(inputStr);
                            }
                            newChat.appendQuestion(inputStr);
                            localResultPairs = newChat.resultsStringPairs;
                            //newChat.getResultAsyncString();
                            inputStr = "";
                            //inputChars = inputStr.c_str();
                            //tokens_this_session = newChat.last_tokens;
                            helpLabel = "Generating... ";//+std::to_string(tokens_this_session) + " tokens.";
                            
                            
                            //newChat.isContinue = 'w';
                            newChat.startGen();
                            output = "...";
                            newChat.getResultAsyncStringFull();
                            copiedDialog = false;
                            scrolled = false;
                            
                        }
                        
                        if (ImGui::Button("Choose a template")) {
                            ImGui::OpenPopup("Templates");
                        }
                        
                        if (ImGui::BeginPopup("Templates"))
                        {
                            ImVec2 work_size = viewport->WorkSize;
                            ImGui::BeginChild("Templates list", ImVec2(  work_size.x * 0.5, work_size.y * 0.7));
                                templatesListSelect(templatesJson, inputStr);
                            ImGui::EndChild();
                            ImGui::EndPopup();
                            
                        }
                            
                        if(cancelled){
                            //ImGui::SameLine();
                            if (ImGui::Button("Repeat")) {
                            
                                //newChat.appendQuestion(inputStr);
                                localResultPairs = newChat.resultsStringPairs;
                                //newChat.getResultAsyncString();
                                //inputStr = "";
                                //inputChars = inputStr.c_str();
                                helpLabel = "ReGenerating...";
                                
                                
                                //newChat.isContinue = 'w';
                                newChat.startGen();
                                output = "...";
                                newChat.getResultAsyncStringRepeat();
                                copiedDialog = false;
                                scrolled = false;
                                cancelled = false;
                                
                            }
                        }
                    } //else {
                    //    newChat.getResultAsyncString();
                    //}
                    ImGui::EndChild();
                    
                } else {
// Initial buttons and settings to load a model////////////////////////////////////////////////////
                    if (newChat.isContinue == '_') {
                        if(modelsFolderName != "NULL") {
                            ImGui::TextWrapped(( "Models folder: " + modelsFolderName).c_str());
                            ImGui::Separator();
                        }
                        if(localSettings.modelName != "NULL") 
                            ImGui::TextWrapped(( "Selected model: " + localSettings.modelName).c_str());
                        else
                            ImGui::TextWrapped(( "Default model: " + localSettings.modelFromJson).c_str());
                        ImGui::Separator();
                        
                        if (localSettings.instructFileFromJson != "NULL") {
                            ImGui::TextWrapped( ("Default instruct file: " + localSettings.instructFileFromJson).c_str() );
                            ImGui::Separator();
                        }
                        //ImGui::TextWrapped( ("Font file: " + fontFile).c_str() );
                        ImGui::Spacing();
                        
                        ImGui::Indent();
                        // int checkInputs = localSettings.checkChangedInputs();
                        // if (checkInputs != 0){
                            // if (checkInputs != 2 && checkInputs != 4 && checkInputs != 6) ImGui::TextWrapped( ("Set prompt to " + localSettings.inputPrompt).c_str() );
                            // if (checkInputs != 1 && checkInputs != 4 && checkInputs != 5) ImGui::TextWrapped( ("Set antiprompt to " + localSettings.inputAntiprompt).c_str() );
                            // if (localSettings.inputAntiCFG != "NULL" && checkInputs != 2 && checkInputs != 1 && checkInputs != 3) ImGui::TextWrapped( ("Set CFG antiprompt to " + localSettings.inputAntiCFG).c_str() );
                        // }
                        
                        if (!localSettings.checkInputPrompt()) ImGui::TextWrapped( ("Set prompt to " + localSettings.inputPrompt).c_str() );
                        
                        if (!localSettings.checkInputAntiprompt()) ImGui::TextWrapped( ("Set antiprompt to " + localSettings.inputAntiprompt).c_str() );
                        
                        if (!localSettings.checkInputAntiCFG()) ImGui::TextWrapped( ("Set CFG antiprompt to " + localSettings.inputAntiCFG).c_str() );
                        
                        if (localSettings.params.cfg_scale > 1.0) {
                            ImGui::TextWrapped( ("Set CFG cfg_scale to " + std::to_string(localSettings.params.cfg_scale)).c_str() );
                            ImGui::Separator();
                        }
                        ImGui::Unindent();
                        
                        
                        ImGui::Spacing();
                        
                        if (ImGui::Button("Apply to config")) {
                            //localSettings.getFromJson("config.json");
                            localSettings.updateInput();
                            localSettings.fillLocalJson();
                            localSettings.syncInputs();
                            //localJsonDump = localSettings.modelConfig.dump(3);
                            localSettings.updateDump();
                        }
                        
                        ImGui::SameLine();
                        
                        if (!localSettings.noConfig){
                            if (!hasModel) {
                                if (ImGui::Button("Load and init model")) {
                                    //localSettings.fillLocalJson();
                                    
                                    // if(localSettings.modelName != "NULL" || localSettings.inputPrompt != "NULL" || localSettings.inputAntiprompt != "NULL" || localSettings.inputAntiCFG != "NULL") load(newChat, localSettings);
                                    // else load(newChat);
                                    //load(newChat, localSettings.localConfig, hasModel);
                                    
                                    //newChat.load(localSettings.modelConfig, hasModel);
                                    newChat.jsonConfig = localSettings.modelConfig;
                                    newChat.load();
                                    
                                    hasModel = true;
                                    copiedDialog = false;
                                    copiedSettings = false;
                                    newChat.isContinue = 'l';
                                }
                            } else {
                                if (ImGui::Button("Init model")) {
                                    //localSettings.fillLocalJson();
                                    
                                    // if(localSettings.modelName != "NULL" || localSettings.inputPrompt != "NULL" || localSettings.inputAntiprompt != "NULL" || localSettings.inputAntiCFG != "NULL") load(newChat, localSettings);
                                    // else load(newChat);
                                    copiedDialog = false;
                                    copiedSettings = false;
                                    //load(newChat, localSettings.localConfig, hasModel);
                                    newChat.load(localSettings.modelConfig, hasModel);
                                    //load_task(newChat, localSettings, hasModel);
                                    newChat.isContinue = 'l';
                                }
                            }
                        } else {
                            if(localSettings.modelName == "NULL") ImGui::TextWrapped( "Choose a model first!" );
                            if(localSettings.inputPrompt == "NULL") ImGui::TextWrapped( "Add prompt first! Also, add antiprompt if you don't want endless NN conversation." );
                            
                        }
                        
                        
                        ImGui::Separator();
                        ImGui::Spacing();
                        
                        ImGui::TextWrapped( "Below you can set up prompt and antiprompt instead of preconfigured ones." );                        
                        if (localSettings.inputInstructFile == "NULL"){
                            
                            if (localSettings.instructFileFromJson != "NULL") ImGui::TextWrapped( "This model has an instruct file set for it in config. Prompt and antiprompt will not be used!" );
                            
                            ImGui::InputTextMultiline("Prompt", &inputPrompt); ImGui::SameLine(); HelpMarker( "Prompt can be used as a start of the dialog, providing context and/or format of dialog." ); 
                            if (ImGui::Button("Apply prompt")) {
                                    localSettings.inputPrompt = inputPrompt;
                                } ImGui::SameLine(); if (ImGui::Button("Clear prompt")) {
                                    localSettings.inputPrompt = "NULL";
                            }
                            ImGui::SameLine();
                            //ImGui::BeginChild("Dialog", ImVec2( ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y*0.2f), false);
                            if (ImGui::Button("Open an instruct file...")) {
                                
                                auto inputInstructFile = tinyfd_openFileDialog("Select an instruct file...", currPath,1, instructFilterPatterns, NULL,0);
                                if (inputInstructFile) {
                                    //localSettings.inputInstructFile = inputInstructFile;
                                    
                                    localSettings.readInstructFile(inputInstructFile, inputPrompt, inputAntiprompt);
                                    //localSettings.syncInputs();
                                }
                            }
                            
                            
                            
                            if (localSettings.promptFiles.size()){
                                ImGui::SameLine();
                                
                                if (ImGui::Button("Choose an instruct file...")) {
                                     ImGui::OpenPopup("Prompts");
                                }
                                
                                
                                if (ImGui::BeginPopup("Prompts"))
                                {
                                    ImVec2 work_size = viewport->WorkSize;
                                    ImGui::BeginChild("Prompts list", ImVec2(  work_size.x * 0.3, work_size.y * 0.5));
                                    for ( auto promptFile : localSettings.promptFiles){
                                        //ImGui::TextWrapped(promptFile.filename().c_str());
                                        if (ImGui::Selectable( promptFile.filename().string().c_str() )){
                                            //localSettings.inputInstructFile = promptFile.string();
                                            localSettings.readInstructFile(promptFile.string(), inputPrompt, inputAntiprompt);
                                            //localSettings.syncInputs();
                                        }
                                    }
                                    ImGui::EndChild();
                                    ImGui::EndPopup();
                                    
                                }
                            }
                            
                            ImGui::SameLine();
                            
                            if (ImGui::Button("Save current instruct...")) {
                                //ImGui::OpenPopup("Save instruct");
                                auto savedInstruct = tinyfd_saveFileDialog( inputAntiprompt.c_str() , localSettings.promptFilesFolder.c_str() , 1 , instructFilterPatterns, NULL);
                                
                                if (savedInstruct){
                                    std::string fileContents = inputPrompt;
                                    
                                    int antiPos = fileContents.rfind('\n');
                                    if (antiPos == fileContents.npos && inputPrompt != inputAntiprompt) {
                                        fileContents += '\n' + inputAntiprompt;
                                    }
                                    
                                    //std::string filename = localSettings.promptFilesFolder + savedInstruct + ".txt";
                                    
                                    writeTextFileOver(savedInstruct, fileContents);
                                    localSettings.getFilesList();
                                }
                                
                            }
                            
                            ImVec2 center = ImGui::GetMainViewport()->GetCenter();
                            ImVec2 work_size = viewport->WorkSize;
                            ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

                            if (ImGui::BeginPopupModal("Save instruct"))
                            {
                                
                                
                                if (ImGui::BeginChild("File name", ImVec2( work_size.x * 0.5f, work_size.y * 0.5f)))
                                {
                                    static std::string instructName = inputAntiprompt;
                                    struct TextFilters
                                    {
                                        static int FilterFileLetters(ImGuiInputTextCallbackData* data)
                                        {
                                            if (data->EventChar < 256 && strchr(" /\\*:?\"|<> ", (char)data->EventChar))
                                                return 1;
                                            return 0;
                                        }
                                    };
                                    if (localSettings.promptFilesFolder.back() != '\\' && localSettings.promptFilesFolder.back() != '/') localSettings.promptFilesFolder += '/';
                                    std::string fileContents = inputPrompt;
                                    
                                    int antiPos = fileContents.rfind('\n');
                                    if (antiPos == fileContents.npos) {
                                        fileContents += inputAntiprompt;
                                    }
                                    
                                    ImGui::TextWrapped(( "Save to: " + localSettings.promptFilesFolder + instructName + ".txt").c_str());
                                    ImGui::Separator();
                                    ImGui::TextWrapped(( "Saving: " + fileContents).c_str());
                                    ImGui::Separator();
                                    
                                    ImGui::InputText("< File name", &instructName, ImGuiInputTextFlags_CallbackCharFilter, TextFilters::FilterFileLetters);
                                    
                                    if (instructName.size()){
                                        if (ImGui::Button("Save", ImVec2(work_size.x * 0.4f, 0))) {
                                            
                                            
                                            writeTextFileOver(localSettings.promptFilesFolder + instructName + ".txt", fileContents);
                                            localSettings.getFilesList();
                                            
                                            ImGui::CloseCurrentPopup();
                                        }
                                    }
                                    
                                    if (ImGui::Button("Cancel", ImVec2(work_size.x * 0.4f, 0))) { ImGui::CloseCurrentPopup(); }
                                }
                                
                                ImGui::EndChild();
                                
                                
                                
                                
                                
                                ImGui::EndPopup();
                            }
                            
                            
                            
                            if (localSettings.instructFileFromJson != "NULL"){
                                ImGui::SameLine();
                                if (ImGui::Button("Clear instruct file from config")) {
                                        //localSettings.inputInstructFile = "NULL";
                                        localSettings.clearFile();
                                        //localSettings.updateSettings();
                                        //localSettings.hasInstructFile = false;
                                }
                            }
                        } else {
                            ImGui::TextWrapped( ("Selected file: " + localSettings.inputInstructFile).c_str() );
                            if (ImGui::Button("Clear instruct file")) {
                                    localSettings.inputInstructFile = "NULL";
                                    localSettings.syncInputs();
                            }
                        }
                        
                        //ImGui::EndChild();
                        
                        ImGui::InputText("Antiprompt", &inputAntiprompt); ImGui::SameLine(); HelpMarker( "Antiprompt is needed to control when to stop generating and wait for your input." ); 
                        if (ImGui::Button("Apply antiprompt")) {
                                    localSettings.inputAntiprompt = inputAntiprompt;
                                } ImGui::SameLine(); if (ImGui::Button("Clear antiprompt")) {
                                    localSettings.inputAntiprompt = "NULL";
                                }
                        ImGui::InputTextMultiline("CFG Antiprompt", &inputAntiCFG); ImGui::SameLine(); HelpMarker( "CFG Antiprompt is used to guide the output by defining what should NOT be in it. cfg_scale must be higher than 1.0 to activate CFG." ); 
                        if (ImGui::Button("Apply CFG antiprompt")) {
                                    localSettings.inputAntiCFG = inputAntiCFG;
                                } ImGui::SameLine(); if (ImGui::Button("Clear CFG antiprompt")) {
                                    localSettings.inputAntiCFG = "NULL";
                                }
                        ImGui::SliderFloat("cfg_scale", &localSettings.params.cfg_scale, 1.0f, 4.0f); ImGui::SameLine(); HelpMarker("How strong the cfg is. High values might result in no answer generated.");
                        //ImGui::SliderInt("n_ctx", &localSettings.n_ctx, 2048, 16384, "%2048"); ImGui::SameLine(); HelpMarker("The size of context, must be 1024x");
                        
                        ImGui::SliderInt("n_threads", &localSettings.params.n_threads, 0, totalThreads); ImGui::SameLine(); HelpMarker("Number of threads to use, recommended to leave at least one for the system.");
                        
                        if (clblast == 1) { 
                            ImGui::SliderInt("n_gpu_layers", &localSettings.params.n_gpu_layers, 0, 100); ImGui::SameLine(); HelpMarker("Number of layers to offload onto GPU.");
                        }
                        
                        //paramsPanel(localSettings, totalThreads, ImVec2( ImGui::GetContentRegionAvail().x * 0.70f, ImGui::GetContentRegionAvail().y));
                        
                        
                        if (ImGui::Combo("n_ctx", &n_ctx_idx, " 2048\0 4096\0 8192\0 16384\0 32768\0"))
                        {
                            switch (n_ctx_idx)
                            {
                                case 0: localSettings.params.n_ctx = 2048; break; // 1
                                case 1: localSettings.params.n_ctx = 4096; break; // 2
                                case 2: localSettings.params.n_ctx = 8192; break; // 4
                                case 3: localSettings.params.n_ctx = 16384; break; // 8
                                case 4: localSettings.params.n_ctx = 32768; break; // 16
                            }
                        } ImGui::SameLine(); HelpMarker("The size of context, must be 2048x. WARNING! Check if your model supports context size greater than 2048! You may also want to adjust rope-scaling for n_ctx greater than 4k.");
                        
                        
                        
                        
                        
                    } else {
// Information while  LOADING////////////////////////////////////////////////////////////////////
                        if (localSettings.modelName != "NULL") ImGui::TextWrapped( ("LOADING " + localSettings.modelName).c_str() ); // indicator for loading
                        else {
                            ImGui::TextWrapped( ("LOADING " + localSettings.modelFromJson).c_str() );
                            ImGui::Indent();
                            if (localSettings.modelName != "NULL") ImGui::TextWrapped( ("Set model to: " + localSettings.modelName).c_str() );
                            if (localSettings.inputPrompt != "NULL") ImGui::TextWrapped( ("Set prompt to: " + localSettings.inputPrompt).c_str() );
                            if (localSettings.inputAntiprompt != "NULL") ImGui::TextWrapped( ("Set antiprompt to: " + localSettings.inputAntiprompt).c_str() );
                            if (localSettings.inputAntiCFG != "NULL") ImGui::TextWrapped( ("Set CFG antiprompt to: " + localSettings.inputAntiCFG).c_str() );
                            ImGui::Unindent();
                        }
                    }
                }
    
                ImGui::EndTabItem();
            }
//SETTINGS TAB///////////////////////////////////////////////////////////////////////////////////
            if (ImGui::BeginTabItem("Model parameters"))
            {
                if (newChat.loaded == 9) {
                    // if (newChat.isContinue != 'w') {
                        // if (ImGui::Button("Calculate timings")) {
                            // ImGui::OpenPopup("Timings");
                        // }
                        
                        // if (ImGui::BeginPopupModal("Timings"))
                        // {
                            // ImGui::BeginChild("Curernt timings");
                            // //static std::string aTiming = newChat.newChat.get_timings();
                            // //ImGui::TextWrapped(aTiming.c_str());
                            // ImGui::TextWrapped(newChat.lastTimings.c_str());
                            // if (ImGui::Button("Close", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
                            // ImGui::EndChild();
                            // ImGui::EndPopup();
                            
                        // }
                    // }
                    //ImGui::TextWrapped(newChat.newChat.get_timings().c_str());
                    if (copiedSettings){
                        ImGui::TextWrapped( ("SEED: " + std::to_string(localSettings.params.seed)).c_str() );
                        ImGui::TextWrapped( ("Base (initial) prompt: " + localSettings.params.prompt).c_str() ); ImGui::SameLine(); HelpMarker("This serves as the basis for the dialog, providing context. Make sure it ends with the antiptrompt if you don't want the NN to converse endlessly.");
                        ImGui::Separator();
                        //ImGui::TextWrapped( ("Antiprompt: " + newChat.newChat.params.antiprompt[0]).c_str() ); ImGui::SameLine(); HelpMarker("This serves as the basis for the dialog, providing context. Make sure it ends with the antiptrompt if you don't want the NN to converse endlessly.");
                        ImGui::TextWrapped( ("Antiprompt: " + localSettings.params.antiprompt[0]).c_str() ); ImGui::SameLine(); HelpMarker("This serves as a stop-word for the NN to be able to end generation and wait for your input.");
                        ImGui::Separator();
                        ImGui::TextWrapped( ("CFG (negative) prompt: " + localSettings.params.cfg_negative_prompt).c_str() ); ImGui::SameLine(); HelpMarker("If cfg_scale > 1.0, CFG negative prompt is used for additional guiding.");
                        ImGui::Separator();
                        // if (localSettings.params.rms_norm_eps != localSettings.paramsDefault.rms_norm_eps){
                            // ImGui::TextWrapped( ("rms-norm-eps: " + std::to_string(localSettings.params.rms_norm_eps)).c_str() ); ImGui::SameLine(); HelpMarker("Specific paramenter to adjust for different models (llama2, for example)");
                            // ImGui::Separator();
                        // }
                        if (localSettings.params.rope_freq_scale != paramsDefault.rope_freq_scale){
                            ImGui::TextWrapped( ("rope_freq_scale: " + std::to_string(localSettings.params.rope_freq_scale)).c_str() ); ImGui::SameLine(); HelpMarker("Specific paramenter to adjust for extra large context models - needs to be (base model n_ctx)/(this model n_ctx), for example, 0.25 for llama2 (4096) -based everythinglm-13b-16k (16384)");
                            ImGui::Separator();
                        }
                        if (localSettings.params.rope_freq_base != paramsDefault.rope_freq_base){
                            ImGui::TextWrapped( ("rope_freq_base: " + std::to_string(localSettings.params.rope_freq_base)).c_str() ); ImGui::SameLine(); HelpMarker("Specific paramenter to adjust for extra large context models.");
                            ImGui::Separator();
                        }
                        ImGui::TextWrapped( ("context size: " + std::to_string(localSettings.params.n_ctx)).c_str() ); ImGui::SameLine(); HelpMarker("The amount of data that can be stored and processed by the model within a session - depends on the model and should be used carefully. Models with context higher than 2048 usually have it in their name or description.");
                        ImGui::Separator();
                        ImGui::Spacing();
                        
                        if (newChat.isContinue != 'w') {
                            if (ImGui::Button("Get settings from model")) {
                                localSettings.getSettings(newChat.newChat);
                                localSettings.fillLocalJson();
                            }
                            ImGui::SameLine();
                            
                            if (ImGui::Button("Apply and store settings")) {
                                localSettings.fillLocalJson();
                                localSettings.pushSettings(newChat.newChat);
                                //localJsonDump = localSettings.modelConfig.dump(3);
                                localSettings.updateDump();
                            }
                        }
                    }
                    
                } else {
                    ImGui::TextWrapped("Model isn't loaded yet...");
                    
                    if (ImGui::Button("Store settings to config")) {
                        localSettings.fillLocalJson();
                        //localJsonDump = localSettings.modelConfig.dump(3);
                        localSettings.updateDump();
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Restore settings from config")) {
                        localSettings.getSettingsFromModelConfig();
                        localSettings.fillLocalJson();
                    }
                }

                paramsPanelNew(localSettings.params, totalThreads, ImVec2( ImGui::GetContentRegionAvail().x * 0.70f, ImGui::GetContentRegionAvail().y));
                
                
                
                ImGui::SameLine();
                
                ImGui::BeginChild("Timings", ImVec2( ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y), false);
                
                    ImGui::TextWrapped(aTiming.c_str());
                
                ImGui::EndChild();
                //}
                
                
                ImGui::EndTabItem();
            }
//Configuration json tab//////////////////////////////////////////////////////////////////////////
            if (ImGui::BeginTabItem("Current config"))
            {
                if (ImGui::Button("Reset config")) {
                    localSettings.getFromJson("config.json");
                    localSettings.fillLocalJson();
                    //localJsonDump = localSettings.modelConfig.dump(3);
                    localSettings.updateDump();
                }
                ImGui::SameLine();
                if (ImGui::Button("Update config")) {
                    //localJsonDump = localSettings.modelConfig.dump(3);
                    localSettings.updateDump();
                }
                ImGui::SameLine();
                if (ImGui::Button("Save config"))   { 
                     // auto getModelFolderName = tinyfd_selectFolderDialog("Select a model...", NULL);
                     // if (getModelFolderName) {
                         // modelsFolderName = getModelFolderName;
                         // modelsFolderName += "\\";
                     // }
                    localSettings.pushToMainConfig();
                    writeJson(localSettings.localConfig, "config.json");
                     
                }
                
                ImGui::BeginChild("Parameters from json file");
                    
                    ImGui::TextWrapped(localSettings.localJsonDump.c_str());
                    
                ImGui::EndChild();
                
                ImGui::EndTabItem();
            }
            
            // if (ImGui::BeginTabItem("Style"))
            // {
                // float sz = ImGui::GetTextLineHeight();
                // for (int i = 0; i < ImGuiCol_COUNT; i++)
                // {
                    // const char* name = ImGui::GetStyleColorName((ImGuiCol)i);
                    // ImVec2 p = ImGui::GetCursorScreenPos();
                    // ImGui::GetWindowDrawList()->AddRectFilled(p, ImVec2(p.x + sz, p.y + sz), ImGui::GetColorU32((ImGuiCol)i));
                    // ImGui::Dummy(ImVec2(sz, sz));
                    // ImGui::SameLine();
                    // ImGui::MenuItem(name);
                // }
                
                // ImGui::EndTabItem();
            // }
            
//Templates tab//////////////////////////////////////////////////////////////////////////
            if (ImGui::BeginTabItem("Templates"))
            {
                if (!templatesJson.contains("error")){
                    if (ImGui::Button("Reset config")) {
                        templatesJson = getJson("templates.json");
                        //localSettings.getFromJson("config.json");
                        //localSettings.fillLocalJson();
                        //localJsonDump = localSettings.modelConfig.dump(3);
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Save templates")) {
                        writeJson(templatesJson, "templates.json");
                    }
                    
                    templatesList(templatesJson, inputStr);
                } else {
                    ImGui::TextWrapped("This is where your saved templates (instructs or dialog lines) will be stored for this session - or, if you save them, loaded from templates.json file.");
                }
                
                ImGui::EndTabItem();
            }

            //ImGui::EndTabBar();
        
//Prompt files tab//////////////////////////////////////////////////////////////////////////
            if (ImGui::BeginTabItem("Prompt files"))
            {
                ImGui::TextWrapped(localSettings.promptFilesFolder.c_str());
                ImGui::Separator();
                ImGui::Spacing();
                
                if (localSettings.promptFiles.size()){
                    if (ImGui::Button("Refresh config")) {
                        localSettings.getFilesList();
                    }
                    // ImGui::SameLine();
                    // if (ImGui::Button("Save config")) {
                        // writeJson(templatesJson, "templates.json");
                    // }
                    
                    //templatesList(templatesJson, inputStr);
                    
                    ImGui::BeginChild("Prompt files from folder");
                    
                    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
                    ImVec2 work_size = viewport->WorkSize;
                        
                    for ( auto promptFile : localSettings.promptFiles){
                        //ImGui::TextWrapped(promptFile.string().c_str());
                        if (ImGui::Selectable( promptFile.filename().string().c_str() )){
                            ImGui::OpenPopup(promptFile.filename().string().c_str());
                        }
                        
                        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

                        if (ImGui::BeginPopupModal(promptFile.filename().string().c_str()))
                        {
                            if (ImGui::Button("Close", ImVec2(work_size.x * 0.4f, 0))) { ImGui::CloseCurrentPopup(); }
                            
                            if (ImGui::BeginChild("content", ImVec2( work_size.x * 0.5f, work_size.y * 0.5f)))
                            {
                            
                        
                                std::ifstream file(promptFile);
                                if (!file) {
                                    ImGui::TextWrapped(("Cannot open the file " + promptFile.filename().string()).c_str());
                                } else {
                                    std::string fileContent;
                                    std::copy(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>(), back_inserter(fileContent));
                                    // std::string line;
                                    // while ( getline( file, line ) ) {
                                         // fileContent += line + "\n";
                                      // }
                                    
                                    ImGui::TextWrapped(fileContent.c_str());
                                }
                            }
                            ImGui::EndChild();
                            
                            ImGui::EndPopup();
                        }
                    }
                    
                    ImGui::EndChild();
                } else {
                    ImGui::TextWrapped("This is where a list of prompt files (in .txt) is shown. Open a folder with them to see the list.");
                }
                
                ImGui::EndTabItem();
            }
            
//History tab//////////////////////////////////////////////////////////////////////////
            if (ImGui::BeginTabItem("History"))
            {
                if (sessionHistory.size()){
                
                    ImGui::BeginChild("History of your prompts");
                    int totalNum = 0;
                    for (auto sV : sessionHistory){
                        
                        if (sV.second.size()){
                            ImGui::SeparatorText(sV.first.c_str());
                            for (auto msg : sV.second){
                                ImGui::TextWrapped(msg.c_str());
                                
                                if (ImGui::BeginPopupContextItem(std::to_string(totalNum).c_str()))
                                  {
                                    if (ImGui::Selectable("Use")){
                                        inputStr = msg;
                                    }
                                    
                                    if (ImGui::Selectable("Copy")){
                                        ImGui::LogToClipboard();
                                        ImGui::LogText(msg.c_str());
                                        ImGui::LogFinish();
                                    }

                                    if(templatesJson.contains("error")){
                                        if (ImGui::Selectable("Store as saved")){
                                            templatesJson["saved"].push_back(msg);
                                            templatesJson.erase("error");
                                        }
                                    } else {
                                        for (auto& [key, value] : templatesJson.items() ){
                                            if (value.is_array()) {
                                                if (ImGui::Selectable(("Store as " + key).c_str())){
                                                    templatesJson[key].push_back(msg);
                                                }
                                            }
                                        }
                                    }

                                    ImGui::EndPopup();
                                  }
                                
                                ImGui::Separator();
                                ++totalNum;
                            }
                        }
                    }
                    
                    ImGui::EndChild();
                } else {
                    ImGui::TextWrapped("No session history for now - your messages will appear here once sent.");
                }
                
                ImGui::EndTabItem();
            }          
            
//END TABS/////////////////////////////////////////////////////////////////////////////////////
            ImGui::EndTabBar();
        }
        //ImGui::PopFont();
        
        ImGui::End();
        //}

        // Rendering
        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }
#ifdef __EMSCRIPTEN__
    EMSCRIPTEN_MAINLOOP_END;
#endif

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
