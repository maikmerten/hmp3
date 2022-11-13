# hmp3 - Helix MP3 Encoder, builds on modern systems

## On the license of the Helix MP3 Encoder

This repository contains a source code copy of the Helix MP3 Encoder in the directory [./hmp3](./hmp3). Please study [./hmp3/LICENSE.txt](./hmp3/LICENSE.txt) for licensing information regarding source code and compiled programs.

## On the license of other files in this repository

Files in this repository outside of the [./hmp3](./hmp3) directory are licensed under the terms of the MIT License, unless noted otherwise.

## Compiling

A Makefile tested on Linux is provided in this directory. Provided the usual build systems are installed, a simple `make` should create a `hmp3` program binary in `builds/release`, along with compiled object files.

A binary with debug symbols can be generated with `make debug`.
