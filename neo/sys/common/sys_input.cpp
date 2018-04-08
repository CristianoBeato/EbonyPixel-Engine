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
#include "precompiled.h"
#pragma hdrstop

#include "sys_input.h"

/*
================================================================================================
mouse SDL
================================================================================================
*/
btMouseSDL::btMouseSDL()
{
}

btMouseSDL::~btMouseSDL()
{
}

bool btMouseSDL::Init(void)
{
	m_mousePoll.SetGranularity(64);

	return true;
}

void btMouseSDL::Shutdown(void)
{
	m_mousePoll.Clear();
}

int btMouseSDL::PollMouseInputEvents(int mouseEvents[MAX_MOUSE_EVENTS][2])
{
	int numEvents = m_mousePoll.Num();

	if (numEvents > MAX_MOUSE_EVENTS)
		numEvents = MAX_MOUSE_EVENTS;

	for (int i = 0; i < numEvents; i++)
	{
		const btMousePoll_s& mp = m_mousePoll[i];

		mouseEvents[i][0] = mp.action;
		mouseEvents[i][1] = mp.value;
	}

	m_mousePoll.SetNum(0);
	return numEvents;
}

void btMouseSDL::PollEndMouseInputEvents(void)
{
	m_mousePoll.SetNum(0);
}

void btMouseSDL::Append(int a, int v)
{
	m_mousePoll.Append(btMousePoll_s(a, v));
}

/*
================================================================================================
keyboard SDL
================================================================================================
*/
btKeyboardSDL::btKeyboardSDL()
{
}

btKeyboardSDL::~btKeyboardSDL()
{
}

bool btKeyboardSDL::Init(void)
{
	m_keyboardPoll.SetGranularity(64);

	return true;
}

void btKeyboardSDL::Shutdown(void)
{
	m_keyboardPoll.Clear();
}

int btKeyboardSDL::PollKeyboardInputEvents(void)
{
	return m_keyboardPoll.Num();
}

int btKeyboardSDL::ReturnKeyboardInputEvent(const int n, int & ch, bool & state)
{
	if (n >= m_keyboardPoll.Num())
		return 0;

	ch = m_keyboardPoll[n].m_key;
	state = m_keyboardPoll[n].m_state;
	return 1;
}

void btKeyboardSDL::EndKeyboardInputEvents(void)
{
	m_keyboardPoll.SetNum(0);
}

void btKeyboardSDL::Append(int a, int v)
{
	m_keyboardPoll.Append(btKbdPoll_s(a, v));
}

/*
================================================================================================
Joystick SDL
================================================================================================
*/
btJoystickSDL::btJoystickSDL(void) : idJoystick()
{
}

btJoystickSDL::~btJoystickSDL(void)
{
	Shutdown();
}

bool btJoystickSDL::Init(void)
{
	SDL_GameController*	ctrl;
	SDL_Haptic*			haptic;

	for (int i = 0; i <  SDL_NumJoysticks(); i++)
	{
		if (SDL_IsGameController(i))
		{
			//reserve the controllers handlers
			ctrl = SDL_GameControllerOpen(i);
			if (ctrl)
			{
				common->Printf("GameController %i name: %s\n", i, SDL_GameControllerName(ctrl));
				common->Printf("GameController %i is mapped as \"%s\".\n", i, SDL_GameControllerMapping(ctrl));

				//Get controller haptic device
				haptic = SDL_HapticOpenFromJoystick(SDL_GameControllerGetJoystick(ctrl));
				if (haptic)
				{
					//Get initialize rumble
					if (SDL_HapticRumbleInit(haptic) < 0)
						common->Printf("Warning: Unable to initialize rumble! SDL Error: %s\n", SDL_GetError());
				}
				else
					common->Printf("Warning: Controller does not support haptics! SDL Error: %s\n", SDL_GetError());

				//append to controller list
				joysticks.Append(btJoyHandler_s(ctrl, haptic));
			}
			else
				common->Printf("Could not open gamecontroller %i: %s\n", i, SDL_GetError());
		}
	}

	return true;
}

void btJoystickSDL::Shutdown(void)
{
	//Close game controller with haptics
	for (int i = 0; i < joysticks.Num(); i++)
	{
		if (joysticks[i].controllerHaptic != NULL)
			SDL_HapticClose(joysticks[i].controllerHaptic);

		if (joysticks[i].controller == NULL)
			continue;

		SDL_GameControllerClose(joysticks[i].controller);
	}

	joysticks.Clear();
}

void btJoystickSDL::SetRumble(int deviceNum, int rumbleLow, int rumbleHigh)
{
	if (joysticks.Num() <= deviceNum)
		return;

	SDL_Haptic*  gControllerHaptic = joysticks[deviceNum].controllerHaptic;

	if (gControllerHaptic == NULL)
		return;

	if (SDL_HapticRumblePlay(gControllerHaptic, 0.75, m_JoystickFeedbackDuration) != 0)
	{
		printf("Warning: Unable to play rumble! %s\n", SDL_GetError());
	}
}

int btJoystickSDL::PollInputEvents(int inputDeviceNum)
{
	int numEvents = 0;

	if (joysticks.Num() <= inputDeviceNum)
		return numEvents;

	//todo add assert
	numEvents = joysticks[inputDeviceNum].JoystickPoll.Num();

	return numEvents;
}

int btJoystickSDL::ReturnInputEvent(int deviceNum, const int n, int & action, int & value)
{
	if (joysticks.Num() <= deviceNum)
		return 0;

	// Get last element of the list and copy into argument references
	const btJoystickPoll_s& mp = joysticks[deviceNum].JoystickPoll[n];
	action = mp.action;
	value = mp.value;

	return 1;
}

void btJoystickSDL::EndInputEvents(void)
{
	// Empty the joystick event container. This is called after
	// all joystick events have been read using Sys_ReturnJoystickInputEvent()
	for (size_t i = 0; i < joysticks.Num(); i++)
	{
		joysticks[0].JoystickPoll.SetNum(0);
	}
}

void btJoystickSDL::Append(int deviceNum, int a, int v)
{
	if (joysticks.Num() <= deviceNum)
		return;

	//todo add assert
	joysticks[deviceNum].JoystickPoll.Append(btJoystickPoll_s(a, v));
}
