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
#include <limits.h>

#include "common-def.h"
#include "wmalloc.h"
#include "remerge-defn.h"
#include "hashing.h"
#include "remerge.h"
#include "merge-phrases.h"

static void addPrimPhrase (PROG_INFO *prog_struct, NEWPAIR1 *target, R_UINT count_prims, OLDPAIR *phrases1, R_UINT phrases1_count, OLDPAIR *phrases2, R_UINT phrases2_count);

static R_UINT mergePhrasesHelper (PROG_INFO *prog_struct, NEWBLOCK *target_prel, R_UINT count_phrases, OLDPAIR *phrases1, R_UINT phrases_1_left, R_UINT phrases_1_right, OLDPAIR *phrases2, R_UINT phrases_2_left, R_UINT phrases_2_right);


static void addPrimPhrase (PROG_INFO *prog_struct, NEWPAIR1 *target, R_UINT count_prims, OLDPAIR *phrases1, R_UINT phrases1_count, OLDPAIR *phrases2, R_UINT phrases2_count) {
  OLDPAIR *current;

  target = target + count_prims;
  if (phrases1 != NULL) {
    phrases1 = phrases1 + phrases1_count;
  }
  if (phrases2 != NULL) {
    phrases2 = phrases2 + phrases2_count;
  }

  current = (phrases1 == NULL ? phrases2 : phrases1);
  target -> left_child = current -> left_child;
  target -> right_child = current -> right_child;
  target -> chiastic = current -> chiastic;
  target -> me = count_prims;

  if (phrases1 == NULL) {
    target -> first_prel = UINT_MAX;
    target -> second_prel = phrases2_count;
    target -> generation = phrases2 -> generation;
    phrases2 -> new_prel = count_prims;
  }
  else if (phrases2 == NULL) {
    target -> second_prel = UINT_MAX;
    target -> first_prel = phrases1_count;
    target -> generation = phrases1 -> generation;
    phrases1 -> new_prel = count_prims;
  }
  else {
    target -> first_prel = phrases1_count;
    target -> second_prel = phrases2_count;
    target -> generation = phrases1 -> generation;
    phrases1 -> new_prel = count_prims;
    phrases2 -> new_prel = count_prims;
  }

  target -> len = current -> len;
  if (target -> generation == 0) {
    target -> str = wmalloc (sizeof (R_UINT) * 1);
    target -> str[0] = (R_UINT) target -> chiastic;
    target -> isroot = (R_UCHAR) 1;
  }
  else {
    target -> str = NULL;
    target -> isroot = (R_UCHAR) 0;
  }

  return;
}


/*
**  Merge phrases1 and phrases2 into target for the current generation.
**  If either phrases1 or phrases2 is NULL, then the phrase list that is
**  not NULL is appended to target.
*/
static R_UINT mergePhrasesHelper (PROG_INFO *prog_struct, NEWBLOCK *target_prel, R_UINT count_phrases, OLDPAIR *phrases1, R_UINT phrases_1_left, R_UINT phrases_1_right, OLDPAIR *phrases2, R_UINT phrases_2_left, R_UINT phrases_2_right) {
  R_BOOLEAN found = R_FALSE;
  R_UINT i = 0;  /*  Temporary index into phrases1  */
  R_UINT j = 0;  /*  Temporary index into phrases2  */
  R_UINT hashvalue = 0;

  HASH_PAIR *prev = NULL;
  HASH_PAIR *curr = NULL;

  NEWPAIR1 *target_phrases1 = target_prel -> phrases1;

  HASH_PAIR *phrases_hashtable[TENTPHRASE_SIZE];

  int b = 0;
  int d = 0;
  int e = 0;
  int f = 0;
  int g = 0;

  /*
  R_UINT newname = 0;
  R_UINT memindex;
  R_UINT block;
  Memindex *currindex = NULL;
  HASH_PAIR *list = NULL;
  */

  /*
  prog_struct -> hashpair_root = initSMem (sizeof (HASH_PAIR));
  */

  for (i = 0; (R_ULL_INT) i < TENTPHRASE_SIZE; i++) {
    phrases_hashtable[i] = NULL;
  }

  /*  Make l and r refer to target prel's renumbering  */
  if (phrases1 != NULL) {
    for (i = phrases_1_left; i <= phrases_1_right; i++) {
      phrases1[i].left_child = phrases1[phrases1[i].left_child].new_prel;
      phrases1[i].right_child = phrases1[phrases1[i].right_child].new_prel;
      phrases1[i].chiastic = 0;
    }
  }

  if (phrases2 != NULL) {
    for (j = phrases_2_left; j <= phrases_2_right; j++) {
      phrases2[j].left_child = phrases2[phrases2[j].left_child].new_prel;
      phrases2[j].right_child = phrases2[phrases2[j].right_child].new_prel;
      phrases2[j].chiastic = 0;

      /*  Insert into hash table based on children  */
      hashvalue = hashCode (phrases2[j].left_child, phrases2[j].right_child);

      curr =  wmalloc (sizeof (HASH_PAIR));

      if (phrases_hashtable[hashvalue] == NULL) {
        curr -> left = phrases2[j].left_child;
        curr -> right = phrases2[j].right_child;
        curr -> phrase_2_num = j;
        curr -> next = NULL;
        phrases_hashtable[hashvalue] = curr;
      }
      else {
        prev = phrases_hashtable[hashvalue];
        while (prev -> next != NULL) {
          prev = prev -> next;
	}
        curr -> left = phrases2[j].left_child;
        curr -> right = phrases2[j].right_child;
        curr -> phrase_2_num = j;
        curr -> next = NULL;
        prev -> next = curr;
      }
    }
  }

  /*  Check if both phrase lists are non-NULL  */
  if ((phrases1 != NULL) && (phrases2 != NULL)) {
    for (i = phrases_1_left; i <= phrases_1_right; i++) {
      found = R_FALSE;
      hashvalue = hashCode (phrases1[i].left_child, phrases1[i].right_child);

      if (phrases_hashtable[hashvalue] != NULL) {
        prev = phrases_hashtable[hashvalue];
        while (prev != NULL) {
          if ((prev -> left == phrases1[i].left_child) && (prev -> right == phrases1[i].right_child)) {
            found = R_TRUE;
            phrases2[prev -> phrase_2_num].chiastic = UINT_MAX;
	    b++;
            addPrimPhrase (prog_struct, target_phrases1, count_phrases, phrases1, i, phrases2, prev -> phrase_2_num);
            count_phrases++;
            break;
	  }
          else {
            prev = prev -> next;
	  }
	}
      }

      if (found == R_FALSE) {
	d++;
        addPrimPhrase (prog_struct, target_phrases1, count_phrases, phrases1, i, NULL, UINT_MAX);
        count_phrases++;
      }
    }
    /*    Scan phrases2 for anything not added  */
    for (j = phrases_2_left; j <= phrases_2_right; j++) {
      if (phrases2[j].chiastic != UINT_MAX) {
	e++;
        addPrimPhrase (prog_struct, target_phrases1, count_phrases, NULL, UINT_MAX, phrases2, j);
        count_phrases++;
      }
      else {
        /*  Do nothing  */
      }
    }
  }
  /*  Append phrases from phrases1 to target_phrases1  */
  else if (phrases1 != NULL) {
    for (i = phrases_1_left; i <= phrases_1_right; i++) {
      f++;
      addPrimPhrase (prog_struct, target_phrases1, count_phrases, phrases1, i, NULL, DONT_CARE);
      count_phrases++;
    }
  }
  /*  Append phrases from phrases2 to target_phrases1  */
  else if (phrases2 != NULL) {
    for (j = phrases_2_left; j <= phrases_2_right; j++) {
      g++;
      addPrimPhrase (prog_struct, target_phrases1, count_phrases, NULL, DONT_CARE, phrases2, j);
      count_phrases++;
    }
  }

  /*  
  fprintf (stderr, "%d %d %d %d %d %d %d\n", a, b, c, d, e, f, g);
  */

  /*
  uninitSMem (prog_struct -> hashpair_root);
  prog_struct -> hashpair_root = NULL;
  */

  for (i = 0; (R_ULL_INT) i < TENTPHRASE_SIZE; i++) {
    if (phrases_hashtable[i] != NULL) {
      prev = NULL;
      curr = phrases_hashtable[i];
      while (curr != NULL) {
        prev = curr;
        curr = curr -> next;
        wfree (prev);
      }
    }
  }

  return (count_phrases);
}


/*
**  Function used to merge primitives
*/
void mergePrimitives (PROG_INFO *prog_struct, BLOCK_INFO *block_struct) {
  R_UINT num_prims_1 = (block_struct -> source_prel_1) -> num_primitives;
  R_UINT num_prims_2 = (block_struct -> source_prel_2) -> num_primitives;

  R_UINT count_prims_1 = 0;
  R_UINT count_prims_2 = 0;
  R_UINT count_prims = 0;

  OLDPAIR *phrases1 = (block_struct -> source_prel_1) -> phrases;
  OLDPAIR *phrases2 = (block_struct -> source_prel_2) -> phrases;
  NEWPAIR1 *target_phrases1 = (block_struct -> target_prel) -> phrases1;

  /*  Continue while there are primitives on both preludes  */
  while ((count_prims_1 < num_prims_1) && (count_prims_2 < num_prims_2)) {
    /*  If both primitives are the same, process both  */
    if ((phrases1 + count_prims_1) -> chiastic == (phrases2 + count_prims_2) -> chiastic) {
      addPrimPhrase (prog_struct, target_phrases1, count_prims, phrases1, count_prims_1, phrases2, count_prims_2);
      count_prims_1++;
      count_prims_2++;
    }
    /*  If phrase1's primitive is larger, process phrase2's  */
    else if ((phrases1 + count_prims_1) -> chiastic > (phrases2 + count_prims_2) -> chiastic) {
      addPrimPhrase (prog_struct, target_phrases1, count_prims, NULL, UINT_MAX, phrases2, count_prims_2);
      count_prims_2++;
    }
    /*  If phrase2's primitive is larger, process phrase1's  */
    else if ((phrases1 + count_prims_1) -> chiastic < (phrases2 + count_prims_2) -> chiastic) {
      addPrimPhrase (prog_struct, target_phrases1, count_prims, phrases1, count_prims_1, NULL, UINT_MAX);
      count_prims_1++;
    }
    count_prims++;
  }

  /*  Finish processing phrase1's primitives  */
  if (count_prims_1 != num_prims_1) {
    while (count_prims_1 != num_prims_1) {
      addPrimPhrase (prog_struct, target_phrases1, count_prims, phrases1, count_prims_1, NULL, UINT_MAX);
      count_prims_1++;
      count_prims++;
    }
  }
  /*  Finish processing phrase2's primitives  */
  else {
    while (count_prims_2 != num_prims_2) {
      addPrimPhrase (prog_struct, target_phrases1, count_prims, NULL, UINT_MAX, phrases2, count_prims_2);
      count_prims_2++;
      count_prims++;
    }
  }

  (block_struct -> target_prel) -> num_primitives = count_prims;

#ifdef DEBUG
    fprintf (stderr, "New number of primitives:  %u (%u, %u)\n", count_prims, num_prims_1, num_prims_2);
#endif

  return;
}


void mergePhrases (PROG_INFO *prog_struct, BLOCK_INFO *block_struct) {
  R_UINT num_phrases_1 = (block_struct -> source_prel_1) -> num_primitives + (block_struct -> source_prel_1) -> num_phrases;
  R_UINT num_phrases_2 = (block_struct -> source_prel_2) -> num_primitives + (block_struct -> source_prel_2) -> num_phrases;

  R_UINT count_phrases_1 = (block_struct -> source_prel_1) -> num_primitives;
  R_UINT count_phrases_2 = (block_struct -> source_prel_2) -> num_primitives;
  R_UINT count_phrases = (block_struct -> target_prel) -> num_primitives;

  /*  The generation just processed was the primitives (0)  */
  R_UINT generation = 0;

  OLDPAIR *phrases1 = (block_struct -> source_prel_1) -> phrases;
  OLDPAIR *phrases2 = (block_struct -> source_prel_2) -> phrases;

  /*  Mark the left and right boundaries for each source prelude for
  **  the current generation since the helper function processes a
  **  generation at a time  */
  R_UINT phrases_1_left = (block_struct -> source_prel_1) -> num_primitives;
  R_UINT phrases_1_right = (block_struct -> source_prel_1) -> num_primitives;
  R_UINT phrases_2_left = (block_struct -> source_prel_2) -> num_primitives;
  R_UINT phrases_2_right = (block_struct -> source_prel_2) -> num_primitives;

  while ((count_phrases_1 < num_phrases_1) && (count_phrases_2 < num_phrases_2)) {
    generation++;

    while ((phrases_1_right < num_phrases_1) && ((phrases1 + phrases_1_right) -> generation == generation)) {
      phrases_1_right++;
    }

    while ((phrases_2_right < num_phrases_2) && ((phrases2 + phrases_2_right) -> generation == generation)) {
      phrases_2_right++;
    }

    phrases_1_right--;
    phrases_2_right--;

    count_phrases = mergePhrasesHelper (prog_struct, (block_struct -> target_prel), count_phrases, phrases1, phrases_1_left, phrases_1_right, phrases2, phrases_2_left, phrases_2_right);

    count_phrases_1 += phrases_1_right - phrases_1_left + 1;
    count_phrases_2 += phrases_2_right - phrases_2_left + 1;

    phrases_1_right++;
    phrases_2_right++;
    phrases_1_left = phrases_1_right;
    phrases_2_left = phrases_2_right;

  }

  /*  Appending remaining phrases from phrase hierarchy #1  */
  while (count_phrases_1 < num_phrases_1) {
    generation++;
    /*
    fprintf (stderr, "Generation %d %d\n", (phrases1 + phrases_1_right) -> generation, generation);
    */
    while (phrases_1_right < num_phrases_1 && ((phrases1 + phrases_1_right) -> generation == generation)) {
      phrases_1_right++;
    }
    phrases_1_right--;
    count_phrases = mergePhrasesHelper (prog_struct, (block_struct -> target_prel), count_phrases, phrases1, phrases_1_left, phrases_1_right, NULL, DONT_CARE, DONT_CARE);
    count_phrases_1 += phrases_1_right - phrases_1_left + 1;
    phrases_1_right++;
    phrases_1_left = phrases_1_right;
  }

  /*  Appending remaining phrases from phrase hierarchy #2  */
  while (count_phrases_2 < num_phrases_2) {
    generation++;
    /*
    fprintf (stderr, "Generation %d %d\n", (phrases2 + phrases_2_right) -> generation, generation);
    */
    while (((phrases2 + phrases_2_right) -> generation == generation) && phrases_2_right < num_phrases_2) {
      phrases_2_right++;
    }
    phrases_2_right--;
    count_phrases = mergePhrasesHelper (prog_struct, (block_struct -> target_prel), count_phrases, NULL, DONT_CARE, DONT_CARE, phrases2, phrases_2_left, phrases_2_right);
    count_phrases_2 += phrases_2_right - phrases_2_left + 1;
    phrases_2_right++;
    phrases_2_left = phrases_2_right;
  }

  count_phrases -= (block_struct -> target_prel) -> num_primitives;
  (block_struct -> target_prel) -> num_phrases = count_phrases;
  (block_struct -> target_prel) -> num_generation = generation;

  /*
  fprintf (stderr, "Leaving mergePhrases:  %d %d %d\n", (block_struct -> target_prel) -> num_primitives, (block_struct -> target_prel) -> num_phrases, (block_struct -> target_prel) -> num_generation);
  */

  return;
}

