#include "stdafx.h"

#include "WtwMsg.h"

namespace wtwGPG
{

	void WtwMsg::alloc(const wtwMessageDef& msg)
	{
		if(msg.contactData.id)
			wtwMsg.contactData.id = _wcsdup(msg.contactData.id);

		wtwMsg.contactData.netId = msg.contactData.netId;
		
		if(msg.contactData.netClass)
			wtwMsg.contactData.netClass = _wcsdup(msg.contactData.netClass);

		wtwMsg.msgFlags = msg.msgFlags;
		wtwMsg.msgTime = msg.msgTime;
	}

	void WtwMsg::dealloc()
	{
		if(wtwMsg.msgMessage)
		{
			free((void*)wtwMsg.msgMessage);
			wtwMsg.msgMessage = NULL;
		}

		if(wtwMsg.contactData.id)
		{
			free((void*)wtwMsg.contactData.id);
			wtwMsg.contactData.id = NULL;
		}

		if(wtwMsg.contactData.netClass)
		{
			free((void*)wtwMsg.contactData.netClass);
			wtwMsg.contactData.netClass = NULL;
		}
	}

}; // namespace wtwGPG
