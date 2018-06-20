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
#include "precompiled.h"
#pragma hdrstop

#include "Model_iqm.h"
#include "renderer/tr_local.h"

idRenderModelIQM::idRenderModelIQM(void)
{
}

idRenderModelIQM::~idRenderModelIQM(void)
{
}

void idRenderModelIQM::InitFromFile(const char * fileName)
{
	int							size;
	iqmheader					Header;
	uint32						numjoints = 0, numMeshes = 0;
	size_t						jointsBuffLen = 0;
	btSmartPtr<iqmjoint>		joints;
	btSmartPtr<iqmmesh>			meshes;
	btSmartPtr<iqmvertexarray>	vas;

	//Vertexes arrays
	btSmartPtr<float>	vpos;
	btSmartPtr<float>	vtc;
	btSmartPtr<float>	vnorm;
	btSmartPtr<float>	vtan;
	btSmartPtr<byte>	vindex;
	btSmartPtr<byte>	vweight;

	//store path for debug
	m_modelPath = fileName;
	
	//idFile	* srcFile = fileSystem->OpenFileRead(fileName);
	btScopedFile srcFile(fileName);
	if (!srcFile.isOpen())
	{
		MakeDefaultModel();
		return;
	}

	size = srcFile->Length();
	if (!size || size < 0)
	{
		MakeDefaultModel();
		return;
	}

	//TODO: This way only load the model as little endian,

 	//need implement a better way to load little endian models
	//srcFile->ReadBig<iqmheader>(Header);
 	srcFile->Read(&Header, sizeof(iqmheader));

	//check for a valid IQM model and 
	//sanity check... don't load files bigger than 16 MB
	if (stricmp(Header.magic, IQM_MAGIC) != 0 || Header.version != IQM_VERSION 
		|| Header.filesize > (16 << 20) || Header.num_meshes <= 0
		)
	{
		MakeDefaultModel();
		return;
	}

#if 0



#else //this can be a better way, but we have problem whit text

	//load model Joints
	numjoints = Header.num_joints;

	//Set skeleton 
	m_joints.SetGranularity(1);
	m_joints.SetNum(numjoints);

	//reserve memory for the joints
	joints.Alloc(numjoints, TAG_MODEL);

	//read joins 
	srcFile->Seek(Header.ofs_joints, FS_SEEK_SET);// move file carret to the joits buffer ofest
	srcFile->Read(joints.GetInternalPtr(), sizeof(iqmjoint) * numjoints);

	for (uint32 i = 0; i < numjoints; i++)
	{
		iqmjoint &j = joints.GetInternalPtr()[i];
		//set the bone name
		idStr jointName = &texts[j.name];

		//Set the bone orientation 
		m_joints[i] = btIqmJoint(	idVec3(j.translate[0], j.translate[1], j.translate[2]),
									idQuat(j.rotate[0], j.rotate[1], j.rotate[2], j.rotate[3]),
									idVec3(j.scale[0], j.scale[1], j.scale[2]));
		//chek if have parent
		if (j.parent > -1)
			m_joints[i].parent = &m_joints[j.parent];
		else
			m_joints[i].parent = NULL;
	}

	//Load Faces
	if (Header.num_triangles < 0)
	{
		common->Error("idRenderModelIQM::InitFromFile(%s), invalid tris number", fileName);
		common->Printf()
		MakeDefaultModel();
		return;
	}

	idList<int> tris;
	tris.SetNum(Header.num_triangles * 3);

	srcFile->Seek(Header.ofs_triangles, FS_SEEK_SET);
	for (uint32 i = 0; i < Header.num_triangles; i++)
	{
		int index[3];
		srcFile->ReadInt(tris[i * 3 + 0]);
		srcFile->ReadInt(tris[i * 3 + 1]);
		srcFile->ReadInt(tris[i * 3 + 2]);
	}

	//Load vertices
	srcFile->Seek(Header.ofs_vertexarrays, FS_SEEK_SET);
	vas.Alloc(Header.num_vertexarrays, TAG_MODEL);
	srcFile->Read(vas.GetInternalPtr(), sizeof(iqmvertexarray) * Header.num_vertexarrays);
	for (uint32 i = 0; i < Header.num_vertexarrays; i++)
	{
		iqmvertexarray &va = vas.GetInternalPtr()[i];
		switch (va.type)
		{
		case IQM_POSITION: 
		{
			if (va.format != IQM_FLOAT || va.size != 3)
			{
				//TODO: raise a error
				break;
			}

			vpos.Alloc(Header.num_vertexes * 3, TAG_MODEL);
			srcFile->Seek(va.offset, FS_SEEK_SET);
			srcFile->Read(vpos.GetInternalPtr(), (sizeof(float) * 3)* Header.num_vertexes);
			break;
		}
		case IQM_NORMAL:
		{
			if (va.format != IQM_FLOAT || va.size != 3)
			{
				//TODO: raise a error
				break;
			}
			vnorm.Alloc(Header.num_vertexes * 3, TAG_MODEL);
			srcFile->Seek(va.offset, FS_SEEK_SET);
			srcFile->Read(vnorm.GetInternalPtr(), (sizeof(float) * 3)* Header.num_vertexes);
			break;
		}
		case IQM_TANGENT:
		{
			if (va.format != IQM_FLOAT || va.size != 4)
			{
				//TODO: raise a error
				break;
			}
			vtan.Alloc(Header.num_vertexes * 4, TAG_MODEL);
			srcFile->Seek(va.offset, FS_SEEK_SET);
			srcFile->Read(vtan.GetInternalPtr(), (sizeof(float) * 4)* Header.num_vertexes);

			break;
		}
		case IQM_TEXCOORD: 
		{
			if (va.format != IQM_FLOAT || va.size != 2)
			{
				//TODO: raise a error
				break;
			}
			vtc.Alloc(Header.num_vertexes * 2, TAG_MODEL);
			srcFile->Seek(va.offset, FS_SEEK_SET);
			srcFile->Read(vtc.GetInternalPtr(), (sizeof(float) * 2)* Header.num_vertexes);
			break;
		}
		case IQM_BLENDINDEXES:
		{
			if (va.format != IQM_UBYTE || va.size != 4)
			{
				//TODO: raise a error
				break;
			}
			vindex.Alloc(Header.num_vertexes * 4, TAG_MODEL);
			srcFile->Seek(va.offset, FS_SEEK_SET);
			srcFile->Read(vindex.GetInternalPtr(), (sizeof(byte) * 4)* Header.num_vertexes);
			break;
		}
		case IQM_BLENDWEIGHTS: 
		{
			if (va.format != IQM_UBYTE || va.size != 4)
			{
				//TODO: raise a error
				break;
			}
			vweight.Alloc(Header.num_vertexes * 4, TAG_MODEL);
			srcFile->Seek(va.offset, FS_SEEK_SET);
			srcFile->Read(vweight.GetInternalPtr(), (sizeof(byte) * 4)* Header.num_vertexes);
			break;
		}
		}
	}

	//Set meshes 
	m_meshes.SetGranularity(1);
	m_meshes.SetNum(numMeshes);

	//reserve memory for the joints
	meshes.Alloc(numMeshes, TAG_MODEL);

	//read joins 
	srcFile->Seek(Header.ofs_meshes, FS_SEEK_SET);// move file carret to the joits buffer ofest
	srcFile->Read(meshes.GetInternalPtr(), sizeof(iqmmesh) * numMeshes);
	for (uint32 i = 0; i < numMeshes; i++)
	{
		iqmmesh &m = meshes.GetInternalPtr()[i];

	}
#endif
}

dynamicModel_t idRenderModelIQM::IsDynamicModel(void) const
{
	return DM_CACHED;
}

idBounds idRenderModelIQM::Bounds(const renderEntity_s * ent) const
{
	return idBounds();
}

void idRenderModelIQM::Print(void) const
{
}

void idRenderModelIQM::List(void) const
{
}

void idRenderModelIQM::TouchData(void)
{
}

void idRenderModelIQM::PurgeModel(void)
{
}

void idRenderModelIQM::LoadModel(void)
{
}

int idRenderModelIQM::Memory(void) const
{
	return 0;
}

idRenderModel * idRenderModelIQM::InstantiateDynamicModel(const renderEntity_s * ent, const viewDef_t * view, idRenderModel * cachedModel)
{
	return nullptr;
}

int idRenderModelIQM::NumJoints(void) const
{
	return 0;
}

const btGameJoint * idRenderModelIQM::GetJoints(void) const
{
	return nullptr;
}

jointHandle_t idRenderModelIQM::GetJointHandle(const char * name) const
{
	return jointHandle_t();
}

const char * idRenderModelIQM::GetJointName(jointHandle_t handle) const
{
	return nullptr;
}

const idJointQuat * idRenderModelIQM::GetDefaultPose(void) const
{
	return nullptr;
}

int idRenderModelIQM::NearestJoint(int surfaceNum, int a, int b, int c) const
{
	return 0;
}

#if 0
bool idRenderModelIQM::SupportsBinaryModel(void)
{
	return false;
}

bool idRenderModelIQM::LoadBinaryModel(idFile * file, const ID_TIME_T sourceTimeStamp)
{
	return false;
}

void idRenderModelIQM::WriteBinaryModel(idFile * file, ID_TIME_T * _timeStamp) const
{
}
#endif

btIqmJoint::btIqmJoint(const idVec3 post, const idQuat orient, const idVec3 size)
{
	pose.q = orient;
	pose.t = post;
	poseMat.SetRotation(orient.ToMat3());
	poseMat.SetTranslation(post);
}
