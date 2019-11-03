README
---

Design a architecture of SDK with kconfig.

# Quickly Start

```shell
$ make menuconfig
  or
$ make xxx_defconfig # in configs folder
```

+ dependency

```shell
$ sudo apt-get -y install flex gperf libncurses5-dev libreadline6 libreadline6-dev automake autoconf libtool build-essential cmake make gcc texinfo pkg-config gettext git tig
```

+ Qemu

    - In QEMU monitor press `Ctrl-Alt-2`
        > switch to `serial0 interface` (test UART I/O)

    - in serial0 press `Ctrl-Alt-1`
        switch to QEMU monitor


# Reference
* [esp-idf](https://github.com/espressif/esp-idf)
* [Kernel](https://www.kernel.org/) linux kernel.
* [F9-kernel](https://github.com/f9micro/f9-kernel) An efficient and secure microkernel built for ARM Cortex-M cores, inspired by L4.
* [qemu-system-gnuarmeclipse](https://github.com/xpack-dev-tools/qemu-arm-xpack/releases/)
    > this Qemu is for Cortex-M serial
