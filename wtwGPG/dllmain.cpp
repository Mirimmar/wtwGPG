// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"

#include "PluginController.h"

extern "C"
{

	BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
	{
		wtwGPG::PluginController::getInstance().setDllHINSTANCE(hinstDLL);
		return 1;
	}

	WTWGPG_DLL_EXPORT WTWPLUGINFO* WINAPI queryPlugInfo(DWORD /*apiVersion*/, DWORD /*masterVersion*/)
	{
		return const_cast<WTWPLUGINFO*>(wtwGPG::PluginController::getInstance().getPlugInfo());
	}

	WTWGPG_DLL_EXPORT int WINAPI pluginLoad(DWORD callReason, WTWFUNCTIONS* fn)
	{
		return wtwGPG::PluginController::getInstance().onLoad(callReason, fn);
	}

	WTWGPG_DLL_EXPORT int WINAPI pluginUnload(DWORD /*callReason*/)
	{
		return wtwGPG::PluginController::getInstance().onUnload();
	}

} // extern "C"