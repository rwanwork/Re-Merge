###########################################################################
##  Re-Merge
##  Block merging for off-line compression.
##
##  Copyright (C) 2003-2022 by Raymond Wan, All rights reserved.
##  Contact:  rwan.work@gmail.com
##
##  This file is part of Re-Merge.
##  
##  Re-Merge is free software; you can redistribute it and/or 
##  modify it under the terms of the GNU General Public License 
##  as published by the Free Software Foundation; either version 
##  3 of the License, or (at your option) any later version.
##  
##  Re-Merge is distributed in the hope that it will be useful,
##  but WITHOUT ANY WARRANTY; without even the implied warranty of
##  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##  GNU General Public License for more details.
##  
##  You should have received a copy of the GNU General Public 
##  License along with Re-Merge; if not, see 
##  <http://www.gnu.org/licenses/>.
###########################################################################

##  Set the minimum required CMake version
cmake_minimum_required (VERSION 3.5 FATAL_ERROR)


########################################
##  Variables specific to this project
set (TARGET_NAME_REMERGE "remerge")
set (CURR_PROJECT_NAME "Re-Merge")


##  Source files for Re-Merge
set (REMERGE_SRC_FILES
  utils.c
  wmalloc.c 
  main-remerge.c
  remerge.c
  hashing.c
  bitin.c
  bitout.c
  phrase-slide-decode.c
  phrase-slide-encode.c
  merge-phrases.c
  sort-phrases.c
  phase_1.c
  phase_2a.c
  phase_2b.c
  phase_2_all.c
  phase_3.c
  io.c
  phase_a.c
)


########################################
##  Set up the software

##  Project name
project (${CURR_PROJECT_NAME} VERSION 1.0 LANGUAGES C)
message (STATUS "Setting up ${CURR_PROJECT_NAME}, version ${PROJECT_VERSION}...")


########################################
##  Create the executables

if (NOT TARGET ${TARGET_NAME_REMERGE})
  add_executable (${TARGET_NAME_REMERGE} ${COMMON_SRC_FILES} ${REMERGE_SRC_FILES})
  target_link_libraries (${TARGET_NAME_REMERGE} m)
  install (TARGETS ${TARGET_NAME_REMERGE} DESTINATION bin)
endif (NOT TARGET ${TARGET_NAME_REMERGE})


########################################
##  Extra cflags.  Add to EXTRA_CFLAGS, but do not add more than one
##  due to the amount of output that would be produced.
##  * Print the phrase hierarchy as text in the order in which it is
##    encoded:  -DPHRASE_HIERARCHY
##  * Print malloc information:  -DCOUNT_MALLOC
##  * Print out more information than -v:  -DDEBUG

set (EXTRA_CFLAGS "")


##  Turn on lots of warnings; set optimization flag to -O3
set (MY_C_FLAGS "-Wno-long-long -pedantic -Wall -Wwrite-strings -Wcast-align -Wstrict-prototypes -Wmissing-prototypes -Wnested-externs -Wshadow -Winline -O3")

##  Create the final compiler flags for the C compiler
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${MY_C_FLAGS} ${EXTRA_CFLAGS}")


############################################################
##  CTest
############################################################

# ENABLE_TESTING ()

