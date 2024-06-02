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

# Define the default target now so that it is always the first target
BUILD_TARGETS = demo demo_cl demo_ggml demo_ggml_cl

#CXX = g++
#CXX = clang++

ifndef UNAME_S
UNAME_S := $(shell uname -s)
endif

ifndef UNAME_P
UNAME_P := $(shell uname -p)
endif

ifndef UNAME_M
UNAME_M := $(shell uname -m)
endif

# In GNU make default CXX is g++ instead of c++.  Let's fix that so that users
# of non-gcc compilers don't have to provide g++ alias or wrapper.
DEFCC  := cc
DEFCXX := c++
ifeq ($(origin CC),default)
CC  := $(DEFCC)
endif
ifeq ($(origin CXX),default)
CXX := $(DEFCXX)
endif

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
CCPP = c++2a

ifdef COMPACT
OPT = -Os
CXXFLAGS_UI = -Os
endif

# -Ofast tends to produce faster code, but may not be available for some compilers.
ifdef LLAMA_FAST
OPT += -Ofast
OPTC += -Ofast
else
OPT += -O3
OPTC += -O3
endif

OPT_UI = -O3


CONFLAG =

ifdef CON
CONFLAG = -mconsole
endif

ifdef CONW
CONFLAG = -mconsole -mwindows
endif

FLAG_S = static

ifdef LLAMA_SHARED
FLAG_S = shared
endif

ifdef AVX
ARCH = -march=native -mtune=native -mavx
ARCH_NAME = _AVX
TMP = o/avx/
else
ARCH = -march=native -mtune=native
TMP = o/
endif

# IMGUI_DIR = imgui
IMGUI_DIR = imgui_f6836ff
SOURCES = main.cpp
# ifndef SDL2
# SOURCES = main_vk2.cpp
# else
# SOURCES = main_sdl2.cpp
# endif
SOURCES += $(IMGUI_DIR)/imgui.cpp $(IMGUI_DIR)/imgui_demo.cpp $(IMGUI_DIR)/imgui_draw.cpp $(IMGUI_DIR)/imgui_tables.cpp $(IMGUI_DIR)/imgui_widgets.cpp
SOURCES += $(IMGUI_DIR)/backends/imgui_impl_sdl2.cpp

#renderer
ifndef SDL2
SOURCES += $(IMGUI_DIR)/backends/imgui_impl_vulkan.cpp
RENDERER = _VK
else 
#SOURCES += $(IMGUI_DIR)/backends/imgui_impl_opengl3.cpp
SOURCES += $(IMGUI_DIR)/backends/imgui_impl_sdlrenderer2.cpp
RENDERER = _SDL2
endif

#main format
EXE = Llama_Chat_gguf$(ARCH_NAME)$(RENDERER)
EXE_OB = $(EXE)_openblas
EXE_VK = $(EXE)_vulkan
EXE_VK2 = $(EXE)_vulkan_pre
EXE_CL = $(EXE)_clblast
# old format
EXE_GGML = Llama_Chat_ggml
EXE_CL_GGML = $(EXE_GGML)_clblast

SOURCES += $(IMGUI_DIR)/misc/cpp/imgui_stdlib.cpp
OBJS0 = $(addprefix $(TMP)imgui/, $(addsuffix .o, $(basename $(notdir $(SOURCES)))))
ifndef SDL2
OBJS = $(subst $(TMP)imgui/main.o,main_vk2.cpp,$(OBJS0))
else
OBJS = $(subst $(TMP)imgui/main.o,main_sdl2.cpp,$(OBJS0))
endif
OBJS += $(TMP)tinyfiledialogs/tinyfiledialogs.o

# OBJS_UI_VK = $(subst main_vk2.cpp,VULKAN/main_vk.cpp,$(OBJS))

FILE_D = -Itinyfiledialogs
I_GGUF = -I. -Ibase -Iinclude
I_GGUF_PRE = -I. -Ipre_backend -Iinclude
I_GGML = -Iggml -Iinclude

#OBJS_OB = $(subst main.o,main_ob.o,$(OBJS))
#OBJS_CL = $(subst main.o,cl_main.o,$(OBJS))
#OBJS_GGML = $(subst main.o,old_main.o,$(OBJS))
#OBJS_CL_GGML = $(subst main.o,old_cl_main.o,$(OBJS))
#OBJS_GGML = $(subst $(TMP)main.o, ,$(OBJS))
UNAME_S := $(shell uname -s)
LINUX_GL_LIBS = -lGL

ifndef LLAMA_NO_CCACHE
CCACHE := $(shell which ccache)
ifdef CCACHE
export CCACHE_SLOPPINESS = time_macros
$(info I ccache found, compilation results will be cached. Disable with LLAMA_NO_CCACHE.)
CC    := $(CCACHE) $(CC)
CXX   := $(CCACHE) $(CXX)
else
$(info I ccache not found. Consider installing it for faster compilation.)
endif # CCACHE
endif # LLAMA_NO_CCACHE

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
CXXFLAGS_UI += $(OPT_UI) -std=$(CCPP) -fPIC -DNDEBUG $(ARCH) -DGGML_USE_K_QUANTS -DLOG_DISABLE_LOGS -w
CXXFLAGS_UI += -I$(IMGUI_DIR) -I$(IMGUI_DIR)/backends
CXXFLAGS_UI += -g -Wall -Wformat -pipe

ifdef SDL2
CXXFLAGS_UI += -DSDL2
endif

#for general ggml-gguf
CFLAGS = $(OPTC) -std=$(CCC) -fPIC $(GNUPDATEC) -DNDEBUG $(ARCH) -DGGML_USE_K_QUANTS -DLOG_DISABLE_LOGS -w -pipe

#for all chatTest
CXXFLAGS = $(OPT) -std=$(CCPP) $(GNUPDATECXX) -fPIC -DNDEBUG $(ARCH) -DGGML_USE_K_QUANTS -DLOG_DISABLE_LOGS -DGGML_USE_LLAMAFILE -w -pipe



# The stack is only 16-byte aligned on Windows, so don't let gcc emit aligned moves.
# https://gcc.gnu.org/bugzilla/show_bug.cgi?id=54412
# https://github.com/ggerganov/llama.cpp/issues/2922
ifneq '' '$(findstring mingw,$(shell $(CC) -dumpmachine))'
    CFLAGS   += -Xassembler -muse-unaligned-vector-move
    CXXFLAGS += -Xassembler -muse-unaligned-vector-move -D_WIN32_WINNT=0x602
    CXXFLAGS_UI   += -Xassembler -muse-unaligned-vector-move -D_WIN32_WINNT=0x602
endif

LIBS =
LDFLAGS  =

ifndef LLAMA_NO_OPENMP
	CXXFLAGS += -fopenmp -DGGML_USE_OPENMP
	CXXFLAGS_UI += -fopenmp -DGGML_USE_OPENMP
	CFLAGS += -fopenmp -DGGML_USE_OPENMP
endif # LLAMA_NO_OPENMP

# Windows Sockets 2 (Winsock) for network-capable apps
ifeq ($(_WIN32),1)
	LWINSOCK2 := -lws2_32
endif

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
	#CFLAGS = $(CXXFLAGS)
	#CFLAGS_CL = $(CXXFLAGS_UI_CL)
endif

ifeq ($(UNAME_S), Darwin) #APPLE
    ECHO_MESSAGE = "Mac OS X"
    LIBS += -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo `sdl2-config --libs`
    LIBS += -L/usr/local/lib -L/opt/local/lib

    CXXFLAGS_UI += `sdl2-config --cflags`
    CXXFLAGS_UI += -I/usr/local/include -I/opt/local/include	
	#CFLAGS = $(CXXFLAGS)
	#CFLAGS_CL = $(CXXFLAGS_UI_CL)
endif

ifeq ($(OS), Windows_NT)
    ECHO_MESSAGE = "MinGW"
    LIBS += -$(FLAG_S) -lgdi32 -lopengl32 -limm32 `pkg-config --$(FLAG_S) --libs sdl2`

    CXXFLAGS_UI += `pkg-config --cflags sdl2`
    #CFLAGS = $(CXXFLAGS)
    #CFLAGS_CL = $(CXXFLAGS_UI_CL)
endif

#LIBS_VK = $(LIBS) -lclblast

ifndef SDL2
LIBS += -lshell32 -lvulkan
LDFLAGS_VK = -lvulkan
#LDFLAGS_VK+ = -lclblast
else
#LDFLAGS_VK = 
#LDFLAGS_VK+ = -lvulkan-1 -lclblast
LDFLAGS_VK+ = -lvulkan
#CXXFLAGS_UI += -I$(VULKAN_DIR)/include
endif




# For emojis

CXXFLAGS_UI += -DIMGUI_USE_WCHAR32

CXXFLAGS_UI_CL = $(CXXFLAGS_UI) -DGGML_USE_CLBLAST
CXXFLAGS_UI_GGML = $(CXXFLAGS_UI) -DGGML_OLD_FORMAT
CXXFLAGS_UI_CL_GGML = $(CXXFLAGS_UI) -DGGML_OLD_FORMAT -DGGML_USE_CLBLAST
CXXFLAGS_UI_VK = $(CXXFLAGS_UI) -DGGML_USE_VULKAN

CFLAGS_CL = $(CFLAGS) -DGGML_USE_CLBLAST
CFLAGS_GGML = $(CFLAGS) -DGGML_OLD_FORMAT
CFLAGS_CL_GGML = $(CFLAGS) -DGGML_USE_CLBLAST -DGGML_OLD_FORMAT
CFLAGS_VK = $(CFLAGS) -DGGML_USE_VULKAN

CXXFLAGS_CL = $(CXXFLAGS) -DGGML_USE_CLBLAST
CXXFLAGS_GGML = $(CXXFLAGS) -DGGML_OLD_FORMAT
CXXFLAGS_CL_GGML = $(CXXFLAGS) -DGGML_USE_CLBLAST -DGGML_OLD_FORMAT
CXXFLAGS_VK = $(CXXFLAGS) -DGGML_USE_VULKAN

CXXFLAGS_E = -DGGML_EXPERIMENTAL
CXXFLAGS_E1 = -DGGML_EXPERIMENTAL1 -DGGML_EXPERIMENTAL

##---------------------------------------------------------------------
## BUILD RULES
##---------------------------------------------------------------------

GGUF_F = base
ifdef PRE_BACKEND
GGUF_F = pre_backend
endif
GGML_F = GGML
  
#GGML

OBJS_GGML1 = $(TMP)old_ggml.o $(TMP)old_ggml-alloc.o $(TMP)old_llama.o $(TMP)old_common.o $(TMP)old_k_quants.o $(TMP)old_grammar-parser.o

$(TMP)old_ggml.o: GGML/ggml.c GGML/ggml.h
	$(CC)  $(CFLAGS_GGML)   -c $< -o $@
    
$(TMP)old_ggml-alloc.o: GGML/ggml-alloc.c GGML/ggml.h GGML/ggml-alloc.h
	$(CC)  $(CFLAGS_GGML)   -c $< -o $@

$(TMP)old_llama.o: GGML/llama.cpp GGML/ggml.h GGML/ggml-alloc.h GGML/llama.h  GGML/llama-util.h
	$(CXX) $(CXXFLAGS_GGML) -c $< -o $@

$(TMP)old_common.o: GGML/common.cpp GGML/common.h
	$(CXX) $(CXXFLAGS_GGML) -c $< -o $@

$(TMP)old_libllama$(DSO_EXT): $(TMP)old_llama.o $(TMP)old_ggml.o
	$(CXX) $(CXXFLAGS_GGML) -shared -fPIC -o $@ $^ $(LDFLAGS)
    
$(TMP)old_k_quants.o: GGML/k_quants.c GGML/k_quants.h
	$(CC) $(CFLAGS_GGML) -c $< -o $@
    
$(TMP)old_grammar-parser.o: GGML/grammar-parser.cpp GGML/grammar-parser.h
	$(CXX) $(CXXFLAGS_GGML) -c $< -o $@

# Separate GGML CLBLAST

# Mac provides OpenCL as a framework
ifeq ($(UNAME_S),Darwin)
    LDFLAGS_CL_GGML += -lclblast -framework OpenCL
else
    #LDFLAGS2 += -lclblast -lOpenCL
    LDFLAGS_CL_GGML += -Llib/OpenCL.lib -Llib/clblast.lib
endif
    #LDFLAGS_CL += -lclblast -lOpenCL
CXXFLAGS_CL_GGML += -lclblast -lOpenCL
CXXFLAGS_UI_CL_GGML += -lclblast -lOpenCL


OBJS_CL_GGML1    = $(TMP)old_cl_k_quants.o $(TMP)old_cl_ggml-opencl.o $(TMP)old_cl_ggml.o $(TMP)old_cl_ggml-alloc.o $(TMP)old_cl_llama.o $(TMP)old_cl_common.o $(TMP)old_cl_grammar-parser.o

$(TMP)old_cl_ggml-opencl.o: GGML/ggml-opencl.cpp GGML/ggml-opencl.h
	$(CXX) $(CXXFLAGS_CL_GGML) -c $< -o $@
    
$(TMP)old_cl_ggml.o: GGML/ggml.c GGML/ggml.h
	$(CC)  $(CFLAGS_CL_GGML)   -c $< -o $@
    
$(TMP)old_cl_ggml-alloc.o: GGML/ggml-alloc.c GGML/ggml.h GGML/ggml-alloc.h
	$(CC)  $(CFLAGS_CL_GGML)   -c $< -o $@

$(TMP)old_cl_llama.o: GGML/llama.cpp GGML/ggml.h GGML/ggml-alloc.h GGML/llama.h
	$(CXX) $(CXXFLAGS_CL_GGML) -c $< -o $@

$(TMP)old_cl_common.o: GGML/common.cpp GGML/common.h
	$(CXX) $(CXXFLAGS_CL_GGML) -c $< -o $@

$(TMP)old_cl_libllama$(DSO_EXT): $(TMP)old_cl_llama.o $(TMP)old_cl_ggml.o
	$(CXX) $(CXXFLAGS_CL_GGML) -shared -fPIC -o $@ $^ $(LDFLAGS_CL_GGML)
    
$(TMP)old_cl_k_quants.o: GGML/k_quants.c GGML/k_quants.h
	$(CC) $(CFLAGS_CL_GGML) -c $< -o $@
    
$(TMP)old_cl_grammar-parser.o: GGML/grammar-parser.cpp GGML/grammar-parser.h
	$(CXX) $(CXXFLAGS_CL_GGML) -c $< -o $@
  
#VULKAN



#CXXFLAGS_VK += -I$(VULKAN_DIR)/include

OBJS_VK = $(TMP)vk_ggml.o $(TMP)vk_ggml-alloc.o $(TMP)vk_ggml-backend.o $(TMP)vk_llama.o $(TMP)vk_sampling.o $(TMP)vk_common.o $(TMP)vk_ggml-quants.o $(TMP)vk_grammar-parser.o $(TMP)vk_ggml-vulkan.o $(TMP)vk_unicode.o $(TMP)vk_unicode-data.o $(TMP)vk_sgemm.o

$(TMP)vk_ggml.o: base/ggml.c base/ggml.h
	$(CC)  $(CFLAGS_VK)   -c $< -o $@
	
$(TMP)vk_ggml-alloc.o: base/ggml-alloc.c base/ggml.h base/ggml-alloc.h
	$(CC)  $(CFLAGS_VK)   -c $< -o $@
	
$(TMP)vk_ggml-backend.o: base/ggml-backend.c base/ggml.h base/ggml-backend.h
	$(CC)  $(CFLAGS_VK)   -c $< -o $@
	
$(TMP)vk_ggml-vulkan.o: base/ggml-vulkan.cpp base/ggml-vulkan.h
	$(CXX) $(CXXFLAGS_VK) $(LDFLAGS_VK) -c $< -o $@

$(TMP)vk_ggml-quants.o: base/ggml-quants.c base/ggml.h base/ggml-quants.h base/ggml-common.h
	$(CC) $(CFLAGS_VK)    -c $< -o $@
	
$(TMP)vk_unicode.o: base/unicode.cpp base/unicode.h
	$(CXX) $(CXXFLAGS_VK) -c $< -o $@

$(TMP)vk_unicode-data.o: base/unicode-data.cpp base/unicode-data.h
	$(CXX) $(CXXFLAGS_VK) -c $< -o $@
	
$(TMP)vk_llama.o: base/llama.cpp base/unicode.h base/ggml.h base/ggml-alloc.h base/ggml-backend.h base/llama.h
	$(CXX) $(CXXFLAGS_VK) -c $< -o $@
	
VK_COMMON_H_DEPS = base/common.h base/sampling.h base/llama.h
VK_COMMON_DEPS   = $(TMP)vk_common.o $(TMP)vk_sampling.o $(TMP)vk_grammar-parser.o

$(TMP)vk_common.o: base/common.cpp $(VK_COMMON_H_DEPS)
	$(CXX) $(CXXFLAGS_VK) -c $< -o $@
	
$(TMP)vk_sampling.o: base/sampling.cpp $(VK_COMMON_H_DEPS)
	$(CXX) $(CXXFLAGS_VK) -c $< -o $@
    
	
$(TMP)vk_sgemm.o: base/sgemm.cpp base/sgemm.h base/ggml.h
	$(CXX) $(CXXFLAGS_VK) -c $< -o $@
    
$(TMP)vk_grammar-parser.o: base/grammar-parser.cpp base/grammar-parser.h
	$(CXX) $(CXXFLAGS_VK) -c $< -o $@
  
#####################################
################################ GGUF

OBJS_GGUF = $(TMP)ggml.o $(TMP)ggml-alloc.o $(TMP)ggml-backend.o $(TMP)llama.o $(TMP)sampling.o $(TMP)common.o $(TMP)ggml-quants.o $(TMP)grammar-parser.o $(TMP)unicode.o $(TMP)unicode-data.o $(TMP)sgemm.o
    
$(TMP)tinyfiledialogs/tinyfiledialogs.o: tinyfiledialogs/tinyfiledialogs.c tinyfiledialogs/tinyfiledialogs.h
	$(CC)  $(CFLAGS)   -c $< -o $@

$(TMP)ggml.o: base/ggml.c base/ggml.h
	$(CC)  $(CFLAGS)   -c $< -o $@
	
$(TMP)ggml-alloc.o: base/ggml-alloc.c base/ggml.h base/ggml-alloc.h
	$(CC)  $(CFLAGS)   -c $< -o $@
	
$(TMP)ggml-backend.o: base/ggml-backend.c base/ggml.h base/ggml-backend.h
	$(CC)  $(CFLAGS)   -c $< -o $@

$(TMP)ggml-quants.o: base/ggml-quants.c base/ggml.h base/ggml-quants.h base/ggml-common.h
	$(CC) $(CFLAGS)    -c $< -o $@
	
$(TMP)unicode.o: base/unicode.cpp base/unicode.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TMP)unicode-data.o: base/unicode-data.cpp base/unicode-data.h
	$(CXX) $(CXXFLAGS) -c $< -o $@
	
$(TMP)llama.o: base/llama.cpp base/unicode.h base/ggml.h base/ggml-alloc.h base/ggml-backend.h base/llama.h
	$(CXX) $(CXXFLAGS) -c $< -o $@
	
COMMON_H_DEPS = base/common.h base/sampling.h base/llama.h
COMMON_DEPS   = $(TMP)common.o $(TMP)sampling.o $(TMP)grammar-parser.o

$(TMP)common.o: base/common.cpp $(COMMON_H_DEPS)
	$(CXX) $(CXXFLAGS) -c $< -o $@
	
$(TMP)sampling.o: base/sampling.cpp $(COMMON_H_DEPS)
	$(CXX) $(CXXFLAGS) -c $< -o $@
    
$(TMP)sgemm.o: base/sgemm.cpp base/sgemm.h base/ggml.h
	$(CXX) $(CXXFLAGS) -c $< -o $@
    
$(TMP)grammar-parser.o: base/grammar-parser.cpp base/grammar-parser.h
	$(CXX) $(CXXFLAGS) -c $< -o $@
   
    
# Separate CLBLAST

# Mac provides OpenCL as a framework
ifeq ($(UNAME_S),Darwin)
    LDFLAGS_CL += -lclblast -framework OpenCL
else
    #LDFLAGS2 += -lclblast -lOpenCL
    LDFLAGS_CL += -Llib/OpenCL.lib -Llib/clblast.lib
    #LDFLAGS_CL += -lclblast -lOpenCL
endif

CXXFLAGS_CL += -lclblast -lOpenCL
CXXFLAGS_UI_CL += -lclblast -lOpenCL


#OBJS_GGUF_CL    = $(TMP)cl_ggml-quants.o $(TMP)cl_ggml-opencl-gguf.o $(TMP)cl_ggml.o $(TMP)cl_ggml-alloc.o $(TMP)cl_ggml-backend.o $(TMP)cl_llama.o $(TMP)cl_sampling.o $(TMP)cl_common.o $(TMP)cl_grammar-parser.o
OBJS_GGUF_CL    = $(TMP)cl_ggml.o $(TMP)cl_ggml-quants.o $(TMP)cl_ggml-opencl-gguf.o $(TMP)cl_ggml-alloc.o $(TMP)cl_ggml-backend.o $(TMP)cl_llama.o $(TMP)cl_sampling.o $(TMP)cl_common.o $(TMP)cl_grammar-parser.o $(TMP)cl_unicode.o $(TMP)cl_unicode-data.o $(TMP)cl_sgemm.o

$(TMP)cl_ggml-opencl-gguf.o: base/ggml-opencl.cpp base/ggml-opencl.h
	$(CXX) $(CXXFLAGS_CL) -c $< -o $@
    
$(TMP)cl_grammar-parser.o: base/grammar-parser.cpp base/grammar-parser.h
	$(CXX) $(CXXFLAGS_CL) -c $< -o $@
	
$(TMP)cl_ggml.o: base/ggml.c base/ggml.h
	$(CC)  $(CFLAGS_CL)   -c $< -o $@
	
$(TMP)cl_ggml-alloc.o: base/ggml-alloc.c base/ggml.h base/ggml-alloc.h
	$(CC)  $(CFLAGS_CL)   -c $< -o $@
	
$(TMP)cl_ggml-backend.o: base/ggml-backend.c base/ggml.h base/ggml-backend.h
	$(CC)  $(CFLAGS_CL)   -c $< -o $@
	
$(TMP)cl_ggml-quants.o: base/ggml-quants.c base/ggml.h base/ggml-quants.h base/ggml-common.h
	$(CC) $(CFLAGS)    -c $< -o $@
	
$(TMP)cl_unicode.o: base/unicode.cpp base/unicode.h
	$(CXX) $(CXXFLAGS_CL) -c $< -o $@

$(TMP)cl_unicode-data.o: base/unicode-data.cpp base/unicode-data.h
	$(CXX) $(CXXFLAGS_CL) -c $< -o $@
	
$(TMP)cl_llama.o: base/llama.cpp base/unicode.h base/ggml.h base/ggml-alloc.h base/ggml-backend.h base/llama.h
	$(CXX) $(CXXFLAGS_CL) -c $< -o $@
	
CL_COMMON_H_DEPS = base/common.h base/sampling.h base/llama.h

$(TMP)cl_common.o: base/common.cpp $(CL_COMMON_H_DEPS)
	$(CXX) $(CXXFLAGS_CL) -c $< -o $@
	
$(TMP)cl_sampling.o: base/sampling.cpp $(CL_COMMON_H_DEPS)
	$(CXX) $(CXXFLAGS_CL) -c $< -o $@
	
$(TMP)cl_sgemm.o: base/sgemm.cpp base/sgemm.h base/ggml.h
	$(CXX) $(CXXFLAGS_CL) -c $< -o $@
    
# pre-backend

OBJS_GGUF_CL_PRE_BACKEND    = $(TMP)cl_pb_ggml.o $(TMP)cl_pb_ggml-quants.o $(TMP)cl_pb_ggml-opencl-gguf.o $(TMP)cl_pb_ggml-alloc.o $(TMP)cl_pb_ggml-backend.o $(TMP)cl_pb_llama.o $(TMP)cl_pb_sampling.o $(TMP)cl_pb_common.o $(TMP)cl_pb_grammar-parser.o

$(TMP)cl_pb_ggml-opencl-gguf.o: pre_backend/ggml-opencl.cpp pre_backend/ggml-opencl.h
	$(CXX) $(CXXFLAGS_CL) -c $< -o $@
    
$(TMP)cl_pb_ggml.o: pre_backend/ggml.c pre_backend/ggml.h
	$(CC)  $(CFLAGS_CL)   -c $< -o $@
    
$(TMP)cl_pb_ggml-alloc.o: pre_backend/ggml-alloc.c pre_backend/ggml.h pre_backend/ggml-alloc.h
	$(CC)  $(CFLAGS_CL)   -c $< -o $@
    
$(TMP)cl_pb_ggml-backend.o: pre_backend/ggml-backend.c pre_backend/ggml.h pre_backend/ggml-backend.h
	$(CC)  $(CFLAGS)   -c $< -o $@

$(TMP)cl_pb_llama.o: pre_backend/llama.cpp pre_backend/ggml.h pre_backend/ggml-alloc.h pre_backend/ggml-backend.h pre_backend/llama.h
	$(CXX) $(CXXFLAGS_CL) -c $< -o $@
    
$(TMP)cl_pb_sampling.o: pre_backend/sampling.cpp pre_backend/sampling.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TMP)cl_pb_common.o: pre_backend/common.cpp pre_backend/common.h $(TMP)cl_sampling.o
	$(CXX) $(CXXFLAGS_CL) -c $< -o $@
    
$(TMP)cl_pb_ggml-quants.o: pre_backend/ggml-quants.c pre_backend/ggml.h pre_backend/ggml-quants.h
	$(CC) $(CFLAGS)    -c $< -o $@
    
$(TMP)cl_pb_grammar-parser.o: pre_backend/grammar-parser.cpp pre_backend/grammar-parser.h
	$(CXX) $(CXXFLAGS_CL) -c $< -o $@
    
    
# separate OpenBlas


ifneq ($(shell grep -e "Arch Linux" -e "ID_LIKE=arch" /etc/os-release 2>/dev/null),)
    CFLAGS_OB  += -DGGML_USE_OPENBLAS -I/usr/local/include/openblas -I/usr/include/openblas
    LDFLAGS_OB += -lopenblas -lcblas
else
    CFLAGS_OB  += -DGGML_USE_OPENBLAS -I/include/openblas -I/include/openblas
    LDFLAGS_OB += -lopenblas
endif

OBJS_GGUF_OB = $(TMP)ggml_ob.o $(TMP)ggml-alloc_ob.o $(TMP)llama_ob.o $(TMP)common_ob.o $(TMP)k_quants_ob.o $(TMP)grammar-parser_ob.o
  
$(TMP)ggml_ob.o: base/ggml.c base/ggml.h
	$(CC)  $(CFLAGS) $(CFLAGS_OB)   -c $< -o $@
    
$(TMP)ggml-alloc_ob.o: base/ggml-alloc.c base/ggml.h base/ggml-alloc.h
	$(CC)  $(CFLAGS) $(CFLAGS_OB)   -c $< -o $@

#base/threadpool.h
$(TMP)llama_ob.o: base/llama.cpp base/ggml.h base/ggml-alloc.h base/llama.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TMP)common_ob.o: base/common.cpp base/common.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TMP)libllama_ob$(DSO_EXT): $(TMP)llama.o $(TMP)ggml.o $(OBJS_GGUF_OB)
	$(CXX) $(CXXFLAGS) -shared -fPIC -o $@ $^ $(LDFLAGS)
    
$(TMP)k_quants_ob.o: base/k_quants.c base/k_quants.h
	$(CC) $(CFLAGS) $(CFLAGS_OB) -c $< -o $@
    
$(TMP)grammar-parser_ob.o: base/grammar-parser.cpp base/grammar-parser.h
	$(CXX) $(CXXFLAGS) -c $< -o $@
    
# general    
    
$(TMP)imgui/%.o:$(IMGUI_DIR)/%.cpp
	$(CXX) $(CXXFLAGS_UI) -c -o $@ $<

$(TMP)imgui/%.o:$(IMGUI_DIR)/backends/%.cpp
	$(CXX) $(CXXFLAGS_UI) -c -o $@ $<
    
$(TMP)imgui/%.o:$(IMGUI_DIR)/misc/cpp/%.cpp
	$(CXX) $(CXXFLAGS_UI) -c -o $@ $<
    
#CCACHE

# $< is the first prerequisite, i.e. the source file.
# Explicitly compile this to an object file so that it can be cached with ccache.
# The source file is then filtered out from $^ (the list of all prerequisites) and the object file is added instead.

# Helper function that replaces .c, .cpp, and .cu file endings with .o:
GET_OBJ_FILE = $(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(patsubst %.cu,%.o,$(1))))

#NAMED BULDS

chat: $(EXE)_mini
	@echo Build $(EXE)_mini complete for $(ECHO_MESSAGE)
    
chat_cl: $(EXE_CL)_mini
	@echo Build $(EXE_CL)_mini complete for $(ECHO_MESSAGE)
    
chat_vk: $(EXE_VK)_mini
	@echo Build $(EXE_VK)_mini complete for $(ECHO_MESSAGE)
    
demo: $(EXE)
	@echo Build $(EXE) complete for $(ECHO_MESSAGE) 
     
demo_cl: $(EXE_CL)
	@echo Build $(EXE_CL) complete for $(ECHO_MESSAGE)
    
demo_mini: $(EXE)_mini
	@echo Build $(EXE)_mini complete for $(ECHO_MESSAGE)
    
demo_cl_mini: $(EXE_CL)_mini
	@echo Build $(EXE_CL)_mini complete for $(ECHO_MESSAGE)
    
demo_cl_mini_pre_backend: $(EXE_CL)_mini_pre_backend
	@echo Build $(EXE_CL)_mini_pre_backend complete for $(ECHO_MESSAGE)
    
demo_vk: $(EXE_VK)
	@echo Build $(EXE_VK) complete for $(ECHO_MESSAGE)
    
demo_vk_mini: $(EXE_VK)_mini
	@echo Build $(EXE_VK)_mini complete for $(ECHO_MESSAGE)
    
demo_vk2: $(EXE_VK2)
	@echo Build $(EXE_VK2) complete for $(ECHO_MESSAGE)
    
demo_vk2_mini: $(EXE_VK2)_mini
	@echo Build $(EXE_VK2)_mini complete for $(ECHO_MESSAGE)
    
demo_ob: $(EXE_OB)
	@echo Build $(EXE_OB) complete for $(ECHO_MESSAGE) 
    
demo_ggml: $(EXE_GGML)
	@echo Build $(EXE_GGML) complete for $(ECHO_MESSAGE)
    
demo_ggml_cl: $(EXE_CL_GGML)
	@echo Build $(EXE_CL_GGML) complete for $(ECHO_MESSAGE)
    
demos_gguf: $(EXE) $(EXE_CL)
	@echo Build $(EXE) $(EXE_CL) complete for $(ECHO_MESSAGE)
    
demos: $(EXE)_mini $(EXE_CL)_mini $(EXE_VK)_mini
	@echo Build complete for $(ECHO_MESSAGE)
    
demos_old: $(EXE) $(EXE_CL) $(EXE_VK)
	@echo Build complete for $(ECHO_MESSAGE)
    
tests: chatTest chatTest_cl chatTest_vk
	@echo Build chatTest chatTest_cl complete for $(ECHO_MESSAGE)
    
all: $(EXE)_mini $(EXE_CL)_mini chatTest chatTest_cl chatTest_vk
	@echo Build $(EXE)_mini $(EXE_CL)_mini chatTest chatTest_cl chatTest_vk complete for $(ECHO_MESSAGE)
    
all_cl: $(EXE_CL)_mini chatTest_cl
	@echo Build $(EXE_CL)_mini chatTest_cl complete for $(ECHO_MESSAGE)
    
all_cpu: $(EXE)_mini chatTest
	@echo Build $(EXE) chatTest complete for $(ECHO_MESSAGE)
    
all_gguf: $(EXE) $(EXE_CL) chatTest chatTest_cl
	@echo Build $(EXE) $(EXE_CL) chatTest chatTest_cl complete for $(ECHO_MESSAGE)
    
gguf_cpu: $(EXE) chatTest
	@echo Build $(EXE) chatTest complete for $(ECHO_MESSAGE)
    
all_ggml: $(EXE_GGML) chatTest_ggml
	@echo Build $(EXE_GGML) chatTest_ggml complete for $(ECHO_MESSAGE)

# MAIN EXE's    
    
$(EXE): $(OBJS) $(OBJS_GGUF) chat_plain.h thread_chat.h UI.h llama_chat1.res
	$(CXX) $(I_GGUF) $(FILE_D) -o $@ $^ $(CXXFLAGS_UI) $(CONFLAG) $(LDFLAGS) $(LIBS)
     
$(EXE)_mini: $(OBJS) $(OBJS_GGUF) chat_plain.h thread_chat.h UI_simple.h llama_chat1.res
	$(CXX) $(I_GGUF) $(FILE_D) -o $@ $^ $(CXXFLAGS_UI) -DUI_SIMPLE $(CONFLAG) $(LDFLAGS) $(LIBS)
    
chatTest:class_chat.cpp $(OBJS_GGUF) chat_plain.h thread_chat.h
	$(CXX) $(I_GGUF) $(CXXFLAGS) $(filter-out %.h,$^) $(LDFLAGS) -o $@
    
#CLBLAST
    
$(EXE_CL): $(OBJS) $(OBJS_GGUF_CL) chat_plain.h thread_chat.h UI.h llama_chat1.res
	$(CXX) $(I_GGUF) $(FILE_D) -o $@ $^ $(CXXFLAGS_UI_CL) $(CONFLAG) $(LDFLAGS_CL) $(LIBS)
    
$(EXE_CL)_mini: $(OBJS) $(OBJS_GGUF_CL) chat_plain.h thread_chat.h UI_simple.h llama_chat1.res
	$(CXX) $(I_GGUF) $(FILE_D) -o $@ $^ $(CXXFLAGS_UI_CL) -DUI_SIMPLE $(CONFLAG) $(LDFLAGS_CL) $(LIBS)
	#$(CXX) $(I_GGUF) $(FILE_D) $(CXXFLAGS_UI_CL) -DUI_SIMPLE -c $< -o $(call GET_OBJ_FILE, $<)
	#$(CXX) $(I_GGUF) $(FILE_D) $(CXXFLAGS_UI_CL) -DUI_SIMPLE $(filter-out %.h $<,$^) $(call GET_OBJ_FILE, $<) -o $@ $(CONFLAG) $(LDFLAGS_CL) $(LIBS)

chatTest_cl:class_chat.cpp                                  chat_plain.h thread_chat.h $(OBJS_GGUF_CL)
	$(CXX) $(I_GGUF) $(filter-out %.h,$^) -o $@ $(CXXFLAGS_CL)

# VULKAN

$(EXE_VK): $(OBJS) $(OBJS_VK) chat_plain.h thread_chat.h UI.h llama_chat1.res
	 $(CXX) $(I_GGUF) $(FILE_D) $(CXXFLAGS_UI_VK) -o $@ $^ $(CONFLAG) $(LIBS) $(LDFLAGS_VK+)
     
$(EXE_VK)_mini: $(OBJS) $(OBJS_VK) chat_plain.h thread_chat.h UI_simple.h llama_chat1.res
	# $(CXX) $(I_GGUF) $(FILE_D) $(CXXFLAGS_UI_VK) -DUI_SIMPLE -o $@ $^ $(CONFLAG) $(LIBS) $(LDFLAGS_VK+)
	$(CXX) $(I_GGUF) $(FILE_D) $(CXXFLAGS_UI_VK) -DUI_SIMPLE -c $< -o $(call GET_OBJ_FILE, $<)
	$(CXX) $(I_GGUF) $(FILE_D) $(CXXFLAGS_UI_VK) -DUI_SIMPLE $(filter-out %.h $<,$^) $(call GET_OBJ_FILE, $<) -o $@ $(CONFLAG) $(LIBS) $(LDFLAGS_VK+)
     
chatTest_vk:class_chat.cpp $(OBJS_VK) chat_plain.h thread_chat.h
	#$(CXX)  $(I_GGUF) $(CXXFLAGS_VK) $(filter-out %.h,$^) $(LDFLAGS_VK) $(LDFLAGS_VK+) -o $@
	$(CXX)  $(I_GGUF) $(CXXFLAGS_VK) -c $< -o $(call GET_OBJ_FILE, $<)
	$(CXX)  $(I_GGUF) $(CXXFLAGS_VK) $(filter-out %.h $<,$^) $(call GET_OBJ_FILE, $<) -o $@ $(LDFLAGS_VK) $(LDFLAGS_VK+)
#-

$(EXE_VK2): $(OBJS) $(OBJS_VK2) chat_plain.h thread_chat.h UI.h llama_chat1.res
	 $(CXX) -I. -Iinclude -IVULKAN2 $(FILE_D) $(CXXFLAGS_UI_VK) -o $@ $^ $(CONFLAG) $(LIBS) $(LDFLAGS_VK+)
     
$(EXE_VK2)_mini: $(OBJS) $(OBJS_VK2) chat_plain.h thread_chat.h UI_simple.h llama_chat1.res
	 $(CXX) -I. -Iinclude -IVULKAN2 $(FILE_D) $(CXXFLAGS_UI_VK) -DUI_SIMPLE -o $@ $^ $(CONFLAG) $(LIBS) $(LDFLAGS_VK+)
    
chatTest_vk2:class_chat.cpp $(OBJS_VK2) chat_plain.h thread_chat.h
	$(CXX) -I. -Iinclude -IVULKAN2 $(CXXFLAGS_VK) $(filter-out %.h,$^) $(LDFLAGS_VK) $(LDFLAGS_VK+) -o $@

# OTHER
#OpenBLAS

$(EXE_OB): $(OBJS) $(OBJS_GGUF_OB) tinyfiledialogs/tinyfiledialogs.c chat_plain.h thread_chat.h llama_chat1.res
	$(CXX) -Ibase -Iinclude -o $@ $^ $(CXXFLAGS_UI) $(CONFLAG) $(LDFLAGS) $(LDFLAGS_OB) $(LIBS)
    
chatTest_ob:class_chat.cpp $(OBJS_GGUF_OB) include/json.hpp chat_plain.h thread_chat.h
	$(CXX) -Ibase -Iinclude $(CXXFLAGS) $(filter-out %.h,$^) $(LDFLAGS) $(LDFLAGS_OB) -o $@
    
#GGML format

$(EXE_GGML):$(OBJS) $(OBJS_GGML1) chat_plain.h thread_chat.h UI.h llama_chat1.res
	$(CXX) $(I_GGML) $(FILE_D) -o $@ $(filter-out %.h,$^) $(CXXFLAGS_UI_GGML) $(LDFLAGS) $(LIBS)
    
chatTest_ggml:class_chat.cpp $(OBJS_GGML1) chat_plain.h thread_chat.h 
	$(CXX) $(I_GGML) $(CXXFLAGS_GGML) $(filter-out %.h,$^) $(LDFLAGS) -o $@
    
#GGML format w CLBLAST

$(EXE_CL_GGML):$(OBJS) $(OBJS_CL_GGML1) chat_plain.h thread_chat.h UI.h llama_chat1.res
	$(CXX) $(I_GGML) $(FILE_D) -o $@ $(filter-out %.h,$^) $(CXXFLAGS_UI_CL_GGML) $(LDFLAGS_CL_GGML) $(LIBS)
    
chatTest_ggml_cl:class_chat.cpp chat_plain.h thread_chat.h   $(OBJS_CL_GGML1)
	$(CXX) $(I_GGML) $(filter-out %.h,$^) -o $@ $(CXXFLAGS_CL_GGML)
    
#CLBLAST pre-backend
    
$(EXE_CL)_mini_pre_backend: $(OBJS) $(OBJS_GGUF_CL_PRE_BACKEND) chat_plain.h thread_chat.h UI_simple.h llama_chat1.res
	$(CXX) $(I_GGUF_PRE) $(FILE_D) -o $@ $^ $(CXXFLAGS_UI_CL) -DUI_SIMPLE $(CONFLAG) $(LDFLAGS_CL) $(LIBS)

chatTest_cl_pre_backend:class_chat.cpp chat_plain.h thread_chat.h $(OBJS_GGUF_CL_PRE_BACKEND)
	$(CXX) $(I_GGUF_PRE) $(filter-out %.h,$^) -o $@ $(CXXFLAGS_CL)
    
# additional

libss: $(TMP)libllama$(DSO_EXT) $(TMP)old_libllama$(DSO_EXT)

libss_cl: $(TMP)cl_libllama$(DSO_EXT) $(TMP)old_cl_libllama$(DSO_EXT)

chatTestHTTP:class_chat_http.cpp $(OBJS_GGUF) include/json.hpp include/httplib.h chat_plain.h thread_chat.h
	$(CXX) $(CXXFLAGS) -Iinclude $(filter-out %.h,$^) -o $@ $(LWINSOCK2)
    
test-sampling_vk2:test-sampling.cpp $(OBJS_VK2)
	$(CXX) -I. -Iinclude -IVULKAN2 $(CXXFLAGS_VK) $(filter-out %.h,$^) $(LDFLAGS_VK) $(LDFLAGS_VK+) -o $@
    
test-sampling_cl:test-sampling.cpp $(OBJS_GGUF_CL)
	$(CXX) $(I_GGUF) $(filter-out %.h,$^) -o $@ $(CXXFLAGS_CL)
    
test-sampling_cl_pre_backend:test-sampling.cpp $(OBJS_GGUF_CL_PRE_BACKEND)
	$(CXX) $(I_GGUF_PRE) $(filter-out %.h,$^) -o $@ $(CXXFLAGS_CL)
