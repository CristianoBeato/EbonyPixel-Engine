/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 2016-2018 Cristiano Beato

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

#ifndef _CONSOLE_H_
#define _CONSOLE_H_

#include "Gwen/Gwen.h"
#include "Gwen/Controls.h"

enum MyEnum
{
	CONS_MODE_ERR = 0,
	CONS_MODE_SHORT = 1,
	CONS_MODE_FULL = 2
};

#define	COMMAND_HISTORY	64
#define CONSOLE_BUFFER_SIZE		16384


class Console : public Gwen::Controls::Base
{
	static const int k_consWidth = 540;
	static const int k_consHeight = 474;
public:
	Console(void * modal = nullptr);
	~Console();

	void		CreateConsoleWindow(int visLevel);

	void		CreateConsole(void);
	void		DestroyConsole(void);

	void		ShowConsole(bool quitOnClose, bool paralel = false);

	const char*	ConsoleInput(void);
	void		SetErrorText(const char* buf);
	void		ConsoleAppendText(const char* pMsg);

private:
	void		createWindowLayout(int visLevel);
	static int	StaticEntryPoint(void* Ptr);
	void		RunConsole(void);

	//callbacks
	void onButtonCopy(Gwen::Controls::Base* pControl);
	void onButtonClear(Gwen::Controls::Base* pControl);
	void onButtonQuit(Gwen::Controls::Base* pControl);
	void onSetStatusBarText(Gwen::Controls::Base* pControl, void* text);
	void onSubmitTextConsole(Gwen::Controls::Base* pControl);
	
	char									errorString[80];
	char									consoleText[512], returnedText[512];
	bool									quitOnClose;
	idEditField								consoleField;
	idEditField								historyEditLines[COMMAND_HISTORY];
	idStr									consoleHistory;

	bool									m_done;
	void*									m_modal;
	Gwen::Font								m_ConsFont;
	Gwen::Renderer::Base*					m_ConsoleRenderer;
	Gwen::Controls::WindowCanvas*			m_ConsoleCanvas;
	Gwen::Controls::StatusBar*				m_StatusBar;
	Gwen::Controls::TextBox*				m_consoleComandInput;
	Gwen::Controls::TextBox*				m_lastErrorBox;
	Gwen::Controls::TextBoxMultiline*		m_consoleLogBox;

	//parallel console thread 
	SDL_Thread *							m_thread;
};

#endif // !_CONSOLE_H_
