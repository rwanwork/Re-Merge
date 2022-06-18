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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>                                       /*  abs function  */
#include <limits.h>                                         /*  UINT_MAX  */
#include <string.h>

#include "common-def.h"
#include "wmalloc.h"
#include "remerge-defn.h"
#include "phrase-slide-encode.h"
#include "sort-phrases.h"

/*
**  Static functions
*/

static R_UINT getPhraseLength (BLOCK_INFO *block_struct, R_UINT pos);
static void expandPhrase (NEWPAIR1 *phrases, R_UINT pos, R_UINT *str);
static void expandAllPhrases (PROG_INFO *prog_struct, BLOCK_INFO *block_struct);
static R_INT unitComparison (const NEWPAIR1 **first, const NEWPAIR1 **second);
static R_INT genComparison (const NEWPAIR1 **first, const NEWPAIR1 **second);
static R_INT forwardStrComparison (const NEWPAIR1 **first, const NEWPAIR1 **second);

static R_UINT getPhraseLength (BLOCK_INFO *block_struct, R_UINT pos) {
  R_UINT left_len = 0;
  R_UINT right_len = 0;
  NEWPAIR1 *phrases1 = block_struct -> target_prel -> phrases1;

  left_len = phrases1[phrases1[pos].left_child].len;

  if (left_len == 0) {
    left_len = getPhraseLength (block_struct, phrases1[pos].left_child);
    phrases1[phrases1[pos].left_child].len = left_len;
  }

  right_len = phrases1[phrases1[pos].right_child].len;
  if (right_len == 0) {
    right_len = getPhraseLength (block_struct, phrases1[pos].right_child);
    phrases1[phrases1[pos].right_child].len = right_len;
  }

  return (left_len + right_len);
}


static void expandPhrase (NEWPAIR1 *phrases1, R_UINT pos, R_UINT *str) {

  if (phrases1[pos].str != NULL) {
    memcpy (str, phrases1[pos].str, sizeof (R_UINT) * phrases1[pos].len);
  }
  else {
    expandPhrase (phrases1, phrases1[pos].left_child, str);
    expandPhrase (phrases1, phrases1[pos].right_child, str + phrases1[phrases1[pos].left_child].len);
    phrases1[pos].str = str;
  }

  return;
}


static void expandAllPhrases (PROG_INFO *prog_struct, BLOCK_INFO *block_struct) {
  R_UINT i = 0;
  NEWPAIR1 *phrases1 = block_struct -> target_prel -> phrases1;
  R_UINT num_primitives = block_struct -> target_prel -> num_primitives;
  R_UINT num_phrases = block_struct -> target_prel -> num_phrases;

  for (i = (num_primitives + num_phrases - 1); i != UINT_MAX; i--) {
    if (phrases1[i].str == NULL) {
      phrases1[i].len = getPhraseLength (block_struct, i);
      phrases1[i].str = wmalloc (sizeof (R_UINT) * phrases1[i].len);
      phrases1[i].isroot = (R_UCHAR) 1;
      expandPhrase (phrases1, phrases1[i].left_child, phrases1[i].str);
      expandPhrase (phrases1, phrases1[i].right_child, phrases1[i].str + phrases1[phrases1[i].left_child].len);
    }
  }

  return;
}


void unexpandAllPhrases1 (PROG_INFO *prog_struct, BLOCK_INFO *block_struct) {
  R_UINT i = 0;
  NEWPAIR1 *phrases1 = block_struct -> target_prel -> phrases1;
  R_UINT num_primitives = block_struct -> target_prel -> num_primitives;
  R_UINT num_phrases = block_struct -> target_prel -> num_phrases;

  for (i = 0; i < (num_primitives + num_phrases); i++) {
    if (phrases1[i].isroot == (R_UCHAR) 1) {
      wfree (phrases1[i].str);
    }
    phrases1[i].isroot = (R_UCHAR) 0;
    phrases1[i].str = NULL;
  }

  return;
}


void insertSizeNode (NEWBLOCK *target_prel, R_UINT currentsize) {
  SIZENODE *newnode;
  newnode = wmalloc (sizeof (SIZENODE));
  newnode -> value = currentsize;
  newnode -> next = NULL;

  if (target_prel -> sizelist == NULL) {
    target_prel -> sizelist = newnode;
    target_prel -> end = newnode;
  }
  else {
    (target_prel -> end) -> next = newnode;
    target_prel -> end = newnode;
  }

  return;
}


/*
**  Comparison function used by C library qsort function to compare
**  two phrases using their units.
*/
static R_INT unitComparison (const NEWPAIR1 **first, const NEWPAIR1 **second) {
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


static R_INT genComparison (const NEWPAIR1 **first, const NEWPAIR1 **second) {
  return ((R_INT) ((*first) -> generation) - (R_INT) ((*second) -> generation));
}


static R_INT forwardStrComparison (const NEWPAIR1 **first, const NEWPAIR1 **second) {
  R_UINT min_len;
  const NEWPAIR1 *first_pos = *(first);
  const NEWPAIR1 *second_pos = *(second);
  R_INT smallest;
  R_UINT first_len = 0;
  R_UINT second_len = 0;
  R_UINT k = 0;
  R_UINT first_index = 0;
  R_UINT second_index = 0;

  first_len = first_pos -> len;
  second_len = second_pos -> len;

  /*
  **  first.len < second.len:   -1
  **  first.len == second.len:  0
  **  first.len > second.len:   1
  */
  min_len = first_len;
  smallest = INT_MIN;
  if (first_len == second_len) {
    smallest = (R_INT) first_pos -> generation - (R_INT) second_pos -> generation;
  }
  else if (first_len > second_len) {
    min_len = second_len;
    smallest = 1;
  }

  for (k = 0; k < min_len; first_index++, second_index++, k++) {
    if (first_pos -> str[first_index] != second_pos -> str[second_index]) {
      return ((R_INT) (first_pos -> str[first_index]) - (R_INT) (second_pos -> str[second_index]));
    }
  }

  return (smallest);
}


void sortNewPhrases (PROG_INFO *prog_struct, BLOCK_INFO *block_struct) {
  NEWBLOCK *target_block = block_struct -> target_prel;
  R_UINT num_primitives = target_block -> num_primitives;
  R_UINT num_phrases = target_block -> num_phrases;
  NEWPAIR1 **temp_forward_sort = NULL;
  NEWPAIR1 **forward_sort = NULL;
  NEWPAIR1 **gen_unit_sort = NULL;
  NEWPAIR1 **duplicates_sort = NULL;
  R_UINT i = 0;
  R_UINT j = 0;
  R_UINT k = 0;
  R_UINT num_duplicates = 0;
  R_UINT new_generation = 0;
  /*  If 1, then a new generation is required; if 0, then no change  */
  R_UINT target_left = 0;
  R_UINT target_right = 0;
  R_UINT current_generation = 0;

  R_UINT curr_len;
  R_UINT prev_len;

  block_struct -> pstar = num_primitives + num_phrases;

  expandAllPhrases (prog_struct, block_struct);

  /*  Sort in the forward direction  */
  temp_forward_sort = wmalloc (sizeof (NEWPAIR1*) * (block_struct -> pstar));
  for (i = 0; i < (block_struct -> pstar); i++) {
    temp_forward_sort[i] = &(target_block -> phrases1[i]);
  }

  qsort (&temp_forward_sort[0], (size_t) (block_struct -> pstar), sizeof (NEWPAIR1*), (int (*)(const void *, const void *))forwardStrComparison);

  /*  Remove duplicates by choosing lower generation  */
  temp_forward_sort[0] -> replacement_phrase = temp_forward_sort[0] -> me;
  for (i = 1; i < num_primitives + num_phrases; i++) {
    temp_forward_sort[i] -> replacement_phrase = temp_forward_sort[i] -> me;
    curr_len = temp_forward_sort[i] -> len;
    prev_len = temp_forward_sort[i - 1] -> len;
    if (curr_len == prev_len) {
      j = curr_len;
      do {
        j--;
        if (temp_forward_sort[i] -> str[j] != temp_forward_sort[i - 1] -> str[j]) {
	  break;
	}
        if (j == 0) {
          temp_forward_sort[i] -> replacement_phrase = temp_forward_sort[i - 1] -> replacement_phrase;
	  temp_forward_sort[i] -> generation = UINT_MAX;
	  num_duplicates++;
#ifdef DEBUG
	    fprintf (stderr, "Duplicate removed from position %u", temp_forward_sort[i] -> me);
            fprintf (stderr, "\n");
#endif
	}
      } while (j != 0);
    }
  }

#ifdef DEBUG
    fprintf (stderr, "Duplicates removed:  %u\n", num_duplicates);
#endif

  if ((prog_struct -> merge_level != TWOB) && (prog_struct -> merge_level != APPEND)) {
    unexpandAllPhrases1 (prog_struct, block_struct);
  }

  /*  Renumber left and right children and reassign generations  */
  /*  Primitives are skipped since they do not need to be renumbered  */
  for (i = num_primitives; i < num_primitives + num_phrases; i++) {
    new_generation = 0;
    if (target_block -> phrases1[target_block -> phrases1[i].left_child].generation == UINT_MAX) {
      target_block -> phrases1[i].left_child = target_block -> phrases1[target_block -> phrases1[i].left_child].replacement_phrase;
      new_generation = 1;
    }

    if (target_block -> phrases1[target_block -> phrases1[i].right_child].generation == UINT_MAX) {
      target_block -> phrases1[i].right_child = target_block -> phrases1[target_block -> phrases1[i].right_child].replacement_phrase;
      new_generation = 1;
    }

    if (target_block -> phrases1[i].generation != UINT_MAX) {
      if (target_block -> phrases1[target_block -> phrases1[i].left_child].generation > target_block -> phrases1[target_block -> phrases1[i].right_child].generation) {
        target_block -> phrases1[i].generation = target_block -> phrases1[target_block -> phrases1[i].left_child].generation + 1;
      }
      else {
        target_block -> phrases1[i].generation = target_block -> phrases1[target_block -> phrases1[i].right_child].generation + 1;
      }
    }
  }

  forward_sort = wmalloc (sizeof (NEWPAIR1*) * (num_primitives + num_phrases - num_duplicates + 1));
  gen_unit_sort = wmalloc (sizeof (NEWPAIR1*) * (num_primitives + num_phrases - num_duplicates));
  if (num_duplicates != 0) {
    duplicates_sort = wmalloc (sizeof (NEWPAIR1*) * (num_duplicates));
  }

  j = 0;   /*  Index into forward_sort, backward_sort, and gen_unit_sort  */
  k = 0;                                  /*  Index into duplicates_sort  */
  for (i = 0; i < num_primitives + num_phrases; i++) {
    if (temp_forward_sort[i] -> generation == UINT_MAX) {
      temp_forward_sort[i] -> forward_sort_backptr = UINT_MAX;
      duplicates_sort[k] = temp_forward_sort[i];
      k++;
    }
    else {
      temp_forward_sort[i] -> forward_sort_backptr = j;
      forward_sort[j] = temp_forward_sort[i];
      gen_unit_sort[j] = temp_forward_sort[i];
      j++;
    }
  }

  /*  Create a sentinel at the end of the forward_sorted list  */
  forward_sort[j] = temp_forward_sort[i - 1];

  /*  Sort phrases on generations  */
  qsort (&gen_unit_sort[0], (size_t) (num_primitives + num_phrases - num_duplicates), sizeof (NEWPAIR1*), (int (*)(const void *, const void *))genComparison);

#ifdef PHRASE_HIERARCHY
    fprintf (stderr, "Phrase hierarchy in generation/chiastic order.\n");
#endif

  /*  Handle the primitives first  */
  target_left = 0;
  target_right = num_primitives;
  for (i = target_left; i < num_primitives; i++) {
#ifdef PHRASE_HIERARCHY
      fprintf (stderr, "%u --> %lld (%c) \n", i, gen_unit_sort[i] -> chiastic, (R_UCHAR) gen_unit_sort[i] -> chiastic);
#endif
    gen_unit_sort[i] -> new_me = i;
  }

  target_block -> kp = num_primitives;
  target_block -> kpp = 0;
  target_block -> kppSqr = 0;

  insertSizeNode (target_block, target_right - target_left);
  target_left = target_right;

  /*  Now, handle the phrases  */
  while (target_right < (num_primitives + num_phrases - num_duplicates)) {
    current_generation++;
    while ((target_right < (num_primitives + num_phrases - num_duplicates)) && (gen_unit_sort[target_right] -> generation == current_generation)) {
      gen_unit_sort[target_right] -> left_child = target_block -> phrases1[gen_unit_sort[target_right] -> left_child].new_me;
      gen_unit_sort[target_right] -> right_child = target_block -> phrases1[gen_unit_sort[target_right] -> right_child].new_me;
      gen_unit_sort[target_right] -> chiastic = chiasticSlide (gen_unit_sort[target_right] -> left_child, gen_unit_sort[target_right] -> right_child, target_block -> kp, target_block -> kpp, target_block -> kppSqr);
      target_right++;
    }

#ifdef DEBUG
    if (target_right == target_left) {
      fprintf (stderr, "current_generation = %u.\n", current_generation);
    }
#endif

    insertSizeNode (target_block, target_right - target_left);

    /*  Quicksort phrases in current generation on unit  */
    qsort (&gen_unit_sort[target_left], (size_t) (target_right - target_left), sizeof (NEWPAIR1*), (int (*)(const void *, const void *))unitComparison);
    for (i = target_left; i < target_right; i++) {
#ifdef PHRASE_HIERARCHY
      fprintf (stderr, "%u --> %u %u\n", i, gen_unit_sort[i] -> left_child, gen_unit_sort[i] -> right_child);
#endif
      gen_unit_sort[i] -> new_me = i;
    }

    target_block -> kpp = target_block -> kp;
    target_block -> kppSqr = (target_block -> kpp) * (target_block -> kpp);
    target_block -> kp = target_block -> kp + (target_right - target_left);
    target_left = target_right;
  }

#ifdef PHRASE_HIERARCHY
    fprintf (stderr, "\n");
#endif

  target_block -> forward_sort1 = forward_sort;
  target_block -> gen_unit_sort1 = gen_unit_sort;
  target_block -> duplicates_sort1 = duplicates_sort;

  wfree (temp_forward_sort);

#ifdef DEBUG
    fprintf (stderr, "%u + %u = %u ---> %u + %u = %u\n", num_primitives, num_phrases, num_primitives + num_phrases, num_primitives, num_phrases - num_duplicates, num_primitives + num_phrases - num_duplicates);
#endif

  target_block -> num_phrases -= num_duplicates;
  target_block -> num_duplicates = num_duplicates;
  target_block -> num_generation = current_generation;

  return;
}


