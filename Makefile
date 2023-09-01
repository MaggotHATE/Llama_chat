#
# Cross Platform Makefile
# Compatible with MSYS2/MINGW, Ubuntu 14.04.1 and Mac OS X
#
# You will need SDL2 (http://www.libsdl.org):
# Linux:
#   apt-get install libsdl2-dev
# Mac OS X:
#   brew install sdl2
# MSYS2:
#   pacman -S mingw-w64-i686-SDL2
#

#CXX = g++
#CXX = clang++

CCC = c2x
CCPP = c++20

EXE = Llama_Chat_gguf
EXE_CL = Llama_Chat_gguf_clblast
EXE_GGML = Llama_Chat_ggml
EXE_GGML_CL = Llama_Chat_ggml_clblast
IMGUI_DIR = imgui
SOURCES = main.cpp
SOURCES += $(IMGUI_DIR)/imgui.cpp $(IMGUI_DIR)/imgui_demo.cpp $(IMGUI_DIR)/imgui_draw.cpp $(IMGUI_DIR)/imgui_tables.cpp $(IMGUI_DIR)/imgui_widgets.cpp
SOURCES += $(IMGUI_DIR)/backends/imgui_impl_sdl2.cpp $(IMGUI_DIR)/backends/imgui_impl_opengl3.cpp
SOURCES += $(IMGUI_DIR)/misc/cpp/imgui_stdlib.cpp
OBJS = $(addprefix o/, $(addsuffix .o, $(basename $(notdir $(SOURCES)))))
OBJS_CL = $(subst main.o,main_cl.o,$(OBJS))
OBJS_GGML = $(subst main.o,main_ggml.o,$(OBJS))
#OBJS_GGML = $(subst o/main.o, ,$(OBJS))
UNAME_S := $(shell uname -s)
LINUX_GL_LIBS = -lGL


#for main ui
CXXFLAGS = -O3 -std=$(CCPP) -fPIC -DNDEBUG -march=native -mtune=native -DGGML_USE_K_QUANTS -DLOG_DISABLE_LOGS -w
CXXFLAGS2 = -O3 -std=$(CCPP) -fPIC -DNDEBUG -march=native -mtune=native -DGGML_USE_K_QUANTS -DGGML_USE_CLBLAST -DLOG_DISABLE_LOGS -w
CXXFLAGS_GGML = -O3 -std=$(CCPP) -fPIC -DNDEBUG -march=native -mtune=native -DGGML_USE_K_QUANTS -DGGML_OLD_FORMAT -DLOG_DISABLE_LOGS -w
CXXFLAGS += -I$(IMGUI_DIR) -I$(IMGUI_DIR)/backends
CXXFLAGS2 += -I$(IMGUI_DIR) -I$(IMGUI_DIR)/backends
CXXFLAGS_GGML += -I$(IMGUI_DIR) -I$(IMGUI_DIR)/backends
CXXFLAGS += -g -Wall -Wformat -pipe
CXXFLAGS2 += -g -Wall -Wformat -pipe
CXXFLAGS_GGML += -g -Wall -Wformat -pipe



#for general ggml-gguf
CFLAGS = -O3 -std=$(CCC) -fPIC -DNDEBUG -march=native -mtune=native -DGGML_USE_K_QUANTS -DLOG_DISABLE_LOGS -w -pipe
CFLAGS2 = -O3 -std=$(CCC) -fPIC -DNDEBUG -march=native -mtune=native -DGGML_USE_K_QUANTS -DGGML_USE_CLBLAST -DLOG_DISABLE_LOGS -w -pipe
CFLAGS_GGML = -O3 -std=$(CCC) -fPIC -DNDEBUG -march=native -mtune=native -DGGML_USE_K_QUANTS -DGGML_OLD_FORMAT -DLOG_DISABLE_LOGS -w -pipe


#for all chatTest
CXXFLAGS1 = -O3 -std=$(CCPP) -fPIC -DNDEBUG -march=native -mtune=native -DGGML_USE_K_QUANTS -DLOG_DISABLE_LOGS -w
CXXFLAGS3 = -O3 -std=$(CCPP) -fPIC -DNDEBUG -march=native -mtune=native -DGGML_USE_K_QUANTS -DGGML_USE_CLBLAST -DLOG_DISABLE_LOGS -w
CXXFLAGS_GGML1 = -O3 -std=$(CCPP) -fPIC -DNDEBUG -march=native -mtune=native -DGGML_USE_K_QUANTS -DGGML_OLD_FORMAT -DLOG_DISABLE_LOGS -w

# The stack is only 16-byte aligned on Windows, so don't let gcc emit aligned moves.
# https://gcc.gnu.org/bugzilla/show_bug.cgi?id=54412
# https://github.com/ggerganov/llama.cpp/issues/2922
ifneq '' '$(findstring mingw,$(shell $(CC) -dumpmachine))'
	CFLAGS   += -Xassembler -muse-unaligned-vector-move
	CFLAGS2   += -Xassembler -muse-unaligned-vector-move
	CFLAGS_GGML   += -Xassembler -muse-unaligned-vector-move
	CXXFLAGS1 += -Xassembler -muse-unaligned-vector-move
	CXXFLAGS2 += -Xassembler -muse-unaligned-vector-move
	CXXFLAGS3 += -Xassembler -muse-unaligned-vector-move
	CXXFLAGS_GGML1 += -Xassembler -muse-unaligned-vector-move
endif

LIBS =
LDFLAGS  =

##---------------------------------------------------------------------
## OPENGL ES
##---------------------------------------------------------------------

## This assumes a GL ES library available in the system, e.g. libGLESv2.so
# CXXFLAGS += -DIMGUI_IMPL_OPENGL_ES2
# LINUX_GL_LIBS = -lGLESv2
## If you're on a Raspberry Pi and want to use the legacy drivers,
## use the following instead:
# LINUX_GL_LIBS = -L/opt/vc/lib -lbrcmGLESv2

##---------------------------------------------------------------------
## BUILD FLAGS PER PLATFORM
##---------------------------------------------------------------------

ifeq ($(UNAME_S), Linux) #LINUX
	ECHO_MESSAGE = "Linux"
	LIBS += $(LINUX_GL_LIBS) -ldl `sdl2-config --libs`

	CXXFLAGS += `sdl2-config --cflags`
	CXXFLAGS2 += `sdl2-config --cflags`
	CXXFLAGS_GGML += `sdl2-config --cflags`
	#CFLAGS = $(CXXFLAGS)
	#CFLAGS2 = $(CXXFLAGS2)
endif

ifeq ($(UNAME_S), Darwin) #APPLE
	ECHO_MESSAGE = "Mac OS X"
	LIBS += -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo `sdl2-config --libs`
	LIBS += -L/usr/local/lib -L/opt/local/lib

	CXXFLAGS += `sdl2-config --cflags`
	CXXFLAGS += -I/usr/local/include -I/opt/local/include	
    CXXFLAGS2 += `sdl2-config --cflags`
	CXXFLAGS2 += -I/usr/local/include -I/opt/local/include
    CXXFLAGS_GGML += `sdl2-config --cflags`
	CXXFLAGS_GGML += -I/usr/local/include -I/opt/local/include
	#CFLAGS = $(CXXFLAGS)
	#CFLAGS2 = $(CXXFLAGS2)
endif

ifeq ($(OS), Windows_NT)
    ECHO_MESSAGE = "MinGW"
    LIBS += -static -lgdi32 -lopengl32 -limm32 `pkg-config --static --libs sdl2`

    CXXFLAGS += `pkg-config --cflags sdl2`
    CXXFLAGS2 += `pkg-config --cflags sdl2`
    CXXFLAGS_GGML += `pkg-config --cflags sdl2`
    #CFLAGS = $(CXXFLAGS)
    #CFLAGS2 = $(CXXFLAGS2)
endif

##---------------------------------------------------------------------
## BUILD RULES
##---------------------------------------------------------------------
  
#GGML

OBJS_GGML1 = o/ggml_old.o o/ggml-alloc_old.o o/llama_old.o o/common_old.o o/k_quants_old.o o/grammar-parser_old.o

o/ggml_old.o: GGML/ggml.c GGML/ggml.h
	$(CC)  $(CFLAGS_GGML)   -c $< -o $@
    
o/ggml-alloc_old.o: GGML/ggml-alloc.c GGML/ggml.h GGML/ggml-alloc.h
	$(CC)  $(CFLAGS_GGML)   -c $< -o $@

o/llama_old.o: GGML/llama.cpp GGML/ggml.h GGML/ggml-alloc.h GGML/llama.h  GGML/llama-util.h
	$(CXX) $(CXXFLAGS_GGML1) -c $< -o $@

o/common_old.o: GGML/common.cpp GGML/common.h
	$(CXX) $(CXXFLAGS_GGML1) -c $< -o $@

o/libllama_old.so: o/llama.o o/ggml.o $(OBJS)
	$(CXX) $(CXXFLAGS_GGML1) -shared -fPIC -o $@ $^ $(LDFLAGS)
    
o/k_quants_old.o: GGML/k_quants.c GGML/k_quants.h
	$(CC) $(CFLAGS_GGML) -c $< -o $@
    
o/grammar-parser_old.o: GGML/grammar-parser.cpp GGML/grammar-parser.h
	$(CXX) $(CXXFLAGS_GGML1) -c $< -o $@

  
# GGUF

OBJS1 = o/ggml.o o/ggml-alloc.o o/llama.o o/common.o o/k_quants.o o/grammar-parser.o
  
o/ggml.o: base/ggml.c base/ggml.h
	$(CC)  $(CFLAGS)   -c $< -o $@
    
o/ggml-alloc.o: base/ggml-alloc.c base/ggml.h base/ggml-alloc.h
	$(CC)  $(CFLAGS)   -c $< -o $@

o/llama.o: base/llama.cpp base/ggml.h base/ggml-alloc.h base/llama.h
	$(CXX) $(CXXFLAGS1) -c $< -o $@

o/common.o: base/common.cpp base/common.h
	$(CXX) $(CXXFLAGS1) -c $< -o $@

o/libllama.so: o/llama.o o/ggml.o $(OBJS)
	$(CXX) $(CXXFLAGS1) -shared -fPIC -o $@ $^ $(LDFLAGS)
    
o/k_quants.o: base/k_quants.c base/k_quants.h
	$(CC) $(CFLAGS) -c $< -o $@
    
o/grammar-parser.o: base/grammar-parser.cpp base/grammar-parser.h
	$(CXX) $(CXXFLAGS1) -c $< -o $@
    
    
    
# Separate CLBLAST

# Mac provides OpenCL as a framework
ifeq ($(UNAME_S),Darwin)
    LDFLAGS2 += -lclblast -framework OpenCL
else
    #LDFLAGS2 += -Llib/OpenCL.lib -Llib/clblast.lib  -lclblast -lOpenCL
    LDFLAGS2 += -lclblast -lOpenCL
endif

OBJS3    = o/k_quants_cl.o o/ggml-opencl_cl.o o/ggml_cl.o o/ggml-alloc_cl.o o/llama_cl.o o/common_cl.o o/grammar-parser_cl.o

o/ggml-opencl_cl.o: base/ggml-opencl.cpp base/ggml-opencl.h
	$(CXX) $(CXXFLAGS3) -c $< -o $@
    
o/ggml_cl.o: base/ggml.c base/ggml.h
	$(CC)  $(CFLAGS2)   -c $< -o $@
    
o/ggml-alloc_cl.o: base/ggml-alloc.c base/ggml.h base/ggml-alloc.h
	$(CC)  $(CFLAGS2)   -c $< -o $@

o/llama_cl.o: base/llama.cpp base/ggml.h base/ggml-alloc.h base/llama.h
	$(CXX) $(CXXFLAGS3) -c $< -o $@

o/common_cl.o: base/common.cpp base/common.h
	$(CXX) $(CXXFLAGS3) -c $< -o $@

o/libllama_cl.so: o/llama_cl.o o/ggml_cl.o $(OBJS2)
	$(CXX) $(CXXFLAGS3) -shared -fPIC -o $@ $^ $(LDFLAGS2)
    
o/k_quants_cl.o: base/k_quants.c base/k_quants.h
	$(CC) $(CFLAGS2) -c $< -o $@
    
o/grammar-parser_cl.o: base/grammar-parser.cpp base/grammar-parser.h
	$(CXX) $(CXXFLAGS3) -c $< -o $@
    

# general    
    
o/%.o:%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<
    
o/main_cl.o:main.cpp
	$(CXX) $(CXXFLAGS2) -c -o $@ $<
    
o/main_ggml.o:main.cpp
	$(CXX) $(CXXFLAGS_GGML) -c -o $@ $<

o/%.o:$(IMGUI_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

o/%.o:$(IMGUI_DIR)/backends/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<
    
o/%.o:$(IMGUI_DIR)/misc/cpp/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

#BULDS    
    
demo: $(EXE)
	@echo Build complete for $(ECHO_MESSAGE) 
    
demo_cl: $(EXE_CL)
	@echo Build complete for $(ECHO_MESSAGE)
    
demo_ggml: $(EXE_GGML)
	@echo Build complete for $(ECHO_MESSAGE)
    
demos_gguf: $(EXE) $(EXE_CL)
	@echo Build complete for $(ECHO_MESSAGE)
    
demos: $(EXE) $(EXE_CL) $(EXE_GGML)
	@echo Build complete for $(ECHO_MESSAGE)
    
tests: chatTest chatTest_cl chatTest_ggml
	@echo Build complete for $(ECHO_MESSAGE)
    
all: $(EXE) $(EXE_CL) chatTest chatTest_cl $(EXE_GGML) chatTest_ggml
	@echo Build complete for $(ECHO_MESSAGE)
    
all_cpu: $(EXE) chatTest $(EXE_GGML) chatTest_ggml
	@echo Build complete for $(ECHO_MESSAGE)
    
all_gguf: $(EXE) $(EXE_CL) chatTest chatTest_cl
	@echo Build complete for $(ECHO_MESSAGE)
    
gguf_cpu: $(EXE) chatTest
	@echo Build complete for $(ECHO_MESSAGE)
    
ggml: $(EXE_GGML) chatTest_ggml
	@echo Build complete for $(ECHO_MESSAGE)

$(EXE): $(OBJS) $(OBJS1) include/json.hpp tinyfiledialogs/tinyfiledialogs.c chat_plain.h thread_chat.h llama_chat1.res
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LDFLAGS) $(LIBS)
    
$(EXE_CL): $(OBJS_CL) $(OBJS3) include/json.hpp tinyfiledialogs/tinyfiledialogs.c chat_plain.h thread_chat.h llama_chat1.res
	$(CXX) -o $@ $^ $(CXXFLAGS2) $(LDFLAGS2) $(LIBS)
    
chatTest:class_chat.cpp $(OBJS1) include/json.hpp chat_plain.h thread_chat.h
	$(CXX) $(CXXFLAGS1) $(filter-out %.h,$^) $(LDFLAGS) -o $@
    
chatTest_cl:class_chat.cpp                                  include/json.hpp chat_plain.h thread_chat.h $(OBJS3)
	$(CXX) $(CXXFLAGS3) $(filter-out %.h,$^) $(LDFLAGS2) -o $@
    
#GGML format

$(EXE_GGML):$(OBJS_GGML) $(OBJS_GGML1) include/json.hpp tinyfiledialogs/tinyfiledialogs.c GGML/chat_plain.h thread_chat.h llama_chat1.res
	$(CXX) -o $@ $(filter-out %.h,$^) $(CXXFLAGS_GGML) $(LDFLAGS) $(LIBS)
    
chatTest_ggml:class_chat.cpp $(OBJS_GGML1) include/json.hpp GGML/chat_plain.h thread_chat.h 
	$(CXX) $(CXXFLAGS_GGML1) $(filter-out %.h,$^) $(LDFLAGS) -o $@

clean:
	rm -f $(EXE) $(OBJS)
