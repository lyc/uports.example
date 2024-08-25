#!/bin/bash -p

progname=`basename $0`
#progname=${0##*/}        # fast basename hack for ksh, bash

USAGE=\
"usage: $progname [ -m prog ] [ -rdC ] [-s[1|2]] [ -p plugin ] prog [ args ... ]
        -m   -- program which want to execute
        -r   -- execute prog release version
        -d   -- execute prog with valgrind
        -s   -- sysrepo version [1|2] (default: 2)
        -C   -- plugin bulid by CMake system
        -p   -- run sysrepo plugin
	-v   -- verbose"

fatal() {
    if [ "$1" ]
    then
        echo $* >&2
    fi
    echo "$USAGE" 1>&2
    exit 1
}

#
# process the options
#

progs=""
debug="d"
verbose=""
valgrind=""
builder="nonrecursive.mk"
plugin=""
sysrepo="2"

while getopts m:rds:Cp:v OPT
do
    case "$OPT" in
        m)
            eval p=$OPTARG; progs="$progs $p";;
        r)
            debug="";;
	d)
	    valgrind="valgrind --leak-check=full --show-leak-kinds=all";;
        s)
            eval sysrepo=$OPTARG;;
        C)
            eval builder="CMake";;
	p)
            plugin="yes"
            eval p=$OPTARG; progs="$progs $p";;
	v)
	    verbose="yes";;
        *)
            fatal
    esac
done

shiftcount=`expr $OPTIND - 1`
shift $shiftcount

if [ -z $progs ]; then
    if [ -z $1 ]; then
        fatal
    fi
    progs=$1
    shift 1
fi

args=$*
pwd=`pwd`
DESTDIR=${pwd}/local
ld_path=${DESTDIR}/usr/lib
opsys=`uname -s | tr '[:upper:]' '[:lower:]' | sed -e 's/darwin/osx/'`

case $opsys in
    linux)
        ld_env="LD_LIBRARY_PATH"
        ;;
    osx)
        ld_env="DYLD_LIBRARY_PATH"
        ;;
esac

if [ "$builder" = "CMake" ]; then
    debug=""
fi

if [ "$builder" = "CMake" ]; then
    bins="src/plugins/.build"
    libs="src/plugins/.build"
else
if [ "$debug" = "d" ]; then
    bins="objd"
    libs="objd"
else
    bins="obj"
    libs="obj"
fi
fi

if [ "$sysrepo" = "1" ]; then
    export LIBYANG_EXTENSIONS_PLUGINS_DIR=${DESTDIR}/usr/lib/libyang/extensions
    export LIBYANG_USER_TYPES_PLUGINS_DIR=${DESTDIR}/usr/lib/libyang/user_types
fi

if [ "$verbose" = "yes" ]; then
    echo "--------------------"
    echo "args = $args"
    echo "DESTDIR = $DESTDIR"
    echo "opsys = $opsys"
    echo "ld_env = $ld_env"
    echo "ld_path = $ld_path"
    echo "bins = $bins"
    echo "libs = $libs"
    echo "--------------------"
    echo "debug = $debug"
    echo "builder = $builder"
    echo "sysrepo = $sysrepo"
    echo "plugin = $plugin"
    echo "valgrind = $valgrind"
    echo "--------------------"
    echo "progs = $progs"
    echo "--------------------"
    echo
fi

#
# OK, we have test program and args now
#

if [ "$plugin" = "yes" ]; then
    export SRPD_PLUGINS_PATH=${DESTDIR}/usr/lib/sysrepo/plugins

    ${DESTDIR}/usr/bin/sysrepoctl -l
    mkdir -p ${DESTDIR}/usr/lib/sysrepo/plugins
    rm -fr ${DESTDIR}/usr/lib/sysrepo/plugins/*

    for p in $progs; do
        if [ -f ${bins}/lib${p}${debug}.so ]; then
            echo copying ${bins}/lib${p}${debug}.so...
            cp -p ${bins}/lib${p}${debug}.so ${DESTDIR}/usr/lib/sysrepo/plugins
        fi
    done
    if [ "$verbose" = "yes" ]; then
        echo "run: [env $ld_env=local/usr/lib:${bins} $valgrind ${DESTDIR}/usr/bin/sysrepo-plugind -d -v4]"
    fi
    env ${ld_env}=${ld_path}:${libs} $valgrind ${DESTDIR}/usr/bin/sysrepo-plugind -d -v4
    exit 0
fi

for p in $progs; do
    if [ -e ${bins}/${p}${debug} ]; then
        if [ "$verbose" = "yes" ]; then
            echo "run: [env $ld_env=local/usr/lib:${bins} $valgrind ${bins}/${p}${debug} $args $config]"
        fi
        env ${ld_env}=${ld_path}:${libs} $valgrind ${bins}/${p}${debug} $args $config
    elif [ -e ${p}${debug} ]; then
        if [ "$verbose" = "yes" ]; then
            echo "run: [env $ld_env=local/usr/lib:${bins} $valgrind ${p}${debug} $args $config]"
        fi
        env ${ld_env}=${ld_path}:${libs} $valgrind ${p}${debug} $args $config
    else
        echo "Oops! ${bins}/${p}${debug} or ${p}${debug} not existed..."
    fi
done

exit 0

# Local Variables: ***
# mode:ksh ***
# End: ***
