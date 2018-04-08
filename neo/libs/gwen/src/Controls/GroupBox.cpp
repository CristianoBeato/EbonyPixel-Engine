/*
GWEN

Copyright (c) 2010 Facepunch Studios
Copyright (c) 2017 -2018 Cristiano Beato - SDL port
MIT License

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/


#include "Gwen/Controls/GroupBox.h"

using namespace Gwen;
using namespace Gwen::Controls;


GWEN_CONTROL_CONSTRUCTOR( GroupBox )
{
	// Set to true, because it's likely that our
	// children will want mouse input, and they
	// can't get it without us..
	SetMouseInputEnabled( true );
	SetTextPadding( Padding( 10, 0, 0, 0 ) );
	SetAlignment( Pos::Top | Pos::Left );
	Invalidate();
	m_InnerPanel = new Base( this );
	m_InnerPanel->Dock( Pos::Fill );
	m_InnerMargin = 6;
}

void GroupBox::Layout( Skin::Base* skin )
{
	m_InnerPanel->SetMargin( Margin( m_InnerMargin, (TextHeight()/2) + m_InnerMargin, m_InnerMargin, m_InnerMargin ));
	BaseClass::Layout( skin );
}


void GroupBox::Render( Skin::Base* skin )
{
	skin->DrawGroupBox( this, TextX(), TextHeight(), TextWidth() );
}
