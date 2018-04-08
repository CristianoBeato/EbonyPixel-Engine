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

#ifndef _SMART_POINTER_
#define _SMART_POINTER_

template <typename T>
class btSmartPtr
{
public:
	btSmartPtr(void);
	btSmartPtr(T *ptr);

	//destructor 
	~btSmartPtr(void);

	T* operator = ( T* objptr);
	T& operator *(void);
	T* operator ->(void);

	T* GetInternalPtr(void) const;

	//Mimic a dynamic_cast behavior
	template <typename U>
	U *DynamicCast(void) const;

private:
	T *m_ptr;
};

template <typename T>
ID_INLINE btSmartPtr<T>::btSmartPtr(void) : m_ptr(NULL)
{
}

template <typename T>
ID_INLINE btSmartPtr<T>::btSmartPtr(T * ptr) : m_ptr(ptr)
{
}

template <typename T>
ID_INLINE btSmartPtr<T>::~btSmartPtr(void)
{
	delete m_ptr;
}

template<typename T>
ID_INLINE T * btSmartPtr<T>::operator = (T* objptr)
{
	//bypass to const
	return m_ptr = objptr;
}

template<typename T>
ID_INLINE T & btSmartPtr<T>::operator *(void)
{
	return *m_ptr;
}

template<typename T>
ID_INLINE T * btSmartPtr<T>::operator->(void)
{
	return m_ptr;
}

template<typename T>
template<typename U>
ID_INLINE U * btSmartPtr<T>::DynamicCast(void) const
{
	return static_cast<U>(m_ptr);
}

#endif // !_SMART_POINTER_