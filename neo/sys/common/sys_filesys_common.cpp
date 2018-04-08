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

#include "sys_filesys_common.h"

#ifdef _WIN32
#include <direct.h>
#include <io.h>
#else
#include <dirent.h>
#include <pwd.h>
#include <unistd.h>
#endif // _WIN32

#include <errno.h>

const char * btFileSysCommon::DefaultBasePath(void)
{
	if (basePath.IsEmpty())
	{
#if defined(_WIN32) || defined(_WIN64)
		basePath =  Sys_Cwd();
#else
		struct stat st;
		basePath = Sys_EXEPath();
		if (basePath.Length())
		{
			basePath.StripFilename();
			testbase = basePath;
			testbase += "/";
			testbase += BASE_GAMEDIR;
			if (stat(testbase.c_str(), &st) != -1 && S_ISDIR(st.st_mode))
			{
				return basePath.c_str();
			}
			else
			{
				common->Printf("no '%s' directory in exe path %s, skipping\n", BASE_GAMEDIR, basePath.c_str());
			}
		}
		if (basePath != Sys_Cwd())
		{
			basePath = Sys_Cwd();
			testbase = basePath;
			testbase += "/";
			testbase += BASE_GAMEDIR;
			if (stat(testbase.c_str(), &st) != -1 && S_ISDIR(st.st_mode))
			{
				return basePath.c_str();
			}
			else
			{
				common->Printf("no '%s' directory in cwd path %s, skipping\n", BASE_GAMEDIR, BasePath.c_str());
			}
		}
		common->Printf("WARNING: using hardcoded default base path %s\n", DEFAULT_BASEPATH);
		return DEFAULT_BASEPATH;
#endif
		basePath.BackSlashesToSlashes();
	}
	return basePath.c_str();
}

const char * btFileSysCommon::DefaultSavePath(void)
{
//Beato: uses getenv() on linux, because sdl set in "/home/user name/.local/share/SAVE_PATH/"
#if defined(__linux__)
	sprintf(savePath, "%s/.rbdoom3bfg", getenv("HOME"));
#else
	if (savePath.IsEmpty())
	{
		char* base_path = SDL_GetPrefPath(GAME_PRODUCER, SAVE_PATH);
		if (base_path)
		{
			savePath = SDL_strdup(base_path);
			SDL_free(base_path);
		}
		else
			savePath = SAVE_PATH;

		savePath.BackSlashesToSlashes();
	}
#endif
	return savePath.c_str();
}

const char * btFileSysCommon::EXEPath(void)
{
	if(exePath.IsEmpty())
		exePath = SDL_GetBasePath();

	return exePath.c_str();
}

void btFileSysCommon::Mkdir(const char * path)
{
#ifdef _WIN32
	_mkdir(path);
#else
	mkdir(path, 0777);
#endif // _WIN32
}

bool btFileSysCommon::Rmdir(const char * path)
{
#ifdef _WIN32
	return _rmdir(path) == 0;
#else
	return (rmdir(path) == 0);
#endif
}

const char * btFileSysCommon::Sys_Cwd(void)
{
	static char cwd[MAX_OSPATH];
#ifdef _WIN32
	_getcwd(cwd, sizeof(cwd) - 1);
#else
	getcwd(cwd, sizeof(cwd) - 1);
#endif
	cwd[MAX_OSPATH - 1] = 0;
	return cwd;
}

sysFolder_t btFileSysCommon::IsFolder(const char * path)
{
//Stupid Microsoft, for what ?
#ifdef _WIN32
	struct _stat buffer;
	if (_stat(path, &buffer) < 0)
		return FOLDER_ERROR;

	return (buffer.st_mode & _S_IFDIR) != 0 ? FOLDER_YES : FOLDER_NO;
#else
	struct stat buffer;

	if (stat(path, &buffer) < 0)
		return FOLDER_ERROR;

	return (buffer.st_mode & S_IFDIR) != 0 ? FOLDER_YES : FOLDER_NO;
#endif
}

bool btFileSysCommon::IsFileWritable(const char * path)
{
#if _WIN32
	struct _stat st;
	if (_stat(path, &st) == -1)
		return true;
#else
	struct stat st;
	if (stat(path, &st) == -1)
		return true;
#endif
	return (st.st_mode & S_IWRITE) != 0;
}

int btFileSysCommon::ListFiles(const char * directory, const char * extension, idStrList & list)
{
#if defined(_WIN32) || defined(_WIN64)
	idStr		search;
	struct _finddata_t findinfo;
	// RB: 64 bit fixes, changed int to intptr_t
	intptr_t	findhandle;
	// RB end
	int			flag;

	if (!extension)
		extension = "";

	// passing a slash as extension will find directories
	if (extension[0] == '/' && extension[1] == 0) 
	{
		extension = "";
		flag = 0;
	}
	else 
	{
		flag = _A_SUBDIR;
	}

	sprintf(search, "%s\\*%s", directory, extension);

	// search
	list.Clear();

	findhandle = _findfirst(search, &findinfo);
	if (findhandle == -1)
		return -1;

	do {
		if (flag ^ (findinfo.attrib & _A_SUBDIR))
			list.Append(findinfo.name);
	} while (_findnext(findhandle, &findinfo) != -1);

	_findclose(findhandle);
#else
	struct dirent* d;
	DIR* fdir;
	bool dironly = false;
	char search[MAX_OSPATH];
	struct stat st;
	bool debug;

	list.Clear();

	debug = cvarSystem->GetCVarBool("fs_debug");
	// DG: we use fnmatch for shell-style pattern matching
	// so the pattern should at least contain "*" to match everything,
	// the extension will be added behind that (if !dironly)
	idStr pattern("*");

	// passing a slash as extension will find directories
	if (extension[0] == '/' && extension[1] == 0)
		dironly = true;
	else
	{
		// so we have *<extension>, the same as in the windows code basically
		pattern += extension;
	}
	// DG end

	// NOTE: case sensitivity of directory path can screw us up here
	if ((fdir = opendir(directory)) == NULL)
	{
		if (debug)
			common->Printf("Sys_ListFiles: opendir %s failed\n", directory);
		return -1;
	}

	// DG: use readdir_r instead of readdir for thread safety
	// the following lines are from the readdir_r manpage.. fscking ugly.
	int nameMax = pathconf(directory, _PC_NAME_MAX);
	if (nameMax == -1)
		nameMax = 255;
	int direntLen = offsetof(struct dirent, d_name) + nameMax + 1;

	struct dirent* entry = (struct dirent*)Mem_Alloc(direntLen, TAG_CRAP);

	if (entry == NULL)
	{
		common->Warning("Sys_ListFiles: Mem_Alloc for entry failed!");
		closedir(fdir);
		return 0;
	}

	while (readdir_r(fdir, entry, &d) == 0 && d != NULL)
	{
		// DG end
		idStr::snPrintf(search, sizeof(search), "%s/%s", directory, d->d_name);
		if (stat(search, &st) == -1)
			continue;
		if (!dironly)
		{
			// DG: the original code didn't work because d3 bfg abuses the extension
			// to match whole filenames and patterns in the savegame-code, not just file extensions...
			// so just use fnmatch() which supports matching shell wildcard patterns ("*.foo" etc)
			// if we should ever need case insensitivity, use FNM_CASEFOLD as third flag
			if (fnmatch(pattern.c_str(), d->d_name, 0) != 0)
				continue;
			// DG end
		}
		if ((dironly && !(st.st_mode & S_IFDIR)) ||
			(!dironly && (st.st_mode & S_IFDIR)))
			continue;

		list.Append(d->d_name);
	}

	closedir(fdir);
	Mem_Free(entry);

	if (debug)
		common->Printf("Sys_ListFiles: %d entries in %s\n", list.Num(), directory);

#endif
	return list.Num();
}

ID_TIME_T btFileSysCommon::FileTimeStamp(idFileHandle fp)
{
#if _WIN32
	FILETIME writeTime;
	GetFileTime(fp, NULL, NULL, &writeTime);

	/*
	FILETIME = number of 100-nanosecond ticks since midnight
	1 Jan 1601 UTC. time_t = number of 1-second ticks since
	midnight 1 Jan 1970 UTC. To translate, we subtract a
	FILETIME representation of midnight, 1 Jan 1970 from the
	time in question and divide by the number of 100-ns ticks
	in one second.
	*/

	SYSTEMTIME base_st =
	{
		1970,   // wYear
		1,      // wMonth
		0,      // wDayOfWeek
		1,      // wDay
		0,      // wHour
		0,      // wMinute
		0,      // wSecond
		0       // wMilliseconds
	};

	FILETIME base_ft;
	SystemTimeToFileTime(&base_st, &base_ft);

	LARGE_INTEGER itime;
	itime.QuadPart = reinterpret_cast<LARGE_INTEGER&>(writeTime).QuadPart;
	itime.QuadPart -= reinterpret_cast<LARGE_INTEGER&>(base_ft).QuadPart;
	itime.QuadPart /= 10000000LL;
	return itime.QuadPart;
#else
	struct stat st;
	fstat(fileno(fp), &st);
	return st.st_mtime;
#endif
}

int btFileSysCommon::GetDriveFreeSpace(const char * path)
{
	int ret = 26;
#if _WIN32
	DWORDLONG lpFreeBytesAvailable;
	DWORDLONG lpTotalNumberOfBytes;
	DWORDLONG lpTotalNumberOfFreeBytes;

	//FIXME: see why this is failing on some machines
	if (::GetDiskFreeSpaceEx(path, (PULARGE_INTEGER)&lpFreeBytesAvailable, (PULARGE_INTEGER)&lpTotalNumberOfBytes, (PULARGE_INTEGER)&lpTotalNumberOfFreeBytes))
	{
		ret = (double)(lpFreeBytesAvailable) / (1024.0 * 1024.0);
	}
#else
	

	struct statvfs st;

	if (statvfs(path, &st) == 0)
	{
		unsigned long blocksize = st.f_bsize;
		unsigned long freeblocks = st.f_bfree;

		unsigned long free = blocksize * freeblocks;

		ret = (double)(free) / (1024.0 * 1024.0);
	}

	
#endif
	return ret;
}

int64 btFileSysCommon::GetDriveFreeSpaceInBytes(const char * path)
{
	int64 ret = 1;
#if _WIN32
	DWORDLONG lpFreeBytesAvailable;
	DWORDLONG lpTotalNumberOfBytes;
	DWORDLONG lpTotalNumberOfFreeBytes;
	
	//FIXME: see why this is failing on some machines
	if (::GetDiskFreeSpaceEx(path, (PULARGE_INTEGER)&lpFreeBytesAvailable, (PULARGE_INTEGER)&lpTotalNumberOfBytes, (PULARGE_INTEGER)&lpTotalNumberOfFreeBytes))
	{
		ret = lpFreeBytesAvailable;
	}
#else
	struct statvfs st;

	if (statvfs(path, &st) == 0)
	{
		unsigned long blocksize = st.f_bsize;
		unsigned long freeblocks = st.f_bfree;

		unsigned long free = blocksize * freeblocks;

		ret = free;
	}
#endif
	return ret;
}
