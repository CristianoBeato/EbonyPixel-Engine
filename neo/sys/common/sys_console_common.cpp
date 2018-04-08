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

#include "precompiled.h"
#pragma hdrstop

#include "sys_console_common.h"
#include "Gwen/Skins/TexturedBase.h"
#include "Gwen/Controls.h"

Console::Console(void * modal) : Gwen::Controls::Base(NULL), m_modal(modal)
{
	m_ConsoleRenderer = NULL;
	m_ConsoleCanvas = NULL;
	m_StatusBar = NULL;
	m_consoleLogBox = NULL;
	m_consoleComandInput = NULL;
	m_lastErrorBox = NULL;
}

Console::~Console()
{
}

void Console::CreateConsoleWindow(int visLevel)
{

	if (SDL_WasInit(SDL_INIT_VIDEO) != 0)
	{
		if (SDL_InitSubSystem(SDL_INIT_VIDEO) > 0)
		{
			printf("ERROR: SDL_InitSubSystem %s\n", SDL_GetError());
			return;
		}
	}
	// Create a GWEN Renderer
	m_ConsoleRenderer = new Gwen::Renderer::SDL();

	// Create a GWEN skin
	Gwen::Skin::TexturedBase* pSkin = new Gwen::Skin::TexturedBase(m_ConsoleRenderer);

	//preload the font
	m_ConsFont.facename = L"base/newfonts/orbitron_medium.ttf";
	m_ConsFont.size = 12;
	m_ConsoleRenderer->LoadFont(&m_ConsFont);

	// Create a Canvas (it's root, on which all other GWEN panels are created)
	m_ConsoleCanvas = new Gwen::Controls::WindowCanvas(-1, -1, k_consWidth, k_consHeight, pSkin,	GAME_NAME, SDL_WINDOW_UTILITY);

	// The fonts work differently in sdl - it can't use
	// system fonts. So force the skin to use a local one.
	pSkin->Init("base/mainSkin.bmp");
	pSkin->SetDefaultFont(L"base/newfonts/consola.ttf", 12);

	m_ConsoleCanvas->SetDrawBackground(true);
	m_ConsoleCanvas->SetBackgroundColor(Gwen::Color(150, 170, 170, 255));

	//create the windgets
	createWindowLayout(visLevel);
}

void Console::CreateConsole(void)
{
	consoleField.Clear();

	for (int i = 0; i < COMMAND_HISTORY; i++)
		historyEditLines[i].Clear();
}

void Console::DestroyConsole(void)
{
	consoleHistory.Clear();

	delete m_ConsoleRenderer;
	m_ConsoleRenderer = NULL;
	delete m_ConsoleCanvas;
	m_ConsoleCanvas = NULL;
	delete m_StatusBar;
	m_StatusBar = NULL;
	delete m_consoleComandInput;
	m_consoleComandInput = NULL;
	delete m_lastErrorBox;
	m_lastErrorBox = NULL;
	delete m_consoleLogBox;
	m_consoleLogBox = NULL;
	delete m_ConsoleCanvas;
	m_ConsoleCanvas = NULL;
	delete m_ConsoleRenderer;
	m_ConsoleRenderer = NULL;
}

void Console::ShowConsole(bool quitOnClose, bool paralel)
{
	this->quitOnClose = quitOnClose;

	void* ThreadParam = reinterpret_cast<void*>(this);
	//if true create console in a parallel thread 
	if (paralel)
	{
		m_thread = SDL_CreateThread(StaticEntryPoint, "GameConsole", ThreadParam);
		SDL_DetachThread(m_thread);
	}
	else
		StaticEntryPoint(ThreadParam);
}

const char * Console::ConsoleInput(void)
{
	if (consoleText[0] == 0)
		return NULL;

	strcpy(returnedText, consoleText);
	consoleText[0] = 0;

	return returnedText;
}

void Console::SetErrorText(const char * buf)
{
	Gwen::TextObject console = buf;
	m_lastErrorBox->SetText(console);
}

void Console::ConsoleAppendText(const char * pMsg)
{
	char buffer[CONSOLE_BUFFER_SIZE * 2];
	char* b = buffer;
	const char* msg;
	int bufLen;
	int i = 0;
	static unsigned long s_totalChars;

	//
	// if the message is REALLY long, use just the last portion of it
	//
	if (strlen(pMsg) > CONSOLE_BUFFER_SIZE - 1)
		msg = pMsg + strlen(pMsg) - CONSOLE_BUFFER_SIZE + 1;
	else
		msg = pMsg;

	// copy into an intermediate buffer
	while (msg[i] && ((b - buffer) < sizeof(buffer) - 1))
	{
		if (msg[i] == '\n' && msg[i + 1] == '\r')
		{
			b[0] = '\r';
			b[1] = '\n';
			b += 2;
			i++;
		}
		else if (msg[i] == '\r')
		{
			b[0] = '\r';
			b[1] = '\n';
			b += 2;
		}
		else if (msg[i] == '\n')
		{
			b[0] = '\r';
			b[1] = '\n';
			b += 2;
		}
		else if (idStr::IsColor(&msg[i]))
		{
			i++;
		}
		else
		{
			*b = msg[i];
			b++;
		}
		i++;
	}
	*b = 0;
	bufLen = b - buffer;

	s_totalChars += bufLen;

	// replace selection instead of appending if we're overflowing
	if (s_totalChars > 0x7000)
	{
		//SendMessage(s_wcd.hwndBuffer, EM_SETSEL, 0, -1);
		s_totalChars = bufLen;
	}

	// put this text into the console log
	consoleHistory.Append(buffer);

	//update console log output
	if (m_consoleLogBox != NULL)
		m_consoleLogBox->SetText(consoleHistory.c_str());
}

void Console::createWindowLayout(int visLevel)
{
	//the status Bar
	{
		m_StatusBar = new Gwen::Controls::StatusBar(m_ConsoleCanvas);
		m_StatusBar->SetPadding(Gwen::Padding(0, 0, 0, 0));
		m_StatusBar->SetMargin(Gwen::Margin(0, 0, 0, 0));
		m_StatusBar->Dock(Gwen::Pos::Bottom);
		m_StatusBar->SendToBack();
	}
	{
		Gwen::Controls::Layout::Tile * ButtonTabble = new Gwen::Controls::Layout::Tile(m_ConsoleCanvas);
		ButtonTabble->SetPadding(Gwen::Padding(0, 0, 0, 0));
		ButtonTabble->SetMargin(Gwen::Margin(4, 2, 4, 2));
		ButtonTabble->SetHeight(24);
		ButtonTabble->Dock(Gwen::Pos::CenterH | Gwen::Pos::Bottom);
		//Copy Btn
		{
			Gwen::Controls::Button * CopyBtn = new Gwen::Controls::Button(ButtonTabble, "Copy");
			CopyBtn->SetText(L"copy");
			CopyBtn->SetWidth(72);
			CopyBtn->onPress.Add(this, &Console::onButtonCopy);
			CopyBtn->Dock(Gwen::Pos::Left);
		}
		//Clear Btn
		{
			Gwen::Controls::Button * ClerBtn = new Gwen::Controls::Button(ButtonTabble, "Clear");
			ClerBtn->SetText(L"Clear");
			ClerBtn->SetWidth(72);
			ClerBtn->SetMargin(Gwen::Margin(2, 0, 0, 0));
			ClerBtn->onPress.Add(this, &Console::onButtonClear);
			ClerBtn->Dock(Gwen::Pos::Left);
		}
		//Quit Btn
		{
			Gwen::Controls::Button * QutnBtn = new Gwen::Controls::Button(ButtonTabble, "Quit");
			QutnBtn->SetText(L"Quit");
			QutnBtn->SetWidth(72);
			QutnBtn->onPress.Add(this, &Console::onButtonQuit);
			QutnBtn->Dock(Gwen::Pos::Right);
		}
	}
	//Input controls
	if(visLevel > 0)
	{
		Gwen::Controls::Layout::Tile * InputTabble = new Gwen::Controls::Layout::Tile(m_ConsoleCanvas);
		InputTabble->SetPadding(Gwen::Padding(0, 0, 0, 0));
		InputTabble->SetMargin(Gwen::Margin(4, 2, 4, 2));
		InputTabble->SetHeight(24);
		InputTabble->Dock(Gwen::Pos::CenterH | Gwen::Pos::Bottom);
		//Submit Btn
		{
			Gwen::Controls::Button * SubmitBtn = new Gwen::Controls::Button(InputTabble, "Submit");
			SubmitBtn->SetText(L"Submit");
			SubmitBtn->SetTextColor(Gwen::Color(0, 0, 0, 255));
			SubmitBtn->SetWidth(72);
			SubmitBtn->onPress.Add(this, &Console::onSubmitTextConsole);
			SubmitBtn->Dock(Gwen::Pos::Right);
		}
		{
			m_consoleComandInput = new Gwen::Controls::TextBox(InputTabble, "ImputLabel");
			m_consoleComandInput->SetText("");
			m_consoleComandInput->SetWidth(455);
			m_consoleComandInput->SetCursor(1);
			m_consoleComandInput->Dock(Gwen::Pos::Left);
			m_consoleComandInput->onReturnPressed.Add(this, &Console::onSubmitTextConsole);
		}
	}
	{
		Gwen::Controls::Layout::Tile * LogTabble = new Gwen::Controls::Layout::Tile(m_ConsoleCanvas);
		LogTabble->SetPadding(Gwen::Padding(0, 0, 0, 0));
		LogTabble->SetMargin(Gwen::Margin(4, 2, 4, 2));
		LogTabble->SetHeight(390);
		LogTabble->Dock(Gwen::Pos::CenterH | Gwen::Pos::Top);
		//Error Text Box
		{
			m_lastErrorBox = new Gwen::Controls::TextBox(LogTabble);
			m_lastErrorBox->SetSize(526, 30);
			m_lastErrorBox->SetEditable(false);
			m_lastErrorBox->SetTextColor(Gwen::Color(255, 0, 0, 255)); //RED color for the Console
			m_lastErrorBox->SetShouldDrawBackground(true);
			m_lastErrorBox->SetMargin(Gwen::Margin(0, 2, 0, 2));
			m_lastErrorBox->Dock(Gwen::Pos::CenterV | Gwen::Pos::Top);
		}
		//Log text box
		{
			m_consoleLogBox = new Gwen::Controls::TextBoxMultiline(LogTabble);
			m_consoleLogBox->SetTextColor(Gwen::Color(0, 0, 0, 255));
			//m_consoleLogBox->SetFont(&m_ConsFont);
			m_consoleLogBox->SetSize(526, 354);
			m_consoleLogBox->SetShouldDrawBackground(true);
			m_consoleLogBox->SetCursor(1);
			m_consoleLogBox->SetMargin(Gwen::Margin(0, 2, 0, 0));
			m_consoleLogBox->Dock(Gwen::Pos::CenterV | Gwen::Pos::Bottom);
		}
		
	}
}

int Console::StaticEntryPoint(void* Ptr)
{
	Console* ConsoleHandler = reinterpret_cast<Console*>(Ptr);

	if (ConsoleHandler)
		ConsoleHandler->RunConsole();

	return 0;
}

void Console::RunConsole(void)
{
	// program main loop
	m_done = false;
	while (!m_done)
	{
		// if it's QUIT then quit..
		if (m_ConsoleCanvas->WantsQuit())
			m_done = true;

		// DRAWING STARTS HERE
		m_ConsoleCanvas->DoThink();
	} // end main loop
}

void Console::onButtonCopy(Gwen::Controls::Base * pControl)
{
	sys->SetClipboardData(consoleHistory.c_str());
	printf("console log copied\n");
}

void Console::onButtonClear(Gwen::Controls::Base * pControl)
{
	m_consoleLogBox->SetText("");
	consoleHistory.Clear();
	printf("console log clear\n");
}

void Console::onButtonQuit(Gwen::Controls::Base * pControl)
{
	//send a close comand
	m_ConsoleCanvas->InputQuit();
}

void Console::onSetStatusBarText(Gwen::Controls::Base * pControl, void * text)
{
	const wchar_t * Vec = (wchar_t*)text;
	m_StatusBar->SetTextColor(Gwen::Color(0, 0, 0, 0xff));
	m_StatusBar->SetText(Vec);
}

void Console::onSubmitTextConsole(Gwen::Controls::Base * pControl)
{
	//get the text from imput and send to the game console
	Gwen::TextObject command = m_consoleComandInput->GetText();
	ConsoleAppendText(command.c_str());

	m_consoleComandInput->SetText(L"");

	//get the game console and send to the log output
	Gwen::TextObject console =	ConsoleInput();
	m_consoleLogBox->SetText(console);
}
