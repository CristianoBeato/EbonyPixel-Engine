
add_definitions(-D_ID_ALLOW_TOOLS_)

file(GLOB COMPILER_INCLUDES compilers/*.h)
source_group("compilers" FILES ${COMPILER_INCLUDES})

file(GLOB COMPILER_AAS_INCLUDES compilers/aas/*.h)
file(GLOB COMPILER_AAS_SOURCES compilers/aas/*.cpp)
source_group("compilers\\aas" FILES ${COMPILER_AAS_INCLUDES})
source_group("compilers\\aas" FILES ${COMPILER_AAS_SOURCES})

file(GLOB COMPILER_DMAP_INCLUDES compilers/dmap/*.h)
file(GLOB COMPILER_DMAP_SOURCES compilers/dmap/*.cpp)
source_group("compilers\\dmap" FILES ${COMPILER_DMAP_INCLUDES})
source_group("compilers\\dmap" FILES ${COMPILER_DMAP_SOURCES})

file(GLOB COMPILER_ROQVQ_INCLUDES compilers/roqvq/*.h)
file(GLOB COMPILER_ROQVQ_SOURCES compilers/roqvq/*.cpp)
source_group("compilers\\roqvq" FILES ${COMPILER_ROQVQ_INCLUDES})
source_group("compilers\\roqvq" FILES ${COMPILER_ROQVQ_SOURCES})

file(GLOB PROFILING_INCLUDES profiling/*.h)
file(GLOB PROFILING_SOURCES profiling/*.cpp)
source_group("profiling//" FILES ${PROFILING_INCLUDES})
source_group("profiling" FILES ${PROFILING_SOURCES})

set(TOOLS_INCLUDES_ALL
	${COMPILER_INCLUDES}
	${COMPILER_AAS_INCLUDES}
	${COMPILER_DMAP_INCLUDES}
	${COMPILER_ROQVQ_INCLUDES}
	${PROFILING_INCLUDES}
	)
	
set(TOOLS_SOURCES_ALL
	${COMPILER_DMAP_SOURCES}
	${COMPILER_AAS_SOURCES}
	${COMPILER_ROQVQ_SOURCES}
	${PROFILING_SOURCES}
	)

add_library(tools ${TOOLS_SOURCES_ALL} ${TOOLS_INCLUDES_ALL})