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

#ifndef __LIQUID_MODEL_H__
#define __LIQUID_MODEL_H__

#include "renderer/models/internal/Model_dinamic.h"

/*
===============================================================================

	Liquid model

===============================================================================
*/

class idRenderModelLiquid : public btRenderModelDinamic
{
public:
	idRenderModelLiquid(void);
	
	virtual void				InitFromFile( const char* fileName );
	virtual bool				SupportsBinaryModel(void) { return false; }
	virtual dynamicModel_t		IsDynamicModel(void) const;
	virtual idRenderModel* 		InstantiateDynamicModel( const struct renderEntity_s* ent, const viewDef_t* view, idRenderModel* cachedModel );
	virtual idBounds			Bounds( const struct renderEntity_s* ent ) const;
	
	virtual void				Reset(void);
	void						IntersectBounds( const idBounds& bounds, float displacement );
	
private:
	modelSurface_t				GenerateSurface( float lerp );
	void						WaterDrop( int x, int y, float* page );
	void						Update(void);
	
	int							verts_x;
	int							verts_y;
	float						scale_x;
	float						scale_y;
	int							time;
	int							liquid_type;
	int							update_tics;
	int							seed;
	
	idRandom					random;
	
	const idMaterial* 			shader;
	deformInfo_t* 				deformInfo;		// used to create srfTriangles_t from base frames
	// and new vertexes
	
	float						density;
	float						drop_height;
	int							drop_radius;
	float						drop_delay;
	
	idList<float, TAG_MODEL>	pages;
	float* 						page1;
	float* 						page2;
	
	idList<idDrawVert, TAG_MODEL>	verts;
	
	int							nextDropTime;
};

#endif /* !__LIQUID_MODEL_H__ */