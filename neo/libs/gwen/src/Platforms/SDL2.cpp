/*
GWEN

Copyright (c) 2010 Facepunch Studios
Copyright (c) 2017 -2018 Cristiano Beato - SDL port
MIT License

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include "Gwen/Macros.h"
#include "Gwen/Platforms/Platform.h"
#include "Gwen/Input/SDL2.h"

#include <SDL.h>

static Gwen::UnicodeString gs_ClipboardEmulator;
static Gwen::Input::SDL2 GwenInput;

void Gwen::Platform::Sleep( unsigned int iMS )
{
	SDL_Delay(iMS);
}

void Gwen::Platform::SetCursor( unsigned char iCursor )
{
	SDL_SystemCursor cursors[] = {
		SDL_SYSTEM_CURSOR_ARROW,
		SDL_SYSTEM_CURSOR_IBEAM,
		SDL_SYSTEM_CURSOR_WAIT,
		SDL_SYSTEM_CURSOR_CROSSHAIR,
		SDL_SYSTEM_CURSOR_WAITARROW,
		SDL_SYSTEM_CURSOR_SIZENWSE,
		SDL_SYSTEM_CURSOR_SIZENESW,
		SDL_SYSTEM_CURSOR_SIZEWE,
		SDL_SYSTEM_CURSOR_SIZENS,
		SDL_SYSTEM_CURSOR_SIZEALL,
		SDL_SYSTEM_CURSOR_NO,
		SDL_SYSTEM_CURSOR_HAND,
		SDL_NUM_SYSTEM_CURSORS
	};

	SDL_SetCursor(SDL_CreateSystemCursor(cursors[iCursor]));
}

Gwen::UnicodeString Gwen::Platform::GetClipboardText()
{
	Gwen::String cpbText = SDL_GetClipboardText();
	return Gwen::Utility::StringToUnicode(cpbText);
}

bool Gwen::Platform::SetClipboardText( const Gwen::UnicodeString & str )
{
	Gwen::String cpbText = Gwen::Utility::UnicodeToString(str);
	return (SDL_SetClipboardText(cpbText.c_str()) != 0);
}

#define CLOCKS_PER_SEC 1000

float Gwen::Platform::GetTimeInSeconds()
{
	float fSeconds = (float)SDL_GetTicks() / (float)CLOCKS_PER_SEC;
	return fSeconds;
}

bool Gwen::Platform::FileOpen( const String & Name, const String & StartPath, const String & Extension, Gwen::Event::Handler* pHandler, Event::Handler::FunctionWithInformation fnCallback )
{
	// No platform independent way to do this.
	// Ideally you would open a system dialog here
	return false;
}

bool Gwen::Platform::FileSave( const String & Name, const String & StartPath, const String & Extension, Gwen::Event::Handler* pHandler, Gwen::Event::Handler::FunctionWithInformation fnCallback )
{
	// No platform independent way to do this.
	// Ideally you would open a system dialog here
	return false;
}

bool Gwen::Platform::FolderOpen( const String & Name, const String & StartPath, Gwen::Event::Handler* pHandler, Event::Handler::FunctionWithInformation fnCallback )
{
	return false;
}

void* Gwen::Platform::CreatePlatformWindow( int x, int y, int w, int h, const Gwen::String & strWindowTitle, unsigned int flags)
{
	if (SDL_WasInit(SDL_INIT_VIDEO) != 0)
		if (SDL_InitSubSystem(SDL_INIT_VIDEO) > 0)
		{
			printf("ERROR: SDL_InitSubSystem %s\n", SDL_GetError());
			return NULL;
		}

	SDL_Window	*window = SDL_CreateWindow(strWindowTitle.c_str(), x, y, w, h, flags);
	return static_cast<void*>(window);
}

void Gwen::Platform::DestroyPlatformWindow( void* pPtr )
{
	SDL_Window* window = static_cast<SDL_Window*>(pPtr);
	SDL_DestroyWindow(window);
}

void Gwen::Platform::MessagePump( void* pWindow, Gwen::Controls::Canvas* ptarget )
{
	if (SDL_WasInit(SDL_INIT_EVENTS) != 0)
		if (SDL_InitSubSystem(SDL_INIT_EVENTS) > 0)
		{
			printf("ERROR: SDL_InitSubSystem %s\n", SDL_GetError());
			return;
		}

	SDL_Event Event;
	if (SDL_WaitEvent(&Event) == 0)
		return;

	EventPump(pWindow, ptarget, Event);
}

void Gwen::Platform::EventPump(void * pWindow, Gwen::Controls::Canvas * ptarget, SDL_Event eventHandler)
{
	SDL_Window *window = static_cast<SDL_Window*>(pWindow);
	
	if (!window)
		return;

	if (!ptarget)
		return;
	
	Uint32 windowId = SDL_GetWindowID(window);
	GwenInput.Initialize(ptarget);
	
	GwenInput.ProcessEvents(eventHandler, windowId);

	//Get the Events
	GwenInput.ProcessWindowEvents(eventHandler, windowId);
	
	//check for mouse focus
	if (ptarget->GetMouseInputEnabled())
		GwenInput.ProcessMouseEvents(eventHandler, windowId);
	
	//if have the keyboard focus process the events
	if (ptarget->GetKeyboardInputEnabled())
		GwenInput.ProcessKeyboardEvents(eventHandler, windowId);
}

void Gwen::Platform::SetBoundsPlatformWindow( void* pPtr, int x, int y, int w, int h )
{
	SDL_Window *window = static_cast<SDL_Window*>(pPtr);
	//SDL_GL_GetDrawableSize()
}

void Gwen::Platform::SetWindowMaximized( void* pPtr, bool bMax, Gwen::Point & pNewPos, Gwen::Point & pNewSize )
{
	SDL_Window *window = static_cast<SDL_Window*>(pPtr);
	SDL_MaximizeWindow(window);
}

void Gwen::Platform::SetWindowMinimized( void* pPtr, bool bMinimized )
{
	SDL_Window *window = static_cast<SDL_Window*>(pPtr);
	SDL_MinimizeWindow(window);
}

bool Gwen::Platform::HasFocusPlatformWindow( void* pPtr )
{
	//check if current window is the focused
	SDL_Window *window = static_cast<SDL_Window*>(pPtr);
	Uint32 Flags = SDL_GetWindowFlags(window);

	if (Flags == SDL_WINDOW_INPUT_FOCUS || Flags == SDL_WINDOW_MOUSE_FOCUS)
		return true;

	return false;
}

bool Gwen::Platform::IsWindowVisible(void* pWindow)
{
	//check if current window is the focused
	SDL_Window *window = static_cast<SDL_Window*>(pWindow);
	Uint32 Flags = SDL_GetWindowFlags(window);

	if (Flags != SDL_WINDOW_HIDDEN || Flags != SDL_WINDOW_MINIMIZED)
		return true;

	return false;
}

void Gwen::Platform::GetDesktopSize( int & w, int & h )
{
	SDL_Rect info;
	if (SDL_WasInit(SDL_INIT_VIDEO) < 0)
		SDL_assert("SDL_INIT_VIDEO was not ini");
	SDL_GetDisplayBounds(0, &info);

	w = info.w;
	h = info.h;
}

void Gwen::Platform::GetCursorPos( Gwen::Point & po )
{
	SDL_GetRelativeMouseState((int*)&po.x, (int*)&po.y);
}
