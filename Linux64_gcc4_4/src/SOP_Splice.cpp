/**
Houdini Splice v1.0
Copyright (c)2013 Suk Milan(milansuk86@gmail.com). All Rights Reserved.
This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
This file may not be redistributed in whole or significant part.
This notice may not be removed or altered from any source distribution.
*/
#include "SOP_Splice.h"


#define FEC_STATIC
#define FECS_STATIC
#include <CreationSplice.h>
using namespace CreationSplice;



OP_ERROR SOP_Splice::cookMySop(OP_Context &context)
{
	m_time = context.getTime();
 	m_boss = UTgetInterrupt();
	if (lockInputs(context) >= UT_ERROR_ABORT)
		return error();

	m_input1 = (GU_Detail*)inputGeo(0, context);
	duplicateSource(0, context);


	//Splice test...
	Initialize();
	Node node1;
	Node node2("myKLEnabledNode");	//this creates crash!
	Finalize();


	unlockInputs();
	return error();
}


OP_ERROR SOP_Splice::getOut(const char* err, const char* warning)
{
	if(err)		if(strlen(err))		addError(SOP_MESSAGE, err);
	if(warning)	if(strlen(warning))	addWarning(SOP_MESSAGE, warning);
	
	unlockInputs();
	return error();
}



const char *SOP_Splice::inputLabel(unsigned inum) const
{
    switch (inum)
    {
	case 0: return "Input1";
    }
    return "Unknown source"; 
}

bool SOP_Splice::updateParmsFlags()
{
	bool changed = SOP_Node::updateParmsFlags();

	return changed;
}

OP_Node *SOP_Splice::myConstructor(OP_Network *net, const char *name, OP_Operator *op)
{
    return new SOP_Splice(net, name, op);
}
SOP_Splice::SOP_Splice(OP_Network *net, const char *name, OP_Operator *op)
	: SOP_Node(net, name, op)
{
	getFullPath(m_operatorName);
}
SOP_Splice::~SOP_Splice()
{
}

//main procedure for library(plugin)
void newSopOperator(OP_OperatorTable *table)
{
	table->addOperator(new OP_Operator(
						"splice",
						"splice",
						HDK_BulletSOP_Splice::SOP_Splice::myConstructor,
						HDK_BulletSOP_Splice::SOP_Splice::myTemplateList,
						1,
						1,
						0));
}



PRM_Template SOP_Splice::myTemplateList[] = {

    PRM_Template()
};
