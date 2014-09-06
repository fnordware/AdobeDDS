
///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2014, Brendan Bolles
// 
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
// *	   Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
// *	   Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
///////////////////////////////////////////////////////////////////////////

// ------------------------------------------------------------------------
//
// DDS Photoshop plug-in
//
// by Brendan Bolles <brendan@fnordware.com>
//
// ------------------------------------------------------------------------

#include "DDS.h"

#include "DDS_UI.h"
#include "DDS_version.h"

#include <Windows.h>
#include <commctrl.h>

#include <stdio.h>

extern HINSTANCE hDllInstance;

enum {
	OUT_noUI = -1,
	OUT_OK = IDOK,
	OUT_Cancel = IDCANCEL,
	OUT_Format_Menu,
	OUT_Mipmap_Radio,
	OUT_Alpha_Radio_None,
	OUT_Alpha_Radio_Transparency,
	OUT_Alpha_Radio_Channel,
};

// sensible Win macros
#define GET_ITEM(ITEM)	GetDlgItem(hwndDlg, (ITEM))

#define SET_CHECK(ITEM, VAL)	SendMessage(GET_ITEM(ITEM), BM_SETCHECK, (WPARAM)(VAL), (LPARAM)0)
#define GET_CHECK(ITEM)			SendMessage(GET_ITEM(ITEM), BM_GETCHECK, (WPARAM)0, (LPARAM)0)

#define ENABLE_ITEM(ITEM, ENABLE)	EnableWindow(GetDlgItem(hwndDlg, (ITEM)), (ENABLE));



static DialogFormat			g_format = DIALOG_FMT_DXT5;
static DialogAlpha			g_alpha = DIALOG_ALPHA_NONE;
static bool					g_mipmap = FALSE;

static bool					g_have_transparency = false;
static const char			*g_alpha_name = NULL;

static WORD	g_item_clicked = 0;



static BOOL CALLBACK DialogProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam) 
{ 
    BOOL fError; 
 
    switch(message) 
    { 
		case WM_INITDIALOG:
			do{
				// set up the menu
				// I prefer to do it programatically to insure that the compression types match the index
				const char *opts[] = {	"DXT1",
										"DXT1A",
										"DXT2",
										"DXT3",
										"DXT4",
										"DXT5",
										"DXT5A",
										"3Dc",
										"DXN" };

				HWND menu = GetDlgItem(hwndDlg, OUT_Format_Menu);

				for(int i=DIALOG_FMT_DXT1; i <= DIALOG_FMT_DXN; i++)
				{
					SendMessage(menu,( UINT)CB_ADDSTRING, (WPARAM)wParam, (LPARAM)(LPCTSTR)opts[i] );
					SendMessage( menu,(UINT)CB_SETITEMDATA, (WPARAM)i, (LPARAM)(DWORD)i); // this is the compresion number

					if(i == g_format)
						SendMessage( menu, CB_SETCURSEL, (WPARAM)i, (LPARAM)0);
				}
			}while(0);

			SendMessage(GetDlgItem(hwndDlg, OUT_Mipmap_Radio), BM_SETCHECK, (WPARAM)g_mipmap, (LPARAM)0);

			if(!g_have_transparency)
			{
				ENABLE_ITEM(OUT_Alpha_Radio_Transparency, FALSE);

				if(g_alpha == DIALOG_ALPHA_TRANSPARENCY)
				{
					g_alpha = (g_alpha_name != NULL ? DIALOG_ALPHA_CHANNEL : DIALOG_ALPHA_NONE);
				}
			}

			if(g_alpha_name == NULL)
			{
				ENABLE_ITEM(OUT_Alpha_Radio_Channel, FALSE);

				if(g_alpha == DIALOG_ALPHA_CHANNEL)
				{
					g_alpha = (g_have_transparency ? DIALOG_ALPHA_TRANSPARENCY : DIALOG_ALPHA_NONE);
				}
			}
			else
			{
				SetDlgItemText(hwndDlg, OUT_Alpha_Radio_Channel, g_alpha_name);
			}

			SET_CHECK( (g_alpha == DIALOG_ALPHA_NONE ? OUT_Alpha_Radio_None :
						g_alpha == DIALOG_ALPHA_TRANSPARENCY ? OUT_Alpha_Radio_Transparency :
						g_alpha == DIALOG_ALPHA_CHANNEL ? OUT_Alpha_Radio_Channel :
						OUT_Alpha_Radio_None), TRUE);

			return TRUE;
 
		case WM_NOTIFY:
			return FALSE;

        case WM_COMMAND: 
			g_item_clicked = LOWORD(wParam);

            switch(g_item_clicked)
            { 
                case OUT_OK: 
				case OUT_Cancel:  // do the same thing, but g_item_clicked will be different
					HWND menu = GetDlgItem(hwndDlg, OUT_Format_Menu);

					// get the channel index associated with the selected menu item
					LRESULT cur_sel = SendMessage(menu,(UINT)CB_GETCURSEL, (WPARAM)0, (LPARAM)0);

					g_format = (DialogFormat)SendMessage(menu,(UINT)CB_GETITEMDATA, (WPARAM)cur_sel, (LPARAM)0);

					g_mipmap = GET_CHECK(OUT_Mipmap_Radio);

					g_alpha =	GET_CHECK(OUT_Alpha_Radio_None) ? DIALOG_ALPHA_NONE :
								GET_CHECK(OUT_Alpha_Radio_Transparency) ? DIALOG_ALPHA_TRANSPARENCY :
								GET_CHECK(OUT_Alpha_Radio_Channel) ? DIALOG_ALPHA_CHANNEL :
								DIALOG_ALPHA_TRANSPARENCY;

					EndDialog(hwndDlg, 0);
					return TRUE;
            } 
    } 
    return FALSE; 
} 

bool
DDS_OutUI(
	DDS_OutUI_Data		*params,
	bool				have_transparency,
	const char			*alpha_name,
	const void			*plugHndl,
	const void			*mwnd)
{
	g_format		= params->format;
	g_alpha			= params->alpha;
	g_mipmap		= params->mipmap;
	
	g_have_transparency = have_transparency;
	g_alpha_name = alpha_name;

	int status = DialogBox(hDllInstance, (LPSTR)"OUT_DIALOG", (HWND)mwnd, (DLGPROC)DialogProc);


	if(g_item_clicked == OUT_OK)
	{
		params->format			= g_format;
		params->alpha			= g_alpha;
		params->mipmap			= g_mipmap;

		return true;
	}
	else
		return false;
}


enum {
	ABOUT_noUI = -1,
	ABOUT_OK = IDOK,
	ABOUT_Plugin_Version_String = 4,
};

static const char *g_plugin_version_string = NULL;

static BOOL CALLBACK AboutProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam) 
{ 
    BOOL fError; 
 
    switch(message) 
    { 
		case WM_INITDIALOG:
				SetDlgItemText(hwndDlg, ABOUT_Plugin_Version_String, g_plugin_version_string);

			return TRUE;
 
		case WM_NOTIFY:
			return FALSE;

        case WM_COMMAND: 
            switch(LOWORD(wParam))
            { 
                case OUT_OK: 
				case OUT_Cancel:
					EndDialog(hwndDlg, 0);
					return TRUE;
            } 
    } 
    return FALSE; 
} 

void
DDS_About(
	const char		*plugin_version_string,
	const void		*plugHndl,
	const void		*mwnd)
{
	g_plugin_version_string = plugin_version_string;

	int status = DialogBox(hDllInstance, (LPSTR)"ABOUT_DIALOG", (HWND)mwnd, (DLGPROC)AboutProc);
}

