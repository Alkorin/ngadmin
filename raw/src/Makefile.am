
noinst_LTLIBRARIES = librawnsdp.la

librawnsdp_la_SOURCES = attr.c list.c net.c packet.c str.c
librawnsdp_la_CPPFLAGS = -I$(top_srcdir)/raw/include/ -I$(top_srcdir)/lib/include/
librawnsdp_la_CFLAGS = -fno-strict-aliasing
librawnsdp_la_LIBADD = $(RT_LIBS)

