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
#include "hashing.h"

/*
**  Given a pair, a hash value is returned.  Code adapted from Algorithms
**  in C (Third edition) by Robert Sedgewick
*/
R_UINT hashCode (R_UINT left, R_UINT right) {
  R_UINT acc;

  acc = (R_UINT) (((left * LEFT_HASHCODE) % TENTPHRASE_SIZE) + ((right * RIGHT_HASHCODE) % TENTPHRASE_SIZE)) % TENTPHRASE_SIZE;

  return (acc);
}


/*
**  Given a string, a hash value is returned.  Code adapted from Algorithms
**  in C (Third edition) by Robert Sedgewick
*/
R_UINT hashCodeStr (R_UINT len, R_UINT *str) {
  R_UINT acc = 0;
  R_UINT a = 127;
  R_UINT i = 0;

  for (i = 0; i < len; i++, str++) {
    acc = (a * acc + *str) % TENTPHRASE_SIZE;
  }
  return (acc);
}

