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

#include "renderer/tr_local.h"
#include "Model_static.h"
#include "Model_skined.h"

idRenderModelStatic::idRenderModelStatic(void) : idRenderModelLocal()
{
}

idRenderModelStatic::~idRenderModelStatic(void)
{
}

/*
================
idRenderModelStatic::InitFromFile
================
*/
void idRenderModelStatic::InitFromFile(const char* fileName)
{
	bool loaded;

	InitEmpty(fileName);

	//try load the model format
	loaded = Load();
	m_reloadable = true;

	if (!loaded)
	{
		common->Warning("Couldn't load model: '%s'", m_name.c_str());
		MakeDefaultModel();
		return;
	}

	// it is now available for use
	m_purged = false;

	// create the bounds for culling and dynamic surface creation
	FinishSurfaces();
}

/*
========================
idRenderModelStatic::LoadBinaryModel
========================
*/
bool idRenderModelStatic::LoadBinaryModel(idFile* file, const ID_TIME_T sourceTimeStamp)
{
	if (file == NULL)
		return false;

	unsigned int magic = 0;
	file->ReadBig(magic);
	if (magic != BRM_MAGIC)
		return false;

	file->ReadBig(m_timeStamp);

	// RB: source might be from .resources, so we ignore the time stamp and assume a release build
	if (!fileSystem->InProductionMode() && (sourceTimeStamp != FILE_NOT_FOUND_TIMESTAMP) && (sourceTimeStamp != 0) && (sourceTimeStamp != m_timeStamp))
	{
		return false;
	}
	// RB end

	common->UpdateLevelLoadPacifier();

	int numSurfaces;
	file->ReadBig(numSurfaces);
	m_surfaces.SetNum(numSurfaces);
	for (int i = 0; i < m_surfaces.Num(); i++)
	{
		file->ReadBig(m_surfaces[i].id);
		idStr materialName;
		file->ReadString(materialName);
		if (materialName.IsEmpty())
			m_surfaces[i].shader = NULL;
		else
			m_surfaces[i].shader = declManager->FindMaterial(materialName);

		bool isGeometry;
		file->ReadBig(isGeometry);
		m_surfaces[i].geometry = NULL;
		if (isGeometry)
		{
			bool temp;

			m_surfaces[i].geometry = R_AllocStaticTriSurf();

			// Read the contents of srfTriangles_t
			srfTriangles_t& tri = *m_surfaces[i].geometry;

			file->ReadVec3(tri.bounds[0]);
			file->ReadVec3(tri.bounds[1]);

			int ambientViewCount = 0;	// FIXME: remove
			file->ReadBig(ambientViewCount);
			file->ReadBig(tri.generateNormals);
			file->ReadBig(tri.tangentsCalculated);
			file->ReadBig(tri.perfectHull);
			file->ReadBig(tri.referencedIndexes);

			file->ReadBig(tri.numVerts);
			tri.verts = NULL;
			int numInFile = 0;
			file->ReadBig(numInFile);
			if (numInFile > 0)
			{
				R_AllocStaticTriSurfVerts(&tri, tri.numVerts);
				assert(tri.verts != NULL);
				for (int j = 0; j < tri.numVerts; j++)
				{
					file->ReadVec3(tri.verts[j].xyz);
					file->ReadBigArray(tri.verts[j].st, 2);
					file->ReadBigArray(tri.verts[j].normal, 4);
					file->ReadBigArray(tri.verts[j].tangent, 4);
					file->ReadBigArray(tri.verts[j].color, sizeof(tri.verts[j].color) / sizeof(tri.verts[j].color[0]));
					file->ReadBigArray(tri.verts[j].color2, sizeof(tri.verts[j].color2) / sizeof(tri.verts[j].color2[0]));
				}
			}

			file->ReadBig(numInFile);
			if (numInFile == 0)
			{
				tri.preLightShadowVertexes = NULL;
			}
			else
			{
				R_AllocStaticTriSurfPreLightShadowVerts(&tri, numInFile);
				for (int j = 0; j < numInFile; j++)
				{
					file->ReadVec4(tri.preLightShadowVertexes[j].xyzw);
				}
			}

			file->ReadBig(tri.numIndexes);
			tri.indexes = NULL;
			tri.silIndexes = NULL;
			if (tri.numIndexes > 0)
			{
				R_AllocStaticTriSurfIndexes(&tri, tri.numIndexes);
				file->ReadBigArray(tri.indexes, tri.numIndexes);
			}
			file->ReadBig(numInFile);
			if (numInFile > 0)
			{
				R_AllocStaticTriSurfSilIndexes(&tri, tri.numIndexes);
				file->ReadBigArray(tri.silIndexes, tri.numIndexes);
			}

			file->ReadBig(tri.numMirroredVerts);
			tri.mirroredVerts = NULL;
			if (tri.numMirroredVerts > 0)
			{
				R_AllocStaticTriSurfMirroredVerts(&tri, tri.numMirroredVerts);
				file->ReadBigArray(tri.mirroredVerts, tri.numMirroredVerts);
			}

			file->ReadBig(tri.numDupVerts);
			tri.dupVerts = NULL;
			if (tri.numDupVerts > 0)
			{
				R_AllocStaticTriSurfDupVerts(&tri, tri.numDupVerts);
				file->ReadBigArray(tri.dupVerts, tri.numDupVerts * 2);
			}

			file->ReadBig(tri.numSilEdges);
			tri.silEdges = NULL;
			if (tri.numSilEdges > 0)
			{
				R_AllocStaticTriSurfSilEdges(&tri, tri.numSilEdges);
				assert(tri.silEdges != NULL);
				for (int j = 0; j < tri.numSilEdges; j++)
				{
					file->ReadBig(tri.silEdges[j].p1);
					file->ReadBig(tri.silEdges[j].p2);
					file->ReadBig(tri.silEdges[j].v1);
					file->ReadBig(tri.silEdges[j].v2);
				}
			}

			file->ReadBig(temp);
			tri.dominantTris = NULL;
			if (temp)
			{
				R_AllocStaticTriSurfDominantTris(&tri, tri.numVerts);
				assert(tri.dominantTris != NULL);
				for (int j = 0; j < tri.numVerts; j++)
				{
					file->ReadBig(tri.dominantTris[j].v2);
					file->ReadBig(tri.dominantTris[j].v3);
					file->ReadFloat(tri.dominantTris[j].normalizationScale[0]);
					file->ReadFloat(tri.dominantTris[j].normalizationScale[1]);
					file->ReadFloat(tri.dominantTris[j].normalizationScale[2]);
				}
			}

			file->ReadBig(tri.numShadowIndexesNoFrontCaps);
			file->ReadBig(tri.numShadowIndexesNoCaps);
			file->ReadBig(tri.shadowCapPlaneBits);

			tri.ambientSurface = NULL;
			tri.nextDeferredFree = NULL;
			tri.indexCache = 0;
			tri.ambientCache = 0;
			tri.shadowCache = 0;
		}
	}

	file->ReadVec3(m_bounds[0]);
	file->ReadVec3(m_bounds[1]);

	file->ReadBig(m_overlaysAdded);
	file->ReadBig(m_lastModifiedFrame);
	file->ReadBig(m_lastArchivedFrame);
	file->ReadString(m_name);
	file->ReadBig(m_isStaticWorldModel);
	file->ReadBig(m_defaulted);
	file->ReadBig(m_purged);
	file->ReadBig(m_fastLoad);
	file->ReadBig(m_reloadable);
	file->ReadBig(m_levelLoadReferenced);		// should this actually be saved/loaded?
	file->ReadBig(m_hasDrawingSurfaces);
	file->ReadBig(m_hasInteractingSurfaces);
	file->ReadBig(m_hasShadowCastingSurfaces);

	return true;
}

/*
========================
idRenderModelStatic::WriteBinaryModel
========================
*/
void idRenderModelStatic::WriteBinaryModel(idFile* file, ID_TIME_T* _timeStamp) const
{
	if (file == NULL)
	{
		common->Printf("Failed to WriteBinaryModel\n");
		return;
	}

	file->WriteBig(BRM_MAGIC);

	if (_timeStamp != NULL)
		file->WriteBig(*_timeStamp);
	else
		file->WriteBig(m_timeStamp);

	file->WriteBig(m_surfaces.Num());
	for (int i = 0; i < m_surfaces.Num(); i++)
	{
		file->WriteBig(m_surfaces[i].id);
		if (m_surfaces[i].shader != NULL && m_surfaces[i].shader->GetName() != NULL)
		{
			file->WriteString(m_surfaces[i].shader->GetName());
		}
		else
		{
			file->WriteString("");
		}

		file->WriteBig(m_surfaces[i].geometry != NULL);
		if (m_surfaces[i].geometry != NULL)
		{
			srfTriangles_t& tri = *m_surfaces[i].geometry;

			file->WriteVec3(tri.bounds[0]);
			file->WriteVec3(tri.bounds[1]);

			int ambientViewCount = 0;	// FIXME: remove
			file->WriteBig(ambientViewCount);
			file->WriteBig(tri.generateNormals);
			file->WriteBig(tri.tangentsCalculated);
			file->WriteBig(tri.perfectHull);
			file->WriteBig(tri.referencedIndexes);

			// shadow models use numVerts but have no verts
			file->WriteBig(tri.numVerts);
			if (tri.verts != NULL)
			{
				file->WriteBig(tri.numVerts);
			}
			else
			{
				file->WriteBig((int)0);
			}

			if (tri.numVerts > 0 && tri.verts != NULL)
			{
				for (int j = 0; j < tri.numVerts; j++)
				{
					file->WriteVec3(tri.verts[j].xyz);
					file->WriteBigArray(tri.verts[j].st, 2);
					file->WriteBigArray(tri.verts[j].normal, 4);
					file->WriteBigArray(tri.verts[j].tangent, 4);
					file->WriteBigArray(tri.verts[j].color, sizeof(tri.verts[j].color) / sizeof(tri.verts[j].color[0]));
					file->WriteBigArray(tri.verts[j].color2, sizeof(tri.verts[j].color2) / sizeof(tri.verts[j].color2[0]));
				}
			}

			if (tri.preLightShadowVertexes != NULL)
			{
				file->WriteBig(tri.numVerts * 2);
				for (int j = 0; j < tri.numVerts * 2; j++)
				{
					file->WriteVec4(tri.preLightShadowVertexes[j].xyzw);
				}
			}
			else
			{
				file->WriteBig((int)0);
			}

			file->WriteBig(tri.numIndexes);

			if (tri.numIndexes > 0)
			{
				file->WriteBigArray(tri.indexes, tri.numIndexes);
			}

			if (tri.silIndexes != NULL)
			{
				file->WriteBig(tri.numIndexes);
			}
			else
			{
				file->WriteBig((int)0);
			}

			if (tri.numIndexes > 0 && tri.silIndexes != NULL)
			{
				file->WriteBigArray(tri.silIndexes, tri.numIndexes);
			}

			file->WriteBig(tri.numMirroredVerts);
			if (tri.numMirroredVerts > 0)
			{
				file->WriteBigArray(tri.mirroredVerts, tri.numMirroredVerts);
			}

			file->WriteBig(tri.numDupVerts);
			if (tri.numDupVerts > 0)
			{
				file->WriteBigArray(tri.dupVerts, tri.numDupVerts * 2);
			}

			file->WriteBig(tri.numSilEdges);
			if (tri.numSilEdges > 0)
			{
				for (int j = 0; j < tri.numSilEdges; j++)
				{
					file->WriteBig(tri.silEdges[j].p1);
					file->WriteBig(tri.silEdges[j].p2);
					file->WriteBig(tri.silEdges[j].v1);
					file->WriteBig(tri.silEdges[j].v2);
				}
			}

			file->WriteBig(tri.dominantTris != NULL);
			if (tri.dominantTris != NULL)
			{
				for (int j = 0; j < tri.numVerts; j++)
				{
					file->WriteBig(tri.dominantTris[j].v2);
					file->WriteBig(tri.dominantTris[j].v3);
					file->WriteFloat(tri.dominantTris[j].normalizationScale[0]);
					file->WriteFloat(tri.dominantTris[j].normalizationScale[1]);
					file->WriteFloat(tri.dominantTris[j].normalizationScale[2]);
				}
			}

			file->WriteBig(tri.numShadowIndexesNoFrontCaps);
			file->WriteBig(tri.numShadowIndexesNoCaps);
			file->WriteBig(tri.shadowCapPlaneBits);
		}
	}

	file->WriteVec3(m_bounds[0]);
	file->WriteVec3(m_bounds[1]);
	file->WriteBig(m_overlaysAdded);
	file->WriteBig(m_lastModifiedFrame);
	file->WriteBig(m_lastArchivedFrame);
	file->WriteString(m_name);

	// shadowHull

	file->WriteBig(m_isStaticWorldModel);
	file->WriteBig(m_defaulted);
	file->WriteBig(m_purged);
	file->WriteBig(m_fastLoad);
	file->WriteBig(m_reloadable);
	file->WriteBig(m_levelLoadReferenced);
	file->WriteBig(m_hasDrawingSurfaces);
	file->WriteBig(m_hasInteractingSurfaces);
	file->WriteBig(m_hasShadowCastingSurfaces);
}

// RB begin
void idRenderModelStatic::ExportOBJ(idFile* objFile, idFile* mtlFile, ID_TIME_T* _timeStamp) const
{
	if (objFile == NULL || mtlFile == NULL)
	{
		common->Printf("Failed to ExportOBJ\n");
		return;
	}

	//objFile->Printf( "// generated by %s\n//\n\n", ENGINE_VERSION );

	int numVerts = 0;
	idList< const idMaterial* > materials;

	for (int i = 0; i < m_surfaces.Num(); i++)
	{
		// shadow models use numVerts but have no verts
		if ((m_surfaces[i].geometry != NULL) && (m_surfaces[i].geometry->numVerts > 0) && (m_surfaces[i].geometry->numIndexes > 0) && (m_surfaces[i].geometry->verts != NULL))
		{
			objFile->Printf("o Geometry.%i\n", m_surfaces[i].id);

			srfTriangles_t& tri = *m_surfaces[i].geometry;

			//file->WriteVec3( tri.bounds[0] );
			//file->WriteVec3( tri.bounds[1] );

			// TODO print additional info ?

			//file->WriteBig( tri.generateNormals );
			//file->WriteBig( tri.tangentsCalculated );
			//file->WriteBig( tri.perfectHull );
			//file->WriteBig( tri.referencedIndexes );

			if (tri.numVerts > 0 && tri.verts != NULL)
			{
				for (int j = 0; j < tri.numVerts; j++)
				{
					objFile->Printf("v %1.6f %1.6f %1.6f\n", tri.verts[j].xyz.x, tri.verts[j].xyz.y, tri.verts[j].xyz.z);
				}

				for (int j = 0; j < tri.numVerts; j++)
				{
					const idVec2 vST = tri.verts[j].GetTexCoord();

					objFile->Printf("vt %1.6f %1.6f\n", vST.x, 1.0f - vST.y);
				}

				for (int j = 0; j < tri.numVerts; j++)
				{
					const idVec3 n = tri.verts[j].GetNormalRaw();

					objFile->Printf("vn %1.6f %1.6f %1.6f\n", n.x, n.y, n.z);
				}

				//file->WriteBigArray( tri.verts[j].st, 2 );
				//file->WriteBigArray( tri.verts[j].normal, 4 );
				//file->WriteBigArray( tri.verts[j].tangent, 4 );
				//file->WriteBigArray( tri.verts[j].color, sizeof( tri.verts[j].color ) / sizeof( tri.verts[j].color[0] ) );
				//file->WriteBigArray( tri.verts[j].color2, sizeof( tri.verts[j].color2 ) / sizeof( tri.verts[j].color2[0] ) );
			}

			if (m_surfaces[i].shader != NULL && m_surfaces[i].shader->GetName() != NULL)
			{
				objFile->Printf("usemtl %s\n", m_surfaces[i].shader->GetName());

				materials.AddUnique(m_surfaces[i].shader);
			}

			objFile->Printf("s 1\n");

			for (int j = 0; j < tri.numIndexes; j += 3)
			{
				objFile->Printf("f %i/%i/%i %i/%i/%i %i/%i/%i\n",
					tri.indexes[j + 2] + 1 + numVerts,
					tri.indexes[j + 2] + 1 + numVerts,
					tri.indexes[j + 2] + 1 + numVerts,

					tri.indexes[j + 1] + 1 + numVerts,
					tri.indexes[j + 1] + 1 + numVerts,
					tri.indexes[j + 1] + 1 + numVerts,

					tri.indexes[j + 0] + 1 + numVerts,
					tri.indexes[j + 0] + 1 + numVerts,
					tri.indexes[j + 0] + 1 + numVerts);
			}

			objFile->Printf("\n");

			numVerts += tri.numVerts;
		}
	}

	for (int i = 0; i < materials.Num(); i++)
	{
		const idMaterial* material = materials[i];

		mtlFile->Printf("newmtl %s\n", material->GetName());

		if (material->GetFastPathDiffuseImage())
		{
			idStr path = material->GetFastPathDiffuseImage()->GetName();
			path.SlashesToBackSlashes();
			path.DefaultFileExtension(".tga");

			mtlFile->Printf("\tmap_Kd //..\\..\\..\\%s\n", path.c_str());
		}
		else if (material->GetEditorImage())
		{
			idStr path = material->GetEditorImage()->GetName();
			path.SlashesToBackSlashes();
			path.DefaultFileExtension(".tga");

			mtlFile->Printf("\tmap_Kd //..\\..\\..\\%s\n", path.c_str());
		}


		mtlFile->Printf("\n");
	}
}
// RB end

bool idRenderModelStatic::Load(void)
{
	MakeDefaultModel();
	return true;
}

/*
================
idRenderModelStatic::IsDynamicModel
================
*/
dynamicModel_t idRenderModelStatic::IsDynamicModel(void) const
{
	// dynamic subclasses will override this
	return DM_STATIC;
}

/*
================
idRenderModelStatic::InstantiateDynamicModel
================
*/
idRenderModel* idRenderModelStatic::InstantiateDynamicModel(const struct renderEntity_s* ent, const viewDef_t* view, idRenderModel* cachedModel)
{
	if (cachedModel)
	{
		delete cachedModel;
		cachedModel = NULL;
	}
	common->Error("InstantiateDynamicModel called on static model '%s'", m_name.c_str());
	return NULL;
}

/*
================
idRenderModelStatic::NumJoints
================
*/
int idRenderModelStatic::NumJoints(void) const
{
	return 0;
}

/*
================
idRenderModelStatic::GetJoints
================
*/
const  idList<btGameJoint> idRenderModelStatic::GetJoints(void) const
{
	return idList<btGameJoint>();
}

/*
================
idRenderModelStatic::GetJointHandle
================
*/
jointHandle_t idRenderModelStatic::GetJointHandle(const char* name) const
{
	return INVALID_JOINT;
}

/*
================
idRenderModelStatic::GetJointName
================
*/
const char* idRenderModelStatic::GetJointName(jointHandle_t handle) const
{
	return "";
}

/*
================
idRenderModelStatic::GetDefaultPose
================
*/
const idJointQuat* idRenderModelStatic::GetDefaultPose(void) const
{
	return NULL;
}

/*
================
idRenderModelStatic::NearestJoint
================
*/
int idRenderModelStatic::NearestJoint(int surfaceNum, int a, int b, int c) const
{
	return INVALID_JOINT;
}
