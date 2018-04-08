set(GAME_INCLUDES
	Achievements.h
	Actor.h
	AF.h
	AFEntity.h
	AimAssist.h
	BrittleFracture.h
	Camera.h
	#EndLevel.h
	Entity.h
	Fx.h
	Game.h
	GameEdit.h
	Game_local.h
	Grabber.h
	IK.h
	Item.h
	Leaderboards.h
	Light.h
	Misc.h
	Moveable.h
	Mover.h
	MultiplayerGame.h
	Player.h
	PlayerIcon.h
	PlayerView.h
	PredictedValue.h
	Projectile.h
	Pvs.h
	SecurityCamera.h
	SmokeParticles.h
	Sound.h
	Target.h
	Trigger.h
	Weapon.h
	WorldSpawn.h)
source_group("" FILES ${GAME_INCLUDES})

set(GAME_SOURCES
	Achievements.cpp
	Actor.cpp
	AF.cpp
	AFEntity.cpp
	AimAssist.cpp
	BrittleFracture.cpp
	Camera.cpp
	#EndLevel.cpp
	Entity.cpp
	Fx.cpp
	GameEdit.cpp
	Game_local.cpp
	Game_network.cpp
	Grabber.cpp
	IK.cpp
	Item.cpp
	Leaderboards.cpp
	Light.cpp
	Misc.cpp
	Moveable.cpp
	Mover.cpp
	MultiplayerGame.cpp
	Player.cpp
	PlayerIcon.cpp
	PlayerView.cpp
	precompiled.cpp
	Projectile.cpp
	Pvs.cpp
	SecurityCamera.cpp
	SmokeParticles.cpp
	Sound.cpp
	Target.cpp
	Trigger.cpp
	Weapon.cpp
	WorldSpawn.cpp)
source_group("" FILES ${GAME_SOURCES})

file(GLOB GAME_AI_INCLUDES ai/*.h)
source_group("ai" FILES ${GAME_AI_INCLUDES})
file(GLOB GAME_AI_SOURCES ai/*.cpp)
source_group("ai" FILES ${GAME_AI_SOURCES})

source_group("anim" FILES ${GAME_ANIM_INCLUDES})
file(GLOB GAME_ANIM_INCLUDES anim/*.h)
source_group("anim" FILES ${GAME_ANIM_SOURCES})
file(GLOB GAME_ANIM_SOURCES anim/*.cpp)

file(GLOB GAME_GAMESYS_INCLUDES gamesys/*.h)
source_group("gamesys" FILES ${GAME_GAMESYS_INCLUDES})
set(GAME_GAMESYS_SOURCES
	
	#gamesys/Callbacks.cpp
	gamesys/Class.cpp
	#gamesys/DebugGraph.cpp
	gamesys/Event.cpp
	gamesys/SaveGame.cpp
	gamesys/SysCmds.cpp
	gamesys/SysCvar.cpp
	#gamesys/TypeInfo.cpp
	)
source_group("gamesys" FILES ${GAME_GAMESYS_SOURCES})

file(GLOB GAME_MENUS_INCLUDES menus/*.h)
source_group("menus" FILES ${GAME_MENUS_INCLUDES})
file(GLOB GAME_MENUS_SOURCES menus/*.cpp)
source_group("menus" FILES ${GAME_MENUS_SOURCES})

file(GLOB GAME_PHYSICS_INCLUDES physics/*.h)
source_group("physics" FILES ${GAME_PHYSICS_INCLUDES})
file(GLOB GAME_PHYSICS_SOURCES physics/*.cpp)
source_group("physics" FILES ${GAME_PHYSICS_SOURCES})

file(GLOB GAME_SCRIPT_INCLUDES script/*.h)
source_group("script" FILES ${GAME_SCRIPT_INCLUDES})
file(GLOB GAME_SCRIPT_SOURCES script/*.cpp)
source_group("script" FILES ${GAME_SCRIPT_SOURCES})

set( GAME_INCLUDES_ALL
	${GAME_INCLUDES}
	${GAME_AI_INCLUDES}	
	${GAME_ANIM_INCLUDES}
	${GAME_GAMESYS_INCLUDES}
	${GAME_MENUS_INCLUDES}
	${GAME_PHYSICS_INCLUDES}
	${GAME_SCRIPT_INCLUDES}
	)

set( GAME_SOURCES_ALL
	${GAME_SOURCES}
	${GAME_AI_SOURCES}
	${GAME_ANIM_SOURCES}
	${GAME_GAMESYS_SOURCES}
	${GAME_MENUS_SOURCES}
	${GAME_PHYSICS_SOURCES}
	${GAME_SCRIPT_SOURCES}
	)

include_directories(
	${CMAKE_SOURCE_DIR}
	${CMAKE_SOURCE_DIR}/idlib)
	
if(USE_PRECOMPILED_HEADERS) 
		set(GAME_PRECOMPILED_SOURCES ${GAME_SOURCES_ALL})
		list(REMOVE_ITEM GAME_PRECOMPILED_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/precompiled.cpp)
		  
	if(MSVC)
		set_source_files_properties( ${GAME_PRECOMPILED_SOURCES} PROPERTIES
									COMPILE_FLAGS "/Yuprecompiled.h"
									OBJECT_DEPENDS "precompiled.pch"
		)
		
		set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/precompiled.cpp PROPERTIES
									COMPILE_FLAGS "/Ycprecompiled.h"
									OBJECT_OUTPUTS "precompiled.pch"
		)
	else()
		foreach( src_file ${GAME_PRECOMPILED_SOURCES} )
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
		
		add_custom_target(precomp_header_game ALL
	                  COMMAND ${CMAKE_CXX_COMPILER} ${_compiler_FLAGS} -x c++-header precompiled.h -o precompiled.h.gch
	                  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	                  COMMENT "Creating game/precompiled.h.gch for renderer"
	                  )
	endif()
endif()
	
if(MONOLITH)
	add_library(game  STATIC ${GAME_SOURCES_ALL} ${GAME_INCLUDES_ALL})
else()
	add_definitions(-D__DOOM_DLL__ -DGAME_DLL)
	add_library(game  SHARED ${GAME_SOURCES_ALL} ${GAME_INCLUDES_ALL})
endif()

if(NOT MSVC)
	if (USE_PRECOMPILED_HEADERS)
		add_dependencies(game precomp_header_game)
	endif(USE_PRECOMPILED_HEADERS)
endif()