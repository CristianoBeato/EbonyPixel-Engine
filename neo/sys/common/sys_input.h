/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
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
#ifndef _SYS_IMPUT_H_
#define _SYS_IMPUT_H_

#include <SDL_mouse.h>
#include <SDL_keycode.h>
#include <SDL_keyboard.h>
#include <SDL_gamecontroller.h>
#include <SDL_haptic.h>

/*
================================================================================================
mouse SDL
================================================================================================
*/
class btMouseSDL
{
	struct btMousePoll_s
	{
		int action;
		int value;

		btMousePoll_s()
		{
		}

		btMousePoll_s(int a, int v)
		{
			action = a;
			value = v;
		}
	};

public:
	btMouseSDL(void);
	~btMouseSDL(void);

	bool	Init(void);
	void	Shutdown(void);
	void	Release(void);
	void	Grab(void);
	int		PollMouseInputEvents(int mouseEvents[MAX_MOUSE_EVENTS][2]);
	void	PollEndMouseInputEvents(void);
	void	Append(int a, int v);
private:
	idList<btMousePoll_s> m_mousePoll;
};

/*
================================================================================================
keyboard SDL
================================================================================================
*/
class btKeyboardSDL
{
	struct btKbdPoll_s
	{
		int		m_key;
		bool	m_state;

		btKbdPoll_s(void)
		{
		}

		btKbdPoll_s(int k, bool s)
		{
			m_key = k;
			m_state = s;
		}
	};

public:
	btKeyboardSDL(void);
	~btKeyboardSDL(void);

	bool	Init(void);
	void	Shutdown(void);

	int		PollKeyboardInputEvents(void);
	int		ReturnKeyboardInputEvent(const int n, int& ch, bool& state);
	void	EndKeyboardInputEvents(void);
	void	Append( int a, int v);
private:
	idList<btKbdPoll_s> m_keyboardPoll;
};

/*
================================================================================================
Joystick SDL
================================================================================================
*/
class btJoystickSDL : idJoystick
{
	struct btJoystickPoll_s
	{
		int action;
		int value;

		btJoystickPoll_s(void)
		{
		}

		btJoystickPoll_s(int a, int v)
		{
			action = a;
			value = v;
		}
	};

	struct btJoyHandler_s
	{
		btJoyHandler_s(void)
		{
			controller = NULL;
			controllerHaptic = NULL;
		}

		btJoyHandler_s(SDL_GameController* cont, SDL_Haptic* hap)
		{
			controller = cont;
			controllerHaptic = hap;
		}

		SDL_GameController*			controller;
		SDL_Haptic*					controllerHaptic;
		idList<btJoystickPoll_s>	JoystickPoll;
	};

public:
	btJoystickSDL(void);
	~btJoystickSDL(void);

	virtual bool	Init(void);
	virtual void	Shutdown(void);
	virtual void	SetRumble(int deviceNum, int rumbleLow, int rumbleHigh);
	virtual int		PollInputEvents(int inputDeviceNum);
	virtual int		ReturnInputEvent(int deviceNum, const int n, int& action, int& value);
	virtual void	EndInputEvents(void);
	void			Append(int deviceNum, int a, int v);
private:
	float			m_JoystickFeedbackStrenght;
	Uint32			m_JoystickFeedbackDuration;
	idSysMutex						mutexXis;		// lock this before using currentXis or stickIntegrations
	idList<btJoyHandler_s>			joysticks;
};

#endif // !_SYS_IMPUT_H_
