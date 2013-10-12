
ACLOCAL_AMFLAGS = -I .

SUBDIRS = raw wireshark

if ENABLE_LIB
SUBDIRS += lib
endif

if ENABLE_CLI
SUBDIRS += cli
endif

if ENABLE_SPY
SUBDIRS += spy
endif

if ENABLE_EMU
SUBDIRS += emu
endif

