/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.

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

In addition, the Doom 3 BFG Edition Source Code is also subject to certain additional terms.
You should have received a copy of these additional terms immediately following the terms and conditions of the
GNU General Public License which accompanied the Doom 3 BFG Edition Source Code.  If not, 
please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in
writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/
#include "precompiled.h"
#pragma hdrstop

#include "sys_local.h"
#include "sys_public.h"

#include <SDL_main.h>
#include <SDL_ttf.h>

#include "common/sys_console_common.h"

#if defined(WIN32)
#include "win32/win_local.h"
#else
#include "posix/posix_public.h"
#endif

idCVar sys_allowMultipleInstances("sys_allowMultipleInstances", "0", CVAR_SYSTEM | CVAR_BOOL, "allow multiple instances running concurrently");
idCVar sys_outputEditString("sys_outputEditString", "1", CVAR_SYSTEM | CVAR_BOOL, "");
idCVar sys_viewlog("sys_viewlog", "0", CVAR_SYSTEM | CVAR_ARCHIVE | CVAR_INTEGER, "");
cpuid_t		cpuid;

#define MAXPRINTMSG 4096

Console				sysCon;
idSysLocal			sysLocal;
idSys* 				sys = &sysLocal;
static idStr cmdLine;

static void Sys_StroreArgs(int cmd_argc, char** cmd_argv)
{
	for (int i = 0; i < cmd_argc; i++)
	{
		cmdLine.Append(cmd_argv[i]);
	}
}

/*
========================
Sys_GetCmdLine
========================
*/
const char* Sys_GetCmdLine()
{
	return cmdLine.c_str();
}

/*
==================
Sys_FlushCacheMemory

On windows, the vertex buffers are write combined, so they
don't need to be flushed from the cache
==================
*/
void Sys_FlushCacheMemory(void *base, int bytes)
{
}

void Sys_SetErrorText(const char* buf)
{
	//send the error to show in the console
	sysCon.SetErrorText(buf);

	//create a message box whit the error 
	btMessageBox *boxMem = new btMessageBox(SDL_MESSAGEBOX_INFORMATION, "ERROR!", buf);

	//set main game window child
	boxMem->SetParent(sys->GetWindowHandler());
	//show the mesg
	boxMem->Show();

	delete boxMem;
}

/*
==================
Sys_Sentry
==================
*/
void Sys_Sentry()
{
}

/*
=================
Sys_In_Restart_f

Restart the input subsystem
=================
*/
void Sys_In_Restart_f(const idCmdArgs &args)
{
	sys->ShutdownInput();
	sys->InitInput();
}

/*
================
Sys_Init
The cvar system must already be setup
================
*/
void Sys_Init(void)
{
	//start sdl
	SDL_Init(SDL_INIT_EVERYTHING);
	TTF_Init();

	idStr string;

	cmdSystem->AddCommand("in_restart", Sys_In_Restart_f, CMD_FL_SYSTEM, "restarts the input system");

	common->Printf("%1.0f MHz ", sys->ClockTicksPerSecond() / 1000000.0f);
	cpuid = sys->GetProcessorId();

	string.Clear();

	if (cpuid & CPUID_AMD)
		string += "AMD CPU";
	else if (cpuid & CPUID_INTEL)
		string += "Intel CPU";
	else if (cpuid & CPUID_UNSUPPORTED)
		string += "unsupported CPU";
	else
		string += "generic CPU";

	string += " with ";
	if (cpuid & CPUID_MMX)
		string += "MMX & ";
	if (cpuid & CPUID_3DNOW)
		string += "3DNow! & ";
	if (cpuid & CPUID_SSE)
		string += "SSE & ";
	if (cpuid & CPUID_SSE2)
		string += "SSE2 & ";
	if (cpuid & CPUID_SSE3)
		string += "SSE3 & ";
	if (cpuid & CPUID_HTT)
		string += "HTT & ";

	string.StripTrailing(" & ");
	string.StripTrailing(" with ");

	common->Printf(string.c_str());
	//sys_cpustring.SetString(string);
}

/*
========================
Sys_ReLaunch
========================
*/
void Sys_ReLaunch()
{
#if defined(_WIN32)
	Win_ReLaunch();
#else
	Posix_ReLaunch();
#endif
}

/*
================
Sys_Shutdown
================
*/
void Sys_Shutdown(void)
{
#if defined(_WIN32)
	Win_Shutdown();
#else
	Posix_Shutdown();
#endif
	SDL_Quit();
	TTF_Quit();
}

/*
==============
Sys_Quit
==============
*/
void Sys_Quit(void)
{
	sysCon.DestroyConsole();
#if defined(_WIN32)
	Win_Quit();
#else
	Posix_Quit();
#endif
}

#if defined(ID_MCHECK) && defined(__linux__)

#include <mcheck.h>
/*
===============
mem consistency stuff
===============
*/
const char* mcheckstrings[] =
{
	"MCHECK_DISABLED",
	"MCHECK_OK",
	"MCHECK_FREE",	// block freed twice
	"MCHECK_HEAD",	// memory before the block was clobbered
	"MCHECK_TAIL"	// memory after the block was clobbered
};

void abrt_func(mcheck_status status)
{
	Sys_Printf("memory consistency failure: %s\n", mcheckstrings[status + 1]);
	Posix_SetExit(EXIT_FAILURE);
	common->Quit();
}

#endif

bool Sys_AlreadyRunning(void)
{
	if (!sys_allowMultipleInstances.GetBool())
	{

	}

	return false;
}

/*
==============
Sys_Printf
==============
*/
void Sys_Printf(const char *fmt, ...)
{
	char		msg[MAXPRINTMSG];
	va_list argptr;

#if defined(_WIN32)
	va_start(argptr, fmt);
	idStr::vsnPrintf(msg, MAXPRINTMSG - 1, fmt, argptr);
	va_end(argptr);
	msg[sizeof(msg) - 1] = '\0';

#if 0
	OutputDebugString(msg);
#else
	printf(msg);
#endif

#elif defined(__ANDROID__)
	va_start(argptr, fmt);
	idStr::vsnPrintf(msg, sizeof(msg), fmt, argptr);
	va_end(argptr);
	msg[sizeof(msg) - 1] = '\0';

	__android_log_print(ANDROID_LOG_DEBUG, "RBDoom3", msg);
#else
	tty_Hide();
	va_start(argptr, fmt);
	vprintf(fmt, argptr);
	va_end(argptr);
	tty_Show();
#endif

	if (sys_outputEditString.GetBool() && idLib::IsMainThread())
		sysCon.ConsoleAppendText(msg);
}

/*
==============
Sys_DebugPrintf
==============
*/
void Sys_DebugPrintf(const char *fmt, ...)
{
	va_list argptr;
	char msg[MAXPRINTMSG];
#if defined(_WIN32)
	va_start(argptr, fmt);
	idStr::vsnPrintf(msg, MAXPRINTMSG - 1, fmt, argptr);
	msg[sizeof(msg) - 1] = '\0';
	va_end(argptr);

	OutputDebugString(msg);
#elif defined(__ANDROID__)
	va_start(argptr, fmt);
	idStr::vsnPrintf(msg, sizeof(msg), fmt, argptr);
	va_end(argptr);
	msg[sizeof(msg) - 1] = '\0';

	__android_log_print(ANDROID_LOG_DEBUG, "RBDoom3_Debug", msg);
#else
	tty_Hide();
	va_start(argptr, fmt);
	vprintf(fmt, argptr);
	va_end(argptr);
	tty_Show();
#endif
}

/*
==============
Sys_VPrintf
==============
*/
void Sys_VPrintf(const char* fmt, va_list arg)
{
	char msg[MAXPRINTMSG];
#if defined(_WIN32)

	idStr::vsnPrintf(msg, MAXPRINTMSG - 1, fmt, arg);
	msg[sizeof(msg) - 1] = '\0';

	OutputDebugString(msg);
#elif  defined(__ANDROID__)
	__android_log_vprint(ANDROID_LOG_DEBUG, "RBDoom3", fmt, arg);
#else
	tty_Hide();
	vprintf(fmt, arg);
	tty_Show();
#endif
}

/*
==============
Sys_DebugVPrintf
==============
*/
void Sys_DebugVPrintf(const char *fmt, va_list arg)
{
	char msg[MAXPRINTMSG];
#if defined(_WIN32)
	idStr::vsnPrintf(msg, MAXPRINTMSG - 1, fmt, arg);
	msg[sizeof(msg) - 1] = '\0';

	OutputDebugString(msg);
#elif  defined(__ANDROID__)
	__android_log_vprint(ANDROID_LOG_DEBUG, "RBDoom3_Debug", fmt, arg);
#else
	tty_Hide();
	vprintf(fmt, arg);
	tty_Show();
#endif
}

/*
=============
Sys_Error

Show the early console as an error dialog
=============
*/
void Sys_Error(const char *error, ...)
{
	va_list		argptr;
	char		text[MAXPRINTMSG];
#if !defined(_WIN32)
	Sys_Printf("Sys_Error: ");
#endif
	va_start(argptr, error);
	vsprintf(text, error, argptr);
	//Sys_DebugVPrintf(error, argptr);
	va_end(argptr);

	sysCon.CreateConsoleWindow(CONS_MODE_ERR);
	sys->GetGlimpHandle().GrabInput(GRAB_HIDECURSOR);

	sysCon.ConsoleAppendText(text);
	sysCon.ConsoleAppendText("\n");

	Sys_SetErrorText(text);
	
	//timeEndPeriod(1);

	//shutdonw video and input subsystem
	sys->ShutdownInput();
	sys->GLimpShutdown();

	extern idCVar com_productionMode;
	if (com_productionMode.GetInteger() == 0)
		sysCon.ShowConsole(true);

	sysCon.DestroyConsole();

#if !defined(_WIN32)
	Sys_Printf("\n");
	Posix_Exit(EXIT_FAILURE);
#endif
	exit(-1);
}

/*
==================
Sys_SetFatalError
==================
*/
void Sys_SetFatalError(const char* error)
{
#if defined(_WIN32)
	//Win_SetFatalError(error);
#else
	Posix_SetFatalError(error);
#endif // _WIN32
}

/*
==============
Sys_ConsoleInput
==============
*/
const char* Sys_ConsoleInput(void)
{
#if 0
#if defined(_WIN32)
	return Win_ConsoleInput();
#else
	return Posix_ConsoleInput();
#endif
#else
	return sysCon.ConsoleInput();
#endif
}

/*
===============
main
===============
*/
int main(int argc, char *argv[])
{
	// make sure SDL cleans up before exit
	atexit(SDL_Quit);

	// DG: needed for Sys_ReLaunch()
	Sys_StroreArgs(argc, argv);
	// DG end

	// done before Com/Sys_Init since we need this for error output
	sysCon.CreateConsole();
#if defined(_WIN32)
	Win_EarlyInit();
#else
#ifdef ID_MCHECK && defined(__linux__)
	// must have -lmcheck linkage
	mcheck(abrt_func);
	Sys_Printf("memory consistency checking enabled\n");
#endif
	Posix_EarlyInit();
#endif // (_WIN32)

	if (argc > 1)
		common->Init(argc - 1, &argv[1], NULL);
	else
		common->Init(0, NULL, NULL);

#if defined(_WIN32)
	Win_LateInit();
#else
	Posix_LateInit();
#endif

	// hide or show the early console as necessary
	if (sys_viewlog.GetInteger())
	{
		sysCon.CreateConsoleWindow(1);
		sysCon.ShowConsole( true, true);
	}

	//main game loop
	while (1)
	{
#if defined(_WIN32)
#if defined(_DEBUG) && defined(ID_MCHECK)
		Sys_MemFrame();
#endif
		// set exceptions, even if some crappy syscall changes them!
		sys->FPU_EnableExceptions(TEST_FPU_EXCEPTIONS);
#endif

		// run the game
		common->Frame();
	}

	// never gets here
	return 0;
}