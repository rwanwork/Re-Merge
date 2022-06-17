/*    Re-Merge
**    Block merger for recursive pairing.
**    Copyright (C) 2003, 2007 by Raymond Wan (rwan@kuicr.kyoto-u.ac.jp)
**
**    Version 1.0 -- 2007/04/02
**
**    This file is part of the Re-Merge software.
**
**    Re-Merge is free software; you can redistribute it and/or modify
**    it under the terms of the GNU General Public License as published by
**    the Free Software Foundation; either version 2 of the License, or
**    (at your option) any later version.
**
**    Re-Merge is distributed in the hope that it will be useful,
**    but WITHOUT ANY WARRANTY; without even the implied warranty of
**    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**    GNU General Public License for more details.
**
**    You should have received a copy of the GNU General Public License along
**    with Re-Merge; if not, write to the Free Software Foundation, Inc.,
**    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#ifndef PHRASE_SLIDE_ENCODE_H
#define PHRASE_SLIDE_ENCODE_H

R_ULL_INT horizontalSlide (R_ULL_INT left, R_ULL_INT right, R_ULL_INT kp, R_ULL_INT kpp, R_ULL_INT kppSqr);
R_ULL_INT chiasticSlide (R_ULL_INT left, R_ULL_INT right, R_ULL_INT kp, R_ULL_INT kpp, R_ULL_INT kppSqr);

/*
**  Used for testing

R_INT mapNodeComparison (const MAPNODE *first, const MAPNODE *second);
*/

#endif


