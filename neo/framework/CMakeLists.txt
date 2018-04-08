include_directories(
	${CMAKE_SOURCE_DIR}
	${CMAKE_SOURCE_DIR}/idlib)
	
file(GLOB FRAMEWORK_INCLUDES *.h)
file(GLOB FRAMEWORK_SOURCES *.cpp)
source_group("" FILES ${FRAMEWORK_INCLUDES})
source_group("" FILES ${FRAMEWORK_SOURCES})

file(GLOB AAS_INCLUDES aas/*.h)
file(GLOB AAS_SOURCES aas/*.cpp)
source_group("aas" FILES ${AAS_INCLUDES})
source_group("aas" FILES ${AAS_SOURCES})

file(GLOB ASYNC_INCLUDES async/*.h)
file(GLOB ASYNC_SOURCES async/*.cpp)
source_group("async" FILES ${ASYNC_INCLUDES})
source_group("async" FILES ${ASYNC_SOURCES})

file(GLOB_RECURSE CM_INCLUDES cm/*.h)
file(GLOB_RECURSE CM_SOURCES cm/*.cpp)
source_group("cm" FILES ${CM_INCLUDES})
source_group("cm" FILES ${CM_SOURCES})

file(GLOB SWF_INCLUDES swf/*.h)
file(GLOB SWF_SOURCES swf/*.cpp)
source_group("swf" FILES ${SWF_INCLUDES})
source_group("swf" FILES ${SWF_SOURCES})

file(GLOB UI_INCLUDES ui/*.h)
file(GLOB UI_SOURCES ui/*.cpp)
source_group("ui" FILES ${UI_INCLUDES})
source_group("ui" FILES ${UI_SOURCES})

set(FRAMEWORK_INCLUDES_ALL
	${FRAMEWORK_INCLUDES}
	${AAS_INCLUDES}
	${ASYNC_INCLUDES}
	${CM_INCLUDES}
	${SWF_INCLUDES}
	${UI_INCLUDES}

	)

set(FRAMEWORK_SOURCES_ALL
	${FRAMEWORK_SOURCES}
	${AAS_SOURCES}
	${ASYNC_SOURCES}
	${CM_SOURCES}
	${SWF_SOURCES}
	${UI_SOURCES}
	)

	##game logic is build as a library
if(NOT MONOLITH)
	add_definitions(-D__DOOM_DLL__ -DGAME_DLL)
endif()
if(USE_TOOLS)
	add_definitions(-D_ID_ALLOW_TOOLS_)
endif(USE_TOOLS)

if (USE_PRECOMPILED_HEADERS)
		set(FRAMEWORK_PRECOMPILED_SOURCES ${FRAMEWORK_SOURCES_ALL})
		list(REMOVE_ITEM FRAMEWORK_PRECOMPILED_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/precompiled.cpp)
		list(REMOVE_ITEM FRAMEWORK_PRECOMPILED_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/Common_mapconvert.cpp)
		list(REMOVE_ITEM FRAMEWORK_PRECOMPILED_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/Common_demos.cpp)
		
	if(MSVC)
		set_source_files_properties(precompiled.cpp
        PROPERTIES
        COMPILE_FLAGS "/Ycprecompiled.h"
        OBJECT_OUTPUTS "precompiled.pch"
        )
        set_source_files_properties(
            ${FRAMEWORK_PRECOMPILED_SOURCES}
            PROPERTIES
            COMPILE_FLAGS "/Yuprecompiled.h"
            OBJECT_DEPENDS "precompiled.pch"
            )
	else()
	foreach( src_file ${FRAMEWORK_PRECOMPILED_SOURCES} )
		#message(STATUS "-include precompiled.h for ${src_file}")
		set_source_files_properties(
			${src_file}
			PROPERTIES
			COMPILE_FLAGS "-include ${CMAKE_CURRENT_SOURCE_DIR}/precompiled.h"
			)
	endforeach()
	# precompiled magic for GCC/clang, adapted from https://gist.github.com/573926
	STRING(TOUPPER "CMAKE_CXX_FLAGS_${CMAKE_BUILD_TYPE}" _flags_var_name)
	SET(_compiler_FLAGS ${${_flags_var_name}})
	GET_DIRECTORY_PROPERTY(_directory_flags INCLUDE_DIRECTORIES)
	FOREACH(item ${_directory_flags})
		LIST(APPEND _compiler_FLAGS " -I${item}")
	ENDFOREACH(item)
	
	add_custom_target(precomp_header_framework ALL
	                  COMMAND ${CMAKE_CXX_COMPILER} ${_compiler_FLAGS} -x c++-header precompiled.h -o precompiled.h.gch
	                  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	                  COMMENT "Creating framework/precompiled.h.gch for idlib"
	                  )
	endif()
endif(USE_PRECOMPILED_HEADERS)

add_library(framework ${FRAMEWORK_SOURCES_ALL} ${FRAMEWORK_INCLUDES_ALL})

if (USE_PRECOMPILED_HEADERS)
	add_dependencies(framework precomp_header_framework)
endif(USE_PRECOMPILED_HEADERS)