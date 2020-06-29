# Table Redundancy Method for White box AES

A white-box AES implementation for [table redundancy method](https://eprint.iacr.org/2019/959.pdf) for protecting against Differential Fault Analysis in white-box cryptography.

This is an implementation with two redundant computations of [Chow et al.'s scheme](https://link.springer.com/chapter/10.1007/3-540-36492-7_17).

## Build

```
$ mkdir build
$ cd build
$ cmake ..
$ make
```

## Run

```
$ ./WBAES
```

## Included libraries
1. [WBMatrix](https://github.com/Nexus-TYF/WBMatrix)<br>
2. [WhiteBoxAES](https://github.com/Gr1zz/WhiteBoxAES)<br>
