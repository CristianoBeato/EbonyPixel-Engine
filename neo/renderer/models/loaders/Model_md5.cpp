/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
Copyright (C) 2013-2014 Robert Beckebans
Copyright (C) 2016-2018 Cristiano B. Santos

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

#include "precompiled.h"
#pragma hdrstop

#include "Model_md5.h"
#include "renderer/models/internal/Model_skined.h"
#include "renderer/models/Model_local.h"
#include "renderer/tr_local.h"


//static const char* MD5_SnapshotName = "_MD5_Snapshot_";

static const byte MD5B_VERSION = 106;
static const unsigned int MD5B_MAGIC = ( '5' << 24 ) | ( 'D' << 16 ) | ( 'M' << 8 ) | MD5B_VERSION;

idCVar r_useGPUSkinning( "r_useGPUSkinning", "1", CVAR_INTEGER, "animate normals and tangents instead of deriving" );

/***********************************************************************
	idMD5Mesh
***********************************************************************/

static int c_numVerts = 0;
static int c_numWeights = 0;
static int c_numWeightJoints = 0;

struct vertexWeight_t
{
	int							joint;
	idVec3						offset;
	float						jointWeight;
};

/*
====================
idMD5Mesh::idMD5Mesh
====================
*/
idMD5Mesh::idMD5Mesh(void) : btRenderMeshSkined()
{
	surfaceNum = 0;
}

/*
====================
idMD5Mesh::~idMD5Mesh
====================
*/
idMD5Mesh::~idMD5Mesh(void)
{

}

/*
====================
idMD5Mesh::ParseMesh
====================
*/
void idMD5Mesh::ParseMesh( idLexer& parser, int numJoints, const idJointMat* joints )
{
	idToken		token;
	idToken		name;
	
	parser.ExpectTokenString( "{" );
	
	//
	// parse name
	//
	if( parser.CheckTokenString( "name" ) )
	{
		parser.ReadToken( &name );
	}
	
	//
	// parse shader
	//
	parser.ExpectTokenString( "shader" );
	
	parser.ReadToken( &token );
	idStr shaderName = token;
	
	m_shader = declManager->FindMaterial( shaderName );
	
	//
	// parse texture coordinates
	//
	parser.ExpectTokenString( "numverts" );
	int count = parser.ParseInt();
	if( count < 0 )
	{
		parser.Error( "Invalid size: %s", token.c_str() );
	}
	
	this->m_meshNumVerts = count;
	
	idList<idVec2> texCoords;
	idList<int> firstWeightForVertex;
	idList<int> numWeightsForVertex;
	
	texCoords.SetNum( count );
	firstWeightForVertex.SetNum( count );
	numWeightsForVertex.SetNum( count );
	
	int numWeights = 0;
	int maxweight = 0;
	for( int i = 0; i < texCoords.Num(); i++ )
	{
		parser.ExpectTokenString( "vert" );
		parser.ParseInt();
		
		parser.Parse1DMatrix( 2, texCoords[ i ].ToFloatPtr() );
		
		firstWeightForVertex[ i ]	= parser.ParseInt();
		numWeightsForVertex[ i ]	= parser.ParseInt();
		
		if( !numWeightsForVertex[ i ] )
		{
			parser.Error( "Vertex without any joint weights." );
		}
		
		numWeights += numWeightsForVertex[ i ];
		if( numWeightsForVertex[ i ] + firstWeightForVertex[ i ] > maxweight )
		{
			maxweight = numWeightsForVertex[ i ] + firstWeightForVertex[ i ];
		}
	}
	
	//
	// parse tris
	//
	parser.ExpectTokenString( "numtris" );
	count = parser.ParseInt();
	if( count < 0 )
	{
		parser.Error( "Invalid size: %d", count );
	}
	
	idList<uint32> tris;
	tris.SetNum( count * 3 );
	m_meshNumTris = count;
	for( int i = 0; i < count; i++ )
	{
		parser.ExpectTokenString( "tri" );
		parser.ParseInt();
		
		tris[ i * 3 + 0 ] = parser.ParseInt();
		tris[ i * 3 + 1 ] = parser.ParseInt();
		tris[ i * 3 + 2 ] = parser.ParseInt();
	}
	
	//
	// parse weights
	//
	parser.ExpectTokenString( "numweights" );
	count = parser.ParseInt();
	if( count < 0 )
	{
		parser.Error( "Invalid size: %d", count );
	}
	
	if( maxweight > count )
	{
		parser.Warning( "Vertices reference out of range weights in model (%d of %d weights).", maxweight, count );
	}
	
	idList<vertexWeight_t> tempWeights;
	tempWeights.SetNum( count );
	assert( numJoints < 256 );		// so we can pack into bytes
	
	for( int i = 0; i < count; i++ )
	{
		parser.ExpectTokenString( "weight" );
		parser.ParseInt();
		
		int jointnum = parser.ParseInt();
		if( ( jointnum < 0 ) || ( jointnum >= numJoints ) )
		{
			parser.Error( "Joint Index out of range(%d): %d", numJoints, jointnum );
		}
		
		tempWeights[ i ].joint			= jointnum;
		tempWeights[ i ].jointWeight	= parser.ParseFloat();
		
		parser.Parse1DMatrix( 3, tempWeights[ i ].offset.ToFloatPtr() );
	}
	
	// create pre-scaled weights and an index for the vertex/joint lookup
	idVec4* scaledWeights = ( idVec4* ) Mem_Alloc16( numWeights * sizeof( scaledWeights[0] ), TAG_MD5_WEIGHT );
	int* weightIndex = ( int* ) Mem_Alloc16( numWeights * 2 * sizeof( weightIndex[0] ), TAG_MD5_INDEX );
	memset( weightIndex, 0, numWeights * 2 * sizeof( weightIndex[0] ) );
	
	count = 0;
	for( int i = 0; i < texCoords.Num(); i++ )
	{
		int num = firstWeightForVertex[i];
		for( int j = 0; j < numWeightsForVertex[i]; j++, num++, count++ )
		{
			scaledWeights[count].ToVec3() = tempWeights[num].offset * tempWeights[num].jointWeight;
			scaledWeights[count].w = tempWeights[num].jointWeight;
			weightIndex[count * 2 + 0] = tempWeights[num].joint * sizeof( idJointMat );
		}
		weightIndex[count * 2 - 1] = 1;
	}
	
	parser.ExpectTokenString( "}" );
	
	// update counters
	c_numVerts += texCoords.Num();
	c_numWeights += numWeights;
	c_numWeightJoints++;
	for( int i = 0; i < numWeights; i++ )
	{
		c_numWeightJoints += weightIndex[i * 2 + 1];
	}
	
	//
	// build a base pose that can be used for skinning
	//
	idDrawVert* basePose = ( idDrawVert* )Mem_ClearedAlloc( texCoords.Num() * sizeof( *basePose ), TAG_MD5_BASE );
	for( int j = 0, i = 0; i < texCoords.Num(); i++ )
	{
		idVec3 v = ( *( idJointMat* )( ( byte* )joints + weightIndex[j * 2 + 0] ) ) * scaledWeights[j];
		while( weightIndex[j * 2 + 1] == 0 )
		{
			j++;
			v += ( *( idJointMat* )( ( byte* )joints + weightIndex[j * 2 + 0] ) ) * scaledWeights[j];
		}
		j++;
		
		basePose[i].Clear();
		basePose[i].xyz = v;
		basePose[i].SetTexCoord( texCoords[i] );
	}
	
	// build the weights and bone indexes into the verts, so they will be duplicated
	// as necessary at mirror seems
	
	static int maxWeightsPerVert;
	static float maxResidualWeight;
	
	const int MAX_VERTEX_WEIGHTS = 4;
	
	idList< bool > jointIsUsed;
	jointIsUsed.SetNum( numJoints );
	for( int i = 0; i < jointIsUsed.Num(); i++ )
	{
		jointIsUsed[i] = false;
	}
	
	m_numMeshJoints = 0;
	m_maxJointVertDist = 0.0f;
	
	//-----------------------------------------
	// new-style setup for fixed four weights and normal / tangent deformation
	//
	// Several important models have >25% residual weight in joints after the
	// first four, which is worrisome for using a fixed four joint deformation.
	//-----------------------------------------
	for( int i = 0; i < texCoords.Num(); i++ )
	{
		idDrawVert& dv = basePose[i];
		
		// some models do have >4 joint weights, so it is necessary to sort and renormalize
		
		// sort the weights and take the four largest
		int	weights[256];
		const int numWeights = numWeightsForVertex[ i ];
		for( int j = 0; j < numWeights; j++ )
		{
			weights[j] = firstWeightForVertex[i] + j;
		}
		// bubble sort
		for( int j = 0; j < numWeights; j++ )
		{
			for( int k = 0; k < numWeights - 1 - j; k++ )
			{
				if( tempWeights[weights[k]].jointWeight < tempWeights[weights[k + 1]].jointWeight )
				{
					SwapValues( weights[k], weights[k + 1] );
				}
			}
		}
		
		if( numWeights > maxWeightsPerVert )
		{
			maxWeightsPerVert = numWeights;
		}
		
		const int usedWeights = Min( MAX_VERTEX_WEIGHTS, numWeights );
		
		float totalWeight = 0;
		for( int j = 0; j < numWeights; j++ )
		{
			totalWeight += tempWeights[weights[j]].jointWeight;
		}
		assert( totalWeight > 0.999f && totalWeight < 1.001f );
		
		float usedWeight = 0;
		for( int j = 0; j < usedWeights; j++ )
		{
			usedWeight += tempWeights[weights[j]].jointWeight;
		}
		
		const float residualWeight = totalWeight - usedWeight;
		if( residualWeight > maxResidualWeight )
		{
			maxResidualWeight = residualWeight;
		}
		
		byte finalWeights[MAX_VERTEX_WEIGHTS] = { 0 };
		byte finalJointIndecies[MAX_VERTEX_WEIGHTS] = { 0 };
		for( int j = 0; j < usedWeights; j++ )
		{
			const vertexWeight_t& weight = tempWeights[weights[j]];
			const int jointIndex = weight.joint;
			const float fw = weight.jointWeight;
			assert( fw >= 0.0f && fw <= 1.0f );
			const float normalizedWeight = fw / usedWeight;
			finalWeights[j] = idMath::Ftob( normalizedWeight * 255.0f );
			finalJointIndecies[j] = jointIndex;
		}
		
		// Sort the weights and indices for hardware skinning
		for( int k = 0; k < 3; ++k )
		{
			for( int l = k + 1; l < 4; ++l )
			{
				if( finalWeights[l] > finalWeights[k] )
				{
					SwapValues( finalWeights[k], finalWeights[l] );
					SwapValues( finalJointIndecies[k], finalJointIndecies[l] );
				}
			}
		}
		
		// Give any left over to the biggest weight
		finalWeights[0] += Max( 255 - finalWeights[0] - finalWeights[1] - finalWeights[2] - finalWeights[3], 0 );
		
		dv.color[0] = finalJointIndecies[0];
		dv.color[1] = finalJointIndecies[1];
		dv.color[2] = finalJointIndecies[2];
		dv.color[3] = finalJointIndecies[3];
		
		dv.color2[0] = finalWeights[0];
		dv.color2[1] = finalWeights[1];
		dv.color2[2] = finalWeights[2];
		dv.color2[3] = finalWeights[3];
		
		for( int j = usedWeights; j < 4; j++ )
		{
			assert( dv.color2[j] == 0 );
		}
		
		for( int j = 0; j < usedWeights; j++ )
		{
			if( !jointIsUsed[finalJointIndecies[j]] )
			{
				jointIsUsed[finalJointIndecies[j]] = true;
				m_numMeshJoints++;
			}
			const idJointMat& joint = joints[finalJointIndecies[j]];
			float dist = ( dv.xyz - joint.GetTranslation() ).Length();
			if( dist > m_maxJointVertDist )
			{
				m_maxJointVertDist = dist;
			}
		}
	}
	
	m_meshJoints = ( byte* ) Mem_Alloc( m_numMeshJoints * sizeof( m_meshJoints[0] ), TAG_MODEL );
	m_meshNumJoints = 0;
	for( int i = 0; i < numJoints; i++ )
	{
		if( jointIsUsed[i] )
		{
			m_meshJoints[m_meshNumJoints++] = i;
		}
	}
	
	// build the m_deformInfo and collect a final base pose with the mirror
	// seam verts properly including the bone weights
	m_deformInfo = R_BuildDeformInfo( texCoords.Num(), basePose, tris.Num(), tris.Ptr(),
									m_shader->UseUnsmoothedTangents() );
									
	for( int i = 0; i < m_deformInfo->numOutputVerts; i++ )
	{
		for( int j = 0; j < 4; j++ )
		{
			if( m_deformInfo->verts[i].color[j] >= numJoints )
			{
				idLib::FatalError( "Bad joint index" );
			}
		}
	}
	
	Mem_Free( basePose );
}

void idMD5Mesh::LoadBinaryMesh(idFile * file)
{
	idStr materialName;
	file->ReadString(materialName);
	if (materialName.IsEmpty())
		m_shader = NULL;
	else
		m_shader = declManager->FindMaterial(materialName);

	file->ReadBig(m_meshNumVerts);
	file->ReadBig(m_meshNumTris);

	file->ReadBig(m_numMeshJoints);
	m_meshJoints = (byte*)Mem_Alloc(m_numMeshJoints * sizeof(m_meshJoints[0]), TAG_MODEL);

	file->ReadBigArray(m_meshJoints, m_numMeshJoints);
	file->ReadBig(m_maxJointVertDist);

	m_deformInfo = (deformInfo_t*)R_ClearedStaticAlloc(sizeof(deformInfo_t));
	deformInfo_t& deform = *m_deformInfo;

	file->ReadBig(deform.numSourceVerts);
	file->ReadBig(deform.numOutputVerts);
	file->ReadBig(deform.numIndexes);
	file->ReadBig(deform.numMirroredVerts);
	file->ReadBig(deform.numDupVerts);
	file->ReadBig(deform.numSilEdges);

	srfTriangles_t	tri;
	memset(&tri, 0, sizeof(srfTriangles_t));

	if (deform.numOutputVerts > 0)
	{
		R_AllocStaticTriSurfVerts(&tri, deform.numOutputVerts);
		deform.verts = tri.verts;
		file->ReadBigArray(deform.verts, deform.numOutputVerts);
	}

	if (deform.numIndexes > 0)
	{
		R_AllocStaticTriSurfIndexes(&tri, deform.numIndexes);
		R_AllocStaticTriSurfSilIndexes(&tri, deform.numIndexes);
		deform.indexes = tri.indexes;
		deform.silIndexes = tri.silIndexes;
		file->ReadBigArray(deform.indexes, deform.numIndexes);
		file->ReadBigArray(deform.silIndexes, deform.numIndexes);
	}

	if (deform.numMirroredVerts > 0)
	{
		R_AllocStaticTriSurfMirroredVerts(&tri, deform.numMirroredVerts);
		deform.mirroredVerts = tri.mirroredVerts;
		file->ReadBigArray(deform.mirroredVerts, deform.numMirroredVerts);
	}

	if (deform.numDupVerts > 0)
	{
		R_AllocStaticTriSurfDupVerts(&tri, deform.numDupVerts);
		deform.dupVerts = tri.dupVerts;
		file->ReadBigArray(deform.dupVerts, deform.numDupVerts * 2);
	}

	if (deform.numSilEdges > 0)
	{
		R_AllocStaticTriSurfSilEdges(&tri, deform.numSilEdges);
		deform.silEdges = tri.silEdges;
		assert(deform.silEdges != NULL);
		for (int j = 0; j < deform.numSilEdges; j++)
		{
			file->ReadBig(deform.silEdges[j].p1);
			file->ReadBig(deform.silEdges[j].p2);
			file->ReadBig(deform.silEdges[j].v1);
			file->ReadBig(deform.silEdges[j].v2);
		}
	}

	idShadowVertSkinned* shadowVerts = (idShadowVertSkinned*)Mem_Alloc(ALIGN(deform.numOutputVerts * 2 * sizeof(idShadowVertSkinned), 16), TAG_MODEL);
	idShadowVertSkinned::CreateShadowCache(shadowVerts, deform.verts, deform.numOutputVerts);

	deform.staticAmbientCache = vertexCache.AllocStaticVertex(deform.verts, ALIGN(deform.numOutputVerts * sizeof(idDrawVert), VERTEX_CACHE_ALIGN));
	deform.staticIndexCache = vertexCache.AllocStaticIndex(deform.indexes, ALIGN(deform.numIndexes * sizeof(triIndex_t), INDEX_CACHE_ALIGN));
	deform.staticShadowCache = vertexCache.AllocStaticVertex(shadowVerts, ALIGN(deform.numOutputVerts * 2 * sizeof(idShadowVertSkinned), VERTEX_CACHE_ALIGN));

	Mem_Free(shadowVerts);

	file->ReadBig(surfaceNum);
}

void idMD5Mesh::WriteBinaryMesh(idFile * file)
{

	if (m_shader != NULL && m_shader->GetName() != NULL)
	{
		file->WriteString(m_shader->GetName());
	}
	else
	{
		file->WriteString("");
	}

	file->WriteBig(m_meshNumVerts);
	file->WriteBig(m_meshNumTris);

	file->WriteBig(m_numMeshJoints);
	file->WriteBigArray(m_meshJoints, m_numMeshJoints);
	file->WriteBig(m_maxJointVertDist);

	deformInfo_t& deform = *m_deformInfo;

	file->WriteBig(deform.numSourceVerts);
	file->WriteBig(deform.numOutputVerts);
	file->WriteBig(deform.numIndexes);
	file->WriteBig(deform.numMirroredVerts);
	file->WriteBig(deform.numDupVerts);
	file->WriteBig(deform.numSilEdges);

	if (deform.numOutputVerts > 0)
		file->WriteBigArray(deform.verts, deform.numOutputVerts);

	if (deform.numIndexes > 0)
	{
		file->WriteBigArray(deform.indexes, deform.numIndexes);
		file->WriteBigArray(deform.silIndexes, deform.numIndexes);
	}

	if (deform.numMirroredVerts > 0)
		file->WriteBigArray(deform.mirroredVerts, deform.numMirroredVerts);

	if (deform.numDupVerts > 0)
		file->WriteBigArray(deform.dupVerts, deform.numDupVerts * 2);

	if (deform.numSilEdges > 0)
	{
		for (int j = 0; j < deform.numSilEdges; j++)
		{
			file->WriteBig(deform.silEdges[j].p1);
			file->WriteBig(deform.silEdges[j].p2);
			file->WriteBig(deform.silEdges[j].v1);
			file->WriteBig(deform.silEdges[j].v2);
		}
	}

	file->WriteBig(surfaceNum);
}

/*
============
TransformVertsAndTangents
============
*/
void TransformVertsAndTangents( idDrawVert* targetVerts, const int numVerts, const idDrawVert* baseVerts, const idJointMat* joints )
{
	for( int i = 0; i < numVerts; i++ )
	{
		const idDrawVert& base = baseVerts[i];
		
		const idJointMat& j0 = joints[base.color[0]];
		const idJointMat& j1 = joints[base.color[1]];
		const idJointMat& j2 = joints[base.color[2]];
		const idJointMat& j3 = joints[base.color[3]];
		
		const float w0 = base.color2[0] * ( 1.0f / 255.0f );
		const float w1 = base.color2[1] * ( 1.0f / 255.0f );
		const float w2 = base.color2[2] * ( 1.0f / 255.0f );
		const float w3 = base.color2[3] * ( 1.0f / 255.0f );
		
		idJointMat accum;
		idJointMat::Mul( accum, j0, w0 );
		idJointMat::Mad( accum, j1, w1 );
		idJointMat::Mad( accum, j2, w2 );
		idJointMat::Mad( accum, j3, w3 );
		
		targetVerts[i].xyz = accum * idVec4( base.xyz.x, base.xyz.y, base.xyz.z, 1.0f );
		targetVerts[i].SetNormal( accum * base.GetNormal() );
		targetVerts[i].SetTangent( accum * base.GetTangent() );
		targetVerts[i].tangent[3] = base.tangent[3];
	}
}



/*
====================
idMD5Mesh::NearestJoint
====================
*/
int idMD5Mesh::NearestJoint( int a, int b, int c ) const
{
	// duplicated vertices might not have weights
	int vertNum;
	if( a >= 0 && a < m_meshNumVerts )
		vertNum = a;
	else if( b >= 0 && b < m_meshNumVerts)
		vertNum = b;
	else if( c >= 0 && c < m_meshNumVerts)
		vertNum = c;
	else// all vertices are duplicates which shouldn't happen
		return 0;
	
	const idDrawVert& v = m_deformInfo->verts[vertNum];
	
	int bestWeight = 0;
	int bestJoint = 0;
	for( int i = 0; i < 4; i++ )
	{
		if( v.color2[i] > bestWeight )
		{
			bestWeight = v.color2[i];
			bestJoint = v.color[i];
		}
	}
	
	return bestJoint;
}

void idMD5Mesh::Clear(void)
{
}


size_t idMD5Mesh::getMeshUsedMemory(void)
{
	size_t total;
	total += m_numMeshJoints * sizeof(m_meshJoints[0]);
	
	// sum up deform info
	total += sizeof(m_deformInfo);
	total += R_DeformInfoMemoryUsed(m_deformInfo);
	return total;
}

/***********************************************************************

	idRenderModelMD5

***********************************************************************/

/*
====================
idRenderModelMD5::ParseJoint
====================
*/
void idRenderModelMD5::ParseJoint( idLexer& parser, btGameJoint* joint, idJointQuat* defaultPose )
{
	//
	// parse name
	//
	idToken	token;
	parser.ReadToken( &token );
	joint->Name() = token;
	
	//
	// parse parent
	//
	int num = parser.ParseInt();
	if( num < 0 )
		joint->Parent() = NULL;
	else
	{
		if( num >= m_joints.Num() - 1 )
			parser.Error( "Invalid parent for joint '%s'", joint->Name().c_str() );

		joint->Parent() = &m_joints[ num ];
	}
	
	//
	// parse default pose
	//
	parser.Parse1DMatrix( 3, defaultPose->t.ToFloatPtr() );
	parser.Parse1DMatrix( 3, defaultPose->q.ToFloatPtr() );
	defaultPose->q.w = defaultPose->q.CalcW();
}

/*
====================
idRenderModelMD5::InitFromFile
====================
*/
void idRenderModelMD5::InitFromFile( const char* fileName )
{
	m_name = fileName;
	LoadModel();
}

/*
========================
idRenderModelMD5::LoadBinaryModel
========================
*/
bool idRenderModelMD5::LoadBinaryModel( idFile* file, const ID_TIME_T sourceTimeStamp )
{
	if (file == NULL)
		return false;

	unsigned int magic = 0;
	file->ReadBig( magic );
	if( magic != MD5B_MAGIC )
		return false;

	file->ReadBig(m_timeStamp);

	// RB: source might be from .resources, so we ignore the time stamp and assume a release build
	if (!fileSystem->InProductionMode() && (sourceTimeStamp != FILE_NOT_FOUND_TIMESTAMP) && (sourceTimeStamp != 0) && (sourceTimeStamp != m_timeStamp))
		return false;
	// RB end

	common->UpdateLevelLoadPacifier();

	int tempNum;
	file->ReadBig( tempNum );
	m_joints.SetNum( tempNum );
	for( int i = 0; i < m_joints.Num(); i++ )
	{
		file->ReadString(m_joints[i].Name());
		int offset;
		file->ReadBig( offset );
		if( offset >= 0 )
			m_joints[i].Parent() = m_joints.Ptr() + offset;
		else
			m_joints[i].Parent() = NULL;
	}
	
	file->ReadBig( tempNum );
	m_defaultPose.SetNum( tempNum );
	for( int i = 0; i < m_defaultPose.Num(); i++ )
	{
		file->ReadBig( m_defaultPose[i].q.x );
		file->ReadBig( m_defaultPose[i].q.y );
		file->ReadBig( m_defaultPose[i].q.z );
		file->ReadBig( m_defaultPose[i].q.w );
		file->ReadVec3( m_defaultPose[i].t );
	}
	
	file->ReadBig( tempNum );
	m_invertedDefaultPose.SetNum( tempNum );
	for( int i = 0; i < m_invertedDefaultPose.Num(); i++ )
	{
		file->ReadBigArray(m_invertedDefaultPose[ i ].ToFloatPtr(), JOINTMAT_TYPESIZE );
	}
	SIMD_INIT_LAST_JOINT(m_invertedDefaultPose.Ptr(), m_joints.Num() );
	
	file->ReadBig( tempNum );
	m_meshes.SetNum( tempNum );
	for( int i = 0; i < m_meshes.Num(); i++ )
	{
		idMD5Mesh*	mesh = dynamic_cast<idMD5Mesh*>(&m_meshes[i]);
		mesh->LoadBinaryMesh(file);
	}
	
	return true;
}

/*
========================
idRenderModelMD5::WriteBinaryModel
========================
*/
void idRenderModelMD5::WriteBinaryModel( idFile* file, ID_TIME_T* _timeStamp ) const
{
	if (file == NULL)
	{
		common->Printf("Failed to WriteBinaryModel\n");
		return;
	}
	
	file->WriteBig( MD5B_MAGIC );

	if (_timeStamp != NULL)
		file->WriteBig(*_timeStamp);
	else
		file->WriteBig(m_timeStamp);
	
	file->WriteBig( m_joints.Num() );

	for( int i = 0; i < m_joints.Num(); i++ )
	{
		file->WriteString( m_joints[i].Name() );
		int offset = -1;
		if(m_joints[i].Parent() != NULL )
		{
//Beato Begin
			//offset = joints[i]->parent - joints.Ptr();
			offset = m_joints[i].Parent() - (btGameJoint*)m_joints.Ptr();
//Beato End
		}
		file->WriteBig( offset );
	}
	
	file->WriteBig( m_defaultPose.Num() );
	for( int i = 0; i < m_defaultPose.Num(); i++ )
	{
		file->WriteBig( m_defaultPose[i].q.x );
		file->WriteBig( m_defaultPose[i].q.y );
		file->WriteBig( m_defaultPose[i].q.z );
		file->WriteBig( m_defaultPose[i].q.w );
		file->WriteVec3( m_defaultPose[i].t );
	}
	
	file->WriteBig( m_invertedDefaultPose.Num() );
	for( int i = 0; i < m_invertedDefaultPose.Num(); i++ )
	{
		file->WriteBigArray( m_invertedDefaultPose[ i ].ToFloatPtr(), JOINTMAT_TYPESIZE );
	}
	
	file->WriteBig( m_meshes.Num() );

//	for( int i = 0; i < meshes.Num(); i++ )
	//create a copy list to unconst
	idList<btRenderMeshSkined> copyList = m_meshes;
	idList<btRenderMeshSkined>::iterator mesh;
	for (mesh = copyList.begin(); mesh != copyList.end(); mesh++)
	{
		idMD5Mesh* meshRef = mesh.DynamicCast<idMD5Mesh>();// dynamic_cast<idMD5Mesh*>(meshRef);
		meshRef->WriteBinaryMesh(file);
	}
}

/*
====================
idRenderModelMD5::LoadModel

used for initial loads, reloadModel, and reloading the data of purged models
Upon exit, the model will absolutely be valid, but possibly as a default model
====================
*/
void idRenderModelMD5::LoadModel(void)
{

	int			version;
	int			num;
	int			parentNum;
	idToken		token;
	idLexer		parser( LEXFL_ALLOWPATHNAMES | LEXFL_NOSTRINGESCAPECHARS );
	
	if( !m_purged )
	{
		PurgeModel();
	}

	m_purged = false;
	
	if( !parser.LoadFile( m_name ) )
	{
		MakeDefaultModel();
		return;
	}
	
	parser.ExpectTokenString( MD5_VERSION_STRING );
	version = parser.ParseInt();
	
	if( version != MD5_VERSION )
	{
		parser.Error( "Invalid version %d.  Should be version %d\n", version, MD5_VERSION );
	}
	
	//
	// skip commandline
	//
	parser.ExpectTokenString( "commandline" );
	parser.ReadToken( &token );
	
	// parse num joints
	parser.ExpectTokenString( "numJoints" );
	num  = parser.ParseInt();
	//reserve the space for the bones
	ReserveJointsNum(num);

	// parse num meshes
	parser.ExpectTokenString( "numMeshes" );
	num = parser.ParseInt();
	if( num < 0 )
	{
		parser.Error( "Invalid size: %d", num );
	}

	m_meshes.SetGranularity( 1 );
	m_meshes.SetNum( num );
	
	//
	// parse joints
	//
	parser.ExpectTokenString( "joints" );
	parser.ExpectTokenString( "{" );
	idJointMat* poseMat = ( idJointMat* )_alloca16( m_joints.Num() * sizeof( poseMat[0] ) );
	for( int i = 0; i < m_joints.Num(); i++ )
	{
//Beato Begin
		//idMD5Joint* joint = &joints[i];
		btGameJoint* joint = &m_joints[i];
		joint->Id() = jointHandle_t(i);
		idJointQuat*	 pose = &m_defaultPose[i];
		
		ParseJoint( parser, joint, pose );
		poseMat[ i ].SetRotation( pose->q.ToMat3() );
		poseMat[ i ].SetTranslation( pose->t );
		if( joint->Parent())
		{
			//parentNum = joint->parent - (btGameJoint*)joints.Ptr();
			parentNum = joint->getParentId();
			pose->q = ( poseMat[ i ].ToMat3() * poseMat[ parentNum ].ToMat3().Transpose() ).ToQuat();
			pose->t = ( poseMat[ i ].ToVec3() - poseMat[ parentNum ].ToVec3() ) * poseMat[ parentNum ].ToMat3().Transpose();
		}
//Beato End
	}
	parser.ExpectTokenString( "}" );
	
	//generate the inverse base pose
	CreateInverseBasePose(poseMat);
	
	for( int i = 0; i < m_meshes.Num(); i++ )
	{
		parser.ExpectTokenString( "mesh" );
		idMD5Mesh* meshRef = dynamic_cast<idMD5Mesh*>(&m_meshes[i]);
		meshRef->ParseMesh( parser, m_defaultPose.Num(), poseMat );
	}
	
	// calculate the bounds of the model
	m_bounds.Clear();
	for( int i = 0; i < m_meshes.Num(); i++ )
	{
		idBounds meshBounds;
		m_meshes[i].CalculateBounds( poseMat, meshBounds );
		m_bounds.AddBounds( meshBounds );
	}
	
	// set the timestamp for reloadmodels
	fileSystem->ReadFile( m_name, NULL, &m_timeStamp );
	
	common->UpdateLevelLoadPacifier();
}


/*
==============
idRenderModelMD5::List
==============
*/
void idRenderModelMD5::List() const
{
	int			i;
	int			totalTris = 0;
	int			totalVerts = 0;
	
	const idMD5Mesh*	mesh;
	//for( mesh = ()meshes.Ptr(), i = 0; i < meshes.Num(); i++, mesh++ )
	for(btRenderMeshSkined mesh : m_meshes)
	{
		totalTris += mesh.NumTris();
		totalVerts += mesh.NumVerts();
	}
	common->Printf( " %4ik %3i %4i %4i %s(MD5)", Memory() / 1024, m_meshes.Num(), totalVerts, totalTris, Name() );
	
	if( m_defaulted )
		common->Printf( " (DEFAULTED)" );
	
	common->Printf( "\n" );
}

/*
====================
idRenderModelMD5::NearestJoint
====================
*/
int idRenderModelMD5::NearestJoint( int surfaceNum, int a, int b, int c ) const
{
	if( surfaceNum > m_meshes.Num() )
		common->Error( "idRenderModelMD5::NearestJoint: surfaceNum > meshes.Num()" );
	
	const idMD5Mesh* mesh = (idMD5Mesh*)m_meshes.Ptr();
	for( int i = 0; i < m_meshes.Num(); i++, mesh++ )
	{
		if( mesh->surfaceNum == surfaceNum )
			return mesh->NearestJoint( a, b, c );
	}
	return 0;
}

/*
====================
idRenderModelMD5::TouchData

models that are already loaded at level start time
will still touch their materials to make sure they
are kept loaded
====================
*/
void idRenderModelMD5::TouchData()
{
	for( int i = 0; i < m_meshes.Num(); i++ )
		declManager->FindMaterial(m_meshes[i].getShaderName());
}

/*
===================
idRenderModelMD5::PurgeModel

frees all the data, but leaves the class around for dangling references,
which can regenerate the data with LoadModel()
===================
*/
void idRenderModelMD5::PurgeModel()
{
	m_purged = true;
	m_joints.Clear();
	m_defaultPose.Clear();
	m_meshes.Clear();
}

/*
===================
idRenderModelMD5::Memory
===================
*/
int	idRenderModelMD5::Memory() const
{
	int total = sizeof( *this );
	total += m_joints.MemoryUsed() + m_defaultPose.MemoryUsed() + m_meshes.MemoryUsed();
	
	// count up strings
	for( int i = 0; i < m_joints.Num(); i++ )
	{
		total += m_joints[i].Name().DynamicMemoryUsed();
	}
	
	// count up meshes
	// for( int i = 0; i < meshes.Num(); i++ )
	// create a copy list to unconst
	idList<btRenderMeshSkined> copyList = m_meshes;
	idList<btRenderMeshSkined>::iterator mesh;
	for (mesh = copyList.begin(); mesh != copyList.end(); mesh++)
	{
		idMD5Mesh* meshRef = mesh.DynamicCast<idMD5Mesh>();//dynamic_cast<idMD5Mesh*>(&meshes[i]);
		//get the amouth of memory used by mesh joints
		total += meshRef->getMeshUsedMemory();
	}
	return total;
}
