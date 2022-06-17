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

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>                                         /*  UINT_MAX  */

#include "common-def.h"
#include "wmalloc.h"
#include "remerge-defn.h"
#include "remerge.h"
#include "utils.h"
#include "io.h"
#include "bitout.h"
#include "phase_1.h"

/*
**  Static functions
*/
static void intEncodeHierarchy (FILE *out_prel, R_UINT a, R_UINT b, R_ULL_INT lo, R_ULL_INT hi, NEWPAIR1 *target_phrases[]);

static void intEncodeHierarchy (FILE *out_prel, R_UINT a, R_UINT b, R_ULL_INT lo, R_ULL_INT hi, NEWPAIR1 *target_phrases[]) {
  R_ULL_INT mid;
  R_UINT halfway, range;

  range = b - a;

  switch (range) {
    case 0:  return;
    case 1:  binaryEncode (out_prel, target_phrases[a] -> chiastic, lo, hi);
             return;
  }
  halfway = range >> 1;  
                       /*  Divide range by 2, getting the halfway point  */

  mid = target_phrases[a + halfway] -> chiastic;
                               /*  Obtain the unit of the halfway point  */

  binaryEncode (out_prel, mid, lo + (R_ULL_INT) halfway, hi - (R_ULL_INT) (range - halfway - 1));
                                        /*  Binary encode the mid-point  */
  intEncodeHierarchy (out_prel, a, a + halfway, lo, mid, target_phrases);
                                   /*  Recursively encode the left half  */
  intEncodeHierarchy (out_prel, a + halfway + 1, b, mid + 1ull, hi, target_phrases);
                                  /*  Recursively encode the right half  */
  return;
}


void encodeHierarchy_OneBlock (PROG_INFO *prog_struct, BLOCK_INFO *block_struct) {
  R_ULL_INT kpp = 0ull;
  R_ULL_INT kppSqr = 0ull;
  R_ULL_INT kp = 0ull;
  R_ULL_INT kpSqr;
  R_UINT max;
  R_UINT logRange;
  R_UINT topRange;
  R_UINT currentsize;
  SIZENODE *sizes = (block_struct -> target_prel) -> sizelist;
  SIZENODE *back_sizes = NULL;
  NEWPAIR1 **target_phrases = (block_struct -> target_prel) -> gen_unit_sort1;
  R_UINT curr_gen = 0;

  R_UINT num_prims = sizes -> value;
  R_UINT num_phrases = (block_struct -> target_prel) -> num_phrases;

  deltaEncode (prog_struct -> out_prel, num_prims + num_phrases, 0);
            /*  Delta encode the total number of primitives and phrases  */

  deltaEncode (prog_struct -> out_prel, num_prims, 1);
                        /*  Delta encode the total number of primitives  */

  max = (R_UINT) target_phrases[num_prims - 1] -> chiastic;
                  /*  Get the maximum ASCII value of all the primitives  */
  logRange = (R_UINT) ceilLog (max + 1);
  topRange = 1u << logRange;
  gammaEncode (prog_struct -> out_prel, logRange, 0);
                    /*  Calculate and gamma encode the log of the range  */

#ifdef DEBUG
  fprintf (stderr, "Generation %u:  %u\n", curr_gen, num_prims);
#endif

  intEncodeHierarchy (prog_struct -> out_prel, 0, num_prims, 0, topRange, target_phrases);
                                              /*  Encode the primitives  */

  kp = num_prims;
  sizes = sizes -> next;  
                    /*  Get the node of the first generation of phrases  */

  back_sizes = sizes;
  while (kp < (num_prims + num_phrases)) {
    curr_gen++;
    currentsize = sizes -> value;
#ifdef DEBUG
    fprintf (stderr, "Generation %u:  %u\n", curr_gen, currentsize);
#endif
    gammaEncode (prog_struct -> out_prel, currentsize, 1);
                            /*  Gamma encode the size of the generation  */
    kpSqr = kp * kp;

    intEncodeHierarchy (prog_struct -> out_prel, (R_UINT) kp, (R_UINT) (kp + currentsize), 0, kpSqr - kppSqr, target_phrases);
 	                           /*  Encode the generation of phrases  */

    kpp = kp;
    kppSqr = kpSqr;
    kp += currentsize;
    back_sizes = sizes;
    sizes = sizes -> next;
    wfree (back_sizes);
  }

  /*  Remove the node with the size of primitives  */
  wfree ((block_struct -> target_prel) -> sizelist);
  (block_struct -> target_prel) -> sizelist = NULL;

  if (kp > (num_prims + num_phrases)) {
    fprintf (stderr, "Wrong generation sizes. %u %u %u %u\n", (R_UINT) kp, num_prims, num_phrases, num_prims + num_phrases);
  }

  return;
}


/*
**  Write an End of File marker in the prelude file and flush the
**  buffer.
*/
void flushBuffer (PROG_INFO *prog_struct) {
  writeBits (prog_struct -> out_prel, 1, 1, R_FALSE);
  writeBits (prog_struct -> out_prel, 0, 0, R_FALSE);
  writeBits (prog_struct -> out_prel, 0, 0, R_FALSE);
  writeBits (prog_struct -> out_prel, 0, 0, R_TRUE);     /*  Flush buffers  */
}


/*
**  Functions for manipulating the sequence
*/

void reEncodeSequence_Phase1 (PROG_INFO *prog_struct, BLOCK_INFO *block_struct, R_UINT block_id) {
  R_UINT x;
  R_UINT y;
  R_UINT z;
  R_UINT temp_y;
  R_UINT bytes_read;

  NEWPAIR1 *target_phrases = (block_struct -> target_prel) -> phrases1;
  OLDPAIR *source_phrases;
  R_UINT *in_seq_count;
  R_UINT *out_seq_count = &(block_struct -> sprime);

  if (block_id == 1) {
    source_phrases = block_struct -> source_prel_1 -> phrases;
    in_seq_count = &(block_struct -> s1);
  }
  else {
    source_phrases = block_struct -> source_prel_2 -> phrases;
    in_seq_count = &(block_struct -> s2);
    if (block_struct -> p2 == 0) {
      y = 0;
      *(prog_struct -> out_buf_p) = y;
      prog_struct -> out_buf_p++;
      (*out_seq_count)++;

      if (prog_struct -> out_buf_p == prog_struct -> out_buf_end) {
	bytes_read = writeBuffer (prog_struct, (R_FLOAT) 1);
      }

      return;
    }
  }

  while (R_TRUE) {
    if (prog_struct -> in_buf_p == prog_struct -> in_buf_end) {

      if (feof (prog_struct -> in_seq) == R_TRUE) {
	fprintf (stderr, "ERROR:  Unexpected EOF. %s: %u.\n", __FILE__, __LINE__);
        exit (EXIT_FAILURE);
      }

      bytes_read = readBuffer (prog_struct);
    }

    if (prog_struct -> out_buf_p == prog_struct -> out_buf_end) {
      bytes_read = writeBuffer (prog_struct, (R_FLOAT) 1);
    }

    x = (*(prog_struct -> in_buf_p)) - 1;
    z = (*(prog_struct -> pflag_buf_p));
    (prog_struct -> in_buf_p)++;
    (prog_struct -> pflag_buf_p)++;
    (*in_seq_count)++;

    /*  Loop exits when x = UINT_MAX  */
    if (x == UINT_MAX) {

      if (block_id == 2) {
        y = 0;
        *(prog_struct -> out_buf_p) = y;
        prog_struct -> out_buf_p++;
        (*out_seq_count)++;
      }

      if (prog_struct -> out_buf_p == prog_struct -> out_buf_end) {
        bytes_read = writeBuffer (prog_struct, (R_FLOAT) 1);
      }

      break;
    }

    temp_y = target_phrases[source_phrases[x].new_prel].replacement_phrase;
    y = target_phrases[temp_y].new_me;

    (*(prog_struct -> out_buf_p)) = ((R_UINT) (y + 1)) | z;
    (prog_struct -> out_buf_p)++;
    (*out_seq_count)++;
  }

  return;
}

