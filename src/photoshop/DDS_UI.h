
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

#ifndef DDS_UI_H
#define DDS_UI_H

typedef enum {
	DIALOG_FMT_DXT1,
	DIALOG_FMT_DXT1A,
	DIALOG_FMT_DXT2,
	DIALOG_FMT_DXT3,
	DIALOG_FMT_DXT4,
	DIALOG_FMT_DXT5,
	DIALOG_FMT_DXT5A,
	DIALOG_FMT_3DC,
	DIALOG_FMT_DXN,
	DIALOG_FMT_UNCOMPRESSED
} DialogFormat;

typedef enum {
	DIALOG_ALPHA_NONE,
	DIALOG_ALPHA_TRANSPARENCY,
	DIALOG_ALPHA_CHANNEL
} DialogAlpha;

typedef enum {
	DIALOG_FILTER_BOX,
	DIALOG_FILTER_TENT,
	DIALOG_FILTER_LANCZOS4,
	DIALOG_FILTER_MITCHELL,
	DIALOG_FILTER_KAISER
} Dialog_Filter;

typedef struct {
	DialogAlpha		alpha;
} DDS_InUI_Data;

typedef struct {
	DialogFormat		format;
	DialogAlpha			alpha;
	bool				premultiply;
	bool				mipmap;
	Dialog_Filter		filter;
} DDS_OutUI_Data;

// DDS UI
//
// return true if user hit OK
// if user hit OK, params block will have been modified
//
// plugHndl is bundle identifier string on Mac, hInstance on win
// mwnd is the main window for Windows

bool
DDS_InUI(
	DDS_InUI_Data		*params,
	bool				has_alpha,
	const void			*plugHndl,
	const void			*mwnd);

bool
DDS_OutUI(
	DDS_OutUI_Data		*params,
	bool				have_transparency,
	const char			*alpha_name,
	bool				ae_ui,
	const void			*plugHndl,
	const void			*mwnd);

void
DDS_About(
	const char		*plugin_version_string,
	const void		*plugHndl,
	const void		*mwnd);
	

// Mac prefs keys
#define DDS_PREFS_ID		"com.fnordware.Photoshop.DDS"
#define DDS_PREFS_ALPHA		"Alpha Mode"
#define DDS_PREFS_AUTO		"Auto"


// Windows registry keys
#define DDS_PREFIX		"Software\\fnord\\DDS"
#define DDS_ALPHA_KEY	"Alpha"
#define DDS_AUTO_KEY	"Auto"


#endif // DDS_UI_H