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
#ifndef _IMAGE_TGA_H_
#define _IMAGE_TGA_H_

#include "renderer/Image/Image_loader.h"

/*
========================================================================
TGA files are used for 24/32 bit images
========================================================================
*/
class btTgaImage : public btImageLoader
{
	typedef struct _TargaHeader
	{
		uint8 			id_length;
		uint8			colormap_type;
		uint8			image_type;

		int16			colormap_index;
		int16			colormap_length;
		uint8			colormap_size;

		int16			x_origin;
		int16			y_origin;
		uint16			width;
		uint16			height;
		
		uint8			pixel_size;
		uint8			attributes;
	} TargaHeader;

	const static uint8	TARGA_NOIMAGEDATA = 0; //INVALID IMAGE
	const static uint8	TARGA_COLOR_MAP = 1;  //NOT SUPORTED
	const static uint8	TARGA_COLOR_DATA = 2;
	const static uint8	TARGA_BLACK_WHITE = 3;
	const static uint8	TARGA_RLE_COLOR_MAP = 9; //NOT SUPORTED
	const static uint8	TARGA_RLE_COLOR_DATA = 10;
	const static uint8	TARGA_RLE_BLACK_WHITE = 11;
public:
	btTgaImage(void);
	virtual ~btTgaImage(void);

	virtual void Load(void** pic, int* width, int* height);
	virtual void Write(const void* data, uint width, uint height, uint bitsPerPixel, bool flipVertical);

private:
	void	LoadHeader(TargaHeader* &hdr);
	void	loadPixelData(byte * targa_rgba, const TargaHeader	header);
	void	loadRLEPixelData(byte * targa_rgba, const TargaHeader	header);
};

#endif // !_IMAGE_TGA_H_

