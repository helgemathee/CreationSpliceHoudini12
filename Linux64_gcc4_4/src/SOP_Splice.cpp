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


void myLogFunc(const char * message, unsigned int length)
{
//	printf("[myLogFunc] %s\n", message);
}

void myLogErrorFunc(const char * message, unsigned int length)
{
	printf("[myLogErrorFunc] Error: %s\n", message);
}

void myCompilerErrorFunc(
unsigned int row, 
unsigned int col, 
const char * file,
const char * level,
const char * desc
)
{
	printf("[myCompilerErrorFunc] KL Error: %s, Line %d, Col %d: %s\n", file, (int)row, (int)col, desc);
}

void myKLReportFunc(const char * message, unsigned int length)
{
	printf("[myKLReportFunc] KL Reports: %s\n", message);
}

void myKLStatusFunc(const char * topic, unsigned int topicLength,  const char * message, unsigned int messageLength)
{
	printf("[myKLStatusFunc] KL Status for '%s': %s\n", topic, message);

	if(std::string(topic) == "licensing")
	{

		//sets and checks license
		try
		{
			CreationSplice::setStandaloneLicense("LICENSE fabricinc fabric 1.0 5-sep-2014 uncounted hostid=8c89a5802e1a share=uh issued=5-sep-2013 _ck=35099c32d2 sig=\"60Q04580H2ED98XMDQ44R4F42GHRWR22YNRFM1X308AG13GKHQEEFPX6TDE10BYE50VKFH5E3P574\"");
		}
		catch(CreationSplice::Exception e)
		{
			printf("Caught error: %s\n", e.what());
		}

		if(CreationSplice::isLicenseValid())	printf("Splice license has been validated successfully!\n");
		else									printf("Splice license is not valid!\n");
	}
}




OP_ERROR SOP_Splice::cookMySop(OP_Context &context)
{
	m_context = context;
	m_time = context.getTime();
	m_boss = UTgetInterrupt();
	OP_Node::flags().timeDep = 1;	//update every frame

	if (lockInputs(context) >= UT_ERROR_ABORT)
		return error();

	for(int i=0; i < 4; i++)
		m_input[i] = (GU_Detail*)inputGeo(i, context);

	duplicateSource(0, context);


	if(!m_node)
	{
		if(!createSpliceNode())
			return error();
	}
	else
	{
		if(isSpliceNodeChanged())
		{
			printf("Recreating Splice node!\n");
			if(!createSpliceNode())
				return error();
		}
	}

	copyHoudiniToSpliceV();
	copyHoudiniToSpliceP();
	copyHoudiniToSpliceA();

	m_node->evaluate();

	copySpliceToHoudini();


	unlockInputs();
	return error();
}



void addPortString(UT_String &str, UT_String name)
{
	str = name;
	str += UT_String("Port");
}



bool SOP_Splice::createSpliceMembersV()
{
	for(int i=1; i < LISTV_NUM()+1; i++)
	{
		if(LISTV_ENABLE(i))
		{
			UT_String name;
			LISTV_NAME(i, name);
			if(name.length() > 0)
			{
				UT_String namePort;	addPortString(namePort, name);
				try
				{
					m_node->addMember(name.buffer(), getType( LISTV_TYPE(i) ));
					m_node->addPort(namePort.buffer(), name.buffer(), CreationSplice::Port_Mode_IN);
				}
				catch(CreationSplice::Exception e)
				{
					printf("createSpliceMembersV() error: %s\n", e.what());
					return false;
				}
			}
			else
			{
				//you don't set a name
				return false;
			}
		}
	}

	return true;
}

bool SOP_Splice::createSpliceMembersP()
{
	for(int i=1; i < LISTP_NUM()+1; i++)
	{
		if(LISTP_ENABLE(i))
		{
			UT_String name;
			LISTP_NAME(i, name);
			if(name.length() > 0)
			{
				UT_String namePort;	addPortString(namePort, name);

				try
				{
					m_node->addMember(name.buffer(), getType( LISTP_TYPE(i) ));
					m_node->addPort(namePort.buffer(), name.buffer(), CreationSplice::Port_Mode_IN);
				}
				catch(CreationSplice::Exception e)
				{
					printf("createSpliceMembersP() error: %s\n", e.what());
					return false;
				}
			}
			else
			{
				//you don't set a name
				return false;
			}
		}
	}

	return true;
}

bool SOP_Splice::createSpliceMembersA()
{
	for(int i=1; i < LISTA_NUM()+1; i++)
	{
		if(LISTA_ENABLE(i))
		{
			UT_String name;
			LISTA_NAME(i, name);
			if(name.length() > 0)
			{
				UT_String namePort;	addPortString(namePort, name);

				m_node->addMember(name.buffer(), getType( LISTA_TYPE(i) ));
				m_node->addPort(namePort.buffer(), name.buffer(), getMode( LISTA_MODE(i) ));
			}	
			else
			{
				//you don't set a name
				return false;
			}
		}
	}

	return true;
}




bool SOP_Splice::copyHoudiniToSpliceV()
{
	for(int i=1; i < LISTV_NUM()+1; i++)
	{
		if(LISTV_ENABLE(i))
		{
			UT_String name;
			LISTV_NAME(i, name);
			if(name.length() > 0)
			{
				try
				{
					UT_String namePort;	addPortString(namePort, name);
					CreationSplice::Port port = m_node->getPort(namePort.buffer());

					const int tsize = getTypeSize( LISTV_TYPE(i) );
					std::vector<float> values(tsize);
					for(int ii=0; ii < tsize; ii++)
						values[ii] = LISTV_VALUE(i, ii);

					port.setAllSlicesData(&values[0], sizeof(float) * tsize);
				}
				catch(CreationSplice::Exception e)
				{
					printf("copyHoudiniToSpliceV() error: %s\n", e.what());
					return false;
				}

			}	
			else
			{
				//you don't set a name
				return false;
			}
		}
	}

	return true;
}

bool SOP_Splice::copyHoudiniToSpliceP()
{
	for(int i=1; i < LISTP_NUM()+1; i++)
	{
		if(LISTP_ENABLE(i))
		{
			UT_String name;
			LISTP_NAME(i, name);
			if(name.length() > 0)
			{
				try
				{
					UT_String namePort;	addPortString(namePort, name);
					CreationSplice::Port port = m_node->getPort(namePort.buffer());

					const int tsize = getTypeSize( LISTP_TYPE(i) );
					std::vector<float> values(tsize);
					UT_String path;		LISTP_PATH(i, path);
					UT_String param;	LISTP_PARAM(i, param);
					OP_Node* path_node = this->findNode(path);
					if(path_node)
					{
						for(int ii=0; ii < tsize; ii++)
							values[ii] = (float)path_node->evalFloat(param, ii, m_time);
					}

					port.setAllSlicesData(&values[0], sizeof(float) * tsize);
				}
				catch(CreationSplice::Exception e)
				{
					printf("copyHoudiniToSpliceP() error: %s\n", e.what());
					return false;
				}
			}	
			else
			{
				//you don't set a name
				return false;
			}
		}
	}

	return true;
}

bool SOP_Splice::copyHoudiniToSpliceA()
{
	for(int i=1; i < LISTA_NUM()+1; i++)
	{
		if(LISTA_ENABLE(i))
		{
			UT_String name;
			LISTA_NAME(i, name);
			if(name.length() > 0)
			{
				try
				{

				UT_String namePort;	addPortString(namePort, name);
				CreationSplice::Port port = m_node->getPort(namePort.buffer());

				const int tsize = getTypeSize( LISTA_TYPE(i) );
				std::vector<float> values;
				UT_String path;		LISTA_PATH(i, path);
				UT_String attr_name;	LISTA_ATTR(i, attr_name);
				SOP_Node* path_node = 0;
				
				if(path.length() && !isModeOut( LISTA_MODE(i) ))
					path_node = static_cast<SOP_Node*>(this->findNode(path));


			//get GEO
				GU_Detail* my_gdp = 0;
				if(path_node)
				{
				 addExtraInput(path_node, OP_INTEREST_DATA);  
				 my_gdp = const_cast<GU_Detail*>(path_node->getCookedGeo(m_context));
				}
				else
					my_gdp = m_input[ LISTA_INUM(i)-1 ];
		
				if(!my_gdp)
				{
					char err[255];	sprintf(err, "Path %s can't find\n", path.buffer());
					getOut(err, 0);
					return false;
				}



			//check attribute
				const int itype = LISTA_ITYPE(i);
				GEO_AttributeHandle attrH = getAttrType(my_gdp, itype, attr_name);
				if(!attrH.isAttributeValid())
				{
					char err[255];	sprintf(err, "Attribute %s doesn't exist\n", attr_name.buffer());
					getOut(err, 0);
					return false;
				}
				if(tsize > attrH.getFloatCount(true))
				{
					char err[255];	sprintf(err, "Destiny Attribute(%s) has lower size than source\n", attr_name.buffer());
					getOut(err, 0);
					return false;
				}


			//get Num rows
				unsigned int N = 0;
				switch(itype)
				{
				case POINT:	N = (unsigned int)my_gdp->getPointMap().indexSize();		break;
				case VERTEX:	N = (unsigned int)my_gdp->getVertexMap().indexSize();		break;
				case PRIMITIVE:	N = (unsigned int)my_gdp->getPrimitiveMap().indexSize();	break;
				case DETAIL:	N = (unsigned int)my_gdp->getGlobalMap().indexSize();		break;
				}

				values.resize( tsize * N );


			//copy data
				GEO_Primitive* pr;
				GEO_Point* pt;
				int fi = 0;
				switch(itype)
				{
				case POINT:
					GA_FOR_ALL_GPOINTS(my_gdp, pt)
					{
						attrH.setElement(pt);
						for(int ii=0; ii < tsize; ii++)
							values[fi++] = (float)attrH.getF(ii);
					}
					break;

				case VERTEX:
					//Work In progress
					//...
					
					break;
				case PRIMITIVE:
					GA_FOR_ALL_PRIMITIVES(my_gdp, pr)
					{
						attrH.setElement(pr);
						for(int ii=0; ii < tsize; ii++)
							values[fi++] = (float)attrH.getF(ii);
					}
					break;

				case DETAIL:
						attrH.setElement(my_gdp);
						for(int ii=0; ii < tsize; ii++)
							values[fi++] = (float)attrH.getF(ii);
					
					break;
				}

				port.setArrayData(&values[0], sizeof(float) * tsize * N);

				}
				catch(CreationSplice::Exception e)
				{
					printf("copyHoudiniToSpliceA() error: %s\n", e.what());
					return false;
				}

			}	
			else
			{
				//you don't set a name
				return false;
			}
		}
	}

	return true;
}







bool SOP_Splice::createSpliceNode()
{
	deleteSpliceNode();

	UT_String nameNode = m_operatorName;
	nameNode += UT_String("Node");

	m_node = new CreationSplice::Node( nameNode.buffer() );

	createSpliceMembersV();
	createSpliceMembersP();
	createSpliceMembersA();

	//copy code
	UT_String code;
	CODE(code);
	m_node->constructKLOperator(m_operatorName.buffer());
	try
	{
		m_node->setKLOperatorSourceCode(m_operatorName.buffer(), code.buffer());
	}
	catch(CreationSplice::Exception e)
	{
		char err[255];
		sprintf(err, "constructKLOperator error: %s\n", e.what());
		getOut(err, 0);
		return false;
	}

	return true;
}
bool SOP_Splice::deleteSpliceNode()
{
	if(m_node)
		delete m_node;
	m_node = 0;
	return true;
}




bool SOP_Splice::copySpliceToHoudini()
{
	for(int i=1; i < LISTA_NUM()+1; i++)
	{
		if(LISTA_ENABLE(i))
		{
			UT_String name;	LISTA_NAME(i, name);
			if(name.length() > 0 && isModeOut( LISTA_MODE(i) ))
			{

				try
				{
					UT_String namePort;	addPortString(namePort, name);
					CreationSplice::Port port = m_node->getPort(namePort.buffer());

					const int tsize = getTypeSize( LISTA_TYPE(i) );
					const int N = port.getArrayCount();

					std::vector<float> values(tsize * N);
					port.getArrayData(&values[0], sizeof(float) * tsize * N);

					UT_String attr_name;	LISTA_ATTR(i, attr_name);
					const int itype = LISTA_ITYPE(i);
					GEO_AttributeHandle attrH = getAttrType(gdp, itype, attr_name);


					GEO_Primitive* pr;
					GEO_Point* pt;
					int fi = 0;
					switch(itype)
					{
					case POINT:
						GA_FOR_ALL_GPOINTS(gdp, pt)
						{
							attrH.setElement(pt);
							for(int ii=0; ii < tsize; ii++)
								attrH.setF(values[fi++], ii);
						}
						break;

					case VERTEX:
						//Work In progress
						//...
					
						break;
					case PRIMITIVE:
						GA_FOR_ALL_PRIMITIVES(gdp, pr)
						{
							attrH.setElement(pr);
							for(int ii=0; ii < tsize; ii++)
								attrH.setF(values[fi++], ii);
						}
						break;

					case DETAIL:
							attrH.setElement(gdp);
							for(int ii=0; ii < tsize; ii++)
								attrH.setF(values[fi++], ii);
					
						break;
					}
				}
				catch(CreationSplice::Exception e)
				{
					char err[255];
					sprintf(err, "copySpliceToHoudini error: %s\n", e.what());
					getOut(err, 0);
					return false;
				}

			}
		}
	}

	return true;
}




bool SOP_Splice::isMemberExists(const char* name)
{
	for(int i=0;i < (int)m_node->getPortCount(); i++)
		  if(strcmp(name, m_node->getPortName(i).getStringData())==0)
			  return true;
	return false;
}


bool SOP_Splice::isSpliceNodeChanged()
{
//	if(strcmp(m_operatorName.buffer(), m_node->getKLOperatorName(0).getStringData()) != 0)
//		return true;

	UT_String code;	CODE(code);
	CreationCore::Variant klcode = m_node->getKLOperatorSourceCode(m_operatorName.buffer());
	if(strcmp(klcode.getStringData(), code.buffer()) != 0)
		return true;

	for(int i=1; i < LISTA_NUM()+1; i++)
	{
		UT_String name;	LISTA_NAME(i, name);
		if(LISTA_ENABLE(i) && name.length() > 0 && isModeOut( LISTA_MODE(i) ))
		{
			UT_String namePort;	addPortString(namePort, name);
			if(!isMemberExists(namePort.buffer()))
				return true;
		}
	}

	return false;
}




const char* SOP_Splice::getType(int type)
{
	switch(type)
	{
	case SCALAR:	return "Scalar";
	case VEC2:	return "Vec2";
	case VEC3:	return "Vec3";
	case VEC4:	return "Vec4";
	case AVEC4:	return "Vec4[]";
	case MAT44:	return "Mat44";
	}
	return "";

}

CreationSplice::Port_Mode SOP_Splice::getMode(int mode)
{
	switch(mode)
	{
	case IN:	return CreationSplice::Port_Mode_IN;
	case OUT:	return CreationSplice::Port_Mode_OUT;
	case IO:	return CreationSplice::Port_Mode_IO;
	}
	return CreationSplice::Port_Mode_IO;
}

bool SOP_Splice::isModeOut(int mode)
{
	CreationSplice::Port_Mode m = getMode(mode);
	return m==CreationSplice::Port_Mode_OUT || m==CreationSplice::Port_Mode_IO;
}



GEO_AttributeHandle SOP_Splice::getAttrType(GU_Detail* my_gdp, int type, const char* name)
{
	switch(type)
	{
	case POINT:	return my_gdp->findPointAttribute(name);
	case VERTEX:	return my_gdp->findVertexAttribute(name);
	case PRIMITIVE:	return my_gdp->findPrimitiveAttribute(name);
	case DETAIL:	return my_gdp->findGlobalAttribute(name);
	}
	return GEO_AttributeHandle();
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
	case VEC2:	return 2;
	case VEC3:	return 3;
	case VEC4:	return 4;
	case AVEC4:	return 4;
	case MAT44:	return 16;
	}
	return 0;
}


bool SOP_Splice::updateParmsFlags()
{
	bool changed = SOP_Node::updateParmsFlags();

	for(int i=1; i < LISTV_NUM()+1; i++)
	{
		int state = evalIntInst("varv_on", &i, 0, m_time, 1);

		changed |= enableParmInst("varv_name", &i, state, 1);
		changed |= enableParmInst("varv_type", &i, state, 1);
		changed |= enableParmInst("varv_mode", &i, state, 1);

		const int tsize = getTypeSize( LISTV_TYPE(i) );
		for(int ii=0; ii < 4; ii++)
			changed |= enableParmInst("varv_value", &i, state && ii < tsize, 1, ii);
	}

	for(int i=1; i < LISTP_NUM()+1; i++)
	{
		int state = evalIntInst("varp_on", &i, 0, m_time, 1);

		changed |= enableParmInst("varp_name", &i, state, 1);
		changed |= enableParmInst("varp_type", &i, state, 1);
		changed |= enableParmInst("varp_mode", &i, state, 1);
		changed |= enableParmInst("varp_path", &i, state, 1);
		changed |= enableParmInst("varp_param", &i, state, 1);
	}

	for(int i=1; i < LISTA_NUM()+1; i++)
	{
		int state = evalIntInst("vara_on", &i, 0, m_time, 1);

		changed |= enableParmInst("vara_name", &i, state, 1);
		changed |= enableParmInst("vara_type", &i, state, 1);
		changed |= enableParmInst("vara_mode", &i, state, 1);
		changed |= enableParmInst("vara_path", &i, state, 1);
		changed |= enableParmInst("vara_attr", &i, state, 1);
		changed |= enableParmInst("vara_itype", &i, state, 1);

		UT_String path;	LISTA_PATH(i, path);
		changed |= enableParmInst("vara_inum", &i, state && !path.length(), 1);


		if(isModeOut( LISTA_MODE(i) ))	//disable path and inum for mode=="Out" or "I/O"
			changed |= enableParmInst("vara_path", &i, 0, 1);
		
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
	getFullPath(m_operatorPath);
	m_operatorName = getName();

	m_node = 0;
}
SOP_Splice::~SOP_Splice()
{
	deleteSpliceNode();
}


static void dsoExit(void *)
{
	CreationSplice::Finalize();	//Clean Splice
}



//main procedure for library(plugin)
void newSopOperator(OP_OperatorTable *table)
{
	UT_Exit::addExitCallback(dsoExit);

	CreationSplice::Initialize();	//Init Splice


	//setup the callback functions
	CreationSplice::Logging::setLogFunc(myLogFunc);
	CreationSplice::Logging::setLogErrorFunc(myLogErrorFunc);
	CreationSplice::Logging::setCompilerErrorFunc(myCompilerErrorFunc);
	CreationSplice::Logging::setKLReportFunc(myKLReportFunc);
	CreationSplice::Logging::setKLStatusFunc(myKLStatusFunc);


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
static PRM_Name typeVNames[] = {
	PRM_Name("0",	"Scalar"),
	PRM_Name("1",	"Vec2"),
	PRM_Name("2",	"Vec3"),
	PRM_Name("3",	"Vec4"),
	PRM_Name(0)
};

static PRM_Name typeNames[] = {
	PRM_Name("0",	"Scalar"),
	PRM_Name("1",	"Vec2"),
	PRM_Name("2",	"Vec3"),
	PRM_Name("3",	"Vec4"),
	PRM_Name("4",	"Vec4[]"),
	PRM_Name("5",	"Mat44"),
	PRM_Name(0)
};
static PRM_Name modeNames[] = {
	PRM_Name("0",	"In"),
	PRM_Name("1",	"Out"),
	PRM_Name("2",	"I/O"),
	PRM_Name(0)
};
static PRM_Name inputTypeNames[] = {
	PRM_Name("0",	"Point"),
	PRM_Name("1",	"Vertex"),
	PRM_Name("2",	"Primitive"),
	PRM_Name("3",	"Detail"),
	PRM_Name(0)
};

static PRM_ChoiceList   typeVMenu((PRM_ChoiceListType)(PRM_CHOICELIST_SINGLE), &(typeVNames[0]) );
static PRM_ChoiceList   typeMenu((PRM_ChoiceListType)(PRM_CHOICELIST_SINGLE), &(typeNames[0]) );
static PRM_ChoiceList   modeMenu((PRM_ChoiceListType)(PRM_CHOICELIST_SINGLE), &(modeNames[0]) );
static PRM_ChoiceList   inputTypeMenu((PRM_ChoiceListType)(PRM_CHOICELIST_SINGLE), &(inputTypeNames[0]) );



static PRM_Name names_value[] = {
	PRM_Name("varv_on", "var_on"),
	PRM_Name("varv_name", "Name"),
	PRM_Name("varv_type", "Type"),
	PRM_Name("varv_value", "Value"),
};
PRM_Template listValue[] = {
	PRM_Template(PRM_TOGGLE	|PRM_TYPE_JOIN_NEXT|PRM_TYPE_LABEL_NONE,1, &names_value[0], PRMoneDefaults),
	PRM_Template(PRM_STRING	|PRM_TYPE_JOIN_NEXT,			1, &names_value[1], 0),
	PRM_Template(PRM_INT	|PRM_TYPE_JOIN_NEXT|PRM_TYPE_LABEL_NONE,1, &names_value[2], 0, &typeVMenu),
	PRM_Template(PRM_FLT,						4, &names_value[3], 0),
    PRM_Template()
};



static PRM_Name names_paramter[] = {
	PRM_Name("varp_on", "var_on"),
	PRM_Name("varp_name", "Name"),
	PRM_Name("varp_type", "Type"),
	PRM_Name("varp_path", "Path"),
	PRM_Name("varp_param", "Param"),
};
PRM_Template listParameter[] = {
	PRM_Template(PRM_TOGGLE	|PRM_TYPE_JOIN_NEXT|PRM_TYPE_LABEL_NONE,	1, &names_paramter[0], PRMoneDefaults),
	PRM_Template(PRM_STRING	|PRM_TYPE_JOIN_NEXT,				1, &names_paramter[1], 0),
	PRM_Template(PRM_INT	|PRM_TYPE_JOIN_NEXT|PRM_TYPE_LABEL_NONE,	1, &names_paramter[2], 0, &typeMenu),
	PRM_Template(PRM_STRING_E|PRM_TYPE_JOIN_NEXT, PRM_TYPE_DYNAMIC_PATH,	1, &names_paramter[3], 0, 0, 0, 0, &PRM_SpareData::sopPath),
	PRM_Template(PRM_STRING,						1, &names_paramter[4], 0),
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
	PRM_Template(PRM_STRING	|PRM_TYPE_JOIN_NEXT,					1, &names_attribute[1], 0),
	PRM_Template(PRM_INT	|PRM_TYPE_JOIN_NEXT		|PRM_TYPE_LABEL_NONE,	1, &names_attribute[2], 0, &typeMenu),
	PRM_Template(PRM_INT	|PRM_TYPE_JOIN_NEXT		|PRM_TYPE_LABEL_NONE,	1, &names_attribute[3], 0, &modeMenu),
	PRM_Template(PRM_STRING_E|PRM_TYPE_JOIN_NEXT, PRM_TYPE_DYNAMIC_PATH,		1, &names_attribute[4], 0, 0, 0, 0, &PRM_SpareData::sopPath),
	PRM_Template(PRM_INT	|PRM_TYPE_JOIN_NEXT		|PRM_TYPE_LABEL_NONE,	1, &names_attribute[5], PRMoneDefaults, 0, &g_range_one_to_four),
	PRM_Template(PRM_STRING	|PRM_TYPE_JOIN_NEXT,					1, &names_attribute[6], 0),
	PRM_Template(PRM_INT	|PRM_TYPE_LABEL_NONE,					1, &names_attribute[7], 0, &inputTypeMenu),
	PRM_Template()
};




static PRM_Name names[] = {
	PRM_Name("list_value", "Values"),		PRM_Name("sep1", "sep1"),
	PRM_Name("list_parameter", "Parameters"),	PRM_Name("sep2", "sep2"),
	PRM_Name("list_attribute", "Attributes"),	PRM_Name("sep3", "sep3"),
	PRM_Name("sep4", "sep4"),
	PRM_Name("code", "KL Code"),
};



PRM_Template SOP_Splice::myTemplateList[] =
{
	PRM_Template(PRM_MULTITYPE_LIST, listValue, 4, &names[0], 0),		PRM_Template(PRM_SEPARATOR, 1, &names[1]),
	PRM_Template(PRM_MULTITYPE_LIST, listParameter, 5, &names[2], 0),	PRM_Template(PRM_SEPARATOR, 1, &names[3]),
	PRM_Template(PRM_MULTITYPE_LIST, listAttributes, 8, &names[4], 0),	PRM_Template(PRM_SEPARATOR, 1, &names[5]),

	PRM_Template(PRM_SEPARATOR, 1, &names[6]),

	PRM_Template(PRM_STRING, 1, &names[7], 0, 0, 0, 0, &PRM_SpareData::stringEditor),

    PRM_Template()
};
