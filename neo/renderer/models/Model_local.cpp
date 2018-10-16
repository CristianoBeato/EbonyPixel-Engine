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

#include "Model_local.h"
#include "internal/Model_skined.h"
#include "renderer/tr_local.h"

idCVar idRenderModelLocal::r_mergeModelSurfaces("r_mergeModelSurfaces", "1", CVAR_BOOL | CVAR_RENDERER, "combine model surfaces with the same material");
idCVar idRenderModelLocal::r_slopVertex("r_slopVertex", "0.01", CVAR_RENDERER, "merge xyz coordinates this far apart");
idCVar idRenderModelLocal::r_slopTexCoord("r_slopTexCoord", "0.001", CVAR_RENDERER, "merge texture coordinates this far apart");
idCVar idRenderModelLocal::r_slopNormal("r_slopNormal", "0.02", CVAR_RENDERER, "merge normals that dot less than this");

/*
================
idRenderModelLocal::idRenderModelLocal
================
*/
idRenderModelLocal::idRenderModelLocal(void)
{
	m_name = "<undefined>";
	m_bounds.Clear();
	m_lastModifiedFrame = 0;
	m_lastArchivedFrame = 0;
	m_overlaysAdded = 0;
	m_isStaticWorldModel = false;
	m_defaulted = false;
	m_purged = false;
	m_fastLoad = false;
	m_reloadable = true;
	m_levelLoadReferenced = false;
	m_hasDrawingSurfaces = true;
	m_hasInteractingSurfaces = true;
	m_hasShadowCastingSurfaces = true;
	m_timeStamp = 0;
}


/*
================
idRenderModelLocal::~idRenderModelLocal
================
*/
idRenderModelLocal::~idRenderModelLocal(void)
{
	PurgeModel();
}


/*
================
idRenderModelLocal::PurgeModel
================
*/
void idRenderModelLocal::PurgeModel(void)
{
	for (int i = 0; i < m_surfaces.Num(); i++)
	{
		modelSurface_t* surf = &m_surfaces[i];

		if (surf->geometry)
			R_FreeStaticTriSurf(surf->geometry);
	}
	m_surfaces.Clear();
	m_purged = true;
}

/*
==============
idRenderModelLocal::Print
==============
*/
void idRenderModelLocal::Print(void) const
{
	common->Printf("%s\n", m_name.c_str());
	common->Printf("Static model.\n");
	common->Printf("bounds: (%f %f %f) to (%f %f %f)\n",
		m_bounds[0][0], m_bounds[0][1], m_bounds[0][2],
		m_bounds[1][0], m_bounds[1][1], m_bounds[1][2]);

	common->Printf("    verts  tris material\n");
	for (int i = 0; i < NumSurfaces(); i++)
	{
		const modelSurface_t*	surf = Surface(i);

		srfTriangles_t* tri = surf->geometry;
		const idMaterial* material = surf->shader;

		if (!tri)
		{
			common->Printf("%2i: %s, NULL surface geometry\n", i, material->GetName());
			continue;
		}

		common->Printf("%2i: %5i %5i %s", i, tri->numVerts, tri->numIndexes / 3, material->GetName());
		if (tri->generateNormals)
			common->Printf(" (smoothed)\n");
		else
			common->Printf("\n");
	}
}

/*
==============
idRenderModelLocal::Memory
==============
*/
int idRenderModelLocal::Memory(void) const
{
	int	totalBytes = 0;

	totalBytes += sizeof(*this);
	totalBytes += m_name.DynamicMemoryUsed();
	totalBytes += m_surfaces.MemoryUsed();

	for (int j = 0; j < NumSurfaces(); j++)
	{
		const modelSurface_t*	surf = Surface(j);
		if (!surf->geometry)
		{
			continue;
		}
		totalBytes += R_TriSurfMemory(surf->geometry);
	}

	return totalBytes;
}

/*
==============
idRenderModelLocal::List
==============
*/
void idRenderModelLocal::List(void) const
{
	int	totalTris = 0;
	int	totalVerts = 0;
	int	totalBytes = 0;

	totalBytes = Memory();

	char	closed = 'C';
	for (int j = 0; j < NumSurfaces(); j++)
	{
		const modelSurface_t*	surf = Surface(j);
		if (!surf->geometry)
		{
			continue;
		}
		if (!surf->geometry->perfectHull)
		{
			closed = ' ';
		}
		totalTris += surf->geometry->numIndexes / 3;
		totalVerts += surf->geometry->numVerts;
	}
	common->Printf("%c%4ik %3i %4i %4i %s", closed, totalBytes / 1024, NumSurfaces(), totalVerts, totalTris, Name());

	if (IsDynamicModel() == DM_CACHED)
	{
		common->Printf(" (DM_CACHED)");
	}
	if (IsDynamicModel() == DM_CONTINUOUS)
	{
		common->Printf(" (DM_CONTINUOUS)");
	}
	if (m_defaulted)
	{
		common->Printf(" (DEFAULTED)");
	}
	if (m_bounds[0][0] >= m_bounds[1][0])
	{
		common->Printf(" (EMPTY BOUNDS)");
	}
	if (m_bounds[1][0] - m_bounds[0][0] > 100000)
	{
		common->Printf(" (HUGE BOUNDS)");
	}

	common->Printf("\n");
}

/*
================
idRenderModelLocal::IsDefaultModel
================
*/
bool idRenderModelLocal::IsDefaultModel(void) const
{
	return m_defaulted;
}


/*
================
idRenderModelLocal::PartialInitFromFile
================
*/
void idRenderModelLocal::PartialInitFromFile(const char* fileName)
{
	m_fastLoad = true;
	InitFromFile(fileName);
}

/*
================
idRenderModelLocal::LoadModel
================
*/
void idRenderModelLocal::LoadModel(void)
{
	PurgeModel();
	InitFromFile(m_name);
}

/*
================
idRenderModelLocal::InitEmpty
================
*/
void idRenderModelLocal::InitEmpty(const char* fileName)
{
	// model names of the form _area* are static parts of the
	// world, and have already been considered for optimized shadows
	// other model names are inline entity models, and need to be
	// shadowed normally
	if (!idStr::Cmpn(fileName, "_area", 5))
		m_isStaticWorldModel = true;
	else
		m_isStaticWorldModel = false;

	m_name = fileName;
	m_reloadable = false;	// if it didn't come from a file, we can't reload it
	PurgeModel();
	m_purged = false;
	m_bounds.Zero();
}

/*
================
idRenderModelLocal::AddSurface
================
*/
void idRenderModelLocal::AddSurface(modelSurface_t surface)
{
	m_surfaces.Append(surface);
	if (surface.geometry)
		m_bounds += surface.geometry->bounds;
}

/*
================
idRenderModelLocal::Name
================
*/
const char* idRenderModelLocal::Name(void) const
{
	return m_name;
}

/*
================
idRenderModelLocal::Timestamp
================
*/
ID_TIME_T idRenderModelLocal::Timestamp(void) const
{
	return m_timeStamp;
}

/*
================
idRenderModelLocal::NumSurfaces
================
*/
int idRenderModelLocal::NumSurfaces(void) const
{
	return m_surfaces.Num();
}

/*
================
idRenderModelLocal::NumBaseSurfaces
================
*/
int idRenderModelLocal::NumBaseSurfaces(void) const
{
	return m_surfaces.Num() - m_overlaysAdded;
}

/*
================
idRenderModelLocal::Surface
================
*/
const modelSurface_t* idRenderModelLocal::Surface(int surfaceNum) const
{
	return &m_surfaces[surfaceNum];
}

/*
================
idRenderModelLocal::AllocSurfaceTriangles
================
*/
srfTriangles_t* idRenderModelLocal::AllocSurfaceTriangles(int numVerts, int numIndexes) const
{
	srfTriangles_t* tri = R_AllocStaticTriSurf();
	R_AllocStaticTriSurfVerts(tri, numVerts);
	R_AllocStaticTriSurfIndexes(tri, numIndexes);
	return tri;
}

/*
================
idRenderModelLocal::FreeSurfaceTriangles
================
*/
void idRenderModelLocal::FreeSurfaceTriangles(srfTriangles_t* tris) const
{
	R_FreeStaticTriSurf(tris);
}

/*
================
idRenderModelLocal::IsStaticWorldModel
================
*/
bool idRenderModelLocal::IsStaticWorldModel(void) const
{
	return m_isStaticWorldModel;
}

/*
================
idRenderModelLocal::IsReloadable
================
*/
bool idRenderModelLocal::IsReloadable(void) const
{
	return m_reloadable;
}

/*
================
idRenderModelLocal::Bounds
================
*/
idBounds idRenderModelLocal::Bounds(const struct renderEntity_s* mdef) const
{
	return m_bounds;
}

/*
================
idRenderModelLocal::DepthHack
================
*/
float idRenderModelLocal::DepthHack(void) const
{
	return 0.0f;
}

/*
================
idRenderModelLocal::ModelHasDrawingSurfaces
================
*/
bool idRenderModelLocal::ModelHasDrawingSurfaces(void) const
{
	return m_hasDrawingSurfaces;
}

/*
================
idRenderModelLocal::ModelHasInteractingSurfaces
================
*/
bool idRenderModelLocal::ModelHasInteractingSurfaces(void) const
{
	return m_hasInteractingSurfaces;
}

/*
================
idRenderModelLocal::ModelHasShadowCastingSurfaces
================
*/
bool idRenderModelLocal::ModelHasShadowCastingSurfaces(void) const
{
	return m_hasShadowCastingSurfaces;
}

/*
================
idRenderModelLocal::MakeDefaultModel
================
*/
void idRenderModelLocal::MakeDefaultModel(void)
{
	m_defaulted = true;

	// throw out any surfaces we already have
	PurgeModel();

	// create one new surface
	modelSurface_t	surf;

	srfTriangles_t* tri = R_AllocStaticTriSurf();

	surf.shader = tr.defaultMaterial;
	surf.geometry = tri;

	R_AllocStaticTriSurfVerts(tri, 24);
	R_AllocStaticTriSurfIndexes(tri, 36);

	AddCubeFace(tri, idVec3(-1, 1, 1), idVec3(1, 1, 1), idVec3(1, -1, 1), idVec3(-1, -1, 1));
	AddCubeFace(tri, idVec3(-1, 1, -1), idVec3(-1, -1, -1), idVec3(1, -1, -1), idVec3(1, 1, -1));

	AddCubeFace(tri, idVec3(1, -1, 1), idVec3(1, 1, 1), idVec3(1, 1, -1), idVec3(1, -1, -1));
	AddCubeFace(tri, idVec3(-1, -1, 1), idVec3(-1, -1, -1), idVec3(-1, 1, -1), idVec3(-1, 1, 1));

	AddCubeFace(tri, idVec3(-1, -1, 1), idVec3(1, -1, 1), idVec3(1, -1, -1), idVec3(-1, -1, -1));
	AddCubeFace(tri, idVec3(-1, 1, 1), idVec3(-1, 1, -1), idVec3(1, 1, -1), idVec3(1, 1, 1));

	tri->generateNormals = true;

	AddSurface(surf);
	FinishSurfaces();
}

/*
================
idRenderModelLocal::FinishSurfaces

The mergeShadows option allows surfaces with different textures to share
silhouette edges for shadow calculation, instead of leaving shared edges
hanging.

If any of the original shaders have the noSelfShadow flag set, the surfaces
can't be merged, because they will need to be drawn in different order.

If there is only one surface, a separate merged surface won't be generated.

A model with multiple surfaces can't later have a skinned shader change the
state of the noSelfShadow flag.

-----------------

Creates mirrored copies of two sided surfaces with normal maps, which would
otherwise light funny.

Extends the bounds of deformed surfaces so they don't cull incorrectly at screen edges.

================
*/
void idRenderModelLocal::FinishSurfaces(void)
{
	int			i;
	int			totalVerts, totalIndexes;

	m_hasDrawingSurfaces = false;
	m_hasInteractingSurfaces = false;
	m_hasShadowCastingSurfaces = false;
	m_purged = false;

	// make sure we don't have a huge bounds even if we don't finish everything
	m_bounds.Zero();


	if (m_surfaces.Num() == 0)
		return;

	// renderBump doesn't care about most of this
	if (m_fastLoad)
	{
		m_bounds.Zero();
		for (i = 0; i < m_surfaces.Num(); i++)
		{
			const modelSurface_t*	surf = &m_surfaces[i];

			R_BoundTriSurf(surf->geometry);
			m_bounds.AddBounds(surf->geometry->bounds);
		}

		return;
	}

	// cleanup all the final surfaces, but don't create sil edges
	totalVerts = 0;
	totalIndexes = 0;

	// decide if we are going to merge all the surfaces into one shadower
	int	numOriginalSurfaces = m_surfaces.Num();

	// make sure there aren't any NULL shaders or geometry
	for (i = 0; i < numOriginalSurfaces; i++)
	{
		const modelSurface_t*	surf = &m_surfaces[i];

		if (surf->geometry == NULL || surf->shader == NULL)
		{
			MakeDefaultModel();
			common->Error("Model %s, surface %i had NULL geometry", m_name.c_str(), i);
		}
		if (surf->shader == NULL)
		{
			MakeDefaultModel();
			common->Error("Model %s, surface %i had NULL shader", m_name.c_str(), i);
		}
	}

	// duplicate and reverse triangles for two sided bump mapped surfaces
	// note that this won't catch surfaces that have their shaders dynamically
	// changed, and won't work with animated models.
	// It is better to create completely separate surfaces, rather than
	// add vertexes and indexes to the existing surface, because the
	// tangent generation wouldn't like the acute shared edges
	for (i = 0; i < numOriginalSurfaces; i++)
	{
		const modelSurface_t*	surf = &m_surfaces[i];

		if (surf->shader->ShouldCreateBackSides())
		{
			srfTriangles_t* newTri;

			newTri = R_CopyStaticTriSurf(surf->geometry);
			R_ReverseTriangles(newTri);

			modelSurface_t	newSurf;

			newSurf.shader = surf->shader;
			newSurf.geometry = newTri;

			AddSurface(newSurf);
		}
	}

	// clean the surfaces
	for (i = 0; i < m_surfaces.Num(); i++)
	{
		const modelSurface_t*	surf = &m_surfaces[i];

		R_CleanupTriangles(surf->geometry, surf->geometry->generateNormals, true, surf->shader->UseUnsmoothedTangents());
		if (surf->shader->SurfaceCastsShadow())
		{
			totalVerts += surf->geometry->numVerts;
			totalIndexes += surf->geometry->numIndexes;
		}
	}

	// add up the total surface area for development information
	for (i = 0; i < m_surfaces.Num(); i++)
	{
		const modelSurface_t*	surf = &m_surfaces[i];
		srfTriangles_t*	tri = surf->geometry;

		for (int j = 0; j < tri->numIndexes; j += 3)
		{
			float	area = idWinding::TriangleArea(tri->verts[tri->indexes[j]].xyz,
				tri->verts[tri->indexes[j + 1]].xyz, tri->verts[tri->indexes[j + 2]].xyz);
			const_cast<idMaterial*>(surf->shader)->AddToSurfaceArea(area);
		}
	}

	// set flags for whole-model rejection
	for (i = 0; i < m_surfaces.Num(); i++)
	{
		const modelSurface_t*	surf = &m_surfaces[i];
		if (surf->shader->IsDrawn())
			m_hasDrawingSurfaces = true;

		if (surf->shader->SurfaceCastsShadow())
			m_hasShadowCastingSurfaces = true;

		if (surf->shader->ReceivesLighting())
			m_hasInteractingSurfaces = true;

		if (strstr(surf->shader->GetName(), "trigger"))
		{
			static int breakHere;
			breakHere++;
		}
	}

	// calculate the bounds
	if (m_surfaces.Num() == 0)
		m_bounds.Zero();
	else
	{
		m_bounds.Clear();
		for (i = 0; i < m_surfaces.Num(); i++)
		{
			modelSurface_t*	surf = &m_surfaces[i];

			// if the surface has a deformation, increase the bounds
			// the amount here is somewhat arbitrary, designed to handle
			// autosprites and flares, but could be done better with exact
			// deformation information.
			// Note that this doesn't handle deformations that are skinned in
			// at run time...
			if (surf->shader->Deform() != DFRM_NONE)
			{
				srfTriangles_t*	tri = surf->geometry;
				idVec3	mid = (tri->bounds[1] + tri->bounds[0]) * 0.5f;
				float	radius = (tri->bounds[0] - mid).Length();
				radius += 20.0f;

				tri->bounds[0][0] = mid[0] - radius;
				tri->bounds[0][1] = mid[1] - radius;
				tri->bounds[0][2] = mid[2] - radius;

				tri->bounds[1][0] = mid[0] + radius;
				tri->bounds[1][1] = mid[1] + radius;
				tri->bounds[1][2] = mid[2] + radius;
			}

			// add to the model bounds
			m_bounds.AddBounds(surf->geometry->bounds);
		}
	}
}

/*
==============
idRenderModelLocal::FreeVertexCache

We are about to restart the vertex cache, so dump everything
==============
*/
void idRenderModelLocal::FreeVertexCache(void)
{
	for (int j = 0; j < m_surfaces.Num(); j++)
	{
		srfTriangles_t* tri = m_surfaces[j].geometry;
		if (tri == NULL)
			continue;

		R_FreeStaticTriSurfVertexCaches(tri);
	}
}


/*
================
idRenderModelLocal::ReadFromDemoFile
================
*/
void idRenderModelLocal::ReadFromDemoFile(class idDemoFile* f)
{
	int i, j, numSurfaces;

	PurgeModel();

	InitEmpty(f->ReadHashString());

	f->ReadInt(numSurfaces);

	for (i = 0; i < numSurfaces; i++)
	{
		modelSurface_t	surf;

		surf.shader = declManager->FindMaterial(f->ReadHashString());

		srfTriangles_t*	tri = R_AllocStaticTriSurf();

		f->ReadInt(tri->numIndexes);
		R_AllocStaticTriSurfIndexes(tri, tri->numIndexes);
		for (j = 0; j < tri->numIndexes; ++j)
			f->ReadInt((int&)tri->indexes[j]);

		f->ReadInt(tri->numVerts);
		R_AllocStaticTriSurfVerts(tri, tri->numVerts);

		idVec3 tNormal, tTangent, tBiTangent;
		for (j = 0; j < tri->numVerts; ++j)
		{
			f->ReadVec3(tri->verts[j].xyz);
			f->ReadBigArray(tri->verts[j].st, 2);
			f->ReadBigArray(tri->verts[j].normal, 4);
			f->ReadBigArray(tri->verts[j].tangent, 4);
			f->ReadUnsignedChar(tri->verts[j].color[0]);
			f->ReadUnsignedChar(tri->verts[j].color[1]);
			f->ReadUnsignedChar(tri->verts[j].color[2]);
			f->ReadUnsignedChar(tri->verts[j].color[3]);
		}

		surf.geometry = tri;

		this->AddSurface(surf);
	}
	this->FinishSurfaces();
}

/*
================
idRenderModelLocal::WriteToDemoFile
================
*/
void idRenderModelLocal::WriteToDemoFile(class idDemoFile* f)
{
	// note that it has been updated
	m_lastArchivedFrame = tr.frameCount;

	f->WriteHashString(this->Name());

	int i, j, iData = m_surfaces.Num();
	f->WriteInt(iData);

	for (i = 0; i < m_surfaces.Num(); i++)
	{
		const modelSurface_t*	surf = &m_surfaces[i];

		f->WriteHashString(surf->shader->GetName());

		srfTriangles_t* tri = surf->geometry;
		f->WriteInt(tri->numIndexes);
		for (j = 0; j < tri->numIndexes; ++j)
			f->WriteInt((int&)tri->indexes[j]);
		f->WriteInt(tri->numVerts);
		for (j = 0; j < tri->numVerts; ++j)
		{
			f->WriteVec3(tri->verts[j].xyz);
			f->WriteBigArray(tri->verts[j].st, 2);
			f->WriteBigArray(tri->verts[j].normal, 4);
			f->WriteBigArray(tri->verts[j].tangent, 4);
			f->WriteUnsignedChar(tri->verts[j].color[0]);
			f->WriteUnsignedChar(tri->verts[j].color[1]);
			f->WriteUnsignedChar(tri->verts[j].color[2]);
			f->WriteUnsignedChar(tri->verts[j].color[3]);
		}
	}
}

/*
================
idRenderModelLocal::IsLoaded
================
*/
bool idRenderModelLocal::IsLoaded(void)
{
	return !m_purged;
}

/*
================
idRenderModelLocal::SetLevelLoadReferenced
================
*/
void idRenderModelLocal::SetLevelLoadReferenced(bool referenced)
{
	m_levelLoadReferenced = referenced;
}

/*
================
idRenderModelLocal::IsLevelLoadReferenced
================
*/
bool idRenderModelLocal::IsLevelLoadReferenced(void)
{
	return m_levelLoadReferenced;
}

/*
=================
idRenderModelLocal::TouchData
=================
*/
void idRenderModelLocal::TouchData(void)
{
	for (int i = 0; i < m_surfaces.Num(); i++)
	{
		const modelSurface_t*	surf = &m_surfaces[i];

		// re-find the material to make sure it gets added to the
		// level keep list
		declManager->FindMaterial(surf->shader->GetName());
	}
}

/*
=================
idRenderModelLocal::DeleteSurfaceWithId
=================
*/
bool idRenderModelLocal::DeleteSurfaceWithId(int id)
{
	int i;

	for (i = 0; i < m_surfaces.Num(); i++)
	{
		if (m_surfaces[i].id == id)
		{
			R_FreeStaticTriSurf(m_surfaces[i].geometry);
			m_surfaces.RemoveIndex(i);
			return true;
		}
	}
	return false;
}

/*
=================
idRenderModelLocal::DeleteSurfacesWithNegativeId
=================
*/
void idRenderModelLocal::DeleteSurfacesWithNegativeId(void)
{
	for (int i = 0; i < m_surfaces.Num(); i++)
	{
		if (m_surfaces[i].id < 0)
		{
			R_FreeStaticTriSurf(m_surfaces[i].geometry);
			m_surfaces.RemoveIndex(i);
			i--;
		}
	}
}

/*
=================
idRenderModelLocal::FindSurfaceWithId
=================
*/
bool idRenderModelLocal::FindSurfaceWithId(int id, int& surfaceNum) const
{
	for (int i = 0; i < m_surfaces.Num(); i++)
	{
		if (m_surfaces[i].id == id)
		{
			surfaceNum = i;
			return true;
		}
	}
	return false;
}

/*
================
idRenderModelLocal::idRenderModelStatic
================
*/
void idRenderModelLocal::AddCubeFace(srfTriangles_t * tri, idVec3 v1, idVec3 v2, idVec3 v3, idVec3 v4)
{
	tri->verts[tri->numVerts + 0].Clear();
	tri->verts[tri->numVerts + 0].xyz = v1 * 8;
	tri->verts[tri->numVerts + 0].SetTexCoord(0, 0);

	tri->verts[tri->numVerts + 1].Clear();
	tri->verts[tri->numVerts + 1].xyz = v2 * 8;
	tri->verts[tri->numVerts + 1].SetTexCoord(1, 0);

	tri->verts[tri->numVerts + 2].Clear();
	tri->verts[tri->numVerts + 2].xyz = v3 * 8;
	tri->verts[tri->numVerts + 2].SetTexCoord(1, 1);

	tri->verts[tri->numVerts + 3].Clear();
	tri->verts[tri->numVerts + 3].xyz = v4 * 8;
	tri->verts[tri->numVerts + 3].SetTexCoord(0, 1);

	tri->indexes[tri->numIndexes + 0] = tri->numVerts + 0;
	tri->indexes[tri->numIndexes + 1] = tri->numVerts + 1;
	tri->indexes[tri->numIndexes + 2] = tri->numVerts + 2;
	tri->indexes[tri->numIndexes + 3] = tri->numVerts + 0;
	tri->indexes[tri->numIndexes + 4] = tri->numVerts + 2;
	tri->indexes[tri->numIndexes + 5] = tri->numVerts + 3;

	tri->numVerts += 4;
	tri->numIndexes += 6;
}
