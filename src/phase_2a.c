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

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>                                         /*  UINT_MAX  */

#include "common-def.h"
#include "wmalloc.h"
#include "remerge-defn.h"
#include "remerge.h"
#include "io.h"
#include "hashing.h"
#include "phase_2a.h"


void hashPhrases (PROG_INFO *prog_struct, BLOCK_INFO *block_struct) {
  R_UINT i;
  R_UINT hashvalue;
  NEWPAIR1 **gen_unit_sort = (block_struct -> target_prel) -> gen_unit_sort1;
  R_UINT prev;
  R_UINT curr;

  (block_struct -> target_prel) -> target_prel_hashtable = wmalloc (((R_UINT) TENTPHRASE_SIZE) * sizeof (R_UINT));
  for (i = 0; (R_ULL_INT) i < TENTPHRASE_SIZE; i++) {
    (block_struct -> target_prel) -> target_prel_hashtable[i] = EMPTY_LIST;
  }

  for (i = 0; i < (block_struct -> target_prel) -> num_primitives; i++) {
    gen_unit_sort[i] -> first_prel = EMPTY_LIST;
    gen_unit_sort[i] -> second_prel = 0;
    gen_unit_sort[i] -> new_me = i;
  }

  /*
  **  For each phrase (and not primitive) in the new prelude:
  */
  for (i = (block_struct -> target_prel) -> num_primitives; i < ((block_struct -> target_prel) -> num_primitives + (block_struct -> target_prel -> num_phrases)); i++) {
    gen_unit_sort[i] -> first_prel = EMPTY_LIST;
    gen_unit_sort[i] -> second_prel = 0;
    gen_unit_sort[i] -> new_me = i;
    hashvalue = hashCode (gen_unit_sort[i] -> left_child, gen_unit_sort[i] -> right_child);
    if ((block_struct -> target_prel) -> target_prel_hashtable[hashvalue] == EMPTY_LIST) {
      (block_struct -> target_prel) -> target_prel_hashtable[hashvalue] = i;
    }
    else {
      curr = (block_struct -> target_prel) -> target_prel_hashtable[hashvalue];
      prev = EMPTY_LIST;

      while ((curr != EMPTY_LIST) && (gen_unit_sort[i] -> left_child < gen_unit_sort[curr] -> left_child)) {
	prev = curr;
	curr = gen_unit_sort[curr] -> first_prel;
      }

      if (prev == EMPTY_LIST) {
        gen_unit_sort[i] -> first_prel = curr;
	(block_struct -> target_prel) -> target_prel_hashtable[hashvalue] = i;
      }
      else {
	gen_unit_sort[i] -> first_prel = curr;
	gen_unit_sort[prev] -> first_prel = i;
      }
    }

    /*  Update child counters to indicate number of parents  */
    gen_unit_sort[gen_unit_sort[i] -> left_child] -> second_prel++;
    gen_unit_sort[gen_unit_sort[i] -> right_child] -> second_prel++;
  }

  return;
}


void reEncodeSequence_Phase2a (PROG_INFO *prog_struct, BLOCK_INFO *block_struct, R_UINT block_id) {
  R_UINT x;
  R_UINT y;
  R_UINT temp_y;
  R_UINT bytes_read;
  NEWPAIR1 **gen_unit_sort = (block_struct -> target_prel) -> gen_unit_sort1;
  NEWPAIR1 *target_phrases = (block_struct -> target_prel) -> phrases1;

  R_UINT prev_flag = WORD_FLAG;
  R_UINT left;
  R_UINT left_flag;
  R_UINT right;
  R_UINT right_flag;
  R_UINT curr;
  R_UINT hashvalue;
  R_UINT replace_count = 0;
  R_UINT old_replace_count = 0;

  OLDPAIR *source_phrases;
  R_UINT *in_seq_count;
  R_UINT *out_seq_count = &(block_struct -> sprime);

  R_UINT z = 0;

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
	fprintf (stderr, "ERROR:  Unexpected EOF. %s: %d.\n", __FILE__, __LINE__);
        exit (EXIT_FAILURE);
      }

      bytes_read = readBuffer (prog_struct);
    }

    if (prog_struct -> out_buf_p == prog_struct -> out_buf_end) {
      bytes_read = writeBuffer (prog_struct, (R_FLOAT) 0.5);
    }

    x = (*(prog_struct -> in_buf_p)) - 1;
    z = (*(prog_struct -> pflag_buf_p));
    (prog_struct -> in_buf_p)++;
    (prog_struct -> pflag_buf_p)++;
    (*in_seq_count)++;

    /*  Loop exits when x = UINT_MAX  */
    if (x == UINT_MAX) {
      y = 0;
      *(prog_struct -> out_buf_p) = y;
      prog_struct -> out_buf_p++;
      (*out_seq_count)++;

      if (prog_struct -> out_buf_p == prog_struct -> out_buf_end) {
	(void) writeBuffer (prog_struct, (R_FLOAT) 1);
      }
      break;
    }

    temp_y = target_phrases[source_phrases[x].new_prel].replacement_phrase;
    y = target_phrases[temp_y].new_me;

    (*(prog_struct -> out_buf_p)) = ((R_UINT) (y + 1)) | z;
    (prog_struct -> out_buf_p)++;
    (*out_seq_count)++;

    /*  At least 2 symbols in the sequence buffer  */
    while ((prog_struct -> out_buf_p != prog_struct -> out_buf) &&
           (prog_struct -> out_buf_p != prog_struct -> out_buf + 1)) {

      left = ((*(prog_struct -> out_buf_p - 2)) & NO_FLAGS) - 1;
      right = ((*(prog_struct -> out_buf_p - 1)) & NO_FLAGS) - 1;

      left_flag = (*(prog_struct -> out_buf_p - 2)) & PUNC_FLAG;
      right_flag = (*(prog_struct -> out_buf_p - 1)) & PUNC_FLAG;

      /*  Punctuation flags prevents pairing  */
      if ((left_flag == PUNC_FLAG) && (right_flag == WORD_FLAG)) {
	break;
      }

      /*  Punctuation-aligned Re-Pair  */
      if (prog_struct -> word_flags == UW_YES) {
	/*  At least 3 symbols in the sequence buffer  */
	if ((left_flag == PUNC_FLAG) && (right_flag == PUNC_FLAG)) {
  	  if (prog_struct -> out_buf_p != prog_struct -> out_buf + 2) {
            prev_flag = (*(prog_struct -> out_buf_p - 3)) & PUNC_FLAG;
	    /*  No pairing if prev_flag is a WORD_FLAG  */
            if (prev_flag == WORD_FLAG) {
	      break;
	    }
	  }
	}
      }

      if ((gen_unit_sort[left] -> second_prel != 0) && (gen_unit_sort[right] -> second_prel != 0)) {
        hashvalue = hashCode (left, right);
        curr = (block_struct -> target_prel) -> target_prel_hashtable[hashvalue];
	old_replace_count = replace_count;
	while ((curr != EMPTY_LIST) && (left <= gen_unit_sort[curr] -> left_child)) {
          if ((gen_unit_sort[curr] -> left_child == left) && (gen_unit_sort[curr] -> right_child == right)) {
            *(prog_struct -> out_buf_p - 2) = (gen_unit_sort[curr] -> new_me + 1) | right_flag;
            (prog_struct -> out_buf_p)--;
	    (*out_seq_count)--;
	    replace_count++;
            break;
	  }
          curr = gen_unit_sort[curr] -> first_prel;
        }
	/*  Leave loop if no replacement was made; remain in loop if
	**  a replacement was made.  */
	if (replace_count == old_replace_count) {
	  break;
	}
      }
      else {
	break;
      }
    }
  }

  return;
}

