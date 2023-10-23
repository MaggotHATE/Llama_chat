#pragma once

#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include <stdio.h>          // printf, fprintf
#include <stdlib.h>         // abort
#include <SDL.h>

#include "tinyfiledialogs.h"
#include <thread_chat.h>

#if defined(GGML_USE_CLBLAST)
#   define clblast 1
#else 
#   define clblast 0
#endif


///////////////////////////////////////
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

static void HelpTooltip(const char* desc)
{
    if (ImGui::BeginItemTooltip())
    {
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

static void TextWTooltip(const char* name, const char* desc)
{
    ImGui::Text(name);
    if (ImGui::BeginItemTooltip())
    {
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

// theme!

void retroTheme(){
    // Setup style
    ImGui::StyleColorsClassic();
    ImGuiStyle& style = ImGui::GetStyle();
    style.Colors[ImGuiCol_Text] = ImVec4(0.31f, 0.25f, 0.24f, 1.00f);
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.94f, 0.94f, 0.94f, 1.00f);
    style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.74f, 0.74f, 0.94f, 1.00f);
    style.Colors[ImGuiCol_ChildBg] = ImVec4(0.68f, 0.68f, 0.68f, 0.00f);
    style.Colors[ImGuiCol_Border] = ImVec4(0.50f, 0.50f, 0.50f, 0.60f);
    style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.62f, 0.70f, 0.72f, 0.56f);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.95f, 0.33f, 0.14f, 0.47f);
    style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.97f, 0.31f, 0.13f, 0.81f);
    style.Colors[ImGuiCol_TitleBg] = ImVec4(0.42f, 0.75f, 1.00f, 0.53f);
    style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.40f, 0.65f, 0.80f, 0.20f);
    style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.40f, 0.62f, 0.80f, 0.15f);
    style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.39f, 0.64f, 0.80f, 0.30f);
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.28f, 0.67f, 0.80f, 0.59f);
    style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.25f, 0.48f, 0.53f, 0.67f);
    //style.Colors[ImGuiCol_ComboBg] = ImVec4(0.89f, 0.98f, 1.00f, 0.99f);
    style.Colors[ImGuiCol_Tab] = ImVec4(0.89f, 0.98f, 1.00f, 0.99f);
    style.Colors[ImGuiCol_CheckMark] = ImVec4(0.48f, 0.47f, 0.47f, 0.71f);
    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.31f, 0.47f, 0.99f, 1.00f);
    style.Colors[ImGuiCol_Button] = ImVec4(1.00f, 0.79f, 0.18f, 0.78f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.42f, 0.82f, 1.00f, 0.81f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.72f, 1.00f, 1.00f, 0.86f);
    style.Colors[ImGuiCol_Header] = ImVec4(0.65f, 0.78f, 0.84f, 0.80f);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.75f, 0.88f, 0.94f, 0.80f);
    style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.55f, 0.68f, 0.74f, 0.80f);//ImVec4(0.46f, 0.84f, 0.90f, 1.00f);
    style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.60f, 0.60f, 0.80f, 0.30f);
    style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.00f, 1.00f, 1.00f, 0.60f);
    style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(1.00f, 1.00f, 1.00f, 0.90f);
    //style.Colors[ImGuiCol_CloseButton] = ImVec4(0.41f, 0.75f, 0.98f, 0.50f);
    //style.Colors[ImGuiCol_CloseButtonHovered] = ImVec4(1.00f, 0.47f, 0.41f, 0.60f);
    //style.Colors[ImGuiCol_CloseButtonActive] = ImVec4(1.00f, 0.16f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(1.00f, 0.99f, 0.54f, 0.43f);
    style.Colors[ImGuiCol_PopupBg] = ImVec4(0.82f, 0.92f, 1.00f, 0.90f);
    style.Alpha = 1.0f;
    //style.WindowFillAlphaDefault = 1.0f;
    style.FrameRounding = 4;
    style.IndentSpacing = 12.0f;
}

// utils for configs

static void sanitizePath(std::string& path){
    int slashes = path.rfind("\\");
    while (slashes != path.npos){
        path.replace(slashes,1,"/");
        slashes = path.rfind("\\");
    }
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

static void paramsPanelNew(gpt_params& params, int& totalThreads, ImVec2 size){
    ImGui::BeginChild("Params", size, false);
    
#if GGML_OLD_FORMAT
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
#else
        //ImGui::SliderInt("n_threads", &localSettings.n_threads, 1, 4);
        ImGui::SliderFloat("temp", &params.sparams.temp, 0.0f, 2.0f);
        if (ImGui::BeginPopupContextItem("temp"))
        {
            if (ImGui::Selectable("Reset to default")){
                params.sparams.temp = paramsDefault.sparams.temp;
            }
            ImGui::EndPopup();
        } ImGui::SameLine(); HelpMarker( ("Adjust the randomness of the generated text. Lower means more robotic. Default: " + std::to_string(paramsDefault.sparams.temp)).c_str());
        
        ImGui::SliderInt("top_k", &params.sparams.top_k, 0, 100);
        if (ImGui::BeginPopupContextItem("top_k"))
        {
            if (ImGui::Selectable("Reset to default")){
                params.sparams.top_k = paramsDefault.sparams.top_k;
            }
            ImGui::EndPopup();
        } ImGui::SameLine(); HelpMarker(("Top-k sampling. Selects the next token only from the top k most likely tokens predicted by the model. It helps reduce the risk of generating low-probability or nonsensical tokens, but it may also limit the diversity of the output. Default: " + std::to_string(paramsDefault.sparams.top_k)).c_str());
        
        ImGui::SliderFloat("top_p", &params.sparams.top_p, 0.5f, 1.0f);
        if (ImGui::BeginPopupContextItem("top_p"))
        {
            if (ImGui::Selectable("Reset to default")){
                params.sparams.top_p = paramsDefault.sparams.top_p;
            }
            ImGui::EndPopup();
        } ImGui::SameLine(); HelpMarker(("Top-p (Nucleus) sampling. Selects the next token from a subset of tokens that together have a cumulative probability of at least p. Higher values will lead to more diverse text, lower values will generate more focused and conservative text. Default: " + std::to_string(paramsDefault.sparams.top_p)).c_str());
        
        ImGui::SliderFloat("penalty_repeat", &params.sparams.penalty_repeat, 0.0f, 2.0f);
        if (ImGui::BeginPopupContextItem("penalty_repeat"))
        {
            if (ImGui::Selectable("Reset to default")){
                params.sparams.penalty_repeat = paramsDefault.sparams.penalty_repeat;
            }
            ImGui::EndPopup();
        } ImGui::SameLine(); HelpMarker(("Controls the repetition of token sequences in the generated text. Default: " + std::to_string(paramsDefault.sparams.penalty_repeat)).c_str());
        
        ImGui::SliderFloat("penalty_freq", &params.sparams.penalty_freq, 0.0f, 2.0f);
        if (ImGui::BeginPopupContextItem("penalty_freq"))
        {
            if (ImGui::Selectable("Reset to default")){
                params.sparams.penalty_freq = paramsDefault.sparams.penalty_freq;
            }
            ImGui::EndPopup();
        } ImGui::SameLine(); HelpMarker(("Descreases the chance of repeating the same lines in the output. Affects the immediate result. Default: " + std::to_string(paramsDefault.sparams.penalty_freq)).c_str());
        
        ImGui::SliderFloat("penalty_present", &params.sparams.penalty_present, 0.0f, 2.0f);
        if (ImGui::BeginPopupContextItem("penalty_present"))
        {
            if (ImGui::Selectable("Reset to default")){
                params.sparams.penalty_present = paramsDefault.sparams.penalty_present;
            }
            ImGui::EndPopup();
        } ImGui::SameLine(); HelpMarker(("Descreases the chance of repeating the same lines in the output. Affects the entrire dialog. Default: " + std::to_string(paramsDefault.sparams.penalty_present)).c_str());
        
        ImGui::SliderFloat("tfs_z", &params.sparams.tfs_z, 0.5f, 1.0f); ImGui::SameLine();
        if (ImGui::BeginPopupContextItem("tfs_z"))
        {
            if (ImGui::Selectable("Reset to default")){
                params.sparams.tfs_z = paramsDefault.sparams.tfs_z;
            }
            ImGui::EndPopup();
        } HelpMarker(("Tail free sampling, parameter z. TFS filters out logits based on the second derivative of their probabilities. Adding tokens is stopped after the sum of the second derivatives reaches the parameter z. In short: TFS looks how quickly the probabilities of the tokens decrease and cuts off the tail of unlikely tokens using the parameter z. Default: " + std::to_string(paramsDefault.sparams.tfs_z)).c_str());
        
        ImGui::SliderFloat("typical_p", &params.sparams.typical_p, 0.5f, 1.0f);
        if (ImGui::BeginPopupContextItem("typical_p"))
        {
            if (ImGui::Selectable("Reset to default")){
                params.sparams.typical_p = paramsDefault.sparams.typical_p;
            }
            ImGui::EndPopup();
        } ImGui::SameLine(); HelpMarker(("Locally typical sampling, parameter p. Promotes the generation of contextually coherent and diverse text by sampling tokens that are typical or expected based on the surrounding context. Default: " + std::to_string(paramsDefault.sparams.typical_p)).c_str());
        
        ImGui::SliderInt("mirostat", &params.sparams.mirostat, 0, 2); ImGui::SameLine();
        if (ImGui::BeginPopupContextItem("mirostat"))
        {
            if (ImGui::Selectable("Reset to default")){
                params.sparams.mirostat = paramsDefault.sparams.mirostat;
            }
            ImGui::EndPopup();
        } HelpMarker(("Uses Mirostat sampling. Top K, Nucleus, Tail Free and Locally Typical samplers are ignored with this. Default: " + std::to_string(paramsDefault.sparams.mirostat)).c_str());
        
         ImGui::SliderFloat("mirostat_tau", &params.sparams.mirostat_tau, 0.0f, 10.0f);
        if (ImGui::BeginPopupContextItem("mirostat_tau"))
        {
            if (ImGui::Selectable("Reset to default")){
                params.sparams.mirostat_tau = paramsDefault.sparams.mirostat_tau;
            }
            ImGui::EndPopup();
        } ImGui::SameLine(); HelpMarker(("Mirostat learning rate. Default: " + std::to_string(paramsDefault.sparams.mirostat_tau)).c_str());
        
        ImGui::SliderFloat("mirostat_eta", &params.sparams.mirostat_eta, 0.00f, 0.50f);
        if (ImGui::BeginPopupContextItem("mirostat_eta"))
        {
            if (ImGui::Selectable("Reset to default")){
                params.sparams.mirostat_eta = paramsDefault.sparams.mirostat_eta;
            }
            ImGui::EndPopup();
        } ImGui::SameLine(); HelpMarker(("Mirostat target entropy. Default: " + std::to_string(paramsDefault.sparams.mirostat_eta)).c_str());
        
        ImGui::SliderFloat("cfg_scale", &params.sparams.cfg_scale, 1.0f, 4.0f);
        if (ImGui::BeginPopupContextItem("cfg_scale"))
        {
            if (ImGui::Selectable("Reset to default")){
                params.sparams.cfg_scale = paramsDefault.sparams.cfg_scale;
            }
            ImGui::EndPopup();
        } ImGui::SameLine(); HelpMarker(("How strong the cfg is. High values might result in no answer generated. Default: " + std::to_string(paramsDefault.sparams.cfg_scale)).c_str());
#endif
        ImGui::SliderInt("n_threads", &params.n_threads, 1, totalThreads); ImGui::SameLine(); HelpMarker("Number of threads to use for generation, doesn't have to be maximum at the moment - try to find a sweetspot.");
        
        
        //#ifdef GGML_EXPERIMENTAL1
        ImGui::SliderInt("n_threads_batch", &params.n_threads_batch, -1, totalThreads); ImGui::SameLine(); HelpMarker("Number of threads for prompt evaluation, recommended to set to maximum.");
        //#endif
        // ImGui::SliderFloat("cfg_smooth_factor", &localSettings.cfg_smooth_factor, 0.0f, 2.0f); ImGui::SameLine(); HelpMarker("Desfines the mix between outpus with and without cfg.");

    ImGui::EndChild();
}


static void templatesList(nlohmann::json& templatesJson, std::string& inputStr){
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

static void templatesListSelect(nlohmann::json& templatesJson, std::string& inputStr){
    ImGui::BeginChild("Templates from json file");
        for (auto& [key, value] : templatesJson.items() ){
            if (value.is_array()) {
                ImGui::SeparatorText(key.c_str());
                for (auto& element : value) {
                  
                  //HelpMarker(element.get<std::string>().c_str());
                  //ImGui::SameLine();
                  if (ImGui::Selectable(element.get<std::string>().c_str()))
                      inputStr = element.get<std::string>();
                  HelpTooltip(element.get<std::string>().c_str());
                }
            }
        }
        
    ImGui::EndChild();
}

static std::string openFile(const char* const* filterPatterns){
    std::string currPath = filesystem::current_path().string() + '/';
    auto getFileName = tinyfd_openFileDialog("Select a file...", currPath.c_str(),1, filterPatterns, NULL,0);
    if (getFileName) {
        std::string result = getFileName;
        
        
        
        return result;
    }
    return "NULL";
}

static std::string openGrammar(){
    char const *FilterPatterns[1]={"*.gbnf"};
    std::string currPath = filesystem::current_path().string() + '\\';
    auto getFileName = tinyfd_openFileDialog("Select a grammar file...", currPath.c_str(),1, FilterPatterns, NULL,0);
    if (getFileName) {
        std::string result = getFileName;
        sanitizePath(result);
        return result;
    }
    return "";
}

static void addStyling(){
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
}

struct chatUI{
    int width = 600;
    int height = 800;
    float fontSize = 21.0f;
    int latency = 30;
    std::string fontFile = "DroidSans.ttf";
    std::string fontEmojisFile = "GikodotemojiRegular-EaBr4.ttf";
    std::string modelsFolderName = "NULL";
    std::string promptsFolderName = "NULL";
    nlohmann::json configJson = getJson("chatConfig.json");
    
    std::string windowLable = "Llama.cpp chat (gguf format)";
    
    bool show_demo_window = false;
    bool show_another_window = true;
    bool my_tool_active = true;
    bool chatMode = true;
    bool scrolled = true;
    bool cancelled = false;
    bool copy = false;
    bool autoscroll = true;
    
    bool use_work_area = true;
    
    
    configurableChat localSettings;
    
    
    #if GGML_OLD_FORMAT
    char const *modelFilterPatterns[1]={"*.bin"};
    #else
    char const *modelFilterPatterns[2]={"*.gguf","*.bin"};
    #endif

    char const *instructFilterPatterns[1]={"*.txt"};
    char const *fontFilterPatterns[1]={"*.ttf"};
    std::string currPath = filesystem::current_path().string() + '/';
    
    modelThread newChat;
    
    std::vector<std::pair<std::string, std::string>> localResultPairs;
    
    nlohmann::json templatesJson = getJson("templates.json");
    
    std::map<std::string, std::vector<std::string>> sessionHistory;
    
    
    bool copiedDialog = false;
    bool copiedSettings = false;
    bool copiedTimings = false;
    bool initTokens = false;
    bool hasModel = false;
    int totalThreads = get_num_physical_cores();
    
    int n_ctx_idx = 0;
    int tokens_this_session = 0;
    int consumed_this_session = 0;
    int past_this_session = 0;
    
    
    std::string inputStr = "";
    std::string inputPrompt = "";
    std::string inputAntiprompt = "";
    std::string shortModelName = "Model isn't loaded yet.";
    std::string inputAntiCFG = "";
    
    std::string helpLabel = "Your question";
    std::string aTiming = "Not calculated yet...";
    std::string output = "...";
    std::string inputGrammar = "";
    
    std::string textFileContents = "";
    
    
    int maxString = 3000;
    int messageNum = 0;
    
    int messageWidth = width;
    
    ImGuiWindowFlags chat_window_flags = ImGuiWindowFlags_MenuBar|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoCollapse|ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoResize;
    
    void setLable(){
#if GGML_OLD_FORMAT
        windowLable = "Llama.cpp chat (ggmlv3 format)";
#endif
        
#if GGML_USE_CLBLAST
        windowLable += ": CLBLAST";
#elif GGML_USE_VULKAN
        windowLable += ": VULKAN";
#endif
    }
    
    void readFromConfigJson(){
        if (!configJson["error"].is_string()){
            if (configJson["latency"].is_number()) latency = configJson["latency"].get<int>();
            if (configJson["width"].is_number()) width = configJson["width"].get<int>();
            if (configJson["height"].is_number()) height = configJson["height"].get<int>();
            if (configJson["font"].is_string()) fontFile = configJson["font"].get<std::string>();
            if (configJson["fontEmojis"].is_string()) fontEmojisFile = configJson["fontEmojis"].get<std::string>();
            if (configJson["fontSize"].is_number()) fontSize = configJson["fontSize"].get<float>();
            if (configJson.contains("modelsFolder") && configJson["modelsFolder"].is_string()) modelsFolderName = configJson["modelsFolder"].get<std::string>();
            if (configJson.contains("promptsFolder") && configJson["promptsFolder"].is_string()) promptsFolderName = configJson["promptsFolder"].get<std::string>();
            //configJson.erase("error");
        }
    }
    
    void settingsTab(){
        
        if (newChat.loaded == 9) {

            if (copiedSettings){
                ImGui::TextWrapped( ("SEED: " + std::to_string(localSettings.params.seed)).c_str() );
               //ImGui::BeginChild("PreData");
                ImGui::TextWrapped( ("Base (initial) prompt: " + localSettings.params.prompt).c_str() ); ImGui::SameLine(); HelpMarker("This serves as the basis for the dialog, providing context. Make sure it ends with the antiptrompt if you don't want the NN to converse endlessly.");
                ImGui::Separator();
               //ImGui::EndChild();
                ImGui::TextWrapped( ("Antiprompt: " + localSettings.params.antiprompt[0]).c_str() ); ImGui::SameLine(); HelpMarker("This serves as a stop-word for the NN to be able to end generation and wait for your input.");
                ImGui::Separator();
#if GGML_OLD_FORMAT
                ImGui::TextWrapped( ("CFG (negative) prompt: " + localSettings.params.cfg_negative_prompt).c_str() ); 
#else
                ImGui::TextWrapped( ("CFG (negative) prompt: " + localSettings.params.sparams.cfg_negative_prompt).c_str() ); 
#endif
                ImGui::SameLine(); HelpMarker("If cfg_scale > 1.0, CFG negative prompt is used for additional guiding.");
                ImGui::Separator();
                
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
        
            if (copiedTimings == false) {
                aTiming = newChat.lastTimings;
                copiedTimings = true;
            }
        
            ImGui::TextWrapped(aTiming.c_str());
            //ImGui::TextWrapped((newChat.lastTimings).c_str());
        
        ImGui::EndChild();
                
                
    }
    
    void jsonTab(){
        if (ImGui::Button("Reset config")) {
            localSettings.getFromJson("config.json");
            localSettings.fillLocalJson();
            localSettings.updateDump();
        }
        ImGui::SameLine();
        if (ImGui::Button("Update config")) {
            localSettings.updateDump();
        }
        ImGui::SameLine();
        if (ImGui::Button("Save config"))   { 
            localSettings.pushToMainConfig();
            writeJson(localSettings.localConfig, "config.json");
             
        }
        
        ImGui::BeginChild("Parameters from json file");
            
            ImGui::TextWrapped(localSettings.localJsonDump.c_str());
            
        ImGui::EndChild();
    }
    
    void templatesTab(){
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
    }
    
    void promptFilesTab(const ImGuiViewport* viewport){
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
            
            ImVec2 center = viewport->GetCenter();
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
    }
    
    void historyTab(){
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
    }
    
    
    // separating this into more functions breaks "realtime" speed display
    void dialogTab(const ImGuiViewport* viewport){
        ImGui::BeginChild("Dialog tab");
        messageWidth = ImGui::GetWindowWidth();
// staring main loop once loaded the model//////////////////////////////////////////////////////////
        if (localSettings.noConfig){
            ImGui::TextWrapped("No model config file found!");
        }
        
        if (newChat.loaded == 9) {
            
            shortModelName = newChat.shortModelName;
            
            if (!initTokens){
                tokens_this_session = newChat.last_tokens;
                consumed_this_session = newChat.consumed_tokens;
                past_this_session = newChat.past_tokens;
                
                initTokens = true;
            }
            
            if (!copiedSettings){
                localSettings.getSettings(newChat.newChat);
                helpLabel = localSettings.params.antiprompt[0];
                copiedSettings = true;
            }

            {
/////// rendering dialogs ////////////////////////////////////////////////////////////////////////
                ImGui::BeginChild("Dialog", ImVec2( messageWidth * 0.99f, ImGui::GetContentRegionAvail().y*0.75f), false);
                ImGui::Indent();
                    // to avoid mutexes we only use a copy of dialog DB for UI
                    if (newChat.isContinue == 'i') {
                        if (!copiedDialog){
                            localResultPairs = newChat.resultsStringPairs;
                            ImGui::SetScrollFromPosY(ImGui::GetCursorPos().y + ImGui::GetContentRegionAvail().y, 0.25f);
                            copiedDialog = true;
                            copiedTimings = false;
                            //aTiming = newChat.lastTimings;
                            helpLabel = " ";
                            
                        }
                    }


                    messageNum = 0;
                    for (auto& r : localResultPairs){
                        ++messageNum;
                        if (r.first == "AI"){
////////////////////generated
                            //std::cout << r.second;
                            //ImGui::TextWrapped(r.second.c_str());
                            if (chatMode) ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + messageWidth * 0.5f);
                            else {
                                if (messageNum > 1) ImGui::SeparatorText("answer");
                                ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + ImGui::GetContentRegionAvail().x);
                            }
                            
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
                                    
                                    if (ImGui::Selectable("Save on disk")){
                                        auto saveAnswer = tinyfd_saveFileDialog( std::to_string(messageNum).c_str() , currPath.c_str() , 1 , instructFilterPatterns, NULL);
                
                                        if (saveAnswer){
                                            writeTextFileOver(saveAnswer, r.second);
                                        }
                                    }
                                    ImGui::EndPopup();
                                }
                                // ImGui::SameLine();
                                // if (ImGui::Button("W")) {
                                    
                                // }
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
                                    if (ImGui::BeginPopupContextItem((std::to_string(messageNum)+"_s").c_str()))
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
                            
                            // if (ImGui::BeginPopupContextItem(std::to_string(messageNum).c_str()))
                            // {
                                // if (ImGui::Selectable("Copy")){
                                    // ImGui::LogToClipboard();
                                    // ImGui::LogText(r.second.c_str());
                                    // ImGui::LogFinish();
                                // }
                                // ImGui::EndPopup();
                            // }
                            
                            ImGui::PopTextWrapPos();
                            
                            if (messageNum == 1 && chatMode){
                                ImGui::SeparatorText("Dialog");
                            }
                            //ImGui::PushItemWidth(-ImGui::GetContentRegionAvail().x * 0.5f);
                            //ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.5f);
                            //ImGui::Text(r.second.c_str(), wrap_width);
                            //ImGui::PopTextWrapPos();
                            //ImGui::InputTextMultiline(" ", &r.second, ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 2), ImGuiInputTextFlags_ReadOnly);
                        } else {
////////////////////////prompts
                            //if (chatMode) ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x * 0.35f);
                            
                            //std::cout << r.first << r.second;
                            //std::string questionStr = r.first + r.second;
                            //ImGui::SeparatorText(r.first.c_str());
                            if (chatMode) ImGui::SetCursorPosX(ImGui::GetCursorPosX() + messageWidth * 0.40f);
                            else ImGui::SeparatorText("prompt");
                            
                            ImGui::TextWrapped((r.first + r.second).c_str());
                            if (ImGui::BeginPopupContextItem(std::to_string(messageNum).c_str()))
                            {
                                if (ImGui::Selectable("Copy")){
                                    ImGui::LogToClipboard();
                                    ImGui::LogText((r.second.substr(0,r.second.size())).c_str());
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
                    
//////////////// streaming dialog token-after-token
                    if (newChat.isContinue == 'w') {
                        
                        //ImGui::TextWrapped("...");
                        if (chatMode) ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + messageWidth * 0.50f);
                        else {
                            ImGui::SeparatorText("answer");
                            ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + ImGui::GetContentRegionAvail().x);
                        }    
                        output = newChat.lastResult;
                        //copiedTimings = false;
                        //aTiming = newChat.lastTimings;
                        
                        if (output.size() < maxString){
                            ImGui::Text(output.c_str());
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
                        if (autoscroll){
                            scrolled = false;
                            if (!scrolled) {
                                ImGui::SetScrollFromPosY(ImGui::GetCursorPos().y + ImGui::GetContentRegionAvail().y, 0.25f);
                                scrolled = true;
                            }
                        }
                        //ImGui::SeparatorText(("Generating " + std::to_string(newChat.lastSpeed) + " t/s").c_str());
                        if (newChat.isPregen != 'i') ImGui::SeparatorText(std::format("Reading at {:.2f} t/s", newChat.lastSpeedPrompt).c_str());
                        else ImGui::SeparatorText(std::format("Read at {:.2f} t/s, generating at {:.2f} t/s", newChat.lastSpeedPrompt, newChat.lastSpeed).c_str());
                        
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
                ImGui::Unindent();
                ImGui::EndChild();
                
            }
// INPUT ////////////////////////////////////////////////////////////////////////////////////////
            //ImGui::InputTextMultiline("input text", inputStr, IM_ARRAYSIZE(inputStr));
            
            //static ImGuiInputTextFlags flags = ImGuiInputTextFlags_AllowTabInput;
            //ImGui::InputTextMultiline(helpLabel.c_str(), &inputStr, ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16), flags);
            
            
            //static char inputChars[24 * 16];
            //ImGui::InputTextMultiline(helpLabel.c_str(), inputChars, IM_ARRAYSIZE(inputChars));
            auto inputWidth = ImGui::GetContentRegionAvail().x * 0.75f;
            // still need to make input field wrapping
            //ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + ImGui::GetContentRegionAvail().x * 0.65f);
            ImGui::InputTextMultiline("Suffix", &localSettings.params.input_suffix, ImVec2(inputWidth, ImGui::GetTextLineHeight() * 3)); ImGui::SameLine(); HelpMarker( "Suffix is added after your prompt - can be used to instantly set the charater for NN." );  
            if (newChat.isNewSuffix(localSettings.params.input_suffix)){ 
                ImGui::SameLine();
                if (ImGui::Button("Apply")) { newChat.applySuffix(localSettings.params.input_suffix); } 
            }
           ImGui::BeginChild("inputAndSendButtons", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y));
            
            ImGui::InputTextMultiline(helpLabel.c_str(), &inputStr, ImVec2(inputWidth, ImGui::GetContentRegionAvail().y));
            //ImGui::PopTextWrapPos();
            
            ImGui::SameLine();
            
            ImGui::BeginChild("sendButtons", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y));
            // we don't need to show the button if generation is going
            if (newChat.isContinue != 'w') {
            
                
                if (ImGui::Button("Send")) {
                    if (textFileContents.size()) {
                        inputStr += '"' + textFileContents + '"';
                        textFileContents.clear();
                    }
                    
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
                    newChat.getResultAsyncStringFull2(false, true);
                    copiedDialog = false;
                    copiedTimings = false;
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
                
                if (ImGui::Button("Choose a text file")) {
                    auto textFile = tinyfd_openFileDialog("Select a text file...", currPath.c_str(),1, instructFilterPatterns, NULL,0);
                        if (textFile) {
                            textFileContents.clear();
                            std::ifstream file(textFile);
                            if (file) {
                                std::copy(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>(), back_inserter(textFileContents));
                            }
                        }
                }
                
                if (textFileContents.size()) {
                    if (ImGui::Button("Clear text file")) {
                        textFileContents.clear();
                    }
                    
                    
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
           ImGui::EndChild();
            
        } else {
// Initial buttons and settings to load a model////////////////////////////////////////////////////
            if (newChat.isContinue == '_') {
                firstSettings(viewport);
              
            } else {
// Information while  LOADING////////////////////////////////////////////////////////////////////
                loadingIndication();
            }
        }
        ImGui::EndChild();
    }
    
    void firstSettings(const ImGuiViewport* viewport){
        ImGui::BeginChild("InitSettings");
              
        if(modelsFolderName != "NULL") {
            ImGui::TextWrapped(( "Models folder: " + modelsFolderName).c_str());
            ImGui::Separator();
        }
        
        //if (!localSettings.params.input_prefix.empty()) ImGui::TextWrapped(( "Prefix: " + localSettings.params.input_prefix).c_str());
        //if (!localSettings.params.input_suffix.empty()) ImGui::TextWrapped(( "input_suffix: " + localSettings.params.input_suffix).c_str());
        
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

        
        if (!localSettings.checkInputPrompt()) ImGui::TextWrapped( ("Set prompt to " + localSettings.inputPrompt).c_str() );
        
        if (!localSettings.checkInputAntiprompt()) ImGui::TextWrapped( ("Set antiprompt to " + localSettings.inputAntiprompt).c_str() );
        
        if (!localSettings.checkInputAntiCFG()) ImGui::TextWrapped( ("Set CFG antiprompt to " + localSettings.inputAntiCFG).c_str() );
#if GGML_OLD_FORMAT
        if (localSettings.params.cfg_scale > 1.0) {
            ImGui::TextWrapped( ("Set CFG cfg_scale to " + std::to_string(localSettings.params.cfg_scale)).c_str() );
            ImGui::Separator();
        }
#else
        if (localSettings.params.sparams.cfg_scale > 1.0) {
            ImGui::TextWrapped( ("Set CFG cfg_scale to " + std::to_string(localSettings.params.sparams.cfg_scale)).c_str() );
            ImGui::Separator();
        }
#endif
        ImGui::Unindent();
        
        
        ImGui::Spacing();
        
        if (ImGui::Button("Apply to config")) {
            //localSettings.getFromJson("config.json");
            localSettings.grammarFile = inputGrammar; 
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
                    if (std::filesystem::exists(localSettings.modelConfig["model"])){
                        newChat.jsonConfig = localSettings.modelConfig;
                        
                        newChat.load();
                        
                        hasModel = true;
                        copiedDialog = false;
                        copiedSettings = false;
                        newChat.isContinue = 'l';
                    } else {
                        ImGui::OpenPopup("No Model");
                    }
                }
            } else {
                if (ImGui::Button("Init model")) {

                    copiedDialog = false;
                    copiedSettings = false;
                    copiedTimings = false;
                    //load(newChat, localSettings.localConfig, hasModel);
                    newChat.load(localSettings.modelConfig, hasModel);
                    //load_task(newChat, localSettings, hasModel);
                    newChat.isContinue = 'l';
                }
            }
            
            if (ImGui::BeginPopup("No Model")) {
                ImVec2 work_size = viewport->WorkSize;
                ImGui::BeginChild("Prompts list", ImVec2(  work_size.x * 0.5, work_size.y * 0.1));
                std::string noModelMsg = localSettings.modelConfig["model"].get<std::string>() + " doesn't exist!";
                ImGui::TextWrapped( noModelMsg.c_str() );
                ImGui::EndChild();
                ImGui::EndPopup();
                
            }
        } else {
            if(localSettings.modelName == "NULL") ImGui::TextWrapped( "Choose a model first!" );
            if(localSettings.inputPrompt == "NULL") ImGui::TextWrapped( "Add prompt first! Also, add antiprompt if you don't want endless NN conversation." );
            
        }
        
        
        ImGui::Separator();
        ImGui::Spacing();
        
      //ImGui::BeginChild("InitSettings");
        ImGui::TextWrapped( "Below you can set up prompt and antiprompt instead of preconfigured ones." );                        
        float initWidth = viewport->WorkSize.x * 0.65f;
        if (localSettings.inputInstructFile == "NULL"){
            
            
            if (localSettings.instructFileFromJson != "NULL") ImGui::TextWrapped( "This model has an instruct file set for it in config. Prompt and antiprompt will not be used!" );
            
            ImGui::InputTextMultiline("Prompt", &inputPrompt, ImVec2(initWidth, ImGui::GetTextLineHeight() * 6)); ImGui::SameLine(); HelpMarker( "Prompt can be used as a start of the dialog, providing context and/or format of dialog." ); 
            if (ImGui::Button("Apply prompt")) {
                    localSettings.inputPrompt = inputPrompt;
                } ImGui::SameLine(); if (ImGui::Button("Clear prompt")) {
                    localSettings.inputPrompt = "NULL";
            }
            ImGui::SameLine();
            //ImGui::BeginChild("Dialog", ImVec2( ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y*0.2f), false);
            if (ImGui::Button("Open an instruct file...")) {
                
                auto inputInstructFile = tinyfd_openFileDialog("Select an instruct file...", currPath.c_str(),1, instructFilterPatterns, NULL,0);
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
            
            ImVec2 center = viewport->GetCenter();
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
        
        ImGui::InputTextMultiline("Antiprompt", &inputAntiprompt, ImVec2(initWidth, ImGui::GetTextLineHeight() * 2)); ImGui::SameLine(); HelpMarker( "Antiprompt is needed to control when to stop generating and wait for your input." ); 
        if (ImGui::Button("Apply antiprompt")) {
                    localSettings.inputAntiprompt = inputAntiprompt;
                } ImGui::SameLine(); if (ImGui::Button("Clear antiprompt")) {
                    localSettings.inputAntiprompt = "NULL";
                }
        ImGui::InputText("Grammar file", &inputGrammar); ImGui::SameLine(); HelpMarker( "Grammars are more strict rules that help fomratting teh dialog." );        
        if (ImGui::Button("Choose grammar")) {
                inputGrammar = openGrammar();
            }
        ImGui::InputText("Prefix", &localSettings.params.input_prefix); ImGui::SameLine(); HelpMarker( "Prefix sets your character for each input." );        
                
        ImGui::InputTextMultiline("Suffix", &localSettings.params.input_suffix, ImVec2(initWidth, ImGui::GetTextLineHeight() * 3)); ImGui::SameLine(); HelpMarker( "Suffix is added after your prompt - can be used to instantly set the charater for NN." );         
                
        ImGui::InputTextMultiline("CFG Antiprompt", &inputAntiCFG, ImVec2(initWidth, ImGui::GetTextLineHeight() * 5)); ImGui::SameLine(); HelpMarker( "CFG Antiprompt is used to guide the output by defining what should NOT be in it. cfg_scale must be higher than 1.0 to activate CFG." );
        if (ImGui::Button("Apply CFG antiprompt")) {
                    localSettings.inputAntiCFG = inputAntiCFG;
                } ImGui::SameLine(); if (ImGui::Button("Clear CFG antiprompt")) {
                    localSettings.inputAntiCFG = "NULL";
                }
#if GGML_OLD_FORMAT
        ImGui::SliderFloat("cfg_scale", &localSettings.params.cfg_scale, 1.0f, 4.0f); ImGui::SameLine(); HelpMarker("How strong the cfg is. High values might result in no answer generated.");

#else 
        ImGui::SliderFloat("cfg_scale", &localSettings.params.sparams.cfg_scale, 1.0f, 4.0f); ImGui::SameLine(); HelpMarker("How strong the cfg is. High values might result in no answer generated.");
#endif
        ImGui::SliderInt("n_threads", &localSettings.params.n_threads, 1, totalThreads); ImGui::SameLine(); HelpMarker("Number of threads to use for generation, doesn't have to be maximum at the moment - try to find a sweetspot.");
        
        
        //#ifdef GGML_EXPERIMENTAL1
        ImGui::SliderInt("n_threads_batch", &localSettings.params.n_threads_batch, -1, totalThreads); ImGui::SameLine(); HelpMarker("Number of threads for prompt evaluation, recommended to set to maximum.");
        //#endif
#if GGML_USE_CLBLAST || GGML_USE_VULKAN
            ImGui::SliderInt("n_gpu_layers", &localSettings.params.n_gpu_layers, 0, 100); ImGui::SameLine(); HelpMarker("Number of layers to offload onto GPU.");
#endif
        
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
        
        
        
        
      ImGui::EndChild();
    }
    
    void loadingIndication(){
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
    
    void mainUI(const ImGuiViewport* viewport, SDL_Window* window){
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
                                copiedTimings = false;
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
                                copiedTimings = false;
                                newChat.unload();
                            }
                        }
                    }
                }
                if (ImGui::MenuItem("Select models folder", "Ctrl+S"))   {
                     auto getModelFolderName = tinyfd_selectFolderDialog("Select a model...", currPath.c_str());
                     if (getModelFolderName) {
                         modelsFolderName = getModelFolderName;
                         modelsFolderName += "\\";
                     }
                }
                
                if (ImGui::MenuItem("Select prompts folder", "Ctrl+S"))   { 
                     auto getPromptsFolderName = tinyfd_selectFolderDialog("Select a model...", currPath.c_str());
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
                                   copiedTimings = false;

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
        
        ImGui::BeginChild("Styles", ImVec2( ImGui::GetContentRegionAvail().x * 0.4f, fontSize + 6.0f));
        static int style_idx = 3;
        if (ImGui::Combo("", &style_idx, "Dark theme\0Light theme\0Classic theme\0Retro theme\0", 4))
        {
            switch (style_idx)
            {
                case 0: ImGui::StyleColorsDark(); break;
                case 1: ImGui::StyleColorsLight(); break;
                case 2: ImGui::StyleColorsClassic(); break;
                case 3: retroTheme(); break;
            }
        }
        
        
        //ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
        ImGui::SameLine();
        
        ImGui::Checkbox("Chat mode", &chatMode); 
        
        ImGui::EndChild();
        
        
        
        ImGui::SameLine();
        
        ImGui::BeginChild("Name", ImVec2( ImGui::GetContentRegionAvail().x * 0.60f, fontSize + 6.0f));
        //ImGui::Text((shortModelName).c_str());
        ImGui::Checkbox("Autoscroll", &autoscroll); 
        
        //this doesn't seem to do anything, WIP
        // if (newChat.loaded == 9 && newChat.isContinue != 'w'){
            // ImGui::SameLine();
            // if (ImGui::Button("Soft restart")){ 
                // newChat.reset();
                // newChat.microload();
                // copiedDialog = false;
            // }
        // }
        ImGui::EndChild();
        ImGui::SameLine();
        
        ImGui::BeginChild("Buttons", ImVec2( ImGui::GetContentRegionAvail().x, fontSize + 6.0f));
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
                
                
                
                if (ImGui::Button("Restart")){
                    //safeguard
                    if (newChat.loaded != 0){
                        newChat.loaded = 0;
                        tokens_this_session = 0;
                        copiedDialog = false;
                        copiedSettings = false;
                        copiedTimings = false;
                        initTokens = false;
                        cancelled = false;

                        hasModel = false;
                        newChat.unload();
                    }
                }
                ImGui::SameLine();
                ImGui::TextWrapped(("Tokens: " + std::to_string(past_this_session)).c_str());
            }  else {
                //ImGui::SameLine();
                
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
        
        ImGui::EndChild();
        
        //ImGui::Separator();
        //ImGui::Spacing();
        //ImGui::Text("Test");
        
        //ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
        if (ImGui::BeginTabBar("MyTabBar", ImGuiTabBarFlags_None))
        {
// Chat tab (initial info and loading, or chat display)/////////////////////////////////////////////
            if (ImGui::BeginTabItem("Chat"))
            {
                dialogTab(viewport);
                ImGui::EndTabItem();
            }
//SETTINGS TAB///////////////////////////////////////////////////////////////////////////////////
            if (ImGui::BeginTabItem("Model parameters"))
            {
                ImGui::BeginChild("PreData");
                settingsTab();
                ImGui::EndChild();
                
                ImGui::EndTabItem();
            }
//Configuration json tab//////////////////////////////////////////////////////////////////////////
            if (ImGui::BeginTabItem("Current config"))
            {
                jsonTab();
                
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
                templatesTab();
                
                ImGui::EndTabItem();
            }

            //ImGui::EndTabBar();
        
//Prompt files tab//////////////////////////////////////////////////////////////////////////
            if (ImGui::BeginTabItem("Prompt files"))
            {
                promptFilesTab(viewport);
                
                ImGui::EndTabItem();
            }
            
//History tab//////////////////////////////////////////////////////////////////////////
            if (ImGui::BeginTabItem("History"))
            {
                historyTab();
                
                ImGui::EndTabItem();
            }          
            
//END TABS/////////////////////////////////////////////////////////////////////////////////////
            ImGui::EndTabBar();
        }

        
        
        ImGui::End();
        
        
        
    }
    
    void init(){
        localSettings.modelFromJson = getStringFromJson("config.json","model");
        if (promptsFolderName != "NULL") localSettings.promptFilesFolder = promptsFolderName;
        
        localSettings.getFilesList();
        localSettings.getFromJson("config.json");
        localSettings.getSettingsFull();
        localSettings.fillLocalJson();
        localSettings.updateDump();
        
        inputPrompt = localSettings.params.prompt;
        
        n_ctx_idx = sqrt( localSettings.params.n_ctx / 2048 );

        if(localSettings.params.antiprompt.size()) inputAntiprompt = localSettings.params.antiprompt[0];
#if GGML_OLD_FORMAT
        inputAntiCFG = localSettings.params.cfg_negative_prompt;
#else
        inputAntiCFG = localSettings.params.sparams.cfg_negative_prompt;
#endif
    }
    
};