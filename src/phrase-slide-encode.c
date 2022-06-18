/*    Re-Merge
**    Block merger for recursive pairing.
**
**    Copyright (C) 2003-2022 by Raymond Wan, All rights reserved.
**    Contact:  rwan.work@gmail.com
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

#include <stdlib.h>
#include <stdio.h>

#include "common-def.h"
#include "remerge-defn.h"
#include "remerge.h"
#include "phrase-slide-encode.h"

/*  Calculates the horizontal slide for the given left and right values.  */
R_ULL_INT horizontalSlide (R_ULL_INT left, R_ULL_INT right, R_ULL_INT kp, R_ULL_INT kpp, R_ULL_INT kppSqr) {
  R_ULL_INT result;

  if (left < kpp) {
    result = (left * (kp - kpp) + right - kpp);
  }
  else {
    result = (left * kp + right - kppSqr);
  }

  return (result);
}


/*
**  Calculates the chiastic slide for the given left and right values.
*/
R_ULL_INT chiasticSlide (R_ULL_INT left, R_ULL_INT right, R_ULL_INT kp, R_ULL_INT kpp, R_ULL_INT kppSqr) {
  R_ULL_INT result;

  if (left < kpp) {
    result = ((left + left) * (kp - kpp) + kp - 1ull - right);
  }
  else {
    if (right < kpp) {
      result = ((1ull + right + right) * (kp - kpp) + left - kpp);
    }
    else {
      if (left <= right) {
        result = (left * (kp + kp - left) + kp - 1ull - right - kppSqr);
      }
      else {
        result = (right * (kp + kp - right - 2ull) + kp - 1ull + left - kppSqr);
      }
    }
 }

 return (result);
}


