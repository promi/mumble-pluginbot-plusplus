project('mumble-pluginbot-plusplus', 'cpp',
	default_options : ['cpp_std=c++14'])

uv_opt = get_option('uv')

opus_dep = dependency('opus', version : '>=1.1')
openssl_dep = dependency('openssl')
mpdclient_dep = dependency('libmpdclient')
protobuf_dep = dependency('protobuf')
uv_dep = dependency('libuv', version : '>=1.9.0', required : false)

protoc = find_program('protoc')

gen = generator(protoc,
  output    : ['@BASENAME@.pb.cc', '@BASENAME@.pb.h'],
  arguments : ['--proto_path=@CURRENT_SOURCE_DIR@', '--cpp_out=@BUILD_DIR@', '@INPUT@'])

generated = gen.process('Mumble.proto')

src_incdir = include_directories('src')

aither_lib = shared_library(
  'mumble-pluginbot-plusplus-aither',
  'src/aither/console-logger.cc',
  'src/aither/log.cc',
  'src/aither/logger.cc',
  'src/aither/void-logger.cc',
  include_directories : src_incdir)

filesystem_lib = shared_library(
  'mumble-pluginbot-plusplus-filesystem',
  'src/filesystem/path.cc',
  include_directories : src_incdir)

io_lib = shared_library(
  'mumble-pluginbot-plusplus-io',
  'src/io/raw-s16le-file-sample-reader.cc',
  'src/io/raw-s16le-file-sample-writer.cc',
  'src/io/sample-reader.cc',
  'src/io/sample-writer.cc',
  include_directories : src_incdir,
  link_with : filesystem_lib)

network_lib = shared_library(
  'mumble-pluginbot-plusplus-network',
  'src/network/tcp-socket.cc',
  include_directories : src_incdir)

openssl_lib = shared_library(
  'mumble-pluginbot-plusplus-openssl',
  'src/openssl/basic-input-output.cc',
  'src/openssl/memory-basic-input-output.cc',
  'src/openssl/openssl.cc',
  'src/openssl/pem.cc',
  'src/openssl/pkey/envelope.cc',
  'src/openssl/pkey/envelope-message-digest.cc',
  'src/openssl/pkey/rsa.cc',
  'src/openssl/ssl/context.cc',
  'src/openssl/ssl/method.cc',
  'src/openssl/ssl/socket.cc',
  'src/openssl/x509/certificate.cc',
  'src/openssl/x509/extension.cc',
  'src/openssl/x509/extension-factory.cc',
  'src/openssl/x509/name.cc',
  include_directories : src_incdir,
  dependencies : openssl_dep,
  link_with : network_lib)

opus_lib = shared_library(
  'mumble-pluginbot-plusplus-opus',
  'src/opus/decoder.cc',
  'src/opus/encoder.cc',
  include_directories : src_incdir,
  dependencies : opus_dep,
  link_with : aither_lib)  

mumble_lib = shared_library(
  'mumble-pluginbot-plusplus-mumble',
  generated,
  'src/mumble/audio-player.cc',
  'src/mumble/audio-recorder.cc',
  'src/mumble/cert-manager.cc',
  'src/mumble/channel.cc',
  'src/mumble/client.cc',
  'src/mumble/client-impl.cc',
  'src/mumble/connection.cc',
  'src/mumble/img-reader.cc',
  'src/mumble/messages.cc',
  'src/mumble/mumble-2-mumble.cc',
  'src/mumble/packet-data-stream.cc',
  'src/mumble/udp-packet.cc',
  'src/mumble/user.cc',
  include_directories : src_incdir,
  dependencies : [protobuf_dep, openssl_dep],
  link_with : [aither_lib, filesystem_lib, io_lib, network_lib, openssl_lib, opus_lib])

mpd_srcs = [
  'src/mpd/audio-format.cc',
  'src/mpd/client.cc',
  'src/mpd/playlist.cc',
  'src/mpd/song.cc',
  'src/mpd/stats.cc',
  'src/mpd/status.cc',
]

mpd_deps = [mpdclient_dep]
mpd_incdirs = [src_incdir]

uv_ok = false
if uv_opt == 'auto'
  if uv_dep.found()
    uv_ok = true
  endif
elif uv_opt == 'yes'
  if uv_dep.found()
    uv_ok = true
  else
    error('Missing uv dependency')
  endif
endif

if uv_ok
  mpd_srcs += 'src/mpd/status-listener-uvw.cc'
  mpd_incdirs += include_directories('uvw/src')
  mpd_deps += uv_dep
else
  mpd_srcs += 'src/mpd/status-listener.cc'
endif

mpd_lib = shared_library(
  'mumble-pluginbot-plusplus-mpd',
  mpd_srcs,
  include_directories : mpd_incdirs,
  dependencies : mpd_deps,
  link_with : [network_lib])

git_info_sh = find_program('tools/gen-git-info.sh')

git_info_gen = generator(git_info_sh, output : '@BASENAME@.hh', arguments : ['@INPUT@'])

git_info_generated = git_info_gen.process('src/git-info')

executable(
  'mumble-pluginbot-plusplus',
  git_info_generated,
  generated,
  'src/main.cc',
  'src/pluginbot/html.cc',
  'src/pluginbot/main.cc',
  'src/pluginbot/plugin.cc',
  'src/pluginbot/settings.cc',
  'src/pluginbot/setting-descriptors.cc',
  'src/pluginbot/plugins/messages.cc',
  'src/pluginbot/plugins/mpd.cc',
  'src/pluginbot/plugins/version.cc',
  'src/pluginbot/plugins/youtube.cc',
  'src/util/ffmpeg.cc',
  'src/util/image-magick.cc',
  'src/util/shell.cc',
  'src/util/string.cc',
  'src/util/youtube-dl.cc',
  include_directories : src_incdir,
  dependencies : protobuf_dep,
  link_with : [aither_lib, filesystem_lib, mpd_lib, mumble_lib, openssl_lib])
