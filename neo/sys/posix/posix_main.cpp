/*
===========================================================================

Doom 3 GPL Source Code
Copyright (C) 1999-2011 id Software LLC, a ZeniMax Media company.
Copyright (C) 2012 Robert Beckebans

This file is part of the Doom 3 GPL Source Code (?Doom 3 Source Code?).

Doom 3 Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

#pragma hdrstop
#include "precompiled.h"
#include "sys/sys_local.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <dirent.h>

#include <sys/mman.h>
#include <sys/time.h>
#include <pwd.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <fnmatch.h>

// RB begin
#if defined(__ANDROID__)
#include <android/log.h>
#endif

#if defined(__APPLE__)
#include <SDL2/SDL.h>
#endif

#include <sys/statvfs.h>
// RB end

#include "posix_public.h"

#define					MAX_OSPATH 256
#define					COMMAND_HISTORY 64

static idStr			basepath;
static idStr			savepath;

static int				input_hide = 0;

idEditField				input_field;
static char				input_ret[256];

static idStr			history[ COMMAND_HISTORY ];	// cycle buffer
static int				history_count = 0;			// buffer fill up
static int				history_start = 0;			// current history start
static int				history_current = 0;		// goes back in history
idEditField				history_backup;				// the base edit line

// pid - useful when you attach to gdb..
idCVar com_pid( "com_pid", "0", CVAR_INTEGER | CVAR_INIT | CVAR_SYSTEM, "process id" );

// exit - quit - error --------------------------------------------------------

static int set_exit = 0;
static char exit_spawn[ 1024 ];

/*
================
Posix_Exit
================
*/
void Posix_Exit( int ret )
{
	if( tty_enabled )
	{
		Sys_Printf( "shutdown terminal support\n" );
		if( tcsetattr( 0, TCSADRAIN, &tty_tc ) == -1 )
		{
			Sys_Printf( "tcsetattr failed: %s\n", strerror( errno ) );
		}
	}
	// at this point, too late to catch signals
	Posix_ClearSigs();
	
	//if( asyncThread.threadHandle )
	//{
	//	Sys_DestroyThread( asyncThread );
	//}
	
	// process spawning. it's best when it happens after everything has shut down
	if( exit_spawn[0] )
	{
		Sys_DoStartProcess( exit_spawn, false );
	}
	// in case of signal, handler tries a common->Quit
	// we use set_exit to maintain a correct exit code
	if( set_exit )
	{
		exit( set_exit );
	}
	exit( ret );
}

/*
================
Posix_SetExit
================
*/
void Posix_SetExit( int ret )
{
	set_exit = 0;
}

/*
===============
Posix_SetExitSpawn
set the process to be spawned when we quit
===============
*/
void Posix_SetExitSpawn( const char* exeName )
{
	idStr::Copynz( exit_spawn, exeName, 1024 );
}

/*
==================
idSysLocal::StartProcess
if !quit, start the process asap
otherwise, push it for execution at exit
(i.e. let complete shutdown of the game and freeing of resources happen)
NOTE: might even want to add a small delay?
==================
*/
void Posix_StartProcess( const char* exeName, bool quit )
{
	if( quit )
	{
		common->DPrintf( "Sys_StartProcess %s (delaying until final exit)\n", exeName );
		Posix_SetExitSpawn( exeName );
		cmdSystem->BufferCommandText( CMD_EXEC_APPEND, "quit\n" );
		return;
	}
	
	common->DPrintf( "Sys_StartProcess %s\n", exeName );
	Sys_DoStartProcess( exeName );
}

/*
================
Sys_Quit
================
*/
void Posix_Quit()
{
	Posix_Exit( EXIT_SUCCESS );
}

/*
===============
Sys_FPE_handler
===============
*/
void Sys_FPE_handler( int signum, siginfo_t* info, void* context )
{
	assert( signum == SIGFPE );
	Sys_Printf( "FPE\n" );
}

/*
=================
Posix_Shutdown
=================
*/
void Posix_Shutdown()
{
	basepath.Clear();
	savepath.Clear();
	for( int i = 0; i < COMMAND_HISTORY; i++ )
	{
		history[ i ].Clear();
	}
}

/*
================
Sys_ShowConsole
================
*/
void Posix_ShowConsole( int visLevel, bool quitOnClose ) { }

// ---------------------------------------------------------------------------
/*
===============
Posix_EarlyInit
===============
*/
void Posix_EarlyInit()
{
	//memset( &asyncThread, 0, sizeof( asyncThread ) );
	
	exit_spawn[0] = '\0';
	Posix_InitSigs();
	
	// set the base time
	sys->Milliseconds();
	
	//Posix_InitPThreads();
}

/*
===============
Posix_LateInit
===============
*/
void Posix_LateInit()
{
	Posix_InitConsoleInput();
	com_pid.SetInteger( getpid() );
	common->Printf( "pid: %d\n", com_pid.GetInteger() );
//	common->Printf( "%d MB System Memory\n", Sys_GetSystemRam() );

//#ifndef ID_DEDICATED
	//common->Printf( "%d MB Video Memory\n", Sys_GetVideoRam() );
//#endif

	//Posix_StartAsyncThread( );
}

/*
================
Sys_SetLanguageFromSystem
================
*/
extern idCVar sys_lang;
void Sys_SetLanguageFromSystem()
{
	sys_lang.SetString( Sys_DefaultLanguage() );
}

/*
=================
Sys_OpenURL
=================
*/
void Posix_OpenURL( const char* url, bool quit )
{
	const char*	scr
	ipt_path;
	idFile*		script_file;
	char		cmdline[ 1024 ];
	
	static bool	quit_spamguard = false;
	
	if( quit_spamguard )
	{
		common->DPrintf( "Sys_OpenURL: already in a doexit sequence, ignoring %s\n", url );
		return;
	}
	
	common->Printf( "Open URL: %s\n", url );
	// opening an URL on *nix can mean a lot of things ..
	// just spawn a script instead of deciding for the user :-)
	
	// look in the savepath first, then in the basepath
	script_path = fileSystem->BuildOSPath( cvarSystem->GetCVarString( "fs_savepath" ), "", "openurl.sh" );
	script_file = fileSystem->OpenExplicitFileRead( script_path );
	if( !script_file )
	{
		script_path = fileSystem->BuildOSPath( cvarSystem->GetCVarString( "fs_basepath" ), "", "openurl.sh" );
		script_file = fileSystem->OpenExplicitFileRead( script_path );
	}
	if( !script_file )
	{
		common->Printf( "Can't find URL script 'openurl.sh' in either savepath or basepath\n" );
		common->Printf( "OpenURL '%s' failed\n", url );
		return;
	}
	fileSystem->CloseFile( script_file );
	
	// if we are going to quit, only accept a single URL before quitting and spawning the script
	if( quit )
	{
		quit_spamguard = true;
	}
	
	common->Printf( "URL script: %s\n", script_path );
	
	// StartProcess is going to execute a system() call with that - hence the &
	idStr::snPrintf( cmdline, 1024, "%s '%s' &",  script_path, url );
	sys->StartProcess( cmdline, quit );
}


