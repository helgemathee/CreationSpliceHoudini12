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



};

};
using namespace HDK_BulletSOP_Splice;
