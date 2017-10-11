FROM drydockaarch64/u16:master

RUN sudo apt-get install -yy clang clang-3.8 gcc-4.8 g++-4.8 graphviz ruby-dev libpcap-dev automake autoconf libtool libssl-dev mscgen ccache linux-headers-`uname -r` kmod

ADD . /tmp/odp

RUN cd /tmp/odp && ./installDeps.sh

RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-4.8 40 --slave /usr/bin/g++ g++ /usr/bin/g++-4.8

RUN update-alternatives --install /usr/bin/clang clang /usr/bin/clang-3.8 40