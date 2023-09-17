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

# detect Windows
ifneq ($(findstring _NT,$(UNAME_S)),)
	_WIN32 := 1
endif

# library name prefix
ifneq ($(_WIN32),1)
	LIB_PRE := lib
endif

# Dynamic Shared Object extension
ifneq ($(_WIN32),1)
	DSO_EXT := .so
else
	DSO_EXT := .dll
endif

CCC = c2x
CCPP = c++20

# -Ofast tends to produce faster code, but may not be available for some compilers.
ifdef LLAMA_FAST
OPT = -Ofast
else
OPT = -O3
endif

EXE = Llama_Chat_gguf
EXE_OB = Llama_Chat_gguf_openblas
EXE_CL = Llama_Chat_gguf_clblast
EXE_GGML = Llama_Chat_ggml
EXE_CL_GGML = Llama_Chat_ggml_clblast
IMGUI_DIR = imgui
SOURCES = main.cpp
SOURCES += $(IMGUI_DIR)/imgui.cpp $(IMGUI_DIR)/imgui_demo.cpp $(IMGUI_DIR)/imgui_draw.cpp $(IMGUI_DIR)/imgui_tables.cpp $(IMGUI_DIR)/imgui_widgets.cpp
SOURCES += $(IMGUI_DIR)/backends/imgui_impl_sdl2.cpp $(IMGUI_DIR)/backends/imgui_impl_opengl3.cpp
SOURCES += $(IMGUI_DIR)/misc/cpp/imgui_stdlib.cpp
OBJS0 = $(addprefix o/imgui/, $(addsuffix .o, $(basename $(notdir $(SOURCES)))))
OBJS = $(subst o/imgui/main.o,main.cpp,$(OBJS0))

#OBJS_OB = $(subst main.o,main_ob.o,$(OBJS))
#OBJS_CL = $(subst main.o,cl_main.o,$(OBJS))
#OBJS_GGML = $(subst main.o,old_main.o,$(OBJS))
#OBJS_CL_GGML = $(subst main.o,old_cl_main.o,$(OBJS))
#OBJS_GGML = $(subst o/main.o, ,$(OBJS))
UNAME_S := $(shell uname -s)
LINUX_GL_LIBS = -lGL

# clock_gettime came in POSIX.1b (1993)
# CLOCK_MONOTONIC came in POSIX.1-2001 / SUSv3 as optional
# posix_memalign came in POSIX.1-2001 / SUSv3
# M_PI is an XSI extension since POSIX.1-2001 / SUSv3, came in XPG1 (1985)
GNUPDATEC = -D_XOPEN_SOURCE=600
GNUPDATECXX = -D_XOPEN_SOURCE=600

# Somehow in OpenBSD whenever POSIX conformance is specified
# some string functions rely on locale_t availability,
# which was introduced in POSIX.1-2008, forcing us to go higher
ifeq ($(UNAME_S),OpenBSD)
	GNUPDATEC   += -U_XOPEN_SOURCE -D_XOPEN_SOURCE=700
	GNUPDATECXX += -U_XOPEN_SOURCE -D_XOPEN_SOURCE=700
endif

# Data types, macros and functions related to controlling CPU affinity and
# some memory allocation are available on Linux through GNU extensions in libc
ifeq ($(UNAME_S),Linux)
	GNUPDATEC   += -D_GNU_SOURCE
	GNUPDATECXX += -D_GNU_SOURCE
endif

# RLIMIT_MEMLOCK came in BSD, is not specified in POSIX.1,
# and on macOS its availability depends on enabling Darwin extensions
# similarly on DragonFly, enabling BSD extensions is necessary
ifeq ($(UNAME_S),Darwin)
	GNUPDATEC   += -D_DARWIN_C_SOURCE
	GNUPDATECXX += -D_DARWIN_C_SOURCE
endif
ifeq ($(UNAME_S),DragonFly)
	GNUPDATEC   += -D__BSD_VISIBLE
	GNUPDATECXX += -D__BSD_VISIBLE
endif

# alloca is a non-standard interface that is not visible on BSDs when
# POSIX conformance is specified, but not all of them provide a clean way
# to enable it in such cases
ifeq ($(UNAME_S),FreeBSD)
	GNUPDATEC   += -D__BSD_VISIBLE
	GNUPDATECXX += -D__BSD_VISIBLE
endif
ifeq ($(UNAME_S),NetBSD)
	GNUPDATEC   += -D_NETBSD_SOURCE
	GNUPDATECXX += -D_NETBSD_SOURCE
endif
ifeq ($(UNAME_S),OpenBSD)
	GNUPDATEC   += -D_BSD_SOURCE
	GNUPDATECXX += -D_BSD_SOURCE
endif

#for main ui
CXXFLAGS_UI = -O3 -std=$(CCPP) -fPIC -DNDEBUG -march=native -mtune=native -DGGML_USE_K_QUANTS -DLOG_DISABLE_LOGS -w
CXXFLAGS_UI_CL = -O3 -std=$(CCPP) -fPIC -DNDEBUG -march=native -mtune=native -DGGML_USE_K_QUANTS -DGGML_USE_CLBLAST -DLOG_DISABLE_LOGS -w
CXXFLAGS_UI_GGML = -O3 -std=$(CCPP) -fPIC -DNDEBUG -march=native -mtune=native -DGGML_USE_K_QUANTS -DGGML_OLD_FORMAT -DLOG_DISABLE_LOGS -w
CXXFLAGS_UI_CL_GGML = -O3 -std=$(CCPP) -fPIC -DNDEBUG -march=native -mtune=native -DGGML_USE_K_QUANTS -DGGML_OLD_FORMAT -DGGML_USE_CLBLAST -DLOG_DISABLE_LOGS -w
CXXFLAGS_UI += -I$(IMGUI_DIR) -I$(IMGUI_DIR)/backends
CXXFLAGS_UI_CL += -I$(IMGUI_DIR) -I$(IMGUI_DIR)/backends
CXXFLAGS_UI_GGML += -I$(IMGUI_DIR) -I$(IMGUI_DIR)/backends
CXXFLAGS_UI_CL_GGML += -I$(IMGUI_DIR) -I$(IMGUI_DIR)/backends
CXXFLAGS_UI += -g -Wall -Wformat -pipe
CXXFLAGS_UI_CL += -g -Wall -Wformat -pipe
CXXFLAGS_UI_GGML += -g -Wall -Wformat -pipe
CXXFLAGS_UI_CL_GGML += -g -Wall -Wformat -pipe



#for general ggml-gguf
CFLAGS = $(OPT) -std=$(CCC) -fPIC $(GNUPDATEC) -DNDEBUG -march=native -mtune=native -DGGML_USE_K_QUANTS -DLOG_DISABLE_LOGS -w -pipe
CFLAGS_CL = $(OPT) -std=$(CCC) -fPIC $(GNUPDATEC) -DNDEBUG -march=native -mtune=native -DGGML_USE_K_QUANTS -DGGML_USE_CLBLAST -DLOG_DISABLE_LOGS -w -pipe
CFLAGS_GGML = $(OPT) -std=$(CCC) -fPIC $(GNUPDATEC) -DNDEBUG -march=native -mtune=native -DGGML_USE_K_QUANTS -DGGML_OLD_FORMAT -DLOG_DISABLE_LOGS -w -pipe
CFLAGS_CL_GGML = $(OPT) -std=$(CCC) -fPIC $(GNUPDATEC) -DNDEBUG -march=native -mtune=native -DGGML_USE_K_QUANTS -DGGML_USE_CLBLAST -DGGML_OLD_FORMAT -DLOG_DISABLE_LOGS -w -pipe
CFLAGS_VK = $(OPT) -std=$(CCC) -fPIC $(GNUPDATEC) -DNDEBUG -march=native -mtune=native -DGGML_USE_K_QUANTS -DGGML_USE_VULKAN -DLOG_DISABLE_LOGS -w -pipe


#for all chatTest
CXXFLAGS = $(OPT) -std=$(CCPP) $(GNUPDATECXX) -fPIC -DNDEBUG -march=native -mtune=native -DGGML_USE_K_QUANTS -DLOG_DISABLE_LOGS -w -pipe
CXXFLAGS_CL = $(OPT) -std=$(CCPP) $(GNUPDATECXX) -fPIC -DNDEBUG -march=native -mtune=native -DGGML_USE_K_QUANTS -DGGML_USE_CLBLAST -DLOG_DISABLE_LOGS -w -pipe
CXXFLAGS_GGML = $(OPT) -std=$(CCPP) $(GNUPDATECXX) -fPIC -DNDEBUG -march=native -mtune=native -DGGML_USE_K_QUANTS -DGGML_OLD_FORMAT -DLOG_DISABLE_LOGS -w -pipe
CXXFLAGS_CL_GGML = $(OPT) -std=$(CCPP) $(GNUPDATECXX) -fPIC -DNDEBUG -march=native -mtune=native -DGGML_USE_K_QUANTS -DGGML_USE_CLBLAST -DGGML_OLD_FORMAT -DLOG_DISABLE_LOGS -w -pipe
CXXFLAGS_VK = $(OPT) -std=$(CCPP) $(GNUPDATECXX) -fPIC -DNDEBUG -march=native -mtune=native -DGGML_USE_K_QUANTS -DGGML_USE_VULKAN -DLOG_DISABLE_LOGS -w -pipe


# The stack is only 16-byte aligned on Windows, so don't let gcc emit aligned moves.
# https://gcc.gnu.org/bugzilla/show_bug.cgi?id=54412
# https://github.com/ggerganov/llama.cpp/issues/2922
ifneq '' '$(findstring mingw,$(shell $(CC) -dumpmachine))'
    CFLAGS   += -Xassembler -muse-unaligned-vector-move
    CFLAGS_CL   += -Xassembler -muse-unaligned-vector-move
    CFLAGS_GGML   += -Xassembler -muse-unaligned-vector-move
    CFLAGS_CL_GGML   += -Xassembler -muse-unaligned-vector-move
    CXXFLAGS += -Xassembler -muse-unaligned-vector-move
    CXXFLAGS_UI   += -Xassembler -muse-unaligned-vector-move
    CXXFLAGS_UI_CL += -Xassembler -muse-unaligned-vector-move
    CXXFLAGS_UI_GGML   += -Xassembler -muse-unaligned-vector-move
    CXXFLAGS_UI_CL_GGML += -Xassembler -muse-unaligned-vector-move
    CXXFLAGS_CL += -Xassembler -muse-unaligned-vector-move
    CXXFLAGS_GGML += -Xassembler -muse-unaligned-vector-move
    CXXFLAGS_CL_GGML += -Xassembler -muse-unaligned-vector-move
endif

LIBS =
LDFLAGS  =

##---------------------------------------------------------------------
## OPENGL ES
##---------------------------------------------------------------------

## This assumes a GL ES library available in the system, e.g. libGLESv2.so
# CXXFLAGS_UI += -DIMGUI_IMPL_OPENGL_ES2
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

	CXXFLAGS_UI += `sdl2-config --cflags`
	CXXFLAGS_UI_CL += `sdl2-config --cflags`
	CXXFLAGS_UI_GGML += `sdl2-config --cflags`
	CXXFLAGS_UI_CL_GGML += `sdl2-config --cflags`
	#CFLAGS = $(CXXFLAGS)
	#CFLAGS_CL = $(CXXFLAGS_UI_CL)
endif

ifeq ($(UNAME_S), Darwin) #APPLE
    ECHO_MESSAGE = "Mac OS X"
    LIBS += -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo `sdl2-config --libs`
    LIBS += -L/usr/local/lib -L/opt/local/lib

    CXXFLAGS_UI += `sdl2-config --cflags`
    CXXFLAGS_UI += -I/usr/local/include -I/opt/local/include	
    CXXFLAGS_UI_CL += `sdl2-config --cflags`
    CXXFLAGS_UI_CL += -I/usr/local/include -I/opt/local/include
    CXXFLAGS_UI_GGML += `sdl2-config --cflags`
    CXXFLAGS_UI_GGML += -I/usr/local/include -I/opt/local/include
    CXXFLAGS_UI_CL_GGML += `sdl2-config --cflags`
    CXXFLAGS_UI_CL_GGML += -I/usr/local/include -I/opt/local/include
	#CFLAGS = $(CXXFLAGS)
	#CFLAGS_CL = $(CXXFLAGS_UI_CL)
endif

ifeq ($(OS), Windows_NT)
    ECHO_MESSAGE = "MinGW"
    LIBS += -static -lgdi32 -lopengl32 -limm32 `pkg-config --static --libs sdl2`

    CXXFLAGS_UI += `pkg-config --cflags sdl2`
    CXXFLAGS_UI_CL += `pkg-config --cflags sdl2`
    CXXFLAGS_UI_GGML += `pkg-config --cflags sdl2`
    CXXFLAGS_UI_CL_GGML += `pkg-config --cflags sdl2`
    #CFLAGS = $(CXXFLAGS)
    #CFLAGS_CL = $(CXXFLAGS_UI_CL)
endif

# For emojis

CXXFLAGS_UI += -DIMGUI_USE_WCHAR32
CXXFLAGS_UI_CL += -DIMGUI_USE_WCHAR32
CXXFLAGS_UI_GGML += -DIMGUI_USE_WCHAR32
CXXFLAGS_UI_CL_GGML += -DIMGUI_USE_WCHAR32

##---------------------------------------------------------------------
## BUILD RULES
##---------------------------------------------------------------------
  
#GGML

OBJS_GGML1 = o/old_ggml.o o/old_ggml-alloc.o o/old_llama.o o/old_common.o o/old_k_quants.o o/old_grammar-parser.o

o/old_ggml.o: GGML/ggml.c GGML/ggml.h
	$(CC)  $(CFLAGS_GGML)   -c $< -o $@
    
o/old_ggml-alloc.o: GGML/ggml-alloc.c GGML/ggml.h GGML/ggml-alloc.h
	$(CC)  $(CFLAGS_GGML)   -c $< -o $@

o/old_llama.o: GGML/llama.cpp GGML/ggml.h GGML/ggml-alloc.h GGML/llama.h  GGML/llama-util.h
	$(CXX) $(CXXFLAGS_GGML) -c $< -o $@

o/old_common.o: GGML/common.cpp GGML/common.h
	$(CXX) $(CXXFLAGS_GGML) -c $< -o $@

o/old_libllama$(DSO_EXT): o/old_llama.o o/old_ggml.o
	$(CXX) $(CXXFLAGS_GGML) -shared -fPIC -o $@ $^ $(LDFLAGS)
    
o/old_k_quants.o: GGML/k_quants.c GGML/k_quants.h
	$(CC) $(CFLAGS_GGML) -c $< -o $@
    
o/old_grammar-parser.o: GGML/grammar-parser.cpp GGML/grammar-parser.h
	$(CXX) $(CXXFLAGS_GGML) -c $< -o $@

# Separate GGML CLBLAST

# Mac provides OpenCL as a framework
ifeq ($(UNAME_S),Darwin)
    LDFLAGS_CL_GGML += -lclblast -framework OpenCL
else
    #LDFLAGS2 += -Llib/OpenCL.lib -Llib/clblast.lib  -lclblast -lOpenCL
    LDFLAGS_CL_GGML += -Llib/OpenCL.lib -Llib/clblast.lib
endif
    #LDFLAGS_CL += -Llib/OpenCL.lib -Llib/clblast.lib -lclblast -lOpenCL
CXXFLAGS_CL_GGML += -lclblast -lOpenCL
CXXFLAGS_UI_CL_GGML += -lclblast -lOpenCL


OBJS_CL_GGML1    = o/old_cl_k_quants.o o/old_cl_ggml-opencl.o o/old_cl_ggml.o o/old_cl_ggml-alloc.o o/old_cl_llama.o o/old_cl_common.o o/old_cl_grammar-parser.o

o/old_cl_ggml-opencl.o: GGML/ggml-opencl.cpp GGML/ggml-opencl.h
	$(CXX) $(CXXFLAGS_CL_GGML) -c $< -o $@
    
o/old_cl_ggml.o: GGML/ggml.c GGML/ggml.h
	$(CC)  $(CFLAGS_CL_GGML)   -c $< -o $@
    
o/old_cl_ggml-alloc.o: GGML/ggml-alloc.c GGML/ggml.h GGML/ggml-alloc.h
	$(CC)  $(CFLAGS_CL_GGML)   -c $< -o $@

o/old_cl_llama.o: GGML/llama.cpp base/ggml.h base/ggml-alloc.h GGML/llama.h
	$(CXX) $(CXXFLAGS_CL_GGML) -c $< -o $@

o/old_cl_common.o: GGML/common.cpp GGML/common.h
	$(CXX) $(CXXFLAGS_CL_GGML) -c $< -o $@

o/old_cl_libllama$(DSO_EXT): o/old_cl_llama.o o/old_cl_ggml.o
	$(CXX) $(CXXFLAGS_CL_GGML) -shared -fPIC -o $@ $^ $(LDFLAGS_CL_GGML)
    
o/old_cl_k_quants.o: GGML/k_quants.c GGML/k_quants.h
	$(CC) $(CFLAGS_CL_GGML) -c $< -o $@
    
o/old_cl_grammar-parser.o: GGML/grammar-parser.cpp GGML/grammar-parser.h
	$(CXX) $(CXXFLAGS_CL_GGML) -c $< -o $@
  
#VULKAN

LDFLAGS_VK = -lvulkan -lopenblas -lglslang -lSPIRV -lSPIRV-Tools-opt -lSPIRV-Tools -lshaderc_combined



OBJS_VK = o/ggml_vk.o o/ggml-alloc_vk.o o/llama_vk.o o/common_vk.o o/k_quants_vk.o o/grammar-parser_vk.o o/ggml-vulkan.o

o/ggml-vulkan.o: VULKAN/ggml-vulkan.cpp VULKAN/ggml-vulkan.h
	$(CXX) $(CXXFLAGS_VK) -c $< -o $@

o/ggml_vk.o: VULKAN/ggml.c GGML/ggml.h
	$(CC)  $(CFLAGS_VK)   -c $< -o $@
    
o/ggml-alloc_vk.o: VULKAN/ggml-alloc.c VULKAN/ggml.h VULKAN/ggml-alloc.h
	$(CC)  $(CFLAGS_VK)   -c $< -o $@

o/llama_vk.o: VULKAN/llama.cpp VULKAN/ggml.h VULKAN/ggml-alloc.h VULKAN/llama.h  VULKAN/llama-util.h
	$(CXX) $(CXXFLAGS_VK) -c $< -o $@

o/common_vk.o: VULKAN/common.cpp VULKAN/common.h
	$(CXX) $(CXXFLAGS_VK) -c $< -o $@
    
o/k_quants_vk.o: VULKAN/k_quants.c VULKAN/k_quants.h
	$(CC) $(CFLAGS_VK) -c $< -o $@
    
o/grammar-parser_vk.o: VULKAN/grammar-parser.cpp VULKAN/grammar-parser.h
	$(CXX) $(CXXFLAGS_VK) -c $< -o $@
  
  
# GGUF

OBJS_GGUF = o/ggml.o o/ggml-alloc.o o/llama.o o/common.o o/k_quants.o o/grammar-parser.o
OBJS_GGUF_STATIC = base/llama.cpp base/common.cpp base/grammar-parser.cpp base/ggml.c base/ggml-alloc.c base/k_quants.c base/ggml.h base/ggml-alloc.h base/llama.h base/common.h base/k_quants.h base/grammar-parser.h
  
o/ggml.o: base/ggml.c base/ggml.h
	$(CC)  $(CFLAGS)   -c $< -o $@
    
o/ggml-alloc.o: base/ggml-alloc.c base/ggml.h base/ggml-alloc.h
	$(CC)  $(CFLAGS)   -c $< -o $@

#base/threadpool.h
o/llama.o: base/llama.cpp base/ggml.h base/ggml-alloc.h base/llama.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

o/common.o: base/common.cpp base/common.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

o/libllama$(DSO_EXT): $(OBJS_GGUF)
	$(CXX) $(CXXFLAGS) -shared -fPIC -o $@ $^ $(LDFLAGS)
    
o/libllama_static$(DSO_EXT): $(OBJS_GGUF)
	$(CXX) $(CXXFLAGS) -static -fPIC -o $@ $^ $(LDFLAGS)
    
o/k_quants.o: base/k_quants.c base/k_quants.h
	$(CC) $(CFLAGS) -c $< -o $@
    
o/grammar-parser.o: base/grammar-parser.cpp base/grammar-parser.h
	$(CXX) $(CXXFLAGS) -c $< -o $@
   
    
# Separate CLBLAST

# Mac provides OpenCL as a framework
ifeq ($(UNAME_S),Darwin)
    LDFLAGS_CL += -lclblast -framework OpenCL
else
    #LDFLAGS2 += -Llib/OpenCL.lib -Llib/clblast.lib  -lclblast -lOpenCL
    LDFLAGS_CL += -Llib/OpenCL.lib -Llib/clblast.lib
    #LDFLAGS_CL += -Llib/OpenCL.lib -Llib/clblast.lib -lclblast -lOpenCL
endif

CXXFLAGS_CL += -lclblast -lOpenCL
CXXFLAGS_UI_CL += -lclblast -lOpenCL


OBJS_GGUF_CL    = o/cl_k_quants.o o/cl_ggml-opencl-gguf.o o/cl_ggml.o o/cl_ggml-alloc.o o/cl_llama.o o/cl_common.o o/cl_grammar-parser.o

o/cl_ggml-opencl-gguf.o: base/ggml-opencl.cpp base/ggml-opencl.h
	$(CXX) $(CXXFLAGS_CL) -c $< -o $@
    
o/cl_ggml.o: base/ggml.c base/ggml.h
	$(CC)  $(CFLAGS_CL)   -c $< -o $@
    
o/cl_ggml-alloc.o: base/ggml-alloc.c base/ggml.h base/ggml-alloc.h
	$(CC)  $(CFLAGS_CL)   -c $< -o $@

#base/threadpool.h
o/cl_llama.o: base/llama.cpp base/ggml.h base/ggml-alloc.h base/llama.h
	$(CXX) $(CXXFLAGS_CL) -c $< -o $@

o/cl_common.o: base/common.cpp base/common.h
	$(CXX) $(CXXFLAGS_CL) -c $< -o $@

o/cl_libllama$(DSO_EXT): $(OBJS_GGUF_CL)
	$(CXX) $(CXXFLAGS_CL) -shared -fPIC -o $@ $^ $(LDFLAGS_CL)
    
o/cl_k_quants.o: base/k_quants.c base/k_quants.h
	$(CC) $(CFLAGS_CL) -c $< -o $@
    
o/cl_grammar-parser.o: base/grammar-parser.cpp base/grammar-parser.h
	$(CXX) $(CXXFLAGS_CL) -c $< -o $@
    
    
# separate OpenBlas


ifneq ($(shell grep -e "Arch Linux" -e "ID_LIKE=arch" /etc/os-release 2>/dev/null),)
    CFLAGS_OB  += -DGGML_USE_OPENBLAS -I/usr/local/include/openblas -I/usr/include/openblas
    LDFLAGS_OB += -lopenblas -lcblas
else
    CFLAGS_OB  += -DGGML_USE_OPENBLAS -I/include/openblas -I/include/openblas
    LDFLAGS_OB += -lopenblas
endif

OBJS_GGUF_OB = o/ggml_ob.o o/ggml-alloc_ob.o o/llama_ob.o o/common_ob.o o/k_quants_ob.o o/grammar-parser_ob.o
  
o/ggml_ob.o: base/ggml.c base/ggml.h
	$(CC)  $(CFLAGS) $(CFLAGS_OB)   -c $< -o $@
    
o/ggml-alloc_ob.o: base/ggml-alloc.c base/ggml.h base/ggml-alloc.h
	$(CC)  $(CFLAGS) $(CFLAGS_OB)   -c $< -o $@

#base/threadpool.h
o/llama_ob.o: base/llama.cpp base/ggml.h base/ggml-alloc.h base/llama.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

o/common_ob.o: base/common.cpp base/common.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

o/libllama_ob$(DSO_EXT): o/llama.o o/ggml.o $(OBJS_GGUF_OB)
	$(CXX) $(CXXFLAGS) -shared -fPIC -o $@ $^ $(LDFLAGS)
    
o/k_quants_ob.o: base/k_quants.c base/k_quants.h
	$(CC) $(CFLAGS) $(CFLAGS_OB) -c $< -o $@
    
o/grammar-parser_ob.o: base/grammar-parser.cpp base/grammar-parser.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

# general    
    
o/imgui/%.o:$(IMGUI_DIR)/%.cpp
	$(CXX) $(CXXFLAGS_UI) -c -o $@ $<

o/imgui/%.o:$(IMGUI_DIR)/backends/%.cpp
	$(CXX) $(CXXFLAGS_UI) -c -o $@ $<
    
o/imgui/%.o:$(IMGUI_DIR)/misc/cpp/%.cpp
	$(CXX) $(CXXFLAGS_UI) -c -o $@ $<

#NAMED BULDS    
    
demo: $(EXE)
	@echo Build $(EXE) complete for $(ECHO_MESSAGE) 
    
demo_cl: $(EXE_CL)
	@echo Build $(EXE_CL) complete for $(ECHO_MESSAGE)
    
demo_ob: $(EXE_OB)
	@echo Build $(EXE_OB) complete for $(ECHO_MESSAGE) 
    
demo_ggml: $(EXE_GGML)
	@echo Build $(EXE_GGML) complete for $(ECHO_MESSAGE)
    
demo_ggml_cl: $(EXE_CL_GGML)
	@echo Build $(EXE_CL_GGML) complete for $(ECHO_MESSAGE)
    
demos_gguf: $(EXE) $(EXE_CL)
	@echo Build $(EXE) $(EXE_CL) complete for $(ECHO_MESSAGE)
    
demos: $(EXE) $(EXE_CL) $(EXE_GGML) $(EXE_CL_GGML)
	@echo Build complete for $(ECHO_MESSAGE)
    
tests: chatTest chatTest_cl chatTest_ggml
	@echo Build chatTest chatTest_cl chatTest_ggml complete for $(ECHO_MESSAGE)
    
all: $(EXE) $(EXE_CL) chatTest chatTest_cl $(EXE_GGML) chatTest_ggml
	@echo Build $(EXE) $(EXE_CL) chatTest chatTest_cl $(EXE_GGML) chatTest_ggml complete for $(ECHO_MESSAGE)
    
all_cl: $(EXE_CL) chatTest_cl
	@echo Build $(EXE_CL) chatTest_cl complete for $(ECHO_MESSAGE)
    
all_cpu: $(EXE) chatTest $(EXE_GGML) chatTest_ggml
	@echo Build $(EXE) chatTest $(EXE_GGML) chatTest_ggml complete for $(ECHO_MESSAGE)
    
all_gguf: $(EXE) $(EXE_CL) chatTest chatTest_cl
	@echo Build $(EXE) $(EXE_CL) chatTest chatTest_cl complete for $(ECHO_MESSAGE)
    
gguf_cpu: $(EXE) chatTest
	@echo Build $(EXE) chatTest complete for $(ECHO_MESSAGE)
    
all_ggml: $(EXE_GGML) chatTest_ggml
	@echo Build $(EXE_GGML) chatTest_ggml complete for $(ECHO_MESSAGE)

# MAIN EXE's    
    
$(EXE): $(OBJS) $(OBJS_GGUF) include/json.hpp tinyfiledialogs/tinyfiledialogs.c chat_plain.h thread_chat.h llama_chat1.res
	$(CXX) -o $@ $^ $(CXXFLAGS_UI) $(LDFLAGS) $(LIBS)
    
chatTest:class_chat.cpp $(OBJS_GGUF) include/json.hpp chat_plain.h thread_chat.h
	$(CXX) $(CXXFLAGS) $(filter-out %.h,$^) $(LDFLAGS) -o $@
    
#CLBLAST
    
$(EXE_CL): $(OBJS) $(OBJS_GGUF_CL) include/json.hpp tinyfiledialogs/tinyfiledialogs.c chat_plain.h thread_chat.h llama_chat1.res
	$(CXX) -o $@ $^ $(CXXFLAGS_UI_CL) $(LDFLAGS_CL) $(LIBS)
        
chatTest_cl:class_chat.cpp                                  include/json.hpp chat_plain.h thread_chat.h $(OBJS_GGUF_CL)
	$(CXX) $(filter-out %.h,$^) -o $@ $(CXXFLAGS_CL)
    
#OpenBLAS

$(EXE_OB): $(OBJS) $(OBJS_GGUF_OB) include/json.hpp tinyfiledialogs/tinyfiledialogs.c chat_plain.h thread_chat.h llama_chat1.res
	$(CXX) -o $@ $^ $(CXXFLAGS_UI) $(LDFLAGS) $(LDFLAGS_OB) $(LIBS)
    
chatTest_ob:class_chat.cpp $(OBJS_GGUF_OB) include/json.hpp chat_plain.h thread_chat.h
	$(CXX) $(CXXFLAGS) $(filter-out %.h,$^) $(LDFLAGS) $(LDFLAGS_OB) -o $@
    
#GGML format

$(EXE_GGML):$(OBJS) $(OBJS_GGML1) include/json.hpp tinyfiledialogs/tinyfiledialogs.c GGML/chat_plain.h thread_chat.h llama_chat1.res
	$(CXX) -o $@ $(filter-out %.h,$^) $(CXXFLAGS_UI_GGML) $(LDFLAGS) $(LIBS)
    
chatTest_ggml:class_chat.cpp $(OBJS_GGML1) include/json.hpp GGML/chat_plain.h thread_chat.h 
	$(CXX) $(CXXFLAGS_GGML) $(filter-out %.h,$^) $(LDFLAGS) -o $@
    
# VULKAN

chatTest_vk:class_chat.cpp $(OBJS_VK) include/json.hpp VULKAN/chat_plain.h thread_chat.h 
	$(CXX) $(CXXFLAGS_VK) $(filter-out %.h,$^) $(LDFLAGS_VK) -o $@
    
#GGML format w CLBLAST

$(EXE_CL_GGML):$(OBJS) $(OBJS_CL_GGML1) include/json.hpp tinyfiledialogs/tinyfiledialogs.c GGML/chat_plain.h thread_chat.h llama_chat1.res
	$(CXX) -o $@ $(filter-out %.h,$^) $(CXXFLAGS_UI_CL_GGML) $(LDFLAGS_CL_GGML) $(LIBS)
    
chatTest_ggml_cl:class_chat.cpp include/json.hpp GGML/chat_plain.h thread_chat.h  $(OBJS_CL_GGML1)
	$(CXX) $(filter-out %.h,$^) -o $@ $(CXXFLAGS_CL_GGML)
    
# additional

libss: o/libllama$(DSO_EXT) o/old_libllama$(DSO_EXT)

libss_cl: o/cl_libllama$(DSO_EXT) o/old_cl_libllama$(DSO_EXT)
