// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "CImageWriterWEBP.h"

#ifdef _IRR_COMPILE_WITH_WEBP_WRITER_

#ifdef _IRR_COMPILE_WITH_LIBWEBP_
#include "libwebp/src/webp/encode.h"
#endif // _IRR_COMPILE_WITH_LIBWEBP_

#include "CColorConverter.h"
#include "IWriteFile.h"
#include "CImage.h"
#include "irrString.h"
#include "os.h"

namespace irr
{
namespace video
{

IImageWriter* createImageWriterWEBP()
{
	return new CImageWriterWEBP;
}

CImageWriterWEBP::CImageWriterWEBP()
{
#ifdef _DEBUG
	setDebugName("CImageWriterWEBP");
#endif
}

bool CImageWriterWEBP::isAWriteableFileExtension(const io::path& filename) const
{
#ifdef _IRR_COMPILE_WITH_LIBWEBP_
	return core::hasFileExtension(filename, "webp");
#else
	return false;
#endif
}

bool CImageWriterWEBP::writeImage(io::IWriteFile* file, IImage* image, u32 param) const
{
#ifdef _IRR_COMPILE_WITH_LIBWEBP_
	if (!file || !image)
		return false;

	const core::dimension2d<u32> dim = image->getDimension();
	const u32 width = dim.Width;
	const u32 height = dim.Height;

	// Quality factor: param 0-100, default 80 if not specified
	float quality = (param == 0) ? 80.0f : (float)param;
	if (quality > 100.0f) quality = 100.0f;

	// Determine if the image has alpha channel
	bool hasAlpha = false;
	switch (image->getColorFormat())
	{
	case ECF_A8R8G8B8:
	case ECF_A1R5G5B5:
		hasAlpha = true;
		break;
	default:
		break;
	}

	// Prepare image data in the format WebP expects
	u8* tmpImage = 0;
	u32 stride = 0;

	if (hasAlpha)
	{
		// Prepare alpha image bytes for WebP.
		// - little-endian Irrlicht ECF_A8R8G8B8 bytes are BGRA -> use WebPEncodeBGRA
		// - big-endian Irrlicht ECF_A8R8G8B8 bytes are ARGB -> reorder to RGBA and use WebPEncodeRGBA
		stride = width * 4;
		tmpImage = new (std::nothrow) u8[height * stride];
		if (!tmpImage)
		{
			os::Printer::log("WebPWriter: failed to allocate memory for image conversion", file->getFileName(), ELL_ERROR);
			return false;
		}

		u8* data = (u8*)image->lock();
		const u32 pitch = image->getPitch();
		u8* src = data;
		u8* dst = tmpImage;
		for (u32 y = 0; y < height; ++y)
		{
			switch (image->getColorFormat())
			{
			case ECF_A8R8G8B8:
				CColorConverter::convert_A8R8G8B8toA8R8G8B8(src, width, dst);
				break;
			case ECF_A1R5G5B5:
				CColorConverter::convert_A1R5G5B5toA8R8G8B8(src, width, dst);
				break;
			default:
				os::Printer::log("WebPWriter: unsupported color format for alpha", file->getFileName(), ELL_ERROR);
				image->unlock();
				delete[] tmpImage;
				return false;
			}
			src += pitch;
			dst += stride;
		}
		image->unlock();

#ifdef __BIG_ENDIAN__
		// Convert in-place from ARGB to RGBA for libwebp.
		for (u32 y = 0; y < height; ++y)
		{
			u8* p = tmpImage + y * stride;
			for (u32 x = 0; x < width; ++x)
			{
				const u8 a = p[0];
				const u8 r = p[1];
				const u8 g = p[2];
				const u8 b = p[3];
				p[0] = r;
				p[1] = g;
				p[2] = b;
				p[3] = a;
				p += 4;
			}
		}
#endif
	}
	else
	{
		// Prepare RGB image bytes for WebP.
		stride = width * 3;
		tmpImage = new (std::nothrow) u8[height * stride];
		if (!tmpImage)
		{
			os::Printer::log("WebPWriter: failed to allocate memory for image conversion", file->getFileName(), ELL_ERROR);
			return false;
		}

		u8* data = (u8*)image->lock();
		const u32 pitch = image->getPitch();
		u8* src = data;
		u8* dst = tmpImage;
		for (u32 y = 0; y < height; ++y)
		{
			switch (image->getColorFormat())
			{
			case ECF_R8G8B8:
				CColorConverter::convert_R8G8B8toR8G8B8(src, width, dst);
				break;
			case ECF_R5G6B5:
				CColorConverter::convert_R5G6B5toR8G8B8(src, width, dst);
				break;
			case ECF_A8R8G8B8:
				CColorConverter::convert_A8R8G8B8toR8G8B8(src, width, dst);
				break;
			case ECF_A1R5G5B5:
				CColorConverter::convert_A1R5G5B5toR8G8B8(src, width, dst);
				break;
			default:
				os::Printer::log("WebPWriter: unsupported color format", file->getFileName(), ELL_ERROR);
				image->unlock();
				delete[] tmpImage;
				return false;
			}
			src += pitch;
			dst += stride;
		}
		image->unlock();
	}

	// Encode the image
	u8* output = 0;
	size_t outputSize = 0;

	if (hasAlpha)
	{
		// Encode with correct channel order.
#ifdef __BIG_ENDIAN__
		outputSize = WebPEncodeRGBA(tmpImage, width, height, stride, quality, &output);
#else
		outputSize = WebPEncodeBGRA(tmpImage, width, height, stride, quality, &output);
#endif
	}
	else
	{
		// Encode RGB
		outputSize = WebPEncodeRGB(tmpImage, width, height, stride, quality, &output);
	}

	delete[] tmpImage;

	if (outputSize == 0 || !output)
	{
		os::Printer::log("WebPWriter: failed to encode WebP image", file->getFileName(), ELL_ERROR);
		return false;
	}

	// Write to file
	const s32 written = file->write(output, (u32)outputSize);
	WebPFree(output);

	if (written != (s32)outputSize)
	{
		os::Printer::log("WebPWriter: failed to write WebP data to file", file->getFileName(), ELL_ERROR);
		return false;
	}

	return true;
#else
	return false;
#endif // _IRR_COMPILE_WITH_LIBWEBP_
}

} // namespace video
} // namespace irr

#endif // _IRR_COMPILE_WITH_WEBP_WRITER_
