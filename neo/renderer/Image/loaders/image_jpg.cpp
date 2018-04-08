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

#include "image_jpg.h"
#include "renderer/tr_local.h"

// hooks from jpeg lib to our system
void jpg_Error(const char* fmt, ...)
{
	va_list		argptr;
	char		msg[2048];

	va_start(argptr, fmt);
	vsprintf(msg, fmt, argptr);
	va_end(argptr);

	common->FatalError("%s", msg);
}

void jpg_Printf(const char* fmt, ...)
{
	va_list		argptr;
	char		msg[2048];

	va_start(argptr, fmt);
	vsprintf(msg, fmt, argptr);
	va_end(argptr);

	common->Printf("%s", msg);
}

/*
================================================================================================

	btJpgImage
	JPG LOADING

	Interfaces with the huge libjpeg

================================================================================================
*/
btJpgImage::btJpgImage(void) : btImageLoader()
{
}

btJpgImage::~btJpgImage(void)
{
}

void btJpgImage::Load(void ** pic, int * width, int * height)
{
	JSAMPARRAY buffer;		//Output row buffer
	byte*	fbuffer, *bbuf, *out;
	uint	len;
	int row_stride;		//physical row width in output buffer

	//file not open
	if (!m_imageFile)
		return;


	// JDC: because fill_input_buffer() blindly copies INPUT_BUF_SIZE bytes,
	// we need to make sure the file buffer is padded or it may crash
	if (pic)
		*pic = NULL;		// until proven otherwise

	len = m_imageFile->Length();

	if (!pic)
		return;	// just getting timestamp

	fbuffer = (byte*)Mem_ClearedAlloc(len + 4096, TAG_JPG);
	m_imageFile->Read(fbuffer, len);

	//Step 1: allocate and initialize JPEG decompression object

	//We have to set up the error handler first, in case the initialization
	//step fails.  (Unlikely, but it could happen if you are out of memory.)
	//This routine fills in the contents of struct jerr, and returns jerr's
	//address which we place into the link field in cinfo.
	cinfo.err = jpeg_std_error(&jerr);

	//Now we can initialize the JPEG decompression object.
	jpeg_create_decompress(&cinfo);

	//Step 2: specify data source (eg, a file)

#ifdef USE_NEWER_JPEG
	jpeg_mem_src(&cinfo, fbuffer, len);
#else
	jpeg_stdio_src(&cinfo, fbuffer);
#endif
	//Step 3: read file parameters with jpeg_read_header()

	jpeg_read_header(&cinfo, true);
	//We can ignore the return value from jpeg_read_header since
	//(a) suspension is not possible with the stdio data source, and
	//(b) we passed TRUE to reject a tables-only JPEG file as an error.
	//See libjpeg.doc for more info.

	//Step 4: set parameters for decompression

	//In this example, we don't need to change any of the defaults set by
	//jpeg_read_header(), so we do nothing here.

	//Step 5: Start decompressor

	jpeg_start_decompress(&cinfo);
	//We can ignore the return value since suspension is not possible
	//with the stdio data source.

	//We may need to do some setup of our own at this point before reading
	//the data.  After jpeg_start_decompress() we have the correct scaled
	//output image dimensions available, as well as the output colormap
	//if we asked for color quantization.
	//In this example, we need to make an output work buffer of the right size.

	//JSAMPLEs per row in output buffer
	row_stride = cinfo.output_width * cinfo.output_components;

	if (cinfo.output_components != 4)
		common->DWarning("JPG %s is unsupported color depth (%d)", m_imageName.c_str(), cinfo.output_components);

	out = (byte*)R_StaticAlloc(cinfo.output_width * cinfo.output_height * 4, TAG_IMAGE);

	*pic = out;
	*width = cinfo.output_width;
	*height = cinfo.output_height;

	//Step 6: while (scan lines remain to be read)
	//           jpeg_read_scanlines(...); 

	//Here we use the library's state variable cinfo.output_scanline as the
	//loop counter, so that we don't have to keep track ourselves.
	while (cinfo.output_scanline < cinfo.output_height)
	{
		//jpeg_read_scanlines expects an array of pointers to scanlines.
		//Here the array is only one element long, but you could ask for
		//more than one scanline at a time if that's more convenient.
		bbuf = ((out + (row_stride * cinfo.output_scanline)));
		buffer = &bbuf;
		jpeg_read_scanlines(&cinfo, buffer, 1);
	}

	// clear all the alphas to 255
	{
		int	i, j;
		byte*	buf;

		buf = (byte*)*pic;

		j = cinfo.output_width * cinfo.output_height * 4;
		for (i = 3; i < j; i += 4)
		{
			buf[i] = 255;
		}
	}

	//Step 7: Finish decompression

	jpeg_finish_decompress(&cinfo);
	//We can ignore the return value since suspension is not possible
	//with the stdio data source.

	//Step 8: Release JPEG decompression object

	//This is an important step since it will release a good deal of memory.
	jpeg_destroy_decompress(&cinfo);

	//After finish_decompress, we can close the input file.
	//Here we postpone it until after no more JPEG errors are possible,
	//so as to simplify the setjmp error logic above.  (Actually, I don't
	//think that jpeg_destroy can do an error exit, but why assume anything...)
	Mem_Free(fbuffer);

	//At this point you may want to check to see whether any corrupt-data
	//warnings occurred (test whether jerr.pub.num_warnings is nonzero).
	
	//And we're done!
}

void btJpgImage::Write(const void * data, uint width, uint height, uint bytesPerPixel, bool flipVertical)
{
}
