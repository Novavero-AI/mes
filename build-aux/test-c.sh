#! /usr/bin/env bash

# GNU Mes --- Maxwell Equations of Software
# Copyright © 2018,2019,2025 Janneke Nieuwenhuizen <janneke@gnu.org>
# Copyright © 2025 Stefan <stefan-guix@vodafonemail.de>
#
# This file is part of GNU Mes.
#
# GNU Mes is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or (at
# your option) any later version.
#
# GNU Mes is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with GNU Mes.  If not, see <http://www.gnu.org/licenses/>.

set -e

if test -z "$config_sh"; then
    . ./config.sh
fi

if [ "$V" = 2 ]; then
    set -x
fi

t=${1-lib/tests/scaffold/t.c}
b=$(dirname "$t")/$(basename "$t" .c)
o="$b"
o=lib/tests/${b#*lib/tests/}
if [ "$o" = "$b" ]; then
    o=./$(basename "$t" .c)
fi

rm -f "$o"
CC=${CC-gcc}
if test -z "$CPPFLAGS"; then
    CPPFLAGS="
-D HAVE_CONFIG_H=1
-I ../include
-I ${srcdir}/include
-I ${srcdir}/include/$mes_kernel/$mes_cpu
"
    LDFLAGS=-nostdlib
fi

i=$(basename "$t" .c)

if [ -z "${MES_CHECKING_BUILTIN_LIBS}" ]
then
    MES_CHECKING_BUILTIN_LIBS="`${CC} --print-libgcc-file-name`"
fi

# We use a POSIX compatible trick to get the first character from a
# variable: Drop from it the suffix, which is the varible content with
# its first character removed.  Finally compare this first character to
# the group-of-ten.

if (echo $i | grep -Eq '^[012]'); then
    LIBS="${MES_CHECKING_BUILTIN_LIBS} -l c-mini"
elif (echo $i | grep -Eq '^[34]'); then
    LIBS="-l c-mini ${MES_CHECKING_BUILTIN_LIBS} -l c-mini"
elif (echo $i | grep -Eq '^[78]'); then
    LIBS="-l c+tcc ${MES_CHECKING_BUILTIN_LIBS} -l c+tcc"
elif (echo $i | grep -Eq '^[9a]'); then
    LIBS="-l c+gnu ${MES_CHECKING_BUILTIN_LIBS} -l c+gnu"
else
    # Make it possible to resolve raise(), required by libgcc.a, provided
    # in libc.a.  The final command line has to have "-lc -lgcc -lc".
    # See <https://www.openwall.com/lists/musl/2018/05/09/1>.
    LIBS="-l c ${MES_CHECKING_BUILTIN_LIBS} -l c"
fi

if test $mes_kernel = gnu\
        && test -z "$LIBS"; then
    LIBS="-l c-mini -l mescc"
fi

if test $mes_libc = system; then
    crt1=
    LIBS='-l mes -l mescc'
else
    crt1=crt1.o
fi

$CC -g -c $AM_CPPFLAGS $CPPFLAGS $AM_CFLAGS $CFLAGS -o "$o".o "$t"
$CC -g $AM_CFLAGS $CFLAGS $AM_LDFLAGS $LDFLAGS -L . -o "$o" $crt1 "$o".o $LIBS

set +e
"$o" -s --long file0 file1 > "$o".1 2> "$o".2
r=$?
set -e
if [ -f "$b".exit ]; then
    e=$(cat "$b".exit)
else
    e=0
fi
if [ $r != $e ]; then
    if [ $r != 0 ]; then
        exit $r;
    fi
    exit 1
fi
if [ -f "$b".stdout ]; then
    $DIFF -u "$b".stdout "$o".1
fi
if [ -f "$b".stderr ]; then
    $DIFF -u "$b".stderr "$o".2
fi
