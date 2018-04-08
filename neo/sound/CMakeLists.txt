file(GLOB SOUND_INCLUDES *.h)
file(GLOB SOUND_SOURCES *.cpp)
source_group("" FILES ${SOUND_INCLUDES})
source_group("" FILES ${SOUND_SOURCES})

if(XAUDIO2)
	file(GLOB SOUND_LIB_INCLUDES XAudio2/*.h)
	file(GLOB SOUND_LIB_SOURCES XAudio2/*.cpp)
	source_group("XAudio2" FILES ${SOUND_LIB_INCLUDES})
	source_group("XAudio2" FILES ${SOUND_LIB_SOURCES})
elseif(OPENAL)
	file(GLOB OPENAL_INCLUDES OpenAL/*.h)
	file(GLOB SOUND_LIB_SOURCES OpenAL/*.cpp)
	source_group("OpenAL" FILES ${SOUND_LIB_INCLUDES})
	source_group("OpenAL" FILES ${SOUND_LIB_SOURCES})
else()
	file(GLOB SOUND_LIB_INCLUDES stub/*.h)
	file(GLOB SOUND_LIB_SOURCES stub/*.cpp)
	source_group("stub" FILES ${SOUND_LIB_INCLUDES})
	source_group("stub" FILES ${SOUND_LIB_SOURCES})
endif()

set(SOUND_INCLUDES_ALL
	${SOUND_INCLUDES}
	${SOUND_LIB_INCLUDES}
	)
set(SOUND_SOURCES_ALL
	${SOUND_SOURCES}
	${SOUND_LIB_SOURCES}
	)

include_directories(${AUDIO_LIB_INCLUDE_DIRS})	

if(XAUDIO2)
	add_definitions(-D_USE_XAUDIO2_)
elseif(OPENAL)
	add_definitions(-D_USE_OPENAL_)
endif()

add_library(sound ${SOUND_SOURCES_ALL} ${SOUND_INCLUDES_ALL})
