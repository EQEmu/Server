FROM debian:stretch

ENV EQEMU_DOCKER=20170710 \
LANG=en_US.UTF-8

# This needs to run as one continuous block for release to keep image size down
RUN apt-get update \
&& apt-get install -yqq gettext-base mlocate curl wget \
&& curl https://raw.githubusercontent.com/EQEmu/Server/master/utils/scripts/linux_installer/install.sh | bash \
&& apt-get purge -yqq mysql-server \
&& apt-get autoremove -qqy \
&& apt-get clean \
&& rm -rf /var/lib/apt/lists/*

RUN useradd emu \
&& mkdir -p /home/emu \
&& chown -R emu. /home/emu

USER emu
WORKDIR /home/emu

USER emu

CMD ["/bin/bash"]
