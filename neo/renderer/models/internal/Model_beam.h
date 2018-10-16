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

#ifndef __BEAM_MODEL_H__
#define __BEAM_MODEL_H__

#include "renderer/models/internal/Model_dinamic.h"

/*
===============================================================================
Beam model
===============================================================================
*/
class idRenderModelBeam : public btRenderModelDinamic
{
public:
	virtual bool				IsLoaded(void) const;
	virtual idRenderModel* 		InstantiateDynamicModel(const struct renderEntity_s* ent, const viewDef_t* view, idRenderModel* cachedModel);
	virtual idBounds			Bounds(const struct renderEntity_s* ent) const;

};

/*
===============================================================================
Trail model
===============================================================================
*/
#define MAX_TRAIL_PTS	20

struct Trail_t
{
	int							lastUpdateTime;
	int							duration;

	idVec3						pts[MAX_TRAIL_PTS];
	int							numPoints;
};

class idRenderModelTrail : public btRenderModelDinamic
{
	idList<Trail_t, TAG_MODEL>	trails;
	int							numActive;
	idBounds					trailBounds;

public:
	idRenderModelTrail(void);

	virtual bool				IsLoaded(void) const;
	virtual idRenderModel* 		InstantiateDynamicModel(const struct renderEntity_s* ent, const viewDef_t* view, idRenderModel* cachedModel);
	virtual idBounds			Bounds(const struct renderEntity_s* ent) const;

	int							NewTrail(idVec3 pt, int duration);
	void						UpdateTrail(int index, idVec3 pt);
	void						DrawTrail(int index, const struct renderEntity_s* ent, srfTriangles_t* tri, float globalAlpha);
};

/*
===============================================================================
Lightning model
===============================================================================
*/

class idRenderModelLightning : public btRenderModelDinamic
{
public:
	virtual bool				IsLoaded(void) const;
	virtual idRenderModel* 		InstantiateDynamicModel(const struct renderEntity_s* ent, const viewDef_t* view, idRenderModel* cachedModel);
	virtual idBounds			Bounds(const struct renderEntity_s* ent) const;
};

#endif /* !__BEAM_MODEL_H__ */
