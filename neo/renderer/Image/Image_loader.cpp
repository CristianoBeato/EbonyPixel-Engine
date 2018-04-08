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


#include "Image_loader.h"
#include "Image_process.h"
#include "renderer/tr_local.h"

//the formats importers
#include "loaders/image_tga.h"
#include "loaders/image_png.h"
#include "loaders/image_jpg.h"
//#include "image_hdr.h"

/*
================================================================================================

	btImageLoader

================================================================================================
*/
btImageLoader::btImageLoader(void)
{
	m_imageFile = NULL;
	m_imagetimestamp = 0;
}

bool btImageLoader::Open(idStr filename, const char * basePath, ID_TIME_T* timestamp, bool toWrite)
{
	if (toWrite)
		m_imageFile = fileSystem->OpenFileWrite(filename.c_str(), basePath);
	else
		m_imageFile = fileSystem->OpenFileRead(filename.c_str(), true, basePath);

	if (!m_imageFile)
		return false;

	m_imageName = filename;
	m_imagetimestamp = m_imageFile->Timestamp();

	if (timestamp)
		*timestamp = m_imagetimestamp;
	
	return true;
}

void btImageLoader::Close(void)
{
	m_imageName.Clear();
	m_imagetimestamp = 0;
	fileSystem->CloseFile(m_imageFile);
}


/*

This file only has a single entry point:

void R_LoadImage( const char *name, byte **pic, int *width, int *height, bool makePowerOf2 );

*/
struct loaderDef_s
{
	imageFileType_t		imgType;
	const char			ext[4];
};

static const unsigned k_numImageLoaders = 3;
static const loaderDef_s k_imagesTipesLoaders[k_numImageLoaders] = 
{
	{ TGA, "tga"},
	{ PNG, "png" },
	{ JPG, "jpg" }//,
	//{ BMP, "bmp" },
	//{ HDR, "hdr" }
};

/*
=================
R_LoadImage

Loads any of the supported image types into a cannonical
32 bit format.

Automatically attempts to load .jpg files if .tga files fail to load.

*pic will be NULL if the load failed.

Anything that is going to make this into a texture would use
makePowerOf2 = true, but something loading an image as a lookup
table of some sort would leave it in identity form.

It is important to do this at image load time instead of texture load
time for bump maps.

Timestamp may be NULL if the value is going to be ignored

If pic is NULL, the image won't actually be loaded, it will just find the
timestamp.
=================
*/
void R_LoadImage(const char* cname, byte** pic, int* width, int* height, imageDataType_t *type, ID_TIME_T* timestamp, bool makePowerOf2)
{
	btImageLoader *imgLoader = NULL;
	idStr ext, origName, name = cname;
	if (pic)
		*pic = NULL;

	//set the timestamp
	if (timestamp)
		*timestamp = FILE_NOT_FOUND_TIMESTAMP;

	//set the width
	if (width)
		*width = 0;

	//set the height
	if (height)
		*height = 0;

	//set the defalt
	if (type)
		*type = DT_UNSIGNED_BYTE;

	if (name.Length() < 5)
		return;

	name.ToLower();
	origName = name;
	name.ExtractFileExtension(ext);

	// RB begin
	if (!ext.IsEmpty())
	{
		if (!ext.Icmp("tga"))
			imgLoader = new(TAG_IMAGE) btTgaImage;
		else if (!ext.Icmp("jpg"))
			imgLoader = new(TAG_IMAGE) btJpgImage;
		else if (!ext.Icmp("png"))
			imgLoader = new(TAG_IMAGE) btPngImage;
/*		else if (!ext.Icmp("hdr"))
		{
			imgLoader = new(TAG_IMAGE) btHdrImage;
			if (type != NULL)
				*type = DT_FLOAT32;
		}*/
		else
			common->Warning("R_LoadImage(%s), trying Load a image whit a unknow file format\n", cname);

		if (imgLoader->Open(name, NULL, timestamp))
			goto done;
	}

	//begin from the last 
	int loader = k_numImageLoaders - 1;
	do
	{
		delete imgLoader;

		name.SetFileExtension(k_imagesTipesLoaders[loader].ext);
		if (k_imagesTipesLoaders[loader].imgType == TGA)
			imgLoader = new(TAG_IMAGE) btTgaImage;
		else if (k_imagesTipesLoaders[loader].imgType == PNG)
			imgLoader = new(TAG_IMAGE) btPngImage;
		else if (k_imagesTipesLoaders[loader].imgType == JPG)
			imgLoader = new(TAG_IMAGE) btJpgImage;
/*		else if (k_imagesTipesLoaders[loader].imgType == HDR)
		{
			imgLoader = new(TAG_IMAGE) btHdrImage;
			if (type != NULL)
				*type = DT_FLOAT32;
		}*/

		if(imgLoader->Open(name, NULL, timestamp))
			goto done;

		loader--;
	} while (loader >= 0);

	common->Warning("Can't determine the type of loading image file: %s\n", cname);
	return;

done:
	imgLoader->Load((void**)pic, width, height);

	//done close the image file
	imgLoader->Close();
	// RB end

	if ((width && *width < 1) || (height && *height < 1))
	{
		if (pic && *pic)
		{
			R_StaticFree(*pic);
			*pic = 0;
		}
	}

	return;
}

/*
=======================
R_LoadCubeImages

Loads six files with proper extensions
=======================
*/
bool R_LoadCubeImages(const char* imgName, cubeFiles_t extensions, byte* pics[6], int* outSize, imageDataType_t *type, ID_TIME_T* timestamp)
{
	int		i, j;
	const char*	cameraSides[6] = { "_forward.tga", "_back.tga", "_left.tga", "_right.tga",
		"_up.tga", "_down.tga"
	};
	const char*	axisSides[6] = { "_px.tga", "_nx.tga", "_py.tga", "_ny.tga",
		"_pz.tga", "_nz.tga"
	};
	const char**	sides;
	char	fullName[MAX_IMAGE_NAME];
	int		width, height, size = 0;

	if (extensions == CF_CAMERA)
		sides = cameraSides;
	else
		sides = axisSides;

	// FIXME: precompressed cube map files
	if (pics)
		memset(pics, 0, 6 * sizeof(pics[0]));

	if (timestamp)
		*timestamp = 0;

	for (i = 0; i < 6; i++)
	{
		idStr::snPrintf(fullName, sizeof(fullName), "%s%s", imgName, sides[i]);

		ID_TIME_T thisTime;
		if (!pics)
			R_LoadImageProgram(fullName, NULL, &width, &height, &thisTime);// just checking timestamps
		else
			R_LoadImageProgram(fullName, &pics[i], &width, &height, &thisTime);

		if (thisTime == FILE_NOT_FOUND_TIMESTAMP)
			break;

		if (i == 0)
			size = width;

		if (width != size || height != size)
		{
			common->Warning("Mismatched sizes on cube map '%s'", imgName);
			break;
		}

		if (timestamp)
		{
			if (thisTime > *timestamp)
			{
				*timestamp = thisTime;
			}
		}

		if (pics && extensions == CF_CAMERA)
		{
			// convert from "camera" images to native cube map images
			byte*	currFace = pics[i];
			switch (i)
			{
			case 0:	// forward
			{
				/*if(*type == DT_FLOAT32)
					R_RotatePicF32((float*)currFace, width);
				else*/
				{
					R_RotatePic(currFace, width);
				}
				break;
			}
			case 1:	// back
			{
				/*if (*type == DT_FLOAT32)
				{
					R_RotatePicF32((float*)currFace, width);
					R_HorizontalFlipF32((float*)currFace, width, height);
					R_VerticalFlipF32((float*)currFace, width, height);
				}
				else*/
				{
					R_RotatePic(currFace, width);
					R_HorizontalFlip(currFace, width, height);
					R_VerticalFlip(currFace, width, height);
				}
				break;
			}
			case 2:	// left
			{
				//if (*type == DT_FLOAT32)
					//R_VerticalFlipF32((float*)currFace, width, height);
				//else
					R_VerticalFlip(currFace, width, height);
				break;
			}
			case 3:	// right
			{
				/*if (*type == DT_FLOAT32)
					R_HorizontalFlipF32(currFace, width, height);
				else*/
					R_HorizontalFlip(currFace, width, height);
				break;
			}
			case 4:	// up
			{
				/*if (*type == DT_FLOAT32)
					R_RotatePicF32((float*)currFace, width);
				else*/
					R_RotatePic((byte*)currFace, width);

				break;
			}
			case 5: // down
			{
				/*if (*type == DT_FLOAT32)
					R_RotatePicF32((float*)currFace, width);
				else*/
					R_RotatePic((byte*)currFace, width);

				break;
			}
			}
		}
	}

	if (i != 6)
	{
		// we had an error, so free everything
		if (pics)
		{
			for (j = 0; j < i; j++)
			{
				R_StaticFree(pics[j]);
			}
		}

		if (timestamp)
		{
			*timestamp = 0;
		}
		return false;
	}

	if (outSize)
	{
		*outSize = size;
	}
	return true;
}

void R_WriteImage(idStr filename, const byte * data, int width, int height, imageFileType_t imgType, bool flipVertical, const char * basePath)
{
	idStr name = filename;
	name.StripFileExtension();
	btSmartPtr<btImageLoader> imageWriter;

	if (imgType == TGA)
	{
		imageWriter = new(TAG_IMAGE) btTgaImage();
		name.SetFileExtension("tga");
	}
	else if (imgType == PNG)
	{
		imageWriter = new(TAG_IMAGE) btPngImage();
		name.SetFileExtension("png");
	}
	else if (imgType == JPG)
	{
		imageWriter = new(TAG_JPG) btJpgImage();
		name.SetFileExtension("jpg");
	}
/*	else if (imgType == HDR)
	{
		imageWriter = new(TAG_IMAGE) btHdrImage();
		name.SetFileExtension("hdr");
	}*/
	else
	{
		common->Warning("R_WriteImage(%s),Try writing a invalid image format", filename.c_str());
		return;
	}

	//open to read file
	imageWriter->Open(name.c_str(), basePath, NULL, true);
	//fill image buffer and close
	imageWriter->Write(data, width, height, 32, flipVertical);
}
