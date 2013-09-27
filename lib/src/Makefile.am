
lib_LTLIBRARIES = libngadmin.la

libngadmin_la_SOURCES = network.c bitrate.c firmware.c libconf.c mirror.c misc.c \
			netconf.c ports.c qos.c session.c vlan.c
libngadmin_la_CPPFLAGS = -DBUILD_LIB -I../include/ -I../../raw/include/
libngadmin_la_CFLAGS = -fno-strict-aliasing
libngadmin_la_LIBADD = ../../raw/src/librawnsdp.la

