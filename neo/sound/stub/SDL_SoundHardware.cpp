/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
Copyright (C) 2013 Robert Beckebans
Copyright (c) 2010 by Chris Robinson <chris.kcat@gmail.com> (OpenAL Info Utility)

This file is part of the Doom 3 BFG Edition GPL Source Code ("Doom 3 BFG Edition Source Code").

Doom 3 BFG Edition Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 BFG Edition Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 BFG Edition Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 BFG Edition Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 BFG Edition Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

#include "precompiled.h"
#pragma hdrstop

#include "sound/snd_local.h"
#include "SDL_SoundHardware.h"

#include <SDL_audio.h>

idCVar s_showLevelMeter( "s_showLevelMeter", "0", CVAR_BOOL | CVAR_ARCHIVE, "Show VU meter" );
idCVar s_meterTopTime( "s_meterTopTime", "1000", CVAR_INTEGER | CVAR_ARCHIVE, "How long (in milliseconds) peaks are displayed on the VU meter" );
idCVar s_meterPosition( "s_meterPosition", "100 100 20 200", CVAR_ARCHIVE, "VU meter location (x y w h)" );
idCVar s_device( "s_device", "-1", CVAR_INTEGER | CVAR_ARCHIVE, "Which audio device to use (listDevices to list, -1 for default)" );
idCVar s_showPerfData( "s_showPerfData", "0", CVAR_BOOL, "Show XAudio2 Performance data" );
extern idCVar s_volume_dB;


static void listDevices_f(const idCmdArgs& args)
{
	idLib::Printf("Available playback devices:\n");
	uint32 numDevices = SDL_GetNumAudioDrivers();

	for (auto i = 0; i < numDevices; i++)
	{
		const char* deviceName = SDL_GetAudioDriver(i);
		if (!deviceName)
			continue;


	}

/*
========================
idSoundHardware_OpenAL::idSoundHardware_OpenAL
========================
*/
idSoundHardware_SDL::idSoundHardware_SDL(void)
{
}

void idSoundHardware_SDL::PrintDeviceList( const char* list )
{
	if( !list || *list == '\0' )
	{
		idLib::Printf( "    !!! none !!!\n" );
	}
	else
	{
		do
		{
			idLib::Printf( "    %s\n", list );
			list += strlen( list ) + 1;
		}
		while( *list != '\0' );
	}
}

void idSoundHardware_SDL::PrintSDLCInfo( ALCdevice* device )
{
	ALCint major, minor;
	
	if( device )
	{
		const ALCchar* devname = NULL;
		idLib::Printf( "\n" );
		if( alcIsExtensionPresent( device, "ALC_ENUMERATE_ALL_EXT" ) != AL_FALSE )
		{
			devname = alcGetString( device, ALC_ALL_DEVICES_SPECIFIER );
		}
		
		if( CheckALCErrors( device ) != ALC_NO_ERROR || !devname )
		{
			devname = alcGetString( device, ALC_DEVICE_SPECIFIER );
		}
		
		idLib::Printf( "** Info for device \"%s\" **\n", devname );
	}
	alcGetIntegerv( device, ALC_MAJOR_VERSION, 1, &major );
	alcGetIntegerv( device, ALC_MINOR_VERSION, 1, &minor );
	
	if( CheckALCErrors( device ) == ALC_NO_ERROR )
		idLib::Printf( "ALC version: %d.%d\n", major, minor );
		
	if( device )
	{
		idLib::Printf( "SDL extensions: %s", alGetString( AL_EXTENSIONS ) );
		
		//idLib::Printf("ALC extensions:");
		//printList(alcGetString(device, ALC_EXTENSIONS), ' ');
		CheckALCErrors( device );
	}
}

void idSoundHardware_SDL::PrintSDLnfo(void)
{
	idLib::Printf( "SDL vendor string: %s\n", alGetString( AL_VENDOR ) );
	idLib::Printf( "SDL renderer string: %s\n", alGetString( AL_RENDERER ) );
	idLib::Printf( "SDL version string: %s\n", alGetString( AL_VERSION ) );
	idLib::Printf( "SDL extensions: %s", alGetString( AL_EXTENSIONS ) );
	//PrintList(alGetString(AL_EXTENSIONS), ' ');
	CheckALErrors();
}

/*
========================
idSoundHardware_OpenAL::Init
========================
*/
void idSoundHardware_SDL::Init(void)
{
	if (SDL_WasInit(SDL_INIT_AUDIO) != 0)
	{
		if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0)
			common->Error("ERROR idSoundHardware_SDL::Init, SDL_InitSubSystem falhed, %s", SDL_GetError());

	}

	cmdSystem->AddCommand("listDevices", listDevices_f, 0, "Lists the connected sound devices", NULL);
	common->Printf("Setup SDL audio device and context... ");
	common->Printf( "Done.\n" );

	sdlNumAudioDrivers = SDL_GetNumAudioDrivers();


	PrintSDLInfo();
}

/*
========================
idSoundHardware_OpenAL::Shutdown
========================
*/
void idSoundHardware_SDL::Shutdown(void)
{
}

/*
========================
idSoundHardware_OpenAL::AllocateVoice
========================
*/
idSoundVoice* idSoundHardware_SDL::AllocateVoice( const idSoundSample* leadinSample, const idSoundSample* loopingSample )
{

	return NULL;
}

/*
========================
idSoundHardware_OpenAL::FreeVoice
========================
*/
void idSoundHardware_SDL::FreeVoice( idSoundVoice* voice )
{

}

/*
========================
idSoundHardware_OpenAL::Update
========================
*/
void idSoundHardware_SDL::Update(void)
{
}


