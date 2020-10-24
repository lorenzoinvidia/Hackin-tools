#!/bin/bash
# **********************************************************************
#  PoC: hex-to-carray.sh	
#  Author: lorenzoinvidia
#
#  Description:
#  Convert a hexdump to c array
#
# ********************************************************************** 


if [ "$#" -ne 1 ]; then 
    echo "Usage: hex-to-carray.sh <hexfile>"
    exit 1
fi

# 1 cat arg0
# 2 add space at beginning
# 3 translate spaces to \x
# 4 add space after 64 chars
# 5 format as a line of 16 bytes each
# 6 add quotes
cat $1 | sed 's/^/ /g' |sed 's/ /\\x/g' | sed 's/.\{64\}/& /g' | tr " " "\n" | sed 's/^/"/g' | sed 's/$/"/g'