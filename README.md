# mumble-pluginbot-plusplus

## Introduction

mumble-pluginbot-plusplus is an extensible [Mumble] (https://wiki.mumble.info/wiki/Main_Page) bot which can play audio, can be fed by different sources, and much more.

It is a C++ rewrite of several libraries and the original mumble-ruby-pluginbot written in Ruby:

- [opus-ruby] (https://github.com/dafoxia/opus-ruby)
- [mumble-ruby] (https://github.com/dafoxia/mumble-ruby)
- [mumble-ruby-pluginbot] (https://github.com/MusicGenerator/mumble-ruby-pluginbot) @[0ef9aa585dc83cae337db6e6f36266939d2b410c] (https://github.com/MusicGenerator/mumble-ruby-pluginbot/tree/0ef9aa585dc83cae337db6e6f36266939d2b410c)
- [ruby-mpd] (https://github.com/archSeer/ruby-mpd)

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
- FIFO reading / OPUS encoding / Streaming
- MPD plugin
- Version plugin
- Messages plugin
- YouTube plugin

What doesn't work:

- Some chat commands don't work
- Soundcloud plugin
- Bandcamp plugin
- Ektoplazm plugin
- Radiostream plugin
- CELT support
- Portaudio support
- WAV reading / writing
- Automatic MPD startup
- Probably more ;)

## Dependencies

The code is using modern C++ language features only present in C++14.

- GCC 4.9+ or Clang 3.8+
- autoconf
- automake
- libtool
- OpenSSL (libcrypto + libssl)
- libopus 1.1
- libprotobuf
- protobuf (for the protoc compiler)
- libmpdclient
- libuv (will most likely be an optional dependency in the future)
- uvw (is automatically pulled in via a Git submodule)

On Debian you can get the dependencies using Apt:

    $ sudo apt-get install build-essential automake libtool pkg-config libopus-dev libssl-dev libprotobuf-dev libmpdclient-dev protobuf-compiler

## Building + Installing

The repository is autotools-based, you have to regenerate the configure script:

    $ ./autogen.sh

Then the usual autotools process applies:

    $ ./configure
    $ make
    $ sudo make install
	
There is experimental support for Debian Jessie, to build there you should invoke configure like this:

    $ CXX='g++ -std=c++1y' ./configure --without-libuv

It should also work without installing, libtool provides a wrapper script for this:

    $ ./mumble-pluginbot-plusplus --help
    
A running mpd is currently necessary or the bot will crash, there is a script included to start mpd:

    $ tools/start-mpd.sh

However the script expects a working mpd.conf in `~/.config/mumble-pluginbot-plusplus/mpd.conf`. You can start with the example script from the mumble-ruby-pluginbot repository:

https://github.com/MusicGenerator/mumble-ruby-pluginbot/blob/master/templates/mpd.conf

## Debugging

When using `gdb` you should either install first and invoke `gdb` as usual or you can use `libtool`s execute mode to debug without installing:

    $ libtool --mode=execute gdb --args ./mumble-pluginbot-plusplus -h mumble.example.org ...
    
For the best debugging experience you should compile with debugging symbols and without optimizations:

    $ CXX='clang++ -std=c++14' CXXFLAGS='-O0 -ggdb' ./configure

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
- libmumble-pluginbot-plusplus-mpd - Utility classes to talk to a MPD server
- mumble-pluginbot-plusplus - The main binary containing the core Bot and the enabled plugins

