AC_INIT([mumble-pluginbot-plusplus], 0.3)
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([foreign])
AM_SILENT_RULES([yes])

AC_PROG_CXX
LT_INIT

PKG_CHECK_MODULES(MUMBLE_PLUGINBOT_PLUSPLUS, [opus >= 1.1 openssl protobuf libmpdclient
libuv >= 1.9.0])

AC_CONFIG_FILES([Makefile])

AC_OUTPUT

