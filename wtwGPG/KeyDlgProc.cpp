
#include "KeyDlgProc.h"

#include "resource.h"
#include "..\common\StringUtils.h"

namespace wtwGPG
{

	INT_PTR CALLBACK KeyDlgProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
	{
		switch (Msg)
		{
			case WM_INITDIALOG:
			{
				std::wstring privateKey = strUtils::convertEnc(reinterpret_cast<const char*>(lParam));
				if (!privateKey.empty())
					return SetWindowText(GetDlgItem(hDlg, IDC_EDIT_KEY), privateKey.c_str());
				return TRUE;
			}
			case WM_COMMAND:
			{
				switch (LOWORD(wParam))
				{
					case IDOK:
					{
						std::string key = strUtils::getDlgItemTextStr(hDlg, IDC_EDIT_KEY);
						EndDialog(hDlg, reinterpret_cast<INT_PTR>(_strdup(key.c_str())));
						return TRUE;
					}
					case IDCANCEL:
					{
						EndDialog(hDlg, NULL);
						return TRUE;
					}
					default:
					{
						return FALSE;
					}
				}

			} // case WM_COMMAND

		} // switch (Msg)

		return FALSE;
	}

} // namespace wtwGPG