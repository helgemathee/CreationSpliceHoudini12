/**
Houdini Splice v1.0
Copyright (c)2013 Suk Milan(milansuk86@gmail.com). All Rights Reserved.
This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
This file may not be redistributed in whole or significant part.
This notice may not be removed or altered from any source distribution.
*/

/*
Install:
copy "SOP_Splice.so" file and "RT", "Exts" folders to /home/ubuntu/houdini12.5/dso

run console and write:
	export FABRIC_RT_PATH=/home/ubuntu/houdini12.5/dso/RT
	export FABRIC_EXTS_PATH=/home/ubuntu/houdini12.5/dso/Exts

	cd /opt/hfs12.5.469/
	source houdini_setup

	cd < "path to your install folder" /CreationSpliceHoudini12_linux64_gcc4_4>
	sh build.sh

	houdini -foreground

Enjoy it!
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


/*
	//Splice test...
	Node node("myKLEnabledNode");

	// create an operator
	std::string klCode = "";
	klCode += "operator helloWorldOp() {\n";
	klCode += "  report('Hello World from KL!');\n";
	klCode += "}\n";
	node.constructKLOperator("helloWorldOp", klCode.c_str());

	//evaluate the node
	node.evaluate();
*/


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
	case 0: return "Input 1";
	case 1: return "Input 2";
	case 2: return "Input 3";
	case 3: return "Input 4";
    }
    return "Unknown source"; 
}




int SOP_Splice::getTypeSize(int type)
{
	switch(type)
	{
	case SCALAR:	return 1;
	case FLOAT2:	return 2;
	case FLOAT3:	return 3;
	case FLOAT4:	return 4;
	case MAT44:		return 16;
	}
	return 0;
}


bool SOP_Splice::updateParmsFlags()
{
	bool changed = SOP_Node::updateParmsFlags();

	for(int i=1; i < LIST_VALUE_NUM()+1; i++)
	{
		int state = evalIntInst("varv_on", &i, 0, m_time, 1);

		changed |= enableParmInst("varv_name", &i, state, 1);
		changed |= enableParmInst("varv_type", &i, state, 1);
		changed |= enableParmInst("varv_mode", &i, state, 1);

		const int tsize = getTypeSize( LIST_VALUE_TYPE(i) );
		for(int ii=0; ii < 4; ii++)
			changed |= enableParmInst("varv_value", &i, state && ii < tsize, 1, ii);
	}

	for(int i=1; i < LIST_PARAM_NUM()+1; i++)
	{
		int state = evalIntInst("varp_on", &i, 0, m_time, 1);

		changed |= enableParmInst("varp_name", &i, state, 1);
		changed |= enableParmInst("varp_type", &i, state, 1);
		changed |= enableParmInst("varp_mode", &i, state, 1);
		changed |= enableParmInst("varp_path", &i, state, 1);
		changed |= enableParmInst("varp_param", &i, state, 1);
	}

	for(int i=1; i < LIST_ATTR_NUM()+1; i++)
	{
		int state = evalIntInst("vara_on", &i, 0, m_time, 1);

		changed |= enableParmInst("vara_name", &i, state, 1);
		changed |= enableParmInst("vara_type", &i, state, 1);
		changed |= enableParmInst("vara_mode", &i, state, 1);
		changed |= enableParmInst("vara_path", &i, state, 1);
		changed |= enableParmInst("vara_attr", &i, state, 1);
		changed |= enableParmInst("vara_itype", &i, state, 1);

		UT_String path;		evalStringInst("vara_path", &i, path, 0, m_time, 1);
		changed |= enableParmInst("vara_inum", &i, state && !path.length(), 1);
	}

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


static void dsoExit(void *)
{
//	Finalize();	//Clean Splice
}



//main procedure for library(plugin)
void newSopOperator(OP_OperatorTable *table)
{
	UT_Exit::addExitCallback(dsoExit);

//	Initialize();	//Init Splice

	table->addOperator(new OP_Operator(
						"splice",
						"splice",
						HDK_BulletSOP_Splice::SOP_Splice::myConstructor,
						HDK_BulletSOP_Splice::SOP_Splice::myTemplateList,
						1,
						4,
						0));
}




/**
GUI
*/

static PRM_Name typeNames[] = {
	PRM_Name("0",	"Scalar"),
	PRM_Name("1",	"Float2"),
	PRM_Name("2",	"Float3"),
	PRM_Name("3",	"Float4"),
	PRM_Name("4",	"Mat44"),
	PRM_Name(0)
};
static PRM_Name inputNames[] = {
	PRM_Name("0",	"In"),
	PRM_Name("1",	"Out"),
	PRM_Name("2",	"I/O"),
	PRM_Name(0)
};
static PRM_Name inputTypeNames[] = {
	PRM_Name("0",	"Point"),
	PRM_Name("2",	"Vertex"),
	PRM_Name("1",	"Primitive"),
	PRM_Name("3",	"Detail"),
	PRM_Name(0)
};
static PRM_ChoiceList   typeMenu((PRM_ChoiceListType)(PRM_CHOICELIST_SINGLE), &(typeNames[0]) );
static PRM_ChoiceList   inputMenu((PRM_ChoiceListType)(PRM_CHOICELIST_SINGLE), &(inputNames[0]) );
static PRM_ChoiceList   inputTypeMenu((PRM_ChoiceListType)(PRM_CHOICELIST_SINGLE), &(inputTypeNames[0]) );



static PRM_Name names_value[] = {
	PRM_Name("varv_on", "var_on"),
	PRM_Name("varv_name", "Name"),
	PRM_Name("varv_type", "Type"),
	PRM_Name("varv_mode", "Mode"),
	PRM_Name("varv_value", "Value"),
};
PRM_Template listValue[] = {
	PRM_Template(PRM_TOGGLE	|PRM_TYPE_JOIN_NEXT|PRM_TYPE_LABEL_NONE,1, &names_value[0], PRMoneDefaults),
	PRM_Template(PRM_STRING	|PRM_TYPE_JOIN_NEXT,					1, &names_value[1], 0),
	PRM_Template(PRM_INT	|PRM_TYPE_JOIN_NEXT|PRM_TYPE_LABEL_NONE,1, &names_value[2], 0, &typeMenu),
	PRM_Template(PRM_INT	|PRM_TYPE_JOIN_NEXT|PRM_TYPE_LABEL_NONE,1, &names_value[3], 0, &inputMenu),
	PRM_Template(PRM_FLT,											4, &names_value[4], 0),
    PRM_Template()
};



static PRM_Name names_paramter[] = {
	PRM_Name("varp_on", "var_on"),
	PRM_Name("varp_name", "Name"),
	PRM_Name("varp_type", "Type"),
	PRM_Name("varp_mode", "Mode"),
	PRM_Name("varp_path", "Path"),
	PRM_Name("varp_param", "Param"),
};
PRM_Template listParameter[] = {
	PRM_Template(PRM_TOGGLE	|PRM_TYPE_JOIN_NEXT|PRM_TYPE_LABEL_NONE,	1, &names_paramter[0], PRMoneDefaults),
	PRM_Template(PRM_STRING	|PRM_TYPE_JOIN_NEXT,						1, &names_paramter[1], 0),
	PRM_Template(PRM_INT	|PRM_TYPE_JOIN_NEXT|PRM_TYPE_LABEL_NONE,	1, &names_paramter[2], 0, &typeMenu),
	PRM_Template(PRM_INT	|PRM_TYPE_JOIN_NEXT|PRM_TYPE_LABEL_NONE,	1, &names_paramter[3], 0, &inputMenu),
	PRM_Template(PRM_STRING_E|PRM_TYPE_JOIN_NEXT, PRM_TYPE_DYNAMIC_PATH,1, &names_paramter[4], 0, 0, 0, 0, &PRM_SpareData::sopPath),
	PRM_Template(PRM_STRING,											1, &names_paramter[5], 0),
	PRM_Template()
};



static PRM_Name names_attribute[] = {
	PRM_Name("vara_on", "var_on"),
	PRM_Name("vara_name", "Name"),
	PRM_Name("vara_type", "Type"),
	PRM_Name("vara_mode", "Mode"),
	PRM_Name("vara_path", "Path"),
	PRM_Name("vara_inum", "inum"),
	PRM_Name("vara_attr", "Attribute"),
	PRM_Name("vara_itype", "Type"),
};
PRM_Template listAttributes[] = {
	PRM_Template(PRM_TOGGLE	|PRM_TYPE_JOIN_NEXT		|PRM_TYPE_LABEL_NONE,	1, &names_attribute[0], PRMoneDefaults),
	PRM_Template(PRM_STRING	|PRM_TYPE_JOIN_NEXT,							1, &names_attribute[1], 0),
	PRM_Template(PRM_INT	|PRM_TYPE_JOIN_NEXT		|PRM_TYPE_LABEL_NONE,	1, &names_attribute[2], 0, &typeMenu),
	PRM_Template(PRM_INT	|PRM_TYPE_JOIN_NEXT		|PRM_TYPE_LABEL_NONE,	1, &names_attribute[3], 0, &inputMenu),
	PRM_Template(PRM_STRING_E|PRM_TYPE_JOIN_NEXT, PRM_TYPE_DYNAMIC_PATH,	1, &names_attribute[4], 0, 0, 0, 0, &PRM_SpareData::sopPath),
	PRM_Template(PRM_INT	|PRM_TYPE_JOIN_NEXT		|PRM_TYPE_LABEL_NONE,	1, &names_attribute[5], PRMoneDefaults, 0, &g_range_one_to_four),
	PRM_Template(PRM_STRING	|PRM_TYPE_JOIN_NEXT,							1, &names_attribute[6], 0),
	PRM_Template(PRM_INT	|PRM_TYPE_LABEL_NONE,							1, &names_attribute[7], 0, &inputTypeMenu),
	PRM_Template()
};




static PRM_Name names[] = {
	PRM_Name("list_value", "Values"),			PRM_Name("sep1", "sep1"),
	PRM_Name("list_parameter", "Parameters"),	PRM_Name("sep2", "sep2"),
	PRM_Name("list_attribute", "Attributes"),	PRM_Name("sep3", "sep3"),
	PRM_Name("sep4", "sep4"),
	PRM_Name("code", "KL Code"),
};



PRM_Template SOP_Splice::myTemplateList[] =
{
	PRM_Template(PRM_MULTITYPE_LIST, listValue, 5, &names[0], 0),		PRM_Template(PRM_SEPARATOR, 1, &names[1]),
	PRM_Template(PRM_MULTITYPE_LIST, listParameter, 6, &names[2], 0),	PRM_Template(PRM_SEPARATOR, 1, &names[3]),
	PRM_Template(PRM_MULTITYPE_LIST, listAttributes, 8, &names[4], 0),	PRM_Template(PRM_SEPARATOR, 1, &names[5]),

	PRM_Template(PRM_SEPARATOR, 1, &names[6]),

	PRM_Template(PRM_STRING, 1, &names[7], 0, 0, 0, 0, &PRM_SpareData::stringEditor),

    PRM_Template()
};
