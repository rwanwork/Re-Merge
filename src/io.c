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
#include <string.h>

#include "common-def.h"
#include "remerge-defn.h"
#include "io.h"


R_UINT readBuffer (PROG_INFO *prog_struct) {
  R_UINT i = 0;
  R_UINT bytes_read = 0;

  bytes_read = (R_UINT) fread (prog_struct -> in_buf, sizeof (*(prog_struct -> in_buf)), SEQ_BUF_SIZE, prog_struct -> in_seq);
  prog_struct -> in_buf_end = prog_struct -> in_buf + bytes_read;
  if (ferror (prog_struct -> in_seq) != R_FALSE) {
    perror ("ERROR:  Reading input sequence file");
    exit (EXIT_FAILURE);
  }
  prog_struct -> in_buf_p = prog_struct -> in_buf;

  /*  if punctuation flag specified  */
  if (prog_struct -> word_flags == UW_YES) {
    for (i = 0; i < bytes_read; i++) {
      prog_struct -> pflag_buf[i] = prog_struct -> in_buf[i] & PUNC_FLAG;
      prog_struct -> in_buf[i] = prog_struct -> in_buf[i] & NO_FLAGS;
    }
  }
  else {
    for (i = 0; i < bytes_read; i++) {
      prog_struct -> pflag_buf[i] = 0;
    }
  }

  prog_struct -> pflag_buf_end = prog_struct -> pflag_buf + bytes_read;
  prog_struct -> pflag_buf_p = prog_struct -> pflag_buf;

  return (bytes_read);
}

/*  amt is the fraction of the output buffer to write out; valid
**  values from 0 to 1  */
R_UINT writeBuffer (PROG_INFO *prog_struct, R_FLOAT amt) {
  R_UINT bytes_read = 0;
  R_UINT *temp = NULL;
  R_UINT remain = 0;

  bytes_read = (R_UINT) fwrite (prog_struct -> out_buf, sizeof (*(prog_struct -> out_buf)), (size_t) (SEQ_BUF_SIZE * amt), prog_struct -> out_seq);
  temp = prog_struct -> out_buf + ((R_UINT) (SEQ_BUF_SIZE * amt));
  remain = (R_UINT) (prog_struct -> out_buf_end - temp);
  memmove (prog_struct -> out_buf, temp, (size_t) (remain * 4));
  prog_struct -> out_buf_p = (prog_struct -> out_buf) + remain;

  if (ferror (prog_struct -> out_seq) != R_FALSE) {
    perror ("ERROR:  Reading output sequence file");
    exit (EXIT_FAILURE);
  }

  return (bytes_read);
}
