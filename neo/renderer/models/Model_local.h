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

#ifndef __MODEL_LOCAL_H__
#define __MODEL_LOCAL_H__

class idRenderModel;
struct srfTriangles_t;
struct modelSurface_t;

typedef struct matchVert_s
{
	struct matchVert_s*	next;
	int		v, tv;
	byte	color[4];
	idVec3	normal;
} matchVert_t;

/*
===============================================================================
	render model local
	Render model Base class
===============================================================================
*/
class idRenderModelLocal : public idRenderModel
{
public:
	// the inherited public interface
	static idRenderModel* 		Alloc(void);

	idRenderModelLocal(void);
	virtual						~idRenderModelLocal(void);

	virtual void				PartialInitFromFile(const char* fileName);
	virtual void				PurgeModel(void);
	virtual void				Reset(void) {};
	virtual void				LoadModel(void);
	virtual bool				IsLoaded(void);
	virtual void				SetLevelLoadReferenced(bool referenced);
	virtual bool				IsLevelLoadReferenced(void);
	virtual void				TouchData(void);
	virtual void				InitEmpty(const char* name);
	virtual void				AddSurface(modelSurface_t surface);
	virtual void				FinishSurfaces(void);
	virtual void				FreeVertexCache(void);
	virtual const char* 		Name(void) const;
	virtual void				Print(void) const;
	virtual void				List(void) const;
	virtual int					Memory(void) const;
	virtual ID_TIME_T			Timestamp(void) const;
	virtual int					NumSurfaces(void) const;
	virtual int					NumBaseSurfaces(void) const;
	virtual const modelSurface_t* Surface(int surfaceNum) const;
	virtual srfTriangles_t* 	AllocSurfaceTriangles(int numVerts, int numIndexes) const;
	virtual void				FreeSurfaceTriangles(srfTriangles_t* tris) const;
	virtual bool				IsStaticWorldModel(void) const;
	virtual bool				IsDefaultModel(void) const;
	virtual bool				IsReloadable(void) const;
	
	virtual idBounds			Bounds(const struct renderEntity_s* ent) const;
	virtual void				ReadFromDemoFile(class idDemoFile* f);
	virtual void				WriteToDemoFile(class idDemoFile* f);
	virtual float				DepthHack(void) const;

	virtual bool				ModelHasDrawingSurfaces(void) const;
	virtual bool				ModelHasInteractingSurfaces(void) const;
	virtual bool				ModelHasShadowCastingSurfaces(void) const;

	void						MakeDefaultModel();

	bool						DeleteSurfaceWithId(int id);
	void						DeleteSurfacesWithNegativeId();
	bool						FindSurfaceWithId(int id, int& surfaceNum) const;

public:
	idList<modelSurface_t, TAG_MODEL>	m_surfaces;
	idBounds					m_bounds;
	int							m_overlaysAdded;

protected:
	static void AddCubeFace(srfTriangles_t * tri, idVec3 v1, idVec3 v2, idVec3 v3, idVec3 v4);

	int							m_lastModifiedFrame;
	int							m_lastArchivedFrame;

	idStr						m_name;
	bool						m_isStaticWorldModel;
	bool						m_defaulted;
	bool						m_purged;					// eventually we will have dynamic reloading
	bool						m_fastLoad;					// don't generate tangents and shadow data
	bool						m_reloadable;				// if not, reloadModels won't check timestamp
	bool						m_levelLoadReferenced;		// for determining if it needs to be freed
	bool						m_hasDrawingSurfaces;
	bool						m_hasInteractingSurfaces;
	bool						m_hasShadowCastingSurfaces;
	ID_TIME_T					m_timeStamp;

	static idCVar				r_mergeModelSurfaces;		// combine model surfaces with the same material
	static idCVar				r_slopVertex;				// merge xyz coordinates this far apart
	static idCVar				r_slopTexCoord;				// merge texture coordinates this far apart
	static idCVar				r_slopNormal;				// merge normals that dot less than this
};

#endif /* !__MODEL_LOCAL_H__ */
