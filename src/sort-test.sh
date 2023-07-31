#!/bin/sh

optimize=debug
testfunc() {
    #lldb \
        $exec
}

cd "$(dirname "$0")"
unitest_sh=./unitest.sh
. $unitest_sh

src="\
./sort-test.c
./stList.c
./stData.c
./stObj.c
"

arch_family=defaults
srcset="Plain C"

tests_run
