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

#ifndef __DINAMIC_MODEL_H__
#define __DINAMIC_MODEL_H__

struct deformInfo_t;

/*
===============================================================================
game joint
===============================================================================
*/
class btGameJoint
{
public:
	btGameJoint(void);
	btGameJoint(const char* name, const jointHandle_t id, const btGameJoint* parent = nullptr);

	//return the id handler
	jointHandle_t	&			Id(void);			//return the joint id
	const jointHandle_t			Id(void) const;		//return the joint id
	idStr &						Name(void);			//return the joint name
	const idStr					Name(void) const;	//return the joint name
	const btGameJoint*	&		Parent(void);		//get the parent pointer
	const btGameJoint*			Parent(void) const; //get the parent pointer

	const char*		getParentName(void) const;	//get the name of the parent joint
	jointHandle_t	getParentId(void) const;	//get the parent joint id, return -1 if root

private:
	idStr						m_name;
	jointHandle_t				m_jointId;
	const btGameJoint* 			m_parent;
};

/*
===============================================================================
	Skined Mesh
===============================================================================
*/
class btRenderMeshSkined
{
public:
	btRenderMeshSkined(void);
	virtual	~btRenderMeshSkined(void);

	virtual void		CalculateBounds(const idJointMat* entJoints, idBounds& bounds) const;

	virtual void		UpdateSurface(const struct renderEntity_s* ent,
		const idJointMat* joints,
		const	idJointMat* entJointsInverted,
		modelSurface_t* surf);

	virtual int			NumVerts(void) const;
	virtual int			NumTris(void) const;

	const char*			getShaderName(void) const;

	virtual void		Clear(void);

protected:
	friend class btRenderModelSkined;

	idStr						m_meshName;			// mesh name
	const idMaterial* 			m_shader;			// material applied to mesh
	deformInfo_t* 				m_deformInfo;		// used to create srfTriangles_t from base frames and new vertexes
	unsigned int				m_meshNumJoints;	// num joits at curr mesh
	unsigned int				m_meshNumVerts;		// number of vertices
	unsigned int				m_meshNumTris;		// number of triangles
	unsigned int				m_numMeshJoints;	// number of mesh joints
	byte* 						m_meshJoints;		// the joints used by this mesh
	float						m_maxJointVertDist;	// maximum distance a vertex is separated from a joint
	int							m_surfaceNum;			// number of the static surface created for this mesh

};

/*
===============================================================================
Model Dinamic
Vertex animated, and skined meshes base
===============================================================================
*/
class btRenderModelSkined : public idRenderModelLocal
{
public:
	btRenderModelSkined(void);
	~btRenderModelSkined(void);

	virtual void					InitFromFile(const char* fileName) {};
	virtual bool					LoadBinaryModel(idFile* file, const ID_TIME_T sourceTimeStamp) { return false; };
	virtual void					WriteBinaryModel(idFile* file, ID_TIME_T* _timeStamp = NULL) const {};
	virtual bool					SupportsBinaryModel(void) { return false; }

	virtual void					Clear(void);

	//clear model 
	virtual void					PurgeModel(void);

	//get the model bounds
	virtual idBounds				Bounds(const struct renderEntity_s* ent = NULL) const;

	//get the model dinamic type
	virtual dynamicModel_t			IsDynamicModel(void) const;

	//show on console the model sumary
	virtual void					Print(void) const;

	//count the joints in the skeleton
	virtual int						NumJoints(void) const;

	//get the joint list
	virtual const idList<btGameJoint>	GetJoints(void) const;

	//get the handle of the gived joint name 
	virtual jointHandle_t			GetJointHandle(const char* name) const;

	//get the name of the joit at gived handler, if exist
	virtual const char* 			GetJointName(jointHandle_t handle) const;

	//get the defalt skeleton pose
	virtual const idJointQuat* 		GetDefaultPose(void) const;

	//calc the nearest joint at position
	virtual int						NearestJoint(int surfaceNum, int a, int c, int b) const;

protected:
	//alloc joints
	void							ReserveJointsNum(const unsigned int num);

	//process the inverse pose
	virtual void					CreateInverseBasePose(const idJointMat* poseMat);

	//create the render model
	virtual idRenderModel* 			InstantiateDynamicModel(const struct renderEntity_s* ent,
		const viewDef_t* view, 
		idRenderModel* cachedModel);

	//idList Meshes
	idList<btRenderMeshSkined, TAG_MODEL>	m_meshes;
	//store the joints names and ident
	idList<btGameJoint, TAG_MODEL>			m_joints;
	//the base skeleton pose
	idList<idJointQuat, TAG_MODEL>			m_defaultPose;
	//inverse base pose of the skeletons
	idList<idJointMat, TAG_MODEL>			m_invertedDefaultPose;

private:
	void			DrawJoints(const renderEntity_t* ent, const viewDef_t* view) const;
	static void		TransformJoints(idJointMat* __restrict outJoints,
		const int numJoints, const idJointMat* __restrict inJoints1,
		const idJointMat* __restrict inJoints2);

public:
	// when an skined is instantiated, the inverted joints array is stored to allow GPU skinning
	int							m_numInvertedJoints;
	idJointMat* 				m_jointsInverted;
	vertCacheHandle_t			m_jointsInvertedBuffer;
};

#endif /* !__DINAMIC_MODEL_H__ */
