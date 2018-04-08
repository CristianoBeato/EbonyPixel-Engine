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

#include "precompiled.h"
#pragma hdrstop

#include "sys_local.h"

#include "renderer/tr_local.h"
#include "common/sdl2_scancode_mappings.h"
#include "common/sys_glimp.h"
#include "common/sys_input.h"
#include "common/sys_filesys_common.h"
#include "common/sys_cpu_common.h"
#include "common/sys_process_common.h"

#if defined(WIN32)
#include "sys/win32/win_local.h"
#else
#include "sys/posix/posix_public.h"
#endif
const char* sysLanguageNames[] =
{
	ID_LANG_ENGLISH, ID_LANG_FRENCH, ID_LANG_ITALIAN, ID_LANG_GERMAN, ID_LANG_SPANISH, ID_LANG_JAPANESE, ID_LANG_PORTUGUESE, NULL
};

const int numLanguages = sizeof( sysLanguageNames ) / sizeof sysLanguageNames[ 0 ] - 1;

idCVar sys_lang( "sys_lang", ID_LANG_ENGLISH, CVAR_SYSTEM | CVAR_INIT, "", sysLanguageNames, idCmdSystem::ArgCompletion_String<sysLanguageNames> );

void idSysLocal::DebugPrintf( const char* fmt, ... )
{
	va_list argptr;
	
	va_start( argptr, fmt );
	Sys_DebugVPrintf( fmt, argptr );
	va_end( argptr );
}

idSysLocal::idSysLocal(void)
{
	m_proc = btProcess();
	m_mem = btMemoryStatus();
	m_fs = btFileSysCommon();
	m_Glimp = btGlimp();
	m_CPUinfoSet = btCpuInfo();
	m_FPUinfoSet = btFPUInfo();
	m_gameMouse = btMouseSDL();
	m_gameKeyboard = btKeyboardSDL();
	//m_gameJoystick = btJoystickSDL();
}

void idSysLocal::DebugVPrintf( const char* fmt, va_list arg )
{
	Sys_DebugVPrintf( fmt, arg );
}

/*
==============
idSysLocal::Sleep
==============
*/
void idSysLocal::Sleep(int msec)
{
	SDL_Delay(msec);
}

/*
================
idSysLocal::Milliseconds
================
*/
int idSysLocal::Milliseconds()
{
	return SDL_GetTicks();
}

/*
========================
idSysLocal::Microseconds
========================
*/
uint64 idSysLocal::Microseconds()
{
	uint64 microsecSize = SDL_GetPerformanceFrequency() / 1000000;
	return SDL_GetPerformanceCounter() * microsecSize;
}

double idSysLocal::GetClockTicks()
{
	return SDL_GetPerformanceCounter();
}

double idSysLocal::ClockTicksPerSecond()
{
	return SDL_GetPerformanceFrequency();
}

void idSysLocal::CPUCount(int & numLogicalCPUCores, int & numPhysicalCPUCores, int & numCPUPackages)
{
	m_CPUinfoSet.CPUCount(numLogicalCPUCores, numPhysicalCPUCores, numCPUPackages);
}

cpuid_t idSysLocal::GetProcessorId()
{
	return m_CPUinfoSet.GetCPUId();
}

const char* idSysLocal::GetProcessorString()
{
	return m_CPUinfoSet.GetProcessorString();
}

const char* idSysLocal::FPU_GetState()
{
	return m_FPUinfoSet.GetState();
}

bool idSysLocal::FPU_StackIsEmpty()
{
	return m_FPUinfoSet.StackIsEmpty();
}

void idSysLocal::FPU_SetFTZ( bool enable )
{
	return m_FPUinfoSet.SetFTZ( enable );
}

void idSysLocal::FPU_SetDAZ( bool enable )
{
	return m_FPUinfoSet.SetDAZ( enable );
}

void idSysLocal::FPU_SetPrecision(int precision)
{
	m_FPUinfoSet.SetPrecision(precision);
}

bool idSysLocal::LockMemory( void* ptr, int bytes )
{
	return m_mem.LockMemory( ptr, bytes );
}

bool idSysLocal::UnlockMemory( void* ptr, int bytes )
{
	return m_mem.UnlockMemory( ptr, bytes );
}

void idSysLocal::SetPhysicalWorkMemory(int minBytes, int maxBytes)
{
	m_mem.SetPhysicalWorkMemory(minBytes, maxBytes);
}

void* idSysLocal::DLL_Load( const char* dllName )
{
	return m_proc.DLL_Load( dllName );
}

void* idSysLocal::DLL_GetProcAddress( void* dllHandle, const char* procName )
{
	return  m_proc.DLL_GetProcAddress( dllHandle, procName );
}

void idSysLocal::DLL_Unload( void* dllHandle )
{
	m_proc.DLL_Unload( dllHandle );
}

void idSysLocal::DLL_GetFileName( const char* baseName, char* dllName, int maxLength )
{
	idStr::snPrintf( dllName, maxLength, "%s" CPUSTRING ".dll", baseName );
}

sysEvent_t idSysLocal::GenerateMouseButtonEvent( int button, bool down )
{
	sysEvent_t ev;
	ev.evType = SE_KEY;
	ev.evValue = K_MOUSE1 + button - 1;
	ev.evValue2 = down;
	ev.evPtrLength = 0;
	ev.evPtr = NULL;
	return ev;
}

sysEvent_t idSysLocal::GenerateMouseMoveEvent( int deltax, int deltay )
{
	sysEvent_t ev;
	ev.evType = SE_MOUSE;
	ev.evValue = deltax;
	ev.evValue2 = deltay;
	ev.evPtrLength = 0;
	ev.evPtr = NULL;
	return ev;
}

void idSysLocal::FPU_EnableExceptions( int exceptions )
{
	m_FPUinfoSet.EnableExceptions( exceptions );
}

int idSysLocal::GetDriveFreeSpace(const char* path)
{
	return m_fs.GetDriveFreeSpace(path);
}

int64 idSysLocal::GetDriveFreeSpaceInBytes(const char* path)
{
	return m_fs.GetDriveFreeSpaceInBytes(path);
}

ID_TIME_T idSysLocal::FileTimeStamp(idFileHandle fp)
{
	return m_fs.FileTimeStamp(fp);
}

void idSysLocal::Mkdir(const char * path)
{
	m_fs.Mkdir(path);
}

bool idSysLocal::Rmdir(const char * path)
{
	return m_fs.Rmdir(path);
}

bool idSysLocal::IsFileWritable(const char * path)
{
	return m_fs.IsFileWritable(path);
}

sysFolder_t idSysLocal::IsFolder(const char * path)
{
	return m_fs.IsFolder(path);
}

#if 0
int idSysLocal::ListFiles(const char * directory, const char * extension, idList<class idStr>& list)
{
	return m_fs.ListFiles(directory, extension, list);
}
#endif

btFileSysCommon idSysLocal::GetFSHandler(void)
{
	return this->m_fs;
}

const char * idSysLocal::EXEPath(void)
{
	return m_fs.EXEPath();
}

const char * idSysLocal::CWD(void)
{
	return m_fs.Sys_Cwd();
}

const char * idSysLocal::LaunchPath(void)
{
	return m_fs.EXEPath();
}

const char* idSysLocal::DefaultBasePath(void)
{
	return m_fs.DefaultBasePath();
}

const char*  idSysLocal::DefaultSavePath(void)
{
	return m_fs.DefaultSavePath();
}

void idSysLocal::GrabMouseCursor(bool grabIt)
{
	int flags;

	if (grabIt)
	{
		// DG: disabling the cursor is now done once in GLimp_Init() because it should always be disabled
		flags = GRAB_ENABLE | GRAB_SETSTATE;
		// DG end
	}
	else
	{
		flags = GRAB_SETSTATE;
	}
	m_Glimp.GrabInput(flags);
}

btGlimp idSysLocal::GetGlimpHandle(void)
{
	return this->m_Glimp;
}

void idSysLocal::GLimpPreInit(void)
{
	m_Glimp.PreInit();

	//don't let the screensaver take the window control
	//can ocur while playing whit controlers
	SDL_DisableScreenSaver();
}

bool idSysLocal::GLimpInit(glimpParms_t parms)
{
	return m_Glimp.Init(parms);
}

bool idSysLocal::GLimpSetScreenParms(glimpParms_t parms)
{
	return m_Glimp.SetScreenParms(parms);
}

void idSysLocal::GLimpShutdown(void)
{
	m_Glimp.Shutdown();
}

void idSysLocal::GLimpSetGamma(unsigned short red[256], unsigned short green[256], unsigned short blue[256])
{
	m_Glimp.SetGamma(red, green, blue);
}

void idSysLocal::GLimpSwapBuffers(void)
{
	m_Glimp.SwapBuffers();
}

void idSysLocal::ShowWindow(bool show)
{
	m_Glimp.ShowWindow(show);
}

bool idSysLocal::IsWindowVisible(void)
{
	return m_Glimp.IsWindowVisible();
}

/*
===========
idSysLocal::InitInput
===========
*/
void idSysLocal::InitInput(void)
{
	common->Printf("\n------- Input Initialization -------\n");

	// GameController
	if (SDL_Init(SDL_INIT_EVENTS | SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC))
		common->Printf("SDL_Init(SDL_INIT_EVENTS | SDL_INIT_GAMECONTROLLER) error: %s\n", SDL_GetError());

	m_gameMouse.Init();
	m_gameKeyboard.Init();
	m_gameJoystick.Init();
	common->Printf("------------------------------------\n");
}

/*
===========
idSysLocal::ShutdownInput
===========
*/
void idSysLocal::ShutdownInput(void)
{
	m_gameJoystick.Shutdown();
	m_gameKeyboard.Shutdown();
	m_gameMouse.Shutdown();
}

/*
===========
idSysLocal::ShutdownInput
===========
*/
void idSysLocal::GenerateEvents(void)
{
	char* s = (char*)Sys_ConsoleInput();

	if (s)
		PushConsoleEvent(s);

	SDL_PumpEvents();
}

static int controllerButtonRemap[][2] =
{
	{ K_JOY1, J_ACTION1 },
	{ K_JOY2, J_ACTION2 },
	{ K_JOY3, J_ACTION3 },
	{ K_JOY4, J_ACTION4 },
	{ K_JOY9, J_ACTION9 },
	{ K_JOY11, J_ACTION11 },
	{ K_JOY10, J_ACTION10 },
	{ K_JOY7, J_ACTION7 },
	{ K_JOY8, J_ACTION8 },
	{ K_JOY5, J_ACTION5 },
	{ K_JOY6, J_ACTION6 },
	{ K_JOY_DPAD_UP, J_DPAD_UP },
	{ K_JOY_DPAD_DOWN, J_DPAD_DOWN },
	{ K_JOY_DPAD_LEFT, J_DPAD_LEFT },
	{ K_JOY_DPAD_RIGHT, J_DPAD_RIGHT },
};

/*
===========
idSysLocal::GetEvent
===========
*/
sysEvent_t idSysLocal::GetEvent(void)
{
	sysEvent_t res = {};
	SDL_Event ev;
	int key;

	// when this is returned, it's assumed that there are no more events!
	static const sysEvent_t no_more_events = { SE_NONE, 0, 0, 0, NULL };

	// WM0110: previous state of joystick hat
	static int previous_hat_state = SDL_HAT_CENTERED;

	// utf-32 version of the textinput event
	static int32 uniStr[SDL_TEXTINPUTEVENT_TEXT_SIZE] = { 0 };
	static size_t uniStrPos = 0;

	if (uniStr[0] != 0)
	{
		res.evType = SE_CHAR;
		res.evValue = uniStr[uniStrPos];

		++uniStrPos;

		if (!uniStr[uniStrPos] || uniStrPos == SDL_TEXTINPUTEVENT_TEXT_SIZE)
		{
			memset(uniStr, 0, sizeof(uniStr));
			uniStrPos = 0;
		}

		return res;
	}

	// DG: fake a "mousewheel not pressed anymore" event for SDL2
	// so scrolling in menus stops after one step
	static int mwheelRel = 0;
	if (mwheelRel)
	{
		res.evType = SE_KEY;
		res.evValue = mwheelRel;
		res.evValue2 = 0; // "not pressed anymore"
		mwheelRel = 0;
		return res;
	}
	// DG end

	static int32 uniChar = 0;

	if (uniChar)
	{
		res.evType = SE_CHAR;
		res.evValue = uniChar;

		uniChar = 0;

		return res;
	}

	// loop until there is an event we care about (will return then) or no more events
	while (SDL_PollEvent(&ev))
	{
		switch (ev.type)
		{
		case SDL_WINDOWEVENT:
			switch (ev.window.event)
			{
			case SDL_WINDOWEVENT_FOCUS_GAINED:
			{
				// unset modifier, in case alt-tab was used to leave window and ALT is still set
				// as that can cause fullscreen-toggling when pressing enter...
				SDL_Keymod currentmod = SDL_GetModState();
				int newmod = KMOD_NONE;
				if (currentmod & KMOD_CAPS) // preserve capslock
					newmod |= KMOD_CAPS;

				SDL_SetModState((SDL_Keymod)newmod);

				// DG: un-pause the game when focus is gained, that also re-grabs the input
				//     disabling the cursor is now done once in GLimp_Init() because it should always be disabled
				cvarSystem->SetCVarBool("com_pause", false);
				// DG end
				break;
			}

			case SDL_WINDOWEVENT_FOCUS_LOST:
				// DG: pause the game when focus is lost, that also un-grabs the input
				cvarSystem->SetCVarBool("com_pause", true);
				// DG end
				break;

			case SDL_WINDOWEVENT_LEAVE:
				// mouse has left the window
				res.evType = SE_MOUSE_LEAVE;
				return res;

				// DG: handle resizing and moving of window
			case SDL_WINDOWEVENT_RESIZED:
			{
				int w = ev.window.data1;
				int h = ev.window.data2;
				r_windowWidth.SetInteger(w);
				r_windowHeight.SetInteger(h);

				glConfig.nativeScreenWidth = w;
				glConfig.nativeScreenHeight = h;
				break;
			}

			case SDL_WINDOWEVENT_MOVED:
			{
				int x = ev.window.data1;
				int y = ev.window.data2;
				r_windowX.SetInteger(x);
				r_windowY.SetInteger(y);
				break;
			}
			}

			continue; // handle next event

		case SDL_KEYDOWN:
			if (ev.key.keysym.sym == SDLK_RETURN && (ev.key.keysym.mod & KMOD_ALT) > 0)
			{
				// DG: go to fullscreen on current display, instead of always first display
				int fullscreen = 0;
				if (!renderSystem->IsFullScreen())
				{
					// this will be handled as "fullscreen on current window"
					// r_fullscreen 1 means "fullscreen on first window" in d3 bfg
					fullscreen = -2;
				}
				cvarSystem->SetCVarInteger("r_fullscreen", fullscreen);
				// DG end
				PushConsoleEvent("vid_restart");
				continue; // handle next event
			}

			// DG: ctrl-g to un-grab mouse - yeah, left ctrl shoots, then just use right ctrl :)
			if (ev.key.keysym.sym == SDLK_g && (ev.key.keysym.mod & KMOD_CTRL) > 0)
			{
				bool grab = cvarSystem->GetCVarBool("in_nograb");
				grab = !grab;
				cvarSystem->SetCVarBool("in_nograb", grab);
				continue; // handle next event
			}
			// DG end

			// fall through
		case SDL_KEYUP:
		{
			//bool isChar;

			// DG: special case for SDL_SCANCODE_GRAVE - the console key under Esc
			if (ev.key.keysym.scancode == SDL_SCANCODE_GRAVE)
			{
				key = K_GRAVE;
				uniChar = K_BACKSPACE; // bad hack to get empty console inputline..
			} // DG end, the original code is in the else case
			else
			{
				key = SDLScanCodeToKeyNum(ev.key.keysym.scancode);

				if (key == 0)
				{
					// SDL2 has no ev.key.keysym.unicode anymore.. but the scancode should work well enough for console
					if (ev.type == SDL_KEYDOWN) // FIXME: don't complain if this was an ASCII char and the console is open?
						common->Warning("unmapped SDL key %d scancode %d", ev.key.keysym.sym, ev.key.keysym.scancode);

					continue; // just handle next event
				}
			}

			res.evType = SE_KEY;
			res.evValue = key;
			res.evValue2 = ev.key.state == SDL_PRESSED ? 1 : 0;
			m_gameKeyboard.Append(key, ev.key.state == SDL_PRESSED);

			if (key == K_BACKSPACE && ev.key.state == SDL_PRESSED)
				uniChar = key;

			return res;
		}
		case SDL_TEXTINPUT:
			if (ev.text.text[0] != '\0')
			{
				// fill uniStr array for SE_CHAR events
				ConvertUTF8toUTF32(ev.text.text, uniStr);

				// return an event with the first/only char
				res.evType = SE_CHAR;
				res.evValue = uniStr[0];

				uniStrPos = 1;

				if (uniStr[1] == 0)
				{
					// it's just this one character, clear uniStr
					uniStr[0] = 0;
					uniStrPos = 0;
				}
				return res;
			}

			continue; // just handle next event

		case SDL_MOUSEMOTION:
			// DG: return event with absolute mouse-coordinates when in menu
			// to fix cursor problems in windowed mode
			if (game && game->Shell_IsActive())
			{
				res.evType = SE_MOUSE_ABSOLUTE;
				res.evValue = ev.motion.x;
				res.evValue2 = ev.motion.y;
			}
			else     // this is the old, default behavior
			{
				res.evType = SE_MOUSE;
				res.evValue = ev.motion.xrel;
				res.evValue2 = ev.motion.yrel;
			}
			// DG end
			m_gameMouse.Append(M_DELTAX, ev.motion.xrel);
			m_gameMouse.Append(M_DELTAY, ev.motion.yrel);
			return res;

		case SDL_FINGERDOWN:
		case SDL_FINGERUP:
		case SDL_FINGERMOTION:
			continue; // Avoid 'unknown event' spam when testing with touchpad by skipping this

		case SDL_MOUSEWHEEL:
			res.evType = SE_KEY;

			res.evValue = (ev.wheel.y > 0) ? K_MWHEELUP : K_MWHEELDOWN;
			m_gameMouse.Append(M_DELTAZ, ev.wheel.y);

			res.evValue2 = 1; // for "pressed"

							  // remember mousewheel direction to issue a "not pressed anymore" event
			mwheelRel = res.evValue;

			return res;

		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
			res.evType = SE_KEY;

			switch (ev.button.button)
			{
			case SDL_BUTTON_LEFT:
				res.evValue = K_MOUSE1;
				m_gameMouse.Append(M_ACTION1, ev.button.state == SDL_PRESSED ? 1 : 0);
				break;
			case SDL_BUTTON_MIDDLE:
				res.evValue = K_MOUSE3;
				m_gameMouse.Append(M_ACTION3, ev.button.state == SDL_PRESSED ? 1 : 0);
				break;
			case SDL_BUTTON_RIGHT:
				res.evValue = K_MOUSE2;
				m_gameMouse.Append(M_ACTION2, ev.button.state == SDL_PRESSED ? 1 : 0);
				break;

			default:
				// handle X1 button and above
				if (ev.button.button <= 16) // d3bfg doesn't support more than 16 mouse buttons
				{
					int buttonIndex = ev.button.button - SDL_BUTTON_LEFT;
					res.evValue = K_MOUSE1 + buttonIndex;
					m_gameMouse.Append(M_ACTION1 + buttonIndex, ev.button.state == SDL_PRESSED ? 1 : 0);
				}
				else // unsupported mouse button
				{
					continue; // just ignore
				}
			}

			res.evValue2 = ev.button.state == SDL_PRESSED ? 1 : 0;

			return res;

			// GameController
		case SDL_JOYAXISMOTION:
		case SDL_JOYHATMOTION:
		case SDL_JOYBUTTONDOWN:
		case SDL_JOYBUTTONUP:
		case SDL_JOYDEVICEADDED:
		case SDL_JOYDEVICEREMOVED:
			// Avoid 'unknown event' spam
			continue;

		case SDL_CONTROLLERAXISMOTION:
			res.evType = SE_JOYSTICK;
			res.evValue = J_AXIS_LEFT_X + (ev.caxis.axis - SDL_CONTROLLER_AXIS_LEFTX);
			res.evValue2 = ev.caxis.value;
			m_gameJoystick.Append(ev.cdevice.which, res.evValue, res.evValue2);
			return res;

		case SDL_CONTROLLERBUTTONDOWN:
		case SDL_CONTROLLERBUTTONUP:
	
			m_gameJoystick.Append(ev.cdevice.which, controllerButtonRemap[ev.cbutton.button][1], ev.cbutton.state == SDL_PRESSED ? 1 : 0);

			res.evType = SE_KEY;
			res.evValue = controllerButtonRemap[ev.cbutton.button][0];
			res.evValue2 = ev.cbutton.state == SDL_PRESSED ? 1 : 0;

			m_gameJoystick.Append(ev.jdevice.which, res.evValue, res.evValue2);
			return res;

		case SDL_QUIT:
			PushConsoleEvent("quit");
			res = no_more_events; // don't handle next event, just quit.
			return res;

		case SDL_USEREVENT:
			switch (ev.user.code)
			{
			case SE_CONSOLE:
				res.evType = SE_CONSOLE;
				res.evPtrLength = (intptr_t)ev.user.data1;
				res.evPtr = ev.user.data2;
				return res;
			default:
				common->Warning("unknown user event %u", ev.user.code);
			}
			continue; // just handle next event
		default:
			common->Warning("unknown event %u", ev.type);
			continue; // just handle next event
		}
	}

	res = no_more_events;
	return res;
}

/*
===========
idSysLocal::ClearEvents
===========
*/
void  idSysLocal::ClearEvents(void)
{
	SDL_Event ev;

	while (SDL_PollEvent(&ev))
		continue;

	m_gameKeyboard.PollKeyboardInputEvents();
	m_gameMouse.PollEndMouseInputEvents();
	m_gameJoystick.EndInputEvents();
}

/*
===========
idSysLocal::PollKeyboardInputEvents
===========
*/
int idSysLocal::PollKeyboardInputEvents(void)
{
	return m_gameKeyboard.PollKeyboardInputEvents();
}

/*
===========
idSysLocal::ReturnKeyboardInputEvent
===========
*/
int idSysLocal::ReturnKeyboardInputEvent(const int n, int& ch, bool& state)
{
	return m_gameKeyboard.ReturnKeyboardInputEvent(n, ch, state);
}

/*
===========
idSysLocal::EndKeyboardInputEvents
===========
*/
void idSysLocal::EndKeyboardInputEvents(void)
{
	m_gameKeyboard.EndKeyboardInputEvents();
}

/*
===========
idSysLocal::SetRumble
===========
*/
void idSysLocal::SetRumble(int device, int low, int hi)
{
	m_gameJoystick.SetRumble(device, low, hi);
}

/*
===========
idSysLocal::PollJoystickInputEvents
===========
*/
int idSysLocal::PollJoystickInputEvents(int deviceNum)
{
	return m_gameJoystick.PollInputEvents(deviceNum);
}

/*
===========
idSysLocal::ReturnJoystickInputEvent
===========
*/
int idSysLocal::ReturnJoystickInputEvent(int deviceNum, const int n, int& action, int& value)
{
	return m_gameJoystick.ReturnInputEvent(deviceNum, n, action, value);
}

/*
===========
idSysLocal::EndJoystickInputEvents
===========
*/
void idSysLocal::EndJoystickInputEvents(void)
{
	m_gameJoystick.EndInputEvents();
}

int idSysLocal::PollMouseInputEvents(int mouseEvents[MAX_MOUSE_EVENTS][2])
{
	return m_gameMouse.PollMouseInputEvents(mouseEvents);
}

/*
===========
Sys_GetClipboardData
get a string from clipbord
===========
*/
char* idSysLocal::GetClipboardData(void)
{
	char* txt = SDL_GetClipboardText();

	if (txt == NULL)
	{
		return NULL;
	}
	else if (txt[0] == '\0')
	{
		SDL_free(txt);
		return NULL;
	}

	char* ret = Mem_CopyString(txt);
	SDL_free(txt);
	return ret;
}

/*
===========
SetClipboardData
copy a string to clipboard
===========
*/
void idSysLocal::SetClipboardData(const char* string)
{
	SDL_SetClipboardText(string);
}

SDL_Window*  idSysLocal::GetWindowHandler(void)
{
	return m_Glimp.GetWindowHandler();
}

/*
==================
idSysLocal::OpenURL
==================
*/
void idSysLocal::OpenURL(const char *url, bool doexit)
{
#if defined(_WIN32)
	Win_OpenURL(url, doexit);
#else
	Posix_OpenURL(url, doexit);
#endif // defined(_WIN32)
}

/*
==================
idSysLocal::StartProcess
==================
*/
void idSysLocal::StartProcess(const char *exePath, bool doexit)
{
#if defined(_WIN32)
	Win_StartProcess(exePath, doexit);
#else
	Posix_StartProcess(exePath, doexit);
#endif // defined(_WIN32)
}

/*
=================
Sys_TimeStampToStr
=================
*/
const char* Sys_TimeStampToStr( ID_TIME_T timeStamp )
{
	static char timeString[MAX_STRING_CHARS];
	timeString[0] = '\0';
	
	time_t ts = ( time_t )timeStamp;
	tm*	time = localtime( &ts );
	if( time == NULL )
	{
		// String separated to prevent detection of trigraphs
		return "??" "/" "??" "/" "???? ??:??";
	}
	
	idStr out;
	
	idStr lang = cvarSystem->GetCVarString( "sys_lang" );
	if( lang.Icmp( ID_LANG_ENGLISH ) == 0 )
	{
		// english gets "month/day/year  hour:min" + "am" or "pm"
		out = va( "%02d", time->tm_mon + 1 );
		out += "/";
		out += va( "%02d", time->tm_mday );
		out += "/";
		out += va( "%d", time->tm_year + 1900 );
		out += " ";	// changed to spaces since flash doesn't recognize \t
		if( time->tm_hour > 12 )
		{
			out += va( "%02d", time->tm_hour - 12 );
		}
		else if( time->tm_hour == 0 )
		{
			out += "12";
		}
		else
		{
			out += va( "%02d", time->tm_hour );
		}
		out += ":";
		out += va( "%02d", time->tm_min );
		if( time->tm_hour >= 12 )
		{
			out += "pm";
		}
		else
		{
			out += "am";
		}
	}
	else
	{
		// europeans get "day/month/year  24hour:min"
		out = va( "%02d", time->tm_mday );
		out += "/";
		out += va( "%02d", time->tm_mon + 1 );
		out += "/";
		out += va( "%d", time->tm_year + 1900 );
		out += " ";	// changed to spaces since flash doesn't recognize \t
		out += va( "%02d", time->tm_hour );
		out += ":";
		out += va( "%02d", time->tm_min );
	}
	idStr::Copynz( timeString, out, sizeof( timeString ) );
	
	return timeString;
}

/*
========================
Sys_SecToStr
========================
*/
const char* Sys_SecToStr( int sec )
{
	static char timeString[MAX_STRING_CHARS];
	
	int weeks = sec / ( 3600 * 24 * 7 );
	sec -= weeks * ( 3600 * 24 * 7 );
	
	int days = sec / ( 3600 * 24 );
	sec -= days * ( 3600 * 24 );
	
	int hours = sec / 3600;
	sec -= hours * 3600;
	
	int min = sec / 60;
	sec -= min * 60;
	
	if( weeks > 0 )
	{
		sprintf( timeString, "%dw, %dd, %d:%02d:%02d", weeks, days, hours, min, sec );
	}
	else if( days > 0 )
	{
		sprintf( timeString, "%dd, %d:%02d:%02d", days, hours, min, sec );
	}
	else
	{
		sprintf( timeString, "%d:%02d:%02d", hours, min, sec );
	}
	
	return timeString;
}

// return number of supported languages
int Sys_NumLangs()
{
	return numLanguages;
}

// get language name by index
const char* Sys_Lang( int idx )
{
	if( idx >= 0 && idx < numLanguages )
	{
		return sysLanguageNames[ idx ];
	}
	return "";
}

void Sys_Launch(const char * path, idCmdArgs & args, void * launchData, unsigned int launchDataSize)
{
	Win_Launch(path, args, launchData, launchDataSize);
}

const char* Sys_DefaultLanguage()
{
	// sku breakdowns are as follows
	//  EFIGS	Digital
	//  EF  S	North America
	//   FIGS	EU
	//  E		UK
	// JE    	Japan
	
	// If japanese exists, default to japanese
	// else if english exists, defaults to english
	// otherwise, french
	
	if( !fileSystem->UsingResourceFiles() )
	{
		return ID_LANG_ENGLISH;
	}
	
	idStr fileName;
	
	//D3XP: Instead of just loading a single lang file for each language
	//we are going to load all files that begin with the language name
	//similar to the way pak files work. So you can place english001.lang
	//to add new strings to the english language dictionary
	idFileList* langFiles;
	langFiles = fileSystem->ListFilesTree( "strings", ".lang", true );
	
	idStrList langList = langFiles->GetList();
	
	// Loop through the list and filter
	idStrList currentLangList = langList;
	
	idStr temp;
	for( int i = 0; i < currentLangList.Num(); i++ )
	{
		temp = currentLangList[i];
		temp = temp.Right( temp.Length() - strlen( "strings/" ) );
		temp = temp.Left( temp.Length() - strlen( ".lang" ) );
		currentLangList[i] = temp;
	}
	
	if( currentLangList.Num() <= 0 )
	{
		// call it English if no lang files exist
		sys_lang.SetString( ID_LANG_ENGLISH );
	}
	else if( currentLangList.Num() == 1 )
	{
		sys_lang.SetString( currentLangList[0] );
	}
	else
	{
		if( currentLangList.Find( ID_LANG_JAPANESE ) )
		{
			sys_lang.SetString( ID_LANG_JAPANESE );
		}
		else if( currentLangList.Find( ID_LANG_ENGLISH ) )
		{
			sys_lang.SetString( ID_LANG_ENGLISH );
		}
		else if( currentLangList.Find( ID_LANG_FRENCH ) )
		{
			sys_lang.SetString( ID_LANG_FRENCH );
		}
		else if( currentLangList.Find( ID_LANG_GERMAN ) )
		{
			sys_lang.SetString( ID_LANG_GERMAN );
		}
		else if( currentLangList.Find( ID_LANG_ITALIAN ) )
		{
			sys_lang.SetString( ID_LANG_GERMAN );
		}
		else if( currentLangList.Find( ID_LANG_SPANISH ) )
		{
			sys_lang.SetString( ID_LANG_GERMAN );
		}
		else
		{
			sys_lang.SetString( currentLangList[0] );
		}
	}
	
	fileSystem->FreeFileList( langFiles );
	
	return sys_lang.GetString();// ID_LANG_ENGLISHS
}

void PushConsoleEvent(const char * s)
{
	char* b;
	size_t len;

	len = strlen(s) + 1;
	b = (char*)Mem_Alloc(len, TAG_EVENTS);
	strcpy(b, s);

	SDL_Event event;

	event.type = SDL_USEREVENT;
	event.user.code = SE_CONSOLE;
	event.user.data1 = (void*)len;
	event.user.data2 = b;

	SDL_PushEvent(&event);
}

void ConvertUTF8toUTF32(const char * utf8str, int32 * utf32buf)
{
	static SDL_iconv_t cd = SDL_iconv_t(-1);

	if (cd == SDL_iconv_t(-1))
	{
		const char* toFormat = "UTF-32LE"; // TODO: what does d3bfg expect on big endian machines?
		cd = SDL_iconv_open(toFormat, "UTF-8");
		if (cd == SDL_iconv_t(-1))
		{
			common->Warning("Couldn't initialize SDL_iconv for UTF-8 to UTF-32!"); // TODO: or error?
			return;
		}
	}

	size_t len = strlen(utf8str);

	size_t inbytesleft = len;
	size_t outbytesleft = 4 * SDL_TEXTINPUTEVENT_TEXT_SIZE; // *4 because utf-32 needs 4x as much space as utf-8
	char* outbuf = (char*)utf32buf;
	size_t n = SDL_iconv(cd, &utf8str, &inbytesleft, &outbuf, &outbytesleft);

	if (n == size_t(-1)) // some error occured during iconv
	{
		common->Warning("Converting UTF-8 string \"%s\" from SDL_TEXTINPUT to UTF-32 failed!", utf8str);

		// clear utf32-buffer, just to be sure there's no garbage..
		memset(utf32buf, 0, SDL_TEXTINPUTEVENT_TEXT_SIZE * sizeof(int32));
	}

	// reset cd so it can be used again
	SDL_iconv(cd, NULL, &inbytesleft, NULL, &outbytesleft);
}

int SDLScanCodeToKeyNum(SDL_Scancode sc)
{
	int idx = int(sc);
	assert(idx >= 0 && idx < SDL_NUM_SCANCODES);

	return scanCodeToKeyNum[idx];
}

SDL_Scancode KeyNumToSDLScanCode(int keyNum)
{
	if (keyNum < K_JOY1)
	{
		for (int i = 0; i < SDL_NUM_SCANCODES; ++i)
		{
			if (scanCodeToKeyNum[i] == keyNum)
			{
				return SDL_Scancode(i);
			}
		}
	}
	return SDL_SCANCODE_UNKNOWN;
}
