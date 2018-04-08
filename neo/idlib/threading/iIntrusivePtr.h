/*
 * Copyright (C) 2013 Sergey Kosarevsky (sk@linderdaum.com)
 * Copyright (C) 2013 Viktor Latypov (vl@linderdaum.com)
 * Based on Linderdaum Engine http://www.linderdaum.com
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must display the names 'Sergey Kosarevsky' and
 *    'Viktor Latypov'in the credits of the application, if such credits exist.
 *    The authors of this work must be notified via email (sk@linderdaum.com) in
 *    this case of redistribution.
 *
 * 3. Neither the name of copyright holders nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS
 * IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _INTRUSIVE_PTR_H_
#define _INTRUSIVE_PTR_H_

// Intrusive reference-countable object for garbage collection

namespace LPtr
{
	void IncRef( void* p );
	void DecRef( void* p );
};

// Intrusive smart pointer
template <class T> class btIntrusiveSmartPtr
{
private:
	class btProtector
	{
	private:
		void operator delete( void* );
	};
public:
	// default constructor
	btIntrusiveSmartPtr(): m_objPtr( 0 )
	{
	}
	// copy constructor
	btIntrusiveSmartPtr( const btIntrusiveSmartPtr& Ptr ): m_objPtr( Ptr.m_objPtr )
	{
		LPtr::IncRef( m_objPtr );
	}
	template <typename U> btIntrusiveSmartPtr( const btIntrusiveSmartPtr<U>& Ptr ): m_objPtr( Ptr.GetInternalPtr() )
	{
		LPtr::IncRef( m_objPtr );
	}
	// constructor from T*
	btIntrusiveSmartPtr( T* const Object ): m_objPtr( Object )
	{
		LPtr::IncRef( m_objPtr );
	}
	// destructor
	~btIntrusiveSmartPtr()
	{
		LPtr::DecRef( m_objPtr );
	}
	// check consistency
	inline bool IsValid(void) const
	{
		return m_objPtr != 0;
	}
	/// assignment of btIntrusiveSmartPtr
	btIntrusiveSmartPtr& operator = ( const btIntrusiveSmartPtr& Ptr )
	{
		T* Temp = m_objPtr;
		m_objPtr = Ptr.m_objPtr;

		LPtr::IncRef( Ptr.m_objPtr );
		LPtr::DecRef( Temp );

		return *this;
	}
	/// -> operator
	inline T* operator -> (void) const
	{
		return m_objPtr;
	}
	/// allow "if ( btIntrusiveSmartPtr )" construction
	inline operator btProtector* (void) const
	{
		if ( !m_objPtr )
		{
			return NULL;
		}

		static btProtector Protector;

		return &Protector;
	}
	// cast
	template <typename U> inline btIntrusiveSmartPtr<U> DynamicCast(void) const
	{
		return btIntrusiveSmartPtr<U>( dynamic_cast<U*>( m_objPtr ) );
	}
	// compare
	template <typename U> inline bool operator == ( const btIntrusiveSmartPtr<U>& Ptr1 ) const
	{
		return m_objPtr == Ptr1.GetInternalPtr();
	}
	template <typename U> inline bool operator == ( const U* Ptr1 ) const
	{
		return m_objPtr == Ptr1;
	}
	template <typename U> inline bool operator != ( const btIntrusiveSmartPtr<U>& Ptr1 ) const
	{
		return m_objPtr != Ptr1.GetInternalPtr();
	}
	// helper
	inline T* GetInternalPtr(void) const
	{
		return m_objPtr;
	}
private:
	T*    m_objPtr;
};

#endif
