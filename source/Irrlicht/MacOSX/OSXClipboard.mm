// Copyright (C) 2005-2006 Etienne Petitjean
// Copyright (C) 2007-2012 Christian Stehno
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in Irrlicht.h

#include "irrString.h"
#include "OSXClipboard.h"
#import <Cocoa/Cocoa.h>

void OSXCopyToClipboard(const char *text)
{
	if (text != NULL && strlen(text) > 0)
	{
		NSString* str = [NSString stringWithUTF8String:text];
		if (str)
		{
			NSPasteboard* board = [NSPasteboard generalPasteboard];
			[board declareTypes:@[NSPasteboardTypeString] owner:NSApp];
			[board setString:str forType:NSPasteboardTypeString];
		}
	}
}

const char* OSXCopyFromClipboard()
{
	static irr::core::stringc buffer;
	NSPasteboard* board = [NSPasteboard generalPasteboard];
	NSString* str = [board stringForType:NSPasteboardTypeString];
	if (str == nil)
	{
		buffer = "";
		return buffer.c_str();
	}
	const char* utf8 = [str UTF8String];
	buffer = utf8 ? utf8 : "";
	return buffer.c_str();
}

