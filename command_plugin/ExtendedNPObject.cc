#include "stdafx.h"
#include <sstream>
#include "ExtendedNPObject.h"

NPClass ExtendedNPObject::_npclass = {
    NP_CLASS_STRUCT_VERSION,
    ExtendedNPObject::Allocate,
    ExtendedNPObject::_Deallocate,
    ExtendedNPObject::_Invalidate,
    ExtendedNPObject::_HasMethod,
    ExtendedNPObject::_Invoke,
    ExtendedNPObject::_InvokeDefault,
    ExtendedNPObject::_HasProperty,
    ExtendedNPObject::_GetProperty,
    ExtendedNPObject::_SetProperty,
    ExtendedNPObject::_RemoveProperty,
    ExtendedNPObject::_Enumerate,
    ExtendedNPObject::_Construct
};

void functionLog2(const char * name)
{
	//MessageBoxA(NULL,name, "Function call", MB_OK);
}



NPObject* ExtendedNPObject::Allocate(NPP npp, NPClass *aClass)
{
	functionLog2(__FUNCTION__);
    return (NPObject *)new ExtendedNPObject(npp);
}

void ExtendedNPObject::_Deallocate(NPObject *npobj)
{
	functionLog2(__FUNCTION__);
}

void ExtendedNPObject::_Invalidate(NPObject *npobj)
{
	functionLog2(__FUNCTION__);
}

/*
typedef struct _NPString {
 const NPUTF8 *utf8characters;
 uint32_t utf8length;
} NPString;
*/
bool ExtendedNPObject::_HasMethod(NPObject *npobj, NPIdentifier name)
{
	functionLog2(__FUNCTION__);
	_NPString * bleh = (_NPString *)(name);
	//MessageBoxA(NULL,bleh->UTF8Characters, "Has Method Function call", MB_OK);
	std::stringstream ss;
	return true;
}

NPObject * outputHandler;

#include "PluginInstance.h"

ConsoleAccess::ConsoleString NPStringToConsoleString(const NPString& npString)
{
	ConsoleAccess::ConsoleString cs;
	for(int i=0; i<npString.UTF8Length; i++)
	{
		cs.push_back(npString.UTF8Characters[i]);
	}
	// also push back a \r\n now:
	//cs.push_back('\r');
	cs.push_back('\n');
		return cs;
}

bool ExtendedNPObject::_Invoke(NPObject *npobj, NPIdentifier name, const NPVariant *args, uint32_t argCount, NPVariant *result)
{
	functionLog2(__FUNCTION__);

	//_NPString * functionName = (_NPString *)(name);
	std::string functionName = ((_NPString *)name)->UTF8Characters;
	if(functionName=="consoleInput")
	{
		ExtendedNPObject * extendedNPObject=(ExtendedNPObject *)(npobj);
		PluginInstance* pluginInstance = (PluginInstance*)(extendedNPObject->instance_->pdata);

		std::stringstream ss;

		ConsoleAccess::ConsoleString inputString = NPStringToConsoleString(NPVARIANT_TO_STRING(args[0]));
		inputString.push_back('\0');
		ss << "Input String: " << (const char*)&inputString[0];
		inputString.pop_back();
		// Get key code:
		/*
		int keyCode = NPVARIANT_TO_INT32(args[0]);

		// We have been passed a keycode to send to windows command console.
		

		ss << keyCode << " is awesome" << extendedNPObject << " : " << pluginInstance;
		*/

		//MessageBoxA(NULL,ss.str().c_str(), "Commander", MB_OK);

		pluginInstance->sendString(inputString);
				//MessageBoxA(NULL, "send key complete", "Commander", MB_OK);
	}
	else if(functionName=="setOutputHandler")
	{
		//functionLog2("setOutputHandler");
		// We have been passed a function to call back when we receive input from windows command console.

		// Call the passed function
		outputHandler = NPVARIANT_TO_OBJECT(args[0]);
		outputHandler->referenceCount++;

		// Start the CONSOLE IO:
		ExtendedNPObject * extendedNPObject=(ExtendedNPObject *)(npobj);
		PluginInstance* pluginInstance = (PluginInstance*)(extendedNPObject->instance_->pdata);
		pluginInstance->startConsole();
	}
	else
	{
		// MessageBoxA(NULL,"Bad", "Commander", MB_OK);
	}

	//argumentObject->_class->invoke(argumentObject, 0, 0, result);
	//BOOLEAN_TO_NPVARIANT(false, *result);
	return true;
}

bool ExtendedNPObject::_InvokeDefault(NPObject *npobj, const NPVariant *args, uint32_t argCount, NPVariant *result)
{
	functionLog2(__FUNCTION__);
	return true;
}

bool ExtendedNPObject::_HasProperty(NPObject * npobj, NPIdentifier name)
{
	functionLog2(__FUNCTION__);
	_NPString * bleh = (_NPString *)(name);
	std::stringstream ss;
	return false;
}

bool ExtendedNPObject::_GetProperty(NPObject *npobj, NPIdentifier name, NPVariant *result)
{
	functionLog2(__FUNCTION__);
	return true;
}

bool ExtendedNPObject::_SetProperty(NPObject *npobj, NPIdentifier name, const NPVariant *value)
{
	functionLog2(__FUNCTION__);
	return true;
}

bool ExtendedNPObject::_RemoveProperty(NPObject *npobj, NPIdentifier name)
{
	functionLog2(__FUNCTION__);
	return true;
}

bool ExtendedNPObject::_Enumerate(NPObject *npobj, NPIdentifier **identifier, uint32_t *count)
{
	functionLog2(__FUNCTION__);
	return true;
}

bool ExtendedNPObject::_Construct(NPObject *npobj, const NPVariant *args, uint32_t argCount, NPVariant *result)
{
	functionLog2(__FUNCTION__);
	return true;
}


/*
    The NPUTF8 returned from NPN_UTF8FromIdentifier SHOULD be freed.
*/
// NPUTF8 *NPN_UTF8FromIdentifier(NPIdentifier identifier);