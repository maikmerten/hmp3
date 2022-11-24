# hmp3 - Helix MP3 Encoder, with code maintenance

This repository contains a version of the Helix MP3 Encoder with some maintenance work applied to to build on modern systems. This targets mostly Linux-based machines (tested on x86-64 and ARM), but untested project files for Windows are still included.

## On the license of the Helix MP3 Encoder

This repository contains a source code copy of the Helix MP3 Encoder in the directory [./hmp3](./hmp3). Please study [./hmp3/LICENSE.txt](./hmp3/LICENSE.txt) for licensing information.

## On the license of other files in this repository

Files in this repository outside of the [./hmp3](./hmp3) directory are licensed under the terms of the MIT License, unless noted otherwise.

## Compiling

A Makefile tested on Linux is provided in this directory. With the usual build systems installed, a simple `make` should create a `hmp3` program binary in `builds/release`, along with compiled object files.

A binary with debug symbols can be generated with `make debug`.

## Use examples

* Create a ~128 kbps VBR MP3 file:
  
  `hmp3 input.wav output.mp3`

* Create a 128 kbps CBR MP3 file:
  
  `hmp3 input.wav output.mp3 -B64`

  (Note that `-B` denotes the bitrate per channel, thus stereo input files are being encoded with 128 kbps.)

* Create a ~185 kbps VBR MP3 file, encode frequencies above 16 kHz, with a highpass filter of 19 kHz applied:
  
  `hmp3 input.wav output.mp3 -V100 -HF2 -F19000`


