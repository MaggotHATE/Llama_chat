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

#the main folder for groups of sources
base = base
ggmlsrc_f = $(base)/ggml
llamacpp_f = $(base)/llama
common_f = $(base)


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

vpath=$(ggmlsrc_f):$(llamacpp_f):$(common_f):include

FILE_D = -Itinyfiledialogs
I_GGUF = -I$(ggmlsrc_f) -I$(llamacpp_f) -I$(common_f) -Iinclude -I.
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
CFLAGS = $(I_GGUF) $(OPTC) -std=$(CCC) -fPIC $(GNUPDATEC) -DNDEBUG $(ARCH) -DGGML_USE_K_QUANTS -DLOG_DISABLE_LOGS -w -pipe

#for all chatTest
CXXFLAGS = $(I_GGUF) $(OPT) -std=$(CCPP) $(GNUPDATECXX) -fPIC -DNDEBUG $(ARCH) -DGGML_USE_K_QUANTS -DLOG_DISABLE_LOGS -DGGML_USE_LLAMAFILE -w -pipe



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
LDFLAGS_VK = $(shell pkg-config --libs vulkan)
#LDFLAGS_VK+ = -lclblast
else
#LDFLAGS_VK = 
#LDFLAGS_VK+ = -lvulkan-1 -lclblast
LDFLAGS_VK+ = $(shell pkg-config --libs vulkan)
#CXXFLAGS_UI += -I$(VULKAN_DIR)/include
endif


# For emojis

CXXFLAGS_UI += -DIMGUI_USE_WCHAR32

CXXFLAGS_UI_CL = $(CXXFLAGS_UI) -DGGML_USE_CLBLAST
CXXFLAGS_UI_VK = $(CXXFLAGS_UI) -DGGML_USE_VULKAN

CFLAGS_CL = $(CFLAGS) -DGGML_USE_CLBLAST
CFLAGS_VK = $(CFLAGS) -DGGML_USE_VULKAN

CXXFLAGS_CL = $(CXXFLAGS) -DGGML_USE_CLBLAST
CXXFLAGS_VK = $(CXXFLAGS) -DGGML_USE_VULKAN

ifdef VK_DEBUG
	CXXFLAGS_VK += -DGGML_VULKAN_DEBUG
endif # VK_DEBUG

ifdef VK_MEMDEBUG
	CXXFLAGS_VK += -DGGML_VULKAN_MEMORY_DEBUG
endif # VK_MEMDEBUG

ifdef VK_VALID
	CXXFLAGS_VK += -DGGML_VULKAN_VALIDATE
endif # VK_VALID

ifdef VK_PERF
	CXXFLAGS_VK  += -DGGML_VULKAN_PERF
endif


##---------------------------------------------------------------------
## BUILD RULES
##---------------------------------------------------------------------

GGUF_F = base
ifdef PRE_BACKEND
GGUF_F = pre_backend
endif
GGML_F = GGML
#VULKAN OLD

#CXXFLAGS_VK += -I$(VULKAN_DIR)/include

# OBJS_VK = $(TMP)vk_ggml.o $(TMP)vk_ggml-alloc.o $(TMP)vk_ggml-backend.o $(TMP)vk_llama.o $(TMP)vk_llama-addon.o $(TMP)vk_sampling.o $(TMP)vk_common.o $(TMP)vk_ggml-quants.o $(TMP)vk_grammar-parser.o $(TMP)vk_ggml-vulkan.o $(TMP)vk_ggml-vulkan-shaders.o $(TMP)vk_unicode.o $(TMP)vk_unicode-data.o $(TMP)vk_sgemm.o

# $(TMP)vk_ggml.o: base/ggml.c base/ggml.h
	# $(CC)  $(CFLAGS_VK)   -c $< -o $@
	
# $(TMP)vk_ggml-alloc.o: base/ggml-alloc.c base/ggml.h base/ggml-alloc.h
	# $(CC)  $(CFLAGS_VK)   -c $< -o $@
	
# $(TMP)vk_ggml-backend.o: base/ggml-backend.c base/ggml.h base/ggml-backend.h
	# $(CC)  $(CFLAGS_VK)   -c $< -o $@

# $(TMP)vk_ggml-quants.o: base/ggml-quants.c base/ggml.h base/ggml-quants.h base/ggml-common.h
	# $(CC) $(CFLAGS_VK)    -c $< -o $@
	
# $(TMP)vk_unicode.o: base/unicode.cpp base/unicode.h
	# $(CXX) $(CXXFLAGS_VK) -c $< -o $@

# $(TMP)vk_unicode-data.o: base/unicode-data.cpp base/unicode-data.h
	# $(CXX) $(CXXFLAGS_VK) -c $< -o $@
	
# $(TMP)vk_llama.o: base/llama.cpp base/unicode.h base/ggml.h base/ggml-alloc.h base/ggml-backend.h base/llama.h
	# $(CXX) $(CXXFLAGS_VK) -c $< -o $@
	
# VK_COMMON_H_DEPS = base/common.h base/sampling.h base/llama-addon.h base/llama.h
# VK_COMMON_DEPS   = $(TMP)vk_common.o $(TMP)vk_sampling.o $(TMP)vk_grammar-parser.o

# $(TMP)vk_common.o: base/common.cpp $(VK_COMMON_H_DEPS)
	# $(CXX) $(CXXFLAGS_VK) -c $< -o $@
    
# $(TMP)vk_llama-addon.o: base/llama-addon.cpp $(COMMON_H_DEPS)
	# $(CXX) $(CXXFLAGS_VK) -c $< -o $@
	
# $(TMP)vk_sampling.o: base/sampling.cpp $(VK_COMMON_H_DEPS)
	# $(CXX) $(CXXFLAGS_VK) -c $< -o $@
    
	
# $(TMP)vk_sgemm.o: base/sgemm.cpp base/sgemm.h base/ggml.h
	# $(CXX) $(CXXFLAGS_VK) -c $< -o $@
    
# $(TMP)vk_grammar-parser.o: base/grammar-parser.cpp base/grammar-parser.h
	# $(CXX) $(CXXFLAGS_VK) -c $< -o $@
    
# vulkan-shaders-gen: base/vulkan-shaders-gen.cpp
	# $(CXX) $(CXXFLAGS_VK) -o $@ $(LDFLAGS_VK) base/vulkan-shaders-gen.cpp
	
# $(TMP)vk_ggml-vulkan-shaders.o: base/ggml-vulkan-shaders.cpp base/ggml-vulkan-shaders.hpp
	# $(CXX) $(CXXFLAGS_VK) $(LDFLAGS_VK) -c $< -o $@
	
# $(TMP)vk_ggml-vulkan.o: base/ggml-vulkan.cpp base/ggml-vulkan.h base/ggml-vulkan-shaders.hpp base/ggml-vulkan-shaders.cpp
	# $(CXX) $(CXXFLAGS_VK) $(LDFLAGS_VK) -c $< -o $@

  
#####################################
################################ GGUF
PREFIX = t

OBJS_GGUF = \
    $(TMP)$(PREFIX)_ggml.o \
    $(TMP)$(PREFIX)_ggml-alloc.o \
    $(TMP)$(PREFIX)_ggml-backend.o \
    $(TMP)$(PREFIX)_ggml-quants.o \
    $(TMP)$(PREFIX)_ggml-aarch64.o \
    $(TMP)$(PREFIX)_llama.o \
    $(TMP)$(PREFIX)_llama-vocab.o \
    $(TMP)$(PREFIX)_llama-grammar.o \
    $(TMP)$(PREFIX)_llama-sampling.o \
    $(TMP)$(PREFIX)_llama-addon.o \
    $(TMP)$(PREFIX)_sampling.o \
    $(TMP)$(PREFIX)_common.o \
    $(TMP)$(PREFIX)_grammar-parser.o \
    $(TMP)$(PREFIX)_unicode.o \
    $(TMP)$(PREFIX)_unicode-data.o \
    $(TMP)$(PREFIX)_sgemm.o
    
ifdef OPENBLAS64
	CXXFLAGS += -DGGML_USE_BLAS $(shell pkg-config --cflags-only-I openblas64)
	CXXFLAGS_UI += -DGGML_USE_BLAS $(shell pkg-config --cflags-only-I openblas64)
	CFLAGS   += $(shell pkg-config --cflags-only-other openblas64)
	LDFLAGS  += $(shell pkg-config --libs openblas64) --static
	OBJS_GGUF    += $(TMP)$(PREFIX)_ggml-blas.o
	override EXE = Llama_Chat_gguf$(ARCH_NAME)$(RENDERER)_OPENBLAS
	override PREFIX = obt
endif # GGML_OPENBLAS

ifdef OPENBLAS
	CXXFLAGS += -DGGML_USE_BLAS $(shell pkg-config --cflags-only-I openblas)
	CXXFLAGS_UI += -DGGML_USE_BLAS $(shell pkg-config --cflags-only-I openblas)
	CFLAGS   += $(shell pkg-config --cflags-only-other openblas)
	LDFLAGS  += $(shell pkg-config --libs openblas) --static
	OBJS_GGUF    += $(TMP)$(PREFIX)_ggml-blas.o
	override EXE = Llama_Chat_gguf$(ARCH_NAME)$(RENDERER)_OPENBLAS
	override PREFIX = obt
endif # GGML_OPENBLAS
    
$(TMP)tinyfiledialogs/tinyfiledialogs.o: tinyfiledialogs/tinyfiledialogs.c tinyfiledialogs/tinyfiledialogs.h
	$(CC)  $(CFLAGS)   -c $< -o $@

$(TMP)$(PREFIX)_ggml.o: $(ggmlsrc_f)/ggml.c $(ggmlsrc_f)/ggml.h
	$(CC)  $(CFLAGS)   -c $< -o $@
	
$(TMP)$(PREFIX)_ggml-alloc.o: $(ggmlsrc_f)/ggml-alloc.c $(ggmlsrc_f)/ggml.h $(ggmlsrc_f)/ggml-alloc.h
	$(CC)  $(CFLAGS)   -c $< -o $@
	
$(TMP)$(PREFIX)_ggml-backend.o: $(ggmlsrc_f)/ggml-backend.c $(ggmlsrc_f)/ggml.h $(ggmlsrc_f)/ggml-backend.h
	$(CC)  $(CFLAGS)   -c $< -o $@

$(TMP)$(PREFIX)_ggml-quants.o: \
	$(ggmlsrc_f)/ggml-quants.c \
	$(ggmlsrc_f)/ggml.h \
	$(ggmlsrc_f)/ggml-quants.h \
	$(ggmlsrc_f)/ggml-common.h
	$(CC) $(CFLAGS)    -c $< -o $@

$(TMP)$(PREFIX)_ggml-aarch64.o: \
	$(ggmlsrc_f)/ggml-aarch64.c \
	$(ggmlsrc_f)/ggml.h \
	$(ggmlsrc_f)/ggml-aarch64.h \
	$(ggmlsrc_f)/ggml-common.h
	$(CC) $(CFLAGS)    -c $< -o $@

$(TMP)$(PREFIX)_ggml-blas.o: \
	$(ggmlsrc_f)/ggml-blas.cpp \
	$(ggmlsrc_f)/ggml-blas.h
	$(CXX) $(CXXFLAGS)    -c $< -o $@

$(TMP)$(PREFIX)_sgemm.o: $(llamacpp_f)/sgemm.cpp $(llamacpp_f)/sgemm.h $(ggmlsrc_f)/ggml.h
	$(CXX) $(CXXFLAGS) -c $< -o $@
	
$(TMP)$(PREFIX)_unicode.o: $(llamacpp_f)/unicode.cpp $(llamacpp_f)/unicode.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TMP)$(PREFIX)_unicode-data.o: $(llamacpp_f)/unicode-data.cpp $(llamacpp_f)/unicode-data.h
	$(CXX) $(CXXFLAGS) -c $< -o $@
	
$(TMP)$(PREFIX)_llama.o: $(llamacpp_f)/llama.cpp \
	$(llamacpp_f)/llama-impl.h \
	$(llamacpp_f)/llama-vocab.h \
	$(llamacpp_f)/llama-grammar.h \
	$(llamacpp_f)/llama-sampling.h \
	$(llamacpp_f)/unicode.h \
	$(llamacpp_f)/llama.h \
	$(ggmlsrc_f)/ggml.h \
	$(ggmlsrc_f)/ggml-alloc.h \
	$(ggmlsrc_f)/ggml-backend.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TMP)$(PREFIX)_llama-vocab.o: \
	$(llamacpp_f)/llama-vocab.cpp \
	$(llamacpp_f)/llama-vocab.h \
	$(llamacpp_f)/llama-impl.h \
	$(llamacpp_f)/llama.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TMP)$(PREFIX)_llama-grammar.o: \
	$(llamacpp_f)/llama-grammar.cpp \
	$(llamacpp_f)/llama-grammar.h \
	$(llamacpp_f)/llama-impl.h \
	$(llamacpp_f)/llama-vocab.h \
	$(llamacpp_f)/llama-sampling.h \
	$(llamacpp_f)/llama.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TMP)$(PREFIX)_llama-sampling.o: \
	$(llamacpp_f)/llama-sampling.cpp \
	$(llamacpp_f)/llama-sampling.h \
	$(llamacpp_f)/llama-impl.h \
	$(llamacpp_f)/llama.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

COMMON_H_DEPS = $(common_f)/common.h $(common_f)/sampling.h $(common_f)/llama-addon.h $(llamacpp_f)/llama.h
COMMON_DEPS   = $(TMP)$(PREFIX)_common.o $(TMP)$(PREFIX)_sampling.o $(TMP)$(PREFIX)_grammar-parser.o

$(TMP)$(PREFIX)_common.o: $(common_f)/common.cpp $(COMMON_H_DEPS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TMP)$(PREFIX)_sampling.o: $(common_f)/sampling.cpp $(COMMON_H_DEPS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TMP)$(PREFIX)_llama-addon.o: $(common_f)/llama-addon.cpp $(COMMON_H_DEPS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TMP)$(PREFIX)_grammar-parser.o: $(common_f)/grammar-parser.cpp $(common_f)/grammar-parser.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

# test CLBLAST
#OBJS_GGUF_TEST_CL 

###########################################################################
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
OBJS_GGUF_CL    = \
    $(TMP)clt_ggml.o \
    $(TMP)clt_ggml-alloc.o \
    $(TMP)clt_ggml-backend.o \
    $(TMP)clt_ggml-aarch64.o \
    $(TMP)clt_ggml-opencl-gguf.o \
    $(TMP)clt_llama.o \
    $(TMP)clt_llama-vocab.o \
    $(TMP)clt_llama-grammar.o \
    $(TMP)clt_llama-sampling.o \
    $(TMP)clt_llama-addon.o \
    $(TMP)clt_sampling.o \
    $(TMP)clt_common.o \
    $(TMP)clt_ggml-quants.o \
    $(TMP)clt_grammar-parser.o \
    $(TMP)clt_unicode.o \
    $(TMP)clt_unicode-data.o \
    $(TMP)clt_sgemm.o

$(TMP)clt_ggml-opencl-gguf.o: $(ggmlsrc_f)/ggml-opencl.cpp $(ggmlsrc_f)/ggml-opencl.h
	$(CXX) $(CXXFLAGS_CL) -c $< -o $@
    
$(TMP)clt_ggml.o: $(ggmlsrc_f)/ggml.c $(ggmlsrc_f)/ggml.h
	$(CC)  $(CFLAGS_CL)   -c $< -o $@
	
$(TMP)clt_ggml-alloc.o: $(ggmlsrc_f)/ggml-alloc.c $(ggmlsrc_f)/ggml.h $(ggmlsrc_f)/ggml-alloc.h
	$(CC)  $(CFLAGS_CL)   -c $< -o $@
	
$(TMP)clt_ggml-backend.o: $(ggmlsrc_f)/ggml-backend.c $(ggmlsrc_f)/ggml.h $(ggmlsrc_f)/ggml-backend.h
	$(CC)  $(CFLAGS_CL)   -c $< -o $@

$(TMP)clt_ggml-quants.o: \
	$(ggmlsrc_f)/ggml-quants.c \
	$(ggmlsrc_f)/ggml.h \
	$(ggmlsrc_f)/ggml-quants.h \
	$(ggmlsrc_f)/ggml-common.h
	$(CC) $(CFLAGS_CL)    -c $< -o $@

$(TMP)clt_ggml-aarch64.o: \
	$(ggmlsrc_f)/ggml-aarch64.c \
	$(ggmlsrc_f)/ggml.h \
	$(ggmlsrc_f)/ggml-aarch64.h \
	$(ggmlsrc_f)/ggml-common.h
	$(CC) $(CFLAGS_CL)    -c $< -o $@

$(TMP)clt_sgemm.o: $(llamacpp_f)/sgemm.cpp $(llamacpp_f)/sgemm.h $(ggmlsrc_f)/ggml.h
	$(CXX) $(CXXFLAGS_CL) -c $< -o $@
	
$(TMP)clt_unicode.o: $(llamacpp_f)/unicode.cpp $(llamacpp_f)/unicode.h
	$(CXX) $(CXXFLAGS_CL) -c $< -o $@

$(TMP)clt_unicode-data.o: $(llamacpp_f)/unicode-data.cpp $(llamacpp_f)/unicode-data.h
	$(CXX) $(CXXFLAGS_CL) -c $< -o $@
	
$(TMP)clt_llama.o: $(llamacpp_f)/llama.cpp \
	$(llamacpp_f)/llama-impl.h \
	$(llamacpp_f)/llama-vocab.h \
	$(llamacpp_f)/llama-grammar.h \
	$(llamacpp_f)/llama-sampling.h \
	$(llamacpp_f)/unicode.h \
	$(llamacpp_f)/llama.h \
	$(ggmlsrc_f)/ggml.h \
	$(ggmlsrc_f)/ggml-alloc.h \
	$(ggmlsrc_f)/ggml-backend.h
	$(CXX) $(CXXFLAGS_CL) -c $< -o $@

$(TMP)clt_llama-vocab.o: \
	$(llamacpp_f)/llama-vocab.cpp \
	$(llamacpp_f)/llama-vocab.h \
	$(llamacpp_f)/llama-impl.h \
	$(llamacpp_f)/llama.h
	$(CXX) $(CXXFLAGS_CL) -c $< -o $@

$(TMP)clt_llama-grammar.o: \
	$(llamacpp_f)/llama-grammar.cpp \
	$(llamacpp_f)/llama-grammar.h \
	$(llamacpp_f)/llama-impl.h \
	$(llamacpp_f)/llama-vocab.h \
	$(llamacpp_f)/llama-sampling.h \
	$(llamacpp_f)/llama.h
	$(CXX) $(CXXFLAGS_CL) -c $< -o $@

$(TMP)clt_llama-sampling.o: \
	$(llamacpp_f)/llama-sampling.cpp \
	$(llamacpp_f)/llama-sampling.h \
	$(llamacpp_f)/llama-impl.h \
	$(llamacpp_f)/llama.h
	$(CXX) $(CXXFLAGS_CL) -c $< -o $@

COMMON_H_DEPS = $(common_f)/common.h $(common_f)/sampling.h $(common_f)/llama-addon.h $(llamacpp_f)/llama.h
COMMON_DEPS   = $(TMP)common.o $(TMP)sampling.o $(TMP)grammar-parser.o

$(TMP)clt_common.o: $(common_f)/common.cpp $(COMMON_H_DEPS)
	$(CXX) $(CXXFLAGS_CL) -c $< -o $@

$(TMP)clt_sampling.o: $(common_f)/sampling.cpp $(COMMON_H_DEPS)
	$(CXX) $(CXXFLAGS_CL) -c $< -o $@

$(TMP)clt_llama-addon.o: $(common_f)/llama-addon.cpp $(COMMON_H_DEPS)
	$(CXX) $(CXXFLAGS_CL) -c $< -o $@

$(TMP)clt_grammar-parser.o: $(common_f)/grammar-parser.cpp $(common_f)/grammar-parser.h
	$(CXX) $(CXXFLAGS_CL) -c $< -o $@
    
###########################################################################
# Separate VULKAN

OBJS_VK    = \
    $(TMP)vkt_ggml.o \
    $(TMP)vkt_ggml-alloc.o \
    $(TMP)vkt_ggml-backend.o \
    $(TMP)vkt_ggml-aarch64.o \
    $(TMP)vkt_ggml-vulkan.o \
    $(TMP)vkt_ggml-vulkan-shaders.o \
    $(TMP)vkt_llama.o \
    $(TMP)vkt_llama-vocab.o \
    $(TMP)vkt_llama-grammar.o \
    $(TMP)vkt_llama-sampling.o \
    $(TMP)vkt_llama-addon.o \
    $(TMP)vkt_sampling.o \
    $(TMP)vkt_common.o \
    $(TMP)vkt_ggml-quants.o \
    $(TMP)vkt_grammar-parser.o \
    $(TMP)vkt_unicode.o \
    $(TMP)vkt_unicode-data.o \
    $(TMP)vkt_sgemm.o
    
GLSLC_CMD  = glslc
_ggml_vk_genshaders_cmd = $(shell pwd)/vkt-shaders-gen
_ggml_vk_header = $(ggmlsrc_f)/ggml-vulkan-shaders.hpp
_ggml_vk_source = $(ggmlsrc_f)/ggml-vulkan-shaders.cpp
_ggml_vk_input_dir = $(ggmlsrc_f)/vulkan-shaders
_ggml_vk_shader_deps = $(echo $(_ggml_vk_input_dir)/*.comp)

$(TMP)vkt_ggml-vulkan.o: $(ggmlsrc_f)/ggml-vulkan.cpp $(ggmlsrc_f)/ggml-vulkan.h $(_ggml_vk_header) $(_ggml_vk_source)
	$(CXX) $(CXXFLAGS_VK) $(LDFLAGS_VK) -c $< -o $@

$(_ggml_vk_header): $(_ggml_vk_source)

$(_ggml_vk_source): $(_ggml_vk_shader_deps) vkt-shaders-gen
	$(_ggml_vk_genshaders_cmd) \
		--glslc      $(GLSLC_CMD) \
		--input-dir  $(_ggml_vk_input_dir) \
		--target-hpp $(_ggml_vk_header) \
		--target-cpp $(_ggml_vk_source)
    
vkt-shaders-gen: $(ggmlsrc_f)/vulkan-shaders/vulkan-shaders-gen.cpp
	$(CXX) $(CXXFLAGS_VK) -o $@ $(LDFLAGS_VK) $(ggmlsrc_f)/vulkan-shaders/vulkan-shaders-gen.cpp
	
$(TMP)vkt_ggml-vulkan-shaders.o: $(ggmlsrc_f)/ggml-vulkan-shaders.cpp $(ggmlsrc_f)/ggml-vulkan-shaders.hpp
	$(CXX) $(CXXFLAGS_VK) $(LDFLAGS_VK) -c $< -o $@
    
$(TMP)vkt_ggml.o: $(ggmlsrc_f)/ggml.c $(ggmlsrc_f)/ggml.h
	$(CC)  $(CFLAGS_VK)   -c $< -o $@
	
$(TMP)vkt_ggml-alloc.o: $(ggmlsrc_f)/ggml-alloc.c $(ggmlsrc_f)/ggml.h $(ggmlsrc_f)/ggml-alloc.h
	$(CC)  $(CFLAGS_VK)   -c $< -o $@
	
$(TMP)vkt_ggml-backend.o: $(ggmlsrc_f)/ggml-backend.c $(ggmlsrc_f)/ggml.h $(ggmlsrc_f)/ggml-backend.h
	$(CC)  $(CFLAGS_VK)   -c $< -o $@

$(TMP)vkt_ggml-quants.o: \
	$(ggmlsrc_f)/ggml-quants.c \
	$(ggmlsrc_f)/ggml.h \
	$(ggmlsrc_f)/ggml-quants.h \
	$(ggmlsrc_f)/ggml-common.h
	$(CC) $(CFLAGS_VK)    -c $< -o $@

$(TMP)vkt_ggml-aarch64.o: \
	$(ggmlsrc_f)/ggml-aarch64.c \
	$(ggmlsrc_f)/ggml.h \
	$(ggmlsrc_f)/ggml-aarch64.h \
	$(ggmlsrc_f)/ggml-common.h
	$(CC) $(CFLAGS_VK)    -c $< -o $@

$(TMP)vkt_sgemm.o: $(llamacpp_f)/sgemm.cpp $(llamacpp_f)/sgemm.h $(ggmlsrc_f)/ggml.h
	$(CXX) $(CXXFLAGS_VK) -c $< -o $@
	
$(TMP)vkt_unicode.o: $(llamacpp_f)/unicode.cpp $(llamacpp_f)/unicode.h
	$(CXX) $(CXXFLAGS_VK) -c $< -o $@

$(TMP)vkt_unicode-data.o: $(llamacpp_f)/unicode-data.cpp $(llamacpp_f)/unicode-data.h
	$(CXX) $(CXXFLAGS_VK) -c $< -o $@
	
$(TMP)vkt_llama.o: $(llamacpp_f)/llama.cpp \
	$(llamacpp_f)/llama-impl.h \
	$(llamacpp_f)/llama-vocab.h \
	$(llamacpp_f)/llama-grammar.h \
	$(llamacpp_f)/llama-sampling.h \
	$(llamacpp_f)/unicode.h \
	$(llamacpp_f)/llama.h \
	$(ggmlsrc_f)/ggml.h \
	$(ggmlsrc_f)/ggml-alloc.h \
	$(ggmlsrc_f)/ggml-backend.h
	$(CXX) $(CXXFLAGS_VK) -c $< -o $@

$(TMP)vkt_llama-vocab.o: \
	$(llamacpp_f)/llama-vocab.cpp \
	$(llamacpp_f)/llama-vocab.h \
	$(llamacpp_f)/llama-impl.h \
	$(llamacpp_f)/llama.h
	$(CXX) $(CXXFLAGS_VK) -c $< -o $@

$(TMP)vkt_llama-grammar.o: \
	$(llamacpp_f)/llama-grammar.cpp \
	$(llamacpp_f)/llama-grammar.h \
	$(llamacpp_f)/llama-impl.h \
	$(llamacpp_f)/llama-vocab.h \
	$(llamacpp_f)/llama-sampling.h \
	$(llamacpp_f)/llama.h
	$(CXX) $(CXXFLAGS_VK) -c $< -o $@

$(TMP)vkt_llama-sampling.o: \
	$(llamacpp_f)/llama-sampling.cpp \
	$(llamacpp_f)/llama-sampling.h \
	$(llamacpp_f)/llama-impl.h \
	$(llamacpp_f)/llama.h
	$(CXX) $(CXXFLAGS_VK) -c $< -o $@

COMMON_H_DEPS = $(common_f)/common.h $(common_f)/sampling.h $(common_f)/llama-addon.h $(llamacpp_f)/llama.h
COMMON_DEPS   = $(TMP)common.o $(TMP)sampling.o $(TMP)grammar-parser.o

$(TMP)vkt_common.o: $(common_f)/common.cpp $(COMMON_H_DEPS)
	$(CXX) $(CXXFLAGS_VK) -c $< -o $@

$(TMP)vkt_sampling.o: $(common_f)/sampling.cpp $(COMMON_H_DEPS)
	$(CXX) $(CXXFLAGS_VK) -c $< -o $@

$(TMP)vkt_llama-addon.o: $(common_f)/llama-addon.cpp $(COMMON_H_DEPS)
	$(CXX) $(CXXFLAGS_VK) -c $< -o $@

$(TMP)vkt_grammar-parser.o: $(common_f)/grammar-parser.cpp $(common_f)/grammar-parser.h
	$(CXX) $(CXXFLAGS_VK) -c $< -o $@
    
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
    
chats: $(EXE)_mini $(EXE_CL)_mini $(EXE_VK)_mini
	@echo Build complete for $(ECHO_MESSAGE)
    
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
    
demos_gguf: $(EXE) $(EXE_CL)
	@echo Build $(EXE) $(EXE_CL) complete for $(ECHO_MESSAGE)
    
demos: $(EXE)_mini $(EXE_CL)_mini $(EXE_VK)_mini
	@echo Build complete for $(ECHO_MESSAGE)
    
tests: chatTest chatTest_cl chatTest_vk
	@echo Build chatTest chatTest_cl complete for $(ECHO_MESSAGE)
    
chatTests: chatTest chatTest_cl chatTest_vk
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

