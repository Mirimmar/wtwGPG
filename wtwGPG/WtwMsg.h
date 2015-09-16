#pragma once

#include "stdafx.h"

namespace wtwGPG
{

	class WtwMsg
	{
	public:

		WtwMsg() 
		{
		}

		WtwMsg(const WtwMsg& msg)
		{
			alloc(msg.wtwMsg);
		}

		WtwMsg(const wtwMessageDef& msg)
		{
			alloc(msg);
		}

		~WtwMsg()
		{
			dealloc();
		}

		WtwMsg& operator=(const WtwMsg& msg)
		{
			if (this == &msg)
				return *this;

			dealloc();
			alloc(msg.wtwMsg);
			return *this;
		}

		inline wtwMessageDef& get()
		{
			return wtwMsg;
		}

		void setMessage(const wchar_t* msg)
		{
			if (msg)
				wtwMsg.msgMessage = _wcsdup(msg);
		}

	private:

		wtwMessageDef wtwMsg;

		void alloc(const wtwMessageDef& msg);
		void dealloc();

	}; // class WtwMsg

}; // namespace wtwGPG
