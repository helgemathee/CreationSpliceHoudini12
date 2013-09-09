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


#define FEC_STATIC
#define FECS_STATIC
#include <CreationSplice.h>
//using namespace CreationSplice;


/**
Reading parameters(GUI)
*/
#define INT_PARM(name, vidx, t)	\
	    return (int)evalInt(name, vidx, t);
#define FLT_PARM(name, vidx, t)	\
	    return (float)evalFloat(name, vidx, t);
#define V3_PARM(name, t)	\
	    return UT_Vector3( (float)evalFloat(name, 0, t), (float)evalFloat(name, 1, t), (float)evalFloat(name, 2, t));
#define STRING_PARM(name, t, str)	\
	    evalString(str, name, 0, t);	return str;


#define INT_PARM_MULT(name, vidx, t, inst)	\
	    return (int)evalIntInst(name, inst, vidx, t, 1);
#define FLT_PARM_MULT(name, vidx, t, inst)	\
	    return (float)evalFloatInst(name, inst, vidx, t, 1);
#define V3_PARM_MULT(name, t, inst)	\
	    return UT_Vector3( (float)evalFloatInst(name, inst, 0, t, 1), (float)evalFloatInst(name, inst, 1, t, 1), (float)evalFloatInst(name, inst, 2, t,1));

#define STRING_PARM_MULT(name, t, str, inst)	\
	    evalStringInst(name, inst, str, 0, t);	return str;

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
	OP_Context m_context;
	UT_String m_operatorPath;
	UT_String m_operatorName;
	GU_Detail* m_input[4];

	CreationSplice::Node* m_node;

	enum TYPE
	{
		SCALAR,
		VEC2,
		VEC3,
		VEC4,
		AVEC4,
		MAT44,
	};

	enum MODE
	{
		IN,
		OUT,
		IO,
	};


	enum ATTRTYPE
	{
		POINT,
		VERTEX,
		PRIMITIVE,
		DETAIL,
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
	bool isMemberExists(const char* name);
	bool isSpliceNodeChanged();

	bool copySpliceToHoudini();
	bool deleteSpliceNode();
	bool createSpliceNode();

	bool createSpliceMembersV();
	bool createSpliceMembersP();
	bool createSpliceMembersA();

	bool copyHoudiniToSpliceV();
	bool copyHoudiniToSpliceP();
	bool copyHoudiniToSpliceA();


	int getTypeSize(int type);
	const char* getType(int type);
	CreationSplice::Port_Mode getMode(int type);
	bool isModeOut(int mode);
	GEO_AttributeHandle getAttrType(GU_Detail* my_gdp, int type, const char* name);


	int LISTV_NUM() { INT_PARM("list_value", 0, m_time) }
	int LISTP_NUM() { INT_PARM("list_parameter", 0, m_time) }
	int LISTA_NUM() { INT_PARM("list_attribute", 0, m_time) }


	int LISTV_ENABLE(int inst)						{ INT_PARM_MULT("varv_on", 0, m_time, &inst); }
   	UT_String& LISTV_NAME(int inst, UT_String &str)	{ STRING_PARM_MULT("varv_name", m_time, str, &inst); }
	int LISTV_TYPE(int inst)						{ INT_PARM_MULT("varv_type", 0, m_time, &inst) }
	float LISTV_VALUE(int inst, int vidx)			{ FLT_PARM_MULT("varv_value", vidx, m_time, &inst) }

	
	int LISTP_ENABLE(int inst)						{ INT_PARM_MULT("varp_on", 0, m_time, &inst); }
   	UT_String& LISTP_NAME(int inst, UT_String &str)	{ STRING_PARM_MULT("varp_name", m_time, str, &inst); }
	int LISTP_TYPE(int inst)						{ INT_PARM_MULT("varp_type", 0, m_time, &inst) }
	UT_String& LISTP_PATH(int inst, UT_String &str)	{ STRING_PARM_MULT("varp_path", m_time, str, &inst); }
	UT_String& LISTP_PARAM(int inst, UT_String &str){ STRING_PARM_MULT("varp_param", m_time, str, &inst); }


	
	int LISTA_ENABLE(int inst)						{ INT_PARM_MULT("vara_on", 0, m_time, &inst); }
   	UT_String& LISTA_NAME(int inst, UT_String &str)	{ STRING_PARM_MULT("vara_name", m_time, str, &inst); }
	int LISTA_TYPE(int inst)						{ INT_PARM_MULT("vara_type", 0, m_time, &inst) }
	int LISTA_MODE(int inst)						{ INT_PARM_MULT("vara_mode", 0, m_time, &inst) }
	UT_String& LISTA_PATH(int inst, UT_String &str)	{ STRING_PARM_MULT("vara_path", m_time, str, &inst); }
	int LISTA_INUM(int inst)						{ INT_PARM_MULT("vara_inum", 0, m_time, &inst) }
	UT_String& LISTA_ATTR(int inst, UT_String &str)	{ STRING_PARM_MULT("vara_attr", m_time, str, &inst); }
	int LISTA_ITYPE(int inst)						{ INT_PARM_MULT("vara_itype", 0, m_time, &inst) }







   	UT_String& CODE(UT_String &str)	{ STRING_PARM("code", m_time, str); }




};

};
using namespace HDK_BulletSOP_Splice;
