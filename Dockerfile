FROM node

RUN apt-get update
RUN apt-get install -y \
  clang \
  cmake \
  make \
  git \
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

ARG LIGHTPATH_DIR="/home/lightpath"
ARG CINDER_DIR="${LIGHTPATH_DIR}/cinder"
ARG CINDER_TARGET_GL="es2-rpi"

WORKDIR ${LIGHTPATH_DIR}

RUN git clone --recursive --depth=1 https://github.com/cinder/Cinder.git ${CINDER_DIR}
WORKDIR "${CINDER_DIR}/build"
RUN cmake .. -DCMAKE_BUILD_TYPE=Release -DCINDER_TARGET_GL=${CINDER_TARGET_GL}
RUN make -j 3

# ADD build-cinder.sh .

# RUN ./build-cinder.sh
# RUN ./build-simulator.sh
# RUN npm install

# EXPOSE <port num>
# CMD ["node", "app.js"]
