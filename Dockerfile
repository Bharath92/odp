FROM drydockaarch64/u16:master

RUN sudo apt-get install -yy clang graphviz ruby-dev libpcap-dev automake autoconf libtool libssl-dev mscgen ccache linux-headers-`uname -r`
