/*
===========================================================================

Doom 3 GPL Source Code
Copyright (C) 1999-2011 id Software LLC, a ZeniMax Media company.

This file is part of the Doom 3 GPL Source Code (?Doom 3 Source Code?).

Doom 3 Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

#pragma hdrstop
#include "precompiled.h"

#include "posix_public.h"
#include "sys/sys_local.h"

#include <termios.h>
#include <dlfcn.h>
#include <fcntl.h>

// terminal support
idCVar in_tty("in_tty", "1", CVAR_BOOL | CVAR_INIT | CVAR_SYSTEM, "terminal tab-completion and history");

static bool				tty_enabled = false;
static struct termios	tty_tc;

/*
===============
Posix_InitConsoleInput
===============
*/
void Posix_InitConsoleInput(void)
{
	struct termios tc;

	if (in_tty.GetBool())
	{
		if (isatty(STDIN_FILENO) != 1)
		{
			Sys_Printf("terminal support disabled: stdin is not a tty\n");
			in_tty.SetBool(false);
			return;
		}
		if (tcgetattr(0, &tty_tc) == -1)
		{
			Sys_Printf("tcgetattr failed. disabling terminal support: %s\n", strerror(errno));
			in_tty.SetBool(false);
			return;
		}
		// make the input non blocking
		if (fcntl(STDIN_FILENO, F_SETFL, fcntl(STDIN_FILENO, F_GETFL, 0) | O_NONBLOCK) == -1)
		{
			Sys_Printf("fcntl STDIN non blocking failed.  disabling terminal support: %s\n", strerror(errno));
			in_tty.SetBool(false);
			return;
		}
		tc = tty_tc;
		/*
		ECHO: don't echo input characters
		ICANON: enable canonical mode.  This  enables  the  special
		characters  EOF,  EOL,  EOL2, ERASE, KILL, REPRINT,
		STATUS, and WERASE, and buffers by lines.
		ISIG: when any of the characters  INTR,  QUIT,  SUSP,  or
		DSUSP are received, generate the corresponding signal
		*/
		tc.c_lflag &= ~(ECHO | ICANON);
		/*
		ISTRIP strip off bit 8
		INPCK enable input parity checking
		*/
		tc.c_iflag &= ~(ISTRIP | INPCK);
		tc.c_cc[VMIN] = 1;
		tc.c_cc[VTIME] = 0;
		if (tcsetattr(0, TCSADRAIN, &tc) == -1)
		{
			Sys_Printf("tcsetattr failed: %s\n", strerror(errno));
			Sys_Printf("terminal support may not work correctly. Use +set in_tty 0 to disable it\n");
		}
#if 0
		// make the output non blocking
		if (fcntl(STDOUT_FILENO, F_SETFL, fcntl(STDOUT_FILENO, F_GETFL, 0) | O_NONBLOCK) == -1)
		{
			Sys_Printf("fcntl STDOUT non blocking failed: %s\n", strerror(errno));
		}
#endif
		tty_enabled = true;
		// check the terminal type for the supported ones
		char* term = getenv("TERM");
		if (term)
		{
			if (strcmp(term, "linux") && strcmp(term, "xterm") && strcmp(term, "xterm-color") && strcmp(term, "screen"))
			{
				Sys_Printf("WARNING: terminal type '%s' is unknown. terminal support may not work correctly\n", term);
			}
		}
		Sys_Printf("terminal support enabled ( use +set in_tty 0 to disabled )\n");
	}
	else
	{
		Sys_Printf("terminal support disabled\n");
	}
}

/*
================
terminal support utilities
================
*/

void tty_Del()
{
	char key;
	key = '\b';
	write(STDOUT_FILENO, &key, 1);
	key = ' ';
	write(STDOUT_FILENO, &key, 1);
	key = '\b';
	write(STDOUT_FILENO, &key, 1);
}

void tty_Left()
{
	char key = '\b';
	write(STDOUT_FILENO, &key, 1);
}

void tty_Right()
{
	char key = 27;
	write(STDOUT_FILENO, &key, 1);
	write(STDOUT_FILENO, "[C", 2);
}

// clear the display of the line currently edited
// bring cursor back to beginning of line
void tty_Hide()
{
	int len, buf_len;
	if (!tty_enabled)
	{
		return;
	}
	if (input_hide)
	{
		input_hide++;
		return;
	}
	// clear after cursor
	len = strlen(input_field.GetBuffer()) - input_field.GetCursor();
	while (len > 0)
	{
		tty_Right();
		len--;
	}
	buf_len = strlen(input_field.GetBuffer());
	while (buf_len > 0)
	{
		tty_Del();
		buf_len--;
	}
	input_hide++;
}

// show the current line
void tty_Show()
{
	//	int i;
	if (!tty_enabled)
	{
		return;
	}
	assert(input_hide > 0);
	input_hide--;
	if (input_hide == 0)
	{
		char* buf = input_field.GetBuffer();
		if (buf[0])
		{
			write(STDOUT_FILENO, buf, strlen(buf));

			// RB begin
#if defined(__ANDROID__)
			//__android_log_print(ANDROID_LOG_DEBUG, "RBDoom3_DEBUG", "%s", buf);
#endif
			// RB end

			int back = strlen(buf) - input_field.GetCursor();
			while (back > 0)
			{
				tty_Left();
				back--;
			}
		}
	}
}

void tty_FlushIn()
{
	char key;
	while (read(0, &key, 1) != -1)
	{
		Sys_Printf("'%d' ", key);
	}
	Sys_Printf("\n");
}

/*
================
Posix_ConsoleInput
Checks for a complete line of text typed in at the console.
Return NULL if a complete line is not ready.
================
*/
char* Posix_ConsoleInput()
{
	if (tty_enabled)
	{
		int		ret;
		char	key;
		bool	hidden = false;
		while ((ret = read(STDIN_FILENO, &key, 1)) > 0)
		{
			if (!hidden)
			{
				tty_Hide();
				hidden = true;
			}
			switch (key)
			{
			case 1:
				input_field.SetCursor(0);
				break;
			case 5:
				input_field.SetCursor(strlen(input_field.GetBuffer()));
				break;
			case 127:
			case 8:
				input_field.CharEvent(K_BACKSPACE);
				break;
			case '\n':
				idStr::Copynz(input_ret, input_field.GetBuffer(), sizeof(input_ret));
				assert(hidden);
				tty_Show();
				write(STDOUT_FILENO, &key, 1);
				input_field.Clear();
				if (history_count < COMMAND_HISTORY)
				{
					history[history_count] = input_ret;
					history_count++;
				}
				else
				{
					history[history_start] = input_ret;
					history_start++;
					history_start %= COMMAND_HISTORY;
				}
				history_current = 0;
				return input_ret;
			case '\t':
				input_field.AutoComplete();
				break;
			case 27:
			{
				// enter escape sequence mode
				ret = read(STDIN_FILENO, &key, 1);
				if (ret <= 0)
				{
					Sys_Printf("dropping sequence: '27' ");
					tty_FlushIn();
					assert(hidden);
					tty_Show();
					return NULL;
				}
				switch (key)
				{
				case 79:
					ret = read(STDIN_FILENO, &key, 1);
					if (ret <= 0)
					{
						Sys_Printf("dropping sequence: '27' '79' ");
						tty_FlushIn();
						assert(hidden);
						tty_Show();
						return NULL;
					}
					switch (key)
					{
					case 72:
						// xterm only
						input_field.SetCursor(0);
						break;
					case 70:
						// xterm only
						input_field.SetCursor(strlen(input_field.GetBuffer()));
						break;
					default:
						Sys_Printf("dropping sequence: '27' '79' '%d' ", key);
						tty_FlushIn();
						assert(hidden);
						tty_Show();
						return NULL;
					}
					break;
				case 91:
				{
					ret = read(STDIN_FILENO, &key, 1);
					if (ret <= 0)
					{
						Sys_Printf("dropping sequence: '27' '91' ");
						tty_FlushIn();
						assert(hidden);
						tty_Show();
						return NULL;
					}
					switch (key)
					{
					case 49:
					{
						ret = read(STDIN_FILENO, &key, 1);
						if (ret <= 0 || key != 126)
						{
							Sys_Printf("dropping sequence: '27' '91' '49' '%d' ", key);
							tty_FlushIn();
							assert(hidden);
							tty_Show();
							return NULL;
						}
						// only screen and linux terms
						input_field.SetCursor(0);
						break;
					}
					case 50:
					{
						ret = read(STDIN_FILENO, &key, 1);
						if (ret <= 0 || key != 126)
						{
							Sys_Printf("dropping sequence: '27' '91' '50' '%d' ", key);
							tty_FlushIn();
							assert(hidden);
							tty_Show();
							return NULL;
						}
						// all terms
						input_field.KeyDownEvent(K_INS);
						break;
					}
					case 52:
					{
						ret = read(STDIN_FILENO, &key, 1);
						if (ret <= 0 || key != 126)
						{
							Sys_Printf("dropping sequence: '27' '91' '52' '%d' ", key);
							tty_FlushIn();
							assert(hidden);
							tty_Show();
							return NULL;
						}
						// only screen and linux terms
						input_field.SetCursor(strlen(input_field.GetBuffer()));
						break;
					}
					case 51:
					{
						ret = read(STDIN_FILENO, &key, 1);
						if (ret <= 0)
						{
							Sys_Printf("dropping sequence: '27' '91' '51' ");
							tty_FlushIn();
							assert(hidden);
							tty_Show();
							return NULL;
						}
						if (key == 126)
						{
							input_field.KeyDownEvent(K_DEL);
							break;
						}
						Sys_Printf("dropping sequence: '27' '91' '51' '%d'", key);
						tty_FlushIn();
						assert(hidden);
						tty_Show();
						return NULL;
					}
					case 65:
					case 66:
					{
						// history
						if (history_current == 0)
						{
							history_backup = input_field;
						}
						if (key == 65)
						{
							// up
							history_current++;
						}
						else
						{
							// down
							history_current--;
						}
						// history_current cycle:
						// 0: current edit
						// 1 .. Min( COMMAND_HISTORY, history_count ): back in history
						if (history_current < 0)
						{
							history_current = Min(COMMAND_HISTORY, history_count);
						}
						else
						{
							history_current %= Min(COMMAND_HISTORY, history_count) + 1;
						}
						int index = -1;
						if (history_current == 0)
						{
							input_field = history_backup;
						}
						else
						{
							index = history_start + Min(COMMAND_HISTORY, history_count) - history_current;
							index %= COMMAND_HISTORY;
							assert(index >= 0 && index < COMMAND_HISTORY);
							input_field.SetBuffer(history[index]);
						}
						assert(hidden);
						tty_Show();
						return NULL;
					}
					case 67:
						input_field.KeyDownEvent(K_RIGHTARROW);
						break;
					case 68:
						input_field.KeyDownEvent(K_LEFTARROW);
						break;
					default:
						Sys_Printf("dropping sequence: '27' '91' '%d' ", key);
						tty_FlushIn();
						assert(hidden);
						tty_Show();
						return NULL;
					}
					break;
				}
				default:
					Sys_Printf("dropping sequence: '27' '%d' ", key);
					tty_FlushIn();
					assert(hidden);
					tty_Show();
					return NULL;
				}
				break;
			}
			default:
				if (key >= ' ')
				{
					input_field.CharEvent(key);
					break;
				}
				Sys_Printf("dropping sequence: '%d' ", key);
				tty_FlushIn();
				assert(hidden);
				tty_Show();
				return NULL;
			}
		}
		if (hidden)
		{
			tty_Show();
		}
		return NULL;
	}
	else
	{
		// disabled on OSX. works fine from a terminal, but launching from Finder is causing trouble
		// I'm pretty sure it could be re-enabled if needed, and just handling the Finder failure case right (TTimo)
#ifndef __APPLE__
		// no terminal support - read only complete lines
		int				len;
		fd_set			fdset;
		struct timeval	timeout;

		FD_ZERO(&fdset);
		FD_SET(STDIN_FILENO, &fdset);
		timeout.tv_sec = 0;
		timeout.tv_usec = 0;
		if (select(1, &fdset, NULL, NULL, &timeout) == -1 || !FD_ISSET(0, &fdset))
		{
			return NULL;
		}

		len = read(0, input_ret, sizeof(input_ret));
		if (len == 0)
		{
			// EOF
			return NULL;
		}

		if (len < 1)
		{
			Sys_Printf("read failed: %s\n", strerror(errno));	// something bad happened, cancel this line and print an error
			return NULL;
		}

		if (len == sizeof(input_ret))
		{
			Sys_Printf("read overflow\n");	// things are likely to break, as input will be cut into pieces
		}

		input_ret[len - 1] = '\0';		// rip off the \n and terminate
		return input_ret;
#endif
	}
	return NULL;
}