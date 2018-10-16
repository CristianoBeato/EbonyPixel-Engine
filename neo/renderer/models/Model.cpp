/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
Copyright (C) 2012-2016 Robert Beckebans
Copyright (C) 2014-2016 Kot in Action Creative Artel
Copyright (C) 2017-2018 Cristiano Beato


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

#include "renderer/tr_local.h"
#include "Model_local.h"

//static model loaders
#include "loaders/Model_lwo.h"
#include "loaders/Model_ase.h"
#include "loaders/Model_ma.h"

#include "loaders/Model_liquid.h"

//animate model loaders
#include "loaders/Model_md3.h"
#include "loaders/Model_md5.h"
#include "loaders/Model_iqm.h"

//internal model/meshes
#include "internal/Model_sprite.h"
#include "internal/Model_prt.h"
#include "internal/ModelDecal.h"
#include "internal/ModelOverlay.h"
#include "internal/Model_skined.h"
#include "internal/Model_beam.h"
#include "internal\Model_static.h"
