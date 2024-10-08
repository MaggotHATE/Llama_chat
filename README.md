# Llama_chat
**A chat UI for Llama.cpp.**

![image](https://github.com/MaggotHATE/Llama_chat/blob/main/pics/Llama_chat.PNG)

A highly configurable chat application for running LLMs, based on [llama.cpp](https://github.com/ggerganov/llama.cpp).

This project started from the main example of llama.cpp - the idea was to read parameters from .json files. Now it's a class, separate threads for running llama.cpp, structs for managing and settings - and only then wrapped into UI.

Additionally, chatTest console app is also added for testing purposes, since certain information (like memory used by the model) is printed outside of the original example (in llama.cpp, common.cpp, etc.).

## Features

* Model settings management through config.json file
* UI settings management through chatConfig.json file
* Global and per-model settings
* Quick model select according to config.json
* "Templates" (prompts) with a separate .json file to store, load and quickly use in chat ui.
* "Characters" with their own .json files, partial compatibility with SillyTavern format.
* Assignable folders for models and prompt files
* Quick select of prompt files before loading a model
* Prompts history within one session
* Last result regeneration (still WIP)

### About chatTest

ChatTest is a debugging-oriented version that uses the same internal processing, but also allows bulk generation tests as a way of evaluating models and prompts. After loading a model, input one of these instead of dialog.

* cycle - generates X amount of results by a given prompt. It also has wildcards support for additional randomization. See sci.json as an example.
* test - generates results for each given params preset on the same prompt and seed. See presetsTest.json as an example, which will be used by default.
* regens - generates 20 results by a given prompt, but using regeneration of the last result. You can also drag-n-drop a text file with a prompt, but the file name needs to have "regens.txt" at the end.

In first two cases it will ask you to enter file name or a prompt, cycle will also ask for a custom amount of cycles if no file given. Regens is instant as it relies on the prompt. All results are saved in separate text files inside `tales` folder which needs to be created manually.

## Building

Tested on Windows only for now. AVX2 releases only for now, older releases were AVX only.

### Requirements

Code:
* tinyfiledialogs https://sourceforge.net/projects/tinyfiledialogs/
* imgui https://github.com/ocornut/imgui

Libraries:
* Vulkan SDK (installer is preferred) https://vulkan.lunarg.com/#new_tab
* SDL2 https://github.com/libsdl-org/SDL
* If OpenBLAS is needed: use [releases](https://github.com/OpenMathLib/OpenBLAS/releases) and [Clblast](https://github.com/CNugteren/CLBlast).
* If CLBLAST is needed: use releases of [OpenCL SDK](https://github.com/KhronosGroup/OpenCL-SDK) and [Clblast](https://github.com/CNugteren/CLBlast).

### Building on Windows

1. Use `git clone https://github.com/MaggotHATE/Llama_chat`, or download this repo and unpack it
2. Download w64devkit https://github.com/skeeto/w64devkit/releases and unpack it to a desireable folder
3. Install all prerequisite libraries according to w64devkit installation: in most cases it means copying `lib` and `include` folders into `w64devkit/x86_64-w64-mingw32`
* for example, OpenBLAS has `bin`, `include` and `lib` folders - unpack them into `w64devkit/x86_64-w64-mingw32`
4. Download and copy all prerequisite code: specific version of [imgui](https://github.com/ocornut/imgui/tree/f6836ff37fd361010829621f610837686aa00944) and [tinyfiledialogs](https://sourceforge.net/projects/tinyfiledialogs/)
5. Launch w64devkit.exe and navigate to the project folder, then build:
* `make chat` for CPU-only UI version
* `make chat_ob` for CPU with OPENBLAS64 UI version (helps with prompt processing)
* `make chat_cl` for Clblast UI version
* `make chat_vk` for Vulkan UI version
6. `make chatTest`, `make chatTest_cl` and `make chatTest_vk` for building the debugging program
* if you want to switch off OpenMP, add `OPENMP_OFF=1`, same for SGEMM -`SGEMM_OFF=1`
* if your GPU/iGPU don't support Vulkan, compile UI with SDL2=1 or using `_sdl` recipes (i.e. `chat_sdl`, `chat_cl_sdl`, etc.)
* if you need Windows console for debugging, compile with CONW=1
* see more in makefile

### Credits

* Icon is generated by me using Stable Diffusion (OptimizedSD through https://github.com/n00mkrad/text2image-gui 1.9.0)
* [llama.cpp](https://github.com/ggerganov/llama.cpp)
* [imgui](https://github.com/ocornut/imgui)
* Retro theme based on https://github.com/ocornut/imgui/issues/707#issuecomment-254610737
* [mistral-7b-instruct-v0.1](https://huggingface.co/TheBloke/Mistral-7B-Instruct-v0.1-GGUF) from config.json (q4_K_S version works faster)
* [Mistral-Nemo-Instruct-2407-GGUF](https://huggingface.co/ZeroWw/Mistral-Nemo-Instruct-2407-GGUF) from config.json (specific quants with output and embed tensors quantized to f16, q5 is the smallest)
* [redmond-puffin-13b (previously recommended)](https://huggingface.co/TheBloke/Redmond-Puffin-13B-GGUF) from config.json (q4_K_S version works faster)
* Exceptionally useful tool for visualizing sampling results and finding the pest combination: https://artefact2.github.io/llm-sampling/index.xhtml

### Additional notes

* Vulkan experimental build used [this PR](https://github.com/ggerganov/llama.cpp/pull/2059)
* ggmlv3 version is very old and almost deprecated for now, as almost no new models are using the old format
