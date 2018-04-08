/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
Copyright (C) 2012 dhewg (dhewm3)
Copyright (C) 2012-2014 Robert Beckebans
Copyright (C) 2013 Daniel Gibson
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
#ifndef _SYS_SDL_GLIMP_H_
#define	_SYS_SDL_GLIMP_H_

#include <SDL_video.h>
#include "renderer/tr_local.h"

const static int GRAB_ENABLE = (1 << 0);
const static int GRAB_REENABLE = (1 << 1);
const static int GRAB_HIDECURSOR = (1 << 2);
const static int GRAB_SETSTATE = (1 << 3);

class btGlimp
{
public:
	btGlimp(void);
	~btGlimp(void);

	bool			Init(glimpParms_t parms);
	void			Shutdown(void);
	void			SwapBuffers(void);
	bool			SetScreenParms(glimpParms_t parms);
	void			SetGamma(unsigned short red[256], unsigned short green[256], unsigned short blue[256]);
	void			GrabInput(int flags);
	void			TestSwapBuffers(const idCmdArgs& args);
	void			ShowWindow(bool show);
	bool			IsWindowVisible(void);
	SDL_Window	*	GetWindowHandler(void);
	void			PreInit(void);
	bool			GetModeListForDisplay(const int requestedDisplayNum, idList<vidMode_t>& modeList);
private:
	int		ScreenParmsHandleDisplayIndex(glimpParms_t parms);
	bool	SetScreenParmsFullscreen(glimpParms_t parms);
	bool	SetScreenParmsWindowed(glimpParms_t parms);
	void	setGlParms(int colorbits, int alphabits, int depthbits, int stencilbits, int multiSamples, bool  stereo);
	void	createWindow(int width, int height, int fullScreen, Uint32 flags);

	bool grabbed;
	SDL_Window* window;
	SDL_GLContext context;
};

#endif // !_SYS_SDL_GLIMP_H_
