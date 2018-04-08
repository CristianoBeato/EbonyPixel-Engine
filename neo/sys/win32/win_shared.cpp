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

#pragma hdrstop
#include "precompiled.h"

#include "win_local.h"
#include <lmerr.h>
#include <lmcons.h>
#include <lmwksta.h>
#include <errno.h>
#include <fcntl.h>
#include <direct.h>
#include <io.h>
#include <fcntl.h>
#include <io.h>
#include <conio.h>
#include <mapi.h>
#include <shellapi.h>
#include <shlobj.h>
#include <windowsx.h>

#undef StrCmpN
#undef StrCmpNI
#undef StrCmpI

#ifndef __MRC__
#include <sys/types.h>
#include <sys/stat.h>
#endif
// RB: <DxErr.h> not available on Windows 8 SDK
#if !defined(__MINGW32__)
#include <comdef.h>
#include <comutil.h>
#include <Wbemidl.h>
#include <sal.h>

#pragma warning(disable:4740)	// warning C4740: flow in or out of inline asm code suppresses global optimization
#pragma warning(disable:4731)	// warning C4731: 'XXX' : frame pointer register 'ebx' modified by inline assembly code

// RB: no <atlbase.h> with Visual C++ 2010 Express
#if defined(USE_MFC_TOOLS)
#include <atlbase.h>
#else
#include "win_nanoafx.h"
#endif
#elif !defined(USE_WINRT) // (_WIN32_WINNT < 0x0602 /*_WIN32_WINNT_WIN8*/)
#include <DxErr.h>
#endif // #if !defined(__MINGW32__)
// RB end

#include <ks.h>
#include <ksmedia.h>
#include "sound/snd_local.h"
#include "win_local.h"


#pragma comment (lib, "wbemuuid.lib")

#pragma warning(disable:4740)	// warning C4740: flow in or out of inline asm code suppresses global optimization

typedef struct
{
	HWND			hWnd;
	HINSTANCE		hInstance;

	OSVERSIONINFOEX	osversion;

	// when we get a windows message, we store the time off so keyboard processing
	// can know the exact time of an event (not really needed now that we use async direct input)
	int				sysMsgTime;

	idFileHandle	log_fp;
	CRITICAL_SECTION criticalSections[MAX_CRITICAL_SECTIONS];

} Win32Vars_t;


idCVar Win_arch("sys_arch", "", CVAR_SYSTEM | CVAR_INIT, "");
idCVar Win_cpustring("sys_cpustring", "detect", CVAR_SYSTEM | CVAR_INIT, "");
idCVar Win32_in_mouse("in_mouse", "1", CVAR_SYSTEM | CVAR_BOOL, "enable mouse input");
idCVar win_username("win_username", "", CVAR_SYSTEM | CVAR_INIT, "windows user name");
idCVar win_timerUpdate("win_timerUpdate", "0", CVAR_SYSTEM | CVAR_BOOL, "allows the game to be updated while dragging the window");

static char		sys_cmdline[MAX_STRING_CHARS];

static HANDLE hProcessMutex;
static Win32Vars_t	win32;

/*
================
Sys_GetCurrentUser
================
*/
char* Win_GetCurrentUser()
{
	static char s_userName[1024];
	unsigned long size = sizeof( s_userName );
	
	
	if( !GetUserName( s_userName, &size ) )
	{
		strcpy( s_userName, "player" );
	}
	
	if( !s_userName[0] )
	{
		strcpy( s_userName, "player" );
	}
	
	return s_userName;
}


#pragma optimize( "", on )

#ifdef  defined(_DEBUG) && defined(ID_MCHECK)
static unsigned int debug_total_alloc = 0;
static unsigned int debug_total_alloc_count = 0;
static unsigned int debug_current_alloc = 0;
static unsigned int debug_current_alloc_count = 0;
static unsigned int debug_frame_alloc = 0;
static unsigned int debug_frame_alloc_count = 0;

idCVar sys_showMallocs("sys_showMallocs", "0", CVAR_SYSTEM, "");

// _HOOK_ALLOC, _HOOK_REALLOC, _HOOK_FREE

typedef struct CrtMemBlockHeader
{
	struct _CrtMemBlockHeader *pBlockHeaderNext;	// Pointer to the block allocated just before this one:
	struct _CrtMemBlockHeader *pBlockHeaderPrev;	// Pointer to the block allocated just after this one
	char *szFileName;    // File name
	int nLine;           // Line number
	size_t nDataSize;    // Size of user block
	int nBlockUse;       // Type of block
	long lRequest;       // Allocation number
	byte		gap[4];								// Buffer just before (lower than) the user's memory:
} CrtMemBlockHeader;

#include <crtdbg.h>

/*
==================
Sys_AllocHook

called for every malloc/new/free/delete
==================
*/
int Sys_AllocHook(int nAllocType, void *pvData, size_t nSize, int nBlockUse, long lRequest, const unsigned char * szFileName, int nLine)
{
	CrtMemBlockHeader	*pHead;
	byte				*temp;

	if (nBlockUse == _CRT_BLOCK)
	{
		return(TRUE);
	}

	// get a pointer to memory block header
	temp = (byte *)pvData;
	temp -= 32;
	pHead = (CrtMemBlockHeader *)temp;

	switch (nAllocType) {
	case	_HOOK_ALLOC:
		debug_total_alloc += nSize;
		debug_current_alloc += nSize;
		debug_frame_alloc += nSize;
		debug_total_alloc_count++;
		debug_current_alloc_count++;
		debug_frame_alloc_count++;
		break;

	case	_HOOK_FREE:
		assert(pHead->gap[0] == 0xfd && pHead->gap[1] == 0xfd && pHead->gap[2] == 0xfd && pHead->gap[3] == 0xfd);

		debug_current_alloc -= pHead->nDataSize;
		debug_current_alloc_count--;
		debug_total_alloc_count++;
		debug_frame_alloc_count++;
		break;

	case	_HOOK_REALLOC:
		assert(pHead->gap[0] == 0xfd && pHead->gap[1] == 0xfd && pHead->gap[2] == 0xfd && pHead->gap[3] == 0xfd);

		debug_current_alloc -= pHead->nDataSize;
		debug_total_alloc += nSize;
		debug_current_alloc += nSize;
		debug_frame_alloc += nSize;
		debug_total_alloc_count++;
		debug_current_alloc_count--;
		debug_frame_alloc_count++;
		break;
	}
	return(TRUE);
}

/*
==================
Sys_DebugMemory_f
==================
*/
void Sys_DebugMemory_f() 
{
	common->Printf("Total allocation %8dk in %d blocks\n", debug_total_alloc / 1024, debug_total_alloc_count);
	common->Printf("Current allocation %8dk in %d blocks\n", debug_current_alloc / 1024, debug_current_alloc_count);
}

/*
==================
Sys_MemFrame
==================
*/
void Sys_MemFrame()
{
	if (sys_showMallocs.GetInteger())
	{
		common->Printf("Frame: %8dk in %5d blocks\n", debug_frame_alloc / 1024, debug_frame_alloc_count);
	}

	debug_frame_alloc = 0;
	debug_frame_alloc_count = 0;
}
#endif

/*
========================
Sys_Launch
========================
*/
void Win_Launch(const char * path, idCmdArgs & args, void * data, unsigned int dataSize)
{

	TCHAR				szPathOrig[_MAX_PATH];
	STARTUPINFO			si;
	PROCESS_INFORMATION	pi;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);

	strcpy(szPathOrig, va("\"%s\" %s", sys->EXEPath(), (const char *)data));

	if (!CreateProcess(NULL, szPathOrig, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
		idLib::Error("Could not start process: '%s' ", szPathOrig);
		return;
	}
	cmdSystem->AppendCommandText("quit\n");
}

/*
========================
Sys_GetCmdLine
========================
*/
const char * Win_GetCmdLine()
{
	return sys_cmdline;
}

/*
========================
Sys_ReLaunch
========================
*/
void Win_ReLaunch() 
{
	TCHAR				szPathOrig[MAX_PRINT_MSG];
	STARTUPINFO			si;
	PROCESS_INFORMATION	pi;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);

	// DG: we don't have function arguments in Sys_ReLaunch() anymore, everyone only passed
	//     the command-line +" +set com_skipIntroVideos 1" anyway and it was painful on POSIX systems
	//     so let's just add it here.
	idStr cmdLine = Sys_GetCmdLine();
	if (cmdLine.Find("com_skipIntroVideos") < 0)
		cmdLine.Append(" +set com_skipIntroVideos 1");

	strcpy(szPathOrig, va("\"%s\" %s", sys->EXEPath(), cmdLine.c_str()));
	// DG end

	CloseHandle(hProcessMutex);

	if (!CreateProcess(NULL, szPathOrig, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
		idLib::Error("Could not start process: '%s' ", szPathOrig);
		return;
	}
	cmdSystem->AppendCommandText("quit\n");
}

/*
==============
Sys_Quit
==============
*/
void Win_Quit()
{
	timeEndPeriod(1);
	sys->ShutdownInput();
	//Win_DestroyConsole();
	ExitProcess(0);
}

/*
========================
Sys_Exec

if waitMsec is INFINITE, completely block until the process exits
If waitMsec is -1, don't wait for the process to exit
Other waitMsec values will allow the workFn to be called at those intervals.
========================
*/
bool Win_Exec(const char * appPath, const char * workingPath, const char * args,
	execProcessWorkFunction_t workFn, execOutputFunction_t outputFn, const int waitMS,
	unsigned int & exitCode) {
	exitCode = 0;
	SECURITY_ATTRIBUTES secAttr;
	secAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	secAttr.bInheritHandle = TRUE;
	secAttr.lpSecurityDescriptor = NULL;

	HANDLE hStdOutRead;
	HANDLE hStdOutWrite;
	CreatePipe(&hStdOutRead, &hStdOutWrite, &secAttr, 0);
	SetHandleInformation(hStdOutRead, HANDLE_FLAG_INHERIT, 0);

	HANDLE hStdInRead;
	HANDLE hStdInWrite;
	CreatePipe(&hStdInRead, &hStdInWrite, &secAttr, 0);
	SetHandleInformation(hStdInWrite, HANDLE_FLAG_INHERIT, 0);

	STARTUPINFO si;
	memset(&si, 0, sizeof(si));
	si.cb = sizeof(si);
	si.hStdError = hStdOutWrite;
	si.hStdOutput = hStdOutWrite;
	si.hStdInput = hStdInRead;
	si.wShowWindow = FALSE;
	si.dwFlags |= STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;

	PROCESS_INFORMATION pi;
	memset(&pi, 0, sizeof(pi));

	if (outputFn != NULL)
		outputFn(va("^2Executing Process: ^7%s\n^2working path: ^7%s\n^2args: ^7%s\n", appPath, workingPath, args));
	//else
		//outputFn = ExecOutputFn;

	// we duplicate args here so we can concatenate the exe name and args into a single command line
	const char * imageName = appPath;
	char * cmdLine = NULL;
	{
		// if we have any args, we need to copy them to a new buffer because CreateProcess modifies
		// the command line buffer.
		if (args != NULL) {
			if (appPath != NULL) {
				int len = idStr::Length(args) + idStr::Length(appPath) + 1 /* for space */ + 1 /* for NULL terminator */ + 2 /* app quotes */;
				cmdLine = (char*)Mem_Alloc(len, TAG_TEMP);
				// note that we're putting quotes around the appPath here because when AAS2.exe gets an app path with spaces
				// in the path "w:/zion/build/win32/Debug with Inlines/AAS2.exe" it gets more than one arg for the app name,
				// which it most certainly should not, so I am assuming this is a side effect of using CreateProcess.
				idStr::snPrintf(cmdLine, len, "\"%s\" %s", appPath, args);
			}
			else {
				int len = idStr::Length(args) + 1;
				cmdLine = (char*)Mem_Alloc(len, TAG_TEMP);
				idStr::Copynz(cmdLine, args, len);
			}
			// the image name should always be NULL if we have command line arguments because it is already
			// prefixed to the command line.
			imageName = NULL;
		}
	}

	BOOL result = CreateProcess(imageName, (LPSTR)cmdLine, NULL, NULL, TRUE, 0, NULL, workingPath, &si, &pi);

	if (result == FALSE) {
		TCHAR szBuf[1024];
		LPVOID lpMsgBuf;
		DWORD dw = GetLastError();

		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM,
			NULL,
			dw,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)&lpMsgBuf,
			0, NULL);

		wsprintf(szBuf, "%d: %s", dw, lpMsgBuf);
		if (outputFn != NULL) {
			outputFn(szBuf);
		}
		LocalFree(lpMsgBuf);
		if (cmdLine != NULL) {
			Mem_Free(cmdLine);
		}
		return false;
	}
	else if (waitMS >= 0) {	// if waitMS == -1, don't wait for process to exit
		DWORD ec = 0;
		DWORD wait = 0;
		char buffer[4096];
		for (; ; ) {
			wait = WaitForSingleObject(pi.hProcess, waitMS);
			GetExitCodeProcess(pi.hProcess, &ec);

			DWORD bytesRead = 0;
			DWORD bytesAvail = 0;
			DWORD bytesLeft = 0;
			BOOL ok = PeekNamedPipe(hStdOutRead, NULL, 0, NULL, &bytesAvail, &bytesLeft);
			if (ok && bytesAvail != 0) {
				ok = ReadFile(hStdOutRead, buffer, sizeof(buffer) - 3, &bytesRead, NULL);
				if (ok && bytesRead > 0) {
					buffer[bytesRead] = '\0';
					if (outputFn != NULL) {
						int length = 0;
						for (int i = 0; buffer[i] != '\0'; i++) {
							if (buffer[i] != '\r') {
								buffer[length++] = buffer[i];
							}
						}
						buffer[length++] = '\0';
						outputFn(buffer);
					}
				}
			}

			if (ec != STILL_ACTIVE) {
				exitCode = ec;
				break;
			}

			if (workFn != NULL) {
				if (!workFn()) {
					TerminateProcess(pi.hProcess, 0);
					break;
				}
			}
		}
	}

	// this assumes that windows duplicates the command line string into the created process's
	// environment space.
	if (cmdLine != NULL) {
		Mem_Free(cmdLine);
	}

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	CloseHandle(hStdOutRead);
	CloseHandle(hStdOutWrite);
	CloseHandle(hStdInRead);
	CloseHandle(hStdInWrite);
	return true;
}


/*
================
Sys_AlreadyRunning

returns true if there is a copy of D3 running already
================
*/
bool Win_AlreadyRunning()
{
#ifndef _DEBUG
	if (!win_allowMultipleInstances.GetBool())
	{
		hProcessMutex = ::CreateMutex(NULL, FALSE, "DOOM3");
		if (::GetLastError() == ERROR_ALREADY_EXISTS || ::GetLastError() == ERROR_ACCESS_DENIED)
			return true;
	}
#endif
	return false;
}

/*
================
Sys_Init

The cvar system must already be setup
================
*/
#define OSR2_BUILD_NUMBER 1111
#define WIN98_BUILD_NUMBER 1998

void Sys_win_Init()
{

	CoInitialize(NULL);

	// get WM_TIMER messages pumped every millisecond
	//	SetTimer( NULL, 0, 100, NULL );
	
	// Windows user name
	win_username.SetString(Win_GetCurrentUser());

	//
	// Windows version
	//
	win32.osversion.dwOSVersionInfoSize = sizeof(win32.osversion);

	if (!GetVersionEx((LPOSVERSIONINFO)&win32.osversion))
		Sys_Error("Couldn't get OS info");

	if (win32.osversion.dwMajorVersion < 4)
		Sys_Error(GAME_NAME " requires Windows version 4 (NT) or greater");
	if (win32.osversion.dwPlatformId == VER_PLATFORM_WIN32s)
		Sys_Error(GAME_NAME " doesn't run on Win32s");

	if (win32.osversion.dwPlatformId == VER_PLATFORM_WIN32_NT)
	{
		if (win32.osversion.dwMajorVersion <= 4)
			Win_arch.SetString("WinNT (NT)");
		else if (win32.osversion.dwMajorVersion == 5 && win32.osversion.dwMinorVersion == 0)
			Win_arch.SetString("Win2K (NT)");
		else if (win32.osversion.dwMajorVersion == 5 && win32.osversion.dwMinorVersion == 1)
			Win_arch.SetString("WinXP (NT)");
		else if (win32.osversion.dwMajorVersion == 6)
			Win_arch.SetString("Vista");
		else if (win32.osversion.dwMajorVersion == 6 && win32.osversion.dwMinorVersion == 1)
			Win_arch.SetString("Win7");
		else if (win32.osversion.dwMajorVersion == 6 && win32.osversion.dwMinorVersion == 2)
			Win_arch.SetString("Win8");
		else if (win32.osversion.dwMajorVersion == 6 && win32.osversion.dwMinorVersion == 3)
			Win_arch.SetString("Win8.1");
		else
			Win_arch.SetString("Unknown NT variant");
	}
	else if (win32.osversion.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) 
	{
		if (win32.osversion.dwMajorVersion == 4 && win32.osversion.dwMinorVersion == 0)
		{
			// Win95
			if (win32.osversion.szCSDVersion[1] == 'C')
				Win_arch.SetString("Win95 OSR2 (95)");
			else
				Win_arch.SetString("Win95 (95)");
		}
		else if (win32.osversion.dwMajorVersion == 4 && win32.osversion.dwMinorVersion == 10) 
		{
			// Win98
			if (win32.osversion.szCSDVersion[1] == 'A')
				Win_arch.SetString("Win98SE (95)");
			else
				Win_arch.SetString("Win98 (95)");
		}
		else if (win32.osversion.dwMajorVersion == 4 && win32.osversion.dwMinorVersion == 90) 
		{
			// WinMe
			Win_arch.SetString("WinMe (95)");
		}
		else
			Win_arch.SetString("Unknown 95 variant");
	}
	else
		Win_arch.SetString("unknown Windows variant");
}

/*
================
Sys_Shutdown
================
*/
void Win_Shutdown()
{
	CoUninitialize();
}

//=======================================================================


/*
====================
GetExceptionCodeInfo
====================
*/
const char *GetExceptionCodeInfo(UINT code)
{
	switch (code) {
	case EXCEPTION_ACCESS_VIOLATION: return "The thread tried to read from or write to a virtual address for which it does not have the appropriate access.";
	case EXCEPTION_ARRAY_BOUNDS_EXCEEDED: return "The thread tried to access an array element that is out of bounds and the underlying hardware supports bounds checking.";
	case EXCEPTION_BREAKPOINT: return "A breakpoint was encountered.";
	case EXCEPTION_DATATYPE_MISALIGNMENT: return "The thread tried to read or write data that is misaligned on hardware that does not provide alignment. For example, 16-bit values must be aligned on 2-byte boundaries; 32-bit values on 4-byte boundaries, and so on.";
	case EXCEPTION_FLT_DENORMAL_OPERAND: return "One of the operands in a floating-point operation is denormal. A denormal value is one that is too small to represent as a standard floating-point value.";
	case EXCEPTION_FLT_DIVIDE_BY_ZERO: return "The thread tried to divide a floating-point value by a floating-point divisor of zero.";
	case EXCEPTION_FLT_INEXACT_RESULT: return "The result of a floating-point operation cannot be represented exactly as a decimal fraction.";
	case EXCEPTION_FLT_INVALID_OPERATION: return "This exception represents any floating-point exception not included in this list.";
	case EXCEPTION_FLT_OVERFLOW: return "The exponent of a floating-point operation is greater than the magnitude allowed by the corresponding type.";
	case EXCEPTION_FLT_STACK_CHECK: return "The stack overflowed or underflowed as the result of a floating-point operation.";
	case EXCEPTION_FLT_UNDERFLOW: return "The exponent of a floating-point operation is less than the magnitude allowed by the corresponding type.";
	case EXCEPTION_ILLEGAL_INSTRUCTION: return "The thread tried to execute an invalid instruction.";
	case EXCEPTION_IN_PAGE_ERROR: return "The thread tried to access a page that was not present, and the system was unable to load the page. For example, this exception might occur if a network connection is lost while running a program over the network.";
	case EXCEPTION_INT_DIVIDE_BY_ZERO: return "The thread tried to divide an integer value by an integer divisor of zero.";
	case EXCEPTION_INT_OVERFLOW: return "The result of an integer operation caused a carry out of the most significant bit of the result.";
	case EXCEPTION_INVALID_DISPOSITION: return "An exception handler returned an invalid disposition to the exception dispatcher. Programmers using a high-level language such as C should never encounter this exception.";
	case EXCEPTION_NONCONTINUABLE_EXCEPTION: return "The thread tried to continue execution after a noncontinuable exception occurred.";
	case EXCEPTION_PRIV_INSTRUCTION: return "The thread tried to execute an instruction whose operation is not allowed in the current machine mode.";
	case EXCEPTION_SINGLE_STEP: return "A trace trap or other single-instruction mechanism signaled that one instruction has been executed.";
	case EXCEPTION_STACK_OVERFLOW: return "The thread used up its stack.";
	default: return "Unknown exception";
	}
}

/*
====================
EmailCrashReport

emailer originally from Raven/Quake 4
====================
*/
void EmailCrashReport(LPSTR messageText)
{
	static int lastEmailTime = 0;

	if (sys->Milliseconds() < lastEmailTime + 10000) {
		return;
	}

	lastEmailTime = sys->Milliseconds();

	HINSTANCE mapi = LoadLibrary("MAPI32.DLL");
	if (mapi) {
		LPMAPISENDMAIL	MAPISendMail = (LPMAPISENDMAIL)GetProcAddress(mapi, "MAPISendMail");
		if (MAPISendMail) {
			MapiRecipDesc toProgrammers =
			{
				0,										// ulReserved
				MAPI_TO,							// ulRecipClass
				"DOOM 3 Crash",						// lpszName
				"SMTP:programmers@idsoftware.com",	// lpszAddress
				0,									// ulEIDSize
				0									// lpEntry
			};

			MapiMessage		message = {};
			message.lpszSubject = "DOOM 3 Fatal Error";
			message.lpszNoteText = messageText;
			message.nRecipCount = 1;
			message.lpRecips = &toProgrammers;

			MAPISendMail(
				0,									// LHANDLE lhSession
				0,									// ULONG ulUIParam
				&message,							// lpMapiMessage lpMessage
				MAPI_DIALOG,						// FLAGS flFlags
				0									// ULONG ulReserved
			);
		}
		FreeLibrary(mapi);
	}
}

/*
==================
idSysLocal::OpenURL
==================
*/
void Win_OpenURL(const char *url, bool doexit) 
{
	static bool doexit_spamguard = false;
	HWND wnd;

	if (doexit_spamguard) {
		common->DPrintf("OpenURL: already in an exit sequence, ignoring %s\n", url);
		return;
	}

	common->Printf("Open URL: %s\n", url);

	if (!ShellExecute(NULL, "open", url, NULL, NULL, SW_RESTORE)) {
		common->Error("Could not open url: '%s' ", url);
		return;
	}

	wnd = GetForegroundWindow();
	if (wnd) {
		ShowWindow(wnd, SW_MAXIMIZE);
	}

	if (doexit) {
		doexit_spamguard = true;
		cmdSystem->BufferCommandText(CMD_EXEC_APPEND, "quit\n");
	}
}

/*
==================
Win_StartProcess
==================
*/
void Win_StartProcess(const char *exePath, bool doexit)
{
	TCHAR				szPathOrig[_MAX_PATH];
	STARTUPINFO			si;
	PROCESS_INFORMATION	pi;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);

	strncpy(szPathOrig, exePath, _MAX_PATH);

	if (!CreateProcess(NULL, szPathOrig, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
		common->Error("Could not start process: '%s' ", szPathOrig);
		return;
	}

	if (doexit) 
	{
		cmdSystem->BufferCommandText(CMD_EXEC_APPEND, "quit\n");
	}
}

/*
================
Sys_SetLanguageFromSystem
================
*/
extern idCVar sys_lang;
void Sys_SetLanguageFromSystem()
{
	sys_lang.SetString(Sys_DefaultLanguage());
}

void	Win_EarlyInit(void)
{
	sys->SetPhysicalWorkMemory(192 << 20, 1024 << 20);

	//Sys_GetCurrentMemoryStatus(exeLaunchMemoryStats);

	// done before Com/Sys_Init since we need this for error output
	//Win_CreateConsole();

	// no abort/retry/fail errors
	SetErrorMode(SEM_FAILCRITICALERRORS);

	for (int i = 0; i < MAX_CRITICAL_SECTIONS; i++)
		InitializeCriticalSection(&win32.criticalSections[i]);

	// make sure the timer is high precision, otherwise
	// NT gets 18ms resolution
	timeBeginPeriod(1);

	// get the initial time base
	sys->Milliseconds();

#ifdef DEBUG
	// disable the painfully slow MS heap check every 1024 allocs
	_CrtSetDbgFlag(0);
#endif

	//	Sys_FPU_EnableExceptions( TEST_FPU_EXCEPTIONS );
	sys->FPU_SetPrecision(FPU_PRECISION_DOUBLE_EXTENDED);
}

void	Win_LateInit(void)
{
#if TEST_FPU_EXCEPTIONS != 0
	common->Printf(Sys_FPU_GetState());
#endif

#ifdef SET_THREAD_AFFINITY 
	// give the main thread an affinity for the first cpu
	SetThreadAffinityMask(GetCurrentThread(), 1);
#endif
}

typedef BOOL(WINAPI *LPFN_GLPI)(PSYSTEM_LOGICAL_PROCESSOR_INFORMATION, PDWORD);

enum LOGICAL_PROCESSOR_RELATIONSHIP_LOCAL
{
	localRelationProcessorCore,
	localRelationNumaNode,
	localRelationCache,
	localRelationProcessorPackage
};

/*
========================
CountSetBits
Helper function to count set bits in the processor mask.
========================
*/
DWORD CountSetBits(ULONG_PTR bitMask)
{
	DWORD LSHIFT = sizeof(ULONG_PTR) * 8 - 1;
	DWORD bitSetCount = 0;
	ULONG_PTR bitTest = (ULONG_PTR)1 << LSHIFT;

	for (DWORD i = 0; i <= LSHIFT; i++) {
		bitSetCount += ((bitMask & bitTest) ? 1 : 0);
		bitTest /= 2;
	}

	return bitSetCount;
}

bool	Win_GetCPUInfo(cpuInfo_t & cpuInfo)
{
	PSYSTEM_LOGICAL_PROCESSOR_INFORMATION buffer = NULL;
	PSYSTEM_LOGICAL_PROCESSOR_INFORMATION ptr = NULL;
	PCACHE_DESCRIPTOR Cache;
	LPFN_GLPI	glpi;
	BOOL		done = FALSE;
	DWORD		returnLength = 0;
	DWORD		byteOffset = 0;

	memset(&cpuInfo, 0, sizeof(cpuInfo));

	glpi = (LPFN_GLPI)GetProcAddress(GetModuleHandle(TEXT("kernel32")), "GetLogicalProcessorInformation");
	if (NULL == glpi) {
		idLib::Printf("\nGetLogicalProcessorInformation is not supported.\n");
		return 0;
	}

	while (!done) {
		DWORD rc = glpi(buffer, &returnLength);

		if (FALSE == rc) {
			if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
				if (buffer) {
					free(buffer);
				}

				buffer = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION)malloc(returnLength);
			}
			else {
				idLib::Printf("Sys_CPUCount error: %d\n", GetLastError());
				return false;
			}
		}
		else {
			done = TRUE;
		}
	}

	ptr = buffer;

	while (byteOffset + sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION) <= returnLength) {
		switch ((LOGICAL_PROCESSOR_RELATIONSHIP_LOCAL)ptr->Relationship) {
		case localRelationProcessorCore:
			cpuInfo.processorCoreCount++;

			// A hyperthreaded core supplies more than one logical processor.
			cpuInfo.logicalProcessorCount += CountSetBits(ptr->ProcessorMask);
			break;

		case localRelationNumaNode:
			// Non-NUMA systems report a single record of this type.
			cpuInfo.numaNodeCount++;
			break;

		case localRelationCache:
			// Cache data is in ptr->Cache, one CACHE_DESCRIPTOR structure for each cache. 
			Cache = &ptr->Cache;
			if (Cache->Level >= 1 && Cache->Level <= 3) {
				int level = Cache->Level - 1;
				if (cpuInfo.cacheLevel[level].count > 0) {
					cpuInfo.cacheLevel[level].count++;
				}
				else {
					cpuInfo.cacheLevel[level].associativity = Cache->Associativity;
					cpuInfo.cacheLevel[level].lineSize = Cache->LineSize;
					cpuInfo.cacheLevel[level].size = Cache->Size;
				}
			}
			break;

		case localRelationProcessorPackage:
			// Logical processors share a physical package.
			cpuInfo.processorPackageCount++;
			break;

		default:
			idLib::Printf("Error: Unsupported LOGICAL_PROCESSOR_RELATIONSHIP value.\n");
			break;
		}
		byteOffset += sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
		ptr++;
	}

	free(buffer);

	return true;
}
