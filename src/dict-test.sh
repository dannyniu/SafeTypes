#!/bin/sh

#optimize=debug
testfunc() {
    #lldb \
        $exec | sort -V | uniq -c | tee /dev/tty | wc -l
}

cd "$(dirname "$0")"
unitest_sh=./unitest.sh
. $unitest_sh

src="\
./dict-test.c
./stDict.c
./stData.c
./stObj.c
./siphash.c
"

arch_family=defaults
srcset="Plain C"

tests_run
