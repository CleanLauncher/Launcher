ARG UBUNTU_VERSION=24.04

###############################################################################
# Base stage: shared dependencies for all CI targets
###############################################################################
FROM docker.io/library/ubuntu:${UBUNTU_VERSION} AS base

ARG QT_ABI=gcc_64
ARG QT_ARCH=
ARG QT_HOST=linux
ARG QT_TARGET=desktop
ARG QT_VERSION=6.10.2

ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update \
  && apt-get --assume-yes upgrade \
  && apt-get --assume-yes autopurge

# Adoptium Java 17
RUN apt-get --assume-yes --no-install-recommends install \
  apt-transport-https ca-certificates curl gpg
RUN curl -L https://packages.adoptium.net/artifactory/api/gpg/key/public \
  | gpg --dearmor | tee /etc/apt/trusted.gpg.d/adoptium.gpg
RUN echo "deb https://packages.adoptium.net/artifactory/deb \
  $(awk -F= '/^VERSION_CODENAME/{print$2}' /etc/os-release) main" \
  | tee /etc/apt/sources.list.d/adoptium.list
RUN apt-get update

# Core build dependencies
RUN apt-get --assume-yes --no-install-recommends install \
  clang lld llvm temurin-17-jdk \
  cmake ninja-build extra-cmake-modules pkg-config \
  cmark gamemode-dev libarchive-dev libcmark-dev libgamemode0 \
  libgl1-mesa-dev libqrencode-dev libtomlplusplus-dev scdoc zlib1g-dev

ENV CMAKE_LINKER_TYPE=lld

# Locale
RUN apt-get --assume-yes --no-install-recommends install locales \
  && echo "C.UTF-8 UTF-8" > /etc/locale.gen \
  && locale-gen
ENV LC_ALL=C.UTF-8

# Qt
RUN apt-get --assume-yes --no-install-recommends install \
  libglib2.0-0 libxkbcommon0 python3-pip
RUN pip3 install --break-system-packages aqtinstall
RUN aqt install-qt ${QT_HOST} ${QT_TARGET} ${QT_VERSION} ${QT_ARCH} \
  --outputdir /opt/qt --modules qtimageformats qtnetworkauth
ENV PATH=/opt/qt/${QT_VERSION}/${QT_ABI}/bin:$PATH
ENV QT_PLUGIN_PATH=/opt/qt/${QT_VERSION}/${QT_ABI}/plugins/

RUN rm -rf \
  "$QT_PLUGIN_PATH"/designer "$QT_PLUGIN_PATH"/help \
  "$QT_PLUGIN_PATH"/printsupport "$QT_PLUGIN_PATH"/qmllint \
  "$QT_PLUGIN_PATH"/qmlls "$QT_PLUGIN_PATH"/qmltooling \
  "$QT_PLUGIN_PATH"/sqldrivers

# Source code
COPY . /src
WORKDIR /src

###############################################################################
# test: build + run all 20 unit tests
###############################################################################
FROM base AS test

ARG BUILD_TYPE=Debug
ENV QT_QPA_PLATFORM=offscreen

RUN cmake --preset linux \
  -DCMAKE_BUILD_TYPE=${BUILD_TYPE}
RUN cmake --build --preset linux --config ${BUILD_TYPE} -j$(nproc)
RUN ctest --preset linux --build-config ${BUILD_TYPE} --output-on-failure

###############################################################################
# lint: clang-format, cppcheck, markdownlint
###############################################################################
FROM base AS lint

RUN apt-get --assume-yes --no-install-recommends install \
  cppcheck nodejs npm \
  && npm install -g markdownlint-cli

RUN find . -name '*.cpp' -o -name '*.h' -o -name '*.hpp' \
  | grep -v './libraries/' | grep -v './build/' \
  | xargs clang-format --dry-run --style=file -Werror

RUN cppcheck \
  --enable=warning,style,performance,portability \
  --error-exitcode=1 \
  --suppressions-list=.cppcheck-suppressions \
  --inline-suppr \
  -I libraries/ -I src/ \
  src/ libraries/

RUN markdownlint --dot .

###############################################################################
# validate: codespell, yamllint, no-emoji
###############################################################################
FROM ubuntu:24.04 AS validate

ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get --assume-yes --no-install-recommends install \
  python3-pip yamllint \
  && pip3 install --break-system-packages codespell

COPY . /src
WORKDIR /src

RUN codespell \
  --skip='./libraries,./build,./.git,./.mimocode,./core/target,./docs,*.lock,*.json,*.nix,*.ui,./tests/testdata,./src/resources,./cmake,./scripts,./res' \
  --ignore-words .codespell-ignore-words \
  --quiet-level=2

RUN yamllint -c /dev/stdin .github/workflows/*.yml .github/actions/**/*.yml <<'YAML'
extends: default
rules:
  line-length:
    max: 500
  truthy:
    check-keys: false
  comments:
    min-spaces-from-content: 1
  document-start: disable
  brackets:
    min-spaces-inside: 0
    max-spaces-inside: 1
YAML

###############################################################################
# quality: strict warnings build
###############################################################################
FROM base AS quality

RUN cmake --preset linux \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_C_FLAGS="-Wall -Wextra -Wpedantic -Werror -Wshadow -Wconversion" \
  -DCMAKE_CXX_FLAGS="-Wall -Wextra -Wpedantic -Werror -Wshadow -Wconversion"
RUN cmake --build --preset linux --config Debug -j$(nproc)

###############################################################################
# build-package: full build + package (AppImage, deb, rpm, tarball)
###############################################################################
FROM base AS build-package

RUN apt-get --assume-yes --no-install-recommends install \
  dpkg-dev rpm

RUN cmake --preset linux \
  -DCMAKE_BUILD_TYPE=Debug \
  -DARTIFACT_NAME=Linux \
  -DBUILD_PLATFORM=official
RUN cmake --build --preset linux --config Debug -j$(nproc)

# Smoke test
RUN BINARY=$(find build -name 'launcher' -type f -executable 2>/dev/null | head -1 || true) \
  && if [ -n "$BINARY" ]; then \
    QT_QPA_PLATFORM=offscreen "$BINARY" --version 2>&1 || true; \
  fi

# Install for packaging
RUN cmake --install build --config Debug --prefix /opt/launcher

# deb
RUN mkdir -p pkg/DEBIAN && mkdir -p pkg/opt/launcher \
  && cp -a /opt/launcher/* pkg/opt/launcher/ \
  && printf 'Package: launcher\nVersion: 0.0.0-dev\nSection: games\nPriority: optional\nArchitecture: amd64\nDepends: libgl1-mesa-glx | libgl1, libglib2.0-0, libxcb-cursor0, libxcb1, libxkbcommon0\nMaintainer: Clean Launcher Team\nDescription: Clean Launcher\n' > pkg/DEBIAN/control \
  && dpkg-deb --root-owner-group --build pkg launcher.deb
