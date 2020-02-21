#-------------------------------------------------------------------------------------------------------------
# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License. See https://go.microsoft.com/fwlink/?linkid=2090316 for license information.
#-------------------------------------------------------------------------------------------------------------

# https://github.com/microsoft/vscode-dev-containers/tree/v0.101.1/containers/ubuntu-18.04-git/.devcontainer/Dockerfile
FROM ubuntu:18.04

 ENV DEBIAN_FRONTEND=noninteractive
 RUN apt-get update \
    && apt-get -y install --no-install-recommends build-essential \
    gcc-5 g++-5 libtool cmake curl debconf-utils \
    git git-core libio-stringy-perl liblua5.1 \
    liblua5.1-dev libluabind-dev libmysql++ \
    libperl-dev libperl5i-perl libsodium-dev \
    libsodium23 libmysqlclient-dev lua5.1 \
    minizip make mariadb-client \
# optional, mariadb server
    mariadb-server \ 
# optional, debugging tools
    gdb valgrind \
#
    nano open-vm-tools unzip uuid-dev \
    zlibc wget \
#    #
#    # Clean up
    && apt-get autoremove -y \
    && apt-get clean -y \
    && rm -rf /var/lib/apt/lists/*
ENV DEBIAN_FRONTEND=dialog
EXPOSE 3306
EXPOSE 5558
EXPOSE 5559
EXPOSE 7000
EXPOSE 7001
EXPOSE 7002
EXPOSE 7003
EXPOSE 7004
EXPOSE 7005
EXPOSE 7778
EXPOSE 9000
EXPOSE 9001
EXPOSE 9080
EXPOSE 9081