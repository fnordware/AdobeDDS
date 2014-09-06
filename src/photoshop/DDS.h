
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

#ifndef __DDS_Photoshop_H__
#define __DDS_Photoshop_H__


#include "PIDefines.h"
#include "PIFormat.h"
#include "PIExport.h"
#include "PIUtilities.h"
#include "PIProperties.h"


enum {
	DDS_FMT_DXT1 = 0,
	DDS_FMT_DXT1A,
	DDS_FMT_DXT2,
	DDS_FMT_DXT3,
	DDS_FMT_DXT4,
	DDS_FMT_DXT5,
	DDS_FMT_DXT5A,
	DDS_FMT_DXN,
	DDS_FMT_DXT5_CCxY,
	DDS_FMT_DXT5_xGxR,
	DDS_FMT_DXT5_xGBR,
	DDS_FMT_DXT5_AGBR
};
typedef uint8 DXT_Format;

#define DXT_FMT_DEFAULT DDS_FMT_DXT5

enum {
	DDS_ALPHA_NONE = 0,
	DDS_ALPHA_TRANSPARENCY,
	DDS_ALPHA_CHANNEL
};
typedef uint8 DDS_Alpha;


typedef struct {
	DDS_Alpha	alpha;
	uint8		reserved[31];
	
} DDS_inData;


typedef struct {
	DXT_Format		format;
	DDS_Alpha		alpha;
	Boolean			mipmap;
	uint8			reserved[252];
	
} DDS_outData;


typedef struct Globals
{ // This is our structure that we use to pass globals between routines:

	short				*result;			// Must always be first in Globals.
	FormatRecord		*formatParamBlock;	// Must always be second in Globals.

	//Handle				fileH;				// stores the entire binary file
	
	DDS_inData			in_options;
	DDS_outData			options;
	
} Globals, *GPtr, **GHdl;				// *GPtr = global pointer; **GHdl = global handle



// The routines that are dispatched to from the jump list should all be
// defined as
//		void RoutineName (GPtr globals);
// And this typedef will be used as the type to create a jump list:
typedef void (* FProc)(GPtr globals);


//-------------------------------------------------------------------------------
//	Globals -- definitions and macros
//-------------------------------------------------------------------------------

#define gResult				(*(globals->result))
#define gStuff				(globals->formatParamBlock)

#define gInOptions			(globals->in_options)
#define gOptions			(globals->options)

#define gAliasHandle		(globals->aliasHandle)

//-------------------------------------------------------------------------------
//	Prototypes
//-------------------------------------------------------------------------------


// Everything comes in and out of PluginMain. It must be first routine in source:
DLLExport MACPASCAL void PluginMain (const short selector,
					  	             FormatRecord *formatParamBlock,
						             intptr_t *data,
						             short *result);

// Scripting functions
Boolean ReadScriptParamsOnWrite (GPtr globals);	// Read any scripting params.
OSErr WriteScriptParamsOnWrite (GPtr globals);	// Write any scripting params.

//-------------------------------------------------------------------------------

#endif // __DDS_Photoshop_H__
