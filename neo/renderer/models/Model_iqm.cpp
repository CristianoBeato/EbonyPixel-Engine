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
	int			size;
	iqmheader	Header;
	uint32		numjoints = 0;
	size_t		jointsBuffLen = 0;
	btSmartPtr<iqmjoint> joints;

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


	//check for a valid IQM model
	if (stricmp(Header.magic, IQM_MAGIC) != 0 || Header.version != IQM_VERSION )
	{
		MakeDefaultModel();
		return;
	}

	//sanity check... don't load files bigger than 16 MB
	if (Header.filesize > (16 << 20) || Header.num_meshes <= 0)
	{
		MakeDefaultModel();
		return;
	}

	//Create the text buffer
#if 1
	srcFile->Seek(sizeof(iqmheader), FS_SEEK_SET);
	//TODO: find a better way
	byte * buf = (byte*)Mem_Alloc(Header.filesize, TAG_MODEL);
	srcFile->Read(buf, Header.filesize);
	const char *texts = Header.ofs_text ? (char *)&buf[Header.ofs_text] : "";
	Mem_Free(buf);
#else
	srcFile->Seek(Header.ofs_text, FS_SEEK_SET);
	char *texts = "";
	srcFile->Read(texts, Header.num_text);
#endif

	//load model Joints
	numjoints = Header.num_joints;

	//Set skeleton 
	m_joints.SetGranularity(1);
	m_joints.SetNum(numjoints);
	jointsBuffLen = sizeof(iqmjoint) * numjoints;

	//reserve memory for the joints
	joints.Alloc(numjoints, TAG_MODEL);

	//read joins 
	srcFile->Seek(Header.ofs_joints, FS_SEEK_SET);// move file carret to the joits buffer ofest
	srcFile->Read(joints.GetInternalPtr(), jointsBuffLen);

	for (int i = 0; i < numjoints; i++)
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
