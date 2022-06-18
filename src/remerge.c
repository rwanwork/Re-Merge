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
#include <limits.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "common-def.h"
#include "wmalloc.h"
#include "error.h"
#include "remerge-defn.h"

#include "bitin.h"
#include "phrase-slide-decode.h"
#include "merge-phrases.h"
#include "sort-phrases.h"
#include "phase_1.h"
#include "phase_2_all.h"
#include "phase_2a.h"
#include "phase_2b.h"
#include "phase_3.h"
#include "phase_a.h"
#include "remerge.h"

/*
**  Static functions
*/
static void transferNewPair1_2 (PROG_INFO *prog_struct, BLOCK_INFO *block_struct);
static void transferNewPair1_3 (PROG_INFO *prog_struct, BLOCK_INFO *block_struct);
static void usage (ARGS_INFO *args_struct);
static void intDecode (PROG_INFO *prog_struct, R_UINT a, R_UINT b, R_ULL_INT lo, R_ULL_INT hi, BLOCK *current);
static void initRemerge_OneBlock (PROG_INFO *prog_struct, BLOCK_INFO *block_struct);
static void uninitRemerge_OneBlock (PROG_INFO *prog_struct, BLOCK_INFO *block_struct);
static void decodeHierarchy_OneBlock (PROG_INFO *prog_struct, BLOCK_INFO *block_struct, R_UINT block_id);
static void displayStats_OneBlock (PROG_INFO *prog_struct, BLOCK_INFO *block_struct);


static void transferNewPair1_2 (PROG_INFO *prog_struct, BLOCK_INFO *block_struct) {
  unsigned int i = 0;
  unsigned int total_size = (block_struct -> target_prel) -> num_primitives + (block_struct -> target_prel) -> num_phrases;
  NEWPAIR2 *phrases2 = NULL;
  NEWPAIR1 *phrases1 = (block_struct -> target_prel) -> phrases1;

  phrases2 = wmalloc (sizeof (NEWPAIR2) * total_size);

  for (i = 0; i < total_size; i++) {
    phrases2[i].me = phrases1[i].me;
    phrases2[i].prefix_phrase = NULL;
    phrases2[i].last_prefix_phrase = NULL;
    phrases2[i].secondary_phrase = NULL;
    phrases2[i].next_phrase_list = NULL;
  }

  (block_struct -> target_prel) -> phrases2 = phrases2;

  return;
}


static void transferNewPair1_3 (PROG_INFO *prog_struct, BLOCK_INFO *block_struct) {
  unsigned int i = 0;
  unsigned int total_size = (block_struct -> target_prel) -> num_primitives + (block_struct -> target_prel) -> num_phrases;
  NEWPAIR3 *phrases3 = NULL;
  NEWPAIR1 *phrases1 = (block_struct -> target_prel) -> phrases1;

  phrases3 = wmalloc (sizeof (NEWPAIR3) * total_size);

  for (i = 0; i < total_size; i++) {
    phrases3[i].me = phrases1[i].me;
    phrases3[i].new_me = phrases1[i].new_me;
    phrases3[i].left_child = phrases1[i].left_child;
    phrases3[i].right_child = phrases1[i].right_child;
    phrases3[i].generation = phrases1[i].generation;
    phrases3[i].chiastic = phrases1[i].chiastic;
  }

  /*  Free NEWPAIR1  */
  wfree ((block_struct -> target_prel) -> phrases1);
  (block_struct -> target_prel) -> phrases1 = NULL;
  wfree ((block_struct -> target_prel) -> gen_unit_sort1);
  (block_struct -> target_prel) -> gen_unit_sort1 = NULL;
  wfree ((block_struct -> target_prel) -> forward_sort1);
  (block_struct -> target_prel) -> forward_sort1 = NULL;

  (block_struct -> target_prel) -> gen_unit_sort3 = wmalloc (sizeof (NEWPAIR3*) * total_size);
  for (i = 0; i < total_size; i++) {
    (block_struct -> target_prel) -> gen_unit_sort3[phrases3[i].new_me] = &phrases3[i];
  }

  (block_struct -> target_prel) -> phrases3 = phrases3;

  return;
}


void transferNewPair3_1 (PROG_INFO *prog_struct, BLOCK_INFO *block_struct, unsigned int phrases3_size) {
  unsigned int i = 0;
  NEWPAIR1 *phrases1 = NULL;
  NEWPAIR3 *phrases3 = (block_struct -> target_prel) -> phrases3;

  phrases1 = wmalloc (sizeof (NEWPAIR1) * phrases3_size);

  for (i = 0; i < phrases3_size; i++) {
    phrases1[i].me = phrases3[i].me;
    phrases1[i].new_me = phrases3[i].new_me;
    phrases1[i].forward_sort_backptr = UINT_MAX;
    phrases1[i].left_child = phrases3[i].left_child;
    phrases1[i].right_child = phrases3[i].right_child;
    phrases1[i].generation = phrases3[i].generation;
    phrases1[i].replacement_phrase = UINT_MAX;
    phrases1[i].first_prel = UINT_MAX;
    phrases1[i].second_prel = UINT_MAX;
    phrases1[i].chiastic = phrases3[i].chiastic;
    phrases1[i].str = NULL;
    if (i < (block_struct -> target_prel) -> num_primitives) {
      phrases1[i].len = 1;
    }
    else {
      phrases1[i].len = 0;
    }
    phrases1[i].isroot = (R_UCHAR) 0;
  }

  /*  Free NEWPAIR3  */
  wfree ((block_struct -> target_prel) -> phrases3);
  (block_struct -> target_prel) -> phrases3 = NULL;
  wfree ((block_struct -> target_prel) -> gen_unit_sort3);
  (block_struct -> target_prel) -> gen_unit_sort3 = NULL;

  (block_struct -> target_prel) -> gen_unit_sort1 = wmalloc (sizeof (NEWPAIR1*) * (block_struct -> pprime));
  for (i = 0; i < phrases3_size; i++) {
    /*  If new_me == UINT_MAX, then it isn't in gen_unit_sort  */
    if (phrases1[i].new_me != UINT_MAX) {
      (block_struct -> target_prel) -> gen_unit_sort1[phrases1[i].new_me] = &phrases1[i];
    }
  }

  (block_struct -> target_prel) -> phrases1 = phrases1;

  return;
}

static void usage (ARGS_INFO *args_struct) {
  fprintf (stderr, "Re-Merge\n");
  fprintf (stderr, "========\n\n");
  fprintf (stderr, "Usage:  %s [options]\n\n", args_struct -> progname);
  fprintf (stderr, "Options:\n");
  fprintf (stderr, "-f          : Use punctuation flags for word-based parsing.\n");
  fprintf (stderr, "-i <file>   : Input file. [Required]\n");
  fprintf (stderr, "-v          :  Verbose output\n");
  fprintf (stderr, "-p <option> : Merge phase <option>.  [Required]\n");
  fprintf (stderr, "\t1  : Complete phase hierarchy merge\n");
  fprintf (stderr, "\t2a : Basic sequence merge\n");
  fprintf (stderr, "\t2b : Fast optimal sequence merge\n");
  fprintf (stderr, "\t3  : Locate new phrases\n");
  fprintf (stderr, "\ta  : Append text via stdin\n");
  fprintf (stderr, "-w          :  Do word length counting to .wl file.\n");

  fprintf (stderr, "\nNotes:\n");
  fprintf (stderr, "* <file> is the name of the input file without any extension.\n");
  fprintf (stderr, "  Therefore, the files <file>.prel and <file>.seq are assumed\n");
  fprintf (stderr, "  to exist and will be overwritten!\n");
  fprintf (stderr, "* All phases, except for phase 1, need to be applied to a single\n");
  fprintf (stderr, "block only.  The program will crash otherwise.\n\n");
  fprintf (stderr, "Re-Merge version:  %s (%s)\n\n", __DATE__, __TIME__);
  exit (EXIT_FAILURE);
}


static void intDecode (PROG_INFO *prog_struct, R_UINT a, R_UINT b, R_ULL_INT lo, R_ULL_INT hi, BLOCK *current) {
  R_ULL_INT mid;
  R_UINT halfway;
  R_UINT range;

  range = b - a;
  switch (range) {
    case 0:  return;
    case 1:  (current -> phrases)[a].chiastic = binaryDecode (lo, hi, prog_struct -> bit_in_rec);
             return;
  }
  halfway = range >> 1;

  mid = binaryDecode (lo + (R_ULL_INT) halfway, hi - (R_ULL_INT) (range - halfway - 1), prog_struct -> bit_in_rec);
  (current -> phrases)[a + halfway].chiastic = mid;
  intDecode (prog_struct, a, a + halfway, lo, mid, current);
  intDecode (prog_struct, a + halfway + 1, b, mid + 1ull, hi, current);

  return;
}


static void initRemerge_OneBlock (PROG_INFO *prog_struct, BLOCK_INFO *block_struct) {
  uninitRemerge_OneBlock (prog_struct, block_struct);

  block_struct -> source_prel_1 = wmalloc (sizeof (BLOCK));
  (block_struct -> source_prel_1) -> phrases = NULL;
  (block_struct -> source_prel_1) -> generation_array = NULL;

  block_struct -> source_prel_2 = wmalloc (sizeof (BLOCK));
  (block_struct -> source_prel_2) -> phrases = NULL;
  (block_struct -> source_prel_2) -> generation_array = NULL;

  /*
  prog_struct -> finalstr_root = initUIMem ();
  */

  return;
}


static void uninitRemerge_OneBlock (PROG_INFO *prog_struct, BLOCK_INFO *block_struct) {
  prog_struct -> p1_acc += block_struct -> p1;
  prog_struct -> p2_acc += block_struct -> p2;
  prog_struct -> pstar_acc += block_struct -> pstar;
  prog_struct -> pprime_acc += block_struct -> pprime;
  prog_struct -> s1_acc += block_struct -> s1;
  prog_struct -> s2_acc += block_struct -> s2;
  prog_struct -> sprime_acc += block_struct -> sprime;

  if (block_struct -> source_prel_1 != NULL) {
    if ((block_struct -> source_prel_1) -> generation_array != NULL) {
      wfree ((block_struct -> source_prel_1) -> generation_array);
    }
    if ((block_struct -> source_prel_1) -> phrases != NULL) {
      wfree ((block_struct -> source_prel_1) -> phrases);
    }
    wfree (block_struct -> source_prel_1);

    /*
    **  Increment the block count depending on whether or not the second
    **  block existed.  If it did, then increment by 2, or else increment
    **  by 1.
    */
    if (block_struct -> p1 != 0) {
      if (block_struct -> p2 != 0) {
        prog_struct -> block_count += 2;
      }
      else {
        prog_struct -> block_count++;
      }
    }
  }
  block_struct -> source_prel_1 = NULL;

  if (block_struct -> source_prel_2 != NULL) {
    if ((block_struct -> source_prel_2) -> generation_array != NULL) {
      wfree ((block_struct -> source_prel_2) -> generation_array);
    }
    if ((block_struct -> source_prel_2) -> phrases != NULL) {
      wfree ((block_struct -> source_prel_2) -> phrases);
    }
    wfree (block_struct -> source_prel_2);
  }
  block_struct -> source_prel_2 = NULL;

  if (block_struct -> target_prel != NULL) {
    /*  NEWPAIR1  */
    if ((block_struct -> target_prel) -> phrases1 != NULL) {
      wfree ((block_struct -> target_prel) -> phrases1);
    }
    if (block_struct -> target_prel -> forward_sort1) {
      wfree (block_struct -> target_prel -> forward_sort1);
    }
    if (block_struct -> target_prel -> gen_unit_sort1) {
      wfree (block_struct -> target_prel -> gen_unit_sort1);
    }
    if (block_struct -> target_prel -> duplicates_sort1 != NULL) {
      wfree (block_struct -> target_prel -> duplicates_sort1);
    }
    (block_struct -> target_prel) -> phrases1 = NULL;
    (block_struct -> target_prel) -> forward_sort1 = NULL;
    (block_struct -> target_prel) -> gen_unit_sort1 = NULL;
    (block_struct -> target_prel) -> duplicates_sort1 = NULL;

    /*  NEWPAIR2  */
    if ((block_struct -> target_prel) -> phrases2 != NULL) {
      wfree ((block_struct -> target_prel) -> phrases2);
    }
    (block_struct -> target_prel) -> phrases2 = NULL;

    /*  NEWPAIR3  */
    if ((block_struct -> target_prel) -> phrases3 != NULL) {
      wfree ((block_struct -> target_prel) -> phrases3);
    }
    if (block_struct -> target_prel -> gen_unit_sort3) {
      wfree (block_struct -> target_prel -> gen_unit_sort3);
    }
    (block_struct -> target_prel) -> phrases3 = NULL;
    (block_struct -> target_prel) -> gen_unit_sort3 = NULL;

    if (block_struct -> target_prel -> prims_lookup != NULL) {
      wfree (block_struct -> target_prel -> prims_lookup);
    }
    wfree (block_struct -> target_prel);
  }
  block_struct -> target_prel = NULL;

  block_struct -> max_target_prel_size = 0;

  block_struct -> p1 = 0;
  block_struct -> p2 = 0;
  block_struct -> pstar = 0;
  block_struct -> pprime = 0;
  block_struct -> s1 = 0;
  block_struct -> s2 = 0;
  block_struct -> sprime = 0;

  return;
}


static void decodeHierarchy_OneBlock (PROG_INFO *prog_struct, BLOCK_INFO *block_struct, R_UINT block_id) {
  R_ULL_INT k1sqr, k2sqr;
  R_ULL_INT k1, k2;
  R_UINT generation_size;
  R_UINT num_phrases;
  R_UINT curr_gen = 0;
  R_UINT i = 0;
  BLOCK *current_block = NULL;

  if (block_id == 1) {
    current_block = block_struct -> source_prel_1;
  }
  else {
    current_block = block_struct -> source_prel_2;
  }
   
  current_block -> generation_array = wmalloc (sizeof (GENNODE) * MAX_GEN);

  /*  Number of primitives + Number of phrases  */
  num_phrases = deltaDecode (0, prog_struct -> bit_in_rec);
  if (num_phrases == 0) {
    if (block_id == 1) {
      block_struct -> p1 = 0;
    }
    else {
      block_struct -> p2 = 0;
    }

    current_block -> num_primitives = 0;
    current_block -> num_phrases = 0;

    return;
  }

  current_block -> phrases = wmalloc (num_phrases * sizeof (OLDPAIR));

  /*  Decode the primitives  */
  k2 = k2sqr = k1 = 0;
  generation_size = deltaDecode (1, prog_struct -> bit_in_rec);

  current_block -> generation_array[curr_gen].size = generation_size;
  curr_gen++;

  current_block -> num_primitives = generation_size;
  current_block -> num_phrases = num_phrases - (current_block -> num_primitives);

  intDecode (prog_struct, 0, generation_size, 0, (R_ULL_INT) (1ull << gammaDecode (0, prog_struct -> bit_in_rec)), current_block);
  setUnitPrimitives (generation_size, current_block -> phrases);

  current_block -> num_generation = 0;
   
  /*  Decode the phrases  */
  for (k1 = generation_size; k1 < num_phrases; k1 += generation_size) {
    /*  Size of the generation  */
    generation_size = gammaDecode(1, prog_struct -> bit_in_rec);

    current_block -> generation_array[curr_gen].size = generation_size;
    curr_gen++;

    /*  Decode pairs  */
    k1sqr =  (R_ULL_INT) k1 * (R_ULL_INT) k1;

    intDecode (prog_struct, (R_UINT) k1, (R_UINT) k1 + generation_size, 0, k1sqr - k2sqr, current_block);
    setUnitPhrasesChiastic (k1, k2, k1sqr, k2sqr, generation_size, current_block -> phrases);

    k2 = k1;
    k2sqr = k1sqr;
  }

  current_block -> generation_array[0].cumm_size = current_block -> generation_array[0].size;
  for (i = 1; i < curr_gen; i++) {
    current_block -> generation_array[i].cumm_size = current_block -> generation_array[i - 1].cumm_size + current_block -> generation_array[i].size;
  }

  current_block -> num_generation = curr_gen - 1;
  current_block -> generation_array = wrealloc (current_block -> generation_array, sizeof (GENNODE) * (curr_gen));

  if (block_id == 1) {
    block_struct -> p1 = num_phrases;
  }
  else {
    block_struct -> p2 = num_phrases;
  }

  return;
}


static void displayStats_OneBlock (PROG_INFO *prog_struct, BLOCK_INFO *block_struct) {
  R_CHAR* temp = NULL;

#ifdef DEBUG
  fprintf (stderr, "Statistics for current block:\n");
  fprintf (stderr, "\tPhrase hierarchy 1:\n");
  fprintf (stderr, "\t\tNumber of primitives and phrases:  %u\n", block_struct -> p1);
  fprintf (stderr, "\t\tSequence length:  %u\n", block_struct -> s1);
  fprintf (stderr, "\tPhrase hierarchy 2:\n");
  fprintf (stderr, "\t\tNumber of primitives and phrases:  %u\n", block_struct -> p2);
  fprintf (stderr, "\t\tSequence length:  %u\n", block_struct -> s2);
  fprintf (stderr, "\tMerged phrase hierarchy:\n");
  fprintf (stderr, "\t\tNumber of primitives and phrases:  %u\n", block_struct -> pprime);
  fprintf (stderr, "\tSequence length:  %u\n", block_struct -> sprime);
#endif
  
  if (block_struct -> p2 == 0) {
    fprintf (stdout, "%10u", prog_struct -> block_count);
  }
  else {
    temp = wmalloc (sizeof (R_CHAR) * 10);
    (void) snprintf (temp, (size_t) (sizeof (R_CHAR) * 10), "%u + %u", prog_struct -> block_count, prog_struct -> block_count + 1);
    fprintf (stdout, "%10s", temp);
    wfree (temp);
  }
  fprintf (stdout, "%8u%8u%8u%8u%10u%10u%10u\n", block_struct -> p1, block_struct -> p2, block_struct -> pstar, block_struct -> pprime, block_struct -> s1, block_struct -> s2, block_struct -> sprime);

  return;
}



ARGS_INFO *parseArguments (int argc, R_CHAR *argv[], ARGS_INFO *args_struct) {
  /*  Declarations required for getopt  */
  R_INT c;
  R_CHAR *temp_merge;

  args_struct -> progname = argv[0];
  args_struct -> base_filename = NULL;
  args_struct -> verbose_level = R_FALSE;
  args_struct -> merge_level = NOMERGE;
  args_struct -> word_flags = UW_NO;
  args_struct -> dowordlen = R_FALSE;

  /*  Print usage information if no arguments  */
  if (argc == 1) {
    usage (args_struct);
  }

   /*  Check arguments  */
  while (R_TRUE) {
    c = getopt (argc, argv, "fi:p:vw?");
    if (c == EOF) {
      break;
    }

    switch (c) {
    case 0:
      break;
    case 'f':
      args_struct -> word_flags = UW_YES;
      break;
    case 'i':
      args_struct -> base_filename = optarg;
      break;
    case 'p':
      temp_merge = optarg;
      if (strcmp (temp_merge, "1") == 0) {
        args_struct -> merge_level = ONE;
      }
      else if (strcmp (temp_merge, "2a") == 0) {
	args_struct -> merge_level = TWOA;
      }
      else if (strcmp (temp_merge, "2b") == 0) {
	args_struct -> merge_level = TWOB;
      }
      else if (strcmp (temp_merge, "3") == 0) {
	args_struct -> merge_level = THREE;
      }
      else if (strcmp (temp_merge, "a") == 0) {
	args_struct -> merge_level = APPEND;
      }
      else {
        fprintf (stderr, "Unrecognized option with -p:  %s.\n", temp_merge);
        exit (EXIT_FAILURE);
      }
      break;
    case 'v':
      args_struct -> verbose_level = R_TRUE;
      break;
    case 'w':
      args_struct -> dowordlen = R_TRUE;
      break;
    case '?':
      usage (args_struct);
      break;
    default:
      fprintf (stderr, "getopt returned erroneous character code.\n");
      exit (EXIT_FAILURE);
    }
  }

  if (optind < argc) {
    fprintf (stderr, "The following arguments were not valid:  ");
    while (optind < argc) {
      fprintf (stderr, "%s ", argv[optind++]);
    }
    fprintf (stderr, "\nRun %s with the -? option for help.\n", args_struct -> progname);
    exit (EXIT_FAILURE);
  }

  /*  Check for input filename  */
  if (args_struct -> base_filename == NULL) {
    fprintf (stderr, "Error.  Input filename required with the -i option.\n");
    fprintf (stderr, "Run %s with the -? option for help.\n", args_struct -> progname);
    exit (EXIT_FAILURE);
  }

  if (args_struct -> merge_level == NOMERGE) {
    fprintf (stderr, "Error.  A merge level is required.\n");
    fprintf (stderr, "Run %s with the -? option for help.\n", args_struct -> progname);
    exit (EXIT_FAILURE);
  }

  return (args_struct);
}



void initRemerge (PROG_INFO *prog_struct, BLOCK_INFO *block_struct) {
  R_CHAR *rename_cmd;
  R_UINT system_result;

  /*  Initialize values in PROG_INFO  */
  prog_struct -> progname = NULL;
  prog_struct -> base_filename = NULL;
  prog_struct -> verbose_level = R_FALSE;
  prog_struct -> merge_level = NOMERGE;
  prog_struct -> word_flags = UW_NO;
  prog_struct -> dowordlen = R_FALSE;

  prog_struct -> in_prel = NULL;
  prog_struct -> in_seq = NULL;
  prog_struct -> out_prel = NULL;
  prog_struct -> out_seq = NULL;
  prog_struct -> old_prel_name = NULL;
  prog_struct -> old_seq_name = NULL;
  prog_struct -> new_prel_name = NULL;
  prog_struct -> new_seq_name = NULL;
  prog_struct -> in_buf = NULL;
  prog_struct -> in_buf_end = NULL;
  prog_struct -> in_buf_p = NULL;
  prog_struct -> out_buf = NULL;
  prog_struct -> out_buf_end = NULL;
  prog_struct -> out_buf_p = NULL;
  prog_struct -> pflag_buf = NULL;
  prog_struct -> pflag_buf_end = NULL;
  prog_struct -> pflag_buf_p = NULL;

  prog_struct -> block_count = 1;
  prog_struct -> p1_acc = 0;
  prog_struct -> p2_acc = 0;
  prog_struct -> pstar_acc = 0;
  prog_struct -> pprime_acc = 0;
  prog_struct -> s1_acc = 0;
  prog_struct -> s2_acc = 0;
  prog_struct -> sprime_acc = 0;

  prog_struct -> bit_in_rec = NULL;

  prog_struct -> path_buffer_size = INIT_PATH_BUFFER_SIZE;
  prog_struct -> path_buffer = wmalloc (prog_struct -> path_buffer_size * sizeof (R_UINT));

  /*  Initialize values in BLOCK_INFO  */
  /*  Assumes that initRemerge_OneBlock will be run soon  */
  block_struct -> source_prel_1 = NULL;
  block_struct -> source_prel_2 = NULL;
  block_struct -> target_prel = NULL;
  block_struct -> p1 = 0;
  block_struct -> p2 = 0;
  block_struct -> pstar = 0;
  block_struct -> pprime = 0;
  block_struct -> s1 = 0;
  block_struct -> s2 = 0;
  block_struct -> sprime = 0;

  if (prog_struct -> args_struct != NULL) {
    prog_struct -> progname = prog_struct -> args_struct -> progname;
    prog_struct -> base_filename = prog_struct -> args_struct -> base_filename;
    prog_struct -> verbose_level = prog_struct -> args_struct -> verbose_level;
    prog_struct -> merge_level = prog_struct -> args_struct -> merge_level;
    prog_struct -> word_flags = prog_struct -> args_struct -> word_flags;
    prog_struct -> dowordlen = prog_struct -> args_struct -> dowordlen;
  }

  if (prog_struct -> base_filename != NULL) {
    prog_struct -> old_prel_name = wmalloc (strlen (prog_struct -> base_filename) + 10);
    strcpy (prog_struct -> old_prel_name, prog_struct -> base_filename);
    strcat (prog_struct -> old_prel_name, ".prel.old");

    prog_struct -> old_seq_name = wmalloc (strlen (prog_struct -> base_filename) + 9);
    strcpy (prog_struct -> old_seq_name, prog_struct -> base_filename);
    strcat (prog_struct -> old_seq_name, ".seq.old");

    prog_struct -> new_seq_name = wmalloc (strlen (prog_struct -> base_filename) + 5);
    strcpy (prog_struct -> new_seq_name, prog_struct -> base_filename);
    strcat (prog_struct -> new_seq_name, ".seq");

    prog_struct -> new_prel_name = wmalloc (strlen (prog_struct -> base_filename) + 6);
    strcpy (prog_struct -> new_prel_name, prog_struct -> base_filename);
    strcat (prog_struct -> new_prel_name, ".prel");

    /*  Rename input files to .old versions  */
    /*  Apply command to sequence and prel files:  mv new-name old-name  */
    rename_cmd = wmalloc ((strlen (prog_struct -> new_seq_name) + strlen (prog_struct -> old_seq_name) + 5) * (sizeof (R_CHAR)));
    (void) snprintf (rename_cmd, (strlen (prog_struct -> new_seq_name) + strlen (prog_struct -> old_seq_name) + 5) * (sizeof (R_CHAR)), "mv %s %s", prog_struct -> new_seq_name, prog_struct -> old_seq_name);
    system_result = (R_UINT) system (rename_cmd);
    wfree (rename_cmd);

    rename_cmd = wmalloc ((strlen (prog_struct -> new_prel_name) + strlen (prog_struct -> old_prel_name) + 5) * (sizeof (R_CHAR)));
    (void) snprintf (rename_cmd, (strlen (prog_struct -> new_prel_name) + strlen (prog_struct -> old_prel_name) + 5) * (sizeof (R_CHAR)), "mv %s %s", prog_struct -> new_prel_name, prog_struct -> old_prel_name);
    system_result = (R_UINT) system (rename_cmd);
    wfree (rename_cmd);

    /*
    **  Open prelude, sequence, and output files and test to ensure
    **  open worked
    */
    prog_struct -> in_prel = fopen (prog_struct -> old_prel_name, "r");
    if (! prog_struct -> in_prel) {
      perror (prog_struct -> old_prel_name);
      exit (EXIT_FAILURE);
    }

    prog_struct -> in_seq = fopen (prog_struct -> old_seq_name, "r");
    if (! prog_struct -> in_seq) {
      perror (prog_struct -> old_seq_name);
      exit (EXIT_FAILURE);
    }

    prog_struct -> out_prel = fopen (prog_struct -> new_prel_name, "w");
    if (! prog_struct -> out_prel) {
      perror (prog_struct -> new_prel_name);
      exit (EXIT_FAILURE);
    }

    prog_struct -> out_seq = fopen (prog_struct -> new_seq_name, "w");
    if (! prog_struct -> out_seq) {
      perror (prog_struct -> new_seq_name);
      exit (EXIT_FAILURE);
    }
  }
  else {
  }

  prog_struct -> bit_in_rec = newBitin (prog_struct -> in_prel);

  prog_struct -> in_buf = wmalloc (SEQ_BUF_SIZE * sizeof (R_UINT));
  prog_struct -> in_buf_end = prog_struct -> in_buf + SEQ_BUF_SIZE;
  prog_struct -> in_buf_p = prog_struct -> in_buf_end;

  prog_struct -> out_buf = wmalloc (SEQ_BUF_SIZE * sizeof (R_UINT));
  prog_struct -> out_buf_end = prog_struct -> out_buf + SEQ_BUF_SIZE;
  prog_struct -> out_buf_p = prog_struct -> out_buf;

  prog_struct -> pflag_buf = wmalloc (SEQ_BUF_SIZE * sizeof (R_UINT));
  prog_struct -> pflag_buf_end = prog_struct -> pflag_buf + SEQ_BUF_SIZE;
  prog_struct -> pflag_buf_p = prog_struct -> pflag_buf;

  if (prog_struct -> verbose_level == R_TRUE) {
    fprintf (stdout, "%10s%8s%8s%8s%8s%10s%10s%10s\n", "Block", "P1", "P2", "P*", "P'", "S1", "S2", "S'");
  }

  return;
}


void uninitRemerge (PROG_INFO *prog_struct, BLOCK_INFO *block_struct) {
  R_CHAR *delete_cmd;
  R_UINT system_result;

  if (prog_struct -> verbose_level == R_TRUE) {
    fprintf (stdout, "------------------------------------------------------------------------\n");
    fprintf (stdout, "%10u%8u%8u%8u%8u%10u%10u%10u\n", prog_struct -> block_count - 1, prog_struct -> p1_acc, prog_struct -> p2_acc, prog_struct -> pstar_acc, prog_struct -> pprime_acc, prog_struct -> s1_acc, prog_struct -> s2_acc, prog_struct -> sprime_acc);
  }

#ifdef DEBUG
  fprintf (stderr, "Overall Statistics:\n\n");
  fprintf (stderr, "Base filename:  %s\n", prog_struct -> base_filename);
  fprintf (stderr, "Filenames:  [%s, %s]\n", prog_struct -> new_prel_name, prog_struct -> new_seq_name);
  fprintf (stderr, "Phrases:\n");
  fprintf (stderr, "\tInitial:  %u\n", prog_struct -> p1_acc + prog_struct -> p2_acc);
  fprintf (stderr, "\tFinal:  %u\n", prog_struct -> pprime_acc);
  fprintf (stderr, "Sequence:\n");
  fprintf (stderr, "\tInitial:  %u\n", prog_struct -> s1_acc + prog_struct -> s2_acc);
  fprintf (stderr, "\tFinal:  %u\n", prog_struct -> sprime_acc);
#endif

  if (prog_struct -> out_buf_p != prog_struct -> out_buf) {
    (void) fwrite (prog_struct -> out_buf, sizeof (*prog_struct -> out_buf_p), (size_t) (prog_struct -> out_buf_p - prog_struct -> out_buf), prog_struct -> out_seq);
  }

  wfree (prog_struct -> path_buffer);
  prog_struct -> path_buffer = NULL;

  wfree (prog_struct -> in_buf);
  prog_struct -> in_buf = NULL;
  prog_struct -> in_buf_end = NULL;
  prog_struct -> in_buf_p = NULL;

  wfree (prog_struct -> out_buf);
  prog_struct -> out_buf = NULL;
  prog_struct -> out_buf_end = NULL;
  prog_struct -> out_buf_p = NULL;

  wfree (prog_struct -> pflag_buf);
  prog_struct -> pflag_buf = NULL;
  prog_struct -> pflag_buf_end = NULL;
  prog_struct -> pflag_buf_p = NULL;

  wfree ((prog_struct -> bit_in_rec) -> buffer);
  wfree (prog_struct -> bit_in_rec);

  FCLOSE (prog_struct -> in_prel);
  FCLOSE (prog_struct -> in_seq);
  FCLOSE (prog_struct -> out_prel);
  FCLOSE (prog_struct -> out_seq);

  delete_cmd = wmalloc ((strlen (prog_struct -> old_prel_name) + strlen (prog_struct -> old_seq_name) + 8) * (sizeof (R_CHAR)));
  (void) snprintf (delete_cmd, (strlen (prog_struct -> old_prel_name) + strlen (prog_struct -> old_seq_name) + 8) * (sizeof (R_CHAR)), "rm -f %s %s", prog_struct -> old_prel_name, prog_struct -> old_seq_name);
  system_result = (R_UINT) system (delete_cmd);
  wfree (delete_cmd);

  wfree (prog_struct -> old_prel_name);
  wfree (prog_struct -> old_seq_name);
  wfree (prog_struct -> new_seq_name);
  wfree (prog_struct -> new_prel_name);

  return;
}

void executeRemerge_File (PROG_INFO *prog_struct, BLOCK_INFO *block_struct) {
  R_CHAR *wordlen_fname = NULL;
  FILE *wordlen_fp = NULL;
  R_UINT maxwordlen = 0;
  R_UINT i = 0;
  NEWPAIR1 **gen_unit_sort1 = NULL;

  while (R_TRUE) {

    initRemerge_OneBlock (prog_struct, block_struct);

    decodeHierarchy_OneBlock (prog_struct, block_struct, 1);

    if (block_struct -> p1 == 0) {
      break;
    }

#ifdef DEBUG
      fprintf (stderr, "1 - Decoded %u primitives and phrases.\n", block_struct -> p1);
#endif

    decodeHierarchy_OneBlock (prog_struct, block_struct, 2);

#ifdef DEBUG
      fprintf (stderr, "2 - Decoded %u primitives and phrases.\n", block_struct -> p2);
#endif

    block_struct -> max_target_prel_size = (block_struct -> p1 + block_struct -> p2);
    block_struct -> target_prel = wmalloc (sizeof (NEWBLOCK));

    (block_struct -> target_prel) -> phrases1 = NULL;
    (block_struct -> target_prel) -> forward_sort1 = NULL;
    (block_struct -> target_prel) -> gen_unit_sort1 = NULL;
    (block_struct -> target_prel) -> duplicates_sort1 = NULL;

    (block_struct -> target_prel) -> phrases2 = NULL;

    (block_struct -> target_prel) -> phrases3 = NULL;
    (block_struct -> target_prel) -> gen_unit_sort3 = NULL;

    (block_struct -> target_prel) -> prims_lookup = NULL;
    (block_struct -> target_prel) -> prims_lookup_size = 0;
    (block_struct -> target_prel) -> num_phrases = block_struct -> max_target_prel_size;
    (block_struct -> target_prel) -> sizelist = NULL;
    (block_struct -> target_prel) -> end = NULL;
    (block_struct -> target_prel) -> phrases1 = wmalloc (block_struct -> max_target_prel_size * sizeof (NEWPAIR1));

    mergePrimitives (prog_struct, block_struct);
    mergePhrases (prog_struct, block_struct);

    sortNewPhrases (prog_struct, block_struct);

    block_struct -> pprime = (block_struct -> target_prel) -> num_primitives + (block_struct -> target_prel) -> num_phrases;

    if (prog_struct -> merge_level != THREE) {
      encodeHierarchy_OneBlock (prog_struct, block_struct);
    }

    /*  Print word lengths to file  */
    if (prog_struct -> dowordlen == R_TRUE) {
      /*  After merging of phrase hierarchies, need to re-assign
      **  lengths to gen_unit_sort array. */
      gen_unit_sort1 = block_struct -> target_prel -> gen_unit_sort1;

      for (i = 0; i < block_struct -> target_prel -> num_primitives; i++) {
        gen_unit_sort1[i] -> len = 1;
      }
      for (; i < block_struct -> target_prel -> num_primitives + block_struct -> target_prel -> num_phrases; i++) {
        gen_unit_sort1[i] -> len = gen_unit_sort1[gen_unit_sort1[i] -> left_child] -> len + gen_unit_sort1[gen_unit_sort1[i] -> right_child] -> len;
      }


      wordlen_fname = wmalloc (sizeof (R_CHAR) * (strlen (prog_struct -> base_filename) + 1 + 3));
      strcpy (wordlen_fname, prog_struct -> base_filename);
      strcat (wordlen_fname, ".wl");
      FOPEN (wordlen_fname, wordlen_fp, "w");
      maxwordlen = block_struct -> target_prel -> num_primitives + block_struct -> target_prel -> num_phrases;
      (void) fwrite (&maxwordlen, sizeof (R_UINT), 1, wordlen_fp);
      for (i = 0; i < maxwordlen; i++) {
        (void) fwrite (&block_struct -> target_prel -> gen_unit_sort1[i] -> len, sizeof (R_UINT), 1, wordlen_fp);
      }
      FCLOSE (wordlen_fp);
      wfree (wordlen_fname);
    }

    switch (prog_struct -> merge_level) {

    case ONE:
      reEncodeSequence_Phase1 (prog_struct, block_struct, 1);
      reEncodeSequence_Phase1 (prog_struct, block_struct, 2);

      break;

    case TWOA:
      hashPhrases (prog_struct, block_struct);

      reEncodeSequence_Phase2a (prog_struct, block_struct, 1);

      wfree ((block_struct -> target_prel) -> target_prel_hashtable);
      (block_struct -> target_prel) -> target_prel_hashtable = NULL;
      break;

    case TWOB:
      transferNewPair1_2 (prog_struct, block_struct);
      linkPhraseHierarchy (prog_struct, block_struct);
      reEncodeSequence_Phase2b (prog_struct, block_struct, 1);
      unexpandAllPhrases1 (prog_struct, block_struct);
      break;

    case THREE:
      transferNewPair1_3 (prog_struct, block_struct);
      reEncodeSequence_Phase3 (prog_struct, block_struct, 1);
      encodeHierarchy_OneBlock (prog_struct, block_struct);
      break;

    case APPEND:
      transferNewPair1_2 (prog_struct, block_struct);
      linkPhraseHierarchy (prog_struct, block_struct);
      reEncodeSequence_Append (prog_struct, block_struct, 1);
      unexpandAllPhrases1 (prog_struct, block_struct);
      break;

    default:
      fprintf (stderr, "Error:  Unexpected remerge level.\n");
      exit (EXIT_FAILURE);
    }

    displayStats_OneBlock (prog_struct, block_struct);

    if (block_struct -> p2 == 0) {
      break;
    }
  }

  uninitRemerge_OneBlock (prog_struct, block_struct);

  flushBuffer (prog_struct);

  return;
}


