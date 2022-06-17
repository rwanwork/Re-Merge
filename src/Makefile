##    Re-Merge
##    Block merger for recursive pairing.
##    Copyright (C) 2003, 2007 by Raymond Wan (rwan@kuicr.kyoto-u.ac.jp)
##
##    Version 1.0 -- 2007/04/02
##
##    This file is part of the Re-Merge software.
##
##    Re-Merge is free software; you can redistribute it and/or modify
##    it under the terms of the GNU General Public License as published by
##    the Free Software Foundation; either version 2 of the License, or
##    (at your option) any later version.
##
##    Re-Merge is distributed in the hope that it will be useful,
##    but WITHOUT ANY WARRANTY; without even the implied warranty of
##    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##    GNU General Public License for more details.
##
##    You should have received a copy of the GNU General Public License along
##    with Re-Merge; if not, write to the Free Software Foundation, Inc.,
##    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
##

CC	= gcc
LIBS    = -lm
CFLAGS  = -Wno-long-long -pedantic -Wall -Wwrite-strings -Wcast-align \
          -Wstrict-prototypes -Wmissing-prototypes -Wnested-externs   \
          -Wshadow -Winline -O3 $(EXTRA_CFLAGS)

COMMON_DIR      = ../common

#######################################################################
##  Extra cflags.  Add to EXTRA_CFLAGS, but do not add more than one due
##  to the amount of output that would be produced.
EXTRA_CFLAGS =
##  Print the phrase hierarchy as text in the order in which it is
##    encoded:  -DPHRASE_HIERARCHY
##  Print malloc information:  -DCOUNT_MALLOC
##  Print out more information than -v:  -DDEBUG

PROGRAMS	= remerge
SRC		= main-remerge.c remerge.c hashing.c bitin.c wmalloc.c \
                  bitout.c phrase-slide-decode.c phrase-slide-encode.c \
		  merge-phrases.c sort-phrases.c phase_1.c phase_2a.c  \
                  phase_2b.c phase_2_all.c phase_3.c io.c phase_a.c    \
                  utils.c
SRC-OBJ		=	${SRC:.c=.o}


#######################################################################
# Rules
#######################################################################

all:  $(PROGRAMS)

link:
	rm -f utils.h utils.c wmalloc.h wmalloc.c common-def.h
	ln -s $(COMMON_DIR)/common-def.h common-def.h
	ln -s $(COMMON_DIR)/utils.h utils.h
	ln -s $(COMMON_DIR)/utils.c utils.c
	ln -s $(COMMON_DIR)/wmalloc.h wmalloc.h
	ln -s $(COMMON_DIR)/wmalloc.c wmalloc.c

remerge: $(SRC-OBJ)
	$(CC) $(CFLAGS) -o remerge $(SRC-OBJ) $(LIBS)

#######################################################################
# Cleaning rules
#######################################################################

clean:
	rm -f *~ *.o

clobber:  clean
	rm -f $(PROGRAMS)

