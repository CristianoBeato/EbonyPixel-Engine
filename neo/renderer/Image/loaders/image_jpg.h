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
#ifndef _IMAGE_JPG_H_
#define _IMAGE_JPG_H_

#include "renderer/Image/Image_loader.h"

//Include file for users of JPEG library.
//You will need to have included system headers that define at least
//the typedefs FILE and size_t before you can include jpeglib.h.
//(stdio.h is sufficient on ANSI-conforming systems.)
//You may also wish to include "jerror.h".
#include <jpeglib.h>
#include <jerror.h>


class btJpgImage : public btImageLoader
{
public:
	btJpgImage(void);
	~btJpgImage(void);

	virtual void Load(void** pic, int* width, int* height);
	virtual void Write(const void* data, uint width, uint height, uint bytesPerPixel, bool flipVertical);

private:
	/* 
	This struct contains the JPEG decompression parameters and pointers to
	working space (which is allocated as needed by the JPEG library).
	*/
	struct jpeg_decompress_struct cinfo;
	/*
	We use our private extension JPEG error handler.
	Note that this struct must live as long as the main JPEG parameter
	struct, to avoid dangling-pointer problems.
	*/
	/*
	This struct represents a JPEG error handler.  It is declared separately
	because applications often want to supply a specialized error handler
	(see the second half of this file for an example).  But here we just
	take the easy way out and use the standard error handler, which will
	print a message on stderr and call exit() if compression fails.
	Note that this struct must live as long as the main JPEG parameter
	struct, to avoid dangling-pointer problems.
	*/
	struct jpeg_error_mgr jerr;
};

#endif // !_IMAGE_JPG_H_

