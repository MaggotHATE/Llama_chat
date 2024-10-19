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
BUILD_TARGETS = chat chat_cl chat_vk

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
ARCH = -march=core-avx-i -mtune=core-avx-i -mavx
ARCH_NAME = _AVX
TMP = o/avx/
else
ARCH = -march=native -mtune=native
TMP = o/
endif

#the main folder for groups of sources
base = base_sampling2

ifdef SAMPLING1
base = base
endif

ggmlsrc_f = $(base)/ggml
llamacpp_f = $(base)/llama
uibackend_f = $(base)/UI
include_f = $(base)/include
common_f = $(base)

# IMGUI_DIR = imgui
IMGUI_DIR = imgui_f6836ff
SOURCES = main.cpp

SOURCES += $(IMGUI_DIR)/imgui.cpp $(IMGUI_DIR)/imgui_demo.cpp $(IMGUI_DIR)/imgui_draw.cpp $(IMGUI_DIR)/imgui_tables.cpp $(IMGUI_DIR)/imgui_widgets.cpp
SOURCES += $(IMGUI_DIR)/backends/imgui_impl_sdl2.cpp

#renderer
ifndef SDL2
SOURCES += $(IMGUI_DIR)/backends/imgui_impl_vulkan.cpp
RENDERER = _VK
MAIN = vk2
else 
#SOURCES += $(IMGUI_DIR)/backends/imgui_impl_opengl3.cpp
SOURCES += $(IMGUI_DIR)/backends/imgui_impl_sdlrenderer2.cpp
RENDERER = _SDL2
MAIN = sdl2
endif

MAIN_CPP = $(uibackend_f)/main_$(MAIN).cpp
MAIN_O = $(uibackend_f)/main_$(MAIN).o

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
OBJS = $(subst $(TMP)imgui/main.o,,$(OBJS0))
else
OBJS = $(subst $(TMP)imgui/main.o,,$(OBJS0))
endif
OBJS += $(TMP)tinyfiledialogs/tinyfiledialogs.o

# OBJS_UI_VK = $(subst main_vk2.cpp,VULKAN/main_vk.cpp,$(OBJS))

vpath=$(ggmlsrc_f):$(llamacpp_f):$(common_f)

FILE_D = -Itinyfiledialogs
I_GGUF = -I$(common_f) -I$(ggmlsrc_f) -I$(llamacpp_f) -I$(uibackend_f) -I$(include_f)
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
CXXFLAGS = $(I_GGUF) $(OPT) -std=$(CCPP) $(GNUPDATECXX) -fPIC -DNDEBUG $(ARCH) -DGGML_USE_K_QUANTS -DLOG_DISABLE_LOGS -w -pipe

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

PREFIX_S =
PREFIX_O =
PREFIX_A =

ifndef SGEMM_OFF
	CXXFLAGS += -DGGML_USE_LLAMAFILE
	CXXFLAGS_UI += -DGGML_USE_LLAMAFILE
	CFLAGS += -DGGML_USE_LLAMAFILE
	override PREFIX_S = _SG
endif # SGEMM_OFF

ifndef OPENMP_OFF
	CXXFLAGS += -fopenmp -DGGML_USE_OPENMP
	CXXFLAGS_UI += -fopenmp -DGGML_USE_OPENMP
	CFLAGS += -fopenmp -DGGML_USE_OPENMP
	override PREFIX_O = _OMP
endif # OPENMP

ifndef AMX_OFF
	CXXFLAGS += -DGGML_USE_AMX
	CXXFLAGS_UI += -DGGML_USE_AMX
	CFLAGS += -DGGML_USE_AMX
	override PREFIX_A = _AMX
endif # AMX

PREFIX_BASE = s2$(PREFIX_S)$(PREFIX_A)$(PREFIX_O)

ifdef SAMPLING1
PREFIX_BASE = s1$(PREFIX_S)$(PREFIX_A)$(PREFIX_O)
endif

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

# backends

CXXFLAGS_UI_CL = $(CXXFLAGS_UI) -DGGML_USE_CLBLAST
CXXFLAGS_UI_VK = $(CXXFLAGS_UI) -DGGML_USE_VULKAN

CFLAGS_CL = $(CFLAGS) -DGGML_USE_CLBLAST
CFLAGS_VK = $(CFLAGS) -DGGML_USE_VULKAN

CXXFLAGS_CL = $(CXXFLAGS) -DGGML_USE_CLBLAST
CXXFLAGS_VK = $(CXXFLAGS) -DGGML_USE_VULKAN

# common

$(TMP)tinyfiledialogs/tinyfiledialogs.o: tinyfiledialogs/tinyfiledialogs.c tinyfiledialogs/tinyfiledialogs.h
	$(CC)  $(CFLAGS)   -c $< -o $@

##---------------------------------------------------------------------
## BUILD RULES
##---------------------------------------------------------------------

#####################################
################################ GGUF

PREFIX = t$(PREFIX_BASE)

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
    $(TMP)$(PREFIX)_unicode.o \
    $(TMP)$(PREFIX)_unicode-data.o

# SGEMM and others

ifndef SGEMM_OFF
	OBJS_GGUF += $(TMP)$(PREFIX)_sgemm.o
endif # SGEMM_OFF

ifndef AMX_OFF
	OBJS_GGUF += $(TMP)$(PREFIX)_amx.o $(TMP)$(PREFIX)_mmq.o
endif # AMX

ifdef SAMPLING1
	OBJS_GGUF += $(TMP)$(PREFIX)_grammar-parser.o
endif

# backends

ifdef OPENBLAS64
	override PREFIX = ob64_$(PREFIX_BASE)
	CXXFLAGS += -DGGML_USE_BLAS $(shell pkg-config --cflags-only-I openblas64)
	CXXFLAGS_UI += -DGGML_USE_BLAS $(shell pkg-config --cflags-only-I openblas64)
	CFLAGS   += $(shell pkg-config --cflags-only-other openblas64)
	LDFLAGS  += $(shell pkg-config --libs openblas64) --static
	OBJS_GGUF += $(TMP)$(PREFIX)_ggml-blas.o
else ifdef OPENBLAS
	override PREFIX = ob_$(PREFIX_BASE)
	CXXFLAGS += -DGGML_USE_BLAS $(shell pkg-config --cflags-only-I openblas)
	CXXFLAGS_UI += -DGGML_USE_BLAS $(shell pkg-config --cflags-only-I openblas)
	CFLAGS   += $(shell pkg-config --cflags-only-other openblas)
	LDFLAGS  += $(shell pkg-config --libs openblas) --static
	OBJS_GGUF += $(TMP)$(PREFIX)_ggml-blas.o
else ifdef CLBLAST
	override PREFIX = cl_$(PREFIX_BASE)
	CXXFLAGS += -DGGML_USE_CLBLAST -lclblast -lOpenCL
	CXXFLAGS_UI += -DGGML_USE_CLBLAST -lclblast -lOpenCL
	CFLAGS   += -DGGML_USE_CLBLAST -lclblast -lOpenCL
	LDFLAGS  += -Llib/OpenCL.lib -Llib/clblast.lib
	OBJS_GGUF += $(TMP)$(PREFIX)_ggml-opencl-gguf.o
else ifdef VULKAN
	override PREFIX = vk_$(PREFIX_BASE)
	CXXFLAGS += -DGGML_USE_VULKAN
	CXXFLAGS_UI += -DGGML_USE_VULKAN
	CFLAGS   += -DGGML_USE_VULKAN
	LDFLAGS  += $(shell pkg-config --libs vulkan)
	OBJS_GGUF += $(TMP)$(PREFIX)_ggml-vulkan.o $(TMP)$(PREFIX)_ggml-vulkan-shaders.o
endif

$(TMP)$(PREFIX)_ggml.o: $(ggmlsrc_f)/ggml.c $(ggmlsrc_f)/ggml.h
	$(CC)  $(CFLAGS)   -c $< -o $@
	
$(TMP)$(PREFIX)_ggml-alloc.o: $(ggmlsrc_f)/ggml-alloc.c $(ggmlsrc_f)/ggml.h $(ggmlsrc_f)/ggml-alloc.h
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

$(TMP)$(PREFIX)_ggml-backend.o: $(ggmlsrc_f)/ggml-backend.cpp \
	$(ggmlsrc_f)/ggml-backend-impl.h \
	$(ggmlsrc_f)/ggml.h \
	$(ggmlsrc_f)/ggml-backend.h
	$(CC)  $(CXXFLAGS)   -c $< -o $@

$(TMP)$(PREFIX)_sgemm.o: $(ggmlsrc_f)/llamafile/sgemm.cpp $(ggmlsrc_f)/llamafile/sgemm.h $(ggmlsrc_f)/ggml.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TMP)$(PREFIX)_amx.o: \
	$(ggmlsrc_f)/ggml-amx.cpp \
	$(ggmlsrc_f)/ggml-amx.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TMP)$(PREFIX)_mmq.o: \
	$(ggmlsrc_f)/ggml-amx/mmq.cpp \
	$(ggmlsrc_f)/ggml-amx/mmq.h \
	$(ggmlsrc_f)/ggml.h
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
COMMON_DEPS   = $(TMP)$(PREFIX)_common.o $(TMP)$(PREFIX)_sampling.o

$(TMP)$(PREFIX)_common.o: $(common_f)/common.cpp $(COMMON_H_DEPS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TMP)$(PREFIX)_sampling.o: $(common_f)/sampling.cpp $(COMMON_H_DEPS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TMP)$(PREFIX)_llama-addon.o: $(common_f)/llama-addon.cpp $(COMMON_H_DEPS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TMP)$(PREFIX)_grammar-parser.o: $(common_f)/grammar-parser.cpp $(common_f)/grammar-parser.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

# openblas
$(TMP)$(PREFIX)_ggml-blas.o: \
	$(ggmlsrc_f)/ggml-blas.cpp \
	$(ggmlsrc_f)/ggml-blas.h
	$(CXX) $(CXXFLAGS)    -c $< -o $@

# clblast
$(TMP)$(PREFIX)_ggml-opencl-gguf.o: $(ggmlsrc_f)/ggml-opencl.cpp $(ggmlsrc_f)/ggml-opencl.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

# vulkan
GLSLC_CMD  = glslc
_ggml_vk_genshaders_cmd = $(shell pwd)/vkt-shaders-gen
_ggml_vk_header = $(ggmlsrc_f)/ggml-vulkan-shaders.hpp
_ggml_vk_source = $(ggmlsrc_f)/ggml-vulkan-shaders.cpp
_ggml_vk_input_dir = $(ggmlsrc_f)/vulkan-shaders
_ggml_vk_shader_deps = $(echo $(_ggml_vk_input_dir)/*.comp)

$(TMP)$(PREFIX)_ggml-vulkan.o: $(ggmlsrc_f)/ggml-vulkan.cpp $(ggmlsrc_f)/ggml-vulkan.h $(_ggml_vk_header) $(_ggml_vk_source)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -c $< -o $@

$(_ggml_vk_header): $(_ggml_vk_source)

hascompiler = 1

ifeq (,$(wildcard vkt-shaders-gen.exe))
    hascompiler = 0
endif

hasfiles = 1

ifeq (,$(wildcard $(_ggml_vk_header)))
    hasfiles = 0
endif

ifeq (,$(wildcard $(_ggml_vk_source)))
    hasfiles = 0
endif

ifeq (0,$(hascompiler))
    vkt-compiler = vkt-shaders-gen
endif

ifeq (0,$(hasfiles))

$(_ggml_vk_source): $(_ggml_vk_shader_deps) $(vkt-compiler)
	$(_ggml_vk_genshaders_cmd) \
		--glslc      $(GLSLC_CMD) \
		--input-dir  $(_ggml_vk_input_dir) \
		--target-hpp $(_ggml_vk_header) \
		--target-cpp $(_ggml_vk_source)

endif

vkt-shaders-gen: $(ggmlsrc_f)/vulkan-shaders/vulkan-shaders-gen.cpp
	$(CXX) $(CXXFLAGS) -o $@ $(LDFLAGS) $(ggmlsrc_f)/vulkan-shaders/vulkan-shaders-gen.cpp

$(TMP)$(PREFIX)_ggml-vulkan-shaders.o: $(_ggml_vk_source) $(_ggml_vk_header)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -c $< -o $@


ifdef VK_DEBUG
	CXXFLAGS += -DGGML_VULKAN_DEBUG
	CFLAGS += -DGGML_VULKAN_DEBUG
endif # VK_DEBUG

ifdef VK_MEMDEBUG
	CXXFLAGS += -DGGML_VULKAN_MEMORY_DEBUG
	CFLAGS += -DGGML_VULKAN_MEMORY_DEBUG
endif # VK_MEMDEBUG

ifdef VK_VALID
	CXXFLAGS += -DGGML_VULKAN_VALIDATE
	CFLAGS += -DGGML_VULKAN_VALIDATE
endif # VK_VALID

ifdef VK_PERF
	CXXFLAGS  += -DGGML_VULKAN_PERF
	CFLAGS  += -DGGML_VULKAN_PERF
endif


###########################################################################
# Old CLBLAST

# Mac provides OpenCL as a framework
ifeq ($(UNAME_S),Darwin)
    LDFLAGS_CL += -lclblast -framework OpenCL
else
    #LDFLAGS2 += -lclblast -lOpenCL
    LDFLAGS_CL += -Llib/OpenCL.lib -Llib/clblast.lib
    #LDFLAGS_CL += -lclblast -lOpenCL
endif
    
###########################################################################

# general
    
$(TMP)imgui/%.o:$(IMGUI_DIR)/%.cpp
	$(CXX) $(CXXFLAGS_UI) -c -o $@ $<

$(TMP)imgui/%.o:$(IMGUI_DIR)/backends/%.cpp
	$(CXX) $(CXXFLAGS_UI) -c -o $@ $<
    
$(TMP)imgui/%.o:$(IMGUI_DIR)/misc/cpp/%.cpp
	$(CXX) $(CXXFLAGS_UI) -c -o $@ $<
    
################
# EXE components
################

chat_layer = $(common_f)/chat_layer.h
settings_layer = $(common_f)/threads_layer.h
conapp = $(common_f)/class_chat.cpp
conapp_o = $(common_f)/class_chat.o
ui_simple = $(uibackend_f)/UI_small.h

dualapp = $(common_f)/dualchat.cpp

ifdef SAMPLING1
chat_layer = $(common_f)/chat_plain.h
settings_layer = $(common_f)/thread_chat.h
conapp = class_chat.cpp
conapp_o = class_chat.o
ui_simple = $(uibackend_f)/UI_simple.h
endif

# Final parts
$(TMP)$(PREFIX)_class_chat.o:$(conapp) $(chat_layer) $(settings_layer)
	$(CXX) $(CXXFLAGS) -c $< -o $@
	@echo $(TMP)$(PREFIX)_class_chat.o compiled

$(TMP)$(PREFIX)_dual_chat.o:$(dualapp) $(chat_layer) $(settings_layer)
	$(CXX) $(CXXFLAGS) -c $< -o $@
	@echo $(TMP)$(PREFIX)_dual_chat.o compiled

# Final parts UI
$(TMP)$(PREFIX)_main_$(MAIN).o:$(MAIN_CPP) $(chat_layer) $(settings_layer) $(ui_simple)
	$(CXX) $(I_GGUF) $(FILE_D) $(CXXFLAGS_UI) -DUI_SIMPLE -c $< -o $@

# Naming
chatTest_cpu = chatTest_$(PREFIX)
chatTest_cl = chatTest_$(PREFIX_CL)
chatTest_vk = chatTest_$(PREFIX_VK)

override EXE = LlamaChat_gguf$(ARCH_NAME)$(RENDERER)_$(PREFIX)
override EXE_CL = LlamaChat_gguf$(ARCH_NAME)$(RENDERER)_$(PREFIX_CL)
override EXE_VK = LlamaChat_gguf$(ARCH_NAME)$(RENDERER)_$(PREFIX_VK)

# CCACHE

# $< is the first prerequisite, i.e. the source file.
# Explicitly compile this to an object file so that it can be cached with ccache.
# The source file is then filtered out from $^ (the list of all prerequisites) and the object file is added instead.

# Helper function that replaces .c, .cpp, and .cu file endings with .o:
GET_OBJ_FILE = $(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(patsubst %.cu,%.o,$(1))))
GET_OBJ_FILE1 = $(subst $(conapp_o),$(TMP)$(PREFIX_VK)_class_chat.o,$(GET_OBJ_FILE))
GET_OBJ_FILE2 = $(subst $(MAIN_O),$(TMP)$(PREFIX_VK)_main_$(MAIN).o,$(GET_OBJ_FILE))

# NAMED BULDS

chat: $(EXE)_mini
	@echo Build $(EXE)_mini complete for $(ECHO_MESSAGE)

chat_ob:
	$(MAKE) chat OPENBLAS64=1
	@echo Using OPENBLAS64

chat_cl:
	$(MAKE) chat CLBLAST=1
	@echo Using CLBLAST

chat_vk:
	$(MAKE) chat VULKAN=1
	@echo Using VULKAN

chat_sdl:
	$(MAKE) chat SDL2=1
	@echo UI on SDL2

chat_ob_sdl:
	$(MAKE) chat_ob SDL2=1
	@echo UI on SDL2
    
chat_cl_sdl:
	$(MAKE) chat_cl SDL2=1
	@echo UI on SDL2
    
chat_vk_sdl:
	$(MAKE) chat_vk SDL2=1
	@echo UI on SDL2

chatTest: $(chatTest_cpu)
	@echo Build chatTest complete for $(ECHO_MESSAGE)

chatTest_cl:
	$(MAKE) chatTest CLBLAST=1
	@echo Using CLBLAST
    
chatTest_ob:
	$(MAKE) chatTest OPENBLAS64=1
	@echo Using OPENBLAS64
    
chatTest_vk:
	$(MAKE) chatTest VULKAN=1
	@echo Using VULKAN

# aggregates

chats: chat chat_ob chat_cl chat_vk
    
chats_sdl: chat_sdl chat_ob_sdl chat_cl_sdl chat_vk_sdl
    
tests: chatTest chatTest_ob chatTest_cl chatTest_vk
    
all: chats chats_sdl tests
    
all_ob: chat_ob chat_ob_sdl chatTest_ob
    
all_cl: chat_cl chat_cl_sdl chatTest_cl
    
all_vk: chat_vk chat_vk_sdl chatTest_vk
    
all_cpu: chat chat_sdl chatTest


#    
# MAIN EXE's
# universal recipes
    
$(EXE): $(OBJS) $(OBJS_GGUF) $(chat_layer) $(settings_layer) UI.h llama_chat1.res
	$(CXX) $(I_GGUF) $(FILE_D) -o $@ $^ $(CXXFLAGS_UI) $(CONFLAG) $(LDFLAGS) $(LIBS)
     
$(EXE)_mini:$(TMP)$(PREFIX)_main_$(MAIN).o llama_chat1.res $(OBJS) $(OBJS_GGUF)
	$(CXX) $(I_GGUF) $(FILE_D) -o $@ $^ $(CXXFLAGS_UI) -DUI_SIMPLE $(CONFLAG) $(LDFLAGS) $(LIBS)
    
$(chatTest_cpu):$(TMP)$(PREFIX)_class_chat.o $(OBJS_GGUF)
	@echo ARCH = $(ARCH)
	$(CXX) $(I_GGUF) $(filter-out %.h,$^) $(LDFLAGS) -o $@ $(CXXFLAGS)
    
dualTest:$(TMP)$(PREFIX)_dual_chat.o $(OBJS_GGUF)
	$(CXX) $(I_GGUF) $(CXXFLAGS) $(filter-out %.h,$^) $(LDFLAGS) -o $@

# VULKAN OLD

#chatTest_vk: $(chatTest_vk)

$(EXE_VK): $(OBJS) $(OBJS_VK) $(chat_layer) $(settings_layer) UI.h llama_chat1.res
	 $(CXX) $(I_GGUF) $(FILE_D) $(CXXFLAGS_UI_VK) -o $@ $^ $(CONFLAG) $(LIBS) $(LDFLAGS_VK+)
     
$(EXE_VK)_mini:$(MAIN_CPP) llama_chat1.res $(OBJS) $(OBJS_VK)
	# $(CXX) $(I_GGUF) $(FILE_D) $(CXXFLAGS_UI_VK) -DUI_SIMPLE -o $@ $^ $(CONFLAG) $(LIBS) $(LDFLAGS_VK+)
	$(CXX) $(I_GGUF) $(FILE_D) $(CXXFLAGS_UI_VK) -DUI_SIMPLE -c $< -o $(call GET_OBJ_FILE2, $<)
	$(CXX) $(I_GGUF) $(FILE_D) $(CXXFLAGS_UI_VK) -DUI_SIMPLE $(filter-out %.h $<,$^) $(call GET_OBJ_FILE2, $<) -o $@ $(CONFLAG) $(LIBS) $(LDFLAGS_VK+)
     
$(chatTest_vk):$(conapp) $(OBJS_VK)
	#$(CXX)  $(I_GGUF) $(CXXFLAGS_VK) $(filter-out %.h,$^) $(LDFLAGS_VK) $(LDFLAGS_VK+) -o $@
	$(CXX)  $(I_GGUF) $(CXXFLAGS_VK) -c $< -o $(call GET_OBJ_FILE1, $<)
	$(CXX)  $(I_GGUF) $(CXXFLAGS_VK) $(filter-out %.h $<,$^) $(call GET_OBJ_FILE1, $<) -o $@ $(LDFLAGS_VK) $(LDFLAGS_VK+)
#-

