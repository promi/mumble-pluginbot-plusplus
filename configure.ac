AC_INIT(
  [mumble-pluginbot-plusplus],
  [0.9],
  [https://github.com/promi/mumble-pluginbot-plusplus/issues],
  [],
  [https://github.com/promi/mumble-pluginbot-plusplus])

AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([foreign])
AM_SILENT_RULES([yes])

AC_PROG_CXX
#AX_CXX_COMPILE_STDCXX_14([noext], [mandatory])
AC_CHECK_PROG([PROTOC], [protoc], [protoc])
AS_IF([test "x${PROTOC}" = x],
    [AC_MSG_ERROR(["ProtoBuf compiler 'protoc' not found."])])
LT_INIT

# Make sure that pkg-config is installed!
# The PKG_CHECK_MODULES macro prints a horrible error message when
# pkg-config is not installed at autogen time.
#
# It is also required when the first PKG_CHECK_MODULES is inside a conditional
PKG_PROG_PKG_CONFIG

PKG_CHECK_MODULES(OPUS, [opus >= 1.1])
PKG_CHECK_MODULES(OPENSSL, [openssl])
PKG_CHECK_MODULES(PROTOBUF, [protobuf])
PKG_CHECK_MODULES(MPDCLIENT, [libmpdclient])

AC_ARG_WITH(
  [libuv],
  [AS_HELP_STRING([--with-libuv], [support efficient MPD status polling @<:@default=check@:>@])],
  [],
  [with_libuv=check])

AS_CASE(
  ["$with_libuv"],
  [yes], [PKG_CHECK_MODULES(UV, [libuv >= 1.9.0], [HAVE_LIBUV=1],
                            [AC_MSG_ERROR("libuv >= 1.9.0 is not installed")])],
  [no],  [HAVE_LIBUV=0],
         [PKG_CHECK_MODULES(UV, [libuv >= 1.9.0], [HAVE_LIBUV=1], [HAVE_LIBUV=0])])

AS_IF([test "x$HAVE_LIBUV" = x1], [AC_DEFINE([USE_LIBUV])])
AM_CONDITIONAL([USE_LIBUV], [test "x$HAVE_LIBUV" = x1])

#AC_CONFIG_HEADERS([src/config.h])
AC_CONFIG_FILES([Makefile])

AC_OUTPUT
