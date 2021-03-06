/*
===========================================================================

Elbony Pixel Source Code
Copyright (C) 2017-2018 Cristiano Beato

This file is part of the Elbony Pixel Source Code ("Elbony Pixel Source Code").

Elbony Pixel Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Elbony Pixel Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Elbony Pixel Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Elbony Pixel Source Code is also subject to certain additional terms.
You should have received a copy of these additional terms immediately following the terms and conditions of the
GNU General Public License which accompanied the Elbony Pixel Source Code.

===========================================================================
*/
#ifndef _EVENTS_H_
#define _EVENTS_H_

#include "idLib/threading/Thread.h"
#include "iIntrusivePtr.h"

class btAsyncCapsule : idSysInterlockedInteger
{
public:
	// Run the method
	virtual void Invoke() = 0;
};

class btAsyncQueue
{
	typedef idList<btIntrusiveSmartPtr<btAsyncCapsule> > CallQueue_t;
public:
	btAsyncQueue(void);
	~btAsyncQueue(void);

	// Put the event into the events queue
	virtual void    EnqueueCapsule(const btIntrusiveSmartPtr<btAsyncCapsule>& Capsule);
	// Events demultiplexer as described in Reactor pattern
	virtual void	DemultiplexEvents(void);

private:
	idList<CallQueue_t> m_asyncQueues;

	// switched for shared non-locked access
	CallQueue_t*	m_asyncQueue;
	size_t			m_currentQueue;
	idSysMutex		m_demultiplexerMutex;
};

#endif // !_EVENTS_H_
