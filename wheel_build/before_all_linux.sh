set -e
set -x
if command -v apk; then
    apk add curl swig bison flex flex-dev libdwarf-dev elfutils-dev \
        zlib-dev eigen-dev automake autoconf libtool \
        tcl-dev readline-dev tcl-readline-dev
fi
if command -v yum; then
    yum install -y swig flex zlib-devel readline-devel eigen3-devel \
        elfutils-devel elfutils-libelf-devel libdwarf-devel binutils-devel m4 \
        perl-core tcl-devel tcl-tclreadline-devel
fi

NPROC=$(getconf _NPROCESSORS_ONLN 2>/dev/null || sysctl -n hw.ncpu)
SUDO=""
if [ "$(id -u)" -ne 0 ]; then
SUDO="sudo"
fi
    
# Bison
BISON_VERSION=3.8.2
BISON_SRC_HASH=06c9e13bdf7eb24d4ceb6b59205a4f67c2c7e7213119644430fe82fbd14a0abb
if ! printf '%s\n' '%require "3.8"' '%%' 'start: ;' | bison -o /dev/null /dev/stdin ; then
    WORKDIR=$(mktemp -d)
    (
        cd $WORKDIR
        curl -L --retry 5 --retry-delay 3 \
            https://ftp.gnu.org/gnu/bison/bison-${BISON_VERSION}.tar.gz > bison.tgz
        echo "$BISON_SRC_HASH bison.tgz" | sha256sum -c
        tar --strip-components=1 -xzC . -f bison.tgz
        ./configure
        make clean
        $SUDO make install -j$NPROC
    )
rm -rf "$WORKDIR"
fi
