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
#include <sys/stat.h>                                           /*  stat  */
#include <limits.h>                                         /*  UINT_MAX  */
#include <math.h>                                               /*  sqrt  */
#include <string.h>

#include "common-def.h"
#include "wmalloc.h"
#include "remerge-defn.h"
#include "phrase-slide-encode.h"
#include "sort-phrases.h"
#include "io.h"
#include "remerge.h"
#include "phase_3.h"


static R_UCHAR *bitvector;
static R_UINT bitvector_size;

static TENTATIVE_PAIR *dup_list;
static R_UINT curr_dup_list;
static R_UINT max_dup_list;

static TENTATIVE_PAIR **dup_table;
static R_UINT dup_table_size;


/*
**  Static functions
*/
static R_INT genComparison3 (const NEWPAIR3 **first, const NEWPAIR3 **second);
static R_INT unitComparison3 (const NEWPAIR3 **first, const NEWPAIR3 **second);
static R_INT tentpairComparison (const TENTATIVE_PAIR *first, const TENTATIVE_PAIR *second);
static R_UINT primeFind (R_UINT start);
static void populateHashTable (void);
static R_UINT hashFunction (R_UINT left, R_UINT right, R_UINT func_num);
static R_BOOLEAN setBit (R_UINT position);
static void insertTentativePair (PROG_INFO *prog_struct, R_UINT left, R_UINT right, R_BOOLEAN final_pass);


static R_INT genComparison3 (const NEWPAIR3 **first, const NEWPAIR3 **second) {
  return ((R_INT) (*first) -> generation - (R_INT) (*second) -> generation);
}


static R_INT unitComparison3 (const NEWPAIR3 **first, const NEWPAIR3 **second) {
  if ((*first) -> chiastic > (*second) -> chiastic) {
    return (1);
  }
  else {
    if ((*first) -> chiastic < (*second) -> chiastic) {
      return (-1);
    }
    else {
      return (0);
    }
  }
}

/*
**  Comparison function used by C library qsort function to compare
**  two tentative pairs.
*/
R_INT tentpairComparison (const TENTATIVE_PAIR *first, const TENTATIVE_PAIR *second) {
  if (first -> left == second -> left) {
    return (((R_INT) first -> right - (R_INT) second -> right));
  }
  else {
    return ((R_INT) first -> left - (R_INT) second -> left);
  }
}


static R_UINT primeFind (R_UINT start) {
  R_UINT prime = 0;
  R_UINT flag;
  R_UINT i, j;

  for (i = start;; i++) {
    flag = 1;
    for (j = 2; j <= (R_UINT) (sqrt ((float) i)); j++) {
      if (i % j == 0) {
        flag = 0;
        break;
      }
    }
    if (flag == 1) {
      prime = i;
      break;
    }
  }

  return (prime);
}


static void populateHashTable (void) {
  R_UINT i;
  R_UINT hashvalue;

  /*
  **  Process the dup_list in reverse order so that, in the hash table,
  **  they are in sorted order at each position
  */
  for (i = curr_dup_list - 1; i != UINT_MAX; i--) {
    hashvalue = (R_UINT) (((R_ULL_INT)dup_list[i].left * PRIME1 + (R_ULL_INT)dup_list[i].right * PRIME2) % (R_ULL_INT) dup_table_size);

    dup_list[i].counter = (R_CHAR) 0;
    dup_list[i].symbol = UINT_MAX;
    dup_list[i].next = NULL;

    if (dup_table[hashvalue] == NULL) {
      dup_table[hashvalue] = &dup_list[i];
    }
    else {
      dup_list[i].next = dup_table[hashvalue];
      dup_table[hashvalue] = &dup_list[i];
    }
  }

  return;
}

/*
static R_UINT hashFunction (R_UINT left, R_UINT right, R_UINT func_num) {

  R_UINT result = (R_UINT) (((R_ULL_INT)left * PRIME1 + (R_ULL_INT)right * PRIME2 + (R_ULL_INT)func_num * PRIME3) % (R_ULL_INT) (bitvector_size * 8));

  return (result);
}
*/

static R_UINT hashFunction (R_UINT left, R_UINT right, R_UINT func_num) {

  R_UINT result = 0;

  if (func_num == 1) {
    result = (R_UINT) (((R_ULL_INT)left * PRIME1 + (R_ULL_INT)right * PRIME2 + (R_ULL_INT) PRIME3) % (R_ULL_INT) (bitvector_size * 8));
  }
  else if (func_num == 2) {
    result = (R_UINT) (((R_ULL_INT)left * PRIME3 + (R_ULL_INT)right * PRIME1 + (R_ULL_INT) PRIME2) % (R_ULL_INT) (bitvector_size * 8));
  }
  else {
    result = (R_UINT) (((R_ULL_INT)left * PRIME2 + (R_ULL_INT)right * PRIME3 + (R_ULL_INT) PRIME1) % (R_ULL_INT) (bitvector_size * 8));
  }

  return (result);
}


static R_BOOLEAN setBit (R_UINT position) {
  R_UINT byte_number = position >> 3;
  R_UINT bit_number = position & 7;
  R_UINT mask = 1;
  R_UCHAR temp;

  temp = bitvector[byte_number];
  mask = mask << bit_number;

  bitvector[byte_number] = bitvector[byte_number] | mask;

  /*  Bit was already set in a previous call  */
  if (temp == bitvector[byte_number]) {
    return (R_FALSE);
  }

  /*  New bit was set  */
  return (R_TRUE);
}


static void insertTentativePair (PROG_INFO *prog_struct, R_UINT left, R_UINT right, R_BOOLEAN final_pass) {
  R_UINT i;
  R_UINT j;
  R_FLOAT max_as_float;

  if ((curr_dup_list == max_dup_list) || (final_pass == R_TRUE)) {
    qsort (&dup_list[0], (size_t) curr_dup_list, sizeof (TENTATIVE_PAIR), (int (*)(const void *, const void *))tentpairComparison);

    i = 1;
    j = 1;
    while (i < curr_dup_list) {
      if ((dup_list[i].left != dup_list[i - 1].left) ||
          (dup_list[i].right != dup_list[i - 1].right)) {
        dup_list[j].left = dup_list[i].left;
        dup_list[j].right = dup_list[i].right;
        j++;
      }
      i++;
    }
    if (prog_struct -> verbose_level == R_TRUE) {
      fprintf (stderr, "Packing duplicate list; %u - %u = %u duplicates removed\n", i, j, i - j);
    }
    curr_dup_list = j;
    if (final_pass == R_TRUE) {
      return;
    }

    max_as_float = (R_FLOAT) max_dup_list * (R_FLOAT) LIST_THRESHOLD;
    if (curr_dup_list > (R_UINT) max_as_float) {
      if (prog_struct -> verbose_level == R_TRUE) {
        fprintf (stderr, "%u > %u; enlarging from %u to %u.\n", curr_dup_list, (R_UINT) max_as_float, max_dup_list, max_dup_list << 1);
      }
      max_dup_list = max_dup_list << 1;
      dup_list = wrealloc (dup_list, max_dup_list * sizeof (TENTATIVE_PAIR));
    }
  }

  dup_list[curr_dup_list].left = left;
  dup_list[curr_dup_list].right = right;

  curr_dup_list++;

  return;
}


/*
**  First pass through sequence
*/
static void firstPass (PROG_INFO *prog_struct) {
  R_UINT true_count = 0;
  R_UINT prev = 0, curr = 0;
  R_UINT i = 0;
  R_UINT bytes_read;
  R_UINT prevprev = UINT_MAX;
  R_UINT prevprevflag = UINT_MAX;
  R_UINT prevflag = 0;
  R_UINT flag = 0;

  if (prog_struct -> verbose_level == R_TRUE) {
    fprintf (stderr, "Phase 3:  first pass\n");
  }
  GETSYMBOL (prev, flag);
  i++;
  while (R_TRUE) {
    GETSYMBOL (curr, flag);
    i++;
    if (curr == UINT_MAX) {
      break;
    }
    /*  Consecutive pairs  */
    if ((prev == curr) && (prevprev == prev)) {
      prevprev = UINT_MAX;
      prevprevflag = UINT_MAX;
    }
    else if ((prevflag == PUNC_FLAG) && (flag == WORD_FLAG)) {
    }
    else if ((prevprevflag == WORD_FLAG) && (prevflag == PUNC_FLAG) &&
	     (flag == PUNC_FLAG)) {
    }
    else {
      true_count = 0;
      true_count += setBit (hashFunction (prev, curr, 1));
      true_count += setBit (hashFunction (prev, curr, 2));
      true_count += setBit (hashFunction (prev, curr, 3));
      if (true_count == 0) {
        insertTentativePair (prog_struct, prev, curr, R_FALSE);
      }
    }
    prevprev = prev;
    prevprevflag = prevflag;
    prevflag = flag;
    prev = curr;
  }

  insertTentativePair (prog_struct, prev, curr, R_TRUE);

  wfree (bitvector);

  dup_table_size = primeFind (max_dup_list << 2);
  dup_table = wmalloc (dup_table_size * sizeof (TENTATIVE_PAIR*));
  for (i = 0; i < dup_table_size; i++) {
    dup_table[i] = NULL;
  }
  populateHashTable ();

  return;
}

/*
**  Second pass through sequence
*/
static R_UINT secondPass (PROG_INFO *prog_struct,  BLOCK_INFO *block_struct) {
  R_UINT curr, prev;
  R_UINT hashvalue;
  R_UINT i;
  R_UINT bytes_read;
  TENTATIVE_PAIR *currnode;
  TENTATIVE_PAIR *prevnode;
  R_UINT symbol_count = block_struct -> pprime;
  R_UINT true_match = 0;
  R_UINT false_match = 0;
  R_UINT prevprev = UINT_MAX;
  R_UINT prevprevflag = UINT_MAX;
  R_UINT prevflag = 0;
  R_UINT flag = 0;
  R_BOOLEAN just_paired = R_FALSE;

  if (prog_struct -> verbose_level == R_TRUE) {
    fprintf (stderr, "Phase 3:  second pass\n");
  }
  rewind (prog_struct -> in_seq);
  prog_struct -> in_buf_end = prog_struct -> in_buf + SEQ_BUF_SIZE;
  prog_struct -> in_buf_p = prog_struct -> in_buf_end;

  GETSYMBOL (prev, flag);
  while (R_TRUE) {
    GETSYMBOL (curr, flag);
    if (curr == UINT_MAX) {
      break;
    }

    /*  Consecutive pairs  */
    if (just_paired == R_TRUE) {
      just_paired = R_FALSE;
    }
    else if ((prev == curr) && (prevprev == prev)) {
      prevprev = UINT_MAX;
      prevprevflag = UINT_MAX;
    }
    else if ((prevflag == PUNC_FLAG) && (flag == WORD_FLAG)) {
    }
    else if ((prevprevflag == WORD_FLAG) && (prevflag == PUNC_FLAG) &&
	     (flag == PUNC_FLAG)) {
    }
    else {
      hashvalue = (R_UINT) (((R_ULL_INT)prev * PRIME1 + (R_ULL_INT)curr * PRIME2) % (R_ULL_INT) dup_table_size);

      currnode = dup_table[hashvalue];
      while (currnode != NULL) {
        if ((prev == currnode -> left) && (curr == currnode -> right) && (currnode -> counter < SCHAR_MAX)) {
          currnode -> counter = currnode -> counter + (R_CHAR) 1;
          just_paired = R_TRUE;
          break;
        }
        currnode = currnode -> next;
      }
    }
    prevprev = prev;
    prevprevflag = prevflag;
    prevflag = flag;
    prev = curr;
  }

  for (i = 0; i < dup_table_size; i++) {
    prevnode = NULL;
    currnode = dup_table[i];
    while (currnode != NULL) {
      if ((currnode -> counter == (R_CHAR) 0) || (currnode -> counter == (R_CHAR) 1)) {
	false_match++;
        if (prevnode == NULL) {
          dup_table[i] = currnode -> next;
	  currnode = dup_table[i];
	}
	else {
          prevnode -> next = currnode -> next;
	  currnode = prevnode -> next;
	}
      }
      else {
	currnode -> symbol = symbol_count;
        prevnode = currnode;
        currnode = currnode -> next;
	true_match++;
        symbol_count++;
      }
    }
  }

  if (prog_struct -> verbose_level == R_TRUE) {
    fprintf (stderr, "Number of true matches:  %u\n", true_match);
    fprintf (stderr, "Number of false matches:  %u\n", false_match);
  }

  return (symbol_count);
}


static R_UINT thirdPass (PROG_INFO *prog_struct, R_UINT *in_seq_count, R_UINT *out_seq_count) {
  R_UINT curr, prev;
  R_UINT hashvalue;
  R_UINT bytes_read;
  TENTATIVE_PAIR *currnode;
  R_BOOLEAN match = R_FALSE;
  R_UINT newphrase_count = 0;
  R_UINT prevprevflag = UINT_MAX;
  R_UINT prevflag = UINT_MAX;
  R_UINT currflag = UINT_MAX;

  if (prog_struct -> verbose_level == R_TRUE) {
    fprintf (stderr, "Phase 3:  third pass\n");
  }

  rewind (prog_struct -> in_seq);
  prog_struct -> in_buf_end = prog_struct -> in_buf + SEQ_BUF_SIZE;
  prog_struct -> in_buf_p = prog_struct -> in_buf_end;

  GETSYMBOL (prev, prevflag);
  (*in_seq_count)++;
  while (R_TRUE) {
    GETSYMBOL (curr, currflag);
    (*in_seq_count)++;

    if (curr == UINT_MAX) {
      PUTSYMBOL (prev, prevflag);
      PUTSYMBOL (curr, currflag);
      (*out_seq_count) += 2;
      break;
    }

    match = R_FALSE;
    if ((prevflag == PUNC_FLAG) && (currflag == WORD_FLAG)) {
    }
    else if ((prevprevflag == WORD_FLAG) && (prevflag == PUNC_FLAG) &&
	     (currflag == PUNC_FLAG)) {
    }
    else {
      hashvalue = (R_UINT) (((R_ULL_INT)prev * PRIME1 + (R_ULL_INT)curr * PRIME2) % (R_ULL_INT) dup_table_size);

      currnode = dup_table[hashvalue];
      while (currnode != NULL) {
        if ((prev == currnode -> left) && (curr == currnode -> right)) {
          prev = currnode -> symbol;
	  if (currflag == PUNC_FLAG) {
	    prevflag = PUNC_FLAG;
	  }
	  if (currnode -> counter > (R_CHAR) 0) {
            currnode -> counter = currnode -> counter * (R_CHAR) -1;
	    newphrase_count++;
	  }
	  match = R_TRUE;
          break;
        }
        currnode = currnode -> next;
      }
    }
    PUTSYMBOL (prev, prevflag);
    (*out_seq_count)++;
    if (match == R_TRUE) {
      GETSYMBOL (curr, currflag);
      (*in_seq_count)++;
      if (curr == UINT_MAX) {
        PUTSYMBOL (curr, currflag);
        (*out_seq_count) += 2;
        break;
      }
    }

    prevprevflag = prevflag;
    prevflag = currflag;
    prev = curr;
  }

  return (newphrase_count);
}

/*
**  block_struct -> pprime - initial number of primitives and phrases
**  symbol_count - *final* size of block_struct -> target_prel 
**                 -> target_phrases; duplicates during Phase 3 are
**                 included
**  newphrase_count - *final* size of gen_unit_sort; duplicates 
**                    removed during pass 3 of Phase 3
*/
void reEncodeSequence_Phase3 (PROG_INFO *prog_struct, BLOCK_INFO *block_struct, R_UINT block_id) {
  struct stat stbuf;
  R_UINT i, k;
  R_UINT symbol_count;
  R_UINT filesize;
  R_UINT bytes_read;
  TENTATIVE_PAIR *currnode;
  R_UINT newphrase_count;

  OLDPAIR *source_phrases;
  R_UINT *in_seq_count;
  R_UINT *out_seq_count = &(block_struct -> sprime);

  NEWBLOCK *target_block = block_struct -> target_prel;
  NEWPAIR3 *target_phrases = NULL;
  R_UINT target_left;
  R_UINT target_right;

  SIZENODE *curr_sizenode = NULL;
  SIZENODE *prev_sizenode = NULL;
  R_UINT current_generation = 0;
  NEWPAIR3 **gen_unit_sort = NULL;
  NEWPAIR3 **new_gen_unit_sort = NULL;

  R_UINT x;
  R_UINT y;

  R_CHAR *move_cmd = NULL;
  R_CHAR *delete_cmd = NULL;

  R_INT system_result;
  R_UINT flag = 0;

  if (block_id == 1) {
    source_phrases = block_struct -> source_prel_1 -> phrases;
    in_seq_count = &(block_struct -> s1);
  }
  else {
    source_phrases = block_struct -> source_prel_2 -> phrases;
    in_seq_count = &(block_struct -> s2);
  }

  (void) stat (prog_struct -> old_seq_name, &stbuf);
  filesize = (R_UINT) stbuf.st_size;
  filesize = filesize / 4;

  bitvector_size = primeFind (filesize);
  bitvector = wmalloc (sizeof (R_UCHAR) * bitvector_size);
  bitvector = memset (bitvector, 0, (size_t) bitvector_size);

  curr_dup_list = 0;
  max_dup_list = (R_UINT) INIT_DUP_LIST_SIZE;
  dup_list = wmalloc (max_dup_list * sizeof (TENTATIVE_PAIR));

  /*
  **  Perform the three passes of Phase 3
  */
  firstPass (prog_struct);
  symbol_count = secondPass (prog_struct, block_struct);
  newphrase_count = thirdPass (prog_struct, in_seq_count, out_seq_count);

  if (prog_struct -> out_buf_p != prog_struct -> out_buf) {
    (void) fwrite (prog_struct -> out_buf, sizeof (*prog_struct -> out_buf_p), (size_t) (prog_struct -> out_buf_p - prog_struct -> out_buf), prog_struct -> out_seq);
  }
  FCLOSE (prog_struct -> in_seq);
  FCLOSE (prog_struct -> out_seq);

  target_block -> phrases3 = wrealloc (target_block -> phrases3, symbol_count * sizeof (NEWPAIR3));
  target_phrases = target_block -> phrases3;

  for (i = block_struct -> pprime; i < symbol_count; i++) {
    target_phrases[i].me = UINT_MAX;
    target_phrases[i].new_me = UINT_MAX;
    target_phrases[i].generation = UINT_MAX;
  }

  target_block -> gen_unit_sort3 = wrealloc (target_block -> gen_unit_sort3, (block_struct -> pprime + newphrase_count) * sizeof (NEWPAIR3*));
  gen_unit_sort = target_block -> gen_unit_sort3;

  for (i = 0; i < block_struct -> pprime; i++) {
    gen_unit_sort[target_phrases[i].new_me] = &target_phrases[i];
  }

  /*
  **  i index into dup_table
  **  k index into gen_unit_sort
  **
  */
  for (i = 0; i < dup_table_size; i++) {
    currnode = dup_table[i];
    while (currnode != NULL) {
      target_phrases[currnode -> symbol].me = currnode -> symbol;
      target_phrases[currnode -> symbol].new_me = UINT_MAX;
      target_phrases[currnode -> symbol].left_child = currnode -> left;
      target_phrases[currnode -> symbol].right_child = currnode -> right;
      target_phrases[currnode -> symbol].generation = UINT_MAX;

      if (currnode -> counter < (R_CHAR) 0) {
        if (gen_unit_sort[currnode -> left] -> generation > gen_unit_sort[currnode -> right] -> generation) {
          target_phrases[currnode -> symbol].generation = (gen_unit_sort[currnode -> left] -> generation) + 1;
        }
        else {
	  target_phrases[currnode -> symbol].generation = (gen_unit_sort[currnode -> right] -> generation) + 1;
        }
      }

      currnode = currnode -> next;
    }
  }

  wfree (dup_table);
  wfree (dup_list);

  new_gen_unit_sort = wmalloc ((block_struct -> pprime + newphrase_count) * sizeof (NEWPAIR3*));

  /*  Assign new_gen_unit_sort, with duplicates removed  */
  k = 0;
  for (i = 0; i < symbol_count; i++) {
    if (target_phrases[i].generation != UINT_MAX) {
      new_gen_unit_sort[k] = &target_phrases[i];
      k++;
    }
  }

  /*  Free all size nodes  */
  curr_sizenode = target_block -> sizelist;
  while (curr_sizenode != NULL) {
    prev_sizenode = curr_sizenode;
    curr_sizenode = curr_sizenode -> next;
    wfree (prev_sizenode);
  }
  target_block -> sizelist = NULL;

  /*  Sort phrases on generations  */
  qsort (&new_gen_unit_sort[0], (size_t) (block_struct -> pprime + newphrase_count), sizeof (NEWPAIR3*), (int (*)(const void *, const void *))genComparison3);

  /*  Handle the primitives first  */
  target_left = 0;
  target_right = target_block -> num_primitives;
  for (i = target_left; i < target_block -> num_primitives; i++) {
    new_gen_unit_sort[i] -> new_me = i;
  }

  target_block -> kp = block_struct -> target_prel -> num_primitives;
  target_block -> kpp = 0;
  target_block -> kppSqr = 0;

  insertSizeNode (target_block, target_right - target_left);
  target_left = target_right;

  /*  Now, handle the phrases  */
  while (target_right < (block_struct -> pprime + newphrase_count)) {
    current_generation++;
    while ((target_right < (block_struct -> pprime + newphrase_count)) && (new_gen_unit_sort[target_right] -> generation == current_generation)) {
      new_gen_unit_sort[target_right] -> left_child = target_block -> phrases3[new_gen_unit_sort[target_right] -> left_child].new_me;
      new_gen_unit_sort[target_right] -> right_child = target_block -> phrases3[new_gen_unit_sort[target_right] -> right_child].new_me;
      new_gen_unit_sort[target_right] -> chiastic = chiasticSlide (new_gen_unit_sort[target_right] -> left_child, new_gen_unit_sort[target_right] -> right_child, target_block -> kp, target_block -> kpp, target_block -> kppSqr);

      target_right++;
    }

    if (target_right == target_left) {
      fprintf (stderr, "Empty current_generation = %u.\n", current_generation);
      exit (EXIT_FAILURE);
    }

    insertSizeNode (target_block, target_right - target_left);

    /*  Quicksort phrases in current generation on unit  */
    qsort (&new_gen_unit_sort[target_left], (size_t) target_right - target_left, sizeof (NEWPAIR3*), (int (*)(const void *, const void *))unitComparison3);
    for (i = target_left; i < target_right; i++) {
      new_gen_unit_sort[i] -> new_me = i;
    }

    target_block -> kpp = target_block -> kp;
    target_block -> kppSqr = (target_block -> kpp) * (target_block -> kpp);
    target_block -> kp = target_block -> kp + (target_right - target_left);
    target_left = target_right;
  }

  delete_cmd = wmalloc (10 + strlen (prog_struct -> old_seq_name));
  (void) snprintf (delete_cmd, (size_t) (10 + strlen (prog_struct -> old_seq_name)), "rm -f -r %s", prog_struct -> old_seq_name);
  system_result = system (delete_cmd);
  wfree (delete_cmd);

  move_cmd = wmalloc (5 + strlen (prog_struct -> new_seq_name) + strlen (prog_struct -> old_seq_name));
  (void) snprintf (move_cmd, (size_t) (5 + strlen (prog_struct -> new_seq_name) + strlen (prog_struct -> old_seq_name)), "mv %s %s", prog_struct -> new_seq_name, prog_struct -> old_seq_name);
  system_result = system (move_cmd);
  wfree (move_cmd);

  prog_struct -> in_seq = fopen (prog_struct -> old_seq_name, "r");
  prog_struct -> out_seq = fopen (prog_struct -> new_seq_name, "w");

  prog_struct -> in_buf_end = prog_struct -> in_buf + SEQ_BUF_SIZE;
  prog_struct -> in_buf_p = prog_struct -> in_buf_end;

  prog_struct -> out_buf_end = prog_struct -> out_buf + SEQ_BUF_SIZE;
  prog_struct -> out_buf_p = prog_struct -> out_buf;

  /*  Renumber sequence (one-to-one)  */
  if (prog_struct -> verbose_level == R_TRUE) {
    fprintf (stderr, "Phase 3:  fourth pass\n");
  }
  while (R_TRUE) {
    /*  Check if input buffer is empty  */
    GETSYMBOL (x, flag);
    if (x == UINT_MAX) {
      y = UINT_MAX;
      PUTSYMBOL (y, flag);      
      break;
    }
    /*  If x >= pprime, then this phrase is a new phrase  */
    if (x >= block_struct -> pprime) {
      y = target_phrases[x].new_me;
    }
    else {
      y = gen_unit_sort[x] -> new_me;
    }
    PUTSYMBOL (y, flag);
  }

  wfree (target_block -> gen_unit_sort3);
  target_block -> gen_unit_sort3 = new_gen_unit_sort;

  target_block -> num_phrases += newphrase_count;
  block_struct -> pprime += newphrase_count;
  target_block -> num_generation = current_generation;

  /*  Must transfer here because only this function knows the
  **  true size of phrases3 (symbol_count)  */
  transferNewPair3_1 (prog_struct, block_struct, symbol_count);

  return;
}
