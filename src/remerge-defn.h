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

#ifndef REMERGE_DEFN_H
#define REMERGE_DEFN_H

/********************************************************************
Forward declaration of important structures defined in other files
********************************************************************/
struct bitinrec;                                /*  bitinput.h  */
struct sizenode;
struct gennode;
struct oldpair;
struct newpair1;
struct newpair2;
struct newpair3;
struct block;
struct newblock;
struct mapnode;
struct nextphrase;                          /*  phrase_2_all.h  */

/*  Merge levels  */
enum R_MERGE_LEVEL {NOMERGE = 0, ONE = 1, TWOA = 2, TWOB = 3, THREE = 6, APPEND = 7};

/*  Use punctuation flags  */
enum R_USE_WORDFLAGS { UW_NO = 0, UW_YES = 1 };

/********************************************************************
Definitions
********************************************************************/
#define UINT_SIZE_BITS 32

#define SEQ_BUF_SIZE (262144) /* (1 << 18) */
#define OUT_BUF_SIZE (262144) /* (1 << 18) */

#define WORD_FLAG 0x00000000    /*  0000 0...  */
#define PUNC_FLAG 0x80000000    /*  1000 0...  */
#define NO_FLAGS 0x7FFFFFFF     /*  Everything but the top bit  */

#define MAXIMUM(x,y) (x > y ? x : y)
#define MINIMUM(x,y) (x > y ? y : x)

/********************************************************************
Structure definitions
********************************************************************/
/*
**  Structure to be filled in using command line arguments (or
**  directly through other means).  The meaning of each variable is
**  mentioned in the other structure definitions below.
*/
typedef struct args_info {
  R_CHAR *progname;
  R_CHAR *base_filename;
  R_BOOLEAN verbose_level;
  enum R_MERGE_LEVEL merge_level;
  enum R_USE_WORDFLAGS word_flags;
  R_BOOLEAN dowordlen;
} ARGS_INFO;


typedef struct prog_info {
  R_CHAR *progname;                           /*  Program name  */
  R_CHAR *base_filename;
  R_BOOLEAN verbose_level;
  enum R_MERGE_LEVEL merge_level;
  enum R_USE_WORDFLAGS word_flags;
  R_BOOLEAN dowordlen;

  FILE *in_prel;
  FILE *in_seq;
  FILE *out_prel;
  FILE *out_seq;

  R_CHAR *old_prel_name;
  R_CHAR *old_seq_name;
  R_CHAR *new_prel_name;
  R_CHAR *new_seq_name;

  R_UINT *in_buf;
  R_UINT *in_buf_end;
  R_UINT *in_buf_p;

  R_UINT *out_buf;
  R_UINT *out_buf_end;
  R_UINT *out_buf_p;

  R_UINT *pflag_buf;
  R_UINT *pflag_buf_end;
  R_UINT *pflag_buf_p;

  struct bitinrec *bit_in_rec;

  R_UINT block_count;
  R_UINT p1_acc;
  R_UINT p2_acc;
  R_UINT pstar_acc;
  R_UINT pprime_acc;
  R_UINT s1_acc;
  R_UINT s2_acc;
  R_UINT sprime_acc;

  /*
  struct memroot *hashpair_root;
  struct uimemroot *finalstr_root;
  */

  /*  Output buffer for phase 2b  */
  R_UINT *path_buffer;
  R_UINT path_buffer_size;

  ARGS_INFO *args_struct;
           /*  Structure of arguments passed from command line  */
             /*  Should be NULL if no arguments were passed in  */
} PROG_INFO;


typedef struct gennode {
  R_UINT size;
  R_UINT cumm_size;
  R_UINT **bitmaps;
} GENNODE;

typedef struct block_info {
  struct block *source_prel_1;
  struct block *source_prel_2;
  R_UINT max_target_prel_size;

  R_UINT p1;
  R_UINT p2;
  R_UINT pstar;
  R_UINT pprime;
  R_UINT s1;
  R_UINT s2;
  R_UINT sprime;

  struct newblock *target_prel;
  struct mapnode *target_acc;
  R_UINT longest_phrase;
      /*  Length of the longest phrase in the merged hierarchy  */
} BLOCK_INFO;
    

typedef struct sizenode {
  R_UINT value;
  struct sizenode *next;
} SIZENODE;

typedef struct oldpair {
  R_UINT left_child;
  R_UINT right_child;
  R_UINT generation;
  R_UINT new_prel;
  R_ULL_INT chiastic;
  R_UINT len;
} OLDPAIR;


typedef struct newpair1 {
  R_UINT me;                 /*  Position in the phrases array  */
  R_UINT new_me;       /*  Position in the gen_unit_sort array  */
          /*  new_me is output to file as the phrase hierarchy  */
  R_UINT forward_sort_backptr;
                        /*  Position in the forward_sort array  */
         /*  (i.e.  Position of the phrase after it is sorted)  */
  R_UINT left_child;
  R_UINT right_child;
  R_UINT generation;
      /*  generation is UINT_MAX if this pair is a duplicate and removed  */
  R_UINT replacement_phrase;
  /*  The phrase that replaces this one (itself if no replacement exists) */
  /*  The replacement_phrase index refers to the phrases array.  */

  R_UINT first_prel;
/*  Phase 1:  Used as a pointer to the first prelude.  */
/* Phase 2a: Used as a next pointer in the hash table chaining.  */
             /*  Phase2b/2c:  Used to count number of  */
             /*  (struct nextphrase) nodes are in use  */
  R_UINT second_prel;
         /*  Phase 1:  Used as a pointer to the second prelude.  */
  /*  Phase 2:  Used as a counter of how many phrases  */
                           /*  use this one directly.  */
  R_ULL_INT chiastic;
  R_UINT *str;
  R_UINT len;
  /*  Is this the root string during phrase expansion?  */
  R_UCHAR isroot;  
} NEWPAIR1;


typedef struct newpair2 {
  R_UINT me;
  struct newpair1 *prefix_phrase;
  struct newpair1 *last_prefix_phrase;
  struct newpair1 *secondary_phrase;
  struct nextphrase *next_phrase_list;
} NEWPAIR2;


typedef struct newpair3 {
  R_UINT me;                 /*  Position in the phrases array  */
  R_UINT new_me;       /*  Position in the gen_unit_sort array  */
          /*  new_me is output to file as the phrase hierarchy  */
  R_UINT left_child;
  R_UINT right_child;
  R_UINT generation;
      /*  generation is UINT_MAX if this pair is a duplicate and removed  */

  R_ULL_INT chiastic;
} NEWPAIR3;


typedef struct block {
  OLDPAIR *phrases;
  struct gennode *generation_array;
  R_UINT num_primitives;
  R_UINT num_phrases;
  R_UINT num_generation;
} BLOCK;

typedef struct newblock {
  NEWPAIR1 *phrases1;
  NEWPAIR1 **forward_sort1;
  NEWPAIR1 **gen_unit_sort1;
  NEWPAIR1 **duplicates_sort1;

  NEWPAIR2 *phrases2;

  NEWPAIR3 *phrases3;
  NEWPAIR3 **gen_unit_sort3;

  R_UINT *prims_lookup;        /*  Lookup table for primitives  */
  R_UINT prims_lookup_size;   /*  Also, the maximum chiastic slide value for
			      **  the last primitive  */

  R_UINT num_primitives;
  R_UINT num_phrases;
  R_UINT num_generation;
  R_UINT num_duplicates;
  R_ULL_INT kp;
  R_ULL_INT kpp;
  R_ULL_INT kppSqr;
  SIZENODE *sizelist;
  SIZENODE *end;
  R_UINT *target_prel_hashtable;
} NEWBLOCK;

#endif

