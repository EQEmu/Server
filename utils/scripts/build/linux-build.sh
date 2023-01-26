#!/bin/bash

set -x

sudo chown eqemu:eqemu /drone/src/ * -R
sudo chown eqemu:eqemu /home/eqemu/.ccache/ * -R

git submodule init && git submodule update

perl utils/scripts/build/tag-version.pl

mkdir -p build && cd build && cmake -DEQEMU_BUILD_TESTS=ON -DEQEMU_BUILD_LOGIN=ON -DEQEMU_BUILD_LUA=ON -DCMAKE_CXX_FLAGS_RELWITHDEBINFO:STRING="-Os" -DCMAKE_CXX_COMPILER_LAUNCHER=ccache -G 'Unix Makefiles' .. && make -j$((`nproc`-4))

curl https://raw.githubusercontent.com/Akkadius/eqemu-install-v2/master/eqemu_config.json --output eqemu_config.json
./bin/tests

# shellcheck disable=SC2164
cd /drone/src/

chmod +x ./utils/scripts/build/should-release/should-release
./utils/scripts/build/should-release/should-release || exit

rm ./build/bin/*.a
zip -j eqemu-server-linux-x64.zip ./build/bin/*

# shellcheck disable=SC2010
ls -lsh | grep zip
sudo apt-get update && sudo apt-get install -y rclone
rclone config create remote ftp env_auth true > /dev/null
rclone copy eqemu-server-linux-x64.zip remote:
rclone ls remote:
