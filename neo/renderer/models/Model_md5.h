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

#include "Model_local.h"

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

class idMD5Mesh
{
	friend class				idRenderModelMD5;

public:
	idMD5Mesh();
	~idMD5Mesh();

	void						ParseMesh(idLexer& parser, int numJoints, const idJointMat* joints);

	int							NumVerts() const
	{
		return numVerts;
	}
	int							NumTris() const
	{
		return numTris;
	}

	void						UpdateSurface(const struct renderEntity_s* ent, const idJointMat* joints,
												const	idJointMat* entJointsInverted, modelSurface_t* surf);
	void						CalculateBounds(const idJointMat* entJoints, idBounds& bounds) const;
	int							NearestJoint(int a, int b, int c) const;

private:
	const idMaterial* 			shader;				// material applied to mesh
	int							numVerts;			// number of vertices
	int							numTris;			// number of triangles
	byte* 						meshJoints;			// the joints used by this mesh
	int							numMeshJoints;		// number of mesh joints
	float						maxJointVertDist;	// maximum distance a vertex is separated from a joint
	deformInfo_t* 				deformInfo;			// used to create srfTriangles_t from base frames and new vertexes
	int							surfaceNum;			// number of the static surface created for this mesh
};

class idRenderModelMD5 : public idRenderModelStatic
{
public:
	virtual void				InitFromFile(const char* fileName);
	virtual bool				LoadBinaryModel(idFile* file, const ID_TIME_T sourceTimeStamp);
	virtual void				WriteBinaryModel(idFile* file, ID_TIME_T* _timeStamp = NULL) const;
	virtual dynamicModel_t		IsDynamicModel(void) const;
	virtual idBounds			Bounds(const struct renderEntity_s* ent) const;
	virtual void				Print(void) const;
	virtual void				List() const;
	virtual void				TouchData();
	virtual void				PurgeModel();
	virtual void				LoadModel();
	virtual int					Memory() const;
	virtual idRenderModel* 		InstantiateDynamicModel(const struct renderEntity_s* ent, const viewDef_t* view, idRenderModel* cachedModel);
	virtual int					NumJoints(void) const;
	virtual const idMD5Joint* 	GetJoints() const;
	virtual jointHandle_t		GetJointHandle(const char* name) const;
	virtual const char* 		GetJointName(jointHandle_t handle) const;
	virtual const idJointQuat* 	GetDefaultPose() const;
	virtual int					NearestJoint(int surfaceNum, int a, int b, int c) const;

	virtual bool				SupportsBinaryModel()
	{
		return true;
	}

private:
	idList<idMD5Mesh, TAG_MODEL>	meshes;
	idList<idMD5Joint, TAG_MODEL>	joints;
	idList<idJointQuat, TAG_MODEL>	defaultPose;
	idList<idJointMat, TAG_MODEL>	invertedDefaultPose;

	void						DrawJoints(const renderEntity_t* ent, const viewDef_t* view) const;
	void						ParseJoint(idLexer& parser, idMD5Joint* joint, idJointQuat* defaultPose);
};

#endif /* !__MODEL_MD5_H__ */
