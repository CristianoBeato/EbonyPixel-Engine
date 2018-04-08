cmake_minimum_required(VERSION 2.8)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

project(Beato-Tech1)

option(USE_TOOLS
		"enable  built-in toos and compilers" ON)
if(USE_TOOLS)
	option(USE_MFC_TOOLS
			"Compile the built-in MFC based tools" OFF)
endif(USE_TOOLS)
		
option(MONOLITH
		"Embed game logic into main executable" ON)

option(USE_PRECOMPILED_HEADERS
		"Use precompiled headers during build" ON)
	
if(WIN32)
	option(XAUDIO2 "Use Xaudio2 soft instead of SDL audio or SDL (win)" ON)
endif(WIN32)

option(OPENAL "Use OpenAL soft instead of SDL audio or XAUDIO (win)" OFF)
if(OPENAL)
	option(USE_SYSTEM_OPENAL "Use the system OpenAL instead of the bundled one" OFF)
	set(XAUDIO2 FALSE)
endif(OPENAL)


option(ONATIVE
		"Optimize for the host CPU" OFF)
		
option(WINRT
		"Build for Windows RT" OFF)
		
option(WINDOWS10
		"Build for Windows 10" OFF)
		
option(USE_SYSTEM_ZLIB
                "Use the system zlib instead of the bundled one" OFF)

option(USE_SYSTEM_LIBPNG
                "Use the system libpng instead of the bundled one" OFF)

option(USE_SYSTEM_LIBJPEG
                "Use the system libjpeg instead of the bundled one" OFF)

option(USE_SYSTEM_LIBGLEW
                "Use the system libglew instead of the bundled one" OFF)

set(CPU_TYPE "" CACHE STRING "When set, passes this string as CPU-ID which will be embedded into the binary.")

set(CPU_OPTIMIZATION "-mmmx -msse -msse2 -avx" CACHE STRING "Which CPU specific optimitations should be used beside the compiler's default?")

option(USE_INTRINSICS "Compile using intrinsics (e.g mmx, sse, msse2)" ON)

if(UNIX)
	set(OPENAL TRUE)
endif()
		
if(MSVC)
	#message(STATUS CMAKE_ROOT: ${CMAKE_ROOT})
	
	#if(CMAKE_CL_64)
	#	SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/../bin/win64)
	#else()
	#	SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/../bin/win32)
	#endif()
	
	#message(STATUS EXECUTABLE_OUTPUT_PATH: ${EXECUTABLE_OUTPUT_PATH})
	#message(STATUS PROJECT_BINARY_DIR: ${PROJECT_BINARY_DIR})
	#message(STATUS CMAKE_BINARY_DIR: ${CMAKE_BINARY_DIR})
else()
	message(STATUS CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE})
endif()

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_C_COMPILER_ID STREQUAL "Clang")
	add_definitions(-pipe)
	#add_definitions(-Wall)
	add_definitions(-Werror=format-security)
	add_definitions(-Werror=format)
	if(CPU_TYPE)
		add_definitions(-DCPUSTRING="${CPU_TYPE}")
	endif()
	if (CPU_OPTIMIZATION)
		add_definitions(${CPU_OPTIMIZATION})
	endif()
	if (USE_INTRINSICS)
		add_definitions(-DUSE_INTRINSICS)
	endif()
	if(WIN32)
		# require msvcr70.dll or newer for _aligned_malloc etc
		# I think it is from Visual C++ .NET 2002, so it should be available on any remotely modern system.
		add_definitions(-D__MSVCRT_VERSION__=0x0700)
		include_directories(libs/mingw-hacks)
	endif()	
	
	#if(NOT ANDROID)
		add_definitions(-DUSE_EXCEPTIONS)
	#endif()
	
	# the warnings are used for every profile anyway, so put them in a variable
	set(my_warn_flags "-Wno-pragmas -Wno-unused-variable -Wno-switch -Wno-unused-value -Winvalid-pch -Wno-multichar")
	
	if(CMAKE_C_COMPILER_ID STREQUAL "Clang")
		# append clang-specific settings for warnings (the second one make sure clang doesn't complain
		# about unknown -W flags, like -Wno-unused-but-set-variable)
		set(my_warn_flags "${my_warn_flags} -Wno-local-type-template-args -Wno-unknown-warning-option -Wno-inline-new-delete -Wno-switch-enum")
	endif()
	
	if(NOT CMAKE_CROSSCOMPILING AND ONATIVE)
		add_definitions(-march=native)
	endif()

	set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -D_DEBUG -O0 -ggdb -fno-strict-aliasing ${my_warn_flags}")
	#set(CMAKE_C_FLAGS_DEBUGALL "${CMAKE_C_FLAGS_DEBUGALL} -g -ggdb -D_DEBUG -fno-strict-aliasing ${my_warn_flags}")
	#set(CMAKE_C_FLAGS_PROFILE "${CMAKE_C_FLAGS_PROFILE} -g -ggdb -D_DEBUG -O1 -fno-omit-frame-pointer -fno-strict-aliasing ${my_warn_flags}")
	set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3 -ffast-math -fno-unsafe-math-optimizations -fomit-frame-pointer -fno-strict-aliasing ${my_warn_flags}")
	set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} -g -O3 -ffast-math -fno-unsafe-math-optimizations -fomit-frame-pointer -fno-strict-aliasing ${my_warn_flags}")
	set(CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL} -Os -ffast-math -fno-unsafe-math-optimizations -fomit-frame-pointer -fno-strict-aliasing ${my_warn_flags}")
	
	set(CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
	#set(CMAKE_CXX_FLAGS_DEBUGALL ${CMAKE_C_FLAGS_DEBUGALL})
	#set(CMAKE_CXX_FLAGS_PROFILE ${CMAKE_C_FLAGS_PROFILE})
	set(CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
	set(CMAKE_CXX_FLAGS_RELWITHDEBINFO ${CMAKE_C_FLAGS_RELWITHDEBINFO})
	set(CMAKE_CXX_FLAGS_MINSIZEREL ${CMAKE_C_FLAGS_MINSIZEREL})

elseif(MSVC)
	add_compile_options(/W4)
	add_compile_options(/wd4244) # possible loss of data
	add_compile_options(/wd4245) # signed/unsigned mismatch
	add_compile_options(/wd4267) # possible loss of data
	add_compile_options(/wd4714) # 'function' marked as __forceinline not inlined
	add_compile_options(/wd4595) # 'function' non-member operator new or delete functions may not be declared inline
	add_compile_options(/wd4458) # declaration of 'z' hides class member
	add_compile_options(/wd4467) # usage of ATL attributes is deprecated
	# Multiprocessor support for compiling
	# /MP
	
	# /MT  use multithreaded, static runtime library
	#      (=> binary won't need MSVCP100D.dll or similar)
	# /MTd the same with debug symbols
	# /MD and /MDd - the same with dynamic runtime lib
	#     (=> MSVCP100D.dll or similar is needed to run binary)

	# Inline Function Expansion: Any Suitable (/Ob2)
	# "/Ob2",
			
	# enable Intrinsic Functions
	# "/Oi",
			
	# Omit Frame Pointers
	# "/Oy",
	
	# if(MSVC_VERSION EQUAL 1700)
		# #message(STATUS "$ENV{LIBPATH}")
		
		# find_path(Windows_winmd_DIR NAMES Windows.winmd
			# HINTS
			# C:/Program Files (x86)/Windows Kits/8.0/References/CommonConfiguration/Neutral
			# PATH_SUFFIXES "Neutral")
			
		# message(STATUS "${Windows_winmd_DIR}")
		
		# #set(WINRT_OPTIONS "/ZW /D USE_WINRT /AI \"C:\Program Files (x86)\Windows Kits\8.0\References\CommonConfiguration\Neutral\"")
		# #set(WINRT_OPTIONS "/ZW /D USE_WINRT /AI \"$ENV{LIBPATH}\" ")
		# set(WINRT_OPTIONS "/ZW /D USE_WINRT")
	
		# set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /D _DEBUG /MP ${WINRT_OPTIONS}")
		# set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MP /Oi /Oy ${WINRT_OPTIONS}")
		# set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} /MP /Oi ${WINRT_OPTIONS}")
		# set(CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL} /MP /Oi /Oy ${WINRT_OPTIONS}")
		
		# set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MP ${WINRT_OPTIONS}")
		# set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MP /Oi /Oy ${WINRT_OPTIONS}")
		# set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /MP ${WINRT_OPTIONS}")
		# set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} /MP /Oi /Oy ${WINRT_OPTIONS}")
	# else()
		set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /D _DEBUG /MP /MTd")
		set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MP /Oi /Oy /MT")
		set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} /MP /Oi /MTd")
		set(CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL} /MP /Oi /Oy /MT")
		
		set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MP /MTd")
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MP /Oi /Oy /MT")
		set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /MP /MTd")
		set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} /MP /Oi /Oy /MT")
		
		# RB: without /SAFESEH:NO we can't link against ffmpeg libs and VS2013 or we have to build our own
		# libs for newer compilers
		set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /SAFESEH:NO")
		set(CMAKE_EXE_LINKER_FLAGS_MINSIZEREL "${CMAKE_EXE_LINKER_FLAGS_MINSIZEREL} /SAFESEH:NO")
		set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /SAFESEH:NO")
		set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO} /SAFESEH:NO")
	# endif()
	

	add_definitions(-DWIN32 
					-D_WINDOWS
					-D_CRT_SECURE_NO_DEPRECATE
					-D_CRT_NONSTDC_NO_DEPRECATE
					-D_CRT_SECURE_NO_WARNINGS
                                        -D_MBCS
					-DUSE_EXCEPTIONS)
        ## Check for Version ##
        if( WINRT OR WINDOWS10 ) # Windows RT
			add_definitions(-DUSE_WINRT)
        endif()

        if (USE_INTRINSICS)
			add_definitions(-DUSE_INTRINSICS)
        endif()
					
	if(NOT CMAKE_CL_64)
		add_definitions(-D_USE_32BIT_TIME_T)
	endif()
					
	if(USE_MFC_TOOLS)
		add_definitions(-D_AFXDLL)
		
		# 0: Use Standard Windows Libraries
		# 1: Use MFC in a Static Library
		# 2: Use MFC in a Shared DLL 
		set(CMAKE_MFC_FLAG 2)
	endif()
endif()

#if(STANDALONE)
#	add_definitions(-DSTANDALONE)
#endif()
if( NOT MONOLITH)
	add_definitions(-D__DOOM_DLL__ -DGAME_DLL)
endif()

 ##libs
find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIRS})
 
#set(SDL2_BUILDING_LIBRARY ON)
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIR})

find_package(SDL2_TTF REQUIRED)
include_directories(${SDL2_TTF_INCLUDE_DIRS})
 
if (USE_SYSTEM_ZLIB)
  find_package(ZLIB REQUIRED)
endif(USE_SYSTEM_ZLIB)

if (ZLIB_FOUND)
    set(ZLIB_INCLUDE ${ZLIB_INCLUDE_DIRS})
    set(ZLIB_LIBRARY ${ZLIB_LIBRARIES})
else (ZLIB_FOUND)
   set(ZLIB_INCLUDE "libs/zlib")
    set(ZLIB_LIBRARY "" )
endif (ZLIB_FOUND)

if(USE_SYSTEM_LIBPNG)
find_package(PNG REQUIRED)
endif (USE_SYSTEM_LIBPNG)

if (PNG_FOUND)
	set(PNG_INCLUDE ${PNG_INCLUDE_DIRS})
    set(PNG_LIBRARY ${PNG_LIBRARIES})
else (PNG_FOUND)
	set(PNG_INCLUDE "libs/png")
    set(PNG_LIBRARY "" )
endif (PNG_FOUND)

if(USE_SYSTEM_LIBJPEG)
find_package(JPEG REQUIRED)
endif(USE_SYSTEM_LIBJPEG)

if (JPEG_FOUND)
    set(JPEG_INCLUDE ${JPEG_INCLUDE_DIRS})
    set(JPEG_LIBRARY ${JPEG_LIBRARIES})
    add_definitions(-DUSE_NEWER_JPEG)
else (JPEG_FOUND)
	set(JPEG_INCLUDE "libs/jpeg-6")
    set(JPEG_LIBRARY "" )
endif (JPEG_FOUND)

if (USE_SYSTEM_LIBGLEW)
find_package(GLEW)
endif(USE_SYSTEM_LIBGLEW)

if (GLEW_FOUND)
	set(GLEW_INCLUDE ${GLEW_INCLUDE_DIRS})
    set(GLEW_LIBRARY ${GLEW_LIBRARIES})
else (GLEW_FOUND)
	set(GLEW_INCLUDE "libs/glew/include")
    set(GLEW_LIBRARY "" )
    add_definitions(-DGLEW_STATIC)
endif (GLEW_FOUND)

if(OPENAL)
	if (USE_SYSTEM_OPENAL)
		find_package(OPENAL REQUIRED)
	endif(USE_SYSTEM_OPENAL)

	if (OPENAL_FOUND)
		set(AUDIO_LIB_INCLUDE_DIRS ${OPENAL_INCLUDE_DIRS})
		set(AUDIO_LIB_LIBRARY ${OPENAL_LIBRARIES})
	else (OPENAL_FOUND)
		set(AUDIO_LIB_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/libs/openal-soft/include)
		if(CMAKE_CL_64)
			set(AUDIO_LIB_LIBRARY ${CMAKE_CURRENT_SOURCE_DIR}/libs/openal-soft/lib/win64/OpenAL32.lib)
		else()
			set(AUDIO_LIB_LIBRARY ${CMAKE_CURRENT_SOURCE_DIR}/libs/openal-soft/lib/win32/OpenAL32.lib)
		endif()
	endif (OPENAL_FOUND)
elseif(XAUDIO2)
	find_package(DirectX REQUIRED)
	set(AUDIO_LIB_INCLUDE_DIRS ${DirectX_INCLUDE_DIR})
	set(AUDIO_LIB_LIBRARY xaudio2.lib dsound.lib)
else()
	set(AUDIO_LIB_INCLUDE_DIRS "")
	set(AUDIO_LIB_LIBRARY "")
endif(OPENAL)
	
#SET THE BASE SOURCE DIR
include_directories(
	${CMAKE_SOURCE_DIR}
	${CMAKE_SOURCE_DIR}/idlib
	${ZLIB_INCLUDE}
	${GLEW_INCLUDE}
	${JPEG_INCLUDE}
	${PNG_INCLUDE}
	)
#the idlib and framework libs
add_subdirectory(framework)	
add_subdirectory(idlib)
add_subdirectory(game)
add_subdirectory(sound)
add_subdirectory(renderer)

if(USE_TOOLS)
	add_definitions(-D_ID_ALLOW_TOOLS_)
	add_subdirectory(tools)
endif(USE_TOOLS)

file(GLOB IRRXML_INCLUDES libs/irrxml/src/*.h)
file(GLOB IRRXML_SOURCES libs/irrxml/src/*.cpp)

if (NOT JPEG_FOUND)
    set(JPEG_INCLUDES
        libs/jpeg-6/jchuff.h
        libs/jpeg-6/jconfig.h
        libs/jpeg-6/jdct.h
        libs/jpeg-6/jdhuff.h
        libs/jpeg-6/jerror.h
        libs/jpeg-6/jinclude.h
        libs/jpeg-6/jmemsys.h
        libs/jpeg-6/jmorecfg.h
        libs/jpeg-6/jpegint.h
        libs/jpeg-6/jpeglib.h
        libs/jpeg-6/jversion.h)

    set(JPEG_SOURCES
        libs/jpeg-6/jcapimin.cpp
        libs/jpeg-6/jcapistd.cpp
        libs/jpeg-6/jccoefct.cpp
        libs/jpeg-6/jccolor.cpp
        libs/jpeg-6/jcdctmgr.cpp
        libs/jpeg-6/jchuff.cpp
        libs/jpeg-6/jcinit.cpp
        libs/jpeg-6/jcmainct.cpp
        libs/jpeg-6/jcmarker.cpp
        libs/jpeg-6/jcmaster.cpp
        libs/jpeg-6/jcomapi.cpp
        libs/jpeg-6/jcparam.cpp
        libs/jpeg-6/jcphuff.cpp
        libs/jpeg-6/jcprepct.cpp
        libs/jpeg-6/jcsample.cpp
        libs/jpeg-6/jctrans.cpp
        libs/jpeg-6/jdapimin.cpp
        libs/jpeg-6/jdapistd.cpp
        libs/jpeg-6/jdatadst.cpp
        libs/jpeg-6/jdatasrc.cpp
        libs/jpeg-6/jdcoefct.cpp
        libs/jpeg-6/jdcolor.cpp
        libs/jpeg-6/jddctmgr.cpp
        libs/jpeg-6/jdhuff.cpp
        libs/jpeg-6/jdinput.cpp
        libs/jpeg-6/jdmainct.cpp
        libs/jpeg-6/jdmarker.cpp
        libs/jpeg-6/jdmaster.cpp
        libs/jpeg-6/jdmerge.cpp
        libs/jpeg-6/jdphuff.cpp
        libs/jpeg-6/jdpostct.cpp
        libs/jpeg-6/jdsample.cpp
        libs/jpeg-6/jdtrans.cpp
        libs/jpeg-6/jerror.cpp
        libs/jpeg-6/jfdctflt.cpp
        libs/jpeg-6/jfdctfst.cpp
        libs/jpeg-6/jfdctint.cpp
        libs/jpeg-6/jidctflt.cpp
        libs/jpeg-6/jidctfst.cpp
        libs/jpeg-6/jidctint.cpp
        libs/jpeg-6/jidctred.cpp
        #libs/jpeg-6/jload.cpp
        libs/jpeg-6/jmemmgr.cpp
        libs/jpeg-6/jmemnobs.cpp
        #libs/jpeg-6/jpegtran.cpp
        libs/jpeg-6/jquant1.cpp
        libs/jpeg-6/jquant2.cpp
        libs/jpeg-6/jutils.cpp)
else (NOT JPEG_FOUND)
   set(JPEG_INCLUDES "")
   set(JPEG_SOURCES "")
endif (NOT JPEG_FOUND)

if (NOT PNG_FOUND)
    file(GLOB PNG_INCLUDES libs/png/*.h)
    file(GLOB PNG_SOURCES libs/png/*.c)
else (NOT PNG_FOUND)
    set(PNG_INCLUDES "")
    set(PNG_SOURCES "")
endif (NOT PNG_FOUND)

if (NOT ZLIB_FOUND)
file(GLOB ZLIB_INCLUDES libs/zlib/*.h)
file(GLOB ZLIB_SOURCES libs/zlib/*.c)
else (NOT ZLIB_FOUND)
  set(ZLIB_INCLUDES "")
  set(ZLIB_SOURCES "")
endif (NOT ZLIB_FOUND)

file(GLOB MINIZIP_INCLUDES libs/zlib/minizip/*.h)
file(GLOB MINIZIP_SOURCES libs/zlib/minizip/*.c libs/zlib/minizip/*.cpp)
	
if (NOT GLEW_FOUND)
    set(GLEW_INCLUDES libs/glew/include/GL/glew.h)
    set(GLEW_SOURCES libs/glew/src/glew.c)

    if(WIN32)
        set(GLEW_INCLUDES ${GLEW_INCLUDES} libs/glew/include/GL/wglew.h)
    else(WIN32)
        set(GLEW_INCLUDES ${GLEW_INCLUDES} libs/glew/include/GL/glxew.h)
    endif(WIN32)
else (NOT GLEW_FOUND)
    set(GLEW_INCLUDES "")
    set(GLEW_SOURCES "")
endif (NOT GLEW_FOUND)
	
set(FREETYPE_SOURCES
	libs/freetype/src/autofit/autofit.c
	libs/freetype/src/bdf/bdf.c
	libs/freetype/src/cff/cff.c
	libs/freetype/src/base/ftbase.c
	libs/freetype/src/base/ftbitmap.c
	libs/freetype/src/cache/ftcache.c
	libs/freetype/src/base/ftdebug.c
	libs/freetype/src/base/ftgasp.c
	libs/freetype/src/base/ftglyph.c
	libs/freetype/src/gzip/ftgzip.c
	libs/freetype/src/base/ftinit.c
	libs/freetype/src/lzw/ftlzw.c
	libs/freetype/src/base/ftstroke.c
	libs/freetype/src/base/ftsystem.c
	libs/freetype/src/smooth/smooth.c
	libs/freetype/src/base/ftbbox.c
	libs/freetype/src/base/ftmm.c
	libs/freetype/src/base/ftpfr.c
	libs/freetype/src/base/ftsynth.c
	libs/freetype/src/base/fttype1.c
	libs/freetype/src/base/ftwinfnt.c
	libs/freetype/src/pcf/pcf.c
	libs/freetype/src/pfr/pfr.c
	libs/freetype/src/psaux/psaux.c
	libs/freetype/src/pshinter/pshinter.c
	libs/freetype/src/psnames/psmodule.c
	libs/freetype/src/raster/raster.c
	libs/freetype/src/sfnt/sfnt.c
	libs/freetype/src/truetype/truetype.c
	libs/freetype/src/type1/type1.c
	libs/freetype/src/cid/type1cid.c
	libs/freetype/src/type42/type42.c
	libs/freetype/src/winfonts/winfnt.c)
		
set(OGGVORBIS_INCLUDES
	libs/oggvorbis/ogg/ogg.h
	libs/oggvorbis/ogg/os_types.h)
	
set(OGGVORBIS_SOURCES
	libs/oggvorbis/oggsrc/bitwise.c
	libs/oggvorbis/oggsrc/framing.c
	
	libs/oggvorbis/vorbissrc/mdct.c
	libs/oggvorbis/vorbissrc/smallft.c
	libs/oggvorbis/vorbissrc/block.c
	libs/oggvorbis/vorbissrc/envelope.c
	libs/oggvorbis/vorbissrc/windowvb.c
	libs/oggvorbis/vorbissrc/lsp.c
	libs/oggvorbis/vorbissrc/lpc.c
	libs/oggvorbis/vorbissrc/analysis.c
	libs/oggvorbis/vorbissrc/synthesis.c
	libs/oggvorbis/vorbissrc/psy.c
	libs/oggvorbis/vorbissrc/info.c
	libs/oggvorbis/vorbissrc/floor1.c
	libs/oggvorbis/vorbissrc/floor0.c
	libs/oggvorbis/vorbissrc/res0.c
	libs/oggvorbis/vorbissrc/mapping0.c
	libs/oggvorbis/vorbissrc/registry.c
	libs/oggvorbis/vorbissrc/codebook.c
	libs/oggvorbis/vorbissrc/sharedbook.c
	libs/oggvorbis/vorbissrc/lookup.c
	libs/oggvorbis/vorbissrc/bitrate.c
	libs/oggvorbis/vorbissrc/vorbisfile.c)

### === GwenLIB === ###
include_directories(libs/gwen/include)
file(GLOB GWEN_INCLUDES libs/gwen/include/gwen/*.h)
file(GLOB GWEN_SOURCES libs/gwen/src/*.cpp)
source_group("libs\\gwen" FILES ${GWEN_INCLUDES})
source_group("libs\\gwen" FILES ${GWEN_SOURCES})

file(GLOB CONTROLS_INCLUDES libs/gwen/include/gwen/Controls/*.h)
file(GLOB CONTROLS_SOURCES libs/gwen/src/Controls/*.cpp)
source_group("libs\\gwen\\Controls" FILES ${CONTROLS_INCLUDES})
source_group("libs\\gwen\\Controls" FILES ${CONTROLS_SOURCES})

file(GLOB CONTROLS_DLG_INCLUDES libs/gwen/include/gwen/Controls/Dialogs/*.h)
file(GLOB CONTROLS_DLG_SOURCES libs/gwen/src/Controls/Dialogs/*.cpp)
source_group("libs\\gwen\\Controls\\Dialogs" FILES ${CONTROLS_DLG_INCLUDES})
source_group("libs\\gwen\\Controls\\Dialogs" FILES ${CONTROLS_DLG_SOURCES})

file(GLOB CONTROLS_LYT_INCLUDES libs/gwen/include/gwen/Controls/Layout/*.h)
file(GLOB CONTROLS_LYT_SOURCES libs/gwen/src/Controls/Layout/*.cpp)
source_group("libs\\gwen\\Controls\\Layout" FILES ${CONTROLS_LYT_INCLUDES})
source_group("libs\\gwen\\Controls\\Layout" FILES ${CONTROLS_LYT_SOURCES})

file(GLOB CONTROLS_PPT_INCLUDES libs/gwen/include/gwen/Controls/Property/*.h)
file(GLOB CONTROLS_PPT_SOURCES libs/gwen/src/Controls/Property/*.cpp)
source_group("libs\\gwen\\Controls\\Property" FILES ${CONTROLS_PPT_INCLUDES})
source_group("libs\\gwen\\Controls\\Property" FILES ${CONTROLS_PPT_SOURCES})

file(GLOB INPUT_INCLUDES libs/gwen/include/gwen/Input/*.h)
file(GLOB INPUT_SOURCES libs/gwen/src/Input/*.cpp)
source_group("libs\\gwen\\Input" FILES ${INPUT_INCLUDES})
source_group("libs\\gwen\\Input" FILES ${INPUT_SOURCES})

file(GLOB PLATF_INCLUDES libs/gwen/include/gwen/Platforms/*.h)
file(GLOB PLATF_SOURCES libs/gwen/src/Platforms/*.cpp)
source_group("libs\\gwen\\Platforms" FILES ${PLATF_INCLUDES})
source_group("libs\\gwen\\Platforms" FILES ${PLATF_SOURCES})

file(GLOB RENDERERS_INCLUDES libs/gwen/include/gwen/Renderers/*.h)
file(GLOB RENDERERS_SOURCES libs/gwen/src/Renderers/*.cpp)
source_group("libs\\gwen\\Renderers" FILES ${RENDERERS_INCLUDES})
source_group("libs\\gwen\\Renderers" FILES ${RENDERERS_SOURCES})

file(GLOB SKINS_INCLUDES libs/gwen/include/gwen/Skins/*.h)
file(GLOB SKINS_SOURCES libs/gwen/src/Skins/*.cpp)
source_group("libs\\gwen\\Skins" FILES ${SKINS_INCLUDES})
source_group("libs\\gwen\\Skins" FILES ${SKINS_SOURCES})

set(GWEN_LIB_INCLUDES
	${GWEN_INCLUDES}
	${CONTROLS_INCLUDES}
	${CONTROLS_DLG_INCLUDES}
	${CONTROLS_LYT_INCLUDES}
	${CONTROLS_PPT_INCLUDES}
	${INPUT_INCLUDES}
	${PLATF_INCLUDES}
	${RENDERERS_INCLUDES}
	${SKINS_INCLUDES}
)

set( GWEN_LIB_SOURCES
	${GWEN_SOURCES}
	${CONTROLS_SOURCES}
	${CONTROLS_DLG_SOURCES}
	${CONTROLS_LYT_SOURCES}
	${INPUT_SOURCES}
	${PLATF_SOURCES}
	${RENDERERS_SOURCES}
	${SKINS_SOURCES}
)

	
file(GLOB RAPIDJSON_INCLUDES libs/rapidjson/include/rapidjson/*.h)
	
file(GLOB SYS_INCLUDES sys/*.h)
file(GLOB SYS_SOURCES sys/*.cpp)

file(GLOB_RECURSE WIN32_INCLUDES sys/win32/*.h)
#file(GLOB_RECURSE WIN32_SOURCES sys/win32/*.cpp)

set(WIN32_SOURCES
	sys/win32/win_shared.cpp
	sys/win32/win_syscon.cpp)
	
	
set(WIN32_RESOURCES
	# sys/win32/rc/res/BEVEL.BMP
	# sys/win32/rc/res/BITMAP2.BMP
	# sys/win32/rc/res/BMP00001.BMP
	# sys/win32/rc/res/BMP0002.BMP
	# sys/win32/rc/res/DEFTEX.WAL
	# sys/win32/rc/res/ENDCAP.BMP
	# sys/win32/rc/res/GetString.htm
	# sys/win32/rc/res/IBEVEL.BMP
	# sys/win32/rc/res/IENDCAP.BMP
	# sys/win32/rc/res/MEFileToolbar.bmp
	# sys/win32/rc/res/MEtoolbar.bmp
	# sys/win32/rc/res/MaterialEditor.ico
	# sys/win32/rc/res/PropTree.rc2
	# sys/win32/rc/res/Q.BMP
	# sys/win32/rc/res/RADIANT3.GIF
	# sys/win32/rc/res/Radiant.ico
	# sys/win32/rc/res/RadiantDoc.ico
	# sys/win32/rc/res/TOOLBAR1.BMP
	# sys/win32/rc/res/TOOLBAR2.BMP
	# sys/win32/rc/res/Toolbar.bmp
	# sys/win32/rc/res/VIEWDEFA.BMP
	# sys/win32/rc/res/VIEWOPPO.BMP
	# sys/win32/rc/res/bmp00002.bmp
	# sys/win32/rc/res/bmp00003.bmp
	# sys/win32/rc/res/bmp00004.bmp
	# sys/win32/rc/res/bmp00005.bmp
	# sys/win32/rc/res/cchsb.bmp
	# sys/win32/rc/res/ccrgb.bmp
	# sys/win32/rc/res/dbg_back.bmp
	# sys/win32/rc/res/dbg_breakpoint.ico
	# sys/win32/rc/res/dbg_current.ico
	# sys/win32/rc/res/dbg_currentline.ico
	# sys/win32/rc/res/dbg_empty.ico
	# sys/win32/rc/res/dbg_open.bmp
	# sys/win32/rc/res/dbg_toolbar.bmp
	sys/win32/rc/res/doom.ico
	# sys/win32/rc/res/fpoint.cur
	# sys/win32/rc/res/fxed_link.ico
	# sys/win32/rc/res/fxed_toolbar.bmp
	# sys/win32/rc/res/fxeditor.ico
	# sys/win32/rc/res/guied.ico
	# sys/win32/rc/res/guied_collapse.ico
	# sys/win32/rc/res/guied_expand.ico
	# sys/win32/rc/res/guied_hand.cur
	# sys/win32/rc/res/guied_nav_visible.ico
	# sys/win32/rc/res/guied_nav_visibledisabled.ico
	# sys/win32/rc/res/guied_scripts.ico
	# sys/win32/rc/res/guied_scripts_white.ico
	# sys/win32/rc/res/guied_viewer_toolbar.bmp
	# sys/win32/rc/res/icon2.ico
	# sys/win32/rc/res/logo_sm3dfx.bmp
	# sys/win32/rc/res/matedtree.bmp
	# sys/win32/rc/res/me_disabled_icon.ico
	# sys/win32/rc/res/me_enabled.ico
	# sys/win32/rc/res/me_off_icon.ico
	# sys/win32/rc/res/me_on_icon.ico
	# sys/win32/rc/res/qe3.ico
	# sys/win32/rc/res/shaderbar.bmp
	# sys/win32/rc/res/shaderdoc.ico
	# sys/win32/rc/res/shaderframe.ico
	# sys/win32/rc/res/spliter.cur
	)

#if(USE_MFC_TOOLS)
	list(APPEND WIN32_RESOURCES sys/win32/rc/doom.rc)
#else()
#	list(APPEND WIN32_RESOURCES sys/win32/rc/doom_nomfc.rc)
#endif()


file(GLOB POSIX_INCLUDES sys/posix/*.h)
file(GLOB POSIX_SOURCES sys/posix/*.cpp)

file(GLOB COMMON_INCLUDES sys/common/*.h)
file(GLOB COMMON_SOURCES sys/common/*.cpp)

source_group("libs\\irrxml" FILES ${IRRXML_INCLUDES})
source_group("libs\\irrxml" FILES ${IRRXML_SOURCES})

source_group("libs\\jpeg-6" FILES ${JPEG_INCLUDES})
source_group("libs\\jpeg-6" FILES ${JPEG_SOURCES})

source_group("libs\\png" FILES ${PNG_INCLUDES})
source_group("libs\\png" FILES ${PNG_SOURCES})

source_group("libs\\zlib" FILES ${ZLIB_INCLUDES})
source_group("libs\\zlib" FILES ${ZLIB_SOURCES})

source_group("libs\\zlib\\minizip" FILES ${MINIZIP_INCLUDES})
source_group("libs\\zlib\\minizip" FILES ${MINIZIP_SOURCES})

source_group("libs\\glew" FILES ${GLEW_INCLUDES})
source_group("libs\\glew" FILES ${GLEW_SOURCES})

source_group("libs\\rapidjson" FILES ${RAPIDJSON_INCLUDES})
source_group("libs\\freetype" FILES ${FREETYPE_SOURCES})

source_group("sys" FILES ${SYS_INCLUDES})
source_group("sys" FILES ${SYS_SOURCES})

source_group("sys\\win32" FILES ${WIN32_INCLUDES})
source_group("sys\\win32" FILES ${WIN32_SOURCES})
source_group("sys\\win32\\Resources" FILES ${WIN32_RESOURCES})

source_group("sys\\posix" FILES ${POSIX_INCLUDES})
source_group("sys\\posix" FILES ${POSIX_SOURCES})

source_group("sys\\common" FILES ${COMMON_INCLUDES})
source_group("sys\\common" FILES ${COMMON_SOURCES})

set(RBDOOM3_INCLUDES
			${GWEN_INCLUDES}
			${IRRXML_INCLUDES}
			${JPEG_INCLUDES}
			${PNG_INCLUDES}
			${ZLIB_INCLUDES}
			${MINIZIP_INCLUDES}
			${GLEW_INCLUDES}
			${RAPIDJSON_INCLUDES}
			#${FREETYPE_SOURCES}
			${SOUND_INCLUDES}
			#${OGGVORBIS_INCLUDES}
			${COMMON_INCLUDES}
			)
			
set(RBDOOM3_SOURCES
			${GWEN_LIB_SOURCES}
			${IRRXML_SOURCES}
			${JPEG_SOURCES}
			${PNG_SOURCES}
			${ZLIB_SOURCES}
			${MINIZIP_SOURCES}
			${GLEW_SOURCES}
			#${FREETYPE_SOURCES}
			${SOUND_SOURCES}
			#${OGGVORBIS_INCLUDES} ${OGGVORBIS_SOURCES}
			${COMMON_SOURCES}
			)

add_definitions(-D__DOOM__
				#-DBUILD_FREETYPE
				#-DFT2_BUILD_LIBRARY
				)

if(USE_PRECOMPILED_HEADERS) 
	set(RBDOOM3_PRECOMPILED_SOURCES ${RBDOOM3_SOURCES})
	list(REMOVE_ITEM RBDOOM3_PRECOMPILED_SOURCES ${GWEN_LIB_SOURCES} ${JPEG_SOURCES} ${PNG_SOURCES} ${ZLIB_SOURCES} ${GLEW_SOURCES})
	list(REMOVE_ITEM RBDOOM3_PRECOMPILED_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/libs/zlib/minizip/ioapi.c)	
	list(REMOVE_ITEM RBDOOM3_PRECOMPILED_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/sys/precompiled.cpp)	
endif()

set( RBDOOM3_LIBRARIES
			idlib
			renderer
			framework
			game
			sound
			tools	
			${OPENGL_LIBRARY}
			${SDL2_LIBRARY}
			${SDL2_TTF_LIBRARIES}
			${AUDIO_LIB_LIBRARY}
            ${ZLIB_LIBRARY}
            ${PNG_LIBRARY}
            ${JPEG_LIBRARY}
            ${GLEW_LIBRARY}
			${CMAKE_DL_LIBS}
	)


if(MSVC)
	list(APPEND RBDOOM3_INCLUDES
		${SYS_INCLUDES} 
		${WIN32_INCLUDES})
		
	list(APPEND RBDOOM3_SOURCES
		${SYS_SOURCES}
		${WIN32_SOURCES})

	list(REMOVE_DUPLICATES RBDOOM3_SOURCES)

	if(USE_PRECOMPILED_HEADERS)
		set_source_files_properties(
		${RBDOOM3_PRECOMPILED_SOURCES}
		PROPERTIES
		COMPILE_FLAGS "/Yuprecompiled.h"
		OBJECT_DEPENDS "precompiled.pch"
		)
		
		set_source_files_properties(sys/precompiled.cpp
		PROPERTIES
		COMPILE_FLAGS "/Ycprecompiled.h"
		OBJECT_OUTPUTS "precompiled.pch"
		)
	endif()
	
	list(APPEND RBDOOM3_SOURCES ${WIN32_RESOURCES})
	
	add_executable(RBDoom3BFG WIN32 ${RBDOOM3_INCLUDES} ${RBDOOM3_SOURCES})

	add_dependencies(RBDoom3BFG idlib framework game renderer sound tools)
	target_link_libraries(RBDoom3BFG
		dbghelp
		iphlpapi
		winmm
		wsock32.lib
		${RBDOOM3_LIBRARIES}
		)
		
	#CMAKE_BINARY_DIR
	if(CMAKE_CL_64)
		install(TARGETS RBDoom3BFG
			RUNTIME DESTINATION .)
	else()
		install(TARGETS RBDoom3BFG
			RUNTIME DESTINATION .)
	endif()
	
else()

	list(APPEND RBDOOM3_SOURCES
		${SYS_INCLUDES} ${SYS_SOURCES})

	if(UNIX)
		if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
			list(REMOVE_ITEM POSIX_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/sys/posix/platform_linux.cpp)
		else()
			list(REMOVE_ITEM POSIX_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/sys/posix/platform_osx.cpp)
		endif()

		list(APPEND RBDOOM3_SOURCES
			${POSIX_INCLUDES} ${POSIX_SOURCES})
	endif()

	list(REMOVE_DUPLICATES RBDOOM3_SOURCES)
	
	if(USE_PRECOMPILED_HEADERS) 	
		foreach( src_file ${RBDOOM3_PRECOMPILED_SOURCES} )
			#message(STATUS "-include precompiled.h for ${src_file}")
			set_source_files_properties(
				${src_file}
				PROPERTIES
				COMPILE_FLAGS "-include ${CMAKE_CURRENT_SOURCE_DIR}/idlib/precompiled.h"
				)
		endforeach()

		# precompiled magic for GCC/clang, adapted from https://gist.github.com/573926
		STRING(TOUPPER "CMAKE_CXX_FLAGS_${CMAKE_BUILD_TYPE}" _flags_var_name)
		SET(_compiler_FLAGS ${${_flags_var_name}})
		GET_DIRECTORY_PROPERTY(_directory_flags INCLUDE_DIRECTORIES)
		FOREACH(item ${_directory_flags})
			LIST(APPEND _compiler_FLAGS " -I${item}")
		ENDFOREACH(item)
	endif()
		
	GET_DIRECTORY_PROPERTY(_directory_flags DEFINITIONS)
	LIST(APPEND _compiler_FLAGS ${_directory_flags})
	SEPARATE_ARGUMENTS(_compiler_FLAGS)
	
	if(USE_PRECOMPILED_HEADERS)
		# we need to recreate the precompiled header for RBDoom3BFG 
		# (i.e. can't use the one created for idlib before)
		# because some definitions (e.g. -D__IDLIB__ -D__DOOM_DLL__) differ
		add_custom_target(precomp_header_rbdoom3bfg ALL
				COMMAND ${CMAKE_CXX_COMPILER} ${_compiler_FLAGS} -x c++-header idlib/precompiled.h -o idlib/precompiled.h.gch
				WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
				COMMENT "Creating idlib/precompiled.h.gch for RBDoom3BFG"
				)
	endif()
	
	if(WIN32)
		set(remove_command "del")
	else()
		set(remove_command "rm")
	endif()
	
	if (USE_PRECOMPILED_HEADERS)
		# it's ugly enough that the precompiled header binary needs to be in the 
		# source directory (instead of the build directory), so let's at least
		# delete it after build.
		add_custom_target(rm_precomp_header ALL
				COMMAND ${remove_command} "idlib/precompiled.h.gch"
				WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
				COMMENT "remove idlib/precompiled.h.gch"
				)
				
		# make sure this is run after creating idlib
		add_dependencies(precomp_header_rbdoom3bfg idlib)
	endif()
	
	add_executable(RBDoom3BFG WIN32 ${RBDOOM3_SOURCES})

	if (USE_PRECOMPILED_HEADERS)
		# make sure precompiled header is created before executable is compiled
		add_dependencies(RBDoom3BFG precomp_header_rbdoom3bfg)
		
		# make sure precompiled header is deleted after executable is compiled
		add_dependencies(rm_precomp_header RBDoom3BFG)
	endif()	

	if(NOT WIN32)
		if(NOT "${CMAKE_SYSTEM}" MATCHES "Darwin")
			set(RT_LIBRARY rt)
		endif()

		target_link_libraries(RBDoom3BFG		
			${RBDOOM3_LIBRARIES}
			${RT_LIBRARY}
			pthread
			)
	endif()
endif()
