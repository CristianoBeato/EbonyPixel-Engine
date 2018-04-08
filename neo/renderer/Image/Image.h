/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
Copyright (C) 2013-2014 Robert Beckebans
Copyright (C) 2016-2018 Cristiano Beato.

This file is part of the Doom 3 BFG Edition GPL Source Code ("Doom 3 BFG Edition Source Code").

Doom 3 BFG Edition Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 BFG Edition Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 BFG Edition Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 BFG Edition Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 BFG Edition Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

#ifndef _IMAGE_H_
#define _IMAGE_H_
/*
====================================================================
IMAGE
idImage have a one to one correspondance with GL/DX/GCM textures.
No texture is ever used that does not have a corresponding idImage.
====================================================================
*/

static const int	MAX_TEXTURE_LEVELS = 14;

// How is this texture used?  Determines the storage and color format
typedef enum
{
	TD_SPECULAR,			// may be compressed, and always zeros the alpha channel
	TD_DIFFUSE,				// may be compressed
	TD_DEFAULT,				// generic RGBA texture (particles, etc...)
	TD_BUMP,				// may be compressed with 8 bit lookup
	TD_FONT,				// Font image
	TD_LIGHT,				// Light image
	TD_LOOKUP_TABLE_MONO,	// Mono lookup table (including alpha)
	TD_LOOKUP_TABLE_ALPHA,	// Alpha lookup table with a white color channel
	TD_LOOKUP_TABLE_RGB1,	// RGB lookup table with a solid white alpha
	TD_LOOKUP_TABLE_RGBA,	// RGBA lookup table
	TD_COVERAGE,			// coverage map for fill depth pass when YCoCG is used
	TD_DEPTH,				// depth buffer copy for motion blur
	// RB begin
	TD_SHADOW_ARRAY,		// 2D depth buffer array for shadow mapping
	TD_RGBA16F,
	TD_RGBA32F,
	TD_R32F,
	// RB end
} textureUsage_t;

typedef enum
{
	CF_2D,			// not a cube map
	CF_NATIVE,		// _px, _nx, _py, etc, directly sent to GL
	CF_CAMERA,		// _forward, _back, etc, rotated and flipped as needed before sending to GL
	CF_2D_ARRAY		// not a cube map but not a single 2d texture either
} cubeFiles_t;

enum imageFileType_t
{
	TGA,
	PNG,
	JPG,
	HDR
};

#include "ImageOpts.h"
#include "BinaryImage.h"

#define	MAX_IMAGE_NAME	256

// Beato Begin:
//foward definition
class idImageManager;

class btBitmapBuffer
{
public:
	btBitmapBuffer(void);
	~btBitmapBuffer(void);

	void	clear(void);
	void	createPixels(uint imgWidth, uint imgHeight);
	void	copyPixels(byte* pixels, GLuint imgWidth, GLuint imgHeight);
	void	blitPixels(GLuint x, GLuint y, btBitmapBuffer*	&destination);

	byte*				getBuff(void) const;
	uint				getWidth(void) const;
	uint				getHeight(void) const;
	textureUsage_t		getUsage(void) const;

	textureUsage_t		m_bitmapUsage;
private:
	//Unpadded image dimensions
	uint32				m_bitmapWidth;
	uint32				m_bitmapHeight;
	byte*				m_bitmapPixels;

};
//Beato End

class idImage
{
	friend class	Framebuffer;
	friend class	idImageManager;
public:
	idImage( const char* name );
	
	const char* 	GetName() const
	{
		return imgName;
	}
	
	// Makes this image active on the current GL texture unit.
	// automatically enables or disables cube mapping
	// May perform file loading if the image was not preloaded.
	void		Bind();
	
	// Should be called at least once
	void		SetSamplerState( textureFilter_t tf, textureRepeat_t tr );
	
	// used by callback functions to specify the actual data
	// data goes from the bottom to the top line of the image, as OpenGL expects it
	// These perform an implicit Bind() on the current texture unit
	// FIXME: should we implement cinematics this way, instead of with explicit calls?
	void		GenerateImage( const byte* pic, int width, int height,
							   textureFilter_t filter, textureRepeat_t repeat, textureUsage_t usage, int msaaSamples = 0 );
	void		GenerateCubeImage( const byte* pic[6], int size,
								   textureFilter_t filter, textureUsage_t usage );
					
// Beato Begin
	void		generateFromBitmap(btBitmapBuffer * buff);
// Beato End
// RB begin
	void		GenerateShadowArray( int width, int height, textureFilter_t filter, textureRepeat_t repeat, textureUsage_t usage );
// RB end
	
	void		CopyFramebuffer( int x, int y, int width, int height );
	void		CopyDepthbuffer( int x, int y, int width, int height );
	
	void		UploadScratch( const byte* pic, int width, int height );
	
	// estimates size of the GL image based on dimensions and storage type
	int			StorageSize() const;
	
	// print a one line summary of the image
	void		Print() const;
	
	// check for changed timestamp on disk and reload if necessary
	void		Reload( bool force );
	
	void		AddReference()
	{
		refCount++;
	};
	
	void		MakeDefault();	// fill with a grid pattern
	
	const idImageOpts& 	GetOpts() const
	{
		return opts;
	}
	int			GetUploadWidth() const
	{
		return opts.width;
	}
	int			GetUploadHeight() const
	{
		return opts.height;
	}
	
	void		SetReferencedOutsideLevelLoad()
	{
		referencedOutsideLevelLoad = true;
	}
	void		SetReferencedInsideLevelLoad()
	{
		levelLoadReferenced = true;
	}
	void		ActuallyLoadImage( bool fromBackEnd );
	//---------------------------------------------
	// Platform specific implementations
	//---------------------------------------------
	
	void		AllocImage( const idImageOpts& imgOpts, textureFilter_t filter, textureRepeat_t repeat );
	
	// Deletes the texture object, but leaves the structure so it can be reloaded
	// or resized.
	void		PurgeImage();
	
	// z is 0 for 2D textures, 0 - 5 for cube maps, and 0 - uploadDepth for 3D textures. Only
	// one plane at a time of 3D textures can be uploaded. The data is assumed to be correct for
	// the format, either bytes, halfFloats, floats, or DXT compressed. The data is assumed to
	// be in OpenGL RGBA format, the consoles may have to reorganize. pixelPitch is only needed
	// when updating from a source subrect. Width, height, and dest* are always in pixels, so
	// they must be a multiple of four for dxt data.
	void		SubImageUpload( int mipLevel, int destX, int destY, int destZ,
								int width, int height, const void* data,
								int pixelPitch = 0 ) const;
								
	// SetPixel is assumed to be a fast memory write on consoles, degenerating to a
	// SubImageUpload on PCs.  Used to update the page mapping images.
	// We could remove this now, because the consoles don't use the intermediate page mapping
	// textures now that they can pack everything into the virtual page table images.
	void		SetPixel( int mipLevel, int x, int y, const void* data, int dataSize );
	
	// some scratch images are dynamically resized based on the display window size.  This
	// simply purges the image and recreates it if the sizes are different, so it should not be
	// done under any normal circumstances, and probably not at all on consoles.
	void		Resize( int width, int height );
	
	bool		IsCompressed() const
	{
		return ( opts.format == FMT_DXT1 || opts.format == FMT_DXT5 );
	}
	
	void		SetTexParameters();	// update aniso and trilinear
	
	bool		IsLoaded() const
	{
		return texnum != TEXTURE_NOT_LOADED;
	}
	
	static void			GetGeneratedName( idStr& _name, const textureUsage_t& _usage, const cubeFiles_t& _cube );
	
private:
	void				AllocImage();
	void				DeriveOpts();
	
	// parameters that define this image
	idStr				imgName;						// game path, including extension (except for cube maps), may be an image program
	cubeFiles_t			cubeFiles;						// If this is a cube map, and if so, what kind
	void	( *generatorFunction )( idImage* image );	// NULL for files
	textureUsage_t		usage;							// Used to determine the type of compression to use
	idImageOpts			opts;							// Parameters that determine the storage method
	
// beato Begin: generate a imagem from a scrap buffer 
	btBitmapBuffer		*bmpBuffer;
// beato End

	// Sampler settings
	textureFilter_t		filter;
	textureRepeat_t		repeat;
	
	bool				referencedOutsideLevelLoad;
	bool				levelLoadReferenced;	// for determining if it needs to be purged
	bool				defaulted;				// true if the default image was generated because a file couldn't be loaded
	ID_TIME_T			sourceFileTime;			// the most recent of all images used in creation, for reloadImages command
	ID_TIME_T			binaryFileTime;			// the time stamp of the binary file
	
	int					refCount;				// overall ref count
	
	static const GLuint TEXTURE_NOT_LOADED = 0xFFFFFFFF;
	
	GLuint				texnum;				// gl texture binding
	
	// we could derive these in subImageUpload each time if necessary
	GLuint				internalFormat;
	GLuint				dataFormat;
	GLuint				dataType;
	
	
};

ID_INLINE idImage::idImage( const char* name ) : imgName( name )
{
	texnum = TEXTURE_NOT_LOADED;
	internalFormat = 0;
	dataFormat = 0;
	dataType = 0;
	generatorFunction = NULL;
// Beato Begin: Scrap image generator
	bmpBuffer = NULL;
// Beato End
	filter = TF_DEFAULT;
	repeat = TR_REPEAT;
	usage = TD_DEFAULT;
	cubeFiles = CF_2D;
	
	referencedOutsideLevelLoad = false;
	levelLoadReferenced = false;
	defaulted = false;
	sourceFileTime = FILE_NOT_FOUND_TIMESTAMP;
	binaryFileTime = FILE_NOT_FOUND_TIMESTAMP;
	refCount = 0;
}

#endif //!_IMAGE_H_