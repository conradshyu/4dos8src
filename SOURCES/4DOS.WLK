# WLINK control file for 4DOS

system    dos
option    map
option    verbose
option    quiet
option    stack=8k
option    nodefaultlibs
option    nocaseexact
#disable   2030 # multiple starting addresses
library   sasjp.lib
#library  startupm.lib
library   4dostart.lib
library   clibm.lib
