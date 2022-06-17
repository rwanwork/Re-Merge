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

#ifndef PHASE_2_H
#define PHASE_2_H

#define TENTPHRASE_SIZE 65521ull /*  Size of Tentative Phrase hash table  */
#define LEFT_HASHCODE 131071ull
#define RIGHT_HASHCODE 262139ull

#define EMPTY_LIST UINT_MAX

void hashPhrases (PROG_INFO *prog_struct, BLOCK_INFO *block_struct);
void reEncodeSequence_Phase2a (PROG_INFO *prog_struct, BLOCK_INFO *block_struct, R_UINT block_id);

#endif

