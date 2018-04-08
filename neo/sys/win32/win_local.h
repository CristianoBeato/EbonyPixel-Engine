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

#ifndef __WIN_LOCAL_H__
#define __WIN_LOCAL_H__

#include <windows.h>

struct cpuInfo_t {
	int processorPackageCount;
	int processorCoreCount;
	int logicalProcessorCount;
	int numaNodeCount;
	struct cacheInfo_t {
		int count;
		int associativity;
		int lineSize;
		int size;
	} cacheLevel[3];
};

#define TEST_FPU_EXCEPTIONS	/*	FPU_EXCEPTION_INVALID_OPERATION |		*/	\
							/*	FPU_EXCEPTION_DENORMALIZED_OPERAND |	*/	\
							/*	FPU_EXCEPTION_DIVIDE_BY_ZERO |			*/	\
							/*	FPU_EXCEPTION_NUMERIC_OVERFLOW |		*/	\
							/*	FPU_EXCEPTION_NUMERIC_UNDERFLOW |		*/	\
							/*	FPU_EXCEPTION_INEXACT_RESULT |			*/	\
								0

// called first thing. does InitSigs and various things
void	Win_EarlyInit(void);
// called after common has been initialized
void	Win_LateInit(void);
void	Win_Shutdown(void);
void	Win_Quit(void);
void	Win_ReLaunch(void);
void	Win_Launch(const char * path, idCmdArgs & args, void * data, unsigned int dataSize);
bool	Win_Exec(const char * appPath, const char * workingPath, const char * args,
	execProcessWorkFunction_t workFn, execOutputFunction_t outputFn, const int waitMS,
	unsigned int & exitCode);
//void	Win_CreateConsole(void);
//void	Win_DestroyConsole(void);
//void	Win_ShowConsole(int visLevel, bool quitOnClose);
//char*	Win_ConsoleInput(void);
char*	Win_GetCurrentUser(void);
//void	Conbuf_AppendText( const char* msg );

void	Win_OpenURL(const char *url, bool doexit);
void	Win_StartProcess(const char *exePath, bool doexit);
bool	Win_GetCPUInfo(cpuInfo_t & cpuInfo);
#endif /* !__WIN_LOCAL_H__ */
