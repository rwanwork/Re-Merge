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

#ifndef REPAIR_MERGE_H
#define REPAIR_MERGE_H

#define MAX_GEN 256

#define INIT_PATH_BUFFER_SIZE 1024

ARGS_INFO *parseArguments (R_INT argc, R_CHAR *argv[], ARGS_INFO *args_struct);
void initRemerge (PROG_INFO *prog_struct, BLOCK_INFO *block_struct);
void uninitRemerge (PROG_INFO *prog_struct, BLOCK_INFO *block_struct);
void executeRemerge_File (PROG_INFO *prog_struct, BLOCK_INFO *block_struct);
void transferNewPair3_1 (PROG_INFO *prog_struct, BLOCK_INFO *block_struct, unsigned int phrases3_size);

#endif
