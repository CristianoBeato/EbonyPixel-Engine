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

#include "precompiled.h"
#pragma hdrstop

#include "sys_glimp.h"
//#include "sys_sdl_local.h"
#include "renderer/tr_local.h"

idCVar r_useOpenGL32("r_useOpenGL32", "1", CVAR_INTEGER, "0 = OpenGL 2.0, 1 = OpenGL 3.2 compatibility profile, 2 = OpenGL 3.2 core profile", 0, 2);
idCVar in_nograb("in_nograb", "0", CVAR_SYSTEM | CVAR_NOCHEAT, "prevents input grabbing");
// RB: FIXME this shit. We need the OpenGL alpha channel for advanced rendering effects
idCVar r_waylandcompat("r_waylandcompat", "0", CVAR_SYSTEM | CVAR_NOCHEAT | CVAR_ARCHIVE, "wayland compatible framebuffer");

class idSort_VidMode : public idSort_Quick< vidMode_t, idSort_VidMode >
{
public:
	int Compare(const vidMode_t& a, const vidMode_t& b) const
	{
		int wd = a.width - b.width;
		int hd = a.height - b.height;
		int fd = a.displayHz - b.displayHz;
		return (hd != 0) ? hd : (wd != 0) ? wd : fd;
	}
};

// RB: resolutions supported by XreaL
static void FillStaticVidModes(idList<vidMode_t>& modeList)
{
	modeList.AddUnique(vidMode_t(320, 240, 60));
	modeList.AddUnique(vidMode_t(400, 300, 60));
	modeList.AddUnique(vidMode_t(512, 384, 60));
	modeList.AddUnique(vidMode_t(640, 480, 60));
	modeList.AddUnique(vidMode_t(800, 600, 60));
	modeList.AddUnique(vidMode_t(960, 720, 60));
	modeList.AddUnique(vidMode_t(1024, 768, 60));
	modeList.AddUnique(vidMode_t(1152, 864, 60));
	modeList.AddUnique(vidMode_t(1280, 720, 60));
	modeList.AddUnique(vidMode_t(1280, 768, 60));
	modeList.AddUnique(vidMode_t(1280, 800, 60));
	modeList.AddUnique(vidMode_t(1280, 1024, 60));
	modeList.AddUnique(vidMode_t(1360, 768, 60));
	modeList.AddUnique(vidMode_t(1440, 900, 60));
	modeList.AddUnique(vidMode_t(1680, 1050, 60));
	modeList.AddUnique(vidMode_t(1600, 1200, 60));
	modeList.AddUnique(vidMode_t(1920, 1080, 60));
	modeList.AddUnique(vidMode_t(1920, 1200, 60));
	modeList.AddUnique(vidMode_t(2048, 1536, 60));
	modeList.AddUnique(vidMode_t(2560, 1600, 60));

	modeList.SortWithTemplate(idSort_VidMode());
}

btGlimp::btGlimp()
{
	grabbed = false;
	window = NULL;
	context = NULL;
}

btGlimp::~btGlimp()
{
}

bool btGlimp::Init(glimpParms_t parms)
{
	int colorbits = 24;
	int depthbits = 24;
	int stencilbits = 8;

	common->Printf("Initializing OpenGL subsystem\n");

	//GLimp_PreInit(); // DG: make sure SDL is initialized

	// DG: make window resizable
	Uint32 flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;
	// DG end

	if (parms.fullScreen)
		flags |= SDL_WINDOW_FULLSCREEN;


	for (int i = 0; i < 16; i++)
	{
		// 0 - default
		// 1 - minus colorbits
		// 2 - minus depthbits
		// 3 - minus stencil
		if ((i % 4) == 0 && i)
		{
			// one pass, reduce
			switch (i / 4)
			{
			case 2:
				if (colorbits == 24)
					colorbits = 16;
				break;
			case 1:
				if (depthbits == 24)
					depthbits = 16;
				else if (depthbits == 16)
					depthbits = 8;
			case 3:
				if (stencilbits == 24)
					stencilbits = 16;
				else if (stencilbits == 16)
					stencilbits = 8;
			}
		}

		int tcolorbits = colorbits;
		int tdepthbits = depthbits;
		int tstencilbits = stencilbits;

		if ((i % 4) == 3)
		{
			// reduce colorbits
			if (tcolorbits == 24)
				tcolorbits = 16;
		}

		if ((i % 4) == 2)
		{
			// reduce depthbits
			if (tdepthbits == 24)
				tdepthbits = 16;
			else if (tdepthbits == 16)
				tdepthbits = 8;
		}

		if ((i % 4) == 1)
		{
			// reduce stencilbits
			if (tstencilbits == 24)
				tstencilbits = 16;
			else if (tstencilbits == 16)
				tstencilbits = 8;
			else
				tstencilbits = 0;
		}

		int channelcolorbits = 4;
		if (tcolorbits == 24)
			channelcolorbits = 8;

		int alphacolorbits = channelcolorbits;
		if (r_waylandcompat.GetBool())
			alphacolorbits = 0;
		
		//set sdl openGl parameters
		setGlParms(channelcolorbits, alphacolorbits, tdepthbits, tstencilbits, parms.multiSamples, parms.stereo);

		//try create the window
		createWindow(parms.width, parms.height, parms.fullScreen, flags);
		if (!this->window)
		{
			common->DPrintf("Couldn't set GL mode %d/%d/%d: %s", channelcolorbits, tdepthbits, tstencilbits, SDL_GetError() );
			continue;
		}

		if (SDL_GL_SetSwapInterval(r_swapInterval.GetInteger()) < 0)
			common->Warning("SDL_GL_SWAP_CONTROL not supported");

		// RB begin
		SDL_GetWindowSize(this->window, &glConfig.nativeScreenWidth, &glConfig.nativeScreenHeight);
		// RB end

		glConfig.isFullscreen = (SDL_GetWindowFlags(this->window) & SDL_WINDOW_FULLSCREEN) == SDL_WINDOW_FULLSCREEN;

		common->Printf("Using %d color bits, %d depth, %d stencil display\n",
			channelcolorbits, tdepthbits, tstencilbits);

		glConfig.colorBits = tcolorbits;
		glConfig.depthBits = tdepthbits;
		glConfig.stencilBits = tstencilbits;

		// RB begin
		glConfig.displayFrequency = 60;
		glConfig.isStereoPixelFormat = parms.stereo;
		glConfig.multisamples = parms.multiSamples;

		glConfig.pixelAspect = 1.0f;	// FIXME: some monitor modes may be distorted
										// should side-by-side stereo modes be consider aspect 0.5?

										// RB end

		break;
	}

	if (!window)
	{
		common->Printf("No usable GL mode found: %s", SDL_GetError());
		return false;
	}

#ifdef __APPLE__
	glewExperimental = GL_TRUE;
#endif

	GLenum glewResult = glewInit();
	if (GLEW_OK != glewResult)
	{
		// glewInit failed, something is seriously wrong
		common->Printf("^3GLimp_Init() - GLEW could not load OpenGL subsystem: %s", glewGetErrorString(glewResult));
		return false;
	}
	else
		common->Printf("Using GLEW %s\n", glewGetString(GLEW_VERSION));

	return true;
}

void btGlimp::Shutdown(void)
{
	if (context)
	{
		SDL_GL_DeleteContext(context);
		context = NULL;
	}

	if (window)
	{
		SDL_DestroyWindow(window);
		window = NULL;
	}
}

void btGlimp::SwapBuffers(void)
{
	SDL_GL_SwapWindow(window);
}

bool btGlimp::SetScreenParms(glimpParms_t parms)
{
	if (parms.fullScreen > 0 || parms.fullScreen == -2)
	{
		if (!SetScreenParmsFullscreen(parms))
			return false;
	}
	else if (parms.fullScreen == 0) // windowed mode
	{
		if (!SetScreenParmsWindowed(parms))
			return false;
	}
	else
	{
		common->Warning("GLimp_SetScreenParms: fullScreen -1 (borderless window for multiple displays) currently unsupported!");
		return false;
	}

	SDL_GL_SetAttribute(SDL_GL_STEREO, parms.stereo ? 1 : 0);

	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, parms.multiSamples ? 1 : 0);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, parms.multiSamples);

	glConfig.isFullscreen = parms.fullScreen;
	glConfig.isStereoPixelFormat = parms.stereo;
	glConfig.nativeScreenWidth = parms.width;
	glConfig.nativeScreenHeight = parms.height;
	glConfig.displayFrequency = parms.displayHz;
	glConfig.multisamples = parms.multiSamples;

	return true;
}

void btGlimp::SetGamma(unsigned short red[256], unsigned short green[256], unsigned short blue[256])
{
	if (!window)
	{
		common->Warning("GLimp_SetGamma called without window");
		return;
	}

	if (SDL_SetWindowGammaRamp(window, red, green, blue))
		common->Warning("Couldn't set gamma ramp: %s", SDL_GetError());
}

void btGlimp::GrabInput(int flags)
{
	bool grab = flags & GRAB_ENABLE;

	if (grab && (flags & GRAB_REENABLE))
		grab = false;

	if (flags & GRAB_SETSTATE)
		grabbed = grab;

	if (in_nograb.GetBool())
		grab = false;

	if (!window)
	{
		common->Warning("GLimp_GrabInput called without window");
		return;
	}

	//free cursor whe show console
	if (grabbed)
		SDL_ShowCursor(SDL_DISABLE);
	else
		SDL_ShowCursor(SDL_ENABLE);

	// DG: check for GRAB_ENABLE instead of GRAB_HIDECURSOR because we always wanna hide it
	SDL_SetRelativeMouseMode(flags & GRAB_ENABLE ? SDL_TRUE : SDL_FALSE);
	SDL_SetWindowGrab(window, grab ? SDL_TRUE : SDL_FALSE);
}

void btGlimp::TestSwapBuffers(const idCmdArgs & args)
{
	idLib::Printf("TestSwapBuffers\n");
	static const int MAX_FRAMES = 5;
	uint64	timestamps[MAX_FRAMES];
	glDisable(GL_SCISSOR_TEST);

	int frameMilliseconds = 16;
	for (int swapInterval = 2; swapInterval >= -1; swapInterval--)
	{

		for (int i = 0; i < MAX_FRAMES; i++)
		{
			if (swapInterval == -1)
			{
				sys->Sleep(frameMilliseconds);
			}
			if (i & 1)
			{
				glClearColor(0, 1, 0, 1);
			}
			else
			{
				glClearColor(1, 0, 0, 1);
			}
			glClear(GL_COLOR_BUFFER_BIT);
			SDL_GL_SwapWindow(window);
			glFinish();
			timestamps[i] = sys->Microseconds();
		}

		idLib::Printf("\nswapinterval %i\n", swapInterval);
		for (int i = 1; i < MAX_FRAMES; i++)
		{
			idLib::Printf("%i microseconds\n", (int)(timestamps[i] - timestamps[i - 1]));
		}
	}
}

void btGlimp::ShowWindow(bool show)
{
	if (show)
		SDL_ShowWindow(window);
	else
		SDL_HideWindow(window);
}

bool btGlimp::IsWindowVisible(void)
{
	return false;
}

SDL_Window * btGlimp::GetWindowHandler(void)
{
	return window;
}

int btGlimp::ScreenParmsHandleDisplayIndex(glimpParms_t parms)
{
	int displayIdx;
	if (parms.fullScreen > 0)
	{
		displayIdx = parms.fullScreen - 1; // first display for SDL is 0, in parms it's 1
	}
	else // -2 == use current display
	{
		displayIdx = SDL_GetWindowDisplayIndex(window);
		if (displayIdx < 0) // for some reason the display for the window couldn't be detected
			displayIdx = 0;
	}

	if (parms.fullScreen > SDL_GetNumVideoDisplays())
	{
		common->Warning("Can't set fullscreen mode to display number %i, because SDL2 only knows about %i displays!",
			parms.fullScreen, SDL_GetNumVideoDisplays());
		return -1;
	}

	if (parms.fullScreen != glConfig.isFullscreen)
	{
		// we have to switch to another display
		if (glConfig.isFullscreen)
		{
			// if we're already in fullscreen mode but want to switch to another monitor
			// we have to go to windowed mode first to move the window.. SDL-oddity.
			SDL_SetWindowFullscreen(window, SDL_FALSE);
		}
		// select display ; SDL_WINDOWPOS_UNDEFINED_DISPLAY() doesn't work.
		int x = SDL_WINDOWPOS_CENTERED_DISPLAY(displayIdx);
		// move window to the center of selected display
		SDL_SetWindowPosition(window, x, x);
	}
	return displayIdx;
}

bool btGlimp::SetScreenParmsFullscreen(glimpParms_t parms)
{
	SDL_DisplayMode m = { 0 };
	int displayIdx = ScreenParmsHandleDisplayIndex(parms);
	if (displayIdx < 0)
		return false;

	// get current mode of display the window should be full-screened on
	SDL_GetCurrentDisplayMode(displayIdx, &m);

	// change settings in that display mode according to parms
	// FIXME: check if refreshrate, width and height are supported?
	// m.refresh_rate = parms.displayHz;
	m.w = parms.width;
	m.h = parms.height;

	// set that displaymode
	if (SDL_SetWindowDisplayMode(window, &m) < 0)
	{
		common->Warning("Couldn't set window mode for fullscreen, reason: %s", SDL_GetError());
		return false;
	}

	// if we're currently not in fullscreen mode, we need to switch to fullscreen
	if (!(SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN))
	{
		if (SDL_SetWindowFullscreen(window, SDL_TRUE) < 0)
		{
			common->Warning("Couldn't switch to fullscreen mode, reason: %s!", SDL_GetError());
			return false;
		}
	}
	return true;
}

bool btGlimp::SetScreenParmsWindowed(glimpParms_t parms)
{
	SDL_SetWindowSize(window, parms.width, parms.height);
	SDL_SetWindowPosition(window, parms.x, parms.y);

	// if we're currently in fullscreen mode, we need to disable that
	if (SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN)
	{
		if (SDL_SetWindowFullscreen(window, SDL_FALSE) < 0)
		{
			common->Warning("Couldn't switch to windowed mode, reason: %s!", SDL_GetError());
			return false;
		}
	}
	return true;
}

void btGlimp::PreInit(void)
{
	if (!SDL_WasInit(SDL_INIT_VIDEO))
	{
		if (SDL_Init(SDL_INIT_VIDEO))
			common->Error("Error while initializing SDL: %s", SDL_GetError());
	}
}

void btGlimp::setGlParms(int colorbits, int alphabits, int depthbits, int stencilbits, 
	int multiSamples, bool  stereo)
{
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, colorbits);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, colorbits);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, colorbits);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, alphabits);

	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, depthbits);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, stencilbits);

	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, multiSamples ? 1 : 0);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, multiSamples);

	SDL_GL_SetAttribute(SDL_GL_STEREO, stereo ? 1 : 0);

	// RB begin
	if (r_useOpenGL32.GetInteger() > 0)
	{
		glConfig.driverType = GLDRV_OPENGL32_COMPATIBILITY_PROFILE;

		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

		if (r_debugContext.GetBool())
		{
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
		}
	}

	if (r_useOpenGL32.GetInteger() > 1)
	{
		glConfig.driverType = GLDRV_OPENGL32_CORE_PROFILE;
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	}
	// RB end
}

void btGlimp::createWindow(int width, int height, int fullScreen, Uint32 flags)
{
	// DG: set display num for fullscreen
	int windowPos = SDL_WINDOWPOS_UNDEFINED;
	if (fullScreen > 0)
	{
		if (fullScreen > SDL_GetNumVideoDisplays())
		{
			common->Warning("Couldn't set display to num %i because we only have %i displays", fullScreen, SDL_GetNumVideoDisplays());
		}
		else
		{
			// -1 because SDL starts counting displays at 0, while parms.fullScreen starts at 1
			windowPos = SDL_WINDOWPOS_UNDEFINED_DISPLAY((fullScreen - 1));
		}
	}
	// TODO: if parms.fullScreen == -1 there should be a borderless window spanning multiple displays
	/*
	* NOTE that this implicitly handles parms.fullScreen == -2 (from r_fullscreen -2) meaning
	* "do fullscreen, but I don't care on what monitor", at least on my box it's the monitor with
	* the mouse cursor.
	*/

	window = SDL_CreateWindow(GAME_NAME, windowPos,  windowPos, width, height, flags);
	// DG end

	context = SDL_GL_CreateContext(window);
}

/*
====================
btGlimp::GetModeListForDisplay
====================
*/
bool btGlimp::GetModeListForDisplay(const int requestedDisplayNum, idList<vidMode_t>& modeList)
{
	assert(requestedDisplayNum >= 0);

	modeList.Clear();

	// DG: SDL2 implementation
	if (requestedDisplayNum >= SDL_GetNumVideoDisplays())
	{
		// requested invalid displaynum
		return false;
	}

	int numModes = SDL_GetNumDisplayModes(requestedDisplayNum);
	if (numModes > 0)
	{
		for (int i = 0; i < numModes; i++)
		{
			SDL_DisplayMode m;
			int ret = SDL_GetDisplayMode(requestedDisplayNum, i, &m);
			if (ret != 0)
			{
				common->Warning("Can't get video mode no %i, because of %s\n", i, SDL_GetError());
				continue;
			}

			vidMode_t mode;
			mode.width = m.w;
			mode.height = m.h;
			mode.displayHz = m.refresh_rate ? m.refresh_rate : 60; // default to 60 if unknown (0)
			modeList.AddUnique(mode);
		}

		if (modeList.Num() < 1)
		{
			common->Warning("Couldn't get a single video mode for display %i, using default ones..!\n", requestedDisplayNum);
			FillStaticVidModes(modeList);
		}

		// sort with lowest resolution first
		modeList.SortWithTemplate(idSort_VidMode());
	}
	else
	{
		common->Warning("Can't get Video Info, using default modes...\n");
		if (numModes < 0)
		{
			common->Warning("Reason was: %s\n", SDL_GetError());
		}
		FillStaticVidModes(modeList);
	}

	return true;
}
