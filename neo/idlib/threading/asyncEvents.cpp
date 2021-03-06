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
#include "precompiled.h"
#pragma hdrstop

#include "asyncevents.h"

btAsyncQueue::btAsyncQueue(void): m_currentQueue(0), m_asyncQueues(2), m_asyncQueue(&m_asyncQueues[0])
{
}

btAsyncQueue::~btAsyncQueue(void)
{
	//delete m_demultiplexerMutex;
	//m_demultiplexerMutex = NULL;
}

void btAsyncQueue::EnqueueCapsule(const btIntrusiveSmartPtr<btAsyncCapsule>& Capsule)
{
	idScopedCriticalSection Mutex(m_demultiplexerMutex);
	m_asyncQueue->Append(Capsule);
}

void btAsyncQueue::DemultiplexEvents(void)
{
	CallQueue_t* LocalQueue = &m_asyncQueues[m_currentQueue];

	// switch current queue
	{
		idScopedCriticalSection Lock(m_demultiplexerMutex);

		m_currentQueue = (m_currentQueue + 1) % 2;
		m_asyncQueue = &m_asyncQueues[m_currentQueue];
	}

	// invoke callbacks
#if 0
	for (CallQueue_t::iterator i = LocalQueue->begin(); i != LocalQueue->end(); ++i)
		(*i)->Invoke();
#else
	for (int i = 0; i < LocalQueue->Num(); i++)
	{
		LocalQueue->operator[](i)->Invoke();
	}
#endif

	LocalQueue->Clear();
}
