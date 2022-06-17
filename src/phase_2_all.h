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

#ifndef PHASE_2ALL_H
#define PHASE_2ALL_H

#define MAX_CHECK_NODE 1000

#define MAX_SEQ_BUFFER 32767
#define MAX_CHAR_BUFFER (MAX_SEQ_BUFFER << 2)

#define NEXT_PHRASE_INC 8

enum NODE_COLOUR {WHITE = 0, GRAY = 1, BLACK = 2};

/*  Cut vertex status of a char_node  */
enum R_CUT_VERTEX { IS_NOT_CUT = 0, IS_CUT = 1, EOB = 2 };

#define IS_NOT_EMPTY(x) ((phrase_list -> head == NULL) ? 0 : 1)

#define GET_LEFT(x) gen_unit_sort[x] -> left_child
#define GET_RIGHT(x) gen_unit_sort[x] -> right_child

#define GET_PHRASE_LEN(x) (gen_unit_sort[x] -> len)

typedef struct malloc_struct {
  R_UINT max_x;
  R_UINT max_y;
  R_UINT curr_x;
  R_UINT curr_y;
  R_UINT max_count;
  R_UINT curr_count;
} MALLOC_STRUCT;


typedef struct phrase_queue_node {
  R_UINT phrase_id;
  R_UINT offset;
  struct phrase_queue_node *next;
} PHRASE_QUEUE_NODE;


typedef struct check_node {
  R_UINT phrase_id;
  R_UINT phrase_len;
  struct check_node *next;
} CHECK_NODE;


typedef struct phrase_list {
  PHRASE_QUEUE_NODE *head;
  PHRASE_QUEUE_NODE *tail;
  MALLOC_STRUCT *list_info;
  PHRASE_QUEUE_NODE *list;
} PHRASE_LIST;


typedef struct nextphrase {
  R_UINT nextsym;
  struct newpair1 *ph;
} NEXTPHRASE;

typedef struct char_node {
  R_UINT ch;
  R_UINT edge_count;
  CHECK_NODE *edge_list;
  enum NODE_COLOUR colour;                   /*  color in Cormen et. al.  */
  R_UINT distance;                               /*  d in Cormen et. al.  */
  struct char_node *parent;                     /*  pi in Cormen et. al.  */
  R_UINT phrase_id;       /*  The phrase id required to get to this node  */
  struct char_node *next_queue;    
                             /*  Used for queue in finding shortest path  */
  R_UINT search_start;
  /*  Starting point for searching, as a numeric index into gen_unit_sort */
  enum R_CUT_VERTEX cut_vertex;

  /*  Used for finding a shortest path for punctuation-aligned Re-Pair  */
  R_UINT minor_cut_vertex;
  enum R_CUT_VERTEX prev_cut;
} CHAR_NODE;

R_INT binary_ph_cmp (BLOCK_INFO *block_struct, R_UINT phrase_pos, CHAR_NODE *char_buffer, R_UINT check_position, R_UINT segment_end);
R_UINT link_ph_cmp (BLOCK_INFO *block_struct, R_UINT phrase_pos, CHAR_NODE *char_buffer, R_UINT check_position, R_UINT segment_end);

MALLOC_STRUCT *init_NodeArray (R_UINT max_x, R_UINT max_y);
CHECK_NODE *insertPointerNode (CHECK_NODE **check_node_list, MALLOC_STRUCT *check_node_info, R_UINT phrase_id, R_UINT phrase_len);
R_UINT findShortestPath (CHAR_NODE *char_buffer, R_UINT segment_start, R_UINT segment_end, PROG_INFO *prog_struct, MALLOC_STRUCT *check_node_info);

void expandPhraseToBuffer (BLOCK_INFO *block_struct, CHAR_NODE *char_buffer, R_UINT *expand_buffer, R_UINT y, R_UINT z);

/*  Inter-link the phrase hierarchy  */
void linkPrimitivesLookup (PROG_INFO *prog_struct, BLOCK_INFO *block_struct);
void linkPrimaryPrefixPhrase (PROG_INFO *prog_struct, BLOCK_INFO *block_struct);
void linkNextPhrase (PROG_INFO *prog_struct, BLOCK_INFO *block_struct);
void linkSecondaryPrefixPhrase (PROG_INFO *prog_struct, BLOCK_INFO *block_struct);
void linkPhraseHierarchy (PROG_INFO *prog_struct, BLOCK_INFO *block_struct);

#endif
