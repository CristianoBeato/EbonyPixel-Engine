/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
Copyright (C) 2012 Robert Beckebans
Copyright (C) 2016-2018 Cristiano Beato.

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

#ifndef __SYS_PUBLIC__
#define __SYS_PUBLIC__

#include "idlib/CmdArgs.h"
#include <SDL.h>

struct glimpParms_t;
struct vidMode_t;
class btFileSysCommon;
class btGlimp;

/*
===============================================================================

	Non-portable system services.

===============================================================================
*/

enum cpuid_t
{
	CPUID_NONE							= 0x00000,
	CPUID_UNSUPPORTED					= 0x00001,	// unsupported (386/486)
	CPUID_GENERIC						= 0x00002,	// unrecognized processor
	CPUID_INTEL							= 0x00004,	// Intel
	CPUID_AMD							= 0x00008,	// AMD
	CPUID_MMX							= 0x00010,	// Multi Media Extensions
	CPUID_3DNOW							= 0x00020,	// 3DNow!
	CPUID_SSE							= 0x00040,	// Streaming SIMD Extensions
	CPUID_SSE2							= 0x00080,	// Streaming SIMD Extensions 2
	CPUID_SSE3							= 0x00100,	// Streaming SIMD Extentions 3 aka Prescott's New Instructions
	CPUID_ALTIVEC						= 0x00200,	// AltiVec
	CPUID_HTT							= 0x01000,	// Hyper-Threading Technology
	CPUID_CMOV							= 0x02000,	// Conditional Move (CMOV) and fast floating point comparison (FCOMI) instructions
	CPUID_FTZ							= 0x04000,	// Flush-To-Zero mode (denormal results are flushed to zero)
	CPUID_DAZ							= 0x08000,	// Denormals-Are-Zero mode (denormal source operands are set to zero)
	CPUID_XENON							= 0x10000,	// Xbox 360
	CPUID_CELL							= 0x20000	// PS3
};

enum fpuExceptions_t
{
	FPU_EXCEPTION_INVALID_OPERATION		= 1,
	FPU_EXCEPTION_DENORMALIZED_OPERAND	= 2,
	FPU_EXCEPTION_DIVIDE_BY_ZERO		= 4,
	FPU_EXCEPTION_NUMERIC_OVERFLOW		= 8,
	FPU_EXCEPTION_NUMERIC_UNDERFLOW		= 16,
	FPU_EXCEPTION_INEXACT_RESULT		= 32
};

enum fpuPrecision_t
{
	FPU_PRECISION_SINGLE				= 0,
	FPU_PRECISION_DOUBLE				= 1,
	FPU_PRECISION_DOUBLE_EXTENDED		= 2
};

enum fpuRounding_t
{
	FPU_ROUNDING_TO_NEAREST				= 0,
	FPU_ROUNDING_DOWN					= 1,
	FPU_ROUNDING_UP						= 2,
	FPU_ROUNDING_TO_ZERO				= 3
};

enum joystickAxis_t
{
	AXIS_LEFT_X,
	AXIS_LEFT_Y,
	AXIS_RIGHT_X,
	AXIS_RIGHT_Y,
	AXIS_LEFT_TRIG,
	AXIS_RIGHT_TRIG,
	MAX_JOYSTICK_AXIS
};

enum sysEventType_t
{
	SE_NONE,				// evTime is still valid
	SE_KEY,					// evValue is a key code, evValue2 is the down flag
	SE_CHAR,				// evValue is an Unicode UTF-32 char (or non-surrogate UTF-16)
	SE_MOUSE,				// evValue and evValue2 are relative signed x / y moves
	SE_MOUSE_ABSOLUTE,		// evValue and evValue2 are absolute coordinates in the window's client area.
	SE_MOUSE_LEAVE,			// evValue and evValue2 are meaninless, this indicates the mouse has left the client area.
	SE_JOYSTICK,			// evValue is an axis number and evValue2 is the current state (-127 to 127)
	SE_CONSOLE				// evPtr is a char*, from typing something at a non-game console
};

enum sys_mEvents
{
	M_ACTION1,
	M_ACTION2,
	M_ACTION3,
	M_ACTION4,
	M_ACTION5,
	M_ACTION6,
	M_ACTION7,
	M_ACTION8,
	// DG: support some more mouse buttons
	M_ACTION9,
	M_ACTION10,
	M_ACTION11,
	M_ACTION12,
	M_ACTION13,
	M_ACTION14,
	M_ACTION15,
	M_ACTION16,
	// DG end
	M_DELTAX,
	M_DELTAY,
	M_DELTAZ,
	M_INVALID
};

enum sys_jEvents
{
	J_ACTION1,
	J_ACTION2,
	J_ACTION3,
	J_ACTION4,
	J_ACTION5,
	J_ACTION6,
	J_ACTION7,
	J_ACTION8,
	J_ACTION9,
	J_ACTION10,
	J_ACTION11,
	J_ACTION12,
	J_ACTION13,
	J_ACTION14,
	J_ACTION15,
	J_ACTION16,
	J_ACTION17,
	J_ACTION18,
	J_ACTION19,
	J_ACTION20,
	J_ACTION21,
	J_ACTION22,
	J_ACTION23,
	J_ACTION24,
	J_ACTION25,
	J_ACTION26,
	J_ACTION27,
	J_ACTION28,
	J_ACTION29,
	J_ACTION30,
	J_ACTION31,
	J_ACTION32,
	J_ACTION_MAX = J_ACTION32,
	
	J_AXIS_MIN,
	J_AXIS_LEFT_X = J_AXIS_MIN + AXIS_LEFT_X,
	J_AXIS_LEFT_Y = J_AXIS_MIN + AXIS_LEFT_Y,
	J_AXIS_RIGHT_X = J_AXIS_MIN + AXIS_RIGHT_X,
	J_AXIS_RIGHT_Y = J_AXIS_MIN + AXIS_RIGHT_Y,
	J_AXIS_LEFT_TRIG = J_AXIS_MIN + AXIS_LEFT_TRIG,
	J_AXIS_RIGHT_TRIG = J_AXIS_MIN + AXIS_RIGHT_TRIG,
	
	J_AXIS_MAX = J_AXIS_MIN + MAX_JOYSTICK_AXIS - 1,
	
	J_DPAD_UP,
	J_DPAD_DOWN,
	J_DPAD_LEFT,
	J_DPAD_RIGHT,
	
	MAX_JOY_EVENT
};

/*
================================================
The first part of this table maps directly to Direct Input scan codes (DIK_* from dinput.h)
But they are duplicated here for console portability
================================================
*/
enum keyNum_t
{
	K_NONE,
	
	K_ESCAPE,
	K_1,
	K_2,
	K_3,
	K_4,
	K_5,
	K_6,
	K_7,
	K_8,
	K_9,
	K_0,
	K_MINUS,
	K_EQUALS,
	K_BACKSPACE,
	K_TAB,
	K_Q,
	K_W,
	K_E,
	K_R,
	K_T,
	K_Y,
	K_U,
	K_I,
	K_O,
	K_P,
	K_LBRACKET,
	K_RBRACKET,
	K_ENTER,
	K_LCTRL,
	K_A,
	K_S,
	K_D,
	K_F,
	K_G,
	K_H,
	K_J,
	K_K,
	K_L,
	K_SEMICOLON,
	K_APOSTROPHE,
	K_GRAVE,
	K_LSHIFT,
	K_BACKSLASH,
	K_Z,
	K_X,
	K_C,
	K_V,
	K_B,
	K_N,
	K_M,
	K_COMMA,
	K_PERIOD,
	K_SLASH,
	K_RSHIFT,
	K_KP_STAR,
	K_LALT,
	K_SPACE,
	K_CAPSLOCK,
	K_F1,
	K_F2,
	K_F3,
	K_F4,
	K_F5,
	K_F6,
	K_F7,
	K_F8,
	K_F9,
	K_F10,
	K_NUMLOCK,
	K_SCROLL,
	K_KP_7,
	K_KP_8,
	K_KP_9,
	K_KP_MINUS,
	K_KP_4,
	K_KP_5,
	K_KP_6,
	K_KP_PLUS,
	K_KP_1,
	K_KP_2,
	K_KP_3,
	K_KP_0,
	K_KP_DOT,
	K_OEM_102		= 0x56, // from dinput: < > | on UK/German keyboards
	K_F11			= 0x57,
	K_F12			= 0x58,
	K_F13			= 0x64,
	K_F14			= 0x65,
	K_F15			= 0x66,
	K_KANA			= 0x70,
	K_ABNT_C1		= 0x7E, // from dinput: ? on Portugese (Brazilian) keyboards
	K_CONVERT		= 0x79,
	K_NOCONVERT		= 0x7B,
	K_YEN			= 0x7D,
	K_KP_EQUALS		= 0x8D,
	K_CIRCUMFLEX	= 0x90, // this is circumflex on japanese keyboards, ..
	K_PREVTRACK		= 0x90, // from dinput: .. but also "Previous Track"
	K_AT			= 0x91,
	K_COLON			= 0x92,
	K_UNDERLINE		= 0x93,
	K_KANJI			= 0x94,
	K_STOP			= 0x95,
	K_AX			= 0x96,
	K_UNLABELED		= 0x97,
	K_NEXTTRACK		= 0x99, // from dinput
	K_KP_ENTER		= 0x9C,
	K_RCTRL			= 0x9D,
	// some more from dinput:
	K_MUTE          = 0xA0,
	K_CALCULATOR    = 0xA1,
	K_PLAYPAUSE     = 0xA2,
	K_MEDIASTOP     = 0xA4,
	K_VOLUMEDOWN    = 0xAE,
	K_VOLUMEUP      = 0xB0,
	K_WEBHOME       = 0xB2,
	
	K_KP_COMMA		= 0xB3,
	K_KP_SLASH		= 0xB5,
	K_PRINTSCREEN	= 0xB7, // aka SysRq
	K_RALT			= 0xB8,
	K_PAUSE			= 0xC5,
	K_HOME			= 0xC7,
	K_UPARROW		= 0xC8,
	K_PGUP			= 0xC9,
	K_LEFTARROW		= 0xCB,
	K_RIGHTARROW	= 0xCD,
	K_END			= 0xCF,
	K_DOWNARROW		= 0xD0,
	K_PGDN			= 0xD1,
	K_INS			= 0xD2,
	K_DEL			= 0xD3,
	K_LWIN			= 0xDB,
	K_RWIN			= 0xDC,
	K_APPS			= 0xDD,
	K_POWER			= 0xDE,
	K_SLEEP			= 0xDF,
	
	// DG: dinput has some more buttons, let's support them as well
	K_WAKE			= 0xE3,
	K_WEBSEARCH		= 0xE5,
	K_WEBFAVORITES	= 0xE6,
	K_WEBREFRESH	= 0xE7,
	K_WEBSTOP		= 0xE8,
	K_WEBFORWARD	= 0xE9,
	K_WEBBACK		= 0xEA,
	K_MYCOMPUTER	= 0xEB,
	K_MAIL			= 0xEC,
	K_MEDIASELECT	= 0xED,
	
	//------------------------
	// K_JOY codes must be contiguous, too
	//------------------------
	
	K_JOY1 = 256,
	K_JOY2,
	K_JOY3,
	K_JOY4,
	K_JOY5,
	K_JOY6,
	K_JOY7,
	K_JOY8,
	K_JOY9,
	K_JOY10,
	K_JOY11,
	K_JOY12,
	K_JOY13,
	K_JOY14,
	K_JOY15,
	K_JOY16,
	
	K_JOY_STICK1_UP,
	K_JOY_STICK1_DOWN,
	K_JOY_STICK1_LEFT,
	K_JOY_STICK1_RIGHT,
	
	K_JOY_STICK2_UP,
	K_JOY_STICK2_DOWN,
	K_JOY_STICK2_LEFT,
	K_JOY_STICK2_RIGHT,
	
	K_JOY_TRIGGER1,
	K_JOY_TRIGGER2,
	
	K_JOY_DPAD_UP,
	K_JOY_DPAD_DOWN,
	K_JOY_DPAD_LEFT,
	K_JOY_DPAD_RIGHT,
	
	//------------------------
	// K_MOUSE enums must be contiguous (no char codes in the middle)
	//------------------------
	
	K_MOUSE1,
	K_MOUSE2,
	K_MOUSE3,
	K_MOUSE4,
	K_MOUSE5,
	K_MOUSE6,
	K_MOUSE7,
	K_MOUSE8,
	
	// DG: add some more mouse buttons
	K_MOUSE9,
	K_MOUSE10,
	K_MOUSE11,
	K_MOUSE12,
	K_MOUSE13,
	K_MOUSE14,
	K_MOUSE15,
	K_MOUSE16,
	// DG end
	
	K_MWHEELDOWN,
	K_MWHEELUP,
	
	K_LAST_KEY
};

enum sysFolder_t
{
	FOLDER_ERROR = -1,
	FOLDER_NO = 0,
	FOLDER_YES = 1
};

struct sysEvent_t
{
	sysEventType_t	evType;
	int				evValue;
	int				evValue2;
	int				evPtrLength;		// bytes of data pointed to by evPtr, for journaling
	void* 			evPtr;				// this must be manually freed if not NULL
	
	int				inputDevice;
	bool			IsKeyEvent() const
	{
		return evType == SE_KEY;
	}
	bool			IsMouseEvent() const
	{
		return evType == SE_MOUSE;
	}
	bool			IsCharEvent() const
	{
		return evType == SE_CHAR;
	}
	bool			IsJoystickEvent() const
	{
		return evType == SE_JOYSTICK;
	}
	bool			IsKeyDown() const
	{
		return evValue2 != 0;
	}
	keyNum_t		GetKey() const
	{
		return static_cast< keyNum_t >( evValue );
	}
	int				GetXCoord() const
	{
		return evValue;
	}
	int				GetYCoord() const
	{
		return evValue2;
	}
};

struct sysMemoryStats_t
{
	int memoryLoad;
	int totalPhysical;
	int availPhysical;
	int totalPageFile;
	int availPageFile;
	int totalVirtual;
	int availVirtual;
	int availExtendedVirtual;
};

// typedef unsigned long address_t; // DG: this isn't even used
void			Sys_Init(void);
void			Sys_Shutdown(void);
void			Sys_Quit(void);
void			Sys_Error( const char* error, ... );
const char* 	Sys_GetCmdLine();
// DG: Sys_ReLaunch() doesn't need any options (and the old way is painful for POSIX systems)
void			Sys_ReLaunch();
// DG end

void			Sys_Launch( const char* path, idCmdArgs& args,  void* launchData, unsigned int launchDataSize );
void			Sys_SetLanguageFromSystem();
const char* 	Sys_DefaultLanguage();
bool			Sys_AlreadyRunning(void);
//void			Sys_ShowConsole( int visLevel, bool quitOnClose );
void			Sys_SetErrorText(const char* buf);
// will go to the various text consoles
// NOT thread safe - never use in the async paths
void			Sys_Printf( VERIFY_FORMAT_STRING const char* msg, ... );
// guaranteed to be thread-safe
void			Sys_DebugPrintf( VERIFY_FORMAT_STRING const char* fmt, ... );
void			Sys_DebugVPrintf( const char* fmt, va_list arg );
// know early if we are performing a fatal error shutdown so the error message doesn't get lost
void			Sys_SetFatalError(const char* error);
const char*		Sys_ConsoleInput(void);
// a decent minimum sleep time to avoid going below the process scheduler speeds
#define			SYS_MINSLEEP	20

static const int MAX_MOUSE_EVENTS = 256;

// This really isn't the right place to have this, but since this is the 'top level' include
// and has a function signature with 'FILE' in it, it kinda needs to be here =/
// RB begin
#if defined(_WIN32)
typedef HANDLE idFileHandle;
#else
typedef FILE* idFileHandle;
#endif
// RB end

// NOTE: do we need to guarantee the same output on all platforms?
const char* 	Sys_TimeStampToStr( ID_TIME_T timeStamp );
const char* 	Sys_SecToStr( int sec );

// Execute the specified process and wait until it's done, calling workFn every waitMS milliseconds.
// If showOutput == true, std IO from the executed process will be output to the console.
// Note that the return value is not an indication of the exit code of the process, but is false
// only if the process could not be created at all. If you wish to check the exit code of the
// spawned process, check the value returned in exitCode.
typedef bool ( *execProcessWorkFunction_t )();
typedef void ( *execOutputFunction_t )( const char* text );
bool Sys_Exec(	const char* appPath, const char* workingPath, const char* args,
				execProcessWorkFunction_t workFn, execOutputFunction_t outputFn, const int waitMS,
				unsigned int& exitCode );

// localization

#define ID_LANG_ENGLISH		"english"
#define ID_LANG_FRENCH		"french"
#define ID_LANG_ITALIAN		"italian"
#define ID_LANG_GERMAN		"german"
#define ID_LANG_SPANISH		"spanish"
#define ID_LANG_JAPANESE	"japanese"
#define ID_LANG_PORTUGUESE  "portuguese"

int Sys_NumLangs();
const char* Sys_Lang( int idx );

/*
==============================================================

	Networking

==============================================================
*/

typedef enum
{
	NA_BAD,					// an address lookup failed
	NA_LOOPBACK,
	NA_BROADCAST,
	NA_IP
} netadrtype_t;

typedef struct
{
	netadrtype_t	type;
	unsigned char	ip[4];
	unsigned short	port;
} netadr_t;

#define	PORT_ANY			-1

/*
================================================
idUDP
================================================
*/
class idUDP
{
public:
	// this just zeros netSocket and port
	idUDP();
	virtual		~idUDP();
	
	// if the InitForPort fails, the idUDP.port field will remain 0
	bool		InitForPort( int portNumber );
	
	int			GetPort() const
	{
		return bound_to.port;
	}
	netadr_t	GetAdr() const
	{
		return bound_to;
	}
	uint32		GetUIntAdr() const
	{
		return ( bound_to.ip[0] | bound_to.ip[1] << 8 | bound_to.ip[2] << 16 | bound_to.ip[3] << 24 );
	}
	void		Close();
	
	bool		GetPacket( netadr_t& from, void* data, int& size, int maxSize );
	
	bool		GetPacketBlocking( netadr_t& from, void* data, int& size, int maxSize,
								   int timeout );
								   
	void		SendPacket( const netadr_t to, const void* data, int size );
	
	void		SetSilent( bool silent )
	{
		this->silent = silent;
	}
	bool		GetSilent() const
	{
		return silent;
	}
	
	int			packetsRead;
	int			bytesRead;
	
	int			packetsWritten;
	int			bytesWritten;
	
	bool		IsOpen() const
	{
		return netSocket > 0;
	}
	
private:
	netadr_t	bound_to;		// interface and port
	int			netSocket;		// OS specific socket
	bool		silent;			// don't emit anything ( black hole )
};



// parses the port number
// can also do DNS resolve if you ask for it.
// NOTE: DNS resolve is a slow/blocking call, think before you use
// ( could be exploited for server DoS )
bool			Sys_StringToNetAdr( const char* s, netadr_t* a, bool doDNSResolve );
const char* 	Sys_NetAdrToString( const netadr_t a );
bool			Sys_IsLANAddress( const netadr_t a );
bool			Sys_CompareNetAdrBase( const netadr_t a, const netadr_t b );

int				Sys_GetLocalIPCount();
const char* 	Sys_GetLocalIP( int i );

void			Sys_InitNetworking();
void			Sys_ShutdownNetworking();



/*
================================================
idJoystick is managed by each platform's local Sys implementation, and
provides full *Joy Pad* support (the most common device, these days).
================================================
*/
class idJoystick
{
public:
	idJoystick(void) {}
	virtual			~idJoystick(void) { }
	
	virtual bool	Init(void)
	{
		return false;
	}
	virtual void	Shutdown(void) { }
	virtual void	Deactivate(void) { }
	virtual void	SetRumble( int deviceNum, int rumbleLow, int rumbleHigh ) { }
	virtual int		PollInputEvents( int inputDeviceNum )
	{
		return 0;
	}

	virtual int		ReturnInputEvent( const int n, int& action, int& value )
	{
		return 0;
	}

	virtual void	EndInputEvents() { }
};


// Beato Begin

class btMessageBox
{
public:
	btMessageBox(void)
	{
		msgdata = nullptr;
		button = -1;
	}

	btMessageBox(const SDL_MessageBoxFlags flag, const char *title, const char *message)
	{
		SDL_MessageBoxButtonData *buttons;
		int numButtons = 1;
		button = -1;
		success = 0;

		//SDL_malloc is more fast than new
		msgdata = (SDL_MessageBoxData*)SDL_malloc(sizeof(SDL_MessageBoxData));

		msgdata->flags = flag;
		msgdata->title = title;
		msgdata->message = message;
		msgdata->window = NULL; //set null be defatlt

		if (flag != SDL_MESSAGEBOX_INFORMATION)
		{
			numButtons = 2;
			buttons = new SDL_MessageBoxButtonData[2];
			buttons[0] =
			{
				SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT,
				0,
				"OK"
			};

			buttons[1] =
			{
				SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT,
				1,
				"Cancel"
			};
		}
		else
		{
			buttons = new SDL_MessageBoxButtonData
			{
				SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT,
				0,
				"OK"
			};
		}
		msgdata->numbuttons = numButtons;
		msgdata->buttons = buttons;
	}

	~btMessageBox(void)
	{
		Clear();
	}

	void	Clear(void)
	{
		if (msgdata != nullptr)
		{
			SDL_free(msgdata);
			msgdata = nullptr;
		}
	}

	void	SetParent(SDL_Window *window)
	{
		this->msgdata->window = window;
	}

	void	Show(void)
	{
		SDL_assert(msgdata != nullptr);
		success = SDL_ShowMessageBox(msgdata, &button);
	}

private:
	int					button;
	int					success;
	SDL_MessageBoxData	*msgdata;
};
// Beato End

/*
==============================================================

	idSys

==============================================================
*/

class idSys
{
public:

	//Beato: portable functions are now inside system hadler
	virtual void			DebugPrintf( VERIFY_FORMAT_STRING const char* fmt, ... ) = 0;
	virtual void			DebugVPrintf( const char* fmt, va_list arg ) = 0;
	
	// allow game to yield CPU time
	// NOTE: due to SYS_MINSLEEP this is very bad portability karma, and should be completely removed
	virtual void			Sleep(int msec) =0;

	// Sys_Milliseconds should only be used for profiling purposes,
	// any game related timing information should come from event timestamps
	virtual int				Milliseconds() =0;
	virtual uint64			Microseconds() =0;

	// for accurate performance testing
	virtual double			GetClockTicks() = 0;
	virtual double			ClockTicksPerSecond() = 0;

	virtual void			CPUCount(int& numLogicalCPUCores, int& numPhysicalCPUCores, int& numCPUPackages) = 0;
	virtual cpuid_t			GetProcessorId() = 0;
	virtual const char* 	GetProcessorString() = 0;

	virtual const char* 	FPU_GetState() = 0;
	virtual bool			FPU_StackIsEmpty() = 0;
	virtual void			FPU_SetFTZ( bool enable ) = 0;
	virtual void			FPU_SetDAZ( bool enable ) = 0;
	virtual void			FPU_EnableExceptions( int exceptions ) = 0;
	virtual void			FPU_SetPrecision(int precision) = 0;


	// returns amount of drive space in path
	virtual int				GetDriveFreeSpace(const char* path) = 0;

	// returns amount of drive space in path in bytes
	virtual int64			GetDriveFreeSpaceInBytes(const char* path) = 0;
	virtual ID_TIME_T		FileTimeStamp(idFileHandle fp) = 0;
	virtual void			Mkdir(const char* path) = 0;
	virtual bool			Rmdir(const char* path) = 0;
	virtual bool			IsFileWritable(const char* path) = 0;
	// returns FOLDER_YES if the specified path is a folder
	virtual sysFolder_t		IsFolder(const char* path) = 0;
#if 0
	// use fs_debug to verbose Sys_ListFiles
	// returns -1 if directory was not found (the list is cleared)
	virtual int				ListFiles(const char* directory, const char* extension, idList<class idStr>& list) = 0;
#endif // 0
	virtual btFileSysCommon GetFSHandler(void) = 0;
	virtual const char* 	EXEPath(void) = 0;
	virtual const char* 	CWD(void) = 0;
	virtual const char* 	LaunchPath(void) = 0;
	virtual const char* 	DefaultBasePath(void) = 0;
	virtual const char* 	DefaultSavePath(void) = 0;

	virtual void			SetPhysicalWorkMemory(int minBytes, int maxBytes) = 0;
	virtual bool			LockMemory( void* ptr, int bytes ) = 0;
	virtual bool			UnlockMemory( void* ptr, int bytes ) = 0;
	
#if 0
	virtual void*			DLL_Load( const char* dllName ) = 0;
	virtual void* 			DLL_GetProcAddress(void* dllHandle, const char* procName ) = 0;
	virtual void			DLL_Unload(void* dllHandle ) = 0;
	virtual void			DLL_GetFileName( const char* baseName, char* dllName, int maxLength ) = 0;
#else
	virtual void*			DLL_Load(const char* dllName) { return NULL; };
	virtual void* 			DLL_GetProcAddress(void* dllHandle, const char* procName) { return NULL; };
	virtual void			DLL_Unload(void* dllHandle) {};
	virtual void			DLL_GetFileName(const char* baseName, char* dllName, int maxLength) {};
#endif
	
	// when the console is down, or the game is about to perform a lengthy
	// operation like map loading, the system can release the mouse cursor
	// when in windowed mode
	virtual void			GrabMouseCursor(bool grabIt) =0;

	virtual void			ShowWindow(bool show) =0;
	virtual bool			IsWindowVisible(void) =0;

#if 0
	// the number of displays can be found by itterating this until it returns false
	// displayNum is the 0 based value passed to EnumDisplayDevices(), you must add
	// 1 to this to get an r_fullScreen value.
	virtual bool			GetModeListForDisplay(const int displayNum, idList<vidMode_t>& modeList) = 0;
#endif  
	virtual btGlimp			GetGlimpHandle(void) = 0;

	// DG: R_GetModeListForDisplay is called before GLimp_Init(), but SDL needs SDL_Init() first.
	// So add PreInit for platforms that need it, others can just stub it.
	virtual void			GLimpPreInit(void) = 0;

	// If the desired mode can't be set satisfactorily, false will be returned.
	// If succesful, sets glConfig.nativeScreenWidth, glConfig.nativeScreenHeight, and glConfig.pixelAspect
	// The renderer will then reset the glimpParms to "safe mode" of 640x480
	// fullscreen and try again.  If that also fails, the error will be fatal.
	virtual bool			GLimpInit(glimpParms_t parms) = 0;

	// will set up gl up with the new parms
	virtual bool			GLimpSetScreenParms(glimpParms_t parms) = 0;

	// Destroys the rendering context, closes the window, resets the resolution,
	// and resets the gamma ramps.
	virtual void			GLimpShutdown(void) = 0;

	// Sets the hardware gamma ramps for gamma and brightness adjustment.
	// These are now taken as 16 bit values, so we can take full advantage
	// of dacs with >8 bits of precision
	virtual void			GLimpSetGamma(unsigned short red[256], unsigned short green[256], unsigned short blue[256]) = 0;

	virtual void			GLimpSwapBuffers(void) = 0;
	// input is tied to windows, so it needs to be started up and shut down whenever
	// the main window is recreated
	virtual void			InitInput(void) =0;
	virtual void			ShutdownInput(void) =0;

	// event generation
	virtual void			GenerateEvents(void) = 0;
	virtual sysEvent_t		GetEvent(void) = 0;
	virtual void			ClearEvents(void) = 0;

	// keyboard input polling
	virtual int				PollKeyboardInputEvents(void) = 0;
	virtual int				ReturnKeyboardInputEvent(const int n, int& ch, bool& state) = 0;
	virtual void			EndKeyboardInputEvents(void) = 0;

	// joystick input polling
	virtual void			SetRumble(int device, int low, int hi) = 0;
	virtual int				PollJoystickInputEvents(int deviceNum) = 0;
	virtual int				ReturnJoystickInputEvent(int deviceNum, const int n, int& action, int& value) = 0;
	virtual void			EndJoystickInputEvents(void) = 0;

	// mouse input polling
	virtual int				PollMouseInputEvents(int mouseEvents[MAX_MOUSE_EVENTS][2]) = 0;

	virtual sysEvent_t		GenerateMouseButtonEvent( int button, bool down ) = 0;
	virtual sysEvent_t		GenerateMouseMoveEvent( int deltax, int deltay ) = 0;
	
	virtual void			OpenURL( const char* url, bool quit ) = 0;
	virtual void			StartProcess( const char* exePath, bool quit ) = 0;

	// note that this isn't journaled...
	virtual char* 			GetClipboardData(void) =0;
	virtual void			SetClipboardData(const char* string) =0;

	virtual SDL_Window*		GetWindowHandler(void) = 0;

	//beato end

};

extern idSys* 				sys;

bool Sys_LoadOpenAL();
void Sys_FreeOpenAL();


#endif /* !__SYS_PUBLIC__ */
