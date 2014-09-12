
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


#import "DDS_InUI_Controller.h"

@implementation DDS_InUI_Controller

- (id)init:(DialogAlpha)the_alpha
	autoDialog:(BOOL)autoDialog;
{
	self = [super init];
	
	if(!([NSBundle loadNibNamed:@"DDS_InUI" owner:self]))
		return nil;
	
	[alphaMatrix selectCellAtRow:(NSInteger)(the_alpha - 1) column:0];
	
	[autoCheckbox setState:(autoDialog ? NSOnState : NSOffState)];
	
	[theWindow center];
	
	return self;
}

- (IBAction)clickedOK:(id)sender {
	[NSApp stopModal];
}

- (IBAction)clickedCancel:(id)sender {
    [NSApp abortModal];
}

- (IBAction)clickedSetDefaults:(id)sender {
	char alphaMode_char = [self getAlpha];
	CFNumberRef alphaMode = CFNumberCreate(kCFAllocatorDefault, kCFNumberCharType, &alphaMode_char);
	CFBooleanRef autoRef =  (([autoCheckbox state] == NSOnState) ? kCFBooleanTrue : kCFBooleanFalse);
	
	CFPreferencesSetAppValue(CFSTR(DDS_PREFS_ALPHA), alphaMode, CFSTR(DDS_PREFS_ID));
	CFPreferencesSetAppValue(CFSTR(DDS_PREFS_AUTO), autoRef, CFSTR(DDS_PREFS_ID));
	
	CFPreferencesAppSynchronize(CFSTR(DDS_PREFS_ID));
	
	CFRelease(alphaMode);
	CFRelease(autoRef);
}

- (NSWindow *)getWindow {
	return theWindow;
}

- (DialogAlpha)getAlpha {
	if([[alphaMatrix cellAtRow:0 column:0] state] == NSOnState)
		return DIALOG_ALPHA_TRANSPARENCY;
	else
		return DIALOG_ALPHA_CHANNEL;
}

- (BOOL)getAuto {
	return ([autoCheckbox state] == NSOnState);
}

@end
