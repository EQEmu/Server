#!/bin/bash

set -ex

sudo chown eqemu:eqemu /drone/src/ * -R

# Install ClangBuildAnalyzer if missing
if ! command -v clang-build-analyzer &> /dev/null; then
  echo "Installing latest Clang Build Analyzer..."
  LATEST_VERSION=$(curl -s https://api.github.com/repos/aras-p/ClangBuildAnalyzer/releases/latest \
    | grep '"tag_name":' | sed -E 's/.*"v([^"]+)".*/\1/')
  sudo curl -sSL "https://github.com/aras-p/ClangBuildAnalyzer/releases/download/v${LATEST_VERSION}/ClangBuildAnalyzer-linux" \
    -o /usr/local/bin/clang-build-analyzer
  sudo chmod +x /usr/local/bin/clang-build-analyzer
fi

git submodule init && git submodule update
perl utils/scripts/build/tag-version.pl

mkdir -p build

clang-build-analyzer --start build/

cd build && \
  cmake -DEQEMU_BUILD_TESTS=ON \
      -DCMAKE_BUILD_TYPE=Debug \
      -DEQEMU_BUILD_LUA=ON \
      -DEQEMU_BUILD_PERL=ON \
      -DEQEMU_BUILD_LOGIN=ON \
      -DEQEMU_BUILD_STATIC=ON \
      -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
      -DCMAKE_CXX_FLAGS:STRING="-O0 -g -Wno-everything -ftime-trace" \
      -G 'Unix Makefiles' \
      .. && \
  make -j"$(nproc)"

# 🧠 Generate ClangBuildAnalyzer report
clang-build-analyzer --stop ./ /tmp/eqemu.capture
clang-build-analyzer --analyze /tmp/eqemu.capture > report.txt
cat report.txt

ldd ./bin/zone

cd /drone/src/
