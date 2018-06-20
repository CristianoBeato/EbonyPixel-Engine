/*
===========================================================================

Elbony Pixel Source Code
Copyright (C) 2017-2018 Cristiano Beato

This file is part of the Elbony Pixel Source Code ("Elbony Pixel Source Code").

Elbony Pixel Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Elbony Pixel Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Elbony Pixel Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Elbony Pixel Source Code is also subject to certain additional terms.
You should have received a copy of these additional terms immediately following the terms and conditions of the
GNU General Public License which accompanied the Elbony Pixel Source Code.

===========================================================================
*/

#ifndef _MODEL_IQM_H_
#define _MODEL_IQM_H_

#define IQM_MESH_EXT			"iqm"
#define IQM_MAGIC "INTERQUAKEMODEL"
#define IQM_VERSION 2

#include "Model_local.h"

/*
==================================================================
	IQM model Structures
==================================================================
*/

struct iqmheader
{
	char magic[16];
	uint32 version;
	uint32 filesize;
	uint32 flags;
	uint32 num_text, ofs_text;
	uint32 num_meshes, ofs_meshes;
	uint32 num_vertexarrays, num_vertexes, ofs_vertexarrays;
	uint32 num_triangles, ofs_triangles, ofs_adjacency;
	uint32 num_joints, ofs_joints;
	uint32 num_poses, ofs_poses;
	uint32 num_anims, ofs_anims;
	uint32 num_frames, num_framechannels, ofs_frames, ofs_bounds;
	uint32 num_comment, ofs_comment;
	uint32 num_extensions, ofs_extensions;
};

struct iqmmesh
{
	uint32 name;
	uint32 material;
	uint32 first_vertex, num_vertexes;
	uint32 first_triangle, num_triangles;
};

enum
{
	IQM_POSITION = 0,
	IQM_TEXCOORD = 1,
	IQM_NORMAL = 2,
	IQM_TANGENT = 3,
	IQM_BLENDINDEXES = 4,
	IQM_BLENDWEIGHTS = 5,
	IQM_COLOR = 6,
	IQM_CUSTOM = 0x10
};

enum
{
	IQM_BYTE = 0,
	IQM_UBYTE = 1,
	IQM_SHORT = 2,
	IQM_USHORT = 3,
	IQM_INT = 4,
	IQM_UINT = 5,
	IQM_HALF = 6,
	IQM_FLOAT = 7,
	IQM_DOUBLE = 8,
};

struct iqmtriangle
{
	uint32 vertex[3];
};

struct iqmadjacency
{
	uint32 triangle[3];
};

struct iqmjoint
{
	uint32 name;
	int parent;
	float translate[3], rotate[4], scale[3];
};

struct iqmpose
{
	int parent;
	uint32 mask;
	float channeloffset[10];
	float channelscale[10];
};

struct iqmanim
{
	uint32 name;
	uint32 first_frame, num_frames;
	float framerate;
	uint32 flags;
};

enum
{
	IQM_LOOP = 1 << 0
};

struct iqmvertexarray
{
	uint32 type;
	uint32 flags;
	uint32 format;
	uint32 size;
	uint32 offset;
};

struct iqmbounds
{
	float bbmin[3], bbmax[3];
	float xyradius, radius;
};

/*
==================================================================
	Engine IQM structures
==================================================================
*/
class btIqmMesh
{
public:
	btIqmMesh(void) {};
	~btIqmMesh(void) {};

private:

};

class btIqmMesh
{
public:
	btIqmMesh(void) {};
	~btIqmMesh(void) {};
private:

};

class btIqmJoint : public btGameJoint
{
public:
	btIqmJoint(void) {};
	btIqmJoint(const idVec3 post, const idQuat orient, const idVec3 size);
	~btIqmJoint(void) {};

private:
	idJointQuat		pose;
	idJointMat		poseMat;
};

class idRenderModelIQM : public idRenderModelStatic
{
public:
	idRenderModelIQM(void);
	~idRenderModelIQM(void);

	virtual void				InitFromFile(const char* fileName);
	virtual dynamicModel_t		IsDynamicModel(void) const;
	virtual idBounds			Bounds(const struct renderEntity_s* ent) const;
	virtual void				Print(void) const;
	virtual void				List(void) const;
	virtual void				TouchData(void);
	virtual void				PurgeModel(void);
	virtual void				LoadModel(void);
	virtual int					Memory(void) const;
	virtual idRenderModel* 		InstantiateDynamicModel(const struct renderEntity_s* ent, const viewDef_t* view, idRenderModel* cachedModel);
	virtual int					NumJoints(void) const;
	virtual const btGameJoint* 	GetJoints(void) const;
	virtual jointHandle_t		GetJointHandle(const char* name) const;
	virtual const char* 		GetJointName(jointHandle_t handle) const;
	virtual const idJointQuat* 	GetDefaultPose(void) const;
	virtual int					NearestJoint(int surfaceNum, int a, int b, int c) const;

	//tips: we can integrate a IQE loader, and here we put a IQM compiler
	//virtual bool				SupportsBinaryModel(void);
	//virtual bool				LoadBinaryModel(idFile* file, const ID_TIME_T sourceTimeStamp);
	//virtual void				WriteBinaryModel(idFile* file, ID_TIME_T* _timeStamp = NULL) const;

private:
	idStr									m_modelPath;
	idList<btIqmMesh, TAG_MODEL>			m_meshes;
	idList<btIqmJoint, TAG_MODEL>			m_joints;
};

#endif // !_MODEL_IQM_H_
