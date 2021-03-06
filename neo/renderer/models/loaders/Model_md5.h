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

#ifndef __MODEL_MD5_H__
#define __MODEL_MD5_H__

#include "renderer/models/internal/Model_skined.h"

// shared between the renderer, game, and Maya export DLL
#define MD5_VERSION_STRING		"MD5Version"
#define MD5_MESH_EXT			"md5mesh"
#define MD5_ANIM_EXT			"md5anim"
#define MD5_CAMERA_EXT			"md5camera"
#define MD5_VERSION				10

class idMD5Joint : public btGameJoint
{
public:
	idMD5Joint(void) : btGameJoint()
	{
	}
};

/*
===============================================================================

MD5 animated model

===============================================================================
*/
class idMD5Mesh : public btRenderMeshSkined
{
	friend class				idRenderModelMD5;
public:
	idMD5Mesh(void);
	~idMD5Mesh(void);

	size_t						getMeshUsedMemory(void);
	void						ParseMesh(idLexer& parser, int numJoints, const idJointMat* joints);
	void						LoadBinaryMesh(idFile* file);
	void						WriteBinaryMesh(idFile* file);
	int							NearestJoint(int a, int b, int c) const;
	virtual void				Clear(void);

private:
	int							surfaceNum;			// number of the static surface created for this mesh
};

class idRenderModelMD5 : public btRenderModelSkined
{
public:
	virtual void				InitFromFile(const char* fileName);
	virtual void				LoadModel(void);
	virtual bool				LoadBinaryModel(idFile* file, const ID_TIME_T sourceTimeStamp);
	virtual void				WriteBinaryModel(idFile* file, ID_TIME_T* _timeStamp = NULL) const;

	virtual void				List() const;
	virtual void				TouchData();
	virtual void				PurgeModel();
	virtual int					Memory() const;
	
	virtual int					NearestJoint(int surfaceNum, int a, int b, int c) const;

	//
	virtual bool				SupportsBinaryModel(void){ return true; }

private:
	void						ParseJoint(idLexer& parser, btGameJoint* joint, idJointQuat* defaultPose);
};

#endif /* !__MODEL_MD5_H__ */
