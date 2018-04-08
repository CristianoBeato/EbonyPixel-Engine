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

#include "image_hdr.h"
#include "renderer/tr_local.h"

/*
================================================================================================
	btHdrImage
	Load Radiance .hdr files
	whit 32f point color depth
================================================================================================
*/
#define R			0	// Red Color index
#define G			1	// Green Color index
#define B			2	// Blue Color Index
#define E			3	// Luminance exponent

#define  MINELEN	8				// minimum scanline length for encoding
#define  MAXELEN	0x7fff			// maximum scanline length for encoding

btHdrImage::btHdrImage(void) : btImageLoader()
{
}

btHdrImage::~btHdrImage(void)
{
}

void btHdrImage::Load(void ** pic, int * width, int * height)
{
	int i;
	// each pixel takes 3 float32, each component can be of any value...
	float *cols;

	//file not open
	if (!m_imageFile)
		return;

	m_imageFile->Seek(1, FS_SEEK_CUR);

	char cmd[200];
	i = 0;
	char c = 0, oldc;
	while (true)
	{
		oldc = c;
		m_imageFile->ReadChar(c);
		if (c == 0xa && oldc == 0xa)
			break;
		cmd[i++] = c;
	}

	char reso[200];
	i = 0;
	while (true)
	{
		m_imageFile->ReadChar(c);
		reso[i++] = c;
		if (c == 0xa)
			break;
	}

	int w, h;
	if (!sscanf(reso, "-Y %ld +X %ld", &h, &w))
		return;

	*width = w;
	*height = h;

	cols = new(TAG_IMAGE) float[w * h * 3];

	RGBE *scanline = new(TAG_IMAGE) RGBE[w];
	if (!scanline)
		return;

	// convert image 
	for (int y = h - 1; y >= 0; y--)
	{
		if (decrunch(scanline, w) == false)
			break;
		workOnRGBE(scanline, w, cols);
		cols += w * 3;
	}

	delete[] scanline;
}

void btHdrImage::Write(const void * data, uint width, uint height, uint bytesPerPixel, bool flipVertical)
{
}

float btHdrImage::convertComponent(int expo, int val)
{
	float v = val / 256.0f;
	float d = (float)pow(2, expo);
	return v * d;
}

void btHdrImage::workOnRGBE(RGBE *scan, int len, float *cols)
{
	while (len-- > 0)
	{
		int expo = scan[0][E] - 128;
		cols[0] = convertComponent(expo, scan[0][R]);
		cols[1] = convertComponent(expo, scan[0][G]);
		cols[2] = convertComponent(expo, scan[0][B]);
		cols += 3;
		scan++;
	}
}

bool btHdrImage::decrunch(RGBE *scanline, int len)
{
	uint8  i;
	unsigned char val;

	if (len < MINELEN || len > MAXELEN)
		return oldDecrunch(scanline, len);

	m_imageFile->ReadUnsignedChar(i);
	if (i != 2)
	{
		m_imageFile->Seek(-1, FS_SEEK_CUR);
		return oldDecrunch(scanline, len);
	}

	m_imageFile->ReadUnsignedChar(scanline[0][G]);
	m_imageFile->ReadUnsignedChar(scanline[0][B]);
	m_imageFile->ReadUnsignedChar(i);

	if (scanline[0][G] != 2 || scanline[0][B] & 128)
	{
		scanline[0][R] = 2;
		scanline[0][E] = i;
		return oldDecrunch(scanline + 1, len - 1);
	}

	// read each component
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < len; )
		{
			unsigned char code;
			m_imageFile->ReadUnsignedChar(code);
			if (code > 128)
			{ // run
				code &= 127;
				m_imageFile->ReadUnsignedChar(val);
				while (code--)
				{
					scanline[j++][i] = val;
				}
			}
			else
			{	// non-run
				while (code--)
				{
					m_imageFile->ReadUnsignedChar(val);
					scanline[j++][i] = val;
				}
			}
		}
	}

	//check EOF
	return (m_imageFile->Tell() == m_imageFile->Length()) ? false : true;
}

bool btHdrImage::oldDecrunch(RGBE *scanline, int len)
{
	int i;
	int rshift = 0;

	while (len > 0)
	{
		m_imageFile->ReadUnsignedChar(scanline[0][R]);// = fgetc(file);
		m_imageFile->ReadUnsignedChar(scanline[0][G]);// = fgetc(file);
		m_imageFile->ReadUnsignedChar(scanline[0][B]);// = fgetc(file);
		m_imageFile->ReadUnsignedChar(scanline[0][E]);// = fgetc(file);
		if (m_imageFile->Tell() == m_imageFile->Length())
			return false;

		if (scanline[0][R] == 1 && scanline[0][G] == 1 && scanline[0][B] == 1)
		{
			for (i = scanline[0][E] << rshift; i > 0; i--)
			{
				memcpy(&scanline[0][0], &scanline[-1][0], 4);
				scanline++;
				len--;
			}
			rshift += 8;
		}
		else
		{
			scanline++;
			len--;
			rshift = 0;
		}
	}
	return true;
}
