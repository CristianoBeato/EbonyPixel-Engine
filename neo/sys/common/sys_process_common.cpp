/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
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

#include "precompiled.h"
#pragma hdrstop

#include "sys_process_common.h"

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#include "sys/win32/win_local.h"
#endif

typedef intptr_t(*fntype) (const char *);


/*
===============================================================================
btMemoryStatus
===============================================================================
*/
void btMemoryStatus::GetCurrentMemoryStatus(sysMemoryStats_t& stats)
{
	common->Printf("FIXME: GetCurrentMemoryStatus stub\n");
}

void btMemoryStatus::GetExeLaunchMemoryStatus(sysMemoryStats_t & stats)
{
	common->Printf("FIXME: GetExeLaunchMemoryStatus stub\n");
}

bool btMemoryStatus::LockMemory(void * ptr, int bytes)
{
	return true;
}

bool btMemoryStatus::UnlockMemory(void * ptr, int bytes)
{
	return true;
}

void btMemoryStatus::SetPhysicalWorkMemory(int minBytes, int maxBytes)
{
#if defined(_WIN32)
	::SetProcessWorkingSetSize(GetCurrentProcess(), minBytes, maxBytes);
#else
	common->DPrintf("TODO: Sys_SetPhysicalWorkMemory\n");
#endif
}

/*
==============================================================
btProcess
==============================================================
*/
void* btProcess::DLL_Load(const char * path)
{
	void *libHandle = nullptr;
	libHandle = SDL_LoadObject(path);
	if (libHandle != nullptr)
		return libHandle;
	else
		common->Error("SDL_LoadObject('%s') failed: %s\n", path, SDL_GetError());

	return nullptr;
}

void * btProcess::DLL_GetProcAddress(void* handle, const char * sym)
{
	fntype fn = nullptr;
	fn = (fntype)SDL_LoadFunction(handle, sym);
	if (fn == nullptr)
		return	fn;
	else
		common->Error("SDL_LoadFunction('%s') failed: %s\n", sym, SDL_GetError());

	return nullptr;
}

void btProcess::DLL_Unload(void* handle)
{
	SDL_UnloadObject(handle);
}
