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

#ifndef _DINAMIC_MODEL_H_
#define _DINAMIC_MODEL_H_

class btGameJoint;

/*
===============================================================================
Dinamic Model
Base for animate model like particles, sprites, liquid models
===============================================================================
*/

class btRenderModelDinamic : public idRenderModelLocal
{
public:
	btRenderModelDinamic(void);
	~btRenderModelDinamic(void);
	
	virtual void			InitFromFile(const char* fileName) {};
	virtual bool			LoadBinaryModel(idFile* file, const ID_TIME_T sourceTimeStamp) { return false; };
	virtual void			WriteBinaryModel(idFile* file, ID_TIME_T* _timeStamp = NULL) const {};
	virtual bool			SupportsBinaryModel(void) { return false; }

	//continuous, change every time 
	virtual dynamicModel_t	IsDynamicModel(void) const;

	// with the addModels2 arrangement we could have light accepting and
	// shadowing dynamic models, but the original game never did
	virtual bool			ModelHasDrawingSurfaces(void) const { return true; };
	virtual bool			ModelHasInteractingSurfaces(void) const { return false; };
	virtual bool			ModelHasShadowCastingSurfaces(void) const { return false; };

	virtual idRenderModel* 		InstantiateDynamicModel(const struct renderEntity_s* ent, const viewDef_t* view, idRenderModel* cachedModel);

	virtual int					NumJoints(void) const;
	virtual const idList<btGameJoint> GetJoints(void) const;
	virtual jointHandle_t		GetJointHandle(const char* name) const;
	virtual const char* 		GetJointName(jointHandle_t handle) const;
	virtual const idJointQuat* 	GetDefaultPose(void) const;
	virtual int					NearestJoint(int surfaceNum, int a, int b, int c) const;

};

#endif // !_DINAMIC_MODEL_H_

