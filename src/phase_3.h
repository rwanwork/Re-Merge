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

#ifndef PHASE_3_H
#define PHASE_3_H

#define PRIME1 536870909ull
#define PRIME2 1073741789ull
#define PRIME3 2147483647ull

#define INIT_DUP_LIST_SIZE (1 << 10)
#define LIST_THRESHOLD 0.75

typedef struct tentative_pair {
  R_UINT left;
  R_UINT right;
  R_CHAR counter;
  R_UINT symbol;
  struct tentative_pair *next;
} TENTATIVE_PAIR;


#define GETSYMBOL(SYM,SYMFLAG) \
  if (prog_struct -> in_buf_p == prog_struct -> in_buf_end) { \
    if (feof (prog_struct -> in_seq) == R_TRUE) { \
      fprintf (stderr, "ERROR:  Unexpected EOF. %s: %d.\n", __FILE__, __LINE__); \
      exit (EXIT_FAILURE); \
    } \
    bytes_read = readBuffer (prog_struct); \
  } \
  SYM = (*(prog_struct -> in_buf_p)) - 1; \
  SYMFLAG = (*(prog_struct -> pflag_buf_p)); \
  (prog_struct -> pflag_buf_p)++; \
  (prog_struct -> in_buf_p)++;


#define PUTSYMBOL(SYM,SYMFLAG) \
  *(prog_struct -> out_buf_p) = SYM + 1; \
  *(prog_struct -> out_buf_p) = *(prog_struct -> out_buf_p) | SYMFLAG; \
  prog_struct -> out_buf_p++; \
  if (prog_struct -> out_buf_p == prog_struct -> out_buf_end) { \
    bytes_read = writeBuffer (prog_struct, (R_FLOAT) 1); \
  }


void reEncodeSequence_Phase3 (PROG_INFO *prog_struct, BLOCK_INFO *block_struct, R_UINT block_id);

#endif

