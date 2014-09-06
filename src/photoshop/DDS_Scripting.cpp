
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

#include "PIDefines.h"
#include "DDS.h"

#include "DDS_Terminology.h"


static DXT_Format KeyToFormat(OSType key)
{
	return	(key == formatDXT1)		?	DDS_FMT_DXT1 :
			(key == formatDXT2)		?	DDS_FMT_DXT2 :
			(key == formatDXT3)		?	DDS_FMT_DXT3 :
			(key == formatDXT4)		?	DDS_FMT_DXT4 :
			(key == formatDXT5)		?	DDS_FMT_DXT5 :
			DDS_FMT_DXT5;
}

static DDS_Alpha KeyToAlpha(OSType key)
{
	return	(key == alphaChannelNone)			? DDS_ALPHA_NONE :
			(key == alphaChannelTransparency)	? DDS_ALPHA_TRANSPARENCY :
			(key == alphaChannelChannel)		? DDS_ALPHA_CHANNEL :
			DDS_ALPHA_TRANSPARENCY;
}

Boolean ReadScriptParamsOnWrite(GPtr globals)
{
	PIReadDescriptor			token = NULL;
	DescriptorKeyID				key = 0;
	DescriptorTypeID			type = 0;
	OSType						shape = 0, create = 0;
	DescriptorKeyIDArray		array = { NULLID };
	int32						flags = 0;
	OSErr						gotErr = noErr, stickyError = noErr;
	Boolean						returnValue = true;
	int32						storeValue;
	DescriptorEnumID			ostypeStoreValue;
	Boolean						boolStoreValue;
	
	if (DescriptorAvailable(NULL))
	{
		token = OpenReader(array);
		if (token)
		{
			while (PIGetKey(token, &key, &type, &flags))
			{
				switch (key)
				{
					case keyDDSformat:
							PIGetEnum(token, &ostypeStoreValue);
							gOptions.format = KeyToFormat(ostypeStoreValue);
							break;
					
					case keyDDSmipmap:
							PIGetBool(token, &boolStoreValue);
							gOptions.mipmap = boolStoreValue;
							break;

					case keyDDSalpha:
							PIGetEnum(token, &ostypeStoreValue);
							gOptions.alpha = KeyToAlpha(ostypeStoreValue);
							break;
				}
			}

			stickyError = CloseReader(&token); // closes & disposes.
				
			if (stickyError)
			{
				if (stickyError == errMissingParameter) // missedParamErr == -1715
					;
					/* (descriptorKeyIDArray != NULL)
					   missing parameter somewhere.  Walk IDarray to find which one. */
				else
					gResult = stickyError;
			}
		}
		
		returnValue = PlayDialog();
		// return TRUE if want to show our Dialog
	}
	
	return returnValue;
}

		
static OSType FormatToKey(DXT_Format fmt)
{
	return	(fmt == DDS_FMT_DXT1)	? formatDXT1 :
			(fmt == DDS_FMT_DXT2)	? formatDXT2 :
			(fmt == DDS_FMT_DXT3)	? formatDXT3 :
			(fmt == DDS_FMT_DXT4)	? formatDXT4 :
			(fmt == DDS_FMT_DXT5)	? formatDXT5 :
			formatDXT5;
}

static OSType AlphaToKey(DDS_Alpha alpha)
{
	return	(alpha == DDS_ALPHA_NONE)			? alphaChannelNone :
			(alpha == DDS_ALPHA_TRANSPARENCY)	? alphaChannelTransparency :
			(alpha == DDS_ALPHA_CHANNEL)		? alphaChannelChannel :
			alphaChannelTransparency;
}

OSErr WriteScriptParamsOnWrite(GPtr globals)
{
	PIWriteDescriptor			token = nil;
	OSErr						gotErr = noErr;
			
	if (DescriptorAvailable(NULL))
	{
		token = OpenWriter();
		if (token)
		{
			// write keys here
			PIPutEnum(token, keyDDSformat, typeDDSformat, FormatToKey(gOptions.format));

			PIPutBool(token, keyDDSmipmap, gOptions.mipmap);
				
			PIPutEnum(token, keyDDSalpha, typeAlphaChannel, AlphaToKey(gOptions.alpha));
			
			gotErr = CloseWriter(&token); /* closes and sets dialog optional */
			/* done.  Now pass handle on to Photoshop */
		}
	}
	return gotErr;
}


