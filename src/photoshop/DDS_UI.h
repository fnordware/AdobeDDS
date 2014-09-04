
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

#ifndef WEBP_UI_H
#define WEBP_UI_H


typedef enum {
	DIALOG_ALPHA_NONE = 0,
	DIALOG_ALPHA_TRANSPARENCY,
	DIALOG_ALPHA_CHANNEL
} DialogAlpha;

typedef struct {
	DialogAlpha		alpha;
	bool			mult;
} DDS_InUI_Data;

typedef struct {
	bool				lossless;
	int					quality;
	DialogAlpha			alpha;
	bool				lossy_alpha;
	bool				alpha_cleanup;
	bool				save_metadata;
} DDS_OutUI_Data;

// WebP UI
//
// return true if user hit OK
// if user hit OK, params block will have been modified
//
// plugHndl is bundle identifier string on Mac, hInstance on win
// mwnd is the main window for Windows

bool
DDS_InUI(
	DDS_InUI_Data		*params,
	bool				have_alpha,
	const void			*plugHndl,
	const void			*mwnd);

bool
DDS_OutUI(
	DDS_OutUI_Data		*params,
	bool				have_transparency,
	const char			*alpha_name,
	const void			*plugHndl,
	const void			*mwnd);

void
DDS_About(
	const char		*plugin_version_string,
	const char		*DDS_version_string,
	const void		*plugHndl,
	const void		*mwnd);
	

// Mac prefs keys
#define DDS_PREFS_ID		"com.fnordware.Photoshop.DDS"
#define DDS_PREFS_ALPHA		"Alpha Mode"
#define DDS_PREFS_MULT		"Mult"
#define DDS_PREFS_AUTO		"Auto"


// Windows registry keys
#define DDS_PREFIX		"Software\\fnord\\DDS"
#define DDS_ALPHA_KEY	"Alpha"
#define DDS_MULT_KEY	"Mult"
#define DDS_AUTO_KEY	"Auto"


#endif // WEBP_UI_H