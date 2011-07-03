// command_plugin.c	pp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "ExtendedNPObject.h"

#include <npfunctions.h>
#define DllExport   __declspec( dllexport ) 

#include<fstream> 
#include "PluginInstance.h"

NPObject *NPN_RetainObject(NPObject *npobj)
{
	npobj->referenceCount++;
	return npobj;
}

void functionLog(const char * name)
{
	// MessageBoxA(NULL,name, "Function call", MB_OK);
}

void writeALittle()
{
	std::fstream ss("C:/outputc.txt");
	ss << "haha";
	ss.close();
}

NPError NP_LOADDS NPP_New(NPMIMEType pluginType, NPP instance, uint16_t mode, int16_t argc, char* argn[], char* argv[], NPSavedData* saved)
{
	functionLog("NPP_New");

	if(instance == NULL)
	{
		return NPERR_INVALID_INSTANCE_ERROR;
	}
 
    PluginInstance* pPluginObj = new PluginInstance(instance);
 
    if (pPluginObj == NULL)
	{
        return NPERR_OUT_OF_MEMORY_ERROR;
	}
 
	//MessageBoxA(NULL,"1", "NPP NEW", MB_OK);
    instance->pdata = pPluginObj;
    //return pPluginObj->NpapiNew(pluginType, mode, argc, argn, argv, saved);

	return NPERR_NO_ERROR;
}

NPError NPP_Destroy (NPP instance, NPSavedData** save)
{
	functionLog("NPP_Destroy");

    if (instance == NULL)
	{
        return NPERR_INVALID_INSTANCE_ERROR;
	}
    else if (instance->pdata == NULL)
	{
        return NPERR_GENERIC_ERROR;
	}
 
    PluginInstance *inst = (PluginInstance *)instance->pdata;
    //NPError rv = inst->NpapiDestroy(save);
 
    delete inst;
    instance->pdata = NULL;
 
    return NPERR_NO_ERROR;
}

char * description = "ChromeCommander";
NPError NPP_GetValue(NPP instance, NPPVariable variable, void *ret_value)
{
	functionLog("NPP_GetValue");

	NPError rv = NPERR_NO_ERROR;
   switch(variable)
   {
      case NPPVpluginNameString:
		  functionLog("NPPVpluginNameString");
          *((char **)ret_value) = description;
          break;
      case NPPVpluginDescriptionString:    // Plugin description
		  functionLog("NPPVpluginDescriptionString");
          *((char **)ret_value) = description;
          break;
      case NPPVpluginScriptableNPObject:// Scriptable plugin interface (for accessing from javascript)
		  {
			functionLog("NPPVpluginScriptableNPObject");
          //*(NPObject **)ret_value = this-&gt;getScriptableObject();
			//ret_value = (NPObject *)ExtendedNPObject::NewObject(instance);
			*(NPObject **)ret_value = NPN_CreateObject(instance, &ExtendedNPObject::_npclass);
			//NPN_RetainObject((NPObject*)ret_value);
			break;
		  }
      case NPPVpluginWindowBool:
		  functionLog("NPPVpluginWindowBool");
          //*((PRBool *)ret_value) = this-&gt;isWindowed;
          break;
      default:
          rv = NPERR_GENERIC_ERROR;
  }
  return rv;
}


NPError NPP_SetWindow(NPP instance, NPWindow* window)
{
	//MessageBoxA(NULL,"1", "Window has bene set.", MB_OK);
	PluginInstance *pluginInstance = (PluginInstance *)instance->pdata;
	//MessageBoxA(NULL,"1123123123", "Window has bene set.", MB_OK);
	
	//MessageBoxA(NULL,"1123123123", "Window has bene set. sdfdsfsdfdsf", MB_OK);
	//pluginInstance->set_window((HWND*)window->window);
	pluginInstance->set_window((HWND)window->window);

	return NPERR_NO_ERROR;
}

NPError OSCALL NP_GetEntryPoints(NPPluginFuncs* pFuncs)
{
	functionLog("NP_GetEntryPoints");
	pFuncs->newp=&NPP_New;
	pFuncs->destroy=&NPP_Destroy;
	pFuncs->getvalue=&NPP_GetValue;
	pFuncs->setwindow=&NPP_SetWindow;
	//NPP_SetWindowProcPtr setwindow;
	//NPP_DestroyProcPtr

	return NPERR_NO_ERROR;
}

NPError OSCALL NP_Initialize(NPNetscapeFuncs *aNPNFuncs)
{
	functionLog("NP_Initialize");
	return NPERR_NO_ERROR;
}//hikari for guis

NPError OSCALL NP_Shutdown()
{
	functionLog("NP_Shutdown");
	return NPERR_NO_ERROR;
}

/*
    If the class has an allocate function, NPN_CreateObject invokes
    that function, otherwise a NPObject is allocated and
    returned. This method will initialize the referenceCount member of
    the NPObject to 1.
*/
NPObject *NPN_CreateObject(NPP npp, NPClass *aClass)
{
	if(aClass->allocate!=0)
	{
		// Allocate the object:
		functionLog("NPN_CreateObject - allocating");
		NPObject* object = aClass->allocate(npp, aClass);
		return object;
    }
	else
	{
		functionLog("NPN_CreateObject - this object has no allocation function");
		return 0;
	}
}