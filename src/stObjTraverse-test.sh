#!/bin/sh

# optimize=debug
testfunc() {
    # lldb \
        $exec
}

cd "$(dirname "$0")"
unitest_sh=./unitest.sh
. $unitest_sh

src="\
./stObjTraverse-test.c
"

arch_family=defaults
srcset="Plain C"

tests_run
