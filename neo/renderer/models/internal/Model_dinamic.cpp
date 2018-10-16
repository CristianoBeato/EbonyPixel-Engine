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

#include "Model_dinamic.h"
#include "renderer/tr_local.h"
#include "renderer/models/Model_local.h"

//include to bypass "btGameJoint" idlist error
#include "renderer/models/internal/Model_skined.h"


btRenderModelDinamic::btRenderModelDinamic(void) : idRenderModelLocal()
{
}

btRenderModelDinamic::~btRenderModelDinamic(void)
{
}

/*
===============
idRenderModelBeam::IsDynamicModel
===============
*/
dynamicModel_t btRenderModelDinamic::IsDynamicModel(void) const
{
	return DM_CONTINUOUS;	// regenerate for every view
}

idRenderModel * btRenderModelDinamic::InstantiateDynamicModel(const renderEntity_s * ent, const viewDef_t * view, idRenderModel * cachedModel)
{
	if (cachedModel)
	{
		delete cachedModel;
		cachedModel = NULL;
	}
	
	//TODO: change 
	common->Error("InstantiateDynamicModel called on dinamic model before instance '%s'", m_name.c_str());
	return NULL;
}

/*
================
idRenderModelStatic::NumJoints
================
*/
int btRenderModelDinamic::NumJoints(void) const
{
	return 0;
}

/*
================
idRenderModelStatic::GetJoints
================
*/
const  idList<btGameJoint> btRenderModelDinamic::GetJoints(void) const
{
	return idList<btGameJoint>();
}

/*
================
idRenderModelStatic::GetJointHandle
================
*/
jointHandle_t btRenderModelDinamic::GetJointHandle(const char* name) const
{
	return INVALID_JOINT;
}

/*
================
idRenderModelStatic::GetJointName
================
*/
const char* btRenderModelDinamic::GetJointName(jointHandle_t handle) const
{
	return "";
}

/*
================
idRenderModelStatic::GetDefaultPose
================
*/
const idJointQuat* btRenderModelDinamic::GetDefaultPose(void) const
{
	return NULL;
}

/*
================
idRenderModelStatic::NearestJoint
================
*/
int btRenderModelDinamic::NearestJoint(int surfaceNum, int a, int b, int c) const
{
	return INVALID_JOINT;
}
