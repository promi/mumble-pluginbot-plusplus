AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS = -I m4

AM_CPPFLAGS = \
	-Wall \
	-Wextra \
	-pedantic \
	-I$(srcdir)/src \
	-I$(builddir)/src \
	-I$(srcdir)/uvw/src \
	-I$(builddir)/src/mumble \
	$(OPUS_CFLAGS) \
	$(OPENSSL_CFLAGS) \
	$(PROTOBUF_CFLAGS) \
	$(MPDCLIENT_CFLAGS) \
	$(UV_CFLAGS)

lib_LTLIBRARIES = \
	libaither.la \
	libmumble-pluginbot-plusplus-io.la \
	libmumble-pluginbot-plusplus-network.la \
	libmumble-pluginbot-plusplus-openssl.la \
	libmumble-pluginbot-plusplus-opus.la \
	libmumble-pluginbot-plusplus-mumble.la \
	libmumble-pluginbot-plusplus-mpd.la

bin_PROGRAMS = mumble-pluginbot-plusplus

libaither_la_SOURCES = \
	src/aither/log.cc \
	src/aither/logger.cc \
	src/aither/console-logger.cc \
	src/aither/void-logger.cc

libmumble_pluginbot_plusplus_io_la_SOURCES = \
	src/io/sample-reader.cc \
	src/io/sample-writer.cc \
	src/io/raw-s16le-file-sample-reader.cc \
	src/io/raw-s16le-file-sample-writer.cc

libmumble_pluginbot_plusplus_network_la_SOURCES = \
	src/network/tcp-socket.cc

libmumble_pluginbot_plusplus_openssl_la_SOURCES = \
	src/openssl/basic-input-output.cc \
	src/openssl/memory-basic-input-output.cc \
	src/openssl/openssl.cc \
	src/openssl/pem.cc \
	src/openssl/pkey/envelope.cc \
	src/openssl/pkey/envelope-message-digest.cc \
	src/openssl/pkey/rsa.cc \
	src/openssl/ssl/context.cc \
	src/openssl/ssl/method.cc \
	src/openssl/ssl/socket.cc \
	src/openssl/x509/certificate.cc \
	src/openssl/x509/extension.cc \
	src/openssl/x509/extension-factory.cc \
	src/openssl/x509/name.cc

libmumble_pluginbot_plusplus_opus_la_SOURCES = \
	src/opus/decoder.cc \
	src/opus/encoder.cc

libmumble_pluginbot_plusplus_mumble_la_SOURCES = \
	src/mumble/audio-player.cc \
	src/mumble/audio-recorder.cc \
	src/mumble/cert-manager.cc \
	src/mumble/channel.cc \
	src/mumble/client.cc \
	src/mumble/connection.cc \
	src/mumble/img-reader.cc \
	src/mumble/messages.cc \
	src/mumble/mumble-2-mumble.cc \
	src/mumble/packet-data-stream.cc \
	src/mumble/udp-packet.cc \
	src/mumble/user.cc

nodist_libmumble_pluginbot_plusplus_mumble_la_SOURCES = \
	src/mumble/Mumble.pb.cc \
	src/mumble/Mumble.pb.h

libmumble_pluginbot_plusplus_mpd_la_SOURCES = \
	src/mpd/audio-format.cc \
	src/mpd/client.cc \
	src/mpd/playlist.cc \
	src/mpd/song.cc \
	src/mpd/stats.cc \
	src/mpd/status.cc \
	src/mpd/status-listener.cc

mumble_pluginbot_plusplus_SOURCES = \
	src/main.cc \
	src/pluginbot/html.cc \
	src/pluginbot/main.cc \
	src/pluginbot/plugin.cc \
	src/pluginbot/settings.cc \
	src/pluginbot/setting-descriptors.cc \
	src/pluginbot/plugins/messages.cc \
	src/pluginbot/plugins/mpd.cc \
	src/pluginbot/plugins/version.cc \
	src/pluginbot/plugins/youtube.cc \
	src/util/ffmpeg.cc \
	src/util/image-magick.cc \
	src/util/shell.cc \
	src/util/string.cc \
	src/util/youtube-dl.cc

libmumble_pluginbot_plusplus_openssl_la_LIBADD = \
	libmumble-pluginbot-plusplus-io.la \
	libmumble-pluginbot-plusplus-network.la

libmumble_pluginbot_plusplus_mumble_la_LIBADD = \
	libmumble-pluginbot-plusplus-io.la \
	libaither.la \
	libmumble-pluginbot-plusplus-network.la \
	libmumble-pluginbot-plusplus-openssl.la \
	libmumble-pluginbot-plusplus-opus.la

mumble_pluginbot_plusplus_LDADD = \
	$(OPUS_LIBS) \
	$(OPENSSL_LIBS) \
	$(PROTOBUF_LIBS) \
	$(MPDCLIENT_LIBS) \
	$(UV_LIBS) \
	-lm \
	libmumble-pluginbot-plusplus-mpd.la \
	libmumble-pluginbot-plusplus-mumble.la \
	-lstdc++fs

src/git-info.hh: $(HEADERS) $(SOURCES)
	$(srcdir)/tools/echo-git-info.sh > $(builddir)/src/git-info.hh

%.pb.cc %.pb.h: %.proto
	$(PROTOC) --proto_path=$(srcdir) --cpp_out=$(builddir) $^

dist_noinst_DATA = src/mumble/Mumble.proto

MOSTLYCLEANFILES = \
	src/git-info.hh \
	src/mumble/Mumble.pb.cc \
	src/mumble/Mumble.pb.h

BUILT_SOURCES = \
	src/git-info.hh \
	src/mumble/Mumble.pb.h

EXTRA_DIST = tools/echo-git-info.sh
