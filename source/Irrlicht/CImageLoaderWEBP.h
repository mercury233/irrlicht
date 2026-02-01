// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __C_IMAGE_LOADER_WEBP_H_INCLUDED__
#define __C_IMAGE_LOADER_WEBP_H_INCLUDED__

#include "IrrCompileConfig.h"

#ifdef _IRR_COMPILE_WITH_WEBP_LOADER_

#include "IImageLoader.h"

namespace irr
{
namespace video
{

//! Surface Loader for WebP files
class CImageLoaderWEBP : public IImageLoader
{
public:

	//! constructor
	CImageLoaderWEBP();

	//! returns true if the file maybe is able to be loaded by this class
	//! based on the file extension (e.g. ".webp")
	virtual bool isALoadableFileExtension(const io::path& filename) const;

	//! returns true if the file maybe is able to be loaded by this class
	virtual bool isALoadableFileFormat(io::IReadFile* file) const;

	//! creates a surface from the file
	virtual IImage* loadImage(io::IReadFile* file) const;
};

} // end namespace video
} // end namespace irr

#endif // _IRR_COMPILE_WITH_WEBP_LOADER_
#endif // __C_IMAGE_LOADER_WEBP_H_INCLUDED__
