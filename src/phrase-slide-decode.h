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

#include <limits.h>                             /*  Required for INT_MAX  */


#ifndef _PHRASE_SLIDE_H
#define _PHRASE_SLIDE_H

void setUnitPrimitives (R_UINT num_prims, OLDPAIR *phrases);
void setUnitPhrasesHorizontal (R_ULL_INT kp, R_ULL_INT kpp, R_ULL_INT kpsqr, R_ULL_INT kppsqr, R_UINT s, OLDPAIR *phrases);
void setUnitPhrasesChiastic (R_ULL_INT k1, R_ULL_INT k2, R_ULL_INT k1sqr, R_ULL_INT k2sqr, R_UINT s, OLDPAIR *phrases);


#endif


