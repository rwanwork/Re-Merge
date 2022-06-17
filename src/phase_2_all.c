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
#include <sys/times.h>
#include <unistd.h>                                          /*  sysconf  */

#include "common-def.h"
#include "wmalloc.h"
#include "remerge-defn.h"
#include "remerge.h"
#include "io.h"
#include "phase_2_all.h"


static NEWPAIR1 *binary_search (BLOCK_INFO *block_struct, R_UINT init_left, R_UINT init_right, R_UINT init_depth, CHAR_NODE *char_buffer);

/*
**  Compare a string with an infinite string (length = remainder) starting
**  at a particular depth.
**
**  Return:
**    -ve if (str > char_buffer)
**    +ve if (str < char_buffer)
**    0 if (str == char_buffer) at the specified depth
**    INT_MIN if char_buffer exhausted
**    INT_MAX if str exhausted
*/
static R_INT binary_search_compare (NEWPAIR1 *phrase, CHAR_NODE *char_buffer, R_UINT init_depth, R_UINT depth, R_INT *flag) {
  R_INT result = 0;
  R_UINT *str = phrase -> str;
  R_UINT len = phrase -> len;
  R_UINT i = init_depth;

  *flag = -1;
  str = str + i;

  for (; i <= depth; i++, str++) {
    if ((char_buffer[i].ch - (R_CHAR) *str) != (R_CHAR) 0) {
      result = (R_INT) (char_buffer[i].ch - (R_CHAR) *str);
      break;
    }
  }

  if ((result == 0) && (depth == (len - 1))) {
      *flag = (R_INT) (phrase -> forward_sort_backptr);
  }

  return (result);
}


/*
**  Static functions
*/
static NEWPAIR1 *binary_search (BLOCK_INFO *block_struct, R_UINT init_left, R_UINT init_right, R_UINT init_depth, CHAR_NODE *char_buffer) {
  R_UINT mid = 0;
  R_UINT depth = 0;
  R_INT result = 0;
  NEWPAIR1 *curr_phrase = NULL;
  R_UINT left = 0;
  R_UINT right = 0;
  R_INT flag = -2;  /*  initial condition  */

  NEWPAIR1 **forward_sort = (block_struct -> target_prel) -> forward_sort1;

  init_right--;
  depth = init_depth;
  do {
    flag = -2;
    left = init_left;
    right = init_right;
    while (left <= right) {
      mid = (right + left) >> 1;
      result = binary_search_compare (forward_sort[mid], char_buffer, init_depth, depth, &flag);

      if (result <= 0) {
	if ((result == 0) && (flag >= 0)) {
	  return (forward_sort[mid]);

	}
	if (result < 0) {
	  init_right = mid - 1;
	}
	right = mid - 1;
      }
      else if (result > 0) {
	init_left = mid + 1;
	left = mid + 1;
      }
    }
    depth++;
  } while (flag == -1);

  return (curr_phrase);
}


static CHECK_NODE *getPointerNode (CHECK_NODE **check_node_list, MALLOC_STRUCT *check_node_info) {
  CHECK_NODE *newnode = NULL;

  check_node_info -> curr_count++;
  if (check_node_info -> curr_count == check_node_info -> max_count) {
    fprintf (stderr, "Error:  Number of check_nodes in %s [%u] exhausted.\n", __FILE__, __LINE__);
    exit (EXIT_FAILURE);
  }

  check_node_info -> curr_x++;
  if (check_node_info -> curr_x == check_node_info -> max_x) {
    check_node_info -> curr_x = 0;
    check_node_info -> curr_y++;
    if (check_node_info -> curr_y == check_node_info -> max_y) {
      check_node_info -> curr_x = 0;
      check_node_info -> curr_y = 0;
    }
  }

  newnode = &check_node_list[check_node_info -> curr_x][check_node_info -> curr_y];

  return (newnode);
}

MALLOC_STRUCT *init_NodeArray (R_UINT max_x, R_UINT max_y) {
  MALLOC_STRUCT *result;

  result = wmalloc (sizeof (MALLOC_STRUCT));
  result -> max_x = max_x;
  result -> max_y = max_y;
  result -> curr_x = 0;
  result -> curr_y = 0;
  result -> max_count = max_x * max_y;
  result -> curr_count = 0;
  
  return (result);
}

CHECK_NODE *insertPointerNode (CHECK_NODE **check_node_list, MALLOC_STRUCT *check_node_info, R_UINT phrase_id, R_UINT phrase_len) {
  CHECK_NODE *newnode = getPointerNode (check_node_list, check_node_info);

  newnode -> phrase_id = phrase_id;
  newnode -> phrase_len = phrase_len;
  newnode -> next = NULL;

  return (newnode);
}


R_INT binary_ph_cmp (BLOCK_INFO *block_struct, R_UINT phrase_pos, CHAR_NODE *char_buffer, R_UINT check_position, R_UINT segment_end) {
  NEWPAIR1 **forward_sort = (block_struct -> target_prel) -> forward_sort1;
  R_UINT *prims_lookup = (block_struct -> target_prel) -> prims_lookup;
  R_UINT num_prims = (block_struct -> target_prel) -> prims_lookup_size;
  NEWPAIR2 *target_phrases2 = (block_struct -> target_prel) -> phrases2;

  R_UINT start = phrase_pos;
  R_UINT end = prims_lookup[num_prims];
  R_UINT curr = 0;
  R_UINT i;

  R_INT best_match = 0;
  R_UINT remainder = segment_end - check_position;
  NEWPAIR1 *curr_phrase = NULL;
  R_UINT depth = 0;

  R_INT result = 0;

  /*  Find the first available primitive as the right end point  */
  for (i = forward_sort[phrase_pos] -> str[0] + 1; i < num_prims; i++) {
    if (prims_lookup[i] != UINT_MAX) {
      end = prims_lookup[i];
      break;
    }
  }

  char_buffer = char_buffer + check_position;

  while ((end - start) > 1) {
    curr = start + ((end - start) >> 1);
    depth = 0;

    while (char_buffer[depth].ch == (R_CHAR) forward_sort[curr] -> str[depth]) {
      depth++;
      if (depth == forward_sort[curr] -> len) {
        break;
      }
    }

    /*  Replacement for NULL character  */
    if (depth == forward_sort[curr] -> len) {
      result = 0;
    }
    else {
      result = (R_INT) forward_sort[curr] -> str[depth];
    }
    if (((R_INT) char_buffer[depth].ch) - ((R_INT) result) > 0) {
      start = curr;
    }
    else {
      end = curr;
    }
  }

  /*
  **  Assumes a sentinel at the end of the forward_sorted list that is the
  **  same as the last phrase; therefore, if the last phrase is chosen,
  **  start will point to it and end will point to the sentinel while
  **  ensuring (end - start = 1)
  */
  curr_phrase = forward_sort[start];
  while (R_TRUE) {
    if (curr_phrase -> len <= remainder) {
      i = curr_phrase -> len - 1;
      while ((i != UINT_MAX) && (char_buffer[i].ch == (R_CHAR) curr_phrase -> str[i])) {
        i--;
      }
      if (i == UINT_MAX) {
        best_match = (R_INT) curr_phrase -> forward_sort_backptr;
        break;
      }
    }
    curr_phrase = target_phrases2[curr_phrase -> me].prefix_phrase;
  }

  return (best_match);
}


R_UINT link_ph_cmp (BLOCK_INFO *block_struct, R_UINT phrase_pos, CHAR_NODE *char_buffer, R_UINT check_position, R_UINT segment_end) {
  R_UINT nextsym = 0;
  R_UINT k = 0;
  R_UINT best_match = 0;
  R_UINT next_phrase_count = 0;

  NEWPAIR1 **gen_unit_sort = (block_struct -> target_prel) -> gen_unit_sort1;
  NEWPAIR1 *candidate = NULL;  /*  candidate is always the last known
			      **  longest phrase  */
  NEWPAIR1 *temp_candidate = NULL;
  R_UINT candidate_len = 0;
  NEWPAIR2 *target_phrases2 = (block_struct -> target_prel) -> phrases2;

  R_BOOLEAN completely_done = R_FALSE;
  R_UINT start = 0;
  R_UINT end = 0;

  /*
  **  Amount of space left in buffer; always at least the length of the
  **  longest phrase, unless the end of sequence has been reached
  */
  R_UINT buffer_len = segment_end - check_position;

  char_buffer = char_buffer + check_position;
  candidate = gen_unit_sort[char_buffer[0].search_start];

  while (R_TRUE) {
    candidate_len = candidate -> len;
    if (check_position + candidate_len >= segment_end) {
      break;
    }

    nextsym = (R_UINT) char_buffer[candidate_len].ch;
    next_phrase_count = candidate -> first_prel;
    /*  Try to follow next_phrase pointers to find a longer match  */
    completely_done = R_TRUE;
    for (k = 0; k < next_phrase_count; k++) {
      if (nextsym == target_phrases2[candidate -> me].next_phrase_list[k].nextsym) {
        temp_candidate = target_phrases2[candidate -> me].next_phrase_list[k].ph;
        completely_done = R_FALSE;
        break;
      }
      else if (nextsym < target_phrases2[candidate -> me].next_phrase_list[k].nextsym) {
        break;
      }
    }

    /*
    **  If a symbol cannot be added to the current phrase, then
    **  break, keeping candidate
    */
    if (completely_done == R_TRUE) {
      break;
    }

    start = temp_candidate -> forward_sort_backptr;
    if (k != next_phrase_count - 1) {
      end = target_phrases2[candidate -> me].next_phrase_list[k + 1].ph -> forward_sort_backptr;
    }
    else {
      end = target_phrases2[candidate -> me].last_prefix_phrase -> forward_sort_backptr + 1;
    }
    if (start == end) {
      end++;
    }
    temp_candidate = binary_search (block_struct, start, end, candidate -> len, char_buffer);
    /*  No better match found, so leave loop  */
    if (temp_candidate == NULL) {
      break;
    }
    candidate = temp_candidate;
  }

  best_match = candidate -> forward_sort_backptr;

  /*
  **  Set the search starting position at the next adjacent 
  **  position
  */
  if ((buffer_len > 1) && (candidate -> generation != 0)) {
    candidate = target_phrases2[candidate -> me].secondary_phrase;
    candidate_len = candidate -> len;
    while (candidate_len > buffer_len) {
      candidate = target_phrases2[candidate -> me].prefix_phrase;
      candidate_len = candidate -> len;
    }
    if (candidate -> len > gen_unit_sort[char_buffer[1].search_start] -> len) {
      char_buffer[1].search_start = candidate -> new_me;
    }
  }

  return (best_match);
}


/*
**  BFS algorithm adapted from Introduction to Algorithms (Second
**  edition by Cormen et. al., page 532
*/
R_UINT findShortestPath (CHAR_NODE *char_buffer, R_UINT segment_start, R_UINT segment_end, PROG_INFO *prog_struct, MALLOC_STRUCT *check_node_info) {
  CHAR_NODE *head = NULL;               /*  Head, tail, current of queue  */
  CHAR_NODE *tail = NULL;
  CHAR_NODE *curr = NULL;
  CHAR_NODE *next = NULL;
  CHECK_NODE *curr_ptr = NULL;
  R_UINT i;
  R_UINT path_length;
  R_UINT bytes_read;
  R_UINT count = 0;
  R_UINT minor_cut_vertex_pos = 0;
  R_UINT diff = 0;

  R_UINT len = 0;

  minor_cut_vertex_pos = 0;

  /*  Initialise the char_buffer array  */
  /*  If punctuation flags are used  */
  if (prog_struct -> word_flags == UW_YES) {
    for (i = segment_start; i <= segment_end; i++) {
      char_buffer[i].minor_cut_vertex = minor_cut_vertex_pos;
      if (char_buffer[i].cut_vertex == IS_CUT) {
        minor_cut_vertex_pos++;
      }
    }

    /*  The vertex before segment_start is an IS_CUT vertex  */
    char_buffer[segment_start].prev_cut = IS_CUT;
    for (i = segment_start + 1; i <= segment_end; i++) {
      char_buffer[i].prev_cut = char_buffer[i - 1].cut_vertex;
    }
  }
  /*  Just initialise everything to 0  */
  else {
    for (i = segment_start; i <= segment_end; i++) {
      char_buffer[i].minor_cut_vertex = 0;
      char_buffer[i].prev_cut = IS_NOT_CUT;
    }
  }

  char_buffer[segment_start].colour = GRAY;
  char_buffer[segment_start].distance = 0;
  char_buffer[segment_start].parent = NULL;

  head = char_buffer + segment_start;
  tail = char_buffer + segment_start;
  while (head != NULL) {
    curr = head;
    curr_ptr = curr -> edge_list;
    while (curr_ptr != NULL) {
      /*      check_node_info -> curr_count--;*/
      if ((curr + (curr_ptr -> phrase_len)) -> colour == WHITE) {
	next = curr + (curr_ptr -> phrase_len);
	diff = next -> minor_cut_vertex - curr -> minor_cut_vertex;
        len = curr_ptr -> phrase_len;

	if (((curr -> minor_cut_vertex) == (next -> minor_cut_vertex)) ||
	    ((curr -> prev_cut != IS_CUT) && (diff == 1) && (next -> prev_cut == IS_CUT)) ||
	    ((curr -> prev_cut == IS_CUT) && (next -> prev_cut == IS_CUT))) {
          (curr + (curr_ptr -> phrase_len)) -> colour = GRAY;
          (curr + (curr_ptr -> phrase_len)) -> distance = (curr -> distance) + 1;
          (curr + (curr_ptr -> phrase_len)) -> parent = curr;
          (curr + (curr_ptr -> phrase_len)) -> phrase_id = curr_ptr -> phrase_id;
          if (head == NULL) {
            head = curr + (curr_ptr -> phrase_len);
            tail = head;
            head -> next_queue = NULL;
	  }
          else {
            tail -> next_queue = curr + (curr_ptr -> phrase_len);
            tail = tail -> next_queue;
            tail -> next_queue = NULL;
	  }
	}
        else{
	  /*
          fprintf (stderr, "--> %u %u %u\n", curr -> cut_vertex, next -> cut_vertex, diff);
	  */
	}
	  /*
	  fprintf (stderr, "--> %u %u %u %u (%u)\n", curr -> minor_cut_vertex, next -> minor_cut_vertex, curr -> cut_vertex, next -> cut_vertex, curr_ptr -> phrase_len);
	  */
      }
      curr_ptr = curr_ptr -> next;
    }
    curr -> colour = BLACK;
    head = curr -> next_queue;

    /*  Leave loop once the last char_node has been coloured black  */
    if (curr == &char_buffer[segment_end]) {
      break;
    }
  }

  /*  At the end of a segment, no check nodes should be in use  */
  check_node_info -> curr_count = 0;

  path_length = char_buffer[segment_end].distance;
  /*  fprintf (stderr, "path length:  %u\n", path_length);*/
  if (path_length == UINT_MAX) {
    fprintf (stderr, "Error:  Path_length UINT_MAX!\n");
  }
  if (path_length > prog_struct -> path_buffer_size) {
    prog_struct -> path_buffer_size = path_length;
    prog_struct -> path_buffer = wrealloc (prog_struct -> path_buffer, prog_struct -> path_buffer_size * sizeof (R_UINT));
  }

  i = path_length - 1;
  curr = &char_buffer[segment_end];
  while (i != UINT_MAX) {
    prog_struct -> path_buffer[i] = curr -> phrase_id + 1;
    /*  Set punctuation flag if explicit cut vertex  */
    if (curr -> prev_cut == IS_CUT) {
      prog_struct -> path_buffer[i] = prog_struct -> path_buffer[i] | PUNC_FLAG;
    }
    i--;
    curr = curr -> parent;
  }
  for (i = 0; i < path_length; i++) {
    *(prog_struct -> out_buf_p) = prog_struct -> path_buffer[i];
    prog_struct -> out_buf_p++;

    /*  Flush output buffer if necessary  */
    if (prog_struct -> out_buf_p == prog_struct -> out_buf_end) {
      bytes_read = writeBuffer (prog_struct, (R_FLOAT) 1);
      count += bytes_read;
    }
  }

  return (path_length);
}


void expandPhraseToBuffer (BLOCK_INFO *block_struct, CHAR_NODE *char_buffer, R_UINT *expand_buffer, R_UINT y, R_UINT z) {
  R_UINT i = 0;
  NEWPAIR1 *target_phrases = (block_struct -> target_prel) -> phrases1;
  NEWPAIR1 **gen_unit_sort = (block_struct -> target_prel) -> gen_unit_sort1;
  R_UINT len = target_phrases[y].len;
  R_UINT left = 0;
  R_UINT right = 0;
  R_UINT right_offset = 0;

  /*  Initialise positions in char_buffer used by this phrase  */
  for (i = 0; i < len; i++) {
    char_buffer[i].search_start = UINT_MAX;
    char_buffer[i].cut_vertex = IS_NOT_CUT;
    expand_buffer[i] = UINT_MAX;
  }

  char_buffer[0].search_start = y;
  expand_buffer[0] = y;

  for (i = 0; i < len; i++) {
    while (target_phrases[expand_buffer[i]].generation != 0) {
      left = GET_LEFT (expand_buffer[i]);
      right = GET_RIGHT (expand_buffer[i]);
      right_offset = i + GET_PHRASE_LEN (left);

      expand_buffer[i] = left;
      if (char_buffer[i].search_start == UINT_MAX) {
        char_buffer[i].search_start = expand_buffer[i];
      }
      expand_buffer[right_offset] = right;
      if (char_buffer[right_offset].search_start == UINT_MAX) {
        char_buffer[right_offset].search_start = expand_buffer[right_offset];
      }
    }
  }
    
  /*  Force cut vertex because of punctuation flag  */
  if (z == PUNC_FLAG) {
    char_buffer[len - 1].cut_vertex = IS_CUT;
  }

  return;
}


void linkPrimitivesLookup (PROG_INFO *prog_struct, BLOCK_INFO *block_struct) {
  R_UINT *prims_lookup;
  R_UINT prims_lookup_size = 0;
  R_UINT num_primitives = block_struct -> target_prel -> num_primitives;
  NEWPAIR1 **gen_unit_sort = (block_struct -> target_prel) -> gen_unit_sort1;
  R_UINT i = 0;

  /*  Find out the maximum chiastic slide value for the primitives  */
  prims_lookup_size = (R_UINT) gen_unit_sort[num_primitives - 1] -> chiastic;

  prims_lookup = wmalloc ((prims_lookup_size + 1) * sizeof (R_UINT));
  for (i = 0; i < prims_lookup_size; i++) {
    prims_lookup[i] = UINT_MAX;
  }

  for (i = 0; i < num_primitives; i++) {
    prims_lookup[gen_unit_sort[i] -> chiastic] = gen_unit_sort[i] -> forward_sort_backptr;
  }

  block_struct -> target_prel -> prims_lookup = prims_lookup;
  block_struct -> target_prel -> prims_lookup_size = prims_lookup_size;
  return;
}


void linkPrimaryPrefixPhrase (PROG_INFO *prog_struct, BLOCK_INFO *block_struct) {
  R_UINT i = 0;                                 /*  Current phrase index  */
  R_UINT j = 0;                          /*  Potential link phrase index  */
  R_INT k = 0;                                       /*  Character index  */
                         /*  k must be signed so that the for loop exits  */

  R_UINT num_primitives = (block_struct -> target_prel) -> num_primitives;
  R_UINT num_phrases = (block_struct -> target_prel) -> num_phrases;
  R_UINT longest_phrase = 0;

  NEWPAIR1 **forward_sort = (block_struct -> target_prel) -> forward_sort1;
  NEWPAIR2 *target_phrases2 = (block_struct -> target_prel) -> phrases2;

  j = 0;
  for (i = 0; i < num_primitives + num_phrases; i++) {
    if (forward_sort[i] -> len > longest_phrase) {
      longest_phrase = forward_sort[i] -> len;
    }
    if (forward_sort[i] -> generation == 0) {
      target_phrases2[forward_sort[i] -> me].prefix_phrase = NULL;
    }
    else {
      /*  Start at the phrase before i  */
      /*  Will never go beyond 0 since the first phrase must 
      **  be a primitive  */
      for (j = i - 1; j != UINT_MAX; j--) {
        if (forward_sort[j] -> len < forward_sort[i] -> len) {
          /*  Start pos at the end of the shorter phrase, j.  */
          for (k = (R_INT) (forward_sort[j] -> len - 1); k >= 0; k--) {
            if (forward_sort[j] -> str[k] != forward_sort[i] -> str[k]) {
	      break;
	    }
	  }
	  /*  Due to the structure of the phrase hierarchy, no two
	  **  phrases will be identical.  */
          if (k == -1) {
            target_phrases2[forward_sort[i] -> me].prefix_phrase = forward_sort[j];
            break;
	  }
	}
      }
    }
  }

  block_struct -> longest_phrase = longest_phrase;
#ifdef DEBUG
  fprintf (stderr, "\tThe longest phrase has a length of %u.\n", longest_phrase);
#endif
  return;
}


void linkNextPhrase (PROG_INFO *prog_struct, BLOCK_INFO *block_struct) {
  R_UINT num_primitives = (block_struct -> target_prel) -> num_primitives;
  R_UINT num_phrases = (block_struct -> target_prel) -> num_phrases;

  NEWPAIR1 **forward_sort = (block_struct -> target_prel) -> forward_sort1;
  NEWPAIR1 *prefix = NULL;
  R_UINT prefix_len = 0;
  R_UINT nextsym = 0;
  NEWPAIR2 *target_phrases2 = (block_struct -> target_prel) -> phrases2;

  R_UINT i = 0;
  R_UINT j = 0;

  R_UINT count = 0;
  R_UINT count2 = 0;

  /*  For each phrase, initialise the count of next phrases to 0  */
  for (i = 0; i < num_primitives + num_phrases; i++) {
    forward_sort[i] -> first_prel = 0;
    target_phrases2[forward_sort[i] -> me].last_prefix_phrase = NULL;
  }

  for (i = 0; i < num_primitives + num_phrases; i++) {
    if (forward_sort[i] -> generation == 0) {
      /*  Do nothing to primitives  */
    }
    else {
      /*
      **  For each phrase, get the prefix and the symbol after the prefix.
      **  Check the prefix to see if an entry has already been made; if not
      **  add it...if so, then skip this phrase.
      */
      prefix = target_phrases2[forward_sort[i] -> me].prefix_phrase;
      prefix_len = prefix -> len;
      nextsym = forward_sort[i] -> str[prefix_len];
      j = prefix -> first_prel;
      if ((j == 0) || (target_phrases2[prefix -> me].next_phrase_list[j - 1].nextsym != nextsym)) {
	if (j == 0) {
	  target_phrases2[prefix -> me].next_phrase_list = wmalloc ((j + NEXT_PHRASE_INC) * sizeof (NEXTPHRASE));
	  count += NEXT_PHRASE_INC;
	}
	else if (j % NEXT_PHRASE_INC == 0) {
	  target_phrases2[prefix -> me].next_phrase_list = wrealloc (target_phrases2[prefix -> me].next_phrase_list, (j + NEXT_PHRASE_INC) * sizeof (NEXTPHRASE));
	  count += NEXT_PHRASE_INC;
        }
	target_phrases2[prefix -> me].next_phrase_list[j].nextsym = nextsym;
	target_phrases2[prefix -> me].next_phrase_list[j].ph = forward_sort[i];
	prefix -> first_prel++;
      }
    }
  }

  /*  Re-allocate space so that the exact amount is required  */
  for (i = 0; i < num_primitives + num_phrases; i++) {
    if (forward_sort[i] -> first_prel != 0) {
      if (forward_sort[i] -> first_prel % NEXT_PHRASE_INC != 0) {
	target_phrases2[forward_sort[i] -> me].next_phrase_list = wrealloc (target_phrases2[forward_sort[i] -> me].next_phrase_list, forward_sort[i] -> first_prel * sizeof (NEXTPHRASE));
	count2 += forward_sort[i] -> first_prel;
      }
    }
  }

  for (i = 0; i < num_primitives + num_phrases; i++) {
    if (target_phrases2[forward_sort[i] -> me].prefix_phrase != NULL) {
      target_phrases2[target_phrases2[forward_sort[i] -> me].prefix_phrase -> me].last_prefix_phrase = forward_sort[i];
    }
  }

#ifdef DEBUG
  fprintf (stderr, "\tAllocated %u structures in linkNextPhrase. (%u)\n", count, count2);
#endif

  return;
}


void linkSecondaryPrefixPhrase (PROG_INFO *prog_struct, BLOCK_INFO *block_struct) {
  R_UINT num_primitives = (block_struct -> target_prel) -> num_primitives;
  R_UINT num_phrases = (block_struct -> target_prel) -> num_phrases;
  R_UINT total_syms = num_primitives + num_phrases;

  NEWPAIR1 **forward_sort = (block_struct -> target_prel) -> forward_sort1;

  NEWPAIR1 *candidate = NULL;
  NEWPAIR1 *temp_candidate = NULL;
  R_UINT candidate_len = 0;
  R_UINT i = 0;
  R_UINT j = 0;
  R_UINT k = 0;
  R_UINT nextsym = 0;
  R_UINT curr_second_sym = UINT_MAX;
  R_UINT prev_second_sym = UINT_MAX;
  R_BOOLEAN completely_done = R_FALSE;
  R_BOOLEAN done = R_FALSE;
  R_UINT next_phrase_count = 0;
  R_UINT match_pos = 0;
  NEWPAIR2 *target_phrases2 = (block_struct -> target_prel) -> phrases2;

  for (i = 0; i < total_syms; i++) {
    prev_second_sym = curr_second_sym;

    if (forward_sort[i] -> generation == 0) {
      curr_second_sym = UINT_MAX;
      candidate = NULL;
    }
    else if (target_phrases2[forward_sort[i - 1] -> me].secondary_phrase == NULL) {
      curr_second_sym = forward_sort[i] -> str[1];
      candidate = forward_sort[block_struct -> target_prel -> prims_lookup[curr_second_sym]];
    }
    else {
      /*  Guaranteed to not index out of array because the first symbol in
      **  the forward-sorted phrase hierarchy is a primitive  */
      curr_second_sym = forward_sort[i] -> str[1];
      if (curr_second_sym == prev_second_sym) {
	candidate = target_phrases2[forward_sort[i - 1] -> me].secondary_phrase;
	/*  Previous phrase's secondary phrase may be too long; if so, need
	**  to go up prefix phrases.  */
	while (R_TRUE) {
	  /*  If the candidate secondary phrase is longer, then go to its 
	  **  prefix phrase.  */
          if (candidate -> len > (forward_sort[i] -> len - 1)) {
	    candidate = target_phrases2[candidate -> me].prefix_phrase;
	  }
	  else {
	    /*  Check that the prefix phrase matches the current phrase  */
	    for (j = candidate -> len; j != 0; j--) {
	      if (forward_sort[i] -> str[j] != candidate -> str[j - 1]) {
		break;
	      }
	    }
	    if (j != 0) {
	      candidate = target_phrases2[candidate -> me].prefix_phrase;
	    }
	    else {
	      break;                         /*  break out of while loop  */
	    }
	  }
	}
      }
      else {
        candidate = forward_sort[block_struct -> target_prel -> prims_lookup[curr_second_sym]];
      }
    }

    /*  Follow next_phrase pointers to find a longer match  */
    completely_done = R_FALSE;
    if (forward_sort[i] -> len > 2) {
      while (completely_done == R_FALSE) {
        candidate_len = candidate -> len;
	if (candidate_len + 1 >= forward_sort[i] -> len) {
	  completely_done = R_TRUE;
	  break;
	}
        nextsym = forward_sort[i] -> str[candidate_len + 1];
        next_phrase_count = candidate -> first_prel;
        done = R_TRUE;
        for (k = 0; k < next_phrase_count; k++) {
          if (nextsym == target_phrases2[candidate -> me].next_phrase_list[k].nextsym) {
            temp_candidate = target_phrases2[candidate -> me].next_phrase_list[k].ph;
            done = R_FALSE;
            break;
          }
        }
        if (done == R_TRUE) {
          break;
        }

        if (temp_candidate -> len == (target_phrases2[temp_candidate -> me].prefix_phrase -> len + 1)) {
          candidate = temp_candidate;
        }
        else {
	  /*          match_pos = temp_candidate -> prefix_phrase -> len + 1;*/
          match_pos = target_phrases2[temp_candidate -> me].prefix_phrase -> len + 1;
          j = temp_candidate -> forward_sort_backptr;
          done = R_FALSE;
          while (done == R_FALSE) {
            if (j >= total_syms) {
	      temp_candidate = forward_sort[i];
	      done = R_FALSE;
	      while (done == R_FALSE) {
	        for (k = 0; k < temp_candidate -> len; k++) {
                  if (forward_sort[i] -> str[k + 1] != temp_candidate -> str[k]) {
		    break;
	          }
	        }
	        if (k == temp_candidate -> len) {
	          break;
	        }
	        else {
	          temp_candidate = target_phrases2[temp_candidate -> me].prefix_phrase;
	        }
	      }
    	      candidate = temp_candidate;
              completely_done = R_TRUE;
	      break;
            }
            else if (match_pos >= (forward_sort[j] -> len)) {
              completely_done = R_TRUE;
	      break;
            }
            else if (match_pos >= (forward_sort[i] -> len - 1)) {
              completely_done = R_TRUE;
	      break;
            }
            else if (forward_sort[i] -> str[match_pos + 1] < forward_sort[j] -> str[match_pos]) {
              completely_done = R_TRUE;
	      break;
            }
            else if (forward_sort[i] -> str[match_pos + 1] == forward_sort[j] -> str[match_pos]) {
              if (match_pos == (forward_sort[j] -> len - 1)) {
        	for (k = 0; k < forward_sort[j] -> len; k++) {
	          if (forward_sort[i] -> str[k + 1] != forward_sort[j] -> str[k]) {
	            break;
  	          }
	        }
                if (k == forward_sort[j] -> len) {
	          candidate = forward_sort[j];
	          done = R_TRUE;
	        }
	        else {
	          completely_done = R_TRUE;
		  break;
	        }
	        break;
	      }
	      else {
	        match_pos++;
	      }
	    }
	    else {
              j++;
            }
	  }
	}
      }
    }
    target_phrases2[forward_sort[i] -> me].secondary_phrase = candidate;
  }

  return;
}


void linkPhraseHierarchy (PROG_INFO *prog_struct, BLOCK_INFO *block_struct) {
  struct tms currtime;
  R_UINT clockticks;
  R_UINT num = 0;

  clockticks = (R_UINT) sysconf (_SC_CLK_TCK);

  /*
  **  Erase old phrase hierarchy in order to save memory
  **  for phases 2b and 2c.
  */
  if (prog_struct -> merge_level != APPEND) {
    wfree (block_struct -> source_prel_1 -> phrases);
    block_struct -> source_prel_1 -> phrases = NULL;
  }

  (void) times (&currtime);
#ifdef DEBUG
  fprintf (stderr, "\tBegin creating primitives index:  %f\n", (R_FLOAT) currtime.tms_utime / (R_FLOAT) clockticks);
#endif
  linkPrimitivesLookup (prog_struct, block_struct);

  (void) times (&currtime);
#ifdef DEBUG
  fprintf (stderr, "\tBegin linking primary prefix symbols:  %f\n", (R_FLOAT) currtime.tms_utime / (R_FLOAT) clockticks);
#endif
  linkPrimaryPrefixPhrase (prog_struct, block_struct);

  /*  Only phase 2b needs linking  */
  if ((prog_struct -> merge_level == TWOB) || (prog_struct -> merge_level == APPEND)) {
    (void) times (&currtime);
#ifdef DEBUG
    fprintf (stderr, "\tBegin linking extended symbols:  %f\n", (R_FLOAT) currtime.tms_utime / (R_FLOAT) clockticks);
#endif
    linkNextPhrase (prog_struct, block_struct);

    (void) times (&currtime);
#ifdef DEBUG
    fprintf (stderr, "\tBegin linking secondary prefix symbols:  %f\n", (R_FLOAT) currtime.tms_utime / (R_FLOAT) clockticks);
#endif
    linkSecondaryPrefixPhrase (prog_struct, block_struct);
    num = block_struct -> target_prel -> num_primitives + block_struct -> target_prel -> num_phrases;
  }

  (void) times (&currtime);
#ifdef DEBUG
  fprintf (stderr, "\tLinking complete:  %f\n", (R_FLOAT) currtime.tms_utime / (R_FLOAT) clockticks);
#endif

  return;
}

 
