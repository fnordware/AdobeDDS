
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

#ifndef __DDS_Terminology_H__
#define __DDS_Terminology_H__

//-------------------------------------------------------------------------------
//	Options
//-------------------------------------------------------------------------------

//-------------------------------------------------------------------------------
//	Definitions -- Scripting keys
//-------------------------------------------------------------------------------

#define keyDDSformat			'DDSf'
#define keyDDSalpha				'DDSa'
#define keyDDSpremult			'DDSp'
#define keyDDSmipmap			'DDSm'
#define keyDDSfilter			'DDSq'

#define typeDDSformat			'DXTn'

#define formatDXT1				'DXT1'
#define formatDXT1A				'DX1a'
#define formatDXT2				'DXT2'
#define formatDXT3				'DXT3'
#define formatDXT4				'DXT4'
#define formatDXT5				'DXT5'
#define formatDXT5A				'DX5a'
#define format3DC				'D3Dc'
#define formatDXN				'DXNc'
#define formatUncompressed		'DXun'

#define typeAlphaChannel		'alfT'

#define alphaChannelNone		'Nalf'
#define alphaChannelTransparency 'Talf'
#define alphaChannelChannel		'Calf'

#define typeFilter				'filT'

#define filterBox				'Bfil'
#define filterTent				'Tfil'
#define filterLanczos4			'Lfil'
#define filterMitchell			'Mfil'
#define filterKaiser			'Kfil'

#endif // __WebP_Terminology_H__
