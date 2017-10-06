#!/bin/bash -e

install_dpdk() {
  TARGET=${TARGET:-"arm64-armv8a-linuxapp-gcc"}
  git -c advice.detachedHead=false clone -q --depth=1 --single-branch --branch=v17.02 http://dpdk.org/git/dpdk dpdk
  pushd dpdk
  git log --oneline --decorate
  make config T=${TARGET} O=${TARGET}
  pushd ${TARGET}
  sed -ri 's,(CONFIG_RTE_LIBRTE_PMD_PCAP=).*,\1y,' .config
  popd
  make install T=${TARGET} DESTDIR="/opt/dpdk" EXTRA_CFLAGS="-fPIC"
  rm -r ./doc ./${TARGET}/app ./${TARGET}/build
  popd
}

install_netmap() {
  pushd /opt
    git -c advice.detachedHead=false clone -q --depth=1 --single-branch --branch=v11.2 https://github.com/luigirizzo/netmap.git
    pushd netmap/LINUX
    ./configure
    make
    sudo insmod ./netmap.ko
    popd
  popd
}

install_dpdk
#install_netmap
