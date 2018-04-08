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
#ifndef _SYS_COMMON_FILES_H_
#define _SYS_COMMON_FILES_H_

class btFileSysCommon
{
public:
	const char *	DefaultBasePath(void);
	const char*		DefaultSavePath(void);
	const char *	EXEPath(void);
	void			Mkdir(const char* path);
	bool			Rmdir(const char* path);
	const char *	Sys_Cwd(void);
	sysFolder_t		IsFolder(const char* path);
	bool			IsFileWritable(const char* path);
	int				ListFiles(const char* directory, const char* extension, idStrList& list);
	ID_TIME_T		FileTimeStamp(idFileHandle fp);
	int				GetDriveFreeSpace(const char* path);
	int64			GetDriveFreeSpaceInBytes(const char* path);

private:
	//store here the game engine paths
	idStr savePath, basePath, exePath;
};


#endif // !_SYS_COMMON_FILES_H_
