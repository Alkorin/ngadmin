
#include "commands.h"


/* bitrate */
bool do_bitrate_set (int argc, const char **argv, struct ngadmin *nga);
bool do_bitrate_show (int argc, const char **argv, struct ngadmin *nga);


/* cabletest */
bool do_cabletest (int argc, const char **argv, struct ngadmin *nga);


/* defaults */
bool do_defaults (int argc, const char **argv, struct ngadmin *nga);


/* firmware */
bool do_firmware_show (int argc, const char **argv, struct ngadmin *nga);
bool do_firmware_upgrade (int argc, const char **argv, struct ngadmin *nga);


/* help */
bool do_help (int argc, const char **argv, struct ngadmin *nga);


/* igmp */
bool do_igmp_set (int argc, const char **argv, struct ngadmin *nga);
bool do_igmp_show (int argc, const char **argv, struct ngadmin *nga);


/* list */
bool do_list (int argc, const char **argv, struct ngadmin *nga);


/* login */
bool do_login (int argc, const char **argv, struct ngadmin *nga);


/* mirror */
bool do_mirror_disable (int argc, const char **argv, struct ngadmin *nga);
bool do_mirror_set (int argc, const char **argv, struct ngadmin *nga);
bool do_mirror_show (int argc, const char **argv, struct ngadmin *nga);


/* name */
bool do_name_show (int argc, const char **argv, struct ngadmin *nga);
bool do_name_set (int argc, const char **argv, struct ngadmin *nga);
bool do_name_clear (int argc, const char **argv, struct ngadmin *nga);


/* netconf */
bool do_netconf_set (int argc, const char **argv, struct ngadmin *nga);


/* password */
bool do_password_change (int argc, const char **argv, struct ngadmin *nga);
bool do_password_set (int argc, const char **argv, struct ngadmin *nga);


/* ports */
bool do_ports_state (int argc, const char **argv, struct ngadmin *nga);
bool do_ports_statistics_reset (int argc, const char **argv, struct ngadmin *nga);
bool do_ports_statistics_show (int argc, const char **argv, struct ngadmin *nga);


/* qos */
bool do_qos_mode (int argc, const char **argv, struct ngadmin *nga);
bool do_qos_set (int argc, const char **argv, struct ngadmin *nga);
bool do_qos_show (int argc, const char **argv, struct ngadmin *nga);


/* quit */
bool do_quit (int argc, const char **argv, struct ngadmin *nga);


/* restart */
bool do_restart (int argc, const char **argv, struct ngadmin *nga);


/* scan */
bool do_scan (int argc, const char **argv, struct ngadmin *nga);


/* stormfilter */
bool do_stormfilter_enable (int argc, const char **argv, struct ngadmin *nga);
bool do_stormfilter_disable (int argc, const char **argv, struct ngadmin *nga);
bool do_stormfilter_set (int argc, const char **argv, struct ngadmin *nga);
bool do_stormfilter_show (int argc, const char **argv, struct ngadmin *nga);


/* tree */
bool do_tree (int argc, const char **argv, struct ngadmin *nga);


/* vlan */
bool do_vlan_8021q_del (int argc, const char **argv, struct ngadmin *nga);
bool do_vlan_8021q_set (int argc, const char **argv, struct ngadmin *nga);
bool do_vlan_8021q_show (int argc, const char **argv, struct ngadmin *nga);
bool do_vlan_mode_set (int argc, const char **argv, struct ngadmin *nga);
bool do_vlan_mode_show (int argc, const char **argv, struct ngadmin *nga);
bool do_vlan_pvid_set (int argc, const char **argv, struct ngadmin *nga);
bool do_vlan_pvid_show (int argc, const char **argv, struct ngadmin *nga);


/* commands structure */
COM_ROOT_START(commands)
	COM_START(bitrate)
		COM_TERM(set, do_bitrate_set)
		COM_TERM(show, do_bitrate_show)
	COM_END
	
	COM_TERM(cabletest, do_cabletest)
	
	COM_TERM(defaults, do_defaults)
	
	COM_START(firmware)
		COM_TERM(show, do_firmware_show)
		COM_TERM(upgrade, do_firmware_upgrade)
	COM_END
	
	COM_TERM(help, do_help)
	
	COM_START(igmp)
		COM_TERM(set, do_igmp_set)
		COM_TERM(show, do_igmp_show)
	COM_END
	
	COM_TERM(list, do_list)
	
	COM_TERM(login, do_login)
	
	COM_START(mirror)
		COM_TERM(disable, do_mirror_disable)
		COM_TERM(set, do_mirror_set)
		COM_TERM(show, do_mirror_show)
	COM_END
	
	COM_START(name)
		COM_TERM(show, do_name_show)
		COM_TERM(set, do_name_set)
		COM_TERM(clear, do_name_clear)
	COM_END
	
	COM_START(netconf)
		COM_TERM(set, do_netconf_set)
	COM_END
	
	COM_START(password)
		COM_TERM(change, do_password_change)
		COM_TERM(set, do_password_set)
	COM_END
	
	COM_START(ports)
		COM_TERM(state, do_ports_state)
		COM_START(statistics)
			COM_TERM(reset, do_ports_statistics_reset)
			COM_TERM(show, do_ports_statistics_show)
		COM_END
	COM_END
	
	COM_START(qos)
		COM_TERM(mode, do_qos_mode)
		COM_TERM(set, do_qos_set)
		COM_TERM(show, do_qos_show)
	COM_END
	
	COM_TERM(quit, do_quit)
	
	COM_TERM(restart, do_restart)
	
	COM_TERM(scan, do_scan)
	
	COM_START(stormfilter)
		COM_TERM(enable, do_stormfilter_enable)
		COM_TERM(disable, do_stormfilter_disable)
		COM_TERM(set, do_stormfilter_set)
		COM_TERM(show, do_stormfilter_show)
	COM_END
	
	COM_TERM(tree, do_tree)
	
	COM_START(vlan)
		COM_START(802.1q)
			COM_TERM(del, do_vlan_8021q_del)
			COM_TERM(set, do_vlan_8021q_set)
			COM_TERM(show, do_vlan_8021q_show)
		COM_END
		COM_START(mode)
			COM_TERM(set, do_vlan_mode_set)
			COM_TERM(show, do_vlan_mode_show)
		COM_END
		COM_START(port)
			COM_TERM(set, NULL)
			COM_TERM(show, NULL)
		COM_END
		COM_START(pvid)
			COM_TERM(set, do_vlan_pvid_set)
			COM_TERM(show, do_vlan_pvid_show)
		COM_END
	COM_END
COM_ROOT_END


