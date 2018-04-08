/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
Copyright (C) 2013-2014 Robert Beckebans

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

#ifndef _IMAGE_PROCESS_H_
#define _IMAGE_PROCESS_H_
/*
====================================================================

IMAGEPROCESS

FIXME: make an "imageBlock" type to hold byte*,width,height?
====================================================================
*/

int		MakePowerOfTwo( int num );
byte*	R_Dropsample( const byte* in, int inwidth, int inheight, int outwidth, int outheight );
byte*	R_ResampleTexture( const byte* in, int inwidth, int inheight, int outwidth, int outheight );
byte*	R_MipMapWithAlphaSpecularity( const byte* in, int width, int height );
byte*	R_MipMapWithGamma( const byte* in, int width, int height );
byte*	R_MipMap( const byte* in, int width, int height );

// these operate in-place on the provided pixels
void	R_BlendOverTexture( byte* data, int pixelCount, const byte blend[4] );
void	R_HorizontalFlip( byte* data, int width, int height );
void	R_VerticalFlip( byte* data, int width, int height );
void	R_RotatePic( byte* data, int width );
void	R_ApplyCubeMapTransforms( int i, byte* data, int size );

/*
====================================================================

IMAGEPROGRAM

====================================================================
*/

void R_LoadImageProgram( const char* name, byte** pic, int* width, int* height, ID_TIME_T* timestamp, textureUsage_t* usage = NULL );
const char* R_ParsePastImageProgram( idLexer& src );

#endif //!_IMAGE_PROCESS_H_