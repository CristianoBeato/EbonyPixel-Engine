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

In addition, the Doom 3 BFG Edition Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 BFG Edition Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

#ifndef __SYS_LOCAL__
#define __SYS_LOCAL__

#include "common/sys_cpu_common.h"
#include "common/sys_filesys_common.h"
#include "common/sys_process_common.h"
#include "common/sys_glimp.h"
#include "common/sys_input.h"

/*
==============================================================

	idSysLocal

==============================================================
*/

class idSysLocal : public idSys
{
public:
	idSysLocal(void);

	virtual void			DebugPrintf( VERIFY_FORMAT_STRING const char* fmt, ... );
	virtual void			DebugVPrintf( const char* fmt, va_list arg );
	virtual void			Sleep(int msec);
	virtual int				Milliseconds();
	virtual uint64			Microseconds();
	virtual double			GetClockTicks();
	virtual double			ClockTicksPerSecond();
	virtual void			CPUCount(int& numLogicalCPUCores, int& numPhysicalCPUCores, int& numCPUPackages);
	virtual cpuid_t			GetProcessorId();
	virtual const char* 	GetProcessorString();
	virtual const char* 	FPU_GetState();
	virtual bool			FPU_StackIsEmpty();
	virtual void			FPU_SetFTZ( bool enable );
	virtual void			FPU_SetDAZ( bool enable );
	virtual void			FPU_SetPrecision(int precision);
	virtual void			FPU_EnableExceptions( int exceptions );

	virtual int				GetDriveFreeSpace(const char* path);
	virtual int64			GetDriveFreeSpaceInBytes(const char* path);
	virtual ID_TIME_T		FileTimeStamp(idFileHandle fp);
	virtual void			Mkdir(const char* path);
	virtual bool			Rmdir(const char* path);
	virtual bool			IsFileWritable(const char* path);
	virtual sysFolder_t		IsFolder(const char* path);
	//virtual int				ListFiles(const char* directory, const char* extension, idList<class idStr>& list);
	virtual btFileSysCommon GetFSHandler(void);
	virtual const char* 	EXEPath(void);
	virtual const char* 	CWD(void);
	virtual const char* 	LaunchPath(void);
	virtual const char* 	DefaultBasePath(void);
	virtual const char* 	DefaultSavePath(void);

	virtual bool			LockMemory( void* ptr, int bytes );
	virtual bool			UnlockMemory( void* ptr, int bytes );
	virtual void			SetPhysicalWorkMemory(int minBytes, int maxBytes);

	virtual void*			DLL_Load(const char* dllName) override;
	virtual void* 			DLL_GetProcAddress(void* dllHandle, const char* procName) override;
	virtual void			DLL_Unload(void* dllHandle) override;
	virtual void			DLL_GetFileName(const char* baseName, char* dllName, int maxLength) override;
	
	virtual sysEvent_t		GenerateMouseButtonEvent( int button, bool down );
	virtual sysEvent_t		GenerateMouseMoveEvent( int deltax, int deltay );
	
	virtual void			OpenURL( const char* url, bool quit );
	virtual void			StartProcess( const char* exeName, bool quit );

	virtual void			GrabMouseCursor(bool grabIt);
	virtual btGlimp			GetGlimpHandle(void);
	virtual void			GLimpPreInit(void);
	virtual bool			GLimpInit(glimpParms_t parms);
	virtual bool			GLimpSetScreenParms(glimpParms_t parms);
	virtual void			GLimpShutdown(void);
	virtual void			GLimpSetGamma(unsigned short red[256], unsigned short green[256], unsigned short blue[256]);
	virtual void			GLimpSwapBuffers(void);

	virtual void			ShowWindow(bool show);
	virtual bool			IsWindowVisible(void);

	virtual void			InitInput(void);
	virtual void			ShutdownInput(void);

	virtual void			GenerateEvents(void);
	virtual sysEvent_t		GetEvent(void);
	virtual void			ClearEvents(void);

	virtual int				PollKeyboardInputEvents(void);
	virtual int				ReturnKeyboardInputEvent(const int n, int& ch, bool& state);
	virtual void			EndKeyboardInputEvents(void);

	virtual void			SetRumble(int device, int low, int hi);
	virtual int				PollJoystickInputEvents(int deviceNum);
	virtual int				ReturnJoystickInputEvent(int deviceNum, const int n, int& action, int& value);
	virtual void			EndJoystickInputEvents(void);

	virtual int				PollMouseInputEvents(int mouseEvents[MAX_MOUSE_EVENTS][2]);

	virtual char* 			GetClipboardData(void);
	virtual void			SetClipboardData(const char* string);

	virtual SDL_Window*		GetWindowHandler(void);

protected:

	btProcess		m_proc;
	btMemoryStatus	m_mem;
	btFileSysCommon	m_fs;

	btGlimp			m_Glimp;
	btCpuInfo		m_CPUinfoSet;
	btFPUInfo		m_FPUinfoSet;

	btMouseSDL		m_gameMouse;
	btKeyboardSDL	m_gameKeyboard;
	btJoystickSDL	m_gameJoystick;
};


static void PushConsoleEvent(const char* s);

// both strings are expected to have at most SDL_TEXTINPUTEVENT_TEXT_SIZE chars/ints (including terminating null)
static void ConvertUTF8toUTF32(const char* utf8str, int32* utf32buf);

static int SDLScanCodeToKeyNum(SDL_Scancode sc);
static SDL_Scancode KeyNumToSDLScanCode(int keyNum);

// DG: those are needed for moving/resizing windows
extern idCVar r_windowX;
extern idCVar r_windowY;
extern idCVar r_windowWidth;
extern idCVar r_windowHeight;
// DG end

#endif /* !__SYS_LOCAL__ */
