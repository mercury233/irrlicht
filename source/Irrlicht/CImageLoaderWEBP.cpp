// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "CImageLoaderWEBP.h"

#ifdef _IRR_COMPILE_WITH_WEBP_LOADER_

#ifdef _IRR_COMPILE_WITH_LIBWEBP_
#include "libwebp/src/webp/decode.h"
#endif // _IRR_COMPILE_WITH_LIBWEBP_

#include "CImage.h"
#include "CReadFile.h"
#include "os.h"

namespace irr
{
namespace video
{

//! constructor
CImageLoaderWEBP::CImageLoaderWEBP()
{
#ifdef _DEBUG
	setDebugName("CImageLoaderWEBP");
#endif
}

//! returns true if the file maybe is able to be loaded by this class
//! based on the file extension (e.g. ".webp")
bool CImageLoaderWEBP::isALoadableFileExtension(const io::path& filename) const
{
#ifdef _IRR_COMPILE_WITH_LIBWEBP_
	return core::hasFileExtension(filename, "webp");
#else
	return false;
#endif // _IRR_COMPILE_WITH_LIBWEBP_
}

//! returns true if the file maybe is able to be loaded by this class
bool CImageLoaderWEBP::isALoadableFileFormat(io::IReadFile* file) const
{
#ifdef _IRR_COMPILE_WITH_LIBWEBP_
	if (!(file && file->seek(0)))
		return false;

	// WebP file signature: "RIFF" followed by file size and "WEBP"
	u8 header[12];
	if (file->read(header, 12) != 12)
		return false;

	// Reset file position
	file->seek(0);

	// Check for RIFF header and WEBP signature
	return (header[0] == 'R' && header[1] == 'I' && header[2] == 'F' && header[3] == 'F' &&
	        header[8] == 'W' && header[9] == 'E' && header[10] == 'B' && header[11] == 'P');
#else
	return false;
#endif // _IRR_COMPILE_WITH_LIBWEBP_
}

//! creates a surface from the file
IImage* CImageLoaderWEBP::loadImage(io::IReadFile* file) const
{
#ifdef _IRR_COMPILE_WITH_LIBWEBP_
	if (!(file && file->seek(0)))
		return 0;

	// Get file size
	const u32 fileSize = file->getSize();
	if (fileSize == 0 || fileSize > 0x7FFFFFFF) // Max ~2GB safety check
	{
		os::Printer::log("LOAD WEBP: file size is invalid", file->getFileName(), ELL_ERROR);
		return 0;
	}

	// Allocate buffer and read file content
	u8* buffer = new (std::nothrow) u8[fileSize];
	if (!buffer)
	{
		os::Printer::log("LOAD WEBP: failed to allocate memory for file buffer", file->getFileName(), ELL_ERROR);
		return 0;
	}

	if (file->read(buffer, fileSize) != fileSize)
	{
		os::Printer::log("LOAD WEBP: failed to read file", file->getFileName(), ELL_ERROR);
		delete[] buffer;
		return 0;
	}

	// Get image dimensions
	int width = 0, height = 0;
	if (!WebPGetInfo(buffer, fileSize, &width, &height))
	{
		os::Printer::log("LOAD WEBP: invalid WebP file header", file->getFileName(), ELL_ERROR);
		delete[] buffer;
		return 0;
	}

	// Decode WebP into Irrlicht's ECF_A8R8G8B8 byte layout.
	// - little-endian: u32 AARRGGBB stored as bytes B,G,R,A  (BGRA)
	// - big-endian:    u32 AARRGGBB stored as bytes A,R,G,B  (ARGB)
#ifdef __BIG_ENDIAN__
	u8* decodedData = WebPDecodeARGB(buffer, fileSize, &width, &height);
#else
	u8* decodedData = WebPDecodeBGRA(buffer, fileSize, &width, &height);
#endif
	delete[] buffer;

	if (!decodedData)
	{
		os::Printer::log("LOAD WEBP: failed to decode WebP image", file->getFileName(), ELL_ERROR);
		return 0;
	}

	// Create the image
	video::IImage* image = new CImage(ECF_A8R8G8B8, core::dimension2d<u32>(width, height));
	if (!image)
	{
		os::Printer::log("LOAD WEBP: failed to create image", file->getFileName(), ELL_ERROR);
		WebPFree(decodedData);
		return 0;
	}

	// Copy decoded data to image, respecting pitch
	u8* imageData = (u8*)image->lock();
	if (!imageData)
	{
		os::Printer::log("LOAD WEBP: failed to lock image data", file->getFileName(), ELL_ERROR);
		delete image;
		WebPFree(decodedData);
		return 0;
	}

	const u32 pitch = image->getPitch();
	const u32 lineWidth = width * 4;
	const u8* src = decodedData;
	u8* dst = imageData;

	// Copy line by line to handle potential pitch differences
	for (int y = 0; y < height; ++y)
	{
		memcpy(dst, src, lineWidth);
		src += lineWidth;
		dst += pitch;
	}

	image->unlock();

	// Free WebP decoded data
	WebPFree(decodedData);

	return image;
#else
	return 0;
#endif // _IRR_COMPILE_WITH_LIBWEBP_
}

IImageLoader* createImageLoaderWEBP()
{
	return new CImageLoaderWEBP();
}

} // end namespace video
} // end namespace irr

#endif // _IRR_COMPILE_WITH_WEBP_LOADER_
