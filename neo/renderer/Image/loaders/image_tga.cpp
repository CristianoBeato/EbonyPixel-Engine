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

#include "image_tga.h"
#include "renderer/tr_local.h"
#include "renderer/Image/Image_process.h"

/*
================================================================================================

	btTgaImage

================================================================================================
*/
btTgaImage::btTgaImage(void) : btImageLoader()
{
}

btTgaImage::~btTgaImage(void)
{
}

/*
=========================================================
TARGA LOADING
=========================================================
*/
void btTgaImage::Load(void ** pic, int * width, int * height)
{
#if 0
	TargaHeader		*targa_header;
	byte			*targa_rgba;
	int				fileSize, numBytes;
	
	//file not open
	if (!m_imageFile)
		return;

	if (!pic)
		return;	// just getting timestamp

	*pic = NULL;
	fileSize = m_imageFile->Length();

	LoadHeader(targa_header);
	if (!targa_header)
	{
		common->Error("btTgaImage::Load(%s) cant load TGA header\n", m_imageName.c_str());
		return;
	}

	if (targa_header->image_type != TARGA_COLOR_DATA && targa_header->image_type != TARGA_RLE_COLOR_DATA 
		&& targa_header->image_type != TARGA_BLACK_WHITE)
		common->Error("btTgaImage::Load( %s ): Only type 2 (RGB), 3 (gray), 10 (RGB) and 11 (gray) TGA images supported\n", m_imageName.c_str());

	if (targa_header->colormap_type != 0)
		common->Error("btTgaImage::Load( %s ): colormaps not supported\n", m_imageName.c_str());

	if (targa_header->pixel_size != 32 && targa_header->pixel_size != 24 &&
		targa_header->pixel_size != 16 && targa_header->pixel_size != 8)
		common->Error("btTgaImage::Load( %s ): Only 8, 16, 24 or 32 bit images supported (no colormaps)\n", m_imageName.c_str());

	if (targa_header->image_type == TARGA_COLOR_DATA || targa_header->image_type == TARGA_BLACK_WHITE)
	{
		numBytes = targa_header->width * targa_header->height * (targa_header->pixel_size >> 3);
		if (numBytes > fileSize - 18 - targa_header->id_length)
			common->Error("btTgaImage::Load( %s ): incomplete file\n", m_imageName.c_str());
	}

	if (width)
		*width = targa_header->width;

	if (height)
		*height = targa_header->height;

	targa_rgba = (byte*)R_StaticAlloc(targa_header->width * targa_header->height * 4, TAG_IMAGE);
	*pic = targa_rgba;

	// skip TARGA image comment
	if (targa_header->id_length != 0)
		m_imageFile->Seek((int)targa_header->id_length, FS_SEEK_CUR);

	if (targa_header->image_type == TARGA_COLOR_DATA || targa_header->image_type == TARGA_BLACK_WHITE)
		loadPixelData(targa_rgba, *targa_header); // Uncompressed RGB or gray scale image
	else if (targa_header->image_type == TARGA_RLE_COLOR_DATA || targa_header->image_type == TARGA_RLE_BLACK_WHITE)
		loadRLEPixelData(targa_rgba, *targa_header);// Runlength encoded images

	// image flp bit
	if ((targa_header->attributes & (1 << 5))) 
	{
		if (width != NULL && height != NULL)
		{
			byte* pixBuff = (byte*)*pic;
			R_VerticalFlip(pixBuff, *width, *height);
		}
	}
#else
	int		columns, rows, numPixels, fileSize, numBytes;
	byte	*pixbuf;
	int		row, column;
	byte	*buf_p;
	byte	*buffer;
	TargaHeader	targa_header;
	byte		*targa_rgba;

	if (!pic)
		return;	// just getting timestamp

	*pic = NULL;

	//
	// load the file
	//
	fileSize = m_imageFile->Length();
	buffer = (byte*)Mem_Alloc(fileSize, TAG_IMAGE);
	m_imageFile->Read(buffer, fileSize);
	if (!buffer)
		return;

	buf_p = buffer;

	targa_header.id_length = *buf_p++;
	targa_header.colormap_type = *buf_p++;
	targa_header.image_type = *buf_p++;

	targa_header.colormap_index = LittleShort(*(short *)buf_p);
	buf_p += 2;
	targa_header.colormap_length = LittleShort(*(short *)buf_p);
	buf_p += 2;
	targa_header.colormap_size = *buf_p++;
	targa_header.x_origin = LittleShort(*(short *)buf_p);
	buf_p += 2;
	targa_header.y_origin = LittleShort(*(short *)buf_p);
	buf_p += 2;
	targa_header.width = LittleShort(*(short *)buf_p);
	buf_p += 2;
	targa_header.height = LittleShort(*(short *)buf_p);
	buf_p += 2;
	targa_header.pixel_size = *buf_p++;
	targa_header.attributes = *buf_p++;

	if (targa_header.image_type != 2 && targa_header.image_type != 10 && targa_header.image_type != 3)
		common->Error("LoadTGA( %s ): Only type 2 (RGB), 3 (gray), and 10 (RGB) TGA images supported\n", m_imageName.c_str());

	if (targa_header.colormap_type != 0)
		common->Error("LoadTGA( %s ): colormaps not supported\n", m_imageName.c_str());

	if ((targa_header.pixel_size != 32 && targa_header.pixel_size != 24) && targa_header.image_type != 3)
		common->Error("LoadTGA( %s ): Only 32 or 24 bit images supported (no colormaps)\n", m_imageName.c_str());

	if (targa_header.image_type == 2 || targa_header.image_type == 3) 
	{
		numBytes = targa_header.width * targa_header.height * (targa_header.pixel_size >> 3);
		if (numBytes > fileSize - 18 - targa_header.id_length)
			common->Error("LoadTGA( %s ): incomplete file\n", m_imageName.c_str());
	}

	columns = targa_header.width;
	rows = targa_header.height;
	numPixels = columns * rows;

	if (width)
		*width = columns;

	if (height)
		*height = rows;

	targa_rgba = (byte *)R_StaticAlloc(numPixels * 4);
	*pic = targa_rgba;

	if (targa_header.id_length != 0)
		buf_p += targa_header.id_length;  // skip TARGA image comment

	if (targa_header.image_type == 2 || targa_header.image_type == 3)
	{
		// Uncompressed RGB or gray scale image
		for (row = rows - 1; row >= 0; row--)
		{
			pixbuf = targa_rgba + row*columns * 4;
			for (column = 0; column < columns; column++)
			{
				unsigned char red, green, blue, alphabyte;
				switch (targa_header.pixel_size)
				{

				case 8:
					blue = *buf_p++;
					green = blue;
					red = blue;
					*pixbuf++ = red;
					*pixbuf++ = green;
					*pixbuf++ = blue;
					*pixbuf++ = 255;
					break;

				case 24:
					blue = *buf_p++;
					green = *buf_p++;
					red = *buf_p++;
					*pixbuf++ = red;
					*pixbuf++ = green;
					*pixbuf++ = blue;
					*pixbuf++ = 255;
					break;
				case 32:
					blue = *buf_p++;
					green = *buf_p++;
					red = *buf_p++;
					alphabyte = *buf_p++;
					*pixbuf++ = red;
					*pixbuf++ = green;
					*pixbuf++ = blue;
					*pixbuf++ = alphabyte;
					break;
				default:
					common->Error("LoadTGA( %s ): illegal pixel_size '%d'\n", m_imageName.c_str(), targa_header.pixel_size);
					break;
				}
			}
		}
	}
	else if (targa_header.image_type == 10) {   // Runlength encoded RGB images
		unsigned char red, green, blue, alphabyte, packetHeader, packetSize, j;

		red = 0;
		green = 0;
		blue = 0;
		alphabyte = 0xff;

		for (row = rows - 1; row >= 0; row--) {
			pixbuf = targa_rgba + row*columns * 4;
			for (column = 0; column < columns; ) {
				packetHeader = *buf_p++;
				packetSize = 1 + (packetHeader & 0x7f);
				if (packetHeader & 0x80) {        // run-length packet
					switch (targa_header.pixel_size) {
					case 24:
						blue = *buf_p++;
						green = *buf_p++;
						red = *buf_p++;
						alphabyte = 255;
						break;
					case 32:
						blue = *buf_p++;
						green = *buf_p++;
						red = *buf_p++;
						alphabyte = *buf_p++;
						break;
					default:
						common->Error("LoadTGA( %s ): illegal pixel_size '%d'\n", m_imageName.c_str(), targa_header.pixel_size);
						break;
					}

					for (j = 0; j < packetSize; j++) {
						*pixbuf++ = red;
						*pixbuf++ = green;
						*pixbuf++ = blue;
						*pixbuf++ = alphabyte;
						column++;
						if (column == columns) { // run spans across rows
							column = 0;
							if (row > 0) {
								row--;
							}
							else {
								goto breakOut;
							}
							pixbuf = targa_rgba + row*columns * 4;
						}
					}
				}
				else {                            // non run-length packet
					for (j = 0; j < packetSize; j++) {
						switch (targa_header.pixel_size) {
						case 24:
							blue = *buf_p++;
							green = *buf_p++;
							red = *buf_p++;
							*pixbuf++ = red;
							*pixbuf++ = green;
							*pixbuf++ = blue;
							*pixbuf++ = 255;
							break;
						case 32:
							blue = *buf_p++;
							green = *buf_p++;
							red = *buf_p++;
							alphabyte = *buf_p++;
							*pixbuf++ = red;
							*pixbuf++ = green;
							*pixbuf++ = blue;
							*pixbuf++ = alphabyte;
							break;
						default:
							common->Error("LoadTGA( %s ): illegal pixel_size '%d'\n", m_imageName.c_str(), targa_header.pixel_size);
							break;
						}
						column++;
						if (column == columns) { // pixel packet run spans across rows
							column = 0;
							if (row > 0) {
								row--;
							}
							else {
								goto breakOut;
							}
							pixbuf = targa_rgba + row*columns * 4;
						}
					}
				}
			}
		breakOut:;
		}
	}

	// image flp bit
	if ((targa_header.attributes & (1 << 5)))
		R_VerticalFlip((byte*)*pic, *width, *height);

	Mem_Free(buffer);
#endif
}

#define FLIP_BIT 1 << 5
void btTgaImage::Write(const void * data, uint width, uint height, uint bitsPerPixel, bool flipVertical)
{
#if 0
	uint8 attrb = 0;
	uint8 imageType = 0;
	if (bitsPerPixel == 8)
		imageType = TARGA_BLACK_WHITE;
	if (bitsPerPixel == 24)
		imageType = TARGA_COLOR_DATA;
	else if (bitsPerPixel == 32)
		imageType = TARGA_COLOR_DATA;
	else
		common->Error("btTgaImage::Write( %s ): bytesPerPixel = %i not supported", m_imageName.c_str(), bitsPerPixel);

	if (flipVertical)
		attrb = FLIP_BIT;

	TargaHeader header = 
	{
		0,					//id_length 
		0,					//colormap_type
		imageType,			//image_type
		0,					//colormap_index
		0,					//colormap_length
		0,					//colormap_size
		0,					//x_origin
		0,					//y_origin
		width,				//size
		height,				//size
		bitsPerPixel,		//pixel_size
		attrb				//attributes
	};

	byte* pixelBuff = (byte*)data;

	m_imageFile->Write(&header, sizeof(TargaHeader));

	// swap rgb to bgr, and write the color chanels
	uint chanels = width * height * 4;
	for (uint i = 0; i < chanels; i += 4)
	{
		m_imageFile->WriteUnsignedChar(pixelBuff[i + 2]);// blue
		m_imageFile->WriteUnsignedChar(pixelBuff[i + 1]);// green
		m_imageFile->WriteUnsignedChar(pixelBuff[i + 0]);// red
		m_imageFile->WriteUnsignedChar(pixelBuff[i + 3]);// alpha
	}
#else
	byte	*buffer;
	int		i;
	int		bufferSize = width*height * 4 + 18; //the pixel color data plus header
	int     imgStart = 18;

	buffer = (byte *)Mem_Alloc(bufferSize, TAG_IMAGE);
	memset(buffer, 0, 18);
	buffer[2] = 2;		// uncompressed type
	buffer[12] = width & 255;
	buffer[13] = width >> 8;
	buffer[14] = height & 255;
	buffer[15] = height >> 8;
	buffer[16] = 32;	// pixel size
	if (!flipVertical) {
		buffer[17] = (1 << 5);	// flip bit, for normal top to bottom raster order
	}

	// swap rgb to bgr
	for (i = imgStart; i<bufferSize; i += 4) {
		buffer[i] = ((byte*)data)[i - imgStart + 2];		// blue
		buffer[i + 1] = ((byte*)data)[i - imgStart + 1];		// green
		buffer[i + 2] = ((byte*)data)[i - imgStart + 0];		// red
		buffer[i + 3] = ((byte*)data)[i - imgStart + 3];		// alpha
	}

	//fileSystem->WriteFile(filename, buffer, bufferSize);
	m_imageFile->Write(buffer, bufferSize);

	Mem_Free(buffer);
#endif
}

void btTgaImage::LoadHeader(TargaHeader * &hdr)
{
	hdr = new(TAG_IMAGE) TargaHeader;
	//Read TGA Header
#if 0
	m_imageFile->Read(hdr, sizeof(TargaHeader));
#else
	m_imageFile->ReadUnsignedChar(hdr->id_length);
	m_imageFile->ReadUnsignedChar(hdr->colormap_type);
	m_imageFile->ReadUnsignedChar(hdr->image_type);

	m_imageFile->ReadShort(hdr->colormap_index);
	m_imageFile->ReadShort(hdr->colormap_length);
	m_imageFile->ReadUnsignedChar(hdr->colormap_size);

	m_imageFile->ReadShort(hdr->x_origin);
	m_imageFile->ReadShort(hdr->y_origin);
	m_imageFile->ReadUnsignedShort(hdr->width);
	m_imageFile->ReadUnsignedShort(hdr->height);
	m_imageFile->ReadUnsignedChar(hdr->pixel_size);
	m_imageFile->ReadUnsignedChar(hdr->attributes);
#endif
}

void btTgaImage::loadPixelData(byte * targa_rgba, const TargaHeader header)
{
	int				row, rows, column, columns;
	uint8 red, green, blue, alphabyte;
	byte *pixbuf;

	columns = header.width;
	rows = header.height;

	for (row = rows - 1; row >= 0; row--)
	{
		pixbuf = targa_rgba + row * columns * 4;
		switch (header.pixel_size)
		{
		case 8:	//gray scale image
			m_imageFile->ReadUnsignedChar(blue);
			green = red = blue;
			alphabyte = 255;
			break;
		case 16://gray scale whit alpha image
			m_imageFile->ReadUnsignedChar(blue);
			m_imageFile->ReadUnsignedChar(alphabyte);
			green = red = blue;
			break;
			//BGR image
		case 24:
			m_imageFile->ReadUnsignedChar(blue);
			m_imageFile->ReadUnsignedChar(green);
			m_imageFile->ReadUnsignedChar(red);
			alphabyte = 255;
			break;

			//BGRA image
		case 32:
			m_imageFile->ReadUnsignedChar(blue);
			m_imageFile->ReadUnsignedChar(green);
			m_imageFile->ReadUnsignedChar(red);
			m_imageFile->ReadUnsignedChar(alphabyte);
			break;

		default:
			common->Error("LoadTGA( %s ): illegal pixel_size '%d'\n", m_imageName.c_str(), header.pixel_size);
			break;
		}

		*pixbuf++ = red;
		*pixbuf++ = green;
		*pixbuf++ = blue;
		*pixbuf++ = alphabyte;
	}
}

void btTgaImage::loadRLEPixelData(byte * targa_rgba, const TargaHeader header)
{
	int				row, rows, column, columns;
	uint8 red, green, blue, alphabyte, packetHeader, packetSize, j;
	byte *pixbuf;

	columns = header.width;
	rows = header.height;

	red = 0;
	green = 0;
	blue = 0;
	alphabyte = 0xff;

	for (row = rows - 1; row >= 0; row--)
	{
		pixbuf = targa_rgba + row * columns * 4;
		for (column = 0; column < columns; )
		{
			m_imageFile->ReadUnsignedChar(packetHeader);
			packetSize = 1 + (packetHeader & 0x7f);
			if (packetHeader & 0x80)           // run-length packet
			{
				switch (header.pixel_size)
				{
				case 8://gray scale 
					m_imageFile->ReadUnsignedChar(blue);
					red = green = blue;
					alphabyte = 255;
					break;
				case 16://gary scale whit alpha 
					m_imageFile->ReadUnsignedChar(blue);
					m_imageFile->ReadUnsignedChar(alphabyte);
					red = green = blue;
					alphabyte = 255;
					break;
				case 24://color 
					m_imageFile->ReadUnsignedChar(blue);
					m_imageFile->ReadUnsignedChar(green);
					m_imageFile->ReadUnsignedChar(red);
					alphabyte = 255;
					break;
				case 32: //color whit alpha 
					m_imageFile->ReadUnsignedChar(blue);
					m_imageFile->ReadUnsignedChar(green);
					m_imageFile->ReadUnsignedChar(red);
					m_imageFile->ReadUnsignedChar(alphabyte);
					break;
				default:
					common->Error("LoadTGA( %s ): illegal pixel_size '%d'\n", m_imageName.c_str(), header.pixel_size);
					break;
				}

				for (j = 0; j < packetSize; j++)
				{
					*pixbuf++ = red;
					*pixbuf++ = green;
					*pixbuf++ = blue;
					*pixbuf++ = alphabyte;
					column++;
					if (column == columns)    // run spans across rows
					{
						column = 0;
						if (row > 0)
							row--;
						else
							goto breakOut;
						pixbuf = targa_rgba + row * columns * 4;
					}
				}
			}
			else                              // non run-length packet
			{
				for (j = 0; j < packetSize; j++)
				{
					switch (header.pixel_size)
					{
					case 8://Gray scale 
						m_imageFile->ReadUnsignedChar(blue);
						red = green = blue;
					case 16: //Gray scale  whit alpha 
						m_imageFile->ReadUnsignedChar(blue);
						m_imageFile->ReadUnsignedChar(alphabyte);
						red = green = blue;
					case 24: //color 
						m_imageFile->ReadUnsignedChar(blue);
						m_imageFile->ReadUnsignedChar(green);
						m_imageFile->ReadUnsignedChar(red);
						alphabyte = 255;
						break;
					case 32://color whit alpha
						m_imageFile->ReadUnsignedChar(blue);
						m_imageFile->ReadUnsignedChar(green);
						m_imageFile->ReadUnsignedChar(red);
						m_imageFile->ReadUnsignedChar(alphabyte);
						break;
					default:
						common->Error("LoadTGA( %s ): illegal pixel_size '%d'\n", m_imageName.c_str(), header.pixel_size);
						break;
					}
					*pixbuf++ = red;
					*pixbuf++ = green;
					*pixbuf++ = blue;
					*pixbuf++ = alphabyte;
					
					
					column++;
					if (column == columns)    // pixel packet run spans across rows
					{
						column = 0;
						if (row > 0)
							row--;
						else
							goto breakOut;
						pixbuf = targa_rgba + row * columns * 4;
					}
				}
			}
		}
	breakOut:
		continue;
	}
}