#!/bin/bash

BUILD_TYPE=Release

LIGHTPATH_DIR="/home/pi/lightpath"
BIN_DIR="${LIGHTPATH_DIR}/bin"
FADECANDY_DIR="${LIGHTPATH_DIR}/fadecandy"
SIMULATOR_DIR="${LIGHTPATH_DIR}/simulator"
CINDER_DIR="${LIGHTPATH_DIR}/cinder"
CINDER_TARGET_GL="es2-rpi"

sudo apt-get update
sudo apt-get install -y \
  clang \
  cmake \
  make \
  git \
  nodejs \
  libssl-dev \
  libxcursor-dev \
  libgles2-mesa-dev \
  zlib1g-dev \
  libfontconfig1-dev \
  libmpg123-dev \
  libsndfile1 \
  libsndfile1-dev \
  libpulse-dev \
  libasound2-dev \
  libcurl4-gnutls-dev \
  libgstreamer1.0-dev \
  libgstreamer-plugins-bad1.0-dev \
  libgstreamer-plugins-base1.0-dev \
  gstreamer1.0-libav \
  gstreamer1.0-alsa \
  gstreamer1.0-pulseaudio \
  gstreamer1.0-plugins-bad

export CC=/usr/bin/clang
export CXX=/usr/bin/clang++

git clone --recursive --depth=1 https://github.com/scanlime/fadecandy.git ${FADECANDY_DIR}
pushd ${FADECANDY_DIR}/server
make submodules
mkdir -p build
pushd build
cmake .. -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -DCMAKE_RUNTIME_OUTPUT_DIRECTORY=${BIN_DIR}
make -j 3
popd
popd

git clone --recursive --depth=1 https://github.com/cinder/Cinder.git ${CINDER_DIR}
git clone --recursive --depth=1 https://github.com/BanTheRewind/Cinder-Asio.git "${CINDER_DIR}/blocks/Asio"
mkdir -p "${CINDER_DIR}/build"
pushd "${CINDER_DIR}/build"
cmake .. -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -DCINDER_TARGET_GL=${CINDER_TARGET_GL}
make -j 3
popd

mkdir -p ${SIMULATOR_DIR}/build
pushd ${SIMULATOR_DIR}/build
cmake .. -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -DCINDER_TARGET_GL=${CINDER_TARGET_GL} -DCINDER_PATH=${CINDER_DIR} -DCMAKE_RUNTIME_OUTPUT_DIRECTORY=${BIN_DIR} -DCMAKE_CXX_FLAGS="-DCONFIG_HEADLESS"
make -j 3
popd
