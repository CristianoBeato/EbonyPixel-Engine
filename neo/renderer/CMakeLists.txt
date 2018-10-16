file(GLOB RENDERER_INCLUDES *.h)
file(GLOB RENDERER_SOURCES *.cpp)
source_group("" FILES ${RENDERER_INCLUDES})
source_group("" FILES ${RENDERER_SOURCES})

file(GLOB BACKEND_INCLUDES backEnd/*.h)
file(GLOB BACKEND_SOURCES backEnd/*.cpp)
source_group("backEnd" FILES ${BACKEND_INCLUDES})
source_group("backEnd" FILES ${BACKEND_SOURCES})

file(GLOB CONTEXT_INCLUDES Context/*.h)
file(GLOB CONTEXT_SOURCES Context/*.cpp)
source_group("Context" FILES ${CONTEXT_INCLUDES})
source_group("Context" FILES ${CONTEXT_SOURCES})

file(GLOB FRONTEND_INCLUDES frontEnd/*.h)
file(GLOB FRONTEND_SOURCES frontEnd/*.cpp)
source_group("frontEnd" FILES ${FRONTEND_INCLUDES})
source_group("frontEnd" FILES ${FRONTEND_SOURCES})

file(GLOB IMAGE_INCLUDES Image/*.h)
file(GLOB IMAGE_SOURCES Image/*.cpp)
source_group("Image" FILES ${IMAGE_INCLUDES})
source_group("Image" FILES ${IMAGE_SOURCES})

file(GLOB IMAGE_COLOR_INCLUDES Image/Color/*.h)
file(GLOB IMAGE_COLOR_SOURCES Image/Color/*.cpp)
source_group("Image\\Color" FILES ${IMAGE_COLOR_INCLUDES})
source_group("Image\\Color" FILES ${IMAGE_COLOR_SOURCES})

file(GLOB IMAGE_DXT_INCLUDES Image/DXT/*.h)
file(GLOB IMAGE_DXT_SOURCES Image/DXT/*.cpp)
source_group("Image\\DXT" FILES ${IMAGE_DXT_INCLUDES})
source_group("Image\\DXT" FILES ${IMAGE_DXT_SOURCES})

file(GLOB IMAGE_LOADERS_INCLUDES Image/loaders/*.h)
file(GLOB IMAGE_LOADERS_SOURCES Image/loaders/*.cpp)
source_group("Image\\loaders" FILES ${IMAGE_LOADERS_INCLUDES})
source_group("Image\\loaders" FILES ${IMAGE_LOADERS_SOURCES})

file(GLOB JOBS_INCLUDES jobs/*.h)
file(GLOB JOBS_SOURCES  jobs/*.cpp)
source_group("jobs" FILES ${JOBS_INCLUDES})
source_group("jobs" FILES ${JOBS_SOURCES})

file(GLOB MODELS_INCLUDES models/*.h)
file(GLOB MODELS_SOURCES models/*.cpp)
source_group("models" FILES ${MODELS_INCLUDES})
source_group("models" FILES ${MODELS_SOURCES})

file(GLOB MODELS_LOADERS_INCLUDES models/loaders/*.h)
file(GLOB MODELS_LOADERS_SOURCES models/loaders/*.cpp)
source_group("models\\loaders" FILES ${MODELS_LOADERS_INCLUDES})
source_group("models\\loaders" FILES ${MODELS_LOADERS_SOURCES})

file(GLOB MODELS_INTERNAL_INCLUDES models/internal/*.h)
file(GLOB MODELS_INTERNAL_SOURCES models/internal/*.cpp)
source_group("models\\internal" FILES ${MODELS_INTERNAL_INCLUDES})
source_group("models\\internal" FILES ${MODELS_INTERNAL_SOURCES})

file(GLOB OPENGL_INCLUDES OpenGL/*.h)
file(GLOB OPENGL_SOURCES OpenGL/*.cpp)
source_group("OpenGL" FILES ${OPENGL_INCLUDES})
source_group("OpenGL" FILES ${OPENGL_SOURCES})

file(GLOB RENDERWORLD_INCLUDES RenderWorld/*.h)
file(GLOB RENDERWORLD_SOURCES RenderWorld/*.cpp)
source_group("RenderWorld" FILES ${RENDERWORLD_INCLUDES})
source_group("RenderWorld" FILES ${RENDERWORLD_SOURCES})

file(GLOB SMAA_INCLUDES SMAA/*.h)
source_group("SMAA" FILES ${SMAA_INCLUDES})

set(RENDERER_INCLUDES_ALL 
	${RENDERER_INCLUDES}
	${BACKEND_INCLUDES}
	${CONTEXT_INCLUDES}
	${FRONTEND_INCLUDES}
	${IMAGE_INCLUDES}
	${IMAGE_COLOR_INCLUDES}
	${IMAGE_DXT_INCLUDES}
	${IMAGE_LOADERS_INCLUDES}
	${JOBS_INCLUDES}
	${MODELS_INCLUDES}
	${MODELS_LOADERS_INCLUDES}
	${MODELS_INTERNAL_INCLUDES}
	${OPENGL_INCLUDES}
	${RENDERWORLD_INCLUDES}
	${SMAA_INCLUDES}
	)
	
set(RENDERER_SOURCES_ALL
	${RENDERER_SOURCES}
	${BACKEND_SOURCES}
	${FRONTEND_SOURCES}
	${CONTEXT_SOURCES}
	${IMAGE_SOURCES}
	${IMAGE_DXT_SOURCES}
	${IMAGE_LOADERS_SOURCES}
	${IMAGE_COLOR_SOURCES}
	${JOBS_SOURCES}
	${MODELS_SOURCES}
	${MODELS_LOADERS_SOURCES}
	${MODELS_INTERNAL_SOURCES}
	${OPENGL_SOURCES}
	${RENDERWORLD_SOURCES}
	)

if(USE_PRECOMPILED_HEADERS) 
		set(RENDERER_PRECOMPILED_SOURCES ${RENDERER_SOURCES_ALL})
		list(REMOVE_ITEM RENDERER_PRECOMPILED_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/DXT/DXTDecoder.cpp)
		list(REMOVE_ITEM RENDERER_PRECOMPILED_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/DXT/DXTEncoder.cpp)
		list(REMOVE_ITEM RENDERER_PRECOMPILED_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/DXT/DXTEncoder_SSE2.cpp)
		list(REMOVE_ITEM RENDERER_PRECOMPILED_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/jobs/dynamicshadowvolume/DynamicShadowVolume.cpp)
		list(REMOVE_ITEM RENDERER_PRECOMPILED_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/jobs/prelightshadowvolume/PreLightShadowVolume.cpp)
		list(REMOVE_ITEM RENDERER_PRECOMPILED_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/jobs/staticshadowvolume/StaticShadowVolume.cpp)
		list(REMOVE_ITEM RENDERER_PRECOMPILED_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/jobs/ShadowShared.cpp)
		list(REMOVE_ITEM RENDERER_PRECOMPILED_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/RenderLog.cpp)
		list(REMOVE_ITEM RENDERER_PRECOMPILED_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/precompiled.cpp)
		  
	if(MSVC)
	   		set_source_files_properties( ${RENDERER_PRECOMPILED_SOURCES} PROPERTIES
									COMPILE_FLAGS "/Yuprecompiled.h"
									OBJECT_DEPENDS "precompiled.pch"
		)
		
		set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/precompiled.cpp PROPERTIES
									COMPILE_FLAGS "/Ycprecompiled.h"
									OBJECT_OUTPUTS "precompiled.pch"
		)
	else()
		foreach( src_file ${RENDERER_PRECOMPILED_SOURCES} )
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
		
		add_custom_target(precomp_header_renderer ALL
	                  COMMAND ${CMAKE_CXX_COMPILER} ${_compiler_FLAGS} -x c++-header precompiled.h -o precompiled.h.gch
	                  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	                  COMMENT "Creating renderer/precompiled.h.gch for renderer"
	                  )
	endif()
endif()

add_library(renderer ${RENDERER_SOURCES_ALL} ${RENDERER_INCLUDES_ALL})

if(NOT MSVC)
	if (USE_PRECOMPILED_HEADERS)
		add_dependencies(renderer precomp_header_renderer)
	endif(USE_PRECOMPILED_HEADERS)
endif()