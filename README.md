# Llama_chat
A chat UI for Llama.cpp. Uses ImgUI, tinyfiledialogs and json.hpp.

This project started from the main example of llama.cpp - the idea was to read parameters from .json files, and it stays the same here, only with a class, separate therads for running llama.cpp, structs for managing and settings - and only then wrapped into UI. A chatTest console app is also added as an additional example and for testing purposes, since certain infromation (like memory used by the model) is printed outside of the original example. 

## Features

* Two versions: gguf and ggmlv3
* Settings managements through a config.json file
* Global and per-model settings
* "Templates" (prompts) with a separate .json file to store, load and quickly use in chat ui.
* Assignable folders for models and prompt files
* Quick select of prompt files before loading a model
* Quick model select according to config.json

## Building

Tested on Windows only for now. AVX releases only due to old CPU, please compile if using a modern CPU.

### Requirements

* tinyfiledialogs https://sourceforge.net/projects/tinyfiledialogs/
* imgui https://github.com/ocornut/imgui
* OpenCL and CLBLAST if needed (see https://github.com/ggerganov/llama.cpp#clblast for installation guide)
* 

### Builning on Windows

* Use w64devkit
* Launch w64devkit.exe and navigate to the project folder
* `make all_cpu` to compile all cpu-only executables
* `make all` includes CLBLAST only for now (for CUBLAS see https://github.com/ggerganov/llama.cpp/issues/1470 )
