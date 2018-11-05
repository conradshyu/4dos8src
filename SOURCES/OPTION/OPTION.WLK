# Watcom linker control file for OPTION.EXE for DOS
# Copyright 1998, JP Software Inc., All Rights Reserved

name      %linktarg%
option    map=%linkmap%

option    stack=4096
option    maxerrors=25
option    symfile
option    eliminate
option    quiet

format    dos

# Note library paths are processed in reverse order!

libpath   %tui%
libpath   %watcom%\lib386
libfile   windowm.lib

file      %obj%\option, %obj%\dialog, %obj%\argparse, %obj%\iniio
file      %obj%\iniutil, %obj%\scrdef, %obj%\iparse, %obj%\istring
file      %obj%\kparse
