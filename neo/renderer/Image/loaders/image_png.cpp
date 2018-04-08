/*
===========================================================================

Elbony Pixel Source Code
Copyright (C) 2017-2018 Cristiano Beato

This file is part of the Elbony Pixel Source Code ("Elbony Pixel Source Code").

Elbony Pixel Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Elbony Pixel Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Elbony Pixel Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Elbony Pixel Source Code is also subject to certain additional terms.
You should have received a copy of these additional terms immediately following the terms and conditions of the
GNU General Public License which accompanied the Elbony Pixel Source Code.

===========================================================================
*/
#include "precompiled.h"
#pragma hdrstop

#include "image_png.h"
#include "renderer/tr_local.h"

extern "C"
{
	static int png_compressedSize = 0;
	static void png_Error(png_structp pngPtr, png_const_charp msg)
	{
		common->FatalError("%s", msg);
	}

	static void png_Warning(png_structp pngPtr, png_const_charp msg)
	{
		common->Warning("%s", msg);
	}

	static void	png_ReadData(png_structp pngPtr, png_bytep data, png_size_t length)
	{
		idFile * FhlHandler = static_cast<idFile*>(pngPtr->io_ptr);
		assert(FhlHandler);
		//read the file ptr
		FhlHandler->Read((byte*)data, length);
	}

	static void	png_WriteData(png_structp pngPtr, png_bytep data, png_size_t length)
	{
		idFile * FhlHandler = static_cast<idFile*>(pngPtr->io_ptr);
		assert(FhlHandler);
		//read a file data ptr
		FhlHandler->Write(data, length);
		png_compressedSize += length;
	}

	static void	png_FlushData(png_structp pngPtr)
	{
		idFile * FhlHandler = static_cast<idFile*>(pngPtr->io_ptr);
		assert(FhlHandler);
		//flush data to file
		FhlHandler->Flush();
	}
}

/*
================================================================================================

	btPngImage

================================================================================================
*/
btPngImage::btPngImage(void) : btImageLoader()
{
	// create png_struct with the custom error handlers
	pngReadPtr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, png_Error, png_Warning);
	if (!pngReadPtr)
		common->Error("btPngImage: png_create_read_struct failed\n");

	pngWritePtr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, png_Error, png_Warning);
	if (!pngWritePtr)
		common->Error("btPngImage: png_create_write_struct failed\n");
}

btPngImage::~btPngImage(void)
{
}

void btPngImage::Load(void ** pic, int * width, int * height)
{
	png_uint_32 pngWidth, pngHeight;
	int bitDepth, colorType, interlaceType;

	if (!pic)
		return;	// just getting timestamp

	*pic = NULL;

	//file not open
	if (!m_imageFile)
		return;

	png_set_read_fn(pngReadPtr, m_imageFile, png_ReadData);
	png_set_sig_bytes(pngReadPtr, 0);

	infoPtr = png_create_info_struct(pngReadPtr);
	if (!infoPtr)
		common->Error(" btPngImage::Load(%s) png_create_info_struct failed\n", m_imageName.c_str());

	png_read_info(pngReadPtr, infoPtr);

	png_get_IHDR(pngReadPtr, infoPtr, &pngWidth, &pngHeight, &bitDepth, &colorType, &interlaceType, NULL, NULL);

	// 16 bit -> 8 bit
	png_set_strip_16(pngReadPtr);

	// 1, 2, 4 bit -> 8 bit
	if (bitDepth < 8)
		png_set_packing(pngReadPtr);

	//convert palete to RGB
	if (colorType & PNG_COLOR_MASK_PALETTE)
		png_set_expand(pngReadPtr);

	//convert bw to grayscale
	if (!(colorType & PNG_COLOR_MASK_COLOR))
		png_set_gray_to_rgb(pngReadPtr);

	// set paletted or RGB images with transparency to full alpha so we get RGBA
	if (png_get_valid(pngReadPtr, infoPtr, PNG_INFO_tRNS))
		png_set_tRNS_to_alpha(pngReadPtr);

	// make sure every pixel has an alpha value
	if (!(colorType & PNG_COLOR_MASK_ALPHA))
		png_set_filler(pngReadPtr, 255, PNG_FILLER_AFTER);

	png_read_update_info(pngReadPtr, infoPtr);

	byte* out = (byte*)R_StaticAlloc(pngWidth * pngHeight * 4);
	*pic = out;
	*width = pngWidth;
	*height = pngHeight;

	png_uint_32 rowBytes = png_get_rowbytes(pngReadPtr, infoPtr);
	png_bytep* rowPointers = (png_bytep*)R_StaticAlloc(sizeof(png_bytep) * pngHeight);
	for (png_uint_32 row = 0; row < pngHeight; row++)
	{
		rowPointers[row] = (png_bytep)(out + (row * pngWidth * 4));
	}

	png_read_image(pngReadPtr, rowPointers);
	png_read_end(pngReadPtr, infoPtr);
	png_destroy_read_struct(&pngReadPtr, &infoPtr, NULL);
	R_StaticFree(rowPointers);
}

void btPngImage::Write(const void * data, uint width, uint height, uint bitsPerPixel, bool flipVertical)
{
	int ColorType = 0;
	byte	bytesPerPixel = 0;
	assert(m_imageFile);
	png_set_write_fn(pngWritePtr, m_imageFile, png_WriteData, png_FlushData);

	if (bitsPerPixel == 8)
	{
		ColorType = PNG_COLOR_TYPE_GRAY;
		bytesPerPixel = 1;
	}
	if (bitsPerPixel == 16)
	{
		ColorType = PNG_COLOR_TYPE_GRAY_ALPHA;
		bytesPerPixel = 2;
	}
	if (bitsPerPixel == 24)
	{
		ColorType = PNG_COLOR_TYPE_RGB;
		bytesPerPixel = 3;
	}
	else if (bitsPerPixel == 32)
	{
		ColorType = PNG_COLOR_TYPE_RGB_ALPHA;
		bytesPerPixel = 4;
	}
	else
		common->Error("btPngImage::Write( %s ): bytesPerPixel = %i not supported", m_imageName.c_str(), bitsPerPixel);

	infoPtr = png_create_info_struct(pngWritePtr);
	if (!infoPtr)
		common->Error(" btPngImage::Write(%s) png_create_info_struct failed\n", m_imageName.c_str());

	// setup and write header
	png_set_IHDR(pngWritePtr, infoPtr, width, height, 8, ColorType, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	png_write_info(pngWritePtr, infoPtr);

	png_bytep* rowPointers = (png_bytep*)Mem_Alloc(sizeof(png_bytep) * height, TAG_TEMP);
	if (!flipVertical)
	{
		for (int row = 0, flippedRow = height - 1; row < height; row++, flippedRow--)
		{
			rowPointers[flippedRow] = (png_bytep)((byte*)data + (row * width * bytesPerPixel));
		}
	}
	else
	{
		for (int row = 0; row < height; row++)
		{
			rowPointers[row] = (png_bytep)((byte*)data + (row * width * bytesPerPixel));
		}
	}

	//end writing PNG
	png_write_image(pngWritePtr, rowPointers);
	png_write_end(pngWritePtr, infoPtr);
	png_destroy_write_struct(&pngWritePtr, &infoPtr);
	Mem_Free(rowPointers);
}
