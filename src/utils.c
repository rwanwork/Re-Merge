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
#include "utils.h"

/* return the binary log of x, 0 if there's an error */
R_UINT floorLog (R_UINT x) {
  R_UINT y = 0;

  while (x != 0) {
    x >>= 1;
    y++;
  }

  if (y == 0) {
    fprintf (stderr, "Unexpected error in %s, line %u.\n", __FILE__, __LINE__);
    exit (EXIT_FAILURE);
  }

  return (y - 1);
}


/*  Return the ceiling binary log of x, 0 if there is an error  */
R_UINT ceilLog (R_UINT x) {
  R_UINT y = 0;
  x--;
  while (x != 0) {
    x >>= 1;
    y++;
  }

  return (y);
}

R_UINT ceilLogULL (R_ULL_INT x) {
  R_UINT y = 0;
  x--;
  while (x != 0) {
    x >>= 1ull;
    y++;
  }

  return (y);
}

