Re-Merge
========


Table of Contents
-----------------
  I.    Introduction
  II.   About The Source Code
  III.  Compiling
  IV.   Citing
  VI.   Version
  VII.  Copyright and License



I.  Introduction
----------------

Re-Merge is the name of the algorithm and the software which implements the
block merging algorithm for Re-Pair blocks.

Re-Merge is divided into several phases as follows:
  1  - Phrase hierarchy merging.
  2a - On-line pairing of sequence symbols.
  2b - Optimal re-parsing of sequence symbols using the merged hierarchy.
  3  - Locate new phrases to add to the phrase hierarchy.
  a  - Append new ASCII text to the end of a compressed block

In phase 1, the phrase hierarchies are merged in a pair-wise fashion
with each iteration.  Then, either phase 2a or 2b is used to rebuild the
compressed sequence either locally or globally (2a or 2b, respectively)
using the merged sequence.  Phase 3 locates new phrases and adds on to
the phrase hierarchy while phase A appends new text to the end of a
compressed block.


II.  About The Source Code
--------------------------

The source code is written in C and compiled using v4.1.2 of gcc for 
Debian v4.0 (etch).  It has been compiled and successfully run on both an 
Intel Pentium 4 CPU (32-bit) and an Intel Core 2 Duo CPU (64-bit).  
However, it takes no advantages of the 64-bit architecture.

This main purpose of this software was for research.  Therefore, 
additional checks and extraneous information has been added into the 
source code which, if removed, may have a small impact on the execution 
time.


III.  Compiling
---------------

The archive includes a Makefile.  Simply type "make" to build the source 
code.  There is no "configure" script for determining the system set-up, 
though this may happen with a future release.

The file to be compressed is indicated using the -i option, without any
suffixes.  Thus, if the option "-i filename" is given, then
"filename.prel" and "filename.seq" are assumed to exist in the current
directory.  After block merging, these files will be overwritten.

All phases can only be applied to a compressed file which is a single
block, except for phase 1.  So, phase 1 is repeatedly called first until
a single block is created.  Then, the other phases can be used.  If any
of the other phases are used for a multi-block file, then the program
will crash.

Punctuation flags (-f option) assumes the input sequence is made up of
integers (instead of characters) and that the "highest bit" is turned on
to indicate a symbol that is followed by a punctuation mark (and so,
must become a cut vertex).

Run either executable without any arguments to see the list of options.


IV.  Citing
-----------

The algorithm was recently described in:
  - R. Wan and A. Moffat. "Block merging for off-line compression". Journal of
    the American Society for Information Science and Technology, 58(1):3-14,
    2007.

Older citations include:
  - R. Wan and A. Moffat. "Block merging for off-line compression". In A.
    Apostolico and M. Takeda, editors, Proc. 13th Annual Symposium on
    Combinatorial Pattern Matching, pages 32-41. Springer-Verlag Berlin,
    July 2002.
  - R. Wan. "Browsing and Searching Compressed Documents". PhD thesis,
    University of Melbourne, Australia, December 2003.

The last of these 3 describe the application of Re-Merge to a more
generalized sequence of symbols (unsigned integers) which could represent
a Re-Pair sequence that was pre-processed by Pre-Pair.  The other two
restricts the discussion to a blocks of compressed (ASCII) characters.


V.  Contact
-----------

This software was implemented by Raymond Wan:
  E-mail:  rwan@kuicr.kyoto-u.ac.jp
  Homepage:  http://www.bic.kyoto-u.ac.jp/pathway/rwan/

Updates to this software can be found at:
  http://www.bic.kyoto-u.ac.jp/pathway/rwan/software.html

While this software is no longer being actively maintained, I would 
appreciate hearing about any bugs, problems, and comments that you have 
and will try to update it as necessary.


VI.  Version
------------

Changes to this software are recorded in the file CHANGES.


VII.  Copyright and License
---------------------------

Copyright (C) 2003, 2007 by Raymond Wan (rwan@kuicr.kyoto-u.ac.jp)

Re-Merge is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by the 
Free Software Foundation; either version 2 of the License, or (at your 
option) any later version.  Please see the accompanying file, COPYING for 
further details.


