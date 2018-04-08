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
#ifndef _IMAGE_LOADER_H_
#define _IMAGE_LOADER_H_

#include "Image.h"

enum imageDataType_t
{
	DT_UNSIGNED_BYTE,
	DT_UNSIGNED_INT,
	DT_FLOAT32
};

class btImageLoader
{
public:
	btImageLoader(void);
	virtual ~btImageLoader(void) { Close(); };
	virtual bool Open(idStr filename, const char* basePath, ID_TIME_T* timestamp, bool toWrite = false); //open the image to read and write
	virtual void Close(void);
	virtual void Load(void** pic, int* width, int* height) = 0;
	virtual void Write(const void* data, uint width, uint height, uint bitsPerPixel, bool flipVertical) = 0;

protected:
	idStr		m_imageName;
	idFile*		m_imageFile;
	ID_TIME_T	m_imagetimestamp;
private:
};

// data is RGBA
// data is in top-to-bottom raster order unless flipVertical is set
void R_WriteImage(idStr filename, const byte* data, int width, int height, imageFileType_t imgType, bool flipVertical = false, const char* basePath = "fs_savepath");
void R_LoadImage(const char* name, byte** pic, int* width, int* height, imageDataType_t *type, ID_TIME_T* timestamp, bool makePowerOf2);
// pic is in top to bottom raster format
bool R_LoadCubeImages(const char* cname, cubeFiles_t extensions, byte* pic[6], int* size, imageDataType_t *type, ID_TIME_T* timestamp);

#endif // !_IMAGE_LOADER_H_
