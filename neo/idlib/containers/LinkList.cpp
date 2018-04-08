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

#include "LinkList.h"

#if 0
// list link interface
template <typename _type_>
struct btListLink
{
public:

	btListLink(void);
	~btListLink(void);

	// inserts link between two links
	void Link(btListLink *prev, btListLink *next);

	// unlinks link from list
	void Unlink(void);

private:

	// these classes need to access private data members
	friend class idLinkList<_type_>;
	friend struct btListIterator<_type_>;
	friend struct btListConstIterator<_type_>;

	btListLink *m_prev, *m_next;
	idLinkList<_type_> *m_list;
};

/*
================
btListLink<_type_>::btListLink
================
*/
template<typename _type_>
btListLink<_type_>::btListLink(void):
	m_prev(nullptr), 
	m_next(nullptr), 
	m_list(nullptr)
{
	m_prev = m_next = this;
}

/*
================
btListLink<_type_>::~btListLink
================
*/
template<typename _type_>
btListLink<_type_>::~btListLink(void)
{
	Unlink();
}

/*
================
idLinkList<_type_>::InList
Create a new link in list
================
*/
template<typename _type_>
void btListLink<_type_>::Link(btListLink * prev, btListLink * next)
{
	// update prev link
	prev->m_next = this;
	this->m_prev = prev;

	// update next link
	next->m_prev = this;
	this->m_next = next;

	// update list pointer
	m_list = next->m_list;

	// update list size
	++m_list->m_size;
}

/*
================
idLinkList<_type_>::InList
remove the link element 
================
*/
template<typename _type_>
void btListLink<_type_>::Unlink(void)
{
	if (m_list)
	{
		// update links
		m_prev->m_next = m_next;
		m_next->m_prev = m_prev;

		// update list size
		--m_list->m_size;

		// clean up pointers
		m_list = nullptr;
		m_prev = m_next = nullptr;
	}
}

//Helper Functions
template <typename T>
btListLink<T> *GetLinkFromObj(T *obj, uint linkOffset)
{
	return reinterpret_cast<btListLink<T> *>(reinterpret_cast<byte *>(obj) + linkOffset);
}

template <typename T>
T *GetObjFromLink(btListLink<T> *link, uint linkOffset)
{
	return reinterpret_cast<T *>(reinterpret_cast<byte *>(link) - linkOffset);
}
#endif