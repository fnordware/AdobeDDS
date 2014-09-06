
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

#include "DDS_version.h"
#include "DDS_UI.h"

#include "crn_core.h"

//#include "crn_data_stream_serializer.h"
#include "crn_mipmapped_texture.h"

#include <stdio.h>
#include <assert.h>

//using namespace crnlib;

// globals needed by a bunch of Photoshop SDK routines
#ifdef __PIWin__
HINSTANCE hDllInstance = NULL;
#endif

SPBasicSuite * sSPBasic = NULL;
SPPluginRef gPlugInRef = NULL;


static void DoAbout(AboutRecordPtr aboutP)
{
#ifdef __PIMac__
	const char * const plugHndl = "com.fnordware.Photoshop.DDS";
	const void *hwnd = aboutP;	
#else
	const char * const plugHndl = NULL;
	HWND hwnd = (HWND)((PlatformData *)aboutP->platformData)->hwnd;
#endif

	DDS_About(DDS_Build_Complete_Manual, plugHndl, hwnd);
}

#pragma mark-

static void InitGlobals(Ptr globalPtr)
{	
	// create "globals" as a our struct global pointer so that any
	// macros work:
	GPtr globals = (GPtr)globalPtr;
			
	memset(&gInOptions, 0, sizeof(gInOptions));
	memset(&gOptions, 0, sizeof(gOptions));
	
	gInOptions.alpha			= DDS_ALPHA_CHANNEL;
	
	gOptions.format				= DXT_FMT_DEFAULT;
	gOptions.alpha				= DDS_ALPHA_CHANNEL;
	gOptions.mipmap				= FALSE;
}


static Handle myNewHandle(GPtr globals, const int32 inSize)
{
	if(gStuff->handleProcs != NULL && gStuff->handleProcs->numHandleProcs >= 6 && gStuff->handleProcs->newProc != NULL)
	{
		return gStuff->handleProcs->newProc(inSize);
	}
	else
	{
		return PINewHandle(inSize);
	}
}

static Ptr myLockHandle(GPtr globals, Handle h)
{
	if(gStuff->handleProcs != NULL && gStuff->handleProcs->numHandleProcs >= 6 && gStuff->handleProcs->lockProc)
	{
		return gStuff->handleProcs->lockProc(h, TRUE);
	}
	else
	{
		return PILockHandle(h, TRUE);
	}
}

static void myUnlockHandle(GPtr globals, Handle h)
{
	if(gStuff->handleProcs != NULL && gStuff->handleProcs->numHandleProcs >= 6 && gStuff->handleProcs->unlockProc)
	{
		gStuff->handleProcs->unlockProc(h);
	}
	else
	{
		PIUnlockHandle(h);
	}
}

static int32 myGetHandleSize(GPtr globals, Handle h)
{
	if(gStuff->handleProcs != NULL && gStuff->handleProcs->numHandleProcs >= 6 && gStuff->handleProcs->getSizeProc)
	{
		return gStuff->handleProcs->getSizeProc(h);
	}
	else
	{
		return PIGetHandleSize(h);
	}
}

static void mySetHandleSize(GPtr globals, Handle h, const int32 inSize)
{
	if(gStuff->handleProcs != NULL && gStuff->handleProcs->numHandleProcs >= 6 && gStuff->handleProcs->setSizeProc)
	{
		gStuff->handleProcs->setSizeProc(h, inSize);
	}
	else
	{
		PISetHandleSize(h, inSize);
	}
}

static void myDisposeHandle(GPtr globals, Handle h)
{
	if(gStuff->handleProcs != NULL && gStuff->handleProcs->numHandleProcs >= 6 && gStuff->handleProcs->disposeProc != NULL)
	{
		gStuff->handleProcs->disposeProc(h);
	}
	else
	{
		PIDisposeHandle(h);
	}
}


class ps_data_stream : public crnlib::data_stream
{
public:
	ps_data_stream(intptr_t dataFork, attribs_t attribs);
	virtual ~ps_data_stream() {};

	virtual crnlib::uint read(void* pBuf, crnlib::uint len);
	virtual crnlib::uint write(const void* pBuf, crnlib::uint len);
	virtual bool flush() { return true; };
	virtual crnlib::uint64 get_size();
	virtual crnlib::uint64 get_remaining();
	virtual crnlib::uint64 get_ofs();
	virtual bool seek(crnlib::int64 ofs, bool relative);

private:
	intptr_t _dataFork;
};


ps_data_stream::ps_data_stream(intptr_t dataFork, attribs_t attribs) :
	crnlib::data_stream("Photoshop stream", attribs),
	_dataFork(dataFork)
{
	seek(0, false);
}


crnlib::uint
ps_data_stream::read(void* pBuf, crnlib::uint len)
{
#ifdef __PIMac__
	ByteCount count = len;
	
	OSErr result = FSReadFork(_dataFork, fsAtMark, 0, count, pBuf, &count);
	
	return count;
#else
	DWORD count = len, bytes_read = 0;
	
	BOOL result = ReadFile((HANDLE)_dataFork, pBuf, count, &bytes_read, NULL);

	return bytes_read;
#endif
}


crnlib::uint
ps_data_stream::write(const void* pBuf, crnlib::uint len)
{
#ifdef __PIMac__
	ByteCount count = len;

	OSErr result = FSWriteFork(_dataFork, fsAtMark, 0, count, (const void *)pBuf, &count);
	
	return count;
#else
	DWORD count = len, out = 0;
	
	BOOL result = WriteFile((HANDLE)_dataFork, pBuf, count, &out, NULL);
	
	return out;
#endif
}


crnlib::uint64
ps_data_stream::get_size()
{
#ifdef __PIMac__
	SInt64 fork_size = 0;
	
	OSErr result = FSGetForkSize(g_dataFork, &fork_size);
		
	return fork_size;
#else
	return GetFileSize((HANDLE)_dataFork, NULL);
#endif
}


crnlib::uint64
ps_data_stream::get_remaining()
{
	if( is_writable() )
	{
		return crnlib::DATA_STREAM_SIZE_UNKNOWN;
	}
	else
		return (get_size() - get_ofs());
}


crnlib::uint64
ps_data_stream::get_ofs()
{
#ifdef __PIMac__
	SInt64 lpos;

	OSErr result = FSGetForkPosition(_dataFork, &lpos);
	
	return lpos;
#else
	LARGE_INTEGER lpos, zero;

	zero.QuadPart = 0;

	BOOL result = SetFilePointerEx((HANDLE)_dataFork, zero, &lpos, FILE_CURRENT);

	return lpos.QuadPart;
#endif
}


bool
ps_data_stream::seek(crnlib::int64 ofs, bool relative)
{
#ifdef __PIMac__
	const UInt16 positionMode = (relative ? fsFromMark : fsFromStart);
	
	OSErr result = FSSetForkPosition(_dataFork, positionMode, ofs);

	return result;
#else
	LARGE_INTEGER lpos;

	lpos.QuadPart = ofs;

	const DWORD method = ( relative ? FILE_CURRENT : FILE_BEGIN );

#if _MSC_VER < 1300
	DWORD pos = SetFilePointer((HANDLE)_dataFork, lpos.u.LowPart, &lpos.u.HighPart, method);

	BOOL result = (pos != 0xFFFFFFFF || NO_ERROR == GetLastError());
#else
	BOOL result = SetFilePointerEx((HANDLE)_dataFork, lpos, NULL, method);
#endif

	return result;
#endif
}

#pragma mark-


static void DoFilterFile(GPtr globals)
{
	ps_data_stream ps_stream(gStuff->dataFork, crnlib::cDataStreamReadable | crnlib::cDataStreamSeekable);

	crnlib::data_stream_serializer serializer(&ps_stream);

	// copied from mipmapped_texture::read_dds_internal()
	uint8 hdr[4];
	if(!serializer.read(hdr, sizeof(hdr)))
		gResult = formatCannotRead;
	else if(memcmp(hdr, "DDS ", 4) != 0)
		gResult = formatCannotRead;
}


// Additional parameter functions
//   These transfer settings to and from gStuff->revertInfo

static void TwiddleOptions(DDS_inData *options)
{
#ifndef __PIMacPPC__
	// none
#endif
}

static void TwiddleOptions(DDS_outData *options)
{
#ifndef __PIMacPPC__
	// none
#endif
}

template <typename T>
static bool ReadParams(GPtr globals, T *options)
{
	bool found_revert = FALSE;
	
	if( gStuff->revertInfo != NULL )
	{
		if( myGetHandleSize(globals, gStuff->revertInfo) == sizeof(T) )
		{
			T *flat_options = (T *)myLockHandle(globals, gStuff->revertInfo);
			
			// flatten and copy
			TwiddleOptions(flat_options);
			
			memcpy((char*)options, (char*)flat_options, sizeof(T) );
			
			TwiddleOptions(flat_options);
			
			myUnlockHandle(globals, gStuff->revertInfo);
			
			found_revert = TRUE;
		}
	}
	
	return found_revert;
}

template <typename T>
static void WriteParams(GPtr globals, T *options)
{
	T *flat_options = NULL;
	
	if(gStuff->hostNewHdl != NULL) // we have the handle function
	{
		if(gStuff->revertInfo == NULL)
		{
			gStuff->revertInfo = myNewHandle(globals, sizeof(T) );
		}
		else
		{
			if(myGetHandleSize(globals, gStuff->revertInfo) != sizeof(T)  )
				mySetHandleSize(globals, gStuff->revertInfo, sizeof(T) );
		}
		
		flat_options = (T *)myLockHandle(globals, gStuff->revertInfo);
		
		// flatten and copy
		TwiddleOptions(flat_options);
		
		memcpy((char*)flat_options, (char*)options, sizeof(T) );	
		
		TwiddleOptions(flat_options);
			
		
		myUnlockHandle(globals, gStuff->revertInfo);
	}
}


static void HandleError(GPtr globals, const crnlib::mipmapped_texture &dds_file)
{
	const crnlib::dynamic_string &err = dds_file.get_last_error();

	const int size = crnlib::math::minimum<int>(255, err.get_len());

	Str255 p_str;
	p_str[0] = size;
	strncpy((char *)&p_str[1], err.c_str(), size);

	PIReportError(p_str);
	gResult = errReportString;
}


static void DoReadPrepare(GPtr globals)
{
	gStuff->maxData = 0;
}


static void DoReadStart(GPtr globals)
{
	bool reverting = ReadParams(globals, &gInOptions);
	
	if(!reverting && gStuff->hostSig != 'FXTC')
	{
		DDS_InUI_Data params;
		
	#ifdef __PIMac__
		const char * const plugHndl = "com.fnordware.Photoshop.DDS";
		const void *hwnd = globals;	
	#else
		const void * const plugHndl = hDllInstance;
		HWND hwnd = (HWND)((PlatformData *)gStuff->platformData)->hwnd;
	#endif

		// DDS_InUI is responsible for not popping a dialog if the user
		// didn't request it.  It still has to set the read settings from preferences though.
		bool result = DDS_InUI(&params, plugHndl, hwnd);
		
		if(result)
		{
			gInOptions.alpha = params.alpha;
			
			WriteParams(globals, &gInOptions);
		}
		else
			gResult = userCanceledErr;
	}


	if(gResult == noErr)
	{
		ps_data_stream ps_stream(gStuff->dataFork, crnlib::cDataStreamReadable | crnlib::cDataStreamSeekable);

		crnlib::data_stream_serializer serializer(&ps_stream);

		crnlib::mipmapped_texture dds_file;

		if( dds_file.read_dds(serializer) )
		{
			gStuff->imageMode = plugInModeRGBColor;
			gStuff->depth = 8;

			gStuff->imageSize.h = gStuff->imageSize32.h = dds_file.get_width();
			gStuff->imageSize.v = gStuff->imageSize32.v = dds_file.get_height();
			
			gStuff->planes = (dds_file.has_alpha() ? 4 : 3);
			
			if(gInOptions.alpha == DDS_ALPHA_TRANSPARENCY && gStuff->planes == 4)
			{
				gStuff->transparencyPlane = gStuff->planes - 1;
				gStuff->transparencyMatting = 0;
			}

			assert(dds_file.get_num_faces() == 1);
		}
		else
			HandleError(globals, dds_file);
	}
}


static void DoReadContinue(GPtr globals)
{
	ps_data_stream ps_stream(gStuff->dataFork, crnlib::cDataStreamReadable | crnlib::cDataStreamSeekable);

	crnlib::data_stream_serializer serializer(&ps_stream);

	crnlib::mipmapped_texture dds_file;

	if( dds_file.read_dds(serializer) )
	{
		crnlib::image_u8 img(dds_file.get_width(), dds_file.get_height());

		crnlib::image_u8 *img_ptr = dds_file.get_level_image(0, 0, img);

		gStuff->planeBytes = 1;
		gStuff->colBytes = gStuff->planeBytes * 4;
		gStuff->rowBytes = gStuff->colBytes * img_ptr->get_pitch();
		
		gStuff->loPlane = 0;
		gStuff->hiPlane = gStuff->planes - 1;
				
		gStuff->theRect.left = gStuff->theRect32.left = 0;
		gStuff->theRect.right = gStuff->theRect32.right = gStuff->imageSize.h;
		assert(gStuff->imageSize.h == img_ptr->get_width());
		
		gStuff->theRect.top = gStuff->theRect32.top = 0;
		gStuff->theRect.bottom = gStuff->theRect32.bottom = gStuff->imageSize.v;
		assert(gStuff->imageSize.v == img_ptr->get_height());

		gStuff->data = img_ptr->get_pixels();
		
		gResult = AdvanceState();
	}
	else
		HandleError(globals, dds_file);
	
	// very important!
	gStuff->data = NULL;
}


static void DoReadFinish(GPtr globals)
{

}

#pragma mark-

static void DoOptionsPrepare(GPtr globals)
{
	gStuff->maxData = 0;
}


static void DoOptionsStart(GPtr globals)
{
	ReadParams(globals, &gOptions);
	
	if( ReadScriptParamsOnWrite(globals) )
	{
		bool have_transparency = false;
		const char *alpha_name = NULL;
		
		if(gStuff->hostSig == '8BIM')
			have_transparency = (gStuff->documentInfo && gStuff->documentInfo->mergedTransparency);
		else
			have_transparency = (gStuff->planes == 2 || gStuff->planes == 4);

			
		if(gStuff->documentInfo && gStuff->documentInfo->alphaChannels)
			alpha_name = gStuff->documentInfo->alphaChannels->name;
	
	
		DDS_OutUI_Data params;
		
		params.format			= (gOptions.format == DDS_FMT_DXT1 ? DIALOG_FMT_DXT1 :
									gOptions.format == DDS_FMT_DXT2 ? DIALOG_FMT_DXT2 :
									gOptions.format == DDS_FMT_DXT3 ? DIALOG_FMT_DXT3 :
									gOptions.format == DDS_FMT_DXT4 ? DIALOG_FMT_DXT4 :
									gOptions.format == DDS_FMT_DXT5 ? DIALOG_FMT_DXT5 :
									DIALOG_FMT_DXT5);

		params.mipmap			= gOptions.mipmap;
		params.alpha			= (DialogAlpha)gOptions.alpha;
	
	
	#ifdef __PIMac__
		const char * const plugHndl = "com.fnordware.Photoshop.WebP";
		const void *hwnd = globals;	
	#else
		const void * const plugHndl = hDllInstance;
		HWND hwnd = (HWND)((PlatformData *)gStuff->platformData)->hwnd;
	#endif

		bool result = DDS_OutUI(&params, have_transparency, alpha_name, plugHndl, hwnd);
		
		
		if(result)
		{
			gOptions.format			= (params.format == DIALOG_FMT_DXT1 ? DDS_FMT_DXT1 :
										params.format == DIALOG_FMT_DXT2 ? DDS_FMT_DXT2 :
										params.format == DIALOG_FMT_DXT3 ? DDS_FMT_DXT3 :
										params.format == DIALOG_FMT_DXT4 ? DDS_FMT_DXT4 :
										params.format == DIALOG_FMT_DXT5 ? DDS_FMT_DXT5 :
										DDS_FMT_DXT5);

			gOptions.mipmap			= params.mipmap;
			gOptions.alpha			= params.alpha;
			
			WriteParams(globals, &gOptions);
			WriteScriptParamsOnWrite(globals);
		}
		else
			gResult = userCanceledErr;
	}
}


static void DoOptionsContinue(GPtr globals)
{

}


static void DoOptionsFinish(GPtr globals)
{

}

#pragma mark-

static void DoEstimatePrepare(GPtr globals)
{
	gStuff->maxData = 0;
}


static void DoEstimateStart(GPtr globals)
{
	if(gStuff->HostSupports32BitCoordinates && gStuff->imageSize32.h && gStuff->imageSize32.v)
		gStuff->PluginUsing32BitCoordinates = TRUE;
		
	int width = (gStuff->PluginUsing32BitCoordinates ? gStuff->imageSize32.h : gStuff->imageSize.h);
	int height = (gStuff->PluginUsing32BitCoordinates ? gStuff->imageSize32.v : gStuff->imageSize.v);
	
	int64 dataBytes = (int64)width * (int64)height * (int64)gStuff->planes * (int64)(gStuff->depth >> 3);
					  
		
#ifndef MIN
#define MIN(A,B)			( (A) < (B) ? (A) : (B))
#endif
		
	gStuff->minDataBytes = MIN(dataBytes / 2, INT_MAX);
	gStuff->maxDataBytes = MIN(dataBytes, INT_MAX);
	
	gStuff->data = NULL;
}


static void DoEstimateContinue(GPtr globals)
{

}


static void DoEstimateFinish(GPtr globals)
{

}

#pragma mark-

static void DoWritePrepare(GPtr globals)
{
	gStuff->maxData = 0;
}

typedef struct {
	unsigned8	r;
	unsigned8	g;
	unsigned8	b;
	unsigned8	a;
} RGBApixel8;

static void Premultiply(RGBApixel8 *buf, int64 len)
{
	while(len--)
	{
		if(buf->a != 255)
		{	
			const float mult = (float)buf->a / 255.f;
			
			buf->r = ((float)buf->r * mult) + 0.5f;
			buf->g = ((float)buf->g * mult) + 0.5f;
			buf->b = ((float)buf->b * mult) + 0.5f;
		}
		
		buf++;
	}
}


static inline crnlib::pixel_format
Format_PS2Crunch(DXT_Format fmt)
{
	using namespace crnlib;

	return (fmt == DDS_FMT_DXT1 ? PIXEL_FMT_DXT1 :
			fmt == DDS_FMT_DXT1A ? PIXEL_FMT_DXT1A :
			fmt == DDS_FMT_DXT2 ? PIXEL_FMT_DXT2 :
			fmt == DDS_FMT_DXT3 ? PIXEL_FMT_DXT3 :
			fmt == DDS_FMT_DXT4 ? PIXEL_FMT_DXT4 :
			fmt == DDS_FMT_DXT5 ? PIXEL_FMT_DXT5 :
			fmt == DDS_FMT_DXT5A ? PIXEL_FMT_DXT5A :
			fmt == DDS_FMT_DXN ? PIXEL_FMT_DXN :
			fmt == DDS_FMT_DXT5_CCxY ? PIXEL_FMT_DXT5_CCxY :
			fmt == DDS_FMT_DXT5_xGxR ? PIXEL_FMT_DXT5_xGxR :
			fmt == DDS_FMT_DXT5_xGBR ? PIXEL_FMT_DXT5_xGBR :
			fmt == DDS_FMT_DXT5_AGBR ? PIXEL_FMT_DXT5_AGBR :
			PIXEL_FMT_DXT5);
}


static void DoWriteStart(GPtr globals)
{
	ReadParams(globals, &gOptions);
	ReadScriptParamsOnWrite(globals);

	assert(gStuff->imageMode == plugInModeRGBColor);
	assert(gStuff->depth == 8);
	assert(gStuff->planes >= 3);
	
	
	const bool have_transparency = (gStuff->planes >= 4);
	const bool have_alpha_channel = (gStuff->channelPortProcs && gStuff->documentInfo && gStuff->documentInfo->alphaChannels);

	const bool use_transparency = (have_transparency && gOptions.alpha == DDS_ALPHA_TRANSPARENCY);
	const bool use_alpha_channel = (have_alpha_channel && gOptions.alpha == DDS_ALPHA_CHANNEL);
	
	const bool use_alpha = (use_transparency || use_alpha_channel);
	

	const int width = (gStuff->PluginUsing32BitCoordinates ? gStuff->imageSize32.h : gStuff->imageSize.h);
	const int height = (gStuff->PluginUsing32BitCoordinates ? gStuff->imageSize32.v : gStuff->imageSize.v);
	

	crnlib::image_u8 *img = new crnlib::image_u8(width, height);

	if(!use_alpha)
	{
		// OK, these namespaces are getting a little annoying
		const crnlib::pixel_format_helpers::component_flags rgb_only =
							static_cast<crnlib::pixel_format_helpers::component_flags>(
								crnlib::pixel_format_helpers::cCompFlagRValid |
								crnlib::pixel_format_helpers::cCompFlagGValid |
								crnlib::pixel_format_helpers::cCompFlagBValid);

		img->set_comp_flags(rgb_only);
	}


	gStuff->loPlane = 0;
	gStuff->hiPlane = (use_transparency ? 3 : 2);
	gStuff->colBytes = sizeof(unsigned char) * 4;
	gStuff->rowBytes = gStuff->colBytes * img->get_pitch();
	gStuff->planeBytes = sizeof(unsigned char);
	
	gStuff->theRect.left = gStuff->theRect32.left = 0;
	gStuff->theRect.right = gStuff->theRect32.right = width;
	gStuff->theRect.top = gStuff->theRect32.top = 0;
	gStuff->theRect.bottom = gStuff->theRect32.bottom = height;

	gStuff->data = img->get_pixels();

	gResult = AdvanceState();


	if(use_alpha && gOptions.alpha == DDS_ALPHA_CHANNEL && gResult == noErr &&
		gStuff->channelPortProcs && gStuff->documentInfo && gStuff->documentInfo->alphaChannels)
	{
		ReadPixelsProc ReadProc = gStuff->channelPortProcs->readPixelsProc;
		
		ReadChannelDesc *alpha_channel = gStuff->documentInfo->alphaChannels;

		VRect wroteRect;
		VRect writeRect = { 0, 0, height, width };
		PSScaling scaling; scaling.sourceRect = scaling.destinationRect = writeRect;
		PixelMemoryDesc memDesc = { (char *)gStuff->data, gStuff->rowBytes * 8, gStuff->colBytes * 8, 3 * 8, gStuff->depth };					
	
		gResult = ReadProc(alpha_channel->port, &scaling, &writeRect, &memDesc, &wroteRect);
	}

	if(use_transparency && (gStuff->hostSig != 'FXTC') &&
		(gOptions.format == DDS_FMT_DXT2 || gOptions.format == DDS_FMT_DXT4))
	{
		RGBApixel8 *row = (RGBApixel8 *)img->get_pixels();

		for(int y=0; y < height; y++)
		{
			Premultiply(row, width);

			row += img->get_pitch();
		}
	}
	
	if(gResult == noErr)
	{
		crnlib::mipmapped_texture dds_file;

		dds_file.assign(img);
		
		if(gOptions.mipmap)
		{
			crnlib::mipmapped_texture::generate_mipmap_params mipmap_p;

			dds_file.generate_mipmaps(mipmap_p, false);
		}

		if(gOptions.format != DXT_FMT_DEFAULT)
		{
			crnlib::dxt_image::pack_params pack_p;

			dds_file.convert(Format_PS2Crunch(gOptions.format), pack_p);
		}


		const crnlib::data_stream::attribs_t readwrite = crnlib::cDataStreamReadable |
															crnlib::cDataStreamWritable |
															crnlib::cDataStreamSeekable;

		ps_data_stream ps_stream(gStuff->dataFork, readwrite);

		crnlib::data_stream_serializer serializer(&ps_stream);

		if( !dds_file.write_dds(serializer) )
		{
			HandleError(globals, dds_file);
		}
	}
	
	// muy importante
	gStuff->data = NULL;
}


static void DoWriteContinue(GPtr globals)
{

}


static void DoWriteFinish(GPtr globals)
{
	if(gStuff->hostSig != 'FXTC')
		WriteScriptParamsOnWrite(globals);
}


#pragma mark-


DLLExport MACPASCAL void PluginMain(const short selector,
						             FormatRecord *formatParamBlock,
						             intptr_t *data,
						             short *result)
{
	if (selector == formatSelectorAbout)
	{
		sSPBasic = ((AboutRecordPtr)formatParamBlock)->sSPBasic;

	#ifdef __PIWin__
		if(hDllInstance == NULL)
			hDllInstance = GetDLLInstance((SPPluginRef)((AboutRecordPtr)formatParamBlock)->plugInRef);
	#endif

		DoAbout((AboutRecordPtr)formatParamBlock);
	}
	else
	{
		sSPBasic = formatParamBlock->sSPBasic;  //thanks Tom
		
		gPlugInRef = (SPPluginRef)formatParamBlock->plugInRef;
		
	#ifdef __PIWin__
		if(hDllInstance == NULL)
			hDllInstance = GetDLLInstance((SPPluginRef)formatParamBlock->plugInRef);
	#endif

		
	 	static const FProc routineForSelector [] =
		{
			/* formatSelectorAbout  				DoAbout, */
			
			/* formatSelectorReadPrepare */			DoReadPrepare,
			/* formatSelectorReadStart */			DoReadStart,
			/* formatSelectorReadContinue */		DoReadContinue,
			/* formatSelectorReadFinish */			DoReadFinish,
			
			/* formatSelectorOptionsPrepare */		DoOptionsPrepare,
			/* formatSelectorOptionsStart */		DoOptionsStart,
			/* formatSelectorOptionsContinue */		DoOptionsContinue,
			/* formatSelectorOptionsFinish */		DoOptionsFinish,
			
			/* formatSelectorEstimatePrepare */		DoEstimatePrepare,
			/* formatSelectorEstimateStart */		DoEstimateStart,
			/* formatSelectorEstimateContinue */	DoEstimateContinue,
			/* formatSelectorEstimateFinish */		DoEstimateFinish,
			
			/* formatSelectorWritePrepare */		DoWritePrepare,
			/* formatSelectorWriteStart */			DoWriteStart,
			/* formatSelectorWriteContinue */		DoWriteContinue,
			/* formatSelectorWriteFinish */			DoWriteFinish,
			
			/* formatSelectorFilterFile */			DoFilterFile
		};
		
		Ptr globalPtr = NULL;		// Pointer for global structure
		GPtr globals = NULL; 		// actual globals

		
		if(formatParamBlock->handleProcs)
		{
			bool must_init = false;
			
			if(*data == NULL)
			{
				*data = (intptr_t)formatParamBlock->handleProcs->newProc(sizeof(Globals));
				
				must_init = true;
			}

			if(*data != NULL)
			{
				globalPtr = formatParamBlock->handleProcs->lockProc((Handle)*data, TRUE);
				
				if(must_init)
					InitGlobals(globalPtr);
			}
			else
			{
				*result = memFullErr;
				return;
			}

			globals = (GPtr)globalPtr;

			globals->result = result;
			globals->formatParamBlock = formatParamBlock;
		}
		else
		{
			// old lame way
			globalPtr = AllocateGlobals(result,
										 formatParamBlock,
										 formatParamBlock->handleProcs,
										 sizeof(Globals),
						 				 data,
						 				 InitGlobals);

			if(globalPtr == NULL)
			{ // Something bad happened if we couldn't allocate our pointer.
			  // Fortunately, everything's already been cleaned up,
			  // so all we have to do is report an error.
			  
			  *result = memFullErr;
			  return;
			}
			
			// Get our "globals" variable assigned as a Global Pointer struct with the
			// data we've returned:
			globals = (GPtr)globalPtr;
		}


		// Dispatch selector
		if (selector > formatSelectorAbout && selector <= formatSelectorFilterFile)
			(routineForSelector[selector-1])(globals); // dispatch using jump table
		else
			gResult = formatBadParameters;
		
		
		if((Handle)*data != NULL)
		{
			if(formatParamBlock->handleProcs)
			{
				formatParamBlock->handleProcs->unlockProc((Handle)*data);
			}
			else
			{
				PIUnlockHandle((Handle)*data);
			}
		}
		
	
	} // about selector special		
}
