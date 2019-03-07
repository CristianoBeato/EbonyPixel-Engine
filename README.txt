###==========================================================================###
						Ebony Pixel Game Engine
###==========================================================================###

Thank you for downloading Ebony Pixel



=========================================
CONTENTS
=========================================


This file contains the following sections:

	1) SYSTEM REQUIREMENT
	2) GENERAL NOTES
	3) LICENSE
	4) GETTING THE SOURCE CODE
	5) COMPILING ON WIN32
		5.1) WITH VISUAL C++ 2013 EXPRESS EDITION
		5.2) WITH MINGW 
	6) COMPILING ON WIN32 WITH MINGW
	7) COMPILING ON GNU/LINUX
	8) INSTALLATION, GETTING THE GAMEDATA, RUNNING THE GAME
	9) OVERALL CHANGES
	10) CONSOLE VARIABLES
	11) KNOWN ISSUES
	12) BUG REPORTS
	13) GAME MODIFICATIONS
	14) CODE LICENSE EXCEPTIONS
	
=========================================
1) SYSTEM REQUIREMENTS
=========================================

Minimum system requirements:

	CPU: 2 GHz Intel compatible
	System Memory: 512MB
	Graphics card: Any graphics card that supports Direct3D 11 and OpenGL >= 4.0

Recommended system requirements:

	CPU: 3 GHz + Intel compatible
	System Memory: 1024MB+
	Graphics card: Geforce 9600 GT, ATI HD 5650 or higher. 

=========================================
2) GENERAL NOTES
=========================================

This release does not contain any game data, the game data and may
have some incompatibilities with the original content of Doom 3 BFG,
you can implement your own content, or use content available to mod
communities over the internet.

Bink:
-----
The Ebony Pixel engine Code release does not include functionality for rendering Bink Videos.

Back End Rendering of Stencil Shadows:
--------------------------------------
The Ebony Pixel engine Source Code release does not include functionality enabling rendering
of stencil shadows via the "depth fail" method, a functionality commonly known as "Carmack's Reverse".

=========================================
3) LICENSE
=========================================
See COPYING.txt for the GNU GENERAL PUBLIC LICENSE
ADDITIONAL TERMS:  The Ebony Pixel Engine Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU GPL which accompanied the Doom 3 BFG Edition GPL Source Code.  If not, please request a copy in writing from id Software at id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.


=========================================
4) GETTING THE SOURCE CODE
=========================================
This project's GitHub.net Git repository can be checked out through Git with the following instruction set: 

	> git clone https://github.com/CristianoBeato/EbonyPixel-Engine.git

If you don't want to use git, you can download the source as a zip file at
	https://codeload.github.com/CristianoBeato/EbonyPixel-Engine/zip/master

=========================================
5) COMPILING ON WIN32
=========================================

	first download the DirectX SDK (June 2010) http://www.microsoft.com/en-us/download/details.aspx?id=6812

	and download and install the latest CMake.
	__________________________________________
	5.1)WITH VISUAL C++ 2013 EXPRESS EDITION
	__________________________________________
	
		1. Download and install the Visual C++ 2013 Express Edition.

		2. dowload the needed libs
			SDL2 development libraries - https://www.libsdl.org/release/SDL2-devel-2.0.8-VC.zip
			SDL2_TTF - Development libraries: https://www.libsdl.org/projects/SDL_ttf/release/SDL2_ttf-devel-2.0.14-VC.zip
	
			//optional 
			openal 1.1 https://www.openal.org/downloads/OpenAL11CoreSDK.zip

		3. Generate the VC13 projects using CMake by doubleclicking a matching configuration .bat file in the neo/ folder.

	__________________________________________
	5.2)CODE BLOCKS/MINGW/TDM-GCC
	__________________________________________
	
		1. Download and install the MingW/Code Blocks
			MingW - https://sourceforge.net/projects/mingw/files/latest/download?source=files
			or
			https://sourceforge.net/projects/tdm-gcc/files/TDM-GCC%20Installer
			
			MingW/Code Blocks - http://www.codeblocks.org/downloads/26
		
		2. dowload the needed libs
			SDL2 development libraries - https://www.libsdl.org/projects/SDL_ttf/release/SDL2_ttf-devel-2.0.14-mingw.tar.gz
			SDL2_TTF development libraries - https://www.libsdl.org/projects/SDL_ttf/release/SDL2_ttf-devel-2.0.14-mingw.tar.gz
		
		3. Generate the MingW/Code Blocks/TDM using CMake by doubleclicking a matching configuration .bat file in the neo/ folder.
		
=========================================
6) COMPILING ON GNU/LINUX
=========================================
	not tested yet - Work In Progress


___________________________________________________

7) INSTALLATION, GETTING THE GAMEDATA, RUNNING THE GAME
__________________________________________


If you use the prebuilt Win32 binaries then simply extract them to your
C:\Program Files (x86)\Steam\SteamApps\common\Doom 3 BFG Edition\ directory and run RBDoom3BFG.exe.




The following instructions are primarily intented for Linux users and all hackers on other operating systems.

To play the game, you need the game data from a legal copy of the game, which 
unfortunately requires Steam for Windows - Steam for Linux or OSX won't do, because
(at least currently) the Doom 3 BFG game is only installable on Steam for Windows.
Even the DVD version of Doom 3 BFG only contains encrytped data that is decoded
by Steam on install.

On Linux and OSX the easiest way to install is with SteamCMD: https://developer.valvesoftware.com/wiki/SteamCMD
See the description on https://developer.valvesoftware.com/wiki/SteamCMD#Linux (OS X is directly below that) on how to install SteamCMD on your system. You won't have to create a new user.

Then you can download Doom 3 BFG with

> ./steamcmd.sh +@sSteamCmdForcePlatformType windows +login <YOUR_STEAM_LOGIN_NAME> +force_install_dir ./doom3bfg/ +app_update 208200 validate +quit

(replace <YOUR_STEAM_LOGIN_NAME> with your steam login name)
When it's done you should have the normal windows installation of Doom 3 BFG in ./doom3bfg/ and the needed files in ./doom3bfg/base/
That number is the "AppID" of Doom 3 BFG; if you wanna use this to get the data of other games you own, you can look up the AppID at https://steamdb.info/

NOTE that we've previously recommended using download_depot in the Steam console to install the game data. That turned out to be unreliable and result in broken, unusable game data. So use SteamCMD instead, as described above.

Anyway:

1. Install Doom 3 BFG in Steam (Windows version) or SteamCMD, make sure it's getting
   updated/patched.

2. Create your own Doom 3 BFG directory, e.g. /path/to/Doom3BFG/

3. Copy the game-data's base dir from Steam to that directory 
   (e.g. /path/to/Doom3BFG/), it's in
	/your/path/to/Steam/steamapps/common/DOOM 3 BFG Edition/base/
	or, if you used SteamCMD, in the path you used above.

4. Copy your EbPxDemo executable that you created in 5) or 6) and the FFmpeg DLLs to your own 
   Doom 3 BFG directory (/path/to/Doom3BFG).
   
   Your own Doom 3 BFG directory now should look like:
	/path/to/EbPxDemo/
	 ->	EbPxDemo (or EbPxDemo.exe on Windows)
	 -> avcodec-55.dll
	 -> avdevice-55.dll
	 -> avfilter-4.dll
	 -> avformat-55.dll
	 -> avutil-52.dll
	 -> postproc-52.dll
	 -> swresample-0.dll
	 -> swscale-2.dll
	 ->	base/
		 ->	classicmusic/
		 ->	_common.crc
		 ->	(etc)

5. Run the game by executing the EbPxDemo executable.

6. Enjoy

7. If you run into bugs, please report them, see 11)

___________________________________________________

8) OVERALL CHANGES
__________________________________________

- Flexible build system using CMake

- Linux support (32 and 64 bit) - TO FIX

- Win64 support

- OS X support - TO FIX

- SDL cross platform implementations

- OpenAL Soft sound backend primarily developed for Linux but works on Windows as well

- PNG image support

- Endian Swaping by SDL2 
___________________________________________________

9) CONSOLE VARIABLES
__________________________________________

r_antiAliasing - Different Anti-Aliasing modes

r_useShadowMapping [0 or 1] - Use soft shadow mapping instead of hard stencil shadows

r_useHDR [0 or 1] - Use High Dynamic Range lighting

r_hdrAutoExposure [0 or 1] - Adaptive tonemapping with HDR
	This allows to have very bright or very dark scenes but the camera will adopt to it so the scene won't loose details
	
r_exposure [0 .. 1] - Default 0.5, Controls brightness and affects HDR exposure key
	This is what you change in the video brightness options

r_useSSAO [0 .. 1] - Use Screen Space Ambient Occlusion to darken the corners in the scene
	
r_useFilmicPostProcessEffects [0 or 1] - Apply several post process effects to mimic a filmic look"


___________________________________________________

10) KNOWN ISSUES
__________________________________________

- HDR does not work with old-school stencil shadows

- MSAA anti-aliasing modes don't work with HDR: Use SMAA

- Some lights cause shadow acne with shadow mapping

- Some shadows might almost disappear due to the shadow filtering

___________________________________________________

11) BUG REPORTS
__________________________________________

EbonyPixel-Engine is not perfect, it is not bug free as every other software.
For fixing as much problems as possible we need as much bug reports as possible.
We cannot fix anything if we do not know about the problems.

The best way for telling us about a bug is by submitting a bug report at our GitHub bug tracker page:

The most important fact about this tracker is that we cannot simply forget to fix the bugs which are posted there. 
It is also a great way to keep track of fixed stuff.

If you want to report an issue with the game, you should make sure that your report includes all information useful to characterize and reproduce the bug.

    * Search on Google
    * Include the computer's hardware and software description ( CPU, RAM, 3D Card, distribution, kernel etc. )
    * If appropriate, send a console log, a screenshot, an strace ..
    * If you are sending a console log, make sure to enable developer output:

              RBDoom3BFG.exe +set developer 1 +set logfile 2
			  
		You can find your qconsole.log on Windows in C:\Users\<your user name>\Saved Games\id Software\RBDOOM 3 BFG\base\

NOTE: We cannot help you with OS-specific issues like configuring OpenGL correctly, configuring ALSA or configuring the network.
	

___________________________________________________

12) GAME MODIFCATIONS
__________________________________________
	
The Doom 3 BFG Edition GPL Source Code release allows mod editing, in order for it to accept any change in your
mod directory, you should first specify your mod directory adding the following command to the launcher:

"+set fs_game modDirectoryName"

so it would end up looking like: RBDoom3BFG +set fs_game modDirectoryName


IMPORTANT: EbonyPixel-Engine does not support old Doom 3 modiciations that include sourcecode modifications in binary form (.dll)
You can fork EbonyPixel-Engine and create a new renamed binary that includes all required C++ game code modifications.
	
____________________________________________________________________________________

13) CODE LICENSE EXCEPTIONS - The parts that are not covered by the GPL:
_______________________________________________________________________


EXCLUDED CODE:  The code described below and contained in the EbonyPixel-Engine Source Code release
is not part of the Program covered by the GPL and is expressly excluded from its terms. 
You are solely responsible for obtaining from the copyright holder a license for such code and complying with the applicable license terms.


JPEG library
-----------------------------------------------------------------------------
neo/libs/jpeg-6/*

Copyright (C) 1991-1995, Thomas G. Lane

Permission is hereby granted to use, copy, modify, and distribute this
software (or portions thereof) for any purpose, without fee, subject to these
conditions:
(1) If any part of the source code for this software is distributed, then this
README file must be included, with this copyright and no-warranty notice
unaltered; and any additions, deletions, or changes to the original files
must be clearly indicated in accompanying documentation.
(2) If only executable code is distributed, then the accompanying
documentation must state that "this software is based in part on the work of
the Independent JPEG Group".
(3) Permission for use of this software is granted only if the user accepts
full responsibility for any undesirable consequences; the authors accept
NO LIABILITY for damages of any kind.

These conditions apply to any software derived from or based on the IJG code,
not just to the unmodified library.  If you use our work, you ought to
acknowledge us.

NOTE: unfortunately the README that came with our copy of the library has
been lost, so the one from release 6b is included instead. There are a few
'glue type' modifications to the library to make it easier to use from
the engine, but otherwise the dependency can be easily cleaned up to a
better release of the library.

zlib library
---------------------------------------------------------------------------
neo/libs/zlib/*

Copyright (C) 1995-2012 Jean-loup Gailly and Mark Adler

This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
 claim that you wrote the original software. If you use this software
 in a product, an acknowledgment in the product documentation would be
 appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be
 misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.

Base64 implementation
---------------------------------------------------------------------------
neo/idlib/Base64.cpp

Copyright (c) 1996 Lars Wirzenius.  All rights reserved.

June 14 2003: TTimo <ttimo@idsoftware.com>
	modified + endian bug fixes
	http://bugs.debian.org/cgi-bin/bugreport.cgi?bug=197039

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

IO for (un)compress .zip files using zlib
---------------------------------------------------------------------------
neo/libs/zlib/minizip/*

Copyright (C) 1998-2010 Gilles Vollant (minizip) ( http://www.winimage.com/zLibDll/minizip.html )

Modifications of Unzip for Zip64
Copyright (C) 2007-2008 Even Rouault

Modifications for Zip64 support
Copyright (C) 2009-2010 Mathias Svensson ( http://result42.com )

This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.

MD4 Message-Digest Algorithm
-----------------------------------------------------------------------------
neo/idlib/hashing/MD4.cpp
Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
rights reserved.

License to copy and use this software is granted provided that it
is identified as the "RSA Data Security, Inc. MD4 Message-Digest
Algorithm" in all material mentioning or referencing this software
or this function.

License is also granted to make and use derivative works provided
that such works are identified as "derived from the RSA Data
Security, Inc. MD4 Message-Digest Algorithm" in all material
mentioning or referencing the derived work.

RSA Data Security, Inc. makes no representations concerning either
the merchantability of this software or the suitability of this
software for any particular purpose. It is provided "as is"
without express or implied warranty of any kind.

These notices must be retained in any copies of any part of this
documentation and/or software.

MD5 Message-Digest Algorithm
-----------------------------------------------------------------------------
neo/idlib/hashing/MD5.cpp
This code implements the MD5 message-digest algorithm.
The algorithm is due to Ron Rivest.  This code was
written by Colin Plumb in 1993, no copyright is claimed.
This code is in the public domain; do with it what you wish.

CRC32 Checksum
-----------------------------------------------------------------------------
neo/idlib/hashing/CRC32.cpp
Copyright (C) 1995-1998 Mark Adler

