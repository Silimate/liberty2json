# Building and Installation

First of all, clone the repo recursively…

```bash
cd ~
git clone --recursive https://github.com/silimate/liberty2json-sta
```

## Ubuntu 22.04+

### Dependencies

```bash
sudo apt-get update
sudo apt-get install -y \
	wget \
	cmake \
	gcc \
	tcl-dev \
	tcl-tclreadline \
	libeigen3-dev \
	swig \
	bison \
	flex \
	libfl-dev \
	libdwarf-dev \
	libdw-dev \
	libelf-dev \
	elfutils \
	automake \
	autotools-dev

mkdir ~/cudd
curl -L https://raw.githubusercontent.com/davidkebo/cudd/main/cudd_versions/cudd-3.0.0.tar.gz | tar -xzvC ~/cudd --strip-components=1
cd ~/cudd
./configure
make -j$(nproc)
sudo make install
```

### Build

```bash
make run-cmake-release
make release
```

### Test

```bash
make test
```

## macOS

### Dependencies

```bash
cd ~/liberty2json
cd third_party/opensta
brew bundle install
```

### Build

```bash
cd ~/liberty2json
export PATH="$(brew --prefix bison)/bin:$(brew --prefix flex)/bin:$PATH"
make run-cmake-release ADDITIONAL_CMAKE_OPTIONS="-DTCL_LIBRARY=$(brew --prefix tcl-tk@8)/lib/libtcl8.6.dylib -DTCL_INCLUDE_PATH=$(brew --prefix tcl-tk@8)/include/tcl-tk -DFLEX_INCLUDE_DIR=$(brew --prefix flex)/include"
make release
```


### Test

```bash
make test
```


## Nix (Development Only)

### Dependencies

```bash
nix develop github:librelane/librelane/dev#opensta
```

### Build

```bash
make release ADDITIONAL_CMAKE_OPTIONS="$cmakeFlags"
```

### Test

Python is required for testing and unfortunately not included in the Nix
environment.

```bash
make test
```
