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
#include <limits.h>                                         /*  UINT_MAX  */
#include <sys/stat.h>                                           /*  stat  */

#include "common-def.h"
#include "wmalloc.h"
#include "error.h"
#include "remerge-defn.h"
#include "remerge.h"
#include "phase_2_all.h"
#include "io.h"
#include "phase_a.h"


void reEncodeSequence_Append (PROG_INFO *prog_struct, BLOCK_INFO *block_struct, R_UINT block_id) {
  R_INT x = 0;
  R_UINT y = 0;
  R_UINT z = 0;
  R_UINT symbols_read;
  R_UINT symbols_written;
  NEWPAIR1 *target_phrases = (block_struct -> target_prel) -> phrases1;
  NEWPAIR1 **gen_unit_sort = (block_struct -> target_prel) -> gen_unit_sort1;
  NEWPAIR1 **forward_sort = (block_struct -> target_prel) -> forward_sort1;
  NEWPAIR2 *target_phrases2 = (block_struct -> target_prel) -> phrases2;

  R_INT eob_found = 0;

  R_UINT i = 0;
  R_UINT new_path_length;

  R_UINT *expand_buffer = NULL;
  CHAR_NODE *char_buffer = NULL;
  R_UINT buffer_start = 0;
  R_UINT buffer_end = (R_UINT) ((MAX_CHAR_BUFFER) - 1);
  R_UINT segment_start = buffer_start;
  R_UINT segment_end = buffer_start;
  R_UINT check_position = segment_start;
  R_UINT max_position = segment_start;
  R_INT longest_phrase = (R_INT) block_struct -> longest_phrase;

  R_UINT phrase_len = 0;

  MALLOC_STRUCT *check_node_info = NULL;
  CHECK_NODE **check_node_list;
  CHECK_NODE *newnode;

  NEWPAIR1 *curr_phrase = NULL;
  R_UINT ph_cmp_result = 0;

  OLDPAIR *source_phrases;
  R_UINT *in_seq_count = 0;
  R_UINT *out_seq_count = &(block_struct -> sprime);

  /*  Variables used to count the number of segments
  **  and distance between synchronization points  */
  R_UINT total_segment_length = 0;
  R_UINT segment_count = 0;

  R_UINT in_stdin_count = 0;
  R_BOOLEAN eof_found = R_FALSE;

  struct stat stbuf;
  R_UINT filesize = 0;
  R_UINT *temp_in_buf;

  /*  Assume block_id == 1  */
  source_phrases = block_struct -> source_prel_1 -> phrases;
  in_seq_count = &(block_struct -> s2);

  check_node_list = wmalloc (MAX_CHECK_NODE * sizeof (CHECK_NODE*));
  for (i = 0; i < MAX_CHECK_NODE; i++) {
    check_node_list[i] = wmalloc (MAX_CHECK_NODE * sizeof (CHECK_NODE));
  }
  check_node_info = init_NodeArray (MAX_CHECK_NODE, MAX_CHECK_NODE);

  expand_buffer = wmalloc (sizeof (R_UINT) * (longest_phrase + 1));
  char_buffer = wmalloc (sizeof (CHAR_NODE) * ((MAX_CHAR_BUFFER) + 1));
  char_buffer[MAX_CHAR_BUFFER].ch = '\0';

  /*  Initialise all minor_cut_vertex values to 0, in case punctuation
  **  flags are not used.  */
  for (i = 0; i < (R_UINT) (MAX_CHAR_BUFFER + 1); i++) {
    char_buffer[i].minor_cut_vertex = 0;
    char_buffer[i].prev_cut = IS_NOT_CUT;
  }

  temp_in_buf = wmalloc (sizeof (R_UINT) * SEQ_BUF_SIZE);

  (void) stat (prog_struct -> old_seq_name, &stbuf);
  filesize = (R_UINT) stbuf.st_size;
  filesize = (filesize / 4);
  (block_struct -> s1) += filesize;

  /*  Transfer S1 to S'  */
  while (filesize != 0) {
    symbols_read = (R_UINT) fread (temp_in_buf, sizeof (*(temp_in_buf)), SEQ_BUF_SIZE, prog_struct -> in_seq);

    filesize -= symbols_read;
    if (filesize != 0) {
      symbols_written = (R_UINT) fwrite (temp_in_buf, sizeof (*(temp_in_buf)), SEQ_BUF_SIZE, prog_struct -> out_seq);
    }
    else {
      symbols_written = (R_UINT) fwrite (temp_in_buf, sizeof (*(temp_in_buf)), (size_t) (symbols_read - 1), prog_struct -> out_seq);
    }
  }

  while (R_TRUE) {
    if (eob_found != -1) {
      /*  Check if input buffer is empty  */
      if ((eof_found == R_FALSE) && (prog_struct -> in_buf_p == prog_struct -> in_buf_end)) {
	prog_struct -> in_buf_end = prog_struct -> in_buf;

	/*  If there is space left in the buffer, fill it with stdin  */
        while ((eof_found == R_FALSE) && (prog_struct -> in_buf_end != prog_struct -> in_buf + SEQ_BUF_SIZE)) {
          x = fgetc (stdin);
          in_stdin_count++;
          if (x == EOF) {
	    *(prog_struct -> in_buf_end) = 0;
	    prog_struct -> in_buf_end++;
            eof_found = R_TRUE;
            break;
	  }
	  if (block_struct -> target_prel -> prims_lookup[x] == UINT_MAX) {
	    fprintf (stderr, "Undefined character %d (%s, %u)\n", x, __FILE__, __LINE__);
	    exit (EXIT_FAILURE);
	  }
	  *(prog_struct -> in_buf_end) = forward_sort[block_struct -> target_prel -> prims_lookup[x]] -> new_me + 1;
          prog_struct -> in_buf_end++;
        }
        prog_struct -> in_buf_p = prog_struct -> in_buf;
      }

      x = (R_INT) ((*(prog_struct -> in_buf_p)) - 1);
      z = 0;
      (prog_struct -> in_buf_p)++;
      (*in_seq_count)++;
    }

    if (x != INT_MAX) {
      y = target_phrases[x].replacement_phrase;
      y = target_phrases[y].new_me;

      phrase_len = target_phrases[y].len;

      /*  If char_buffer does not have room for the next phrase, then 
      **  copy segment to the beginning  */
      if (segment_end + phrase_len >= buffer_end) {
	if (segment_start == 0) {
	  fprintf (stderr, "char_buffer overflow in %s [%u]\n", __FILE__, __LINE__);
	  exit (EXIT_FAILURE);
	}
	for (i = segment_start; i < segment_end; i++) {
	  char_buffer[i - segment_start].colour = char_buffer[i].colour;
	  char_buffer[i - segment_start].distance = char_buffer[i].distance;
	  char_buffer[i - segment_start].parent = char_buffer[i].parent;
	  char_buffer[i - segment_start].phrase_id = char_buffer[i].phrase_id;
	  char_buffer[i - segment_start].next_queue = char_buffer[i].next_queue;
	  char_buffer[i - segment_start].ch = char_buffer[i].ch;
	  char_buffer[i - segment_start].edge_list = char_buffer[i].edge_list;
	  char_buffer[i - segment_start].edge_count = char_buffer[i].edge_count;
	  char_buffer[i - segment_start].search_start = char_buffer[i].search_start;
	  char_buffer[i - segment_start].cut_vertex = char_buffer[i].cut_vertex;
	}
	check_position -= segment_start;
	max_position -= segment_start;
	segment_end = i - segment_start;
	segment_start = 0;
      }


      /*  Expand starting positions for the current phrase  */
      expandPhraseToBuffer (block_struct, char_buffer + segment_end, expand_buffer, y, z);

      /*  Expand phrase to primitives  */
      for (i = 0; i < phrase_len; i++) {
        char_buffer[segment_end].colour = WHITE;
        char_buffer[segment_end].distance = UINT_MAX;
        char_buffer[segment_end].parent = NULL;
        char_buffer[segment_end].phrase_id = UINT_MAX;
        char_buffer[segment_end].next_queue = NULL;
        char_buffer[segment_end].ch = (R_CHAR) target_phrases[y].str[i];
        char_buffer[segment_end].edge_list = NULL;
        char_buffer[segment_end].edge_count = 0;
	/*
	**  Note:  char_buffer's search_start and cut_vertex set 
        **  by expandPhraseToBuffer
	*/
        segment_end++;
      }
    }
    else {
      char_buffer[segment_end].cut_vertex = EOB;
      eob_found = -1;
    }

    /*  Continue loop while segment is large enough or x is the end of
    **  block marker  */
    while ((((R_INT) segment_end - (R_INT) (check_position + target_phrases[char_buffer[check_position].search_start].len + 1)) >= longest_phrase) || (eob_found == -1)) {
      /*  If end of block reached  */
      if (char_buffer[check_position].cut_vertex == EOB) {
	/*  Need to initialize the last node  */
        char_buffer[segment_end].colour = WHITE;
        char_buffer[segment_end].distance = UINT_MAX;
        char_buffer[segment_end].parent = NULL;
        char_buffer[segment_end].phrase_id = UINT_MAX;
        char_buffer[segment_end].next_queue = NULL;
        char_buffer[segment_end].edge_list = NULL;
        char_buffer[segment_end].edge_count = 0;

        new_path_length = findShortestPath (char_buffer, segment_start, segment_end, prog_struct, check_node_info);
        (*out_seq_count) += new_path_length;

	/*  Add end of block marker  */
        y = 0;
        *(prog_struct -> out_buf_p) = y;
        prog_struct -> out_buf_p++;
        (*out_seq_count)++;

        if (prog_struct -> out_buf_p == prog_struct -> out_buf_end) {
	  symbols_read = writeBuffer (prog_struct, (R_FLOAT) 1);
        }

        wfree (check_node_info);
        for (i = 0; i < MAX_CHECK_NODE; i++) {
          wfree (check_node_list[i]);
        }
        wfree (check_node_list);
        wfree (expand_buffer);
        wfree (char_buffer);
        wfree (temp_in_buf);

        fprintf (stderr, "\tin_stdin_count:  %u\n", in_stdin_count);
        return;
      }

      /*  Search alphabetically sorted list to find a longer match  */
      ph_cmp_result = link_ph_cmp (block_struct, gen_unit_sort[char_buffer[check_position].search_start] -> forward_sort_backptr, char_buffer, check_position, segment_end);

      curr_phrase = forward_sort[ph_cmp_result];

      /*  Traverse prefix phrases to find all matching phrases  */
      while (curr_phrase != NULL) {
	if ((check_position + curr_phrase -> len) > max_position) {
	  max_position = check_position + curr_phrase -> len;
	}
        newnode = insertPointerNode (check_node_list, check_node_info, curr_phrase -> new_me, curr_phrase -> len);
        newnode -> next = char_buffer[check_position].edge_list;
        char_buffer[check_position].edge_list = newnode;
        char_buffer[check_position].edge_count++;
        curr_phrase = target_phrases2[curr_phrase -> me].prefix_phrase;
      }

      check_position++;

      /*  Synchronization point found, so process segment  */
      if (((check_position == max_position) || (char_buffer[check_position].cut_vertex == IS_CUT)) && (eob_found != -1)) {
        total_segment_length += (max_position - segment_start);
        segment_count++;
        new_path_length = findShortestPath (char_buffer, segment_start, max_position, prog_struct, check_node_info);
        (*out_seq_count) += new_path_length;
        char_buffer[max_position].colour = WHITE;
        char_buffer[max_position].distance = UINT_MAX;
        char_buffer[max_position].parent = NULL;
        char_buffer[max_position].phrase_id = UINT_MAX;
        char_buffer[max_position].next_queue = NULL;
        char_buffer[max_position].edge_list = NULL;
        char_buffer[max_position].edge_count = 0;
	segment_start = max_position;
      }
    }
  }

  return;
}
