/**
Houdini Splice v1.0
Copyright (c)2013 Suk Milan(milansuk86@gmail.com). All Rights Reserved.
This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
This file may not be redistributed in whole or significant part.
This notice may not be removed or altered from any source distribution.
*/
#pragma once

#include <UT/UT_DSOVersion.h>
#include <UT/UT_Exit.h>

#include <UT/UT_Math.h>
#include <UT/UT_Interrupt.h>
#include <UT/UT_Vector3.h>
#include <UT/UT_Vector4.h>
#include <UT/UT_Matrix4.h>

#include <GEO/GEO_PrimPart.h>
#include <GEO/GEO_AttributeHandle.h>
#include <GEO/GEO_AttributeHandleList.h>
#include <GEO/GEO_Closure.h>
#include <GEO/GEO_AdjPolyIterator.h>

#include <GU/GU_Detail.h>
#include <GU/GU_PrimPoly.h>
#include <GU/GU_PrimSphere.h>
#include <GU/GU_PrimVolume.h>
#include <GU/GU_RayIntersect.h>
#include <GU/GU_SDF.h>

#include <GQ/GQ_Detail.h>

#include <PRM/PRM_Include.h>
#include <PRM/PRM_ChoiceList.h>
#include <PRM/PRM_SpareData.h>
#include <PRM/PRM_Parm.h>

#include <OP/OP_Operator.h>
#include <OP/OP_OperatorTable.h>
#include <OP/OP_Director.h>

#include <SOP/SOP_Guide.h>
#include <SOP/SOP_Node.h>


#include <time.h>
#include <stdio.h>

#include <cstdlib>
#include <vector>


/**
Reading parameters(GUI)
*/
#define INT_PARM(name, vidx, t)	\
	    return (int)evalInt(name, vidx, t);
#define FLT_PARM(name, vidx, t)	\
	    return (float)evalFloat(name, vidx, t);
#define V3_PARM(name, t)	\
	    return UT_Vector3( (float)evalFloat(name, 0, t), (float)evalFloat(name, 1, t), (float)evalFloat(name, 2, t));

#define INT_PARM_MULT(name, vidx, t, inst)	\
	    return (int)evalIntInst(name, inst, vidx, t, 1);
#define FLT_PARM_MULT(name, vidx, t, inst)	\
	    return (float)evalFloatInst(name, inst, vidx, t, 1);
#define V3_PARM_MULT(name, t, inst)	\
	    return UT_Vector3( (float)evalFloatInst(name, inst, 0, t, 1), (float)evalFloatInst(name, inst, 1, t, 1), (float)evalFloatInst(name, inst, 2, t,1));

/**
Paramters range <>
*/
static PRM_Range g_range_zero_more(PRM_RANGE_RESTRICTED, 0.0, PRM_RANGE_FREE, 10000000000);	//0+
static PRM_Range g_range_one_more(PRM_RANGE_RESTRICTED, 1, PRM_RANGE_FREE, 10000000000);	//1+
static PRM_Range g_range_zero_min_more(PRM_RANGE_RESTRICTED, 0.0001f, PRM_RANGE_FREE, 10000000000);	//0.0001+
static PRM_Range g_range_minus_one_more(PRM_RANGE_RESTRICTED, -1, PRM_RANGE_FREE, 10000000000);	//(-1)+

static PRM_Range g_range_one_to_four(PRM_RANGE_RESTRICTED, 1, PRM_RANGE_RESTRICTED, 4);	//1-4



namespace HDK_BulletSOP_Splice{

/**
...
*/
class SOP_Splice : public SOP_Node
{
private:
	double m_time;
	UT_Interrupt* m_boss;
	UT_String m_operatorName;
	GU_Detail* m_input1;

	enum TYPE
	{
		SCALAR,
		FLOAT2,
		FLOAT3,
		FLOAT4,
		MAT44,
	};

public:
	SOP_Splice(OP_Network *net, const char *name, OP_Operator *op);
	virtual ~SOP_Splice();

	static PRM_Template myTemplateList[];
	static OP_Node *myConstructor(OP_Network*, const char *, OP_Operator *);

protected:
	virtual const char* inputLabel(unsigned idx) const;
	virtual OP_ERROR cookMySop(OP_Context &context);
	virtual bool updateParmsFlags();
	OP_ERROR getOut(const char* err, const char* warning);


private:
	int getTypeSize(int type);



	int LIST_VALUE_NUM() { INT_PARM("list_value", 0, m_time) }
	int LIST_PARAM_NUM() { INT_PARM("list_parameter", 0, m_time) }
	int LIST_ATTR_NUM() { INT_PARM("list_attribute", 0, m_time) }


	int LIST_VALUE_TYPE(int i) { INT_PARM_MULT("varv_type", 0, m_time, &i) }


};

};
using namespace HDK_BulletSOP_Splice;
