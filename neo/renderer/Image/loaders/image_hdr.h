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
#ifndef _IMAGE_HDR_H_
#define _IMAGE_HDR_H_

#include "renderer/Image/Image_loader.h"

/*
=========================================================

HDR LOADING
Load HDR image and convert to a set of float32 RGB triplet.
Tanks Igor Kravtchenko, for the base of the loader
=========================================================
*/
class btHdrImage : public btImageLoader
{
	typedef unsigned char RGBE[4];
public:
	btHdrImage(void);
	virtual ~btHdrImage(void);
	virtual void Load(void** pic, int* width, int* height);
	virtual void Write(const void* data, uint width, uint height, uint bytesPerPixel, bool flipVertical);

private:
	float convertComponent(int expo, int val);
	void workOnRGBE(RGBE *scan, int len, float *cols);
	bool decrunch(RGBE *scanline, int len);
	bool oldDecrunch(RGBE *scanline, int len);
};

#endif // !_IMAGE_PNG_H_

