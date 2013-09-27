
bin_PROGRAMS = admin

admin_SOURCES = admin.c com_bitrate.c com_cabletest.c com_defaults.c com_firmware.c \
		com_help.c com_igmp.c com_list.c com_login.c commands.c com_mirror.c \
		common.c com_name.c com_netconf.c com_password.c com_ports.c com_qos.c \
		com_quit.c com_restart.c com_scan.c com_stormfilter.c com_tree.c com_vlan.c
admin_CPPFLAGS = -I$(top_srcdir)/lib/include/
admin_LDADD = $(top_builddir)/lib/src/libngadmin.la

