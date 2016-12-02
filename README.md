# mumble-pluginbot-plusplus

## Introduction

mumble-pluginbot-plusplus is an extensible [Mumble] (https://wiki.mumble.info/wiki/Main_Page) bot which can play audio, can be fed by different sources, and much more.

It is a C++ rewrite of several libraries and the original mumble-ruby-pluginbot written in Ruby:

- [opus-ruby] (https://github.com/dafoxia/opus-ruby)
- [mumble-ruby] (https://github.com/dafoxia/mumble-ruby)
- [mumble-ruby-plugin-bot] (https://github.com/dafoxia/mumble-ruby-pluginbot)

Thanks to all the original authors! See the source code for copyright information.

## Status

The rewrite is **not complete** yet.

What works:

- Command line parsing (Pass "--help" to the executable to see the syntax)
- Creating a RSA private + public key pair (PEM format)
- Creating a X.509 certificate (PEM format)
- Connection to a Mumble server
- Auto moving to a channel after connect
- Some simple chat commands such as ".help"
- FIFO reading / OPUS encoding / Streaming (in theory, can't be tested at this stage)

What doesn't work:

- Some chat commands don't work
- Plugins
- CELT support
- Portaudio support
- WAV reading / writing
- Automatic MPD startup
- Probably more ;)

## Dependencies

The code is using modern C++ language features only present in C++14 and partly C++17. It currently uses the experimental filesystem library.

- GCC 6.2+ or Clang 3.8+
- autoconf + automake
- OpenSSL
- libopus 1.1
- libprotobuf
- protobuf (for the protoc compiler)

## Building + Installing

The repository is autotools-based, you have to regenerate the configure script:

    $ ./autogen.sh

Currently the protobuf files are not automatically build by the build scripts. You have to do this yourself:

    $ cd src/mumble && protoc --cpp_out=. Mumble.proto && cd -

Then the usual autotools process applies:

    $ ./configure
    $ make
    $ sudo make install

It should also work without installing, libtool provides a wrapper script for this:

    $ ./mumble-pluginbot-plusplus --help

## License

mumble-pluginbot-plusplus is licensed under the AGPLv3+ license. The original code is licensed under the MIT license.

## Code structure

The code consists of several libtool libraries and a binary:

- libaither - Contains a Log class with severity levels
- libmumble-pluginbot-plusplus-io - Basic File I/O
- libmumble-pluginbot-plusplus-network - TCP network socket
- libmumble-pluginbot-plusplus-openssl - Wrapper for OpenSSL (certificate + SSL/TLS)
- libmumble-pluginbot-plusplus-opus - libopus C++ wrapper
- libmumble-pluginbot-plusplus-mumble - Utility classes to talk to a Mumble server
- mumble-pluginbot-plusplus - The main binary containing the core Bot

