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
#include "renderer/models/Model_local.h"

idHashTableT<const char *, char *> enames;

const char *getnamekey(const char *name)
{
	char **exists = enames.access(name);
	if (exists) return *exists;
	char *key = newstring(name);
	enames[key] = key;
	return key;
}

idRenderModelIQM::idRenderModelIQM(void) : btRenderModelSkined()
{
}

idRenderModelIQM::~idRenderModelIQM(void)
{
}

void idRenderModelIQM::InitFromFile(const char * fileName)
{
	m_name = fileName;
	LoadModel();
}

void idRenderModelIQM::LoadModel(void)
{
	char* buf;
	fileSystem->ReadFile(m_name, (void**)&buf, &m_timeStamp);
	if (!buf)
		return ;

	const char *curmesh = getnamekey(""), *curmaterial = getnamekey("");
	bool needmesh = true;
	int fmoffset = 0;
	char buf[512];
	if (!f->getline(buf, sizeof(buf))) return;
	if (!strchr(buf, '#') || strstr(buf, "# Inter-Quake Export") != strchr(buf, '#')) return false;
	while (f->getline(buf, sizeof(buf)))
	{
		char *c = buf;
		while (isspace(*c)) ++c;
		if (isalpha(c[0]) && isalnum(c[1]) && (!c[2] || isspace(c[2]))) switch (*c++)
		{
		case 'v':
			switch (*c++)
			{
			case 'p': epositions.add(parseattribs4(c, Vec4(0, 0, 0, 1))); continue;
			case 't': etexcoords.add(parseattribs4(c)); continue;
			case 'n': enormals.add(parseattribs3(c)); continue;
			case 'x':
			{
				Vec4 tangent(parseattribs3(c), 0);
				Vec3 bitangent(0, 0, 0);
				bitangent.x = parseattrib(c);
				if (maybeparseattrib(c, bitangent.y))
				{
					bitangent.z = parseattrib(c);
					ebitangents.add(bitangent);
				}
				else tangent.w = bitangent.x;
				etangents.add(tangent);
				continue;
			}
			case 'b': eblends.add(parseblends(c)); continue;
			case 'c': ecolors.add(parseattribs4(c, Vec4(0, 0, 0, 1))); continue;
			case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
			{
				int n = c[-1] - '0';
				ecustom[n].add(parseattribs4(c));
				continue;
			}
			case 's':
				parseindex(c, esmoothindexes.add());
				continue;
			}
			break;
		case 'p':
		{
			transform t;
			switch (*c++)
			{
			case 'q':
			{
				t.pos = parseattribs3(c);
				loopk(3) t.orient[k] = parseattrib(c);

				t.orient.restorew();
				double w = parseattrib(c, t.orient.w);
				if (w != t.orient.w)
				{
					t.orient.w = w;
					t.orient.normalize();
					//        double x2 = f.orient.x*f.orient.x, y2 = f.orient.y*f.orient.y, z2 = f.orient.z*f.orient.z, w2 = f.orient.w*f.orient.w, s2 = x2 + y2 + z2 + w2;
					//        f.orient.x = keepsign(f.orient.x, sqrt(max(1.0 - (w2 + y2 + z2) / s2, 0.0)));
					//        f.orient.y = keepsign(f.orient.y, sqrt(max(1.0 - (w2 + x2 + z2) / s2, 0.0)));
					//        f.orient.z = keepsign(f.orient.z, sqrt(max(1.0 - (w2 + x2 + y2) / s2, 0.0)));
					//        f.orient.w = keepsign(f.orient.w, sqrt(max(1.0 - (x2 + y2 + z2) / s2, 0.0)));
				}
				if (t.orient.w > 0) t.orient.flip();
				t.scale = parseattribs3(c, Vec3(1, 1, 1));
				eposes.add(t);
				continue;
			}
			case 'm':
			{
				t.pos = parseattribs3(c);
				Matrix3x3 m;
				m.a = parseattribs3(c);
				m.b = parseattribs3(c);
				m.c = parseattribs3(c);
				Vec3 mscale(Vec3(m.a.x, m.b.x, m.c.x).magnitude(), Vec3(m.a.y, m.b.y, m.c.y).magnitude(), Vec3(m.a.z, m.b.z, m.c.z).magnitude());
				// check determinant for sign of scaling
				if (m.determinant() < 0) mscale = -mscale;
				m.a /= mscale;
				m.b /= mscale;
				m.c /= mscale;
				t.orient = Quat(m);
				if (t.orient.w > 0) t.orient.flip();
				t.scale = parseattribs3(c, Vec3(1, 1, 1)) * mscale;
				eposes.add(t);
				continue;
			}
			case 'a':
			{
				t.pos = parseattribs3(c);
				Vec3 rot = parseattribs3(c);
				t.orient = Quat::fromangles(rot);
				t.scale = parseattribs3(c, Vec3(1, 1, 1));
				eposes.add(t);
				continue;
			}
			}
			break;
		}
		case 'f':
			switch (*c++)
			{
			case 'a':
			{
				int i1 = 0, i2 = 0, i3 = 0;
				if (!parseindex(c, i1) || !parseindex(c, i2)) continue;
				if (needmesh)
				{
					emeshes.add(emesh(curmesh, curmaterial, etriangles.length()));
					needmesh = false;
				}
				if (i1 < 0) i1 = max(epositions.length() + i1, 0);
				if (i2 < 0) i2 = max(epositions.length() + i2, 0);
				while (parseindex(c, i3))
				{
					if (i3 < 0) i3 = max(epositions.length() + i3, 0);
					esmoothgroups.last().flags |= esmoothgroup::F_USED;
					etriangles.add(etriangle(i1, i2, i3, esmoothgroups.length() - 1));
					i2 = i3;
				}
				continue;
			}
			case 'm':
			{
				int i1 = 0, i2 = 0, i3 = 0;
				if (!parseindex(c, i1) || !parseindex(c, i2)) continue;
				if (needmesh)
				{
					emeshes.add(emesh(curmesh, curmaterial, etriangles.length()));
					needmesh = false;
				}
				i1 = i1 < 0 ? max(epositions.length() + i1, 0) : (fmoffset + i1);
				i2 = i2 < 0 ? max(epositions.length() + i2, 0) : (fmoffset + i2);
				while (parseindex(c, i3))
				{
					i3 = i3 < 0 ? max(epositions.length() + i3, 0) : (fmoffset + i3);
					esmoothgroups.last().flags |= esmoothgroup::F_USED;
					etriangles.add(etriangle(i1, i2, i3, esmoothgroups.length() - 1));
					i2 = i3;
				}
				continue;
			}
			case 's':
			{
				int i1 = 0, i2 = 0, i3 = 0;
				uchar flags = 0;
				if (!parseindex(c, i1) || !parseindex(c, i2) || !parseindex(c, i3)) continue;
				flags |= clamp(i1, 0, 1);
				flags |= clamp(i2, 0, 1) << 1;
				flags |= clamp(i3, 0, 1) << 2;
				esmoothgroups.last().flags |= esmoothgroup::F_USED;
				while (parseindex(c, i3))
				{
					esmoothedges.add(flags | 4);
					flags = 1 | ((flags & 4) >> 1) | (clamp(i3, 0, 1) << 2);
				}
				esmoothedges.add(flags);
				continue;
			}
			}
			break;
		}
		char *args = c;
		while (*args && !isspace(*args)) args++;
		if (!strncmp(c, "smoothgroup", max(int(args - c), 11)))
		{
			if (esmoothgroups.last().flags & esmoothgroup::F_USED) esmoothgroups.dup();
			parseindex(args, esmoothgroups.last().key);
		}
		else if (!strncmp(c, "smoothangle", max(int(args - c), 11)))
		{
			if (esmoothgroups.last().flags & esmoothgroup::F_USED) esmoothgroups.dup();
			double angle = parseattrib(args, 0);
			esmoothgroups.last().angle = fabs(cos(clamp(angle, -180.0, 180.0) * M_PI / 180));
		}
		else if (!strncmp(c, "smoothuv", max(int(args - c), 8)))
		{
			if (esmoothgroups.last().flags & esmoothgroup::F_USED) esmoothgroups.dup();
			int val = 1;
			if (parseindex(args, val) && val <= 0) esmoothgroups.last().flags &= ~esmoothgroup::F_UVSMOOTH;
			else esmoothgroups.last().flags |= esmoothgroup::F_UVSMOOTH;
		}
		else if (!strncmp(c, "mesh", max(int(args - c), 4)))
		{
			curmesh = getnamekey(trimname(args));
			if (emeshes.empty() || emeshes.last().name != curmesh) needmesh = true;
			fmoffset = epositions.length();

#if 0
			emesh &m = emeshes.add();
			m.firsttri = etriangles.length();
			fmoffset = epositions.length();
			parsename(args, m.name);
#endif
		}
		else if (!strncmp(c, "material", max(int(args - c), 8)))
		{
			curmaterial = getnamekey(trimname(args));
			if (emeshes.empty() || emeshes.last().material != curmaterial) needmesh = true;
			//            if(emeshes.length()) parsename(c, emeshes.last().material);
		}
		else if (!strncmp(c, "joint", max(int(args - c), 5)))
		{
			ejoint &j = ejoints.add();
			j.name = getnamekey(trimname(args));
			parseindex(args, j.parent);
		}
		else if (!strncmp(c, "vertexarray", max(int(args - c), 11)))
		{
			evarray &va = evarrays.add();
			va.type = findvertexarraytype(trimname(args));
			va.format = findvertexarrayformat(trimname(args));
			va.size = strtol(args, &args, 10);
			copystring(va.name, trimname(args));
		}
		else if (!strncmp(c, "animation", max(int(args - c), 9)))
		{
			eanim &a = eanims.add();
			a.name = getnamekey(trimname(args));
			a.startframe = eframes.length();
			if (!eframes.length() || eframes.last() != eposes.length()) eframes.add(eposes.length());
		}
		else if (!strncmp(c, "frame", max(int(args - c), 5)))
		{
			if (eanims.length() && eframes.length() && eframes.last() != eposes.length()) eframes.add(eposes.length());
		}
		else if (!strncmp(c, "framerate", max(int(args - c), 9)))
		{
			if (eanims.length())
			{
				double fps = parseattrib(args);
				eanims.last().fps = max(fps, 0.0);
			}
		}
		else if (!strncmp(c, "loop", max(int(args - c), 4)))
		{
			if (eanims.length()) eanims.last().flags |= IQM_LOOP;
		}
		else if (!strncmp(c, "comment", max(int(args - c), 7)))
		{
			if (commentdata.length()) break;
			for (;;)
			{
				int len = f->read(commentdata.reserve(1024), 1024);
				commentdata.advance(len);
				if (len < 1024) { commentdata.add('\0'); break; }
			}
		}
	}
}

bool idRenderModelIQM::loadJoints(const byte * buff)
{
	unsigned int i;
	if (!iqmHdr)
		return false;


	const char *str = iqmHdr->ofs_text ? (char *)&buff[iqmHdr->ofs_text] : "";
	iqmjoint *IQMjoints;
	IQMjoints = (iqmjoint *)(buff + iqmHdr->ofs_joints);

	//start reserving the game joints idents, and joints matrixes
	ReserveJointsNum(iqmHdr->num_joints);
	
	//parse the joints
	idJointMat* poseMat = (idJointMat*)_alloca16(m_joints.Num() * sizeof(poseMat[0]));

	//reserve the space for the bones
	for (i = 0; i < iqmHdr->num_joints; i++)
	{
		//give the joint name 
		LL(IQMjoints[i].name);
		m_joints[i].Name() = &str[IQMjoints[i].name];

		//set the parent
		LL(IQMjoints[i].parent);
		if (IQMjoints[i].parent < 0)
			m_joints[i].Parent() = NULL;
		else
		{
			if (IQMjoints[i].parent >= m_joints.Num() - 1)
				common->Error("Invalid parent for joint '%s'", m_joints[i].Name().c_str());

			m_joints[i].Parent() = &m_joints[IQMjoints[i].parent];
		}

		//process the joit defalt pose, and inverted pose 
		idVec3 bonePos = idVec3(IQMjoints[i].translate[0], IQMjoints[i].translate[1], IQMjoints[i].translate[2]);
		btByteSwap::LittleRevBytes(&bonePos, sizeof(float), sizeof(bonePos) / sizeof(float));
		idQuat boneOrient = idQuat(IQMjoints[i].rotate[0],
			IQMjoints[i].rotate[1],
			IQMjoints[i].rotate[2],
			IQMjoints[i].rotate[3]);
		btByteSwap::LittleRevBytes(&boneOrient, sizeof(float), sizeof(boneOrient) / sizeof(float));
		m_defaultPose[i] = idJointQuat(bonePos, boneOrient);

		
		//m_joints[i].Id() = i;
		m_joints[i].Id() = jointHandle_t(i);


		poseMat[i].SetRotation(m_defaultPose[i].q.ToMat3());
		poseMat[i].SetTranslation(m_defaultPose[i].t);
		if (m_joints[i].Parent())
		{
			//parentNum = joint->parent - (btGameJoint*)joints.Ptr();
			int parentNum = m_joints[i].getParentId();
			m_defaultPose[i].q = (poseMat[i].ToMat3() * poseMat[parentNum].ToMat3().Transpose()).ToQuat();
			m_defaultPose[i].t = (poseMat[i].ToVec3() - poseMat[parentNum].ToVec3()) * poseMat[parentNum].ToMat3().Transpose();
		}
	}

	//generate the inverse base pose
	CreateInverseBasePose(poseMat);

	return true;
}

bool idRenderModelIQM::loadVerterxArray(const byte * buff)
{
	unsigned int numIndex, numWeights;
	idList<Byte>	blendIndex;
	idList<float>	blendWeights;

	if (!iqmHdr)
		return false;

	iqmvertexarray *vas = (iqmvertexarray *)&buff[iqmHdr->ofs_vertexarrays];

	//reserve the space for the vertexes
	reserveVertexes(iqmHdr->num_vertexes);

	//get the vertex attribs in vertex
	for (unsigned int vi = 0; vi < iqmHdr->num_vertexarrays; vi++)
	{
		unsigned int i, j;
		iqmvertexarray &va = vas[vi];
		LL(va.type);
		LL(va.format);
		LL(va.size);
		LL(va.offset);

		switch (va.type)
		{
		case IQM_POSITION:
		{
			if (va.format != IQM_FLOAT || va.size != 3)
			{
				common->Warning("idRenderModelIQM::loadMeshes(%s): invalid Vertex position array", m_name.c_str());
				return false;
			}
			float * positions = (float *)&buff[va.offset];
			btByteSwap::LittleRevBytes(positions, sizeof(float), sizeof(positions) / sizeof(float));
			//convert verts
			for (i = 0; i < iqmHdr->num_vertexes; i++)
			{
				idVec3 *pos = &vertexPos[i];
				pos->x = positions[i * va.size + 0];
				pos->y = positions[i * va.size + 1];
				pos->z = positions[i * va.size + 2];
			}
			break;
		}
		case IQM_TEXCOORD:
		{
			if (va.format != IQM_FLOAT || va.size != 2)
			{
				common->Warning("idRenderModelIQM::loadMeshes(%s): invalid Vertex TextCoord array", m_name.c_str());
				return false;
			}

			float * textcoord = (float *)&buff[va.offset];
			btByteSwap::LittleRevBytes(textcoord, sizeof(float), sizeof(textcoord) / sizeof(float));
			//convert verts
			for (i = 0; i < iqmHdr->num_vertexes; i++)
			{
				idVec2 *uv = &vertexUV[i];
				uv->x = textcoord[i * va.size + 0];
				uv->y = textcoord[i * va.size + 1];
				break;
			}
		}
#if 0	//need to be check 
		case IQM_NORMAL:
		{
			if (va.format != IQM_FLOAT || va.size != 3)
			{
				common->Warning("idRenderModelIQM::loadMeshes(%s): invalid Vertex normal array", m_name.c_str());
				return false;
			}

			float * normais = (float *)&buff[va.offset];
			btByteSwap::LittleRevBytes(normais, sizeof(float), sizeof(normais) / sizeof(float));
			//convert verts
			for (i = 0; i < iqmHdr->num_vertexes; i++)
			{
				idVec3 *nor = &vertexNor[i];
				nor->x = normais[i * va.size + 0];
				nor->y = normais[i * va.size + 1];
				nor->z = normais[i * va.size + 2];
			}
			break;
		}
		case IQM_TANGENT:
		{
			if (va.format != IQM_FLOAT || va.size != 4)
			{
				common->Warning("idRenderModelIQM::loadMeshes(%s): invalid Vertex tangent array", m_name.c_str());
				return false;
			}

			float * tangents = (float *)&buff[va.offset];
			btByteSwap::LittleRevBytes(tangents, sizeof(float), sizeof(tangents) / sizeof(float));
			//convert verts
			for (i = 0; i < iqmHdr->num_vertexes; i++)
			{
				idVec3 *tang = &vertexTan[i];
				tang->x = tangents[i * va.size + 0];
				tang->y = tangents[i * va.size + 1];
				tang->z = tangents[i * va.size + 2];
			}
			break;
		}
#endif
		case IQM_BLENDINDEXES:
		{
			if (va.format != IQM_UBYTE || va.size != 4)
			{
				common->Warning("idRenderModelIQM::loadMeshes(%s): invalid Vertex indexes array", m_name.c_str());
				return false;
			}

			byte * indexes = (byte *)&buff[va.offset];
			//convert verts
			numIndex = va.size;
			blendIndex.SetNum(iqmHdr->num_vertexes * numIndex);
			for (i = 0; i < iqmHdr->num_vertexes; i++)
			{
				for (j = 0; j < va.size; j++)
					blendIndex[i * numIndex + j] = indexes[i * numIndex + j];
			}

			break;
		}
		case IQM_BLENDWEIGHTS:
		{
			if (va.format != IQM_UBYTE || va.size != 4)
			{
				common->Warning("idRenderModelIQM::loadMeshes(%s): invalid Vertex wheight array", m_name.c_str());
				return false;
			}

			float * weights = (float *)&buff[va.offset];
			btByteSwap::LittleRevBytes(weights, sizeof(float), sizeof(weights) / sizeof(float));
			//convert verts
			numWeights = va.size;
			blendWeights.SetNum(iqmHdr->num_vertexes * numWeights);
			for (i = 0; i < iqmHdr->num_vertexes; i++)
			{
				for (j = 0; j < va.size; j++)
					blendWeights[i * numWeights + j] = weights[i * numWeights + j];
			}

			break;
		}
		case IQM_COLOR:
		{
			if (va.format != IQM_UBYTE || va.size != 4)
			{
				common->Warning("idRenderModelIQM::loadMeshes(%s): invalid Vertex position array", m_name.c_str());
				return false;
			}
			break;
		}

		//no use custom vertex atribs
		default:
			break;
		}
	}

	if (blendIndex.Size() <= 0 && blendWeights.Size() <= 0)    // have blend data
	{
		//need the same number of Index and Weights per vertex
		idassert(numIndex == numWeights);

		unsigned int i, j, k;
		for (i = 0; i < iqmHdr->num_vertexes; i++)
		{
			btVertexWheight vertexWheight;

			//normalise vertex wheight
			float total = 0.0f;
			for (k = 0; k < numWeights; k++)
			{
				total += blendWeights[i * numWeights + k];
			}

			for (j = 0; j < numIndex; j++)
			{
				vertexWheight.addWeight(i, blendWeights[i * numWeights + j] / total, blendIndex[i * numIndex + j]);
			}
			vertexWheight.finalize();
			weightInfo.Append(vertexWheight);
		}
	}

	return true;
}

bool idRenderModelIQM::loadMeshes(const byte * buff)
{
	unsigned int i = 0;

	const char *str = iqmHdr->ofs_text ? (char *)&buff[iqmHdr->ofs_text] : "";
	
	//load indices
	iqmtriangle *tris = (iqmtriangle *)&buff[iqmHdr->ofs_triangles];
	//reserve space for the triagles
	reserveTriangles(iqmHdr->num_triangles);
	for (unsigned int i = 0; i < iqmHdr->num_triangles; i++)
	{
		iqmtriangle tri = tris[i];
		triangles[i * 3 + 0] = tri.vertex[0];
		triangles[i * 3 + 1] = tri.vertex[1];
		triangles[i * 3 + 2] = tri.vertex[2];
	}

	// load meshes
	iqmmesh *IqmMeshes;
	IqmMeshes = (iqmmesh *)(buff + iqmHdr->ofs_meshes);
	m_meshes.SetGranularity(1);
	m_meshes.SetNum(iqmHdr->num_meshes);
	for (unsigned int i = 0; i < iqmHdr->num_meshes; i++)
	{
		iqmmesh &m = IqmMeshes[i];
		btIqmMesh*	meshInst = dynamic_cast<btIqmMesh*>(&m_meshes[i]);
		LL(m.name);
		LL(m.material);
		LL(m.num_vertexes);
		LL(m.num_triangles);
		LL(m.first_triangle);
		LL(m.first_vertex);

		meshInst->setupMeshData(&str[m.name], &str[m.material], m);
		//gen the mesh reference
		meshInst->BuildTheInternalMesh(this);
	}

	return true;
}

#if 1
bool idRenderModelIQM::LoadBinaryModel(idFile * file, const ID_TIME_T sourceTimeStamp)
{
	int				size;
	void*			buffer;

	size = fileSystem->ReadFile(m_name.c_str(), &buffer, NULL);
	if (!size || size < 0)
		return false;

	iqmHdr = (iqmheader*)buffer;

	//check header magic
	if (strcmp(iqmHdr->magic, IQM_MAGIC) != 0)
	{
		common->Warning("idRenderModelIQM::LoadModel(%s): can't load, not a valid IQM model", m_name.c_str());

		return false;
	}

	//check the model version
	LL(iqmHdr->version);
	if (iqmHdr->version != IQM_VERSION)
	{
		common->Warning("idRenderModelIQM::LoadModel(%s): has wrong version (%i should be %i)", m_name.c_str(), iqmHdr->version, IQM_VERSION);
		return false;
	}

	LL(iqmHdr->filesize);
	if (iqmHdr->filesize > (16 << 20))
	{
		common->Warning("idRenderModelIQM::LoadModel(%s): invalid file size (%i don't load files bigger than %i)",
			m_name.c_str(), iqmHdr->filesize, (16 << 20));
		return false;
	}

	//set the true headers values
	LL(iqmHdr->flags);
	LL(iqmHdr->num_text);
	LL(iqmHdr->ofs_text);
	LL(iqmHdr->num_meshes);
	LL(iqmHdr->ofs_meshes);
	LL(iqmHdr->num_vertexarrays);
	LL(iqmHdr->num_vertexes);
	LL(iqmHdr->ofs_vertexarrays);
	LL(iqmHdr->num_triangles);
	LL(iqmHdr->ofs_triangles);
	LL(iqmHdr->ofs_adjacency);
	LL(iqmHdr->num_joints);
	LL(iqmHdr->ofs_joints);
	LL(iqmHdr->num_poses);
	LL(iqmHdr->ofs_poses);
	LL(iqmHdr->num_anims);
	LL(iqmHdr->ofs_anims);
	LL(iqmHdr->num_frames);
	LL(iqmHdr->num_framechannels);
	LL(iqmHdr->ofs_frames);
	LL(iqmHdr->ofs_bounds);
	LL(iqmHdr->num_comment);
	LL(iqmHdr->ofs_comment);
	LL(iqmHdr->num_extensions);
	LL(iqmHdr->ofs_extensions);

	if (iqmHdr->num_joints > 0 && !loadJoints((byte*)buffer))
	{
		common->Warning("idRenderModelIQM::LoadModel(%s): can't load, model don't have joints", m_name.c_str());
		return false;
	}

	if (iqmHdr->num_vertexarrays > 0 && !loadVerterxArray((byte*)buffer))
	{
		common->Warning("idRenderModelIQM::LoadModel(%s): can't load, model don't have vertex arrays", m_name.c_str());
		return false;
	}

	if (iqmHdr->num_meshes > 0 && !loadMeshes((byte*)buffer))
	{
		common->Warning("idRenderModelIQM::LoadModel(%s): can't load, model don't have meshes", m_name.c_str());
		return false;
	}

#if 0
	if (hdr->num_anims > 0 && !loadiqmanims(filename, hdr, buf))
		return false;
#endif

	return true;
}

void idRenderModelIQM::WriteBinaryModel(idFile * file, ID_TIME_T * _timeStamp) const
{
}
#endif

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

int idRenderModelIQM::Memory(void) const
{
	return 0;
}

idRenderModel * idRenderModelIQM::InstantiateDynamicModel(const renderEntity_s * ent,
								const viewDef_t * view,
								idRenderModel * cachedModel)
{
	return NULL;
}

int idRenderModelIQM::NearestJoint(int surfaceNum, int a, int b, int c) const
{
	return 0;
}


void btIqmMesh::setupMeshData(const idStr name, const idStr mtr, const iqmmesh mesh)
{
	//the number of vertex of the mesh
	this->m_meshNumTris = mesh.num_triangles;
	this->m_meshNumVerts = mesh.num_vertexes;

	this->m_meshName = name;

	//load mesh material intance
	if (mtr.IsEmpty())
		m_shader = NULL;
	else
		m_shader = declManager->FindMaterial(mtr);

}

void btIqmMesh::BuildTheInternalMesh(idRenderModelIQM* modelRef)
{
	//error model loader not referenced
	idassert(modelRef);
	unsigned int i, j, k;

	//setup model, iqm meshes is loaded in the base bone pose
	idDrawVert* basePose = (idDrawVert*)Mem_ClearedAlloc(modelRef->vertexPos.Num() * sizeof(*basePose), TAG_MD5_BASE);
	for (i = 0, i = 0; i < modelRef->vertexPos.Num(); i++)
	{
		basePose[i].Clear();
		basePose[i].xyz = modelRef->vertexPos[i];
		basePose[i].SetTexCoord(modelRef->vertexUV[i]);
		basePose[i].SetNormal(modelRef->vertexNor[i]);
		basePose[i].SetTangent(modelRef->vertexTan[i]);

		//set the vertex influence
		//weightInfo[i].setVertWheight(&basePose[i].color2, &basePose[i].color);
		for (j = 0; j < modelRef->weightInfo[i].getNumWheights(); j++)
		{
			basePose[i].color[j] = modelRef->weightInfo[i].getIndexAt(j);
			basePose[i].color2[j] = idMath::Ftob(modelRef->weightInfo[i].getWheightAt(j) * 255.0f);
		}
	}

	unsigned int numMeshJoints = 0;
	idList< bool > jointIsUsed;
	jointIsUsed.SetNum(modelRef->m_joints.Num());
	for (i = 0; i < jointIsUsed.Num(); i++)
	{
		jointIsUsed[i] = false;
	}

	for (i = 0; i < modelRef->weightInfo.Size(); i++)
	{
		btVertexWheight wi = modelRef->weightInfo[i];
		for (j = 0; j < wi.getNumWheights(); j++)
		{
			if (!jointIsUsed[wi.getIndexAt(j)])
			{
				jointIsUsed[wi.getIndexAt(j)] = true;
				numMeshJoints++;
			}
		}
	}

	// build the deformInfo and collect a final base pose with the mirror
	// seam verts properly including the bone weights
	m_deformInfo = R_BuildDeformInfo(modelRef->vertexPos.Num(), basePose,
		modelRef->triangles.Num(), modelRef->triangles.Ptr(),
		m_shader->UseUnsmoothedTangents());

	for (int i = 0; i < m_deformInfo->numOutputVerts; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			if (m_deformInfo->verts[i].color[j] >= m_numMeshJoints)
			{
				idLib::FatalError("Bad joint index");
			}
		}
	}

	Mem_Free(basePose);
}

void btIqmMesh::Clear(void)
{
	btRenderMeshSkined::Clear();
}

btIQMmodel::btIQMmodel(void)
{
}

btIQMmodel::~btIQMmodel(void)
{
}

void btIQMmodel::reserveTriangles(unsigned int num)
{
	triangles.SetGranularity(1);
	triangles.SetNum(num * 3);
}

void btIQMmodel::reserveVertexes(unsigned int num)
{
	//reserve the vertex space
	vertexPos.SetGranularity(1);
	vertexPos.SetNum(num);
	vertexUV.SetGranularity(1);
	vertexUV.SetNum(num);
	vertexNor.SetGranularity(1);
	vertexNor.SetNum(num);
	vertexNor.SetGranularity(1);
	vertexTan.SetNum(num);
}

void btVertexWheight::addWeight(const unsigned int vertex, const float weight, const byte bone)
{
	unsigned int k;
	if (weight <= 1e-3) 
		return;

	for (k = 0; k < m_sorted; k++)
	{
		if (weight > m_weights[k])
		{
			for (int l = Min<int>(m_sorted - 1, 2); l >= k; l--)
			{
				m_weights[l + 1] = m_weights[l];
				m_indexes[l + 1] = m_indexes[l];
			}
			m_weights[k] = weight;
			m_indexes[k] = bone;
			if (m_sorted < 4) m_sorted++;
			return;
		}
	}
	if (m_sorted >= 4)
		return;

	m_weights[m_sorted] = weight;
	m_indexes[m_sorted] = bone;
	m_sorted++;
}

void btVertexWheight::finalize(void)
{
	for (int j = 0; j < 4 - m_sorted; j++)
	{ 
		m_weights[m_sorted + j] = 0;
		m_indexes[m_sorted + j] = 0;
	}

	if (m_sorted <= 0) 
		return;

	float total = 0.0f;
	for (int j = 0; j < m_sorted; j++)
		total += m_weights[j];

	total = 1.0 / total;
	for (int j = 0; j < m_sorted; j++)
		m_weights[j] *= total;
}

uint32 btVertexWheight::getNumWheights(void) const
{
	idassert(m_weights.Num() == m_indexes.Num());
	return m_weights.Num();
}

const float btVertexWheight::getWheightAt(const unsigned int p)
{
	idassert(m_weights.Num() > 0 && m_weights.Num() > p);
	return m_weights[p];
}

const byte btVertexWheight::getIndexAt(const unsigned int p)
{
	idassert(m_indexes.Num() > 0 && m_indexes.Num() > p);
	return m_indexes[p];
}

void btVertexWheight::setVertWheight(byte *wheight[4], byte *index[4])
{
	unsigned int i;
	for (i = 0; i < m_indexes.Num(); i++)
	{
		*index[i] = m_weights[i];
	}

	for (i = 0; i < m_weights.Num(); i++)
	{
		//*wheight[i] = m_weights[i];
		*wheight[i] = idMath::Ftob(m_weights[i] * 255.0f);
	}
}

