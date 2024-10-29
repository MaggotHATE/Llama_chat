#pragma once

#include "tinyfiledialogs.h"
#include <threads_layer.h>

#if defined(GGML_USE_CLBLAST)
#   define clblast 1
#else 
#   define clblast 0
#endif

#if defined(SDL2)
#   include "backend_sdl2.h"
#else 
#   include "backend_vk.h"
#endif

#include "misc/cpp/imgui_stdlib.h"


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
    style.FrameRounding = 1;
    style.IndentSpacing = 12.0f;
}

void StyleColorsLightNew()
{
    ImGui::StyleColorsLight();
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    colors[ImGuiCol_Text]                   = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_TextDisabled]           = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
    colors[ImGuiCol_WindowBg]               = ImVec4(0.94f, 0.94f, 0.94f, 1.00f);
    colors[ImGuiCol_ChildBg]                = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_PopupBg]                = ImVec4(1.00f, 1.00f, 1.00f, 0.98f);
    colors[ImGuiCol_Border]                 = ImVec4(0.00f, 0.00f, 0.00f, 0.30f);
    colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg]                = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
    colors[ImGuiCol_FrameBgActive]          = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    colors[ImGuiCol_TitleBg]                = ImVec4(0.96f, 0.96f, 0.96f, 1.00f);
    colors[ImGuiCol_TitleBgActive]          = ImVec4(0.82f, 0.82f, 0.82f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(1.00f, 1.00f, 1.00f, 0.51f);
    colors[ImGuiCol_MenuBarBg]              = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
    colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.98f, 0.98f, 0.98f, 0.53f);
    colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.69f, 0.69f, 0.69f, 0.80f);
    colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.49f, 0.49f, 0.49f, 0.80f);
    colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.49f, 0.49f, 0.49f, 1.00f);
    colors[ImGuiCol_CheckMark]              = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_SliderGrab]             = ImVec4(0.26f, 0.59f, 0.98f, 0.78f);
    colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.46f, 0.54f, 0.80f, 0.60f);
    colors[ImGuiCol_Button]                 = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
    colors[ImGuiCol_ButtonHovered]          = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_ButtonActive]           = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
    colors[ImGuiCol_Header]                 = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
    colors[ImGuiCol_HeaderHovered]          = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
    colors[ImGuiCol_HeaderActive]           = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_Separator]              = ImVec4(0.39f, 0.39f, 0.39f, 0.62f);
    colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.14f, 0.44f, 0.80f, 0.78f);
    colors[ImGuiCol_SeparatorActive]        = ImVec4(0.14f, 0.44f, 0.80f, 1.00f);
    colors[ImGuiCol_ResizeGrip]             = ImVec4(0.35f, 0.35f, 0.35f, 0.17f);
    colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
    colors[ImGuiCol_TabHovered]             = colors[ImGuiCol_HeaderHovered];
    colors[ImGuiCol_PlotLines]              = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered]       = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    colors[ImGuiCol_PlotHistogram]          = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.00f, 0.45f, 0.00f, 1.00f);
    colors[ImGuiCol_TableHeaderBg]          = ImVec4(0.78f, 0.87f, 0.98f, 1.00f);
    colors[ImGuiCol_TableBorderStrong]      = ImVec4(0.57f, 0.57f, 0.64f, 1.00f);   // Prefer using Alpha=1.0 here
    colors[ImGuiCol_TableBorderLight]       = ImVec4(0.68f, 0.68f, 0.74f, 1.00f);   // Prefer using Alpha=1.0 here
    colors[ImGuiCol_TableRowBg]             = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_TableRowBgAlt]          = ImVec4(0.30f, 0.30f, 0.30f, 0.09f);
    colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
    colors[ImGuiCol_DragDropTarget]         = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
    colors[ImGuiCol_NavHighlight]           = colors[ImGuiCol_HeaderHovered];
    colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(0.70f, 0.70f, 0.70f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg]      = ImVec4(0.20f, 0.20f, 0.20f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);
}

// utils for configs

// static void sanitizePath(std::string& path){
    // int slashes = path.rfind("\\");
    // while (slashes != path.npos){
        // path.replace(slashes,1,"/");
        // slashes = path.rfind("\\");
    // }
// }

std::string getStringFromJson(std::string fileName, std::string stringName){
    nlohmann::json config;
    std::fstream o1(fileName);
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

static void buttonOnTextChanges(std::string& stringBase, std::string& stringChanged) {
    if (stringBase != stringChanged) {
        if (ImGui::Button("Apply")) {
            stringBase = stringChanged;
        }
    }
}

static void reserveEmpty(std::string& stringBase) {
    if (stringBase.empty()) {
        stringBase.reserve(1048576);
        stringBase.resize(1, ' ');
    }
}

static void sliderTemp(float& temp, float& default_temp)
{
    {
        if (ImGui::Button(" -##temp")) {
            temp -= 0.001f;
        }
        ImGui::SameLine();
        if (ImGui::Button("+ ##temp")) {
            temp += 0.001f;
        }
        ImGui::SameLine();
    }
    ImGui::SliderFloat("temp", &temp, 0.0f, 5.0f);
    if (ImGui::BeginPopupContextItem("temp"))
    {
        if (ImGui::Selectable("Reset to default")){
            temp = default_temp;
        }
        ImGui::EndPopup();
    } ImGui::SameLine(); HelpMarker( ("Defines randomness of generated text. Lower means more robotic. Default: " + std::to_string(default_temp)).c_str());
}

static void sliderTempSmoothing(float& temp_smoothing, float& default_temp_smoothing)
{
    {
        if (ImGui::Button(" -##temp_smoothing")) {
            temp_smoothing -= 0.001f;
        }
        ImGui::SameLine();
        if (ImGui::Button("+ ##temp_smoothing")) {
            temp_smoothing += 0.001f;
        }
        ImGui::SameLine();
    }
    ImGui::SliderFloat("temp_smoothing", &temp_smoothing, 0.0f, 10.0f);
    if (ImGui::BeginPopupContextItem("temp_smoothing"))
    {
        if (ImGui::Selectable("Reset to default")){
            temp_smoothing = default_temp_smoothing;
        }
        ImGui::EndPopup();
    } ImGui::SameLine(); HelpMarker( ("Add more smooth temperature rise for better creativity. Default: " + std::to_string(default_temp_smoothing)).c_str());
}

static void sliderTempSmoothingCurve(float& smoothing_curve, float& default_smoothing_curve)
{
    {
        if (ImGui::Button(" -##smoothing_curve")) {
            smoothing_curve -= 0.001f;
        }
        ImGui::SameLine();
        if (ImGui::Button("+ ##smoothing_curve")) {
            smoothing_curve += 0.001f;
        }
        ImGui::SameLine();
    }
    ImGui::SliderFloat("smoothing_curve", &smoothing_curve, 0.0f, 10.0f);
    if (ImGui::BeginPopupContextItem("smoothing_curve"))
    {
        if (ImGui::Selectable("Reset to default")){
            smoothing_curve = default_smoothing_curve;
        }
        ImGui::EndPopup();
    } ImGui::SameLine(); HelpMarker( ("Controls the curve of smooth temperature. Default: " + std::to_string(default_smoothing_curve)).c_str());
}

static void sliderDynatemp_range(float& dynatemp_range, float& temp, float& default_dynatemp_range)
{
    {
        if (ImGui::Button(" -##dynatemp_range")) {
            dynatemp_range -= 0.001f;
        }
        ImGui::SameLine();
        if (ImGui::Button("+ ##dynatemp_range")) {
            dynatemp_range += 0.001f;
        }
        ImGui::SameLine();
    }
    ImGui::SliderFloat("dynatemp_range", &dynatemp_range, 0.0f, 5.0f);
    if (ImGui::BeginPopupContextItem("dynatemp_range"))
    {
        if (ImGui::Selectable("Reset to default")){
            dynatemp_range = default_dynatemp_range;
        }
        ImGui::EndPopup();
    } ImGui::SameLine(); HelpMarker( (std::format("Dynamically adjusts randomness in a range of temp +- dynatemp_range. Activated if dynatemp_range > 0. Default: {:.2f}. Current: ({:.2f} - {:.2f})",default_dynatemp_range, temp > dynatemp_range ? temp - dynatemp_range : 0, temp + dynatemp_range)).c_str());
}

static void sliderKShift(int& k_shift, int& default_k_shift)
{
    {
        if (ImGui::Button(" -##k_shift")) {
            --k_shift;
        }
        ImGui::SameLine();
        if (ImGui::Button("+ ##k_shift")) {
            k_shift++;
        }
        ImGui::SameLine();
    }
    ImGui::SliderInt("k_shift", &k_shift, 0, 100);
    if (ImGui::BeginPopupContextItem("k_shift"))
    {
        if (ImGui::Selectable("Reset to default")){
            k_shift = default_k_shift;
        }
        ImGui::EndPopup();
    } ImGui::SameLine(); HelpMarker(("Shifts the first inference of the entire dialog to k token, can help with reasosning on greedy sampling. Default: " + std::to_string(default_k_shift)).c_str());
}

static void sliderTopK(int& top_k, int& default_top_k)
{
    {
        if (ImGui::Button(" -##top_k")) {
            --top_k;
        }
        ImGui::SameLine();
        if (ImGui::Button("+ ##top_k")) {
            top_k++;
        }
        ImGui::SameLine();
    }
    ImGui::SliderInt("top_k", &top_k, 0, 100);
    if (ImGui::BeginPopupContextItem("top_k"))
    {
        if (ImGui::Selectable("Reset to default")){
            top_k = default_top_k;
        }
        ImGui::EndPopup();
    } ImGui::SameLine(); HelpMarker(("Top-k sampling. Selects the next token only from the top k most likely tokens predicted by the model. It helps reduce the risk of generating low-probability or nonsensical tokens, but it may also limit the diversity of the output. Set to 1 for greedy sampling. Default: " + std::to_string(default_top_k)).c_str());
}

static void sliderTopP(float& top_p, float& default_top_p){
    {
        if (ImGui::Button(" -##top_p")) {
            top_p -= 0.001f;
        }
        ImGui::SameLine();
        if (ImGui::Button("+ ##top_p")) {
            top_p += 0.001f;
        }
        ImGui::SameLine();
    }
    ImGui::SliderFloat("top_p", &top_p, 0.5f, 1.0f);
    if (ImGui::BeginPopupContextItem("top_p"))
    {
        if (ImGui::Selectable("Reset to default")){
            top_p = default_top_p;
        }
        ImGui::EndPopup();
    } ImGui::SameLine(); HelpMarker(("Top-p (Nucleus) sampling. Selects the next token from a subset of tokens that together have a cumulative probability of at least p. Higher values will lead to more diverse text, lower values will generate more focused and conservative text. Default: " + std::to_string(default_top_p)).c_str());
}

static void sliderRepeatPen(float& repeat_penalty, float& default_repeat_penalty){
    {
        if (ImGui::Button(" -##repeat_penalty")) {
            repeat_penalty -= 0.001f;
        }
        ImGui::SameLine();
        if (ImGui::Button("+ ##repeat_penalty")) {
            repeat_penalty += 0.001f;
        }
        ImGui::SameLine();
    }
    ImGui::SliderFloat("repeat_penalty", &repeat_penalty, 0.0f, 2.0f);
    if (ImGui::BeginPopupContextItem("repeat_penalty"))
    {
        if (ImGui::Selectable("Reset to default")){
            repeat_penalty = default_repeat_penalty;
        }
        ImGui::EndPopup();
    } ImGui::SameLine(); HelpMarker(("Controls the repetition of token sequences in the generated text. Default: " + std::to_string(default_repeat_penalty)).c_str());
}

static void sliderRepeatThresh(float& penalty_threshold, float& default_penalty_threshold){
    {
        if (ImGui::Button(" -##penalty_threshold")) {
            penalty_threshold -= 0.001f;
        }
        ImGui::SameLine();
        if (ImGui::Button("+ ##penalty_threshold")) {
            penalty_threshold += 0.001f;
        }
        ImGui::SameLine();
    }
    ImGui::SliderFloat("penalty_threshold", &penalty_threshold, 0.0f, 2.0f);
    if (ImGui::BeginPopupContextItem("penalty_threshold"))
    {
        if (ImGui::Selectable("Reset to default")){
            penalty_threshold = default_penalty_threshold;
        }
        ImGui::EndPopup();
    } ImGui::SameLine(); HelpMarker(("Controls the repetition of most frequently use tokes, excluding spaces, punctuations, etc. Default: " + std::to_string(default_penalty_threshold)).c_str());
}

static void sliderFrequencyPen(float& frequency_penalty, float& default_frequency_penalty){
    {
        if (ImGui::Button(" -##frequency_penalty")) {
            frequency_penalty -= 0.001f;
        }
        ImGui::SameLine();
        if (ImGui::Button("+ ##frequency_penalty")) {
            frequency_penalty += 0.001f;
        }
        ImGui::SameLine();
    }
    ImGui::SliderFloat("frequency_penalty", &frequency_penalty, 0.0f, 2.0f);
    if (ImGui::BeginPopupContextItem("frequency_penalty"))
    {
        if (ImGui::Selectable("Reset to default")){
            frequency_penalty = default_frequency_penalty;
        }
        ImGui::EndPopup();
    } ImGui::SameLine(); HelpMarker(("Descreases the chance of repeating the same lines in the output. Affects the immediate result. Default: " + std::to_string(default_frequency_penalty)).c_str());
}

static void sliderPresencePen(float& presence_penalty, float& default_presence_penalty){
    {
        if (ImGui::Button(" -##presence_penalty")) {
            presence_penalty -= 0.001f;
        }
        ImGui::SameLine();
        if (ImGui::Button("+ ##presence_penalty")) {
            presence_penalty += 0.001f;
        }
        ImGui::SameLine();
    }
    ImGui::SliderFloat("presence_penalty", &presence_penalty, 0.0f, 2.0f);
    if (ImGui::BeginPopupContextItem("presence_penalty"))
    {
        if (ImGui::Selectable("Reset to default")){
            presence_penalty = default_presence_penalty;
        }
        ImGui::EndPopup();
    } ImGui::SameLine(); HelpMarker(("Descreases the chance of repeating the same lines in the output. Affects the entrire dialog. Default: " + std::to_string(default_presence_penalty)).c_str());
}

static void sliderMinP(float& min_p, float& default_min_p){
    {
        if (ImGui::Button(" -##min_p")) {
            min_p -= 0.001f;
        }
        ImGui::SameLine();
        if (ImGui::Button("+ ##min_p")) {
            min_p += 0.001f;
        }
        ImGui::SameLine();
    }
    ImGui::SliderFloat("min_p", &min_p, 0.00f, 0.99f);
    if (ImGui::BeginPopupContextItem("min_p"))
    {
        if (ImGui::Selectable("Reset to default")){
            min_p = default_min_p;
        }
        ImGui::EndPopup();
    } ImGui::SameLine(); HelpMarker(("New proposed sampling method. Every possible token has a probability percentage attached to it that we will be measuring for consideration. The base min p value represents the starting required percentage. This gets scaled by the top token in the entire list's probability. For example, 0.05 = only include tokens that are at least 5% probable. Default: " + std::to_string(default_min_p)).c_str());
}

static void sliderTfsZ(float& tfs_z, float& default_tfs_z){
    {
        if (ImGui::Button(" -##tfs_z")) {
            tfs_z -= 0.001f;
        }
        ImGui::SameLine();
        if (ImGui::Button("+ ##tfs_z")) {
            tfs_z += 0.001f;
        }
        ImGui::SameLine();
    }
    ImGui::SliderFloat("tfs_z", &tfs_z, 0.5f, 1.0f); ImGui::SameLine();
    if (ImGui::BeginPopupContextItem("tfs_z"))
    {
        if (ImGui::Selectable("Reset to default")){
            tfs_z = default_tfs_z;
        }
        ImGui::EndPopup();
    } HelpMarker(("Tail free sampling, parameter z. TFS filters out logits based on the second derivative of their probabilities. Adding tokens is stopped after the sum of the second derivatives reaches the parameter z. In short: TFS looks how quickly the probabilities of the tokens decrease and cuts off the tail of unlikely tokens using the parameter z. Default: " + std::to_string(default_tfs_z)).c_str());
}

static void sliderTypicalP(float& typical_p, float& default_typical_p){
    {
        if (ImGui::Button(" -##typical_p")) {
            typical_p -= 0.001f;
        }
        ImGui::SameLine();
        if (ImGui::Button("+ ##typical_p")) {
            typical_p += 0.001f;
        }
        ImGui::SameLine();
    }
    ImGui::SliderFloat("typical_p", &typical_p, 0.5f, 1.0f);
    if (ImGui::BeginPopupContextItem("typical_p"))
    {
        if (ImGui::Selectable("Reset to default")){
            typical_p = default_typical_p;
        }
        ImGui::EndPopup();
    } ImGui::SameLine(); HelpMarker(("Locally typical sampling, parameter p. Promotes the generation of contextually coherent and diverse text by sampling tokens that are typical or expected based on the surrounding context. Default: " + std::to_string(default_typical_p)).c_str());
}

static void sliderPStep(float& p_step, float& default_p_step){
    {
        if (ImGui::Button(" -##p_step")) {
            p_step -= 0.001f;
        }
        ImGui::SameLine();
        if (ImGui::Button("+ ##p_step")) {
            p_step += 0.001f;
        }
        ImGui::SameLine();
    }
    ImGui::SliderFloat("p_step", &p_step, 0.1f, 1.0f);
    if (ImGui::BeginPopupContextItem("p_step"))
    {
        if (ImGui::Selectable("Reset to default")){
            p_step = default_p_step;
        }
        ImGui::EndPopup();
    } ImGui::SameLine(); HelpMarker(("P-Step Truncation Sampling. Discards all tokens after the first step in the probability distribution, keeping more tokens. Default: " + std::to_string(default_p_step)).c_str());
}

static void sliderMirostat(int& mirostat, int& default_mirostat){
    {
        if (ImGui::Button(" -##mirostat")) {
            if (mirostat > 0) --mirostat;
        }
        ImGui::SameLine();
        if (ImGui::Button("+ ##mirostat")) {
            if (mirostat < 2) mirostat++;
        }
        ImGui::SameLine();
    }
    ImGui::SliderInt("mirostat", &mirostat, 0, 2); ImGui::SameLine();
    if (ImGui::BeginPopupContextItem("mirostat"))
    {
        if (ImGui::Selectable("Reset to default")){
            mirostat = default_mirostat;
        }
        ImGui::EndPopup();
    } HelpMarker(("Uses Mirostat sampling. Top K, Nucleus, Tail Free and Locally Typical samplers are ignored with this. Default: " + std::to_string(default_mirostat)).c_str());
}

static void sliderMirostatTau(float& mirostat_tau, float& default_mirostat_tau){
    {
        if (ImGui::Button(" -##mirostat_tau")) {
            mirostat_tau -= 0.001f;
        }
        ImGui::SameLine();
        if (ImGui::Button("+ ##mirostat_tau")) {
            mirostat_tau += 0.001f;
        }
        ImGui::SameLine();
    }
    ImGui::SliderFloat("mirostat_tau", &mirostat_tau, 0.0f, 10.0f);
    if (ImGui::BeginPopupContextItem("mirostat_tau"))
    {
        if (ImGui::Selectable("Reset to default")){
            mirostat_tau = default_mirostat_tau;
        }
        ImGui::EndPopup();
    } ImGui::SameLine(); HelpMarker(("Mirostat learning rate. Default: " + std::to_string(default_mirostat_tau)).c_str());
}

static void sliderMirostatEta(float& mirostat_eta, float& default_mirostat_eta){
    {
        if (ImGui::Button(" -##mirostat_eta")) {
            mirostat_eta -= 0.001f;
        }
        ImGui::SameLine();
        if (ImGui::Button("+ ##mirostat_eta")) {
            mirostat_eta += 0.001f;
        }
        ImGui::SameLine();
    }
    ImGui::SliderFloat("mirostat_eta", &mirostat_eta, 0.00f, 0.50f);
    if (ImGui::BeginPopupContextItem("mirostat_eta"))
    {
        if (ImGui::Selectable("Reset to default")){
            mirostat_eta = default_mirostat_eta;
        }
        ImGui::EndPopup();
    } ImGui::SameLine(); HelpMarker(("Mirostat target entropy. Default: " + std::to_string(default_mirostat_eta)).c_str());
}

static void sliderXTCthreshold(float& xtc_threshold, float& default_xtc_threshold){
    {
        if (ImGui::Button(" -##xtc_threshold")) {
            xtc_threshold -= 0.001f;
        }
        ImGui::SameLine();
        if (ImGui::Button("+ ##xtc_threshold")) {
            xtc_threshold += 0.001f;
        }
        ImGui::SameLine();
    }
    ImGui::SliderFloat("xtc_threshold", &xtc_threshold, 0.00f, 0.50f);
    if (ImGui::BeginPopupContextItem("xtc_threshold"))
    {
        if (ImGui::Selectable("Reset to default")){
            xtc_threshold = default_xtc_threshold;
        }
        ImGui::EndPopup();
    } ImGui::SameLine(); HelpMarker(("XTC lower threshold. Default: " + std::to_string(default_xtc_threshold)).c_str());
}

static void paramsPanel(common_params& params, int& totalThreads) {
        //ImGui::SliderInt("n_threads", &localSettings.n_threads, 1, 4);
        sliderTemp(params.sparams.temp, paramsDefault.sparams.temp);
        
        sliderTempSmoothing(params.sparams.smoothing_factor, paramsDefault.sparams.smoothing_factor);
        
        sliderTempSmoothingCurve(params.sparams.smoothing_curve, paramsDefault.sparams.smoothing_curve);
        
        sliderDynatemp_range(params.sparams.dynatemp_range, params.sparams.temp, paramsDefault.sparams.dynatemp_range);

        sliderKShift(params.sparams.k_shift, paramsDefault.sparams.k_shift);

        sliderTopK(params.sparams.top_k, paramsDefault.sparams.top_k);

        sliderTopP(params.sparams.top_p, paramsDefault.sparams.top_p);

        sliderMinP(params.sparams.min_p, paramsDefault.sparams.min_p);

        sliderRepeatPen(params.sparams.penalty_repeat, paramsDefault.sparams.penalty_repeat);

        sliderRepeatThresh(params.sparams.penalty_threshold, paramsDefault.sparams.penalty_threshold);

        sliderFrequencyPen(params.sparams.penalty_freq, paramsDefault.sparams.penalty_freq);

        sliderPresencePen(params.sparams.penalty_present, paramsDefault.sparams.penalty_present);

        sliderTfsZ(params.sparams.tfs_z, paramsDefault.sparams.tfs_z);

        sliderTypicalP(params.sparams.typical_p, paramsDefault.sparams.typical_p);

        sliderPStep(params.sparams.p_step, paramsDefault.sparams.p_step);

        sliderXTCthreshold(params.sparams.xtc_threshold, paramsDefault.sparams.xtc_threshold);

        sliderMirostat(params.sparams.mirostat, paramsDefault.sparams.mirostat);

        sliderMirostatTau(params.sparams.mirostat_tau, paramsDefault.sparams.mirostat_tau);

        sliderMirostatEta(params.sparams.mirostat_eta, paramsDefault.sparams.mirostat_eta);
}

static void paramsPanelNew(common_params& params, int& totalThreads, ImVec2 size){
    if(ImGui::BeginChild("Params", size, false)) {
        
        
        
        ImGui::SliderInt("Generation threads", &params.cpuparams.n_threads, 1, totalThreads); ImGui::SameLine(); HelpMarker("Number of threads to use for generation, doesn't have to be maximum at the moment - try to find a sweetspot.");
        
        ImGui::SliderInt("Evaluation threads", &params.cpuparams_batch.n_threads, -1, totalThreads); ImGui::SameLine(); HelpMarker("Number of threads for prompt evaluation, recommended to set to maximum.");
    #if GGML_USE_CLBLAST || GGML_USE_VULKAN
        ImGui::SliderInt("Gpu layers", &params.n_gpu_layers, 0, 100); ImGui::SameLine(); HelpMarker("Number of layers to offload onto GPU.");
    #endif
        ImGui::TextWrapped(" Samping parameters");
        paramsPanel(params, totalThreads);
    
    ImGui::EndChild();
    }
}


static void templatesList(nlohmann::json& templatesJson, std::string& inputStr){
    if (ImGui::BeginChild("Templates from json file")) {
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
}

static void templatesListSelect(nlohmann::json& templatesJson, std::string& inputStr){
    if (ImGui::BeginChild("Templates from json file")){
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
    
    style.ChildRounding = 1.0f;
    style.FrameRounding = 1.0f;
    style.PopupRounding = 1.0f;
    style.TabRounding = 1.0f;
    style.GrabRounding = 1.0f;
    
    style.FramePadding.x = 10.0f;
    
    //for the "full-frame" main window
    style.WindowRounding = 0.0f;
}

struct imagesArray {
#if defined(SDL2)
    SDL_Texture* default_image;
    int my_image_width, my_image_height;
    std::map<std::string, SDL_Texture*> data;
    std::map<std::string, std::pair<int, int>> sizes;
    
    SDL_Texture* get_image(std::string key) {
        if (data.count(key) > 0) {
            return data.at(key);
        } else return default_image;
    }
    
    void load_image(SDL_Renderer* renderer, std::string key) {
        if (data.count(key) > 0) {
            bool ret = LoadTextureFromFile(key.c_str(), &data.at(key), sizes.at(key).first, sizes.at(key).second, renderer);
        } else {
            bool ret = LoadTextureFromFile("default.png", &default_image, my_image_width, my_image_height, renderer);
        }
    }
    
    void load_images(SDL_Renderer* renderer) {
        for (auto img : data) {
            bool ret = LoadTextureFromFile(img.first.c_str(), &img.second, sizes.at(img.first).first, sizes.at(img.first).second, renderer);
        }
    }
    
    void add(std::string image_name) {
        SDL_Texture* new_image;
        data.emplace(image_name, new_image);
        sizes.emplace(image_name, std::pair<int, int>{1,1});
    }
    
    std::pair<int, int> get_sizes () {
        
    }
#else
    MyTextureData default_image;
    std::map<std::string, MyTextureData> data;

    MyTextureData* get_image(std::string key) {
        if (data.count(key) > 0) {
            printf("getting %s\n", key.c_str());
            return &data.at(key);
        } else return &default_image;
    }
    
    void load_image(std::string key) {
        if (data.count(key) > 0) {
            bool ret = LoadTextureFromFile(key.c_str(), &data.at(key));
            IM_ASSERT(ret);
        } else {
            bool ret = LoadTextureFromFile("default.png", &default_image);
            IM_ASSERT(ret);
        }
    }
    
    void load_images() {
        for (auto img : data) {
            printf("-> %s\n", img.first.c_str());
            bool ret = LoadTextureFromFile(img.first.c_str(), &img.second);
            IM_ASSERT(ret);
        }
    }
    
    void add(std::string image_name) {
        MyTextureData new_image;
        data.emplace(image_name, new_image);
    }
#endif
};

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
    char const *jsonFilterPatterns[1]={"*.json"};
    std::string currPath = filesystem::current_path().string() + '/';
    
    modelThread newChat;
    
    std::vector<std::pair<std::string, std::string>> localResultPairs;
    
    nlohmann::json templatesJson = getJson("templates.json");
    
    presetTest Test;
    nlohmann::json testJson;
    std::string testPrompt;
    std::string testFolder;
    std::vector<std::string> testPresets;
    std::string testPresetsSummary;
    bool isGeneratingTest = false;
    
    std::map<std::string, std::vector<std::string>> sessionHistory;
    
    
    bool copiedDialog = false;
    bool copiedSettings = false;
    bool copiedTimings = false;
    bool initTokens = false;
    bool hasModel = false;
    //int totalThreads = get_num_physical_cores();
    //int totalThreads = get_math_cpu_count();
    int totalThreads = cpu_get_num_physical_cores();
    
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
    // let's make this safer
    std::string char_profile = "";
    std::string char_start = "";
    std::string char_name = "";
    std::string char_system_name = "";
    
    std::string sendButtonName = "Load";
    
    int maxString = 3000;
    int messageNum = 0;
    
    int messageWidth = width;
    
    int themeIdx = 2;
    int mdlIdx = 0;
    
    int regen_cycles = 0;
    
    bool show_models_list = true;
    bool show_immediate_settings = false;
    char* arrow = "<";
    char* vert_arrow = "v";
    
    bool use_menu_left = false;
    bool show_menu_left = false;
    char* menu = "=";
    char* menu_vert = "||";
    
    
    bool show_settings = false;
    bool show_settings_advanced = false;
    bool show_json = false;
    bool show_templates = false;
    bool show_prompts_db = false;
    bool show_tests = false;
    bool show_history = false;
    bool show_profile = false;
    // separate switchers for parts of UI
    bool use_menu_bar = true;
    
    bool use_models_list_left = false;
    bool use_models_list_right = true;
    bool use_settings_window = false;
    bool show_settings_window = false;
    
    bool showModelsList = false;
    
    
#if defined(SDL2)
        SDL_Texture* current_image;
        int* my_image_width, my_image_height;
#else 
        MyTextureData* current_image;
#endif

    imagesArray images;
    
    ImGuiInputTextFlags inputFlags = ImGuiInputTextFlags_AllowTabInput | ImGuiInputTextFlags_EnterReturnsTrue;
    
    ImGuiWindowFlags chat_window_flags = ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoCollapse|ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoFocusOnAppearing|ImGuiWindowFlags_NoBringToFrontOnFocus|ImGuiWindowFlags_NoSavedSettings;
    
    ImGuiWindowFlags overlay_window_flags = ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoCollapse|ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoFocusOnAppearing|ImGuiWindowFlags_NoSavedSettings;
    
    ImGuiWindowFlags popup_window_flags;// = ImGuiWindowFlags_NoMove;
    
    ImGuiWindowFlags sub_window_flags;// = ImGuiWindowFlags_NoCollapse;
    
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
            if (configJson.contains("theme") && configJson["theme"].is_number()) {
                themeIdx = configJson["theme"].get<int>();
            }
            
            
            
            //configJson.erase("error");
        }
    }
    
    void readStyleFromConfigJson(){
        if (!configJson["error"].is_string()){
            if (configJson.contains("style") && configJson["style"].is_object()) {
                ImGuiStyle& style = ImGui::GetStyle();
                
                if (configJson["style"].contains("AntiAliasedLines") && configJson["style"]["AntiAliasedLines"].is_boolean()) style.AntiAliasedLines = configJson["style"]["AntiAliasedLines"];
                if (configJson["style"].contains("AntiAliasedLinesUseTex") && configJson["style"]["AntiAliasedLinesUseTex"].is_boolean()) style.AntiAliasedLinesUseTex = configJson["style"]["AntiAliasedLinesUseTex"];
                if (configJson["style"].contains("AntiAliasedFill") && configJson["style"]["AntiAliasedFill"].is_boolean()) style.AntiAliasedFill = configJson["style"]["AntiAliasedFill"];
               
                if (configJson["style"].contains("FrameBorderSize") && configJson["style"]["FrameBorderSize"].is_number()) style.FrameBorderSize  = configJson["style"]["FrameBorderSize"];
                if (configJson["style"].contains("ChildBorderSize") && configJson["style"]["ChildBorderSize"].is_number()) style.ChildBorderSize  = configJson["style"]["ChildBorderSize"];
                if (configJson["style"].contains("WindowPadding.x") && configJson["style"]["WindowPadding.x"].is_number()) style.WindowPadding.x = configJson["style"]["WindowPadding.x"];
                if (configJson["style"].contains("CellPadding.x") && configJson["style"]["CellPadding.x"].is_number()) style.CellPadding.x = configJson["style"]["CellPadding.x"];
                if (configJson["style"].contains("ItemSpacing.x") && configJson["style"]["ItemSpacing.x"].is_number()) style.ItemSpacing.x  = configJson["style"]["ItemSpacing.x"];
                if (configJson["style"].contains("ChildRounding") && configJson["style"]["ChildRounding"].is_number()) style.ChildRounding  = configJson["style"]["ChildRounding"];
                if (configJson["style"].contains("FrameRounding") && configJson["style"]["FrameRounding"].is_number()) style.FrameRounding  = configJson["style"]["FrameRounding"];
                if (configJson["style"].contains("PopupRounding") && configJson["style"]["PopupRounding"].is_number()) style.PopupRounding  = configJson["style"]["PopupRounding"];
                if (configJson["style"].contains("TabRounding") && configJson["style"]["TabRounding"].is_number()) style.TabRounding  = configJson["style"]["TabRounding"];
                if (configJson["style"].contains("GrabRounding") && configJson["style"]["GrabRounding"].is_number()) style.GrabRounding  = configJson["style"]["GrabRounding"];
                if (configJson["style"].contains("WindowRounding") && configJson["style"]["WindowRounding"].is_number()) style.WindowRounding  = configJson["style"]["WindowRounding"];
                if (configJson["style"].contains("FramePadding.x") && configJson["style"]["FramePadding.x"].is_number()) style.FramePadding.x  = configJson["style"]["FramePadding.x"];
            }
        }
    }
    
    void openCard(){
        if (ImGui::Button("Open a preset card")) {
            auto presetCardFilePath = tinyfd_openFileDialog("Select a preset card file...", currPath.c_str(),1, jsonFilterPatterns, NULL,0);
            
            if (presetCardFilePath) {
                nlohmann::json presetCardFile = getJson(presetCardFilePath);
                localSettings.getSettingsFromJson(presetCardFile);
                // we probably don't want presets to be instantly applied
                //localSettings.fillLocalJson();
            }
            
            
        }
    }
    
    void saveCard(){
        if (ImGui::Button("Save a preset card")) {
            auto presetCardFilePath = tinyfd_saveFileDialog( "preset" , currPath.c_str() , 1 , jsonFilterPatterns, NULL);
            
            if (presetCardFilePath) {
                nlohmann::json presetCardFile = localSettings.createNewCard();
                
                writeJson(presetCardFile, presetCardFilePath);
            }
            
            
        }
    }
    
    void settingsTab() {
        
        
        //if (!localSettings.params.prompt.empty() ) {
            
            //ImGui::SameLine();
            //ImGui::TextWrapped(localSettings.modelName.c_str());
            //ImGui::TextWrapped(localSettings.modelFromJson.c_str());
            ImGui::TextWrapped(aTiming.c_str());
            
            ImGui::TextWrapped( ("Order of samplers: " + newChat.sparamsList ).c_str() );
            ImGui::Separator();
            //ImGui::SameLine();
            if (newChat.loaded == 9) {
                
                if (copiedTimings == false) {
                    aTiming = newChat.lastTimings;
                    copiedTimings = true;
                }

                if (copiedSettings){
                    ImGui::TextWrapped( ("SEED: " + std::to_string(localSettings.params.sparams.seed)).c_str() );
                   //ImGui::BeginChild("PreData");
                    //ImGui::TextWrapped( ("Base (initial) prompt: " + localSettings.params.prompt).c_str() ); ImGui::SameLine(); HelpMarker("This serves as the basis for the dialog, providing context. Make sure it ends with the antiptrompt if you don't want the NN to converse endlessly.");
                    //ImGui::Separator();
                   //ImGui::EndChild();
                    //ImGui::TextWrapped( ("Antiprompt: " + localSettings.params.antiprompt[0]).c_str() ); ImGui::SameLine(); HelpMarker("This serves as a stop-word for the NN to be able to end generation and wait for your input.");
                    ImGui::Separator();
                    
                    if (localSettings.params.rope_freq_scale != paramsDefault.rope_freq_scale){
                        ImGui::TextWrapped( ("rope_freq_scale: " + std::to_string(localSettings.params.rope_freq_scale)).c_str() ); ImGui::SameLine(); HelpMarker("Specific paramenter to adjust for extra large context models - needs to be (base model n_ctx)/(this model n_ctx), for example, 0.25 for llama2 (4096) -based everythinglm-13b-16k (16384)");
                        ImGui::Separator();
                    }
                    if (localSettings.params.rope_freq_base != paramsDefault.rope_freq_base){
                        ImGui::TextWrapped( ("rope_freq_base: " + std::to_string(localSettings.params.rope_freq_base)).c_str() ); ImGui::SameLine(); HelpMarker("Specific paramenter to adjust for extra large context models.");
                        ImGui::Separator();
                    }
                    ImGui::TextWrapped( ("Context size: " + std::to_string(localSettings.params.n_ctx)).c_str() ); ImGui::SameLine(); HelpMarker("The amount of data that can be stored and processed by the model within a session - depends on the model and should be used carefully. Models with context higher than 2048 usually have it in their name or description.");
                    ImGui::Separator();
                    ImGui::Spacing();
                    
                    if (newChat.isContinue != 'w') {
                        if (ImGui::Button("Apply settings")) {
                            localSettings.fillLocalJson();
                            localSettings.pushSettings(newChat.newChat);
                            //localJsonDump = localSettings.modelConfig.dump(3);
                            localSettings.updateDump();
                        }
                        
                        ImGui::SameLine();
                        
                        if (ImGui::Button("Restore settings from model")) {
                            localSettings.getSettings(newChat.newChat);
                            localSettings.fillLocalJson();
                        }
                        
                        ImGui::SameLine();
                        openCard();
                        ImGui::SameLine();
                        saveCard();
                        
                        // ImGui::Checkbox("Temperature first", &newChat.newChat.tempFirst);
                        // ImGui::SameLine();
                        // HelpMarker("Temperature sampling is originally last in llama.cpp, which affect how it's randomeness affects the resulting array of tokens. You can change it for experiment.");
                    //} else ImGui::TextWrapped( (newChat.newChat.tempFirst ? "Temperature is first in sampling." : "Temperature is last in sampling.") );
                    } 
                    
                    
                }
                
                ImGui::TextWrapped(" Model performance settings");
                ImGui::Separator();
                paramsPanelNew(newChat.newChat.params, totalThreads, ImVec2( ImGui::GetContentRegionAvail().x * 0.85, ImGui::GetContentRegionAvail().y));
            } else {
                
                
                
                ImGui::TextWrapped("Model isn't loaded yet...");
                
                if (ImGui::Button("Apply settings")) {
                    localSettings.fillLocalJson();
                    //localJsonDump = localSettings.modelConfig.dump(3);
                    localSettings.updateDump();
                }
                ImGui::SameLine();
                if (ImGui::Button("Restore settings from config")) {
                    localSettings.getSettingsFromModelConfig();
                    localSettings.fillLocalJson();
                }
                ImGui::SameLine();
                openCard();
                ImGui::SameLine();
                saveCard();
                
                ImGui::TextWrapped(" Config performance settings");
                ImGui::Separator();
                paramsPanelNew(localSettings.params, totalThreads, ImVec2( ImGui::GetContentRegionAvail().x * 0.85, ImGui::GetContentRegionAvail().y));
                
            }
            
            
        //}
        //ImGui::BeginChild("SettingsTabsHalf", ImVec2( ImGui::GetContentRegionAvail().x * 0.5, ImGui::GetContentRegionAvail().y), false);
        
        

        //ImGui::EndChild();
        
        
        
        //ImGui::BeginChild("Timings", ImVec2( ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y), false);
            
        
            
            //ImGui::TextWrapped((newChat.lastTimings).c_str());
        
        //ImGui::EndChild();
                
                
    }
    
    void jsonTab(){
        if (ImGui::Button("Reset config")) {
            localSettings.resetConfig("config.json");
            //localSettings.getFromJson("config.json");
            //localSettings.fillLocalJson();
            //localSettings.updateDump();
        }
        ImGui::SameLine();
        if (ImGui::Button("Update config")) {
            localSettings.updateDump();
        }
        ImGui::SameLine();
        // if (ImGui::Button("Save config"))   { 
            // localSettings.pushToMainConfig();
            // writeJson(localSettings.localConfig, "config.json");
             
        // }
        // ImGui::SameLine();
        if (ImGui::Button("Open settings json")) {
            auto jsonConfigFilePath = tinyfd_openFileDialog("Select a json file...", currPath.c_str(),1, jsonFilterPatterns, "config.json",0);
            
            if (jsonConfigFilePath) {
                loadSettingsJson(jsonConfigFilePath);
            }
        }
        
        if (ImGui::BeginChild("Parameters from json file")) {
            
            ImGui::TextWrapped(localSettings.localJsonDump.c_str());
            
        ImGui::EndChild();
        }
        
    }
    
    void templatesTab(){
        
        if (!templatesJson.contains("error")){
            //ImGui::Indent();
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + messageWidth * 0.20f);
            if (ImGui::Button("Reset templates config")) {
                templatesJson = getJson("templates.json");
                //localSettings.getFromJson("config.json");
                //localSettings.fillLocalJson();
                //localJsonDump = localSettings.modelConfig.dump(3);
            }
            ImGui::SameLine();
            if (ImGui::Button("Save templates")) {
                writeJson(templatesJson, "templates.json");
            }
            
            //ImGui::Unindent();
            
            templatesList(templatesJson, inputStr);
        } else {
            ImGui::TextWrapped("This is where your saved templates (instructs or dialog lines) will be stored for this session - or, if you save them, loaded from templates.json file.");
        }
        
    }
    
    void templatesWindow() {
        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImVec2 child_size = ImVec2(ImGui::GetWindowWidth() * 1.4f, ImGui::GetTextLineHeightWithSpacing() * 38);
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

        if (!ImGui::Begin("Templates window", &show_templates, ImGuiWindowFlags_NoSavedSettings))
        {
            ImGui::End();
            return;
        }
        
        
        ImGui::BeginChildFrame(ImGui::GetID("Templates frame"), child_size, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);
        
        templatesTab();
        
        ImGui::EndChildFrame();
        
        ImGui::End();
    }
    
    void promptsDbWindow() {
        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImVec2 child_size = ImVec2(ImGui::GetWindowWidth() * 1.4f, ImGui::GetTextLineHeightWithSpacing() * 38);
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

        if (!ImGui::Begin("Prompts database window", &show_prompts_db, ImGuiWindowFlags_NoSavedSettings))
        {
            ImGui::End();
            return;
        }
        
        
        ImGui::BeginChildFrame(ImGui::GetID("Prompts database frame"), child_size, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);
        
        promptFilesTab(ImGui::GetMainViewport());
        
        ImGui::EndChildFrame();
        
        ImGui::End();
    }
    
    void testsWindow() {
        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImVec2 child_size = ImVec2(ImGui::GetWindowWidth() * 1.4f, ImGui::GetTextLineHeightWithSpacing() * 38);
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

        if (!ImGui::Begin("Tests window", &show_tests, ImGuiWindowFlags_NoSavedSettings))
        {
            ImGui::End();
            return;
        }
        
        
        ImGui::BeginChildFrame(ImGui::GetID("Tests frame"), child_size, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);
        
        testsTab();
        
        ImGui::EndChildFrame();
        
        ImGui::End();
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
            
            if (ImGui::BeginChild("Prompt files from folder")) {
            
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
            }
        } else {
            ImGui::TextWrapped("This is where a list of prompt files (in .txt) is shown. Open a folder with them to see the list.");
        }
    }
    
    void historyTab(){
        if (sessionHistory.size()){
                
            if (ImGui::BeginChild("History of your prompts")) {
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
                                        templatesJson["saved"].emplace_back(msg);
                                        templatesJson.erase("error");
                                    }
                                } else {
                                    for (auto& [key, value] : templatesJson.items() ){
                                        if (value.is_array()) {
                                            if (ImGui::Selectable(("Store as " + key).c_str())){
                                                templatesJson[key].emplace_back(msg);
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
            }
        } else {
            ImGui::TextWrapped("No session history for now - your messages will appear here once sent.");
        }
    }
    
    void message(std::string& msgText, int& messageNum) {
        if (msgText.size() < maxString){
            ImGui::Text((msgText.c_str()));
            if (ImGui::BeginPopupContextItem(std::to_string(messageNum).c_str()))
            {
                if (ImGui::Selectable("Copy")){
                    ImGui::LogToClipboard();
                    ImGui::LogText(msgText.c_str());
                    ImGui::LogFinish();
                }
                
                if (ImGui::Selectable("Save on disk")){
                    auto saveAnswer = tinyfd_saveFileDialog( std::to_string(messageNum).c_str() , currPath.c_str() , 1 , instructFilterPatterns, NULL);

                    if (saveAnswer){
                        writeTextFileOver(saveAnswer, msgText);
                    }
                }
                ImGui::EndPopup();
            }

        } else {
            std::string msg = msgText;
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
                        ImGui::LogText(msgText.c_str());
                        ImGui::LogFinish();
                    }
                    
                    if (ImGui::Selectable("Save on disk")){
                        auto saveAnswer = tinyfd_saveFileDialog( std::to_string(messageNum).c_str() , currPath.c_str() , 1 , instructFilterPatterns, NULL);

                        if (saveAnswer){
                            writeTextFileOver(saveAnswer, msgText);
                        }
                    }
                
                    ImGui::EndPopup();
                }
            }
        
            ImGui::Text((msg.c_str()));
            if (ImGui::BeginPopupContextItem(std::to_string(messageNum).c_str()))
            {
                if (ImGui::Selectable("Copy")){
                    ImGui::LogToClipboard();
                    ImGui::LogText(msgText.c_str());
                    ImGui::LogFinish();
                }
                
                if (ImGui::Selectable("Save on disk")){
                    auto saveAnswer = tinyfd_saveFileDialog( std::to_string(messageNum).c_str() , currPath.c_str() , 1 , instructFilterPatterns, NULL);

                    if (saveAnswer){
                        writeTextFileOver(saveAnswer, msgText);
                    }
                }
                ImGui::EndPopup();
            }
        }
    }
    
    void messageAuto(std::string& msgText, int& messageNum) {
        if (msgText.size() < maxString){
            ImGui::TextWrapped((msgText.c_str()));
            if (ImGui::BeginPopupContextItem(std::to_string(messageNum).c_str()))
            {
                if (ImGui::Selectable("Copy")){
                    ImGui::LogToClipboard();
                    ImGui::LogText(msgText.c_str());
                    ImGui::LogFinish();
                }
                
                if (ImGui::Selectable("Save on disk")){
                    auto saveAnswer = tinyfd_saveFileDialog( std::to_string(messageNum).c_str() , currPath.c_str() , 1 , instructFilterPatterns, NULL);

                    if (saveAnswer){
                        writeTextFileOver(saveAnswer, msgText);
                    }
                }
                ImGui::EndPopup();
            }

        } else {
            std::string msg = msgText;
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
                
                ImGui::TextWrapped((subMsg.c_str()));
                if (ImGui::BeginPopupContextItem((std::to_string(messageNum)+"_s").c_str()))
                {
                    if (ImGui::Selectable("Copy")){
                        ImGui::LogToClipboard();
                        ImGui::LogText(msgText.c_str());
                        ImGui::LogFinish();
                    }
                    
                    if (ImGui::Selectable("Save on disk")){
                        auto saveAnswer = tinyfd_saveFileDialog( std::to_string(messageNum).c_str() , currPath.c_str() , 1 , instructFilterPatterns, NULL);

                        if (saveAnswer){
                            writeTextFileOver(saveAnswer, msgText);
                        }
                    }
                
                    ImGui::EndPopup();
                }
            }
        
            ImGui::TextWrapped((msg.c_str()));
            if (ImGui::BeginPopupContextItem(std::to_string(messageNum).c_str()))
            {
                if (ImGui::Selectable("Copy")){
                    ImGui::LogToClipboard();
                    ImGui::LogText(msgText.c_str());
                    ImGui::LogFinish();
                }
                
                if (ImGui::Selectable("Save on disk")){
                    auto saveAnswer = tinyfd_saveFileDialog( std::to_string(messageNum).c_str() , currPath.c_str() , 1 , instructFilterPatterns, NULL);

                    if (saveAnswer){
                        writeTextFileOver(saveAnswer, msgText);
                    }
                }
                ImGui::EndPopup();
            }
        }
    }
    
    void preparePrompt() {
        // if (textFileContents.size()) {
            // inputStr += '"' + textFileContents + '"';
            // textFileContents.clear();
        // }
        processPrompt(inputStr, '{', '}');
        
        if (inputStr.size()){
            sessionHistory[localSettings.modelName].emplace_back(inputStr);
        }
        newChat.appendQuestion(inputStr);
        remove_last_nl(inputStr);
        localResultPairs = newChat.resultsStringPairs;
        //newChat.getResultAsyncString();
        inputStr = "";
        //inputChars = inputStr.c_str();
        //tokens_this_session = newChat.last_tokens;
        
    }
    
    void sendPrompt() {
        preparePrompt();
        
        helpLabel = "Generating... ";//+std::to_string(tokens_this_session) + " tokens.";
        //newChat.isContinue = 'w';
        newChat.startGen();
        output = "...";
        //newChat.getResultAsyncStringFull2(false, true);
        newChat.runGenerationAsync();
        copiedDialog = false;
        copiedTimings = false;
        scrolled = false;
    }
    
    void regenPrompt() {
        // accomodating pause
        if (newChat.lastResult.empty()) newChat.resultsStringPairs.pop_back();
        else cancelled = false;
        newChat.newChat.rewind();
        localResultPairs = newChat.resultsStringPairs;
        
        helpLabel = "Generating... ";//+std::to_string(tokens_this_session) + " tokens.";
        //newChat.isContinue = 'w';
        newChat.startGen();
        output = "...";
        //newChat.getResultAsyncStringFull2(false, true);
        newChat.runGenerationAsync(false);
        copiedDialog = false;
        copiedTimings = false;
        scrolled = false;
    }
    
    void suffixEdit(char* name) {
        ImGui::InputTextMultiline(name, &localSettings.params.input_suffix, ImVec2(ImGui::GetContentRegionAvail().x * 0.6, ImGui::GetTextLineHeightWithSpacing() * 4)); ImGui::SameLine(); HelpMarker( "Suffix is added after your prompt - can be used to instantly set the charater for NN." );
        if (newChat.isNewSuffix(localSettings.params.input_suffix)){ 
            ImGui::SameLine();
            if (ImGui::Button("Apply")) { newChat.applySuffix(localSettings.params.input_suffix); } 
        }
    }
    
    void contextInstruct(std::string& contextName) {
        static int idx = 0;
        //if (ImGui::CollapsingHeader(contextName.c_str(), ImGuiTreeNodeFlags_None)) {
            if (ImGui::BeginChild("ContextMsg")) {
                //ImGui::PushTextWrapPos(ImGui::GetContentRegionAvail().x);
                messageAuto(char_profile, idx);
                //ImGui::PopTextWrapPos();
            ImGui::EndChild();
            }
        //}
    }
    
    void profileBar(ImVec2& size) {
        if (ImGui::BeginChild("ProfileBar", ImVec2(size.x, size.y - ImGui::GetTextLineHeight()))){
            
            if (ImGui::BeginChild("Profile suffix", ImVec2( ImGui::GetContentRegionAvail().x * 0.2, ImGui::GetContentRegionAvail().y))){
                suffixEdit("##Character");
            ImGui::EndChild();
            }
            ImGui::SameLine();
            if (ImGui::BeginChild("Profile context")){
                static std::string profileName = "Profile";
                //contextInstruct(profileName);
                static int idx = 0;
                messageAuto(localResultPairs[0].second, idx);
            ImGui::EndChild();
            }
            
        ImGui::EndChild();
        }
    }
    
    void sendButtonsVertical(const ImGuiViewport* viewport) {
        //if (newChat.isContinue != 'w') {
            
            if (ImGui::Button("Choose a template")) {
                ImGui::OpenPopup("Templates");
            }
            
            if (ImGui::BeginPopup("Templates", ImGuiWindowFlags_NoSavedSettings))
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
            
            if (ImGui::Button("Choose a wildcards file")) {
                auto wildcardsFile = tinyfd_openFileDialog("Select a wildcards file...", currPath.c_str(),1, jsonFilterPatterns, NULL,0);
                    if (wildcardsFile) {
                        nlohmann::json wildcardsDB = getJson(wildcardsFile);
                        
                        inputStr = processByWildcards(wildcardsDB, inputStr);
                    }
            }
            
            if (ImGui::Button("Prompts history")) {
                show_history = !show_history;
            }
            
            if (newChat.isContinue == 'i' && ( localResultPairs.size() > 2 || !newChat.lastResult.empty())) {
                if (regen_cycles > 0) {
                    //newChat.writeTextFile("tales/",std::to_string(regen_cycles));
                    newChat.writeTextFileUnified("tales/",std::to_string(regen_cycles), regen_cycles == 10);
                    --regen_cycles;
                    regenPrompt();
                } else {
                    if (ImGui::Button("Regenerate...")) {
                        ImGui::OpenPopup("Regens");
                    }
                    
                    if (ImGui::BeginPopup("Regens", ImGuiWindowFlags_NoSavedSettings)) {
                        
                        if (ImGui::Selectable("Regenerate once")) {
                            regenPrompt();
                        }
                        
                        if (ImGui::Selectable("Regenrate x10")) {
                            regen_cycles = 10;
                            regenPrompt();
                        }
                        
                        ImGui::EndPopup();
                    }
                }

                if (ImGui::Button("Save all")) {
                    auto saveAnswer = tinyfd_saveFileDialog( std::to_string(messageNum).c_str() , currPath.c_str() , 1 , instructFilterPatterns, NULL);

                    if (saveAnswer){
                        newChat.writeTextFileSimple(saveAnswer);
                    }
                }
            }
                
            
        //}
    }
    
    void sendButtonsHorizontal(const ImGuiViewport* viewport) {
        //if (newChat.isContinue != 'w') {

            ImGui::SameLine();
            if (ImGui::Button("Choose a template")) {
                ImGui::OpenPopup("Templates");
            }
            ImGui::SameLine();
            if (ImGui::BeginPopup("Templates", ImGuiWindowFlags_NoSavedSettings))
            {
                ImVec2 work_size = viewport->WorkSize;
                ImGui::BeginChild("Templates list", ImVec2(  work_size.x * 0.5, work_size.y * 0.7));
                    templatesListSelect(templatesJson, inputStr);
                ImGui::EndChild();
                ImGui::EndPopup();
                
            }
            ImGui::SameLine();
            if (ImGui::Button("Choose a txt file")) {
                auto textFile = tinyfd_openFileDialog("Select a text file...", currPath.c_str(),1, instructFilterPatterns, NULL,0);
                    if (textFile) {
                        inputStr += " {";
                        inputStr += textFile;
                        inputStr += "}";
                        // textFileContents.clear();
                        // std::ifstream file(textFile);
                        // if (file) {
                            //std::copy(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>(), back_inserter(textFileContents));
                        // }
                    }
            }
            ImGui::SameLine();
            if (textFileContents.size()) {
                if (ImGui::Button("Clear text file")) {
                    textFileContents.clear();
                }
                
                
            }
            ImGui::SameLine();
            if (ImGui::Button("Choose a wildcards json")) {
                auto wildcardsFile = tinyfd_openFileDialog("Select a wildcards file...", currPath.c_str(),1, jsonFilterPatterns, NULL,0);
                    if (wildcardsFile) {
                        nlohmann::json wildcardsDB = getJson(wildcardsFile);
                        
                        inputStr = processByWildcards(wildcardsDB, inputStr);
                    }
            }
            ImGui::SameLine();
            if (ImGui::Button("Prompts history")) {
                //show_history = !show_history;
                ImGui::OpenPopup("History");
            }
            if (newChat.isContinue == 'i' && ( localResultPairs.size() > 2 || !newChat.lastResult.empty())) {
                
                if (regen_cycles > 0 && !cancelled) {
                    newChat.writeTextFile("tales/",std::to_string(regen_cycles));
                    --regen_cycles;
                    regenPrompt();
                } else {
                    ImGui::SameLine();
                    if (ImGui::Button("Regenerate...")) {
                        ImGui::OpenPopup("Regens");
                    }
                    
                    if (ImGui::BeginPopup("Regens", ImGuiWindowFlags_NoSavedSettings)) {
                        
                        if (ImGui::Selectable("Regenerate once")) {
                            regenPrompt();
                        }
                        
                        if (ImGui::Selectable("Regenrate x10")) {
                            newChat.writeTextFile("tales/",std::to_string(regen_cycles));
                            regen_cycles = 10;
                            regenPrompt();
                        }
                        
                        ImGui::EndPopup();
                    }
                }

                ImGui::SameLine();
                if (ImGui::Button("Save all")) {
                    auto saveAnswer = tinyfd_saveFileDialog( std::to_string(messageNum).c_str() , currPath.c_str() , 1 , instructFilterPatterns, NULL);

                    if (saveAnswer){
                        newChat.writeTextFileSimple(saveAnswer);
                    }
                }
            }
            if (ImGui::BeginPopup("History", ImGuiWindowFlags_NoSavedSettings))
            {
                ImVec2 work_size = viewport->WorkSize;
                ImGui::BeginChild("Templates list", ImVec2(  work_size.x * 0.5, work_size.y * 0.7));
                    historyTab();
                ImGui::EndChild();
                ImGui::EndPopup();
                
            }
        //}
    }
    
    std::string getSubprofile() {
        if (!char_profile.empty()) {
            size_t pos = char_profile.find(char_system_name);
            if (pos != char_profile.npos) {
                return char_profile.substr(pos);
            } else return "";
        } else return "";
    }
    
    // separating this into more functions breaks "realtime" speed display
    void dialogTab(const ImGuiViewport* viewport){
        if (ImGui::BeginChild("DialogSpace")) {
            messageWidth = ImGui::GetWindowWidth();
    // staring main loop once loaded the model//////////////////////////////////////////////////////////
            if (localSettings.noConfig){
                ImGui::TextWrapped("No model config file found!");
            }
// Initial buttons and settings to load a model////////////////////////////////////////////////////
            //if (newChat.loaded == 9) {
            if (newChat.isContinue == '_' && showModelsList) {
                simpleStartScreen();
            } else {
                
                if (newChat.loaded == 9) {
                    if (!initTokens){
                        tokens_this_session = newChat.last_tokens;
                        consumed_this_session = newChat.consumed_tokens;
                        past_this_session = newChat.past_tokens;
                        
                        initTokens = true;
                    }
                    
                    if (!copiedSettings){
                        localSettings.getSettings(newChat.newChat);
                        helpLabel = localSettings.params.antiprompt[0];
                        shortModelName = newChat.shortModelName;
                        copiedSettings = true;
                    }
                }
                
                {
/////// rendering dialogs ////////////////////////////////////////////////////////////////////////
                    if (ImGui::BeginChild("Dialog", ImVec2( messageWidth * 0.99f, ImGui::GetContentRegionAvail().y*0.75f))) {
                        ImGui::Indent();
                            // to avoid mutexes we only use a copy of dialog DB for UI
                            if (newChat.isContinue == 'i') {
                                if (!copiedDialog) {
                                    localResultPairs = newChat.resultsStringPairs;
                                    ImGui::SetScrollFromPosY(ImGui::GetCursorPos().y + ImGui::GetContentRegionAvail().y, 0.25f);
                                    copiedDialog = true;
                                    copiedTimings = false;
                                    //aTiming = newChat.lastTimings;
                                    helpLabel = " ";
                                    //getSubprofile();
                                }
                            }


                            messageNum = 0;
                            for (auto& r : localResultPairs){
                                ++messageNum;
                                if (r.first == "AI" || r.first == "INSTRUCT"){
    ////////////////////generated
                                    //std::cout << r.second;
                                    //ImGui::TextWrapped(r.second.c_str());
                                    if (messageNum > 1) {
#if defined(SDL2)
                                        ImGui::Image((void*)current_image, ImVec2(32, 32));
#else 
                                        ImGui::Image((ImTextureID)current_image->DS, ImVec2(32, 32));
#endif
                                        if (chatMode) ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + messageWidth * 0.5f);
                                        else {
                                            //if (messageNum > 1) ImGui::SeparatorText("answer");
                                            ImGui::SameLine();
                                            ImGui::Separator();
                                            ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + ImGui::GetContentRegionAvail().x);
                                        }
                                        
                                        
                                        message(r.second, messageNum);
                                        
                                        ImGui::PopTextWrapPos();
                                    } else if (!char_start.empty()) {
                                        #if defined(SDL2)
                                        ImGui::Image((void*)current_image, ImVec2(32, 32));
#else 
                                        ImGui::Image((ImTextureID)current_image->DS, ImVec2(32, 32));
#endif
                                        if (chatMode) ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + messageWidth * 0.5f);
                                        else {
                                            //if (messageNum > 1) ImGui::SeparatorText("answer");
                                            ImGui::SameLine();
                                            ImGui::Separator();
                                            ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + ImGui::GetContentRegionAvail().x);
                                        }
                                        
                                        message(char_start, messageNum);
                                        
                                        ImGui::PopTextWrapPos();
                                    }
                                    // if (messageNum == 1 && chatMode){
                                        // ImGui::SeparatorText("Dialog");
                                    // }
                                } else {
    ////////////////////////prompts

                                    if (chatMode) {
                                        ImGui::SetCursorPosX(ImGui::GetContentRegionAvail().x - r.first.length()*ImGui::GetFontSize()* 0.45f);
                                        ImGui::Text((r.first).c_str());
                                    } else ImGui::SeparatorText(r.first.c_str());
                                    
                                    //ImGui::BeginChild(std::to_string(messageNum).c_str());
                                    //ImGui::TextWrapped((r.first + r.second).c_str());
                                    auto msgLen = r.second.length()*ImGui::GetFontSize();
                                    
                                    if (chatMode) { 
                                        if (msgLen < messageWidth) {
                                            
                                            ImGui::SetCursorPosX(ImGui::GetContentRegionAvail().x - msgLen * 0.45f);
                                            ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + msgLen * 0.50f);
                                        } else {
                                            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + messageWidth * 0.45f);
                                            ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + messageWidth* 0.50f);
                                        }
                                        //else ImGui::SetCursorPosX(ImGui::GetCursorPosX() + messageWidth * 0.40f);
                                    } else ImGui::PushTextWrapPos(ImGui::GetContentRegionAvail().x);
                                    
                                    
                                    
                                    message(r.second, messageNum);
                                    ImGui::PopTextWrapPos();
                                    //ImGui::EndChild();
                                //ImGui::Separator();
                                //ImGui::Text((r.first + r.second).c_str(), wrap_width);
                                //ImGui::PopTextWrapPos();
                                //ImGui::InputTextMultiline(" ", &questionStr, ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 2), ImGuiInputTextFlags_ReadOnly);
                                    
                                }
                                
                                //ImGui::Spacing();
                                //if (r.second.back() != '\n') std::cout<< DELIMINER;
                            }
                            if (cancelled && !newChat.lastResult.empty()) {
                                int lastMsg = localResultPairs.size() + 1;
                                if (chatMode) ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + messageWidth * 0.5f);
                                    else {
                                        ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + ImGui::GetContentRegionAvail().x);
                                    }
                                message(newChat.lastResult, lastMsg);
                                ImGui::PopTextWrapPos();
                            }
                            
//////////////// streaming dialog token-after-token
                            if (newChat.isContinue == 'w') {
#if defined(SDL2)
                                ImGui::Image((void*)current_image, ImVec2(32, 32));
#else 
                                ImGui::Image((ImTextureID)current_image->DS, ImVec2(32, 32));
#endif
                                if (chatMode) ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + messageWidth * 0.50f);
                                else {
                                    //ImGui::SeparatorText("answer");
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
                                /* if (newChat.isPregen != 'i') ImGui::SeparatorText(std::format("Reading at {:.2f} t/s", newChat.lastSpeedPrompt).c_str());
                                else ImGui::SeparatorText(std::format("Read at {:.2f} t/s, generating at {:.2f} t/s", newChat.lastSpeedPrompt, newChat.lastSpeed).c_str()); */
                                
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
                }
// INPUT ////////////////////////////////////////////////////////////////////////////////////////

                auto inputWidth = ImGui::GetContentRegionAvail().x * 0.75f;

               ImGui::Spacing();
               ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 14);
               if (ImGui::BeginChild("inputAndSendButtons", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y))) {

                sendButtonsHorizontal(viewport);
                if (ImGui::InputTextMultiline("###inputField", &inputStr, ImVec2(inputWidth, ImGui::GetContentRegionAvail().y), inputFlags) == true){
                    if (newChat.isContinue == 'i') sendPrompt();
                    else if (newChat.isContinue == '_') loadModel();
                }
                
                //if (newChat.isContinue == 'i') {
                    
                    //ImGui::SetWindowFontScale(5);
                    
                    if (newChat.isContinue == '_' || newChat.isContinue == 'i') {
                        ImGui::SameLine();
                        ImGui::PushItemWidth(ImGui::GetFontSize() * 8);
                        if (ImGui::Button(sendButtonName.c_str(), ImVec2(ImGui::GetContentRegionAvail().x * 0.5, ImGui::GetContentRegionAvail().y * 0.5))) {
                            if (newChat.isContinue == 'i') {
                                sendPrompt();
                            } else if (newChat.isContinue == '_') loadModel();
                        }
                        ImGui::PopItemWidth();
                    }
                    
                //} else if (newChat.isContinue == '_') loadModel();
                
                //ImGui::BeginChild("sendButtons", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y));
                // we don't need to show the button if generation is going
                 //else {
                //    newChat.getResultAsyncString();
                //}
                //ImGui::EndChild();
               ImGui::EndChild();
               }
            } //else {
// Initial buttons and settings to load a model////////////////////////////////////////////////////
                //if (newChat.isContinue == '_') {
                //    simpleStartScreen();
                    
                  
                //}// else {
    // Information while  LOADING////////////////////////////////////////////////////////////////////
                 //   loadingIndication();
                //}
            //}
        ImGui::EndChild();
        }
    }
    
    void modelToConfig(std::string& modelName) {
        if (newChat.isContinue == '_') {
            localSettings.modelName = modelName;
            // update settings based on localSettings.modelName
            localSettings.updateSettings();
            // create a localSettings.modelConfig
            localSettings.fillLocalJson();
            // create a text dump of current configs
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
    
    void modelsListSelectables(){
        ImVec2 size_selectable_chat(ImGui::GetContentRegionAvail().x, ImGui::GetTextLineHeightWithSpacing() * 1.001f);
        for (int n = 0; n < localSettings.modelsFromConfig.size(); n++) {
            if (ImGui::Selectable(localSettings.modelsFromConfig[n].second.c_str(), localSettings.modelsFromConfig[n].first == localSettings.modelName, 0, size_selectable_chat)){
                
                modelToConfig(localSettings.modelsFromConfig[n].first);
            }
            HelpTooltip(localSettings.modelsFromConfig[n].second.c_str());
        }
    }
    
    void modelsBigListSelectables(){
        ImVec2 size_selectable_chat(ImGui::GetContentRegionAvail().x, ImGui::GetTextLineHeightWithSpacing() * 1.9f);
        for (int n = 0; n < localSettings.modelsFromConfig.size(); n++) {
            std::string modelData;
            
            if (localSettings.localConfig[localSettings.modelsFromConfig[n].first].contains("format_file")) {
                modelData = "format_file: " + localSettings.localConfig[localSettings.modelsFromConfig[n].first]["format_file"].get<std::string>();
                if (localSettings.localConfig[localSettings.modelsFromConfig[n].first].contains("system_message")) modelData += "\nsystem_message: " + localSettings.localConfig[localSettings.modelsFromConfig[n].first]["system_message"].get<std::string>();
            } else if (localSettings.localConfig[localSettings.modelsFromConfig[n].first].contains("system_message")) {
                modelData = "system_message: " + localSettings.localConfig[localSettings.modelsFromConfig[n].first]["system_message"].get<std::string>();
            } else if (localSettings.localConfig[localSettings.modelsFromConfig[n].first].contains("prompt")) {
                modelData = localSettings.localConfig[localSettings.modelsFromConfig[n].first]["prompt"];
            } else modelData = "No prompt/system instruct/format!";
            //std::string modelCard = localSettings.modelsFromConfig[n].second + ": " + modelData;
            
            //ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x * 0.45);
            ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImGui::GetStyleColorVec4(ImGuiCol_MenuBarBg));
            //if (ImGui::Selectable(modelCard.c_str(), localSettings.modelsFromConfig[n].first == localSettings.modelName, 0, size_selectable_chat)){
            if (ImGui::Selectable(localSettings.modelsFromConfig[n].second.c_str(), localSettings.modelsFromConfig[n].first == localSettings.modelName, 0, size_selectable_chat)){
                
                modelToConfig(localSettings.modelsFromConfig[n].first);
                
                if (localSettings.localConfig[localSettings.modelName].contains("img")) {
                    current_image = images.get_image(localSettings.localConfig[localSettings.modelName]["img"].get<std::string>());
                }
            }
            ImGui::PopStyleColor();
            
            HelpTooltip(modelData.c_str());
        }
    }
    
    void simpleStartScreen() {
        if (use_models_list_left) ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 13);
        if (ImGui::BeginChild("Prompt for model", ImVec2( ImGui::GetContentRegionAvail().x * 0.55, ImGui::GetContentRegionAvail().y))) {
            
            if (std::filesystem::exists(localSettings.modelName)){
                if (ImGui::Button(("Load " + localSettings.modelName).c_str())) {
                    showModelsList = false;
                    show_menu_left = false;
                    arrow = ">";
                    menu = "=";
                    loadModel();
                }
                ImGui::SameLine();
                if (ImGui::Button("Open settings json")) {
                    auto jsonConfigFilePath = tinyfd_openFileDialog("Select a json file...", currPath.c_str(),1, jsonFilterPatterns, "config.json",0);
                    
                    if (jsonConfigFilePath) {
                        loadSettingsJson(jsonConfigFilePath);
                        // we probably don't want presets to be instantly applied
                        //localSettings.fillLocalJson();
                    }
                }
            } else ImGui::TextWrapped((localSettings.modelName + " doesn't exist!").c_str());

            ImGui::SeparatorText(("Instruct for " + localSettings.modelName).c_str());
            
            std::string modelData;
            
            if (localSettings.localConfig[localSettings.modelName].contains("format_file")) {
                modelData = "format_file: " + localSettings.localConfig[localSettings.modelName]["format_file"].get<std::string>();
                if (localSettings.localConfig[localSettings.modelName].contains("system_message")) modelData += "\nsystem_message: " + localSettings.localConfig[localSettings.modelName]["system_message"].get<std::string>();
            } else if (localSettings.localConfig[localSettings.modelName].contains("system_message")) {
                modelData = "system_message: " + localSettings.localConfig[localSettings.modelName]["system_message"].get<std::string>();
            } else if (localSettings.localConfig[localSettings.modelName].contains("prompt")) {
                modelData = localSettings.localConfig[localSettings.modelName]["prompt"];
            } else modelData = "No prompt/system instruct/format!";
            
            ImGui::TextWrapped(modelData.c_str());
            ImGui::Separator();
        ImGui::EndChild();
        }
        //ImGui::PopStyleColor();
        ImGui::SameLine();
        
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyleColorVec4(ImGuiCol_MenuBarBg));
        if (ImGui::BeginChild("Models list (big)")) {
            modelsBigListSelectables();
        ImGui::EndChild();
        }
        ImGui::PopStyleColor();
    }
    
    void loadSettingsJson(std::string jsonConfigFilePath) {
        nlohmann::json jsonConfigFile = getJson(jsonConfigFilePath);
        if (jsonConfigFile.contains("model")){
             localSettings.modelName = jsonConfigFile["model"];
        }
        if (jsonConfigFile.contains("name")){
             char_name = jsonConfigFile["name"];
        }
        
        localSettings.getSettingsFromJson(jsonConfigFile);
        //localSettings.getSettingsFull();
        localSettings.fillLocalJson(false);
        localSettings.updateDump();
        localSettings.syncInputs();
        
        localSettings.inputPrompt = localSettings.params.prompt;
        if(localSettings.params.antiprompt.size()) localSettings.inputAntiprompt = localSettings.params.antiprompt[0];
    }
    
    void firstSettings(const float& baseWidth){
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyleColorVec4(ImGuiCol_WindowBg));  // Set a background color
        {
            
            if (localSettings.instructFileFromJson != "NULL") {
                ImGui::TextWrapped( ("Default instruct file: " + localSettings.instructFileFromJson).c_str() );
                ImGui::Separator();
            }
            //ImGui::TextWrapped( ("Font file: " + fontFile).c_str() );
            ImGui::Spacing();
            
            if (!localSettings.noConfig){
                
                //ImGui::SameLine();
                
                if (ImGui::BeginPopup("No Model", ImGuiWindowFlags_NoSavedSettings)) {
                    ImVec2 work_size = ImGui::GetMainViewport()->WorkSize;
                    if (ImGui::BeginChild("Prompts list", ImVec2(  work_size.x * 0.5, work_size.y * 0.1))) {
                        std::string noModelMsg = localSettings.modelConfig["model"].get<std::string>() + " doesn't exist!";
                        ImGui::TextWrapped( noModelMsg.c_str() );
                    ImGui::EndChild();
                    }
                    ImGui::EndPopup();
                    
                }
            } else {
                if(localSettings.modelName == "NULL") ImGui::TextWrapped( "Choose a model first!" );
                if(localSettings.inputPrompt == "NULL") ImGui::TextWrapped( "Add prompt first! Also, add antiprompt if you don't want endless NN conversation." );
                
            }
            
            
            
            
          //ImGui::BeginChild("InitSettings");
            ImGui::TextWrapped( "Below you can set up prompt and antiprompt instead of preconfigured ones." );
            ImGui::Separator();
            ImGui::Spacing();
                
            if (localSettings.inputInstructFile == "NULL" && localSettings.instructFileFromJson != "NULL") ImGui::TextWrapped( "This model has an instruct file set for it in config. Prompt and antiprompt will not be used!" );
                
            //if (!localSettings.checkAll()) {
            
                if (ImGui::Button("Apply to config")) {
                    //localSettings.getFromJson("config.json");
                    localSettings.grammarFile = inputGrammar; 
                    localSettings.updateInput();
                    localSettings.fillLocalJson();
                    localSettings.syncInputs();
                    //localJsonDump = localSettings.modelConfig.dump(3);
                    localSettings.updateDump();
                }
                
                
                //ImGui::Separator();
            //}
            
            
            if (localSettings.inputInstructFile == "NULL") {
                
                
                ImGui::SameLine();

                if (ImGui::Button("Open an instruct file...")) {
                    
                    auto inputInstructFile = tinyfd_openFileDialog("Select an instruct file...", currPath.c_str(),1, instructFilterPatterns, NULL,0);
                    if (inputInstructFile) {
                        //localSettings.inputInstructFile = inputInstructFile;
                        
                        localSettings.readInstructFile(inputInstructFile, localSettings.inputPrompt, localSettings.inputAntiprompt);
                        //localSettings.syncInputs();
                    }
                }
                
                
                
                if (localSettings.promptFiles.size()){
                    ImGui::SameLine();
                    
                    if (ImGui::Button("Choose an instruct file...")) {
                         ImGui::OpenPopup("Prompts");
                    }
                    
                    
                    if (ImGui::BeginPopup("Prompts", ImGuiWindowFlags_NoSavedSettings))
                    {
                        ImVec2 work_size = ImGui::GetMainViewport()->WorkSize;
                        if (ImGui::BeginChild("Prompts list", ImVec2(  work_size.x * 0.3, work_size.y * 0.5))) {
                            for ( auto promptFile : localSettings.promptFiles){
                                //ImGui::TextWrapped(promptFile.filename().c_str());
                                if (ImGui::Selectable( promptFile.filename().string().c_str() )){
                                    //localSettings.inputInstructFile = promptFile.string();
                                    localSettings.readInstructFile(promptFile.string(), localSettings.inputPrompt, localSettings.inputAntiprompt);
                                    //localSettings.syncInputs();
                                }
                            }
                        ImGui::EndChild();
                        }
                        ImGui::EndPopup();
                        
                    }
                }
                
                ImGui::SameLine();
                
                if (ImGui::Button("Save current instruct...")) {
                    //ImGui::OpenPopup("Save instruct");
                    auto savedInstruct = tinyfd_saveFileDialog( localSettings.inputAntiprompt.c_str() , localSettings.promptFilesFolder.c_str() , 1 , instructFilterPatterns, NULL);
                    
                    if (savedInstruct){
                        std::string fileContents = localSettings.inputPrompt;
                        
                        int antiPos = fileContents.rfind('\n');
                        if (antiPos == fileContents.npos && localSettings.inputPrompt != localSettings.inputAntiprompt) {
                            fileContents += '\n' + localSettings.inputAntiprompt;
                        }
                        
                        //std::string filename = localSettings.promptFilesFolder + savedInstruct + ".txt";
                        
                        writeTextFileOver(savedInstruct, fileContents);
                        localSettings.getFilesList();
                    }
                    
                }
                
                ImVec2 center = ImGui::GetMainViewport()->GetCenter();
                ImVec2 work_size = ImGui::GetMainViewport()->WorkSize;
                ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

                if (ImGui::BeginPopupModal("Save instruct"))
                {
                    
                    
                    if (ImGui::BeginChild("File name", ImVec2( work_size.x * 0.5f, work_size.y * 0.5f)))
                    {
                        static std::string instructName = localSettings.inputAntiprompt;
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
                        std::string fileContents = localSettings.inputPrompt;
                        
                        int antiPos = fileContents.rfind('\n');
                        if (antiPos == fileContents.npos) {
                            fileContents += localSettings.inputAntiprompt;
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
            float initWidth = baseWidth * 0.8f;
            //ImGui::EndChild();
            ImGui::InputTextMultiline("Prompt", &localSettings.inputPrompt, ImVec2(initWidth, ImGui::GetTextLineHeight() * 15)); ImGui::SameLine(); HelpMarker( "Prompt can be used as a start of the dialog, providing context and/or format of dialog." ); 

            ImGui::InputTextMultiline("Antiprompt", &localSettings.inputAntiprompt, ImVec2(initWidth, ImGui::GetTextLineHeight() * 3)); ImGui::SameLine(); HelpMarker( "Antiprompt is needed to control when to stop generating and wait for your input." ); 

            ImGui::InputTextMultiline("Prefix", &localSettings.inputPrefix, ImVec2(initWidth, ImGui::GetTextLineHeight() * 3)); ImGui::SameLine(); HelpMarker( "Prefix sets your character for each input." );

            ImGui::InputTextMultiline("Suffix", &localSettings.inputSuffix, ImVec2(initWidth, ImGui::GetTextLineHeight() * 3)); ImGui::SameLine(); HelpMarker( "Suffix is added after your prompt - can be used to instantly set the charater for NN." );

            ImGui::InputText("Grammar file", &inputGrammar); ImGui::SameLine(); HelpMarker( "Grammars are more strict rules that help fomratting teh dialog." );
            if (ImGui::Button("Choose grammar")) {
                    inputGrammar = openGrammar();
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
            
        }
        ImGui::PopStyleColor();
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
    
    void immediateSettings(){
        //ImGui::SetNextItemAllowOverlap();
        if (ImGui::BeginChild("Immediate settings", ImVec2( ImGui::GetContentRegionAvail().x * 0.77f, fontSize + 70.0f))){ 
            ImGui::SliderInt("n_threads", &newChat.newChat.params.cpuparams.n_threads, 1, totalThreads); ImGui::SameLine(); HelpMarker("Number of threads to use for generation, doesn't have to be maximum at the moment - try to find a sweetspot.");
    
            ImGui::SliderInt("n_threads_batch", &newChat.newChat.params.cpuparams_batch.n_threads, -1, totalThreads); ImGui::SameLine(); HelpMarker("Number of threads for prompt evaluation, recommended to set to maximum.");
#if GGML_USE_CLBLAST || GGML_USE_VULKAN
            ImGui::SliderInt("n_gpu_layers", &newChat.newChat.params.n_gpu_layers, 0, 100); ImGui::SameLine(); HelpMarker("Number of layers to offload onto GPU.");
#endif
        ImGui::EndChild();
        }
        //ImGui::SetNextItemAllowOverlap();
    }
    
    void buttonsForSide() {
        if (use_models_list_left) {
            if (ImGui::Button(arrow, ImVec2(fontSize * 1.2f, fontSize * 1.2f))) {
                show_models_list = !show_models_list;
                if (show_models_list == true) arrow = "<";
                else arrow = ">";
            }
            //ImGui::SameLine();
        }
        
        // if (use_menu_left) {
            // if (ImGui::Button(menu, ImVec2(fontSize * 1.2f, fontSize * 1.2f))) {
                // show_menu_left = !show_menu_left;
                // if (show_menu_left == true) menu = "||";
                // else menu = "=";
            // }
        // }
    }
    
    void settingsHeaderButtons(float& width) {
        if (ImGui::Button("Current json")) {
            //show_json = !show_json;
            ImGui::OpenPopup("Json config");
        }
        //}
            
        ImGui::SameLine();
        //if (!tall) ImGui::SameLine();
        
        if (ImGui::Button("Text settings")) {
            //show_settings = !show_settings;
            ImGui::OpenPopup("Basic settings");
        }
            
            //ImGui::SameLine();
        ImGui::SameLine();
        if (ImGui::Button("Sampling parameters")) {
            //show_settings_advanced = !show_settings_advanced;
            ImGui::OpenPopup("Sampling settings");
        }

        ImVec2 settings_size = ImVec2(width * 0.95f, ImGui::GetTextLineHeightWithSpacing() * 33);
        if (ImGui::BeginPopup("Basic settings", ImGuiWindowFlags_NoSavedSettings)) {
            
            if (ImGui::BeginChild("Settings frame", settings_size, ImGuiWindowFlags_NoSavedSettings)){
    
                firstSettings(settings_size.x);
            
            ImGui::EndChild();
            }
        ImGui::EndPopup();
        }
        
        ImVec2 advanced_size = ImVec2(width * 0.95f, ImGui::GetTextLineHeightWithSpacing() * 33);
        //if (newChat.loaded == 9) advanced_size.y = ImGui::GetTextLineHeightWithSpacing() * 35;
        if (ImGui::BeginPopup("Sampling settings", ImGuiWindowFlags_NoSavedSettings)) {
            
            if (ImGui::BeginChild("Sampling settings frame", advanced_size, ImGuiWindowFlags_NoSavedSettings)){
    
                settingsTab();
            
            ImGui::EndChild();
            }    
        ImGui::EndPopup();
        }
        
        ImVec2 json_size = ImVec2(width * 0.95f, ImGui::GetTextLineHeightWithSpacing() * 20);
        if (ImGui::BeginPopup("Json config", ImGuiWindowFlags_NoSavedSettings)) {
            
            if (ImGui::BeginChild("Json frame", json_size, ImGuiWindowFlags_NoSavedSettings)){
    
                jsonTab();
            
            ImGui::EndChild();
            }    
        ImGui::EndPopup();
        }
    }
    
    void settingsHeader(){
        //if (ImGui::BeginChild("Header", ImVec2( ImGui::GetContentRegionAvail().x * 0.67f, fontSize + 7.0f))) {
            
            
            
            ImGui::Checkbox("Chat mode", &chatMode); 
            
            ImGui::SameLine();
            
            ImGui::Checkbox("Autoscroll", &autoscroll);
            
            // ImGui::EndChild();
        // }
    }
    
    void pause() {
        newChat.pause();
        tokens_this_session = newChat.last_tokens;
        consumed_this_session = newChat.consumed_tokens;
        past_this_session = newChat.past_tokens;
        Test.cycling = 0;
        cancelled = true;
    }
    
    void unload() {
        newChat.isUnload = 'y';
        newChat.loaded = 0;
        tokens_this_session = 0;
        regen_cycles = 0;
        //copiedDialog = false;
        //copiedSettings = false;
        //copiedTimings = false;
        //initTokens = false;
        cancelled = false;

        hasModel = false;
        newChat.unload();
        
        sendButtonName = "Load";
        localResultPairs.clear();
    }
    
    void pauseAndUnload() {
        newChat.loaded = 0;
        tokens_this_session = 0;
        regen_cycles = 0;
        // copiedDialog = false;
        // copiedSettings = false;
        // copiedTimings = false;
        // initTokens = false;
        cancelled = false;

        hasModel = false;
        newChat.isUnload = 'y';
        newChat.pause();
        
        sendButtonName = "Load";
        localResultPairs.clear();
    }
    
    void inferencePlot() {
        static float values[90] = {};
        static int values_offset = 0;
        static float maxSpeed = 10.0f;
        static double refresh_time = 0.0;
        while (refresh_time < ImGui::GetTime()) // Create data at fixed 30 Hz rate for the demo
        {
            static float phase = 0.0f;
            values[values_offset] = newChat.lastSpeed;
            if (newChat.lastSpeed > maxSpeed) maxSpeed = newChat.lastSpeed;
            values_offset = (values_offset + 1) % IM_ARRAYSIZE(values);
            phase += 0.10f * values_offset;
            refresh_time += 1.0f / 30.0f;
        }

        // Plots can display overlay texts
        // (in this example, we will display an average value)
        {
            float average = 0.0f;
            for (int n = 0; n < IM_ARRAYSIZE(values); n++)
                average += values[n];
            average /= (float)IM_ARRAYSIZE(values);
            char overlay[8];
            ImGui::SameLine();
            sprintf(overlay, "%f", average);
            ImGui::PlotLines("###speed", values, IM_ARRAYSIZE(values), values_offset, overlay, 0.0f, maxSpeed, ImVec2(ImGui::GetContentRegionAvail().x * 0.4, fontSize + 3.0f));
        }
    }
    
    void infoButtonsHeader() {
        if (ImGui::BeginChild("InfoAndButtons")) {
            if (newChat.loaded == 9){
                inferencePlot();
                //ImGui::Spacing();
                ImGui::SameLine();
                
                if (newChat.isContinue == 'w') {
                    //ImGui::SameLine();
                    if (ImGui::Button("Pause")){ 
                        pause();
                    }
                    ImGui::SameLine();
                } else {
                    if(cancelled){
                        //ImGui::SameLine();
                        if (ImGui::Button("Resume")) {
                        
                            //newChat.appendQuestion(inputStr);
                            localResultPairs = newChat.resultsStringPairs;
                            //newChat.getResultAsyncString();
                            //inputStr = "";
                            //inputChars = inputStr.c_str();
                            helpLabel = "...";
                            
                            
                            //newChat.isContinue = 'w';
                            newChat.continueGen();
                            output = "...";
                            newChat.getResultAsyncStringRepeat();
                            copiedDialog = false;
                            scrolled = false;
                            cancelled = false;
                            
                        }
                        ImGui::SameLine();
                    }
                }
                
                
                if (ImGui::Button("Unload")){
                        //safeguard
                        if (newChat.loaded != 0){
                            if (newChat.isContinue == 'w') pauseAndUnload();
                            else unload();
                        }
                    }
                //ImGui::SameLine();
                ImGui::TextWrapped(("Tokens: " + std::to_string(past_this_session)).c_str());
                
            } else if (newChat.isContinue == '_') {
                //ImGui::Checkbox("Show list of models", &showModelsList);
                //ImGui::TextWrapped((localSettings.modelName).c_str());
            }
            
            ImGui::EndChild();
        }
    }
    
    void testsTab() {
        if(ImGui::BeginChild("Test")) {
                
            if (Test.cycling == 0){
            
                if (ImGui::Button("Open")){ 
                    auto presetTestFilePath = tinyfd_openFileDialog("Select a test file...", currPath.c_str(),1, jsonFilterPatterns, NULL,0);
        
                    if (presetTestFilePath) {
                        testJson = getJson(presetTestFilePath);
                        if (testJson.contains("prompt")) {
                            testPrompt = testJson["prompt"];
                        }
                        if (testJson.contains("folder")) {
                            testFolder = testJson["folder"];
                        }
                        if (testJson.contains("presets")){
                            if (testJson["presets"].is_array()){
                                testPresets = testJson["presets"];
                            }
                        }
                    }
                }
                ImGui::SameLine();
                if (ImGui::Button("Load")){
                    if (std::filesystem::exists("presetsTest.json")){
                        testJson = getJson("presetsTest.json");
                        if (testJson.contains("prompt")) {
                            testPrompt = testJson["prompt"];
                        }
                        if (testJson.contains("folder")) {
                            testFolder = testJson["folder"];
                        }
                        if (testJson.contains("presets")){
                            if (testJson["presets"].is_array()){
                                testPresets = testJson["presets"];
                            }
                        }
                    } else {
                        ImGui::TextWrapped("presetsTest.json is missing! Open a presets test file.");
                    }
                }
                
                ImGui::SameLine();
                if (ImGui::Button("Start")){
                    //Test.startTest(testJson, localSettings, newChat, false, false, false);
                    testJson["prompt"] = testPrompt;
                    testJson["folder"] = testFolder;
                    Test.init(testJson);
                    localSettings.modelConfig["card"] = Test.presetsNames[Test.cycle];
                    localSettings.modelConfig["seed"] = Test.seed;
                    newChat.load(localSettings.modelConfig, false);
                    Test.cycling = 1;
                    
                    
                }
                
                //ImGui::TextWrapped(testPrompt.c_str());
                ImGui::InputTextMultiline("input text", &testPrompt);
                ImGui::InputText("folder", &testFolder);
                
                
                for (auto preset : testPresets){
                    if (std::filesystem::exists(preset+".json")){
                        if (ImGui::Button(preset.c_str())){ 
                            nlohmann::json presetCardFile = getJson(preset);
                            localSettings.getSettingsFromJson(presetCardFile);
                        }
                    } else ImGui::Text((preset + " doesnt's exist!").c_str());
                }
            
            } else {
                if (newChat.loaded == 9) {
                    ImGui::Text(("Current prompt: " + Test.prompt).c_str());
                    ImGui::Text(("Current preset: " + Test.presetsNames[Test.cycle]).c_str());
                    if (newChat.isContinue != 'w') {
                        if (!isGeneratingTest){
                            newChat.appendQuestion(Test.prompt);
                            remove_last_nl(Test.prompt);
                            localResultPairs = newChat.resultsStringPairs;
                            newChat.startGen();
                            //newChat.getResultAsyncStringFull2(false, true);
                            newChat.runGenerationAsync();
                            isGeneratingTest = true;
                        } else {
                            newChat.writeTextFileFull(Test.saveFolder + '/', Test.writeName());
                            newChat.loaded = 0;
                            tokens_this_session = 0;
                            copiedDialog = false;
                            copiedSettings = false;
                            copiedTimings = false;
                            initTokens = false;
                            cancelled = false;

                            hasModel = false;
                            newChat.unload();
                            Test.cycle++;
                            if (Test.cycle == Test.presetsNames.size()) {
                                Test.cycling = 0;
                            } else {
                                localSettings.modelConfig["card"] = Test.presetsNames[Test.cycle];
                                localSettings.modelConfig["seed"] = Test.seed;
                                newChat.load(localSettings.modelConfig, false);
                                isGeneratingTest = false;
                            }
                        }
                    }
                } else ImGui::Text("Loading...");
            }
        
            
        ImGui::EndChild();
        }
    }
    
    void menuBar(SDL_Window* window) {
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File..."))
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
                if (ImGui::MenuItem("Save settings config", "Ctrl+S")) {
                    nlohmann::json settingsJson;
                    SDL_GL_GetDrawableSize(window, &width, &height);
                    
                    settingsJson["width"] = width;
                    settingsJson["height"] = height;
                    settingsJson["font"] = fontFile;
                    settingsJson["fontEmojis"] = fontEmojisFile;
                    settingsJson["fontSize"] = fontSize;
                    if(modelsFolderName != "NULL") settingsJson["modelsFolder"] = modelsFolderName;
                    settingsJson["promptsFolder"] = localSettings.promptFilesFolder;
                    settingsJson["theme"] = themeIdx;
                    
                    ImGuiStyle& style = ImGui::GetStyle();
                    
                    
                    
                    settingsJson["style"]["AntiAliasedLines"] = style.AntiAliasedLines;
                    settingsJson["style"]["AntiAliasedLinesUseTex"] = style.AntiAliasedLinesUseTex;
                    settingsJson["style"]["AntiAliasedFill"] =  style.AntiAliasedFill;
                   
                    settingsJson["style"]["FrameBorderSize"] =  style.FrameBorderSize;
                    settingsJson["style"]["ChildBorderSize"] =  style.ChildBorderSize;
                    settingsJson["style"]["WindowPadding.x"] =  style.WindowPadding.x;
                    settingsJson["style"]["CellPadding.x"] = style.CellPadding.x;
                    settingsJson["style"]["ItemSpacing.x"] = style.ItemSpacing.x;
                    settingsJson["style"]["ChildRounding"] = style.ChildRounding;
                    settingsJson["style"]["FrameRounding"] = style.FrameRounding;
                    settingsJson["style"]["PopupRounding"] = style.PopupRounding;
                    settingsJson["style"]["TabRounding"] = style.TabRounding;
                    settingsJson["style"]["GrabRounding"] = style.GrabRounding;
                    settingsJson["style"]["WindowRounding"] = style.WindowRounding;
                    settingsJson["style"]["FramePadding.x"] = style.FramePadding.x;
                    
                    writeJson(settingsJson, "chatConfig.json");

                }
                if (ImGui::MenuItem("Save model config", "Ctrl+S"))   { 

                    localSettings.pushToMainConfig();
                    writeJson(localSettings.localConfig, "config.json");
                     
                }

                
                ImGui::EndMenu();
            }
            
            if (ImGui::BeginMenu("Themes..."))
            {
                
                if (ImGui::MenuItem("Dark theme")) {
                   ImGui::StyleColorsDark();
                }
                if (ImGui::MenuItem("Light theme")) {
                   ImGui::StyleColorsLight();
                }
                if (ImGui::MenuItem("Classic theme")) {
                   ImGui::StyleColorsClassic();
                }
                if (ImGui::MenuItem("Retro theme")) {
                   retroTheme();
                }
                
                ImGui::EndMenu();
            }
            
            if (ImGui::BeginMenu("Utilities..."))
            {
                if (ImGui::MenuItem("Templates")) {
                    show_templates = !show_templates;
                }
                
                if (ImGui::MenuItem("Prompts database")) {
                    show_prompts_db = !show_prompts_db;
                }
                
                if (ImGui::MenuItem("Cyclic tests")) {
                    show_tests = !show_tests;
                }
                
                // if (ImGui::MenuItem("Prompts history")) {
                    // show_history = !show_history;
                // }
                
                if (ImGui::MenuItem("Open a font file")) {
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
            
            if (ImGui::BeginMenu("Settings..."))
            {
                
                ImGui::CheckboxFlags("Send messages with Enter", &inputFlags, ImGuiInputTextFlags_CtrlEnterForNewLine);
                
                ImGui::EndMenu();
            }
            
            // ImGui::SameLine();
            // if (ImGui::BeginChild("model display"))
            // {
                
                
            if (newChat.isContinue == '_') {
            
                if (ImGui::Selectable((" | Press to see the list | Selected: " + localSettings.modelName).c_str())) {
                    showModelsList = !showModelsList;
                }
            } else ImGui::TextWrapped(("|      Model: " + localSettings.modelName).c_str());
            // ImGui::EndChild();
            // }
            ImGui::EndMenuBar();
        }
    }
    
    void menuSide(SDL_Window* window) {
        //if (ImGui::BeginChild("Menu", ImVec2(ImGui::GetContentRegionAvail().x * 0.2f, ImGui::GetContentRegionAvail().y)))
        if (ImGui::BeginChild("Menu", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y)))
        {
            // if (newChat.loaded == 9 && show_profile){
                // if (ImGui::BeginChild("Profile shift", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetTextLineHeightWithSpacing() * 4.8f))) {
                    
                    // ImGui::EndChild();
                // }
            // }
            
            if (ImGui::CollapsingHeader("File...", ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_DefaultOpen))
            {
                if (ImGui::Button("Open a model")) {
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
                if (ImGui::Button("Select models folder"))   {
                     auto getModelFolderName = tinyfd_selectFolderDialog("Select a model...", currPath.c_str());
                     if (getModelFolderName) {
                         modelsFolderName = getModelFolderName;
                         modelsFolderName += "\\";
                     }
                }
                
                if (ImGui::Button("Select prompts folder"))   { 
                     auto getPromptsFolderName = tinyfd_selectFolderDialog("Select a model...", currPath.c_str());
                     if (getPromptsFolderName) {
                         localSettings.promptFilesFolder = getPromptsFolderName;
                         localSettings.promptFilesFolder += "\\";
                         localSettings.getFilesList();
                     }
                }
                
                ImGui::Checkbox("Demo Window", &show_demo_window); 
                
            }
            
            if (ImGui::CollapsingHeader("Save...", ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_DefaultOpen))
            {
                if (ImGui::Button("Save settings config")) {
                    nlohmann::json settingsJson;
                    SDL_GL_GetDrawableSize(window, &width, &height);
                    
                    settingsJson["width"] = width;
                    settingsJson["height"] = height;
                    settingsJson["font"] = fontFile;
                    settingsJson["fontEmojis"] = fontEmojisFile;
                    settingsJson["fontSize"] = fontSize;
                    if(modelsFolderName != "NULL") settingsJson["modelsFolder"] = modelsFolderName;
                    settingsJson["promptsFolder"] = localSettings.promptFilesFolder;
                    settingsJson["theme"] = themeIdx;
                    
                    ImGuiStyle& style = ImGui::GetStyle();
                    
                    
                    
                    settingsJson["style"]["AntiAliasedLines"] = style.AntiAliasedLines;
                    settingsJson["style"]["AntiAliasedLinesUseTex"] = style.AntiAliasedLinesUseTex;
                    settingsJson["style"]["AntiAliasedFill"] =  style.AntiAliasedFill;
                   
                    settingsJson["style"]["FrameBorderSize"] =  style.FrameBorderSize;
                    settingsJson["style"]["ChildBorderSize"] =  style.ChildBorderSize;
                    settingsJson["style"]["WindowPadding.x"] =  style.WindowPadding.x;
                    settingsJson["style"]["CellPadding.x"] = style.CellPadding.x;
                    settingsJson["style"]["ItemSpacing.x"] = style.ItemSpacing.x;
                    settingsJson["style"]["ChildRounding"] = style.ChildRounding;
                    settingsJson["style"]["FrameRounding"] = style.FrameRounding;
                    settingsJson["style"]["PopupRounding"] = style.PopupRounding;
                    settingsJson["style"]["TabRounding"] = style.TabRounding;
                    settingsJson["style"]["GrabRounding"] = style.GrabRounding;
                    settingsJson["style"]["WindowRounding"] = style.WindowRounding;
                    settingsJson["style"]["FramePadding.x"] = style.FramePadding.x;
                    
                    writeJson(settingsJson, "chatConfig.json");

                }
                if (ImGui::Button("Save model config"))   { 

                    localSettings.pushToMainConfig();
                    writeJson(localSettings.localConfig, "config.json");
                     
                }
            }
            
            if (ImGui::CollapsingHeader("Themes...", ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_DefaultOpen))
            {
                
                if (ImGui::Button("Dark theme")) {
                   ImGui::StyleColorsDark();
                }
                if (ImGui::Button("Light theme")) {
                   ImGui::StyleColorsLight();
                }
                if (ImGui::Button("Classic theme")) {
                   ImGui::StyleColorsClassic();
                }
                if (ImGui::Button("Retro theme")) {
                   retroTheme();
                }
            }
            
            if (ImGui::CollapsingHeader("Utilities...", ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_DefaultOpen))
            {
                if (ImGui::Button("Templates")) {
                    show_templates = !show_templates;
                }
                
                if (ImGui::Button("Prompts database")) {
                    show_prompts_db = !show_prompts_db;
                }
                
                if (ImGui::Button("Cyclic tests")) {
                    show_tests = !show_tests;
                }
                
                // if (ImGui::MenuItem("Prompts history")) {
                    // show_history = !show_history;
                // }
                
                if (ImGui::Button("Open a font file")) {
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
            }
            
            if (ImGui::CollapsingHeader("Settings...", ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_DefaultOpen))
            {
                
                ImGui::CheckboxFlags("Send by Enter", &inputFlags, ImGuiInputTextFlags_CtrlEnterForNewLine);
                
            }
            
            ImGui::EndChild();
        }
    }
    
    
    // void profileHeaderWindow(float& imageSide, ImVec2& pose) {
        // ImVec2 profile_size = ImVec2(ImGui::GetContentRegionAvail().x, imageSide + ImGui::GetStyle().ItemSpacing.y * 4);
        
        // ImGui::SetNextWindowPos(ImVec2(pose.x + imageSide + ImGui::GetStyle().ItemSpacing.x, pose.y), ImGuiCond_Appearing, ImVec2(0.0f, 0.0f));
        // if (!ImGui::Begin("profileHeader window", &show_profile, overlay_window_flags))
        // {
            // ImGui::End();
            // return;
        // }
        
        
        // ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyleColorVec4(ImGuiCol_MenuBarBg));
        // ImGui::BeginChildFrame(ImGui::GetID("profileHeader frame"), profile_size, ImGuiWindowFlags_NoSavedSettings);
        // profileBar(profile_size);
        // ImGui::EndChildFrame();
        // ImGui::PopStyleColor();
        
        // ImGui::End();
    // }
    
    void profileHeader() {
        
        bool showPicture = newChat.loaded == 9;
        auto imageSide = ImGui::GetTextLineHeightWithSpacing() * 4;
        ImVec2 pose = ImVec2(ImGui::GetStyle().ItemSpacing.x, ImGui::GetTextLineHeightWithSpacing() + ImGui::GetStyle().ItemSpacing.y * 4);
        ImVec2 size;
        // suffix text field is (ImGui::GetTextLineHeightWithSpacing() * 4) high, like imageSide
        // need to accomodate that with spacing
        size.y = imageSide + ImGui::GetStyle().ItemSpacing.y * 5;
            
        if (show_profile) size.x = ImGui::GetContentRegionAvail().x;
        else size.x = imageSide + ImGui::GetStyle().ItemSpacing.x * 5;
        ImGui::SetNextWindowPos(pose, ImGuiCond_Appearing, ImVec2(0.0f, 0.0f));
        ImGui::SetNextWindowSize(size);
        if (!ImGui::Begin("profilePicture window", &showPicture, overlay_window_flags))
        {
            ImGui::End();
            return;
        }
        
        // ImGui::BeginChildFrame(ImGui::GetID("profilePicture frame"), size, ImGuiWindowFlags_NoSavedSettings);
        float realImgSide = imageSide * 0.95;
#if defined(SDL2)
        if (ImGui::ImageButton("", (void*)current_image, ImVec2(realImgSide, realImgSide))) {
#else
        if (ImGui::ImageButton("", (ImTextureID)current_image->DS, ImVec2(realImgSide, realImgSide))) {
#endif
            show_profile = !show_profile;
        }
        if (show_profile && !localResultPairs.empty()) {
            ImGui::SameLine();
            // suffixEdit("##Character");
            // static std::string profileName = "Profile instruct";
            // ImGui::SameLine();
            // contextInstruct(profileName);
            //profileBar(size);
            if (ImGui::BeginChild("ProfileBar", ImVec2(size.x - imageSide, size.y * 0.85))){
            
                if (ImGui::BeginChild("Profile suffix", ImVec2( ImGui::GetContentRegionAvail().x * 0.25, ImGui::GetContentRegionAvail().y))){
                    suffixEdit("##Character");
                ImGui::EndChild();
                }
                static std::string profileName = "Profile";
                //contextInstruct(profileName);
                //static int idx = 0;
                ImGui::SameLine();
                //messageAuto(localResultPairs[0].second, idx);
                contextInstruct(profileName);
                
            ImGui::EndChild();
            }
        }
        
        //profileBar(size);
        // ImGui::EndChildFrame();
        ImGui::End();
    }
    
    void profileImage() {
        float realImgSide = ImGui::GetTextLineHeightWithSpacing() * 2.1;
#if defined(SDL2)
        if (ImGui::ImageButton("", (void*)current_image, ImVec2(realImgSide, realImgSide))) {
#else
        if (ImGui::ImageButton("", (ImTextureID)current_image->DS, ImVec2(realImgSide, realImgSide))) {
#endif
            if (newChat.loaded == 9) show_profile = !show_profile;
        }
    }
    
    void profileHeaderImageless() {
        
        auto imageSide = ImGui::GetTextLineHeightWithSpacing() * 4;
        ImVec2 pose = ImVec2(ImGui::GetStyle().ItemSpacing.x, ImGui::GetTextLineHeightWithSpacing() * 2.8f);
        if (use_menu_bar) pose.y += ImGui::GetTextLineHeightWithSpacing();
        ImVec2 size;
        size.y = imageSide + ImGui::GetStyle().ItemSpacing.y * 5;
        size.x = ImGui::GetContentRegionAvail().x;
        ImGui::SetNextWindowPos(pose, ImGuiCond_Appearing, ImVec2(0.0f, 0.0f));
        ImGui::SetNextWindowSize(size);
        if (!ImGui::Begin("profilePicture window", &show_profile, overlay_window_flags))
        {
            ImGui::End();
            return;
        }
        
        // ImGui::BeginChildFrame(ImGui::GetID("profilePicture frame"), size, ImGuiWindowFlags_NoSavedSettings);
        if (!localResultPairs.empty()) {

            if (ImGui::BeginChild("ProfileBar", ImVec2(size.x, size.y * 0.85))){
            
                if (ImGui::BeginChild("Profile suffix", ImVec2( ImGui::GetContentRegionAvail().x * 0.25, ImGui::GetContentRegionAvail().y))){
                    suffixEdit("##Character");
                ImGui::EndChild();
                }
                static std::string profileName = "Profile";
                //contextInstruct(profileName);
                //static int idx = 0;
                ImGui::SameLine();
                //messageAuto(localResultPairs[0].second, idx);
                contextInstruct(profileName);
                
            ImGui::EndChild();
            }
        }
        ImGui::End();
    }
    
    void menuOverlay(SDL_Window* window) {
        ImVec2 pose = ImVec2(13, ImGui::GetContentRegionAvail().y * 0.2f);
        ImVec2 size = ImVec2(ImGui::GetContentRegionAvail().x * 0.3f, ImGui::GetContentRegionAvail().y * 0.75f);
        
        ImGui::SetNextWindowPos(pose, ImGuiCond_Appearing, ImVec2(0.0f, 0.0f));
        ImGui::SetNextWindowSize(size);
        
        ImGui::PushStyleColor(ImGuiCol_WindowBg, IM_COL32(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(180, 180, 180, 255));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(180, 180, 180, 255));
        ImGui::PushStyleColor(ImGuiCol_Border, IM_COL32(0, 0, 0, 0));
        
        if (!ImGui::Begin("menu window", &show_menu_left, overlay_window_flags))
        {
            ImGui::End();
            return;
        }
        
        //ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(0, 0, 0, 0));
        menuSide(window);
        //ImGui::PopStyleColor();
        ImGui::End();
        
        ImGui::PopStyleColor(4);
    }
    
    void showMenuButton() {
        if (use_menu_left) {
            ImVec2 pose = ImVec2(0, ImGui::GetContentRegionAvail().y * 0.35f);
            ImVec2 size = ImVec2(fontSize, ImGui::GetContentRegionAvail().y * 0.6f);
            
            ImGui::SetNextWindowPos(pose, ImGuiCond_Appearing, ImVec2(0.0f, 0.0f));
            ImGui::SetNextWindowSize(size);
            ImGui::PushStyleColor(ImGuiCol_WindowBg, IM_COL32(0, 0, 0, 0));
            ImGui::PushStyleColor(ImGuiCol_Border, IM_COL32(0, 0, 0, 0));
            
            if (!ImGui::Begin("menu button", &show_menu_left, overlay_window_flags))
            {
                ImGui::End();
                return;
            }
            ImGui::PushStyleColor(ImGuiCol_Border, IM_COL32(0, 0, 0, 255));
            ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(255, 255, 255, 255));
            if (ImGui::Button("|", ImVec2(12, ImGui::GetContentRegionAvail().y * 0.6f))) {
                show_menu_left = !show_menu_left;
            }
            ImGui::PopStyleColor(2);
            ImGui::End();
            
            ImGui::PopStyleColor(2);
        }
    }
    
    void headerPlus() {
        auto windowWidth = ImGui::GetWindowWidth();
        buttonsForSide();
        ImGui::SameLine();
        //if (newChat.loaded == 9) {
        ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_Border, IM_COL32(0, 0, 0, 0));
        //ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(0, 0, 0, 0));

        if (newChat.loaded == 9) ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(2, 200, 2, 100));
        else if (newChat.isContinue != '_') ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(2, 2, 200, 100));
        
        if (ImGui::BeginChild("profile picture", ImVec2(ImGui::GetTextLineHeightWithSpacing() * 6.0f, ImGui::GetTextLineHeightWithSpacing() * 2.4f))){
            profileImage();
            ImGui::SameLine();
            if (char_name.empty()) {
                if (newChat.loaded == 9) ImGui::Text("ONLINE");
                else if (newChat.isContinue != '_') ImGui::Text("LOADING");
                else ImGui::Text("OFFLINE");
            } else {
                if (newChat.loaded == 9) ImGui::TextWrapped((char_name + " is online").c_str());
                else if (newChat.isContinue != '_') ImGui::TextWrapped((char_name + " is loading").c_str());
                else ImGui::TextWrapped((char_name + " is offline").c_str());
            }
            
        ImGui::EndChild();
        }
        
        if (newChat.loaded == 9 || newChat.isContinue != '_') ImGui::PopStyleColor();
        
        ImGui::PopStyleColor(3);
        
        ImGui::SameLine();

        if (ImGui::BeginChild("HeaderTaller", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetTextLineHeightWithSpacing() * 2.4f))){
            if (ImGui::BeginChild("settingsAndButtonsHeaders", ImVec2(ImGui::GetContentRegionAvail().x * 0.6f, ImGui::GetContentRegionAvail().y))){
                settingsHeader();
                
                settingsHeaderButtons(windowWidth);
            ImGui::EndChild();
            }
            //if (newChat.loaded == 9) {
                ImGui::SameLine();
                infoButtonsHeader();
            //}
            
            //buttonsHeader();
        ImGui::EndChild();
        }
    }

    void fillImagesData() {
        nlohmann::json tmp;
        if (std::filesystem::exists("characters")){
            for (const auto& dirEntry : std::filesystem::recursive_directory_iterator("characters")) {
                tmp = getJson(dirEntry.path().string());
                if (tmp.contains("img") && tmp["img"].is_string()) {
                    images.add(tmp["img"].get<std::string>());
                }
            }
        }
        if (localSettings.localConfig[localSettings.modelName].contains("img")) 
            images.add(localSettings.localConfig[localSettings.modelName]["img"].get<std::string>());
    }
    
#if defined(SDL2)
    void preloadImage(SDL_Renderer* renderer) {
        //bool ret = LoadTextureFromFile("default.png", &my_texture, my_image_width, my_image_height, renderer);
        images.load_image(renderer, "default.png");
        current_image = images.default_image;
        if (!images.data.empty() && images.data.size() == images.sizes.size()) {
            images.load_images(renderer);
        }

        if (localSettings.localConfig[localSettings.modelName].contains("img")) {
            current_image = images.get_image(localSettings.localConfig[localSettings.modelName]["img"].get<std::string>());
        } else current_image = images.default_image;
        
#else
    void preloadImage() {
        // bool ret = LoadTextureFromFile("default.png", &my_texture);
        // IM_ASSERT(ret);
        images.load_image("default.png");

        if (!images.data.empty()) {
            printf("Preloading images:");
            images.load_images();
        }

        // if (localSettings.localConfig[localSettings.modelName].contains("img")) {
            // current_image = images.get_image(localSettings.localConfig[localSettings.modelName]["img"].get<std::string>());
        // } else 
            current_image = &images.default_image;
#endif
    }
    
    void mainUI(const ImGuiViewport* viewport, SDL_Window* window){
        ImGui::Begin("Chat", NULL, chat_window_flags); 
        if (use_menu_bar) {
            chat_window_flags |= ImGuiWindowFlags_MenuBar;
            menuBar(window);
        }
        
        
        // top headers
        //ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyleColorVec4(ImGuiCol_MenuBarBg));
        headerPlus();
        // settingsHeader();
        // ImGui::SameLine();
        // buttonsHeader();
        // if (newChat.loaded == 9) {
            
        // }
        //ImGui::PopStyleColor();
        
        // profile header
        if (newChat.loaded == 9) {
            //profileHeader();
            if (show_profile) profileHeaderImageless();
        }
        
        //models of the left
        if (use_models_list_left) {
            //if (show_models_list) {
                ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyleColorVec4(ImGuiCol_MenuBarBg));
                ImGui::SetNextItemAllowOverlap();
                if (ImGui::BeginChild("Chats list", ImVec2( ImGui::GetContentRegionAvail().x * 0.2f, ImGui::GetContentRegionAvail().y))) {
                    
                    
                    modelsListSelectables();
                    
                    
                    ImGui::EndChild();
                }
                ImGui::PopStyleColor();
                
                ImGui::SameLine();
            //}
        }
        //menu of the left
        if (use_menu_left) {
            showMenuButton();
            if (show_menu_left){
                // ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyleColorVec4(ImGuiCol_MenuBarBg));
                // menuSide(window);
                // ImGui::PopStyleColor();
                // ImGui::SameLine();
                menuOverlay(window);
            }
        }
        // main dialog
        if (ImGui::BeginChild("Dialog tab")){
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyleColorVec4(ImGuiCol_PopupBg));
            dialogTab(viewport);
            ImGui::PopStyleColor();
            ImGui::EndChild();
        }
        
        // if (use_models_list_right) {
            // if (show_models_list) {
                // ImGui::SameLine();
                // ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyleColorVec4(ImGuiCol_MenuBarBg));
                // if (ImGui::BeginChild("Chats list", ImVec2( ImGui::GetContentRegionAvail().x * 0.2f, ImGui::GetContentRegionAvail().y))) {
                    
                    
                    // modelsListSelectables();
                    
                    
                    // ImGui::EndChild();
                // }
                // ImGui::PopStyleColor();
                
                
            // }
        // }
        
        ImGui::End();
        
        
        
    }
    
    void getCharData(std::string& modelName) {
        if (localSettings.modelConfig.contains("name") && localSettings.modelConfig["name"].is_string()) 
                    char_name = localSettings.modelConfig["name"];

        if (localSettings.modelConfig.contains(modelName)){

            if (localSettings.modelConfig[modelName].contains("prompt") && localSettings.modelConfig[modelName]["prompt"].is_string()) {
                char_profile = localSettings.modelConfig[modelName]["prompt"];
                if (!char_profile.empty() && localSettings.modelConfig[modelName].contains("reverse-prompt") && localSettings.modelConfig[modelName]["reverse-prompt"].is_string()) {
                    size_t pos = char_profile.rfind(localSettings.modelConfig[modelName]["reverse-prompt"]);
                    if (pos != char_profile.npos) char_profile = char_profile.substr(0,pos);
                }
            }
            
            if (localSettings.modelConfig[modelName].contains("input_suffix") && localSettings.modelConfig[modelName]["input_suffix"].is_string()) 
                char_system_name = localSettings.modelConfig[modelName]["input_suffix"];
            
            if (!char_profile.empty() && !char_system_name.empty()) 
                char_start = getSubprofile();
        }
    }
    
    void loadModel(){
        if (std::filesystem::exists(localSettings.modelConfig["model"])){
            newChat.jsonConfig = localSettings.modelConfig;
            
            newChat.load();
            
            hasModel = true;
            copiedDialog = false;
            copiedSettings = false;
            copiedTimings = false;
            initTokens = false;
            newChat.isContinue = 'l';
            sendButtonName = "Send";
            
            std::string modelName = localSettings.modelConfig["model"];
            getCharData(modelName);
            
        } else {
            ImGui::OpenPopup("No Model");
        }
    }
    
    void init(std::string configName = "config.json"){
        localSettings.modelFromJson = getStringFromJson(configName,"model");
        if (promptsFolderName != "NULL") localSettings.promptFilesFolder = promptsFolderName;
        
        // scan existing models from .json
        localSettings.getFilesList();
        // copy from current config to localSettings.localConfig 
        localSettings.getFromJson(configName);
        // get settings from that json 
        localSettings.getSettingsFull();
        // fill in params and json dumps
        modelToConfig(localSettings.modelFromJson);
        
        fillImagesData();

        switch (themeIdx)
        {
            case 0: ImGui::StyleColorsDark(); break;
            case 1: ImGui::StyleColorsLight(); break;
            case 2: ImGui::StyleColorsClassic(); break;
            case 3: retroTheme(); break;
            default: retroTheme(); break;
        }

        if (localSettings.modelsFromConfig.size() > 0){
            for (int n = 0; n < localSettings.modelsFromConfig.size(); n++){
                if (localSettings.modelName == localSettings.modelsFromConfig[n].first) {
                    mdlIdx = n;
                    
                    break;
                }
            }
        }
        
        // check if the settigns are valid
        localSettings.checkLocalConfig();
    }
    
    void stopOnCheck() {
        if (newChat.isContinue == 'l') newChat.isUnload = 'y';
        else if (newChat.loaded != 0){
            if (newChat.isContinue == 'w') {
                pauseAndUnload();
            } else {
                unload();
            }
        }
    }
    
};