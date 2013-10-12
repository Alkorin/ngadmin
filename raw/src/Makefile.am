
noinst_LTLIBRARIES = librawnsdp.la
noinst_HEADERS = encoding_attr.h

librawnsdp_la_SOURCES = attr.c encoding.c encoding_attr.c list.c misc.c net.c
librawnsdp_la_CPPFLAGS = -I$(top_srcdir)/raw/include/ -I$(top_srcdir)/lib/include/
librawnsdp_la_CFLAGS = -fno-strict-aliasing
librawnsdp_la_LIBADD = $(RT_LIBS)

