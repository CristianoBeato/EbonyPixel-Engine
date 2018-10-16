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

#ifndef _MODEL_STATIC_H_
#define _MODEL_STATIC_H_

#include "renderer/models/Model_local.h"

//Binary Render Model Magic
static const byte BRM_VERSION = 109;
static const unsigned int BRM_MAGIC = ('B' << 24) | ('R' << 16) | ('M' << 8) | BRM_VERSION;

/*
===============================================================================
Static model
Base for non animate model 
===============================================================================
*/
class idRenderModelStatic : public idRenderModelLocal
{
public:
	idRenderModelStatic(void);
	virtual						~idRenderModelStatic(void);

	virtual void				InitFromFile(const char* fileName);
	virtual bool				LoadBinaryModel(idFile* file, const ID_TIME_T sourceTimeStamp);
	virtual void				WriteBinaryModel(idFile* file, ID_TIME_T* _timeStamp = NULL) const;
	virtual bool				SupportsBinaryModel(void) { return true; }

	virtual dynamicModel_t		IsDynamicModel(void) const;
	virtual idRenderModel* 		InstantiateDynamicModel(const struct renderEntity_s* ent, const viewDef_t* view, idRenderModel* cachedModel);

	virtual int					NumJoints(void) const;
	virtual const idList<btGameJoint> GetJoints(void) const;
	virtual jointHandle_t		GetJointHandle(const char* name) const;
	virtual const char* 		GetJointName(jointHandle_t handle) const;
	virtual const idJointQuat* 	GetDefaultPose(void) const;
	virtual int					NearestJoint(int surfaceNum, int a, int b, int c) const;

	// RB begin
	virtual void				ExportOBJ(idFile* objFile, idFile* mtlFile, ID_TIME_T* _timeStamp = NULL) const;
	// RB end

protected:
	virtual bool	Load(void);
};

#endif // !_MODEL_STATIC_H_
