Re-Merge
========

Introduction
------------

[Re-Pair](https://github.com/rwanwork/Re-Pair) is the name of the algorithms and the software which implements the recursive pairing algorithm.  Its corresponding decompressor is Des-Pair.

In this repository, we have Re-Merge.  Re-Merge is the name of the algorithm and the software which implements the block merging algorithm for Re-Pair blocks.

Re-Merge is divided into several phases as follows:
  * 1  - Phrase hierarchy merging.
  * 2a - On-line pairing of sequence symbols.
  * 2b - Optimal re-parsing of sequence symbols using the merged hierarchy.
  * 3  - Locate new phrases to add to the phrase hierarchy.
  * a  - Append new ASCII text to the end of a compressed block

In phase 1, the phrase hierarchies are merged in a pair-wise fashion with each iteration.  Then, either phase 2a or 2b is used to rebuild the compressed sequence either locally or globally (2a or 2b, respectively) using the merged sequence.  Phase 3 locates new phrases and adds on to the phrase hierarchy while phase A appends new text to the end of a compressed block.


About The Source Code
---------------------

The source code is written in C and was originally compiled using v4.1.2 of gcc for Debian v4.0 (etch).  It has been compiled and successfully run on both an Intel Pentium 4 CPU (32-bit) and an Intel Core 2 Duo CPU (64-bit).  However, it makes no use of the advantages offered by 64-bit architecture.

In 2022, it could be compiled on an Ubuntu 22.04 (64-bit) system using gcc version 11.2.0, with neglible compiler warnings .

This main purpose of this software was for research.  Therefore, additional checks and extraneous information has been added into the source code which, if removed, may have a small improvement on the execution time.


Compiling
---------

The archive includes a `CMakeLists.txt`.  To create an out-of-source build, create a `build/` directory and run `cmake` version 3.5 or higher.  A `Makefile` will be created.  Then type `make`.  For example, if you're in the source directory,

```
    mkdir build
    cd build
    cmake ..
    make
```

The file to be processed is indicated using the -i option, without any suffixes.  Thus, if the option "-i filename" is given, then "filename.prel" and "filename.seq" are assumed to exist in the current directory.  After block merging, these files will be overwritten.

All phases can only be applied to a compressed file which is a single block, except for phase 1.  So, phase 1 is repeatedly called first until a single block is created.  Then, the other phases can be used.  If any of the other phases are used for a multi-block file, then the program will crash (unfortunately, at this moment, a more elegant error message is not given to the user).

Punctuation flags (-f option) assumes the input sequence is made up of integers (instead of characters) and that the "highest bit" is turned on to indicate a symbol that is followed by a punctuation mark (and so, must become a cut vertex).

Run the `remerge` executable without any arguments to see the list of options.


Citing
------

The algorithm was described in:
```
    R. Wan and A. Moffat. "Block merging for off-line compression". Journal of
    the American Society for Information Science and Technology, 58(1):3-14,
    2007.
```

Older citations include:
```
    R. Wan and A. Moffat. "Block merging for off-line compression". In A.
    Apostolico and M. Takeda, editors, Proc. 13th Annual Symposium on
    Combinatorial Pattern Matching, pages 32-41. Springer-Verlag Berlin,
    July 2002.
```
```
    R. Wan. "Browsing and Searching Compressed Documents". PhD thesis,
    University of Melbourne, Australia, December 2003.
```
    
The last of these 3 describe the application of Re-Merge to a more generalized sequence of symbols (unsigned integers) which could represent a Re-Pair sequence that was pre-processed by Pre-Pair.  The first two citations restricts the discussion to a blocks of compressed (ASCII) characters.


Contact
-------

This software was implemented by me (Raymond Wan) for my PhD thesis at the University of Melbourne (under the supervision of [Prof. Alistair Moffat](http://people.eng.unimelb.edu.au/ammoffat/)).  My contact details:

     E-mail:  rwan.work@gmail.com 

My homepage is [here](http://www.rwanwork.info/).

The latest version of Re-Merge can be downloaded from [GitHub](https://github.com/rwanwork/Re-Merge).

If you have any information about bugs, suggestions for the documentation or just have some general comments, feel free to contact me via e-mail or GitHub.


Version
-------

Changes to this software were recorded in the file ChangeLog up until April 2007.  Since moving the source code to GitHub on June 18, 2022, any changes are recorded in the repository's history.


Copyright and License
---------------------

Copyright (C) 2003-2022 by Raymond Wan (rwan.work@gmail.com)

Re-Merge is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 3 of the License, or (at your option) any later version.  Please see the accompanying file, COPYING.v3 for further details.


About This Repository
---------------------

This GitHub repository was created from the original tarball on my homepage many years ago.


    Raymond Wan
    June 18, 2022

