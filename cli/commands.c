
#include "commands.h"


/* bitrate */
bool do_bitrate_set (int nb, const char **com, struct ngadmin *nga);
bool do_bitrate_show (int nb, const char **com, struct ngadmin *nga);


/* cabletest */
bool do_cabletest (int nb, const char **com, struct ngadmin *nga);


/* defaults */
bool do_defaults (int nb, const char **com, struct ngadmin *nga);


/* firmware */
bool do_firmware_show (int nb, const char **com, struct ngadmin *nga);
bool do_firmware_upgrade (int nb, const char **com, struct ngadmin *nga);


/* help */
bool do_help (int nb, const char **com, struct ngadmin *nga);


/* igmp */
bool do_igmp_set (int nb, const char **com, struct ngadmin *nga);
bool do_igmp_show (int nb, const char **com, struct ngadmin *nga);


/* list */
bool do_list (int nb, const char **com, struct ngadmin *nga);


/* login */
bool do_login (int nb, const char **com, struct ngadmin *nga);


/* mirror */
bool do_mirror_disable (int nb, const char **com, struct ngadmin *nga);
bool do_mirror_set (int nb, const char **com, struct ngadmin *nga);
bool do_mirror_show (int nb, const char **com, struct ngadmin *nga);


/* name */
bool do_name_show (int nb, const char **com, struct ngadmin *nga);
bool do_name_set (int nb, const char **com, struct ngadmin *nga);
bool do_name_clear (int nb, const char **com, struct ngadmin *nga);


/* netconf */
bool do_netconf_set (int nb, const char **com, struct ngadmin *nga);


/* password */
bool do_password_change (int nb, const char **com, struct ngadmin *nga);
bool do_password_set (int nb, const char **com, struct ngadmin *nga);


/* ports */
bool do_ports_state (int nb, const char **com, struct ngadmin *nga);
bool do_ports_statistics_reset (int nb, const char **com, struct ngadmin *nga);
bool do_ports_statistics_show (int nb, const char **com, struct ngadmin *nga);


/* qos */
bool do_qos_mode (int nb, const char **com, struct ngadmin *nga);
bool do_qos_set (int nb, const char **com, struct ngadmin *nga);
bool do_qos_show (int nb, const char **com, struct ngadmin *nga);


/* quit */
bool do_quit (int nb, const char **com, struct ngadmin *nga);


/* restart */
bool do_restart (int nb, const char **com, struct ngadmin *nga);


/* scan */
bool do_scan (int nb, const char **com, struct ngadmin *nga);


/* stormfilter */
bool do_stormfilter_enable (int nb, const char **com, struct ngadmin *nga);
bool do_stormfilter_disable (int nb, const char **com, struct ngadmin *nga);
bool do_stormfilter_set (int nb, const char **com, struct ngadmin *nga);
bool do_stormfilter_show (int nb, const char **com, struct ngadmin *nga);


/* tree */
bool do_tree (int nb, const char **com, struct ngadmin *nga);


/* vlan */
bool do_vlan_8021q_del (int nb, const char **com, struct ngadmin *nga);
bool do_vlan_8021q_set (int nb, const char **com, struct ngadmin *nga);
bool do_vlan_8021q_show (int nb, const char **com, struct ngadmin *nga);
bool do_vlan_mode_set (int nb, const char **com, struct ngadmin *nga);
bool do_vlan_mode_show (int nb UNUSED, const char **com UNUSED, struct ngadmin *nga);
bool do_vlan_pvid_set (int nb, const char **com, struct ngadmin *nga);
bool do_vlan_pvid_show (int nb, const char **com, struct ngadmin *nga);


/* commands structure */
COM_ROOT_START(coms)
	COM_START(bitrate)
		COM_TERM(set, do_bitrate_set, true)
		COM_TERM(show, do_bitrate_show, false)
	COM_END
	
	COM_TERM(cabletest, do_cabletest, true)
	
	COM_TERM(defaults, do_defaults, false)
	
	COM_START(firmware)
		COM_TERM(show, do_firmware_show, false)
		COM_TERM(upgrade, do_firmware_upgrade, true)
	COM_END
	
	COM_TERM(help, do_help, false)
	
	COM_START(igmp)
		COM_TERM(set, do_igmp_set, true)
		COM_TERM(show, do_igmp_show, false)
	COM_END
	
	COM_TERM(list, do_list, false)
	
	COM_TERM(login, do_login, true)
	
	COM_START(mirror)
		COM_TERM(disable, do_mirror_disable, false)
		COM_TERM(set, do_mirror_set, true)
		COM_TERM(show, do_mirror_show, false)
	COM_END
	
	COM_START(name)
		COM_TERM(show, do_name_show, false)
		COM_TERM(set, do_name_set, true)
		COM_TERM(clear, do_name_clear, false)
	COM_END
	
	COM_START(netconf)
		COM_TERM(set, do_netconf_set, true)
	COM_END
	
	COM_START(password)
		COM_TERM(change, do_password_change, true)
		COM_TERM(set, do_password_set, true)
	COM_END
	
	COM_START(ports)
		COM_TERM(state, do_ports_state, false)
		COM_START(statistics)
			COM_TERM(reset, do_ports_statistics_reset, false)
			COM_TERM(show, do_ports_statistics_show, false)
		COM_END
	COM_END
	
	COM_START(qos)
		COM_TERM(mode, do_qos_mode, true)
		COM_TERM(set, do_qos_set, true)
		COM_TERM(show, do_qos_show, false)
	COM_END
	
	COM_TERM(quit, do_quit, false)
	
	COM_TERM(restart, do_restart, false)
	
	COM_TERM(scan, do_scan, false)
	
	COM_START(stormfilter)
		COM_TERM(enable, do_stormfilter_enable, false)
		COM_TERM(disable, do_stormfilter_disable, false)
		COM_TERM(set, do_stormfilter_set, true)
		COM_TERM(show, do_stormfilter_show, false)
	COM_END
	
	COM_TERM(tree, do_tree, false)
	
	COM_START(vlan)
		COM_START(802.1q)
			COM_TERM(del, do_vlan_8021q_del, true)
			COM_TERM(set, do_vlan_8021q_set, true)
			COM_TERM(show, do_vlan_8021q_show, true)
		COM_END
		COM_START(mode)
			COM_TERM(set, do_vlan_mode_set, true)
			COM_TERM(show, do_vlan_mode_show, false)
		COM_END
		COM_START(port)
			COM_TERM(set, NULL, true)
			COM_TERM(show, NULL, false)
		COM_END
		COM_START(pvid)
			COM_TERM(set, do_vlan_pvid_set, true)
			COM_TERM(show, do_vlan_pvid_show, false)
		COM_END
	COM_END
COM_ROOT_END


