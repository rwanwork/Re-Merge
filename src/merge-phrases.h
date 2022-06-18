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

#ifndef MERGE_PHRASES_H
#define MERGE_PHRASES_H

#define DONT_CARE 0

typedef struct hash_pair {
  R_UINT left;
  R_UINT right;
  R_UINT phrase_2_num;
  struct hash_pair *next;
} HASH_PAIR;

void mergePrimitives (PROG_INFO *prog_struct, BLOCK_INFO *block_struct);
void mergePhrases (PROG_INFO *prog_struct, BLOCK_INFO *block_struct);

#endif

