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

#include "Model_skined.h"

#include "renderer/tr_local.h"
#include "renderer/models/Model.h"
#include "renderer/models/Model_local.h"

static const char* SKIN_SnapshotName = "_SKN_Snapshot_";

//need move for here
#if defined(USE_INTRINSICS)
static const __m128 vector_float_posInfinity = { idMath::INFINITY, idMath::INFINITY, idMath::INFINITY, idMath::INFINITY };
static const __m128 vector_float_negInfinity = { -idMath::INFINITY, -idMath::INFINITY, -idMath::INFINITY, -idMath::INFINITY };
#endif

/*
====================
btGameJoint::btGameJoint
====================
*/
btGameJoint::btGameJoint(void) :
	m_jointId(INVALID_JOINT),
	m_parent(nullptr),
	m_name(nullptr)
{
}

/*
====================
btGameJoint::btGameJoint
====================
*/
btGameJoint::btGameJoint(const char * name, const jointHandle_t id, const btGameJoint * parent) :
	m_jointId(id),
	m_parent(parent),
	m_name(name)
{
}

/*
====================
btGameJoint::Id
====================
*/
jointHandle_t & btGameJoint::Id(void)
{
	return m_jointId;
}

/*
====================
btGameJoint::Id
====================
*/
const jointHandle_t btGameJoint::Id(void) const
{
	return m_jointId;
}

/*
====================
btGameJoint::Name
====================
*/
idStr &	btGameJoint::Name(void)
{
	//check if don't have a empty name
//	assert(!m_name.IsEmpty());
	return m_name;
}

/*
====================
btGameJoint::Name
====================
*/
const idStr btGameJoint::Name(void) const
{
	//check if don't have a empty name
//	assert(!m_name.IsEmpty());
	return m_name;
}

/*
====================
btGameJoint::Parent
====================
*/
const btGameJoint *& btGameJoint::Parent(void)
{
	return m_parent;
}

/*
====================
btGameJoint::Parent
====================
*/
const btGameJoint * btGameJoint::Parent(void) const
{
	return m_parent;
}

/*
====================
btGameJoint::getParentName
====================
*/
const char * btGameJoint::getParentName(void) const
{
	if (!m_parent)
		return NULL;

	return m_parent->m_name.c_str();
}

/*
====================
btGameJoint::getParentId
====================
*/
jointHandle_t btGameJoint::getParentId(void) const
{
	//is a root joint
	if (!m_parent)
		return INVALID_JOINT;

	//if have a parent return his joint
	return m_parent->Id();
}

/*
====================
====================
*/
btRenderModelSkined::btRenderModelSkined(void): 
	idRenderModelLocal(),
	m_numInvertedJoints(0),
	m_jointsInverted(NULL),
	m_jointsInvertedBuffer(0)
{
}

btRenderModelSkined::~btRenderModelSkined(void)
{
}

void btRenderModelSkined::Clear(void)
{
}

void btRenderModelSkined::PurgeModel(void)
{
	idRenderModelLocal::PurgeModel();

	if (m_jointsInverted != NULL)
	{
		Mem_Free(m_jointsInverted);
		m_jointsInverted = NULL;
	}

	m_invertedDefaultPose.Clear();
	m_defaultPose.Clear();
	m_joints.Clear();
	m_meshes.Clear();
}

/*
====================
btRenderModelSkined::Bounds

This calculates a rough bounds by using the joint radii without
transforming all the points
====================
*/
idBounds btRenderModelSkined::Bounds(const renderEntity_s * ent) const
{
	// this is the bounds for the reference pose
	if (ent == NULL)
		return m_bounds;

	return ent->bounds;
}

/*
====================
btRenderModelSkined::IsDynamicModel
====================
*/
dynamicModel_t btRenderModelSkined::IsDynamicModel(void) const
{
	return DM_CACHED;
}

/*
==============
btRenderModelSkined::Print
==============
*/
void btRenderModelSkined::Print(void) const
{
	common->Printf("%s\n", m_name.c_str());
	common->Printf("Dynamic model.\n");
	common->Printf("Generated smooth normals.\n");
	common->Printf("    verts  tris weights material\n");
	int	totalVerts = 0;
	int	totalTris = 0;
	const btRenderMeshSkined* mesh = m_meshes.Ptr();
	for (int i = 0; i < m_meshes.Num(); i++, mesh++)
	{
		totalVerts += mesh->NumVerts();
		totalTris += mesh->NumTris();
		common->Printf("%2i: %5i %5i %s\n", i, mesh->NumVerts(), mesh->NumTris(), mesh->getShaderName());
	}

	common->Printf("-----\n");
	common->Printf("%4i verts.\n", totalVerts);
	common->Printf("%4i tris.\n", totalTris);
	common->Printf("%4i joints.\n", m_joints.Num());
}


btRenderMeshSkined::btRenderMeshSkined(void)
{
	m_shader = NULL;
	m_meshNumVerts = 0;
	m_meshNumTris = 0;
	m_meshJoints = NULL;
	m_meshNumJoints = 0;
	m_maxJointVertDist = 0.0f;
	m_deformInfo = NULL;

}

btRenderMeshSkined::~btRenderMeshSkined(void)
{
	Clear();
}

/*
====================
btRenderMeshSkined::CalculateBounds
====================
*/
void btRenderMeshSkined::CalculateBounds(const idJointMat * entJoints, idBounds & bounds) const
{
#if defined(USE_INTRINSICS)
	__m128 minX = vector_float_posInfinity;
	__m128 minY = vector_float_posInfinity;
	__m128 minZ = vector_float_posInfinity;
	__m128 maxX = vector_float_negInfinity;
	__m128 maxY = vector_float_negInfinity;
	__m128 maxZ = vector_float_negInfinity;
	for (int i = 0; i < m_numMeshJoints; i++)
	{
		const idJointMat& joint = entJoints[m_meshJoints[i]];
		__m128 x = _mm_load_ps(joint.ToFloatPtr() + 0 * 4);
		__m128 y = _mm_load_ps(joint.ToFloatPtr() + 1 * 4);
		__m128 z = _mm_load_ps(joint.ToFloatPtr() + 2 * 4);
		minX = _mm_min_ps(minX, x);
		minY = _mm_min_ps(minY, y);
		minZ = _mm_min_ps(minZ, z);
		maxX = _mm_max_ps(maxX, x);
		maxY = _mm_max_ps(maxY, y);
		maxZ = _mm_max_ps(maxZ, z);
	}
	__m128 expand = _mm_splat_ps(_mm_load_ss(&m_maxJointVertDist), 0);
	minX = _mm_sub_ps(minX, expand);
	minY = _mm_sub_ps(minY, expand);
	minZ = _mm_sub_ps(minZ, expand);
	maxX = _mm_add_ps(maxX, expand);
	maxY = _mm_add_ps(maxY, expand);
	maxZ = _mm_add_ps(maxZ, expand);
	_mm_store_ss(bounds.ToFloatPtr() + 0, _mm_splat_ps(minX, 3));
	_mm_store_ss(bounds.ToFloatPtr() + 1, _mm_splat_ps(minY, 3));
	_mm_store_ss(bounds.ToFloatPtr() + 2, _mm_splat_ps(minZ, 3));
	_mm_store_ss(bounds.ToFloatPtr() + 3, _mm_splat_ps(maxX, 3));
	_mm_store_ss(bounds.ToFloatPtr() + 4, _mm_splat_ps(maxY, 3));
	_mm_store_ss(bounds.ToFloatPtr() + 5, _mm_splat_ps(maxZ, 3));

#else

	bounds.Clear();
	for (int i = 0; i < numMeshJoints; i++)
	{
		const idJointMat& joint = entJoints[meshJoints[i]];
		bounds.AddPoint(joint.GetTranslation());
	}
	bounds.ExpandSelf(maxJointVertDist);

#endif
}

/*
====================
btRenderMeshSkined::UpdateSurface
====================
*/
void btRenderMeshSkined::UpdateSurface(const renderEntity_s * ent, const idJointMat * entJoints, const idJointMat * entJointsInverted, modelSurface_t * surf)
{
	tr.pc.c_deformedSurfaces++;
	tr.pc.c_deformedVerts += m_deformInfo->numOutputVerts;
	tr.pc.c_deformedIndexes += m_deformInfo->numIndexes;

	surf->shader = m_shader;

	if (surf->geometry != NULL)
	{
		// if the number of verts and indexes are the same we can re-use the triangle surface
		if (surf->geometry->numVerts == m_deformInfo->numOutputVerts && surf->geometry->numIndexes == m_deformInfo->numIndexes)
			R_FreeStaticTriSurfVertexCaches(surf->geometry);
		else
		{
			R_FreeStaticTriSurf(surf->geometry);
			surf->geometry = R_AllocStaticTriSurf();
		}
	}
	else
		surf->geometry = R_AllocStaticTriSurf();

	srfTriangles_t* tri = surf->geometry;

	// note that some of the data is referenced, and should not be freed
	tri->referencedIndexes = true;
	tri->numIndexes = m_deformInfo->numIndexes;
	tri->indexes = m_deformInfo->indexes;
	tri->silIndexes = m_deformInfo->silIndexes;
	tri->numMirroredVerts = m_deformInfo->numMirroredVerts;
	tri->mirroredVerts = m_deformInfo->mirroredVerts;
	tri->numDupVerts = m_deformInfo->numDupVerts;
	tri->dupVerts = m_deformInfo->dupVerts;
	tri->numSilEdges = m_deformInfo->numSilEdges;
	tri->silEdges = m_deformInfo->silEdges;

	tri->indexCache = m_deformInfo->staticIndexCache;

	tri->numVerts = m_deformInfo->numOutputVerts;

	// RB: added check wether GPU skinning is available at all
	if (r_useGPUSkinning.GetBool() && glConfig.gpuSkinningAvailable)
	{
		if (tri->verts != NULL && tri->verts != m_deformInfo->verts)
		{
			R_FreeStaticTriSurfVerts(tri);
		}
		tri->verts = m_deformInfo->verts;
		tri->ambientCache = m_deformInfo->staticAmbientCache;
		tri->shadowCache = m_deformInfo->staticShadowCache;
		tri->referencedVerts = true;
	}
	else
	{
		if (tri->verts == NULL || tri->verts == m_deformInfo->verts)
		{
			tri->verts = NULL;
			R_AllocStaticTriSurfVerts(tri, m_deformInfo->numOutputVerts);
			assert(tri->verts != NULL);	// quiet analyze warning
			memcpy(tri->verts, m_deformInfo->verts, m_deformInfo->numOutputVerts * sizeof(m_deformInfo->verts[0]));	// copy over the texture coordinates
		}
		TransformVertsAndTangents(tri->verts, m_deformInfo->numOutputVerts, m_deformInfo->verts, entJointsInverted);
		tri->referencedVerts = false;
	}
	tri->tangentsCalculated = true;

	CalculateBounds(entJoints, tri->bounds);
}

int btRenderMeshSkined::NumVerts(void) const
{
	return m_meshNumVerts;
}

int btRenderMeshSkined::NumTris(void) const
{
	return m_meshNumTris;
}

const char * btRenderMeshSkined::getShaderName(void) const
{
	return m_shader->GetName();
}

void btRenderMeshSkined::Clear(void)
{
	//clear joints
	if (m_meshJoints != NULL)
	{
		Mem_Free(m_meshJoints);
		m_meshJoints = NULL;
	}

	//clear deform
	if (m_deformInfo != NULL)
	{
		R_FreeDeformInfo(m_deformInfo);
		m_deformInfo = NULL;
	}
}

/*
====================
btRenderModelSkined::NumJoints
====================
*/
int btRenderModelSkined::NumJoints(void) const
{
	return m_joints.Num();
}

/*
====================
btRenderModelSkined::GetJoints
====================
*/
const idList<btGameJoint> btRenderModelSkined::GetJoints(void) const
{
	return m_joints;
}

/*
====================
btRenderModelSkined::GetJointHandle
====================
*/
jointHandle_t btRenderModelSkined::GetJointHandle(const char * name) const
{
	//try find the joint
	for (int i = 0; i < m_joints.Num(); i++)
	{
		if (idStr::Icmp(m_joints[i].Name(), name) == 0)
			return (jointHandle_t)i;
	}

	//joint not found
	return INVALID_JOINT;
}

/*
====================
btRenderModelSkined::GetJointName
====================
*/
const char * btRenderModelSkined::GetJointName(jointHandle_t handle) const
{
	if ((handle < 0) || (handle >= m_joints.Num()))
		return "<invalid joint>";

	return m_joints[handle].Name().c_str();
}

/*
====================
btRenderModelSkined::GetDefaultPose
====================
*/
const idJointQuat * btRenderModelSkined::GetDefaultPose(void) const
{
	return m_defaultPose.Ptr();
}

/*
====================
btRenderModelSkined::NearestJoint
====================
*/
int btRenderModelSkined::NearestJoint(int surfaceNum, int a, int c, int b) const
{
	return 0;
}

/*
====================
btRenderModelSkined::ReserveJoints
====================
*/
void btRenderModelSkined::ReserveJointsNum(const unsigned int num)
{
	//reserve the bone location instance
	m_joints.SetGranularity(1);
	m_joints.SetNum(num);

	//reserve the bone pose intance
	m_defaultPose.SetGranularity(1);
	m_defaultPose.SetNum(num);
}

/*
====================
btRenderModelSkined::CreateInverseBasePose(void)

create the inverse of the base pose joints to support tech6 style deformation
of base pose vertexes, normals, and tangents.

vertex * joints * inverseJoints == vertex when joints is the base pose
When the joints are in another pose, it gives the animated vertex position
====================
*/
void btRenderModelSkined::CreateInverseBasePose(const idJointMat * poseMat)
{
	m_invertedDefaultPose.SetNum(SIMD_ROUND_JOINTS(m_joints.Num()));

	for (int i = 0; i < m_joints.Num(); i++)
	{
		m_invertedDefaultPose[i] = poseMat[i];
		m_invertedDefaultPose[i].Invert();
	}

	SIMD_INIT_LAST_JOINT(m_invertedDefaultPose.Ptr(), m_joints.Num());
}

/*
====================
idRenderModelMD5::InstantiateDynamicModel
====================
*/
idRenderModel* btRenderModelSkined::InstantiateDynamicModel(const struct renderEntity_s* ent, const viewDef_t* view, idRenderModel* cachedModel)
{
	if (cachedModel != NULL && !r_useCachedDynamicModels.GetBool())
	{
		delete cachedModel;
		cachedModel = NULL;
	}

	if (m_purged)
	{
		common->DWarning("model %s instantiated while purged", Name());
		LoadModel();
	}

	if (!ent->joints)
	{
		common->Printf("idRenderModelMD5::InstantiateDynamicModel: NULL joints on renderEntity for '%s'\n", Name());
		delete cachedModel;
		return NULL;
	}
	else if (ent->numJoints != m_joints.Num())
	{
		common->Printf("idRenderModelMD5::InstantiateDynamicModel: renderEntity has different number of joints than model for '%s'\n", Name());
		delete cachedModel;
		return NULL;
	}

	tr.pc.c_generateMd5++;

	btRenderModelSkined* staticModel;
	if (cachedModel != NULL)
	{
		assert(dynamic_cast<btRenderModelSkined*>(cachedModel) != NULL);
		assert(idStr::Icmp(cachedModel->Name(), SKIN_SnapshotName) == 0);
		staticModel = static_cast<btRenderModelSkined*>(cachedModel);
	}
	else
	{
		staticModel = new(TAG_MODEL) btRenderModelSkined;
		staticModel->InitEmpty(SKIN_SnapshotName);
	}

	staticModel->m_bounds.Clear();

	if (r_showSkel.GetInteger())
	{
		if ((view != NULL) && (!r_skipSuppress.GetBool() || !ent->suppressSurfaceInViewID || (ent->suppressSurfaceInViewID != view->renderView.viewID)))
		{
			// only draw the skeleton
			DrawJoints(ent, view);
		}

		if (r_showSkel.GetInteger() > 1)
		{
			// turn off the model when showing the skeleton
			staticModel->InitEmpty(SKIN_SnapshotName);
			return staticModel;
		}
	}

	// update the GPU joints array
	const int numInvertedJoints = SIMD_ROUND_JOINTS(m_joints.Num());
	if (staticModel->m_jointsInverted == NULL)
	{
		staticModel->m_numInvertedJoints = numInvertedJoints;
		const int alignment = glConfig.uniformBufferOffsetAlignment;
		staticModel->m_jointsInverted = (idJointMat*)Mem_ClearedAlloc(ALIGN(numInvertedJoints * sizeof(idJointMat), alignment), TAG_JOINTMAT);
		staticModel->m_jointsInvertedBuffer = 0;
	}
	else
	{
		assert(staticModel->m_numInvertedJoints == numInvertedJoints);
	}

	TransformJoints(staticModel->m_jointsInverted, m_joints.Num(), ent->joints, m_invertedDefaultPose.Ptr());

	// create all the surfaces
	btRenderMeshSkined* mesh = m_meshes.Ptr();
	for (int i = 0; i < m_meshes.Num(); i++, mesh++)
	{
		// avoid deforming the surface if it will be a nodraw due to a skin remapping
		const idMaterial* shader = mesh->m_shader;

		shader = R_RemapShaderBySkin(shader, ent->customSkin, ent->customShader);

		if (!shader || (!shader->IsDrawn() && !shader->SurfaceCastsShadow()))
		{
			staticModel->DeleteSurfaceWithId(i);
			mesh->m_surfaceNum = -1;
			continue;
		}

		modelSurface_t* surf;

		int surfaceNum = 0;
		if (staticModel->FindSurfaceWithId(i, surfaceNum))
		{
			mesh->m_surfaceNum = surfaceNum;
			surf = &staticModel->m_surfaces[surfaceNum];
		}
		else
		{
			mesh->m_surfaceNum = staticModel->NumSurfaces();
			surf = &staticModel->m_surfaces.Alloc();
			surf->geometry = NULL;
			surf->shader = NULL;
			surf->id = i;
		}

		mesh->UpdateSurface(ent, ent->joints, staticModel->m_jointsInverted, surf);
		assert(surf->geometry != NULL);	// to get around compiler warning

										// the deformation of the tangents can be deferred until each surface is added to the view
		surf->geometry->staticModelWithJoints = staticModel;

		staticModel->m_bounds.AddBounds(surf->geometry->bounds);
	}

	return staticModel;
}

/*
====================
btRenderMeshSkined::DrawJoints
====================
*/
void btRenderModelSkined::DrawJoints(const renderEntity_t* ent, const viewDef_t* view) const
{
	int					i;
	int					num;
	idVec3				pos;
	const idJointMat*	joint;
	const btGameJoint*	mdlJoint;
	int					parentNum;

	num = ent->numJoints;
	joint = ent->joints;
	mdlJoint = m_joints.Ptr();
	for (i = 0; i < num; i++, joint++, mdlJoint++)
	{
		pos = ent->origin + joint->ToVec3() * ent->axis;
		if (mdlJoint->Parent())
		{
			//parentNum = md5Joint->parent - (idMD5Joint*)joints.Ptr();
			parentNum = mdlJoint->getParentId();
			common->RW()->DebugLine(colorWhite,
				ent->origin + ent->joints[parentNum].ToVec3() * ent->axis,
				pos);
		}

		common->RW()->DebugLine(colorRed, pos, pos + joint->ToMat3()[0] * 2.0f * ent->axis);
		common->RW()->DebugLine(colorGreen, pos, pos + joint->ToMat3()[1] * 2.0f * ent->axis);
		common->RW()->DebugLine(colorBlue, pos, pos + joint->ToMat3()[2] * 2.0f * ent->axis);
	}

	idBounds bounds;

	bounds.FromTransformedBounds(ent->bounds, vec3_zero, ent->axis);
	common->RW()->DebugBounds(colorMagenta, bounds, ent->origin);

	if ((r_jointNameScale.GetFloat() != 0.0f) && (bounds.Expand(128.0f).ContainsPoint(view->renderView.vieworg - ent->origin)))
	{
		idVec3	offset(0, 0, r_jointNameOffset.GetFloat());
		float	scale;

		scale = r_jointNameScale.GetFloat();
		joint = ent->joints;
		num = ent->numJoints;
		for (i = 0; i < num; i++, joint++)
		{
			pos = ent->origin + joint->ToVec3() * ent->axis;
			common->RW()->DrawText(m_joints[i].Name(), pos + offset, scale, colorWhite, view->renderView.viewaxis, 1);
		}
	}
}


/*
====================
btRenderModelSkined::TransformJoints
====================
*/
void btRenderModelSkined::TransformJoints(idJointMat* __restrict outJoints, const int numJoints, const idJointMat* __restrict inJoints1, const idJointMat* __restrict inJoints2)
{

	float* outFloats = outJoints->ToFloatPtr();
	const float* inFloats1 = inJoints1->ToFloatPtr();
	const float* inFloats2 = inJoints2->ToFloatPtr();

	assert_16_byte_aligned(outFloats);
	assert_16_byte_aligned(inFloats1);
	assert_16_byte_aligned(inFloats2);

#if defined(USE_INTRINSICS)

	const __m128 mask_keep_last = __m128c(_mm_set_epi32(0xFFFFFFFF, 0x00000000, 0x00000000, 0x00000000));

	for (int i = 0; i < numJoints; i += 2, inFloats1 += 2 * 12, inFloats2 += 2 * 12, outFloats += 2 * 12)
	{
		__m128 m1a0 = _mm_load_ps(inFloats1 + 0 * 12 + 0);
		__m128 m1b0 = _mm_load_ps(inFloats1 + 0 * 12 + 4);
		__m128 m1c0 = _mm_load_ps(inFloats1 + 0 * 12 + 8);
		__m128 m1a1 = _mm_load_ps(inFloats1 + 1 * 12 + 0);
		__m128 m1b1 = _mm_load_ps(inFloats1 + 1 * 12 + 4);
		__m128 m1c1 = _mm_load_ps(inFloats1 + 1 * 12 + 8);

		__m128 m2a0 = _mm_load_ps(inFloats2 + 0 * 12 + 0);
		__m128 m2b0 = _mm_load_ps(inFloats2 + 0 * 12 + 4);
		__m128 m2c0 = _mm_load_ps(inFloats2 + 0 * 12 + 8);
		__m128 m2a1 = _mm_load_ps(inFloats2 + 1 * 12 + 0);
		__m128 m2b1 = _mm_load_ps(inFloats2 + 1 * 12 + 4);
		__m128 m2c1 = _mm_load_ps(inFloats2 + 1 * 12 + 8);

		__m128 tj0 = _mm_and_ps(m1a0, mask_keep_last);
		__m128 tk0 = _mm_and_ps(m1b0, mask_keep_last);
		__m128 tl0 = _mm_and_ps(m1c0, mask_keep_last);
		__m128 tj1 = _mm_and_ps(m1a1, mask_keep_last);
		__m128 tk1 = _mm_and_ps(m1b1, mask_keep_last);
		__m128 tl1 = _mm_and_ps(m1c1, mask_keep_last);

		__m128 ta0 = _mm_splat_ps(m1a0, 0);
		__m128 td0 = _mm_splat_ps(m1b0, 0);
		__m128 tg0 = _mm_splat_ps(m1c0, 0);
		__m128 ta1 = _mm_splat_ps(m1a1, 0);
		__m128 td1 = _mm_splat_ps(m1b1, 0);
		__m128 tg1 = _mm_splat_ps(m1c1, 0);

		__m128 ra0 = _mm_add_ps(tj0, _mm_mul_ps(ta0, m2a0));
		__m128 rd0 = _mm_add_ps(tk0, _mm_mul_ps(td0, m2a0));
		__m128 rg0 = _mm_add_ps(tl0, _mm_mul_ps(tg0, m2a0));
		__m128 ra1 = _mm_add_ps(tj1, _mm_mul_ps(ta1, m2a1));
		__m128 rd1 = _mm_add_ps(tk1, _mm_mul_ps(td1, m2a1));
		__m128 rg1 = _mm_add_ps(tl1, _mm_mul_ps(tg1, m2a1));

		__m128 tb0 = _mm_splat_ps(m1a0, 1);
		__m128 te0 = _mm_splat_ps(m1b0, 1);
		__m128 th0 = _mm_splat_ps(m1c0, 1);
		__m128 tb1 = _mm_splat_ps(m1a1, 1);
		__m128 te1 = _mm_splat_ps(m1b1, 1);
		__m128 th1 = _mm_splat_ps(m1c1, 1);

		__m128 rb0 = _mm_add_ps(ra0, _mm_mul_ps(tb0, m2b0));
		__m128 re0 = _mm_add_ps(rd0, _mm_mul_ps(te0, m2b0));
		__m128 rh0 = _mm_add_ps(rg0, _mm_mul_ps(th0, m2b0));
		__m128 rb1 = _mm_add_ps(ra1, _mm_mul_ps(tb1, m2b1));
		__m128 re1 = _mm_add_ps(rd1, _mm_mul_ps(te1, m2b1));
		__m128 rh1 = _mm_add_ps(rg1, _mm_mul_ps(th1, m2b1));

		__m128 tc0 = _mm_splat_ps(m1a0, 2);
		__m128 tf0 = _mm_splat_ps(m1b0, 2);
		__m128 ti0 = _mm_splat_ps(m1c0, 2);
		__m128 tf1 = _mm_splat_ps(m1b1, 2);
		__m128 ti1 = _mm_splat_ps(m1c1, 2);
		__m128 tc1 = _mm_splat_ps(m1a1, 2);

		__m128 rc0 = _mm_add_ps(rb0, _mm_mul_ps(tc0, m2c0));
		__m128 rf0 = _mm_add_ps(re0, _mm_mul_ps(tf0, m2c0));
		__m128 ri0 = _mm_add_ps(rh0, _mm_mul_ps(ti0, m2c0));
		__m128 rc1 = _mm_add_ps(rb1, _mm_mul_ps(tc1, m2c1));
		__m128 rf1 = _mm_add_ps(re1, _mm_mul_ps(tf1, m2c1));
		__m128 ri1 = _mm_add_ps(rh1, _mm_mul_ps(ti1, m2c1));

		_mm_store_ps(outFloats + 0 * 12 + 0, rc0);
		_mm_store_ps(outFloats + 0 * 12 + 4, rf0);
		_mm_store_ps(outFloats + 0 * 12 + 8, ri0);
		_mm_store_ps(outFloats + 1 * 12 + 0, rc1);
		_mm_store_ps(outFloats + 1 * 12 + 4, rf1);
		_mm_store_ps(outFloats + 1 * 12 + 8, ri1);
	}

#else

	for (int i = 0; i < numJoints; i++)
	{
		idJointMat::Multiply(outJoints[i], inJoints1[i], inJoints2[i]);
	}

#endif
}

