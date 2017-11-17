
/**
 * User interface file. 
 * All client applications which want to use NgAdmin must include this file. 
 * @file ngadmin.h
 */


#ifndef DEF_NGADMIN
#define DEF_NGADMIN


#include <stdbool.h>
#include <arpa/inet.h>

#include <netinet/ether.h>



/**
 * Maximum size of product string. 
 */
#define PRODUCT_SIZE	64

/**
 * Maximum size of name string. 
 **/
#define NAME_SIZE	64

/**
 *  Maximum size of firmware version string. 
 **/
#define FIRMWARE_SIZE	64



/**
 * Error codes. 
 * This enum lists all the error codes the library can return to user. 
 **/
enum {
	ERR_OK = 0,			/**< no error */
	ERR_NET = -1,			/**< network error */
	ERR_NOTLOG = -2,		/**< not logged */
	ERR_DENIED = -3,		/**< access denied */
	ERR_BADPASS = -4,		/**< bad password */
	ERR_BADID = -5,			/**< bad switch id */
	ERR_INVARG = -6,		/**< invalid argument */
	ERR_TIMEOUT = -7,		/**< timeout */
	ERR_MEM = -8,			/**< out of memory */
	ERR_NOTIMPL = -9,		/**< not implemented */
	ERR_BADREPLY = -10,		/**< bad reply */
	ERR_INVOP = -11,		/**< invalid operation */
	ERR_UNKNOWN = -12		/**< unknown error */
};



/**
 * Port speeds. 
 * This enum lists all the speeds a port can have. 
 **/
enum {
	SPEED_UNK = -1,			/**< unknown status */
	SPEED_DOWN = 0,			/**< link down */
	SPEED_10_HD = 1,		/**< 10 Mb/s half duplex */
	SPEED_10_FD = 2,		/**< 10 Mb/s full duplex */
	SPEED_100_HD = 3,		/**< 100 Mb/s half duplex */
	SPEED_100_FD = 4,		/**< 100 Mb/s full duplex */
	SPEED_1000_FD = 5		/**< 1000 Mb/s full duplex */
};



/**
 * VLAN types. 
 * This enum lists all the VLAN types available
 **/
enum {
	VLAN_DISABLED = 0,		/**< VLAN disabled */
	VLAN_PORT_BASIC = 1,		/**< port basic */
	VLAN_PORT_ADV = 2,		/**< port advanced */
	VLAN_DOT_BASIC = 3,		/**< 802.1q basic */
	VLAN_DOT_ADV = 4		/**< 802.1q advanced */
};


/**
 * VLAN port specification. 
 * This enum lists all the VLAN specifications a port can have. 
 **/
enum {
	VLAN_UNSPEC = 0xFF,		/**< unspecified */
	VLAN_NO = 0,			/**< not present */
	VLAN_UNTAGGED = 1,		/**< present, untagged */
	VLAN_TAGGED = 2			/**< present, tagged */
};



/**
 * Minimum VLAN id. 
 **/
#define VLAN_MIN		1

/**
 * Maximum 802.1q VLAN id. 
 **/
#define VLAN_DOT_MAX		4093

/**
 * Maximum port VLAN id.
 **/
#define VLAN_PORT_MAX		9


/**
 * QoS modes. 
 * This enum lists all the availables QoS modes. 
 **/
enum {
	QOS_PORT = 1,			/**< port based */
	QOS_DOT = 2			/**< 802.1p based */
};


/**
 * Port priorities. 
 * This enum lists all the priorities a port can have. 
 **/
enum {
	PRIO_UNSPEC = -1,		/**< unspecified */
	PRIO_HIGH = 1,			/**< high */
	PRIO_MED = 2,			/**< medium */
	PRIO_NORM = 3,			/**< normal */
	PRIO_LOW = 4			/**< low */
};




/**
 * Bitrates. 
 * This enum lists all the available bitrates. 
 **/
enum {
	BITRATE_UNSPEC = -1,	/**< unspecified */
	BITRATE_NOLIMIT = 0,	/**< unlimited */
	BITRATE_512K = 1,	/**< 512 Kb/s */
	BITRATE_1M = 2,		/**< 1 Mb/s */
	BITRATE_2M = 3,		/**< 2 Mb/s */
	BITRATE_4M = 4,		/**< 4 Mb/s */
	BITRATE_8M = 5,		/**< 8 Mb/s */
	BITRATE_16M = 6,	/**< 16 Mb/s */
	BITRATE_32M = 7,	/**< 32 Mb/s */
	BITRATE_64M = 8,	/**< 64 Mb/s */
	BITRATE_128M = 9,	/**< 128 Mb/s */
	BITRATE_256M = 10,	/**< 256 Mb/s */
	BITRATE_512M = 11	/**< 512 Mb/s */
};




/**
 * NgAdmin library main structure. 
 * The structure content is hidden to clients to prevent them to manually 
 * change data and mess up things. 
 **/
struct ngadmin;



/**
 * Network configuration. 
 * Represents the network configuration of a switch. 
 */
struct net_conf {
	struct in_addr ip;		/**< IP */
	struct in_addr netmask;		/**< netmask */
	struct in_addr gw;		/**< gateway IP */
	bool dhcp;			/**< DHCP enabled */
};


/**
 * Switch characteristics. 
 * Represents the main characteristics of a switch. 
 */
struct swi_attr {
	char product[PRODUCT_SIZE];	/**< product name (eg.\ GS108EV1) */
	char name[NAME_SIZE];		/**< custom name */
	char firmware[FIRMWARE_SIZE];	/**< firmware version string */
	unsigned char ports;		/**< number of ports */
	struct ether_addr mac;		/**< MAC address */
	struct net_conf nc;		/**< network configuration */
};


/**
 * Port statistics. 
 * Represents statistics of a particular port. 
 */
struct port_stats {
	unsigned long long recv;	/**< packets received */
	unsigned long long sent;	/**< packets sent */
	unsigned long long crc;		/**< CRC errors */
};


/**
 * IGMP snooping configuration. 
 * Represents the IGMP snooping configuration of a switch. 
 */
struct igmp_conf {
	bool enable;			/**< IGMP snooping enabled */
	unsigned short vlan;		/**< VLAN on which IGMP snooping is done */
	bool validate;			/**< validate IGMPv3 headers */
	bool block;			/**< block unknown multicast addresses */
};


/**
 * Cabletest result.
 */
struct cabletest {
	char port;		/**< port */
	int test_result;			/**< test result code (see enum)  */
	int fault_distance;			/**< fault distance / cable length in meters */
};

/**
 * Cabletest result codes
 */
enum {
	CABLETEST_OK = 0, //  System0056 = OK
	CABLETEST_NO_CABLE = 1, //  System0052 = No Cable
	CABLETEST_OPEN_CABLE = 2, //  System0053 = Open Cable
	CABLETEST_SHORT_CIRCUIT = 3, //  System0054 = Short Circuit
	CABLETEST_FIBER_CABLE = 4, //  System0055 = Fiber Cable
	CABLETEST_SHORTED_CABLE = 5, //  System0064 = Shorted cable
	CABLETEST_UNKNOWN = 6, //  System0065 = Unknown
	CABLETEST_CROSSTALK = 7 //  System0066 = Crosstalk
};

#ifdef __cplusplus
extern "C" {
#endif


/**
 * Initialize NgAdmin library. 
 * This function initializes the NgAdmin library. You must call it before any 
 * other function. 
 * @param iface The network interface to use. 
 * @return A pointer to a ngadmin structure, or NULL if an error occurred. 
 */
struct ngadmin* ngadmin_init (const char *iface);


/**
 * Convert error to string.
 * This function returns a string corresponding to the numerical error code.
 * @param error The numerical error code to convert.
 * @return A pointer to a static string or NULL if the error code is invalid.
 */
const char* ngadmin_errorStr (int error);


/**
 * Close NgAdmin library. 
 * This function frees the resources used by the library. You really should 
 * call this when you are done using the library. 
 * @param nga A pointer to the ngadmin structure. 
 * @return ERR_OK when everything is well or an error code otherwise. 
 */
int ngadmin_close (struct ngadmin *nga);


/**
 * Force the use of the interface. 
 * This function allows to solve two problems: 
 * - When you have multiple network interfaces, sending to the global broadcast 
 *   address may not send the packet on the interface you want. \n
 *   This function forces the packet to go on the interface you specified at 
 *   the library initialization. 
 * - When the switch is not in your network range, because DHCP is disabled or
 *   you started the DHCP server after the switch. \n
 *   This function allows you to ignore the routing table and consider every 
 *   address is directly reachable on the interface. \n
 *   An alternative is to use ngadmin_setKeepBroadcasting, or simply add a route. 
 * 
 * @warning Requires root priviledges. 
 * @see ngadmin_setKeepBroadcasting()
 * @param nga A pointer to the ngadmin structure. 
 * @return ERR_OK when everything is well or an error code otherwise. 
 */
int ngadmin_forceInterface (struct ngadmin *nga);


/**
 * Keep broadcasting even when talking with a particular switch. 
 * By default, once you login on a switch, NgAdmin talks with it using unicast. 
 * This prevents the password from being sent to all your network. \n
 * The switch still replies using broadcast, but the password is not included 
 * in the replies. \n
 * This function allows you to disable this feature and do like the official 
 * Windows application that always use broadcast packets. \n
 * This also allows to configure a switch which is not on your network range 
 * without forcing the interface. \n
 * When you enable this option, you must be aware that on every parameter 
 * change you make on the switch, your password is broadcasted in cleartext 
 * to all your network. 
 * @see ngadmin_forceInterface()
 * @param nga A pointer to the ngadmin structure. 
 * @param value Enable or disable the systematic use of broadcast packets. 
 * @return ERR_OK when everything is well or an error code otherwise. 
 **/
int ngadmin_setKeepBroadcasting (struct ngadmin *nga, bool value);


/**
 * Use global broadcast address. 
 * By default, NgAdmin uses the interface broadcast address. 
 * This option forces the lib to use the global broadcast address 
 * (255.255.255.255) instead. 
 * @warning If you have multiple interfaces, enabling this may cause problems. 
 * @see ngadmin_forceInterface()
 * @param nga A pointer to the ngadmin structure. 
 * @param value Enable or disable the use of the global broadcast address. 
 * @return ERR_OK when everything is well or an error code otherwise. 
 **/
int ngadmin_useGlobalBroadcast (struct ngadmin *nga, bool value);


/**
 * Specify the password to use to login. 
 * Sets the password to use to login on switches. 
 * @param nga A pointer to the ngadmin structure. 
 * @param pass The password string to use. 
 * @return ERR_OK when everything is well or an error code otherwise. 
 **/
int ngadmin_setPassword (struct ngadmin *nga, const char *pass);


/**
 * Set timeout for networking. 
 * Sets the timeout when waiting for network packets. 
 * @param nga A pointer to the ngadmin structure. 
 * @param tv A pointer to a timeval structure. 
 * @return ERR_OK when everything is well or an error code otherwise. 
 **/
int ngadmin_setTimeout (struct ngadmin *nga, const struct timeval *tv);


/**
 * Scan the network for switches. 
 * This function scans the network for Netgear switches that use NSDP. 
 * @warning Systematically blocks for the timeout value. 
 * @note If you are logged on a switch, calling this function will delog you. 
 * @param nga A pointer to the ngadmin structure. 
 * @return ERR_OK when everything is well or an error code otherwise. 
 **/
int ngadmin_scan (struct ngadmin *nga);


/**
 * Get the list of detected switches. 
 * This function allows you gou get the list of all last detected switchs. 
 * @note When a scan is done, this array is no more valid. 
 * @param nga A pointer to the ngadmin structure. 
 * @param nb A pointer to an integer which will receive the number of switches. 
 * @return A pointer to an array of switch characteristics. 
 */
const struct swi_attr* ngadmin_getSwitchTab (struct ngadmin *nga, int *nb);


/**
 * Get the switch on which you are logged. 
 * This function allows you to get the switch on which you are logged. 
 * @param nga A pointer to the ngadmin structure. 
 * @return A pointer the switch characteristics or NULL if you are not logged. 
 **/
const struct swi_attr* ngadmin_getCurrentSwitch (struct ngadmin *nga);



/**
 * Upgrade the switch firmware. 
 * This function allows you to upgrade the switch firmware. 
 * @warning Currently not implemented. 
 * @note You must be logged on a switch. 
 * @param nga A pointer to the ngadmin structure. 
 * @param filename A path to the file of the new firmware to send. 
 * @return ERR_NOTIMPL
 **/
int ngadmin_upgradeFirmware (struct ngadmin *nga, const char *filename);


/**
 * Login on a switch. 
 * This function permits to login on a switch. 
 * @note If you are already logged, this function delogs you whatever the new 
 *       login attempt is successfull or not. 
 * @see ngadmin_setPassword()
 * @param nga A pointer to the ngadmin structure. 
 * @param id The id (position in the switch array) of the switch you want to login to. 
 * @return ERR_OK when everything is well or an error code otherwise. 
 **/
int ngadmin_login (struct ngadmin *nga, int id);


/**
 * Get the ports speed status. 
 * This functions retrieves the ports speed status. 
 * @note You must be logged on a switch. 
 * @param nga A pointer to the ngadmin structure. 
 * @param ports A pointer to an array of ports which will receive ports status. 
 *         Must not be NULL. The array size must be ports_count*sizeof(unsigned char). 
 * @return ERR_OK when everything is well or an error code otherwise. 
 **/
int ngadmin_getPortsStatus (struct ngadmin *nga, unsigned char *ports);


/**
 * Change the name of a switch. 
 * This changes the name of a switch. 
 * @note You must be logged on a switch. 
 * @param nga A pointer to the ngadmin structure. 
 * @param name The name string to use. A NULL value clears the name. 
 * @return ERR_OK when everything is well or an error code otherwise. 
 **/
int ngadmin_setName (struct ngadmin *nga, const char *name);


/**
 * Get the ports statistics. 
 * Retrieves the ports packet statistics. 
 * @note You must be logged on a switch. 
 * @param nga A pointer to the ngadmin structure. 
 * @param ps A pointer to an array of port_stats structures. Must not be NULL. 
 *        The array size must be ports_count*sizeof(struct port_stats). 
 * @return ERR_OK when everything is well or an error code otherwise. 
 **/
int ngadmin_getPortsStatistics (struct ngadmin *nga, struct port_stats *ps);


/**
 * Reset the ports statistics. 
 * This resets the ports packet statistics. 
 * @note You must be logged on a switch. 
 * @param nga A pointer to the ngadmin structure. 
 * @return ERR_OK when everything is well or an error code otherwise. 
 **/
int ngadmin_resetPortsStatistics (struct ngadmin *nga);


/**
 * Change the password of a switch. 
 * This changes the password of a switch. On success, automatically updates 
 * local password so you do not have to relog. 
 * @note You must be logged on a switch. 
 * @param nga A pointer to the ngadmin structure. 
 * @param pass The new password string to use. 
 * @return ERR_OK when everything is well or an error code otherwise. 
 **/
int ngadmin_changePassword (struct ngadmin *nga, const char* pass);


/**
 * Get the broadcast storm filtering state. 
 * Retrieves the broadcast storm filtering state. 
 * @note You must be logged on a switch. 
 * @param nga A pointer to the ngadmin structure. 
 * @param s A pointer to an integer which will receive 0 or 1. 
 * @return ERR_OK when everything is well or an error code otherwise. 
 **/
int ngadmin_getStormFilterState (struct ngadmin *nga, int *s);


/**
 * Set the broadcast storm filtering state. 
 * Changes the broadcast storm filtering state. 
 * @note You must be logged on a switch. 
 * @param nga A pointer to the ngadmin structure. 
 * @param s An integer with value 0 or 1. 
 * @return ERR_OK when everything is well or an error code otherwise. 
 **/
int ngadmin_setStormFilterState (struct ngadmin *nga, int s);


/**
 * Get the broadcast storm bitrates. 
 * Retrieves the broadcast storm filtering bitrates. 
 * @note You must be logged on a switch. 
 * @param nga A pointer to the ngadmin structure. 
 * @param ports A pointer to an array of integers. Must not be NULL. 
 *              The array size must be ports_count*sizeof(int). 
 * @return ERR_OK when everything is well or an error code otherwise. 
 **/
int ngadmin_getStormFilterValues (struct ngadmin *nga, int *ports);


/**
 * Set the broadcast storm bitrates. 
 * Changes the broadcast storm filtering values. 
 * @note You must be logged on a switch. 
 * @param nga A pointer to the ngadmin structure. 
 * @param ports A pointer to an array of integers. Must not be NULL. 
 *              The array size must be ports_count*sizeof(int). 
 * @return ERR_OK when everything is well or an error code otherwise. 
 **/
int ngadmin_setStormFilterValues (struct ngadmin *nga, const int *ports);


/**
 * Get the bitrates limits. 
 * Retrieves the bitrates limits of each port. 
 * @note You must be logged on a switch. 
 * @param nga A pointer to the ngadmin structure. 
 * @param ports A pointer to an array of integers. Must not be NULL. 
 *              The array size must be ports_count*sizeof(int). 
 * @return ERR_OK when everything is well or an error code otherwise. 
 **/
int ngadmin_getBitrateLimits (struct ngadmin *nga, int *ports);


/**
 * Set the bitrates limits. 
 * Changes the bitrates limits of each port. 
 * @note You must be logged on a switch. 
 * @param nga A pointer to the ngadmin structure. 
 * @param ports A pointer to an array of integers. Must not be NULL. 
 *              The array size must be ports_count*sizeof(int). 
 * @return ERR_OK when everything is well or an error code otherwise. 
 **/
int ngadmin_setBitrateLimits (struct ngadmin *nga, const int *ports);


/**
 * Get the QoS mode. 
 * Retrieves the QoS mode. 
 * @note You must be logged on a switch. 
 * @param nga A pointer to the ngadmin structure. 
 * @param s A pointer to an integer. Must not be NULL. 
 * @return ERR_OK when everything is well or an error code otherwise. 
 **/
int ngadmin_getQOSMode (struct ngadmin *nga, int *s);


/**
 * Set the QoS mode. 
 * Changes the QoS mode. 
 * @note You must be logged on a switch. 
 * @param nga A pointer to the ngadmin structure. 
 * @param s An integer with the new mode. 
 * @return ERR_OK when everything is well or an error code otherwise. 
 **/
int ngadmin_setQOSMode (struct ngadmin *nga, int s);


/**
 * Get the QoS values. 
 * Retrieves the QoS priority values for all the ports. 
 * @note You must be logged on a switch. 
 * @note The switch QoS mode should be port based to use this function. 
 * @param nga A pointer to the ngadmin structure. 
 * @param ports A pointer to an array of chars. Must not be NULL.
                The array size must be ports_count*sizeof(ports). 
 * @return ERR_OK when everything is well or an error code otherwise. 
 **/
int ngadmin_getQOSValues (struct ngadmin *nga, char *ports);


/**
 * Set the QoS values. 
 * Changes the QoS priority values for all the ports. 
 * @note You must be logged on a switch. 
 * @note The switch QoS mode should be port based to use this function. 
 * @param nga A pointer to the ngadmin structure. 
 * @param ports A pointer to an array of chars. Must not be NULL.
                The array size must be ports_count*sizeof(ports). 
 * @return ERR_OK when everything is well or an error code otherwise. 
 **/
int ngadmin_setQOSValues (struct ngadmin *nga, const char *ports);


/**
 * Restart the switch. 
 * Restarts the switch. 
 * @note You must be logged on a switch. 
 * @note If successfull, you should wait a few seconds while the switch 
         effectively restarts. 
 * @param nga A pointer to the ngadmin structure. 
 * @return ERR_OK when everything is well or an error code otherwise. 
 **/
int ngadmin_restart (struct ngadmin *nga);


/**
 * Restore the switch default parameters. 
 * Restores the switch default parameters and restarts it. 
 * @note You must be logged on a switch. 
 * @note If successfull, you will be delogged and the switch list will be 
         cleared. You should wait a few seconds while the switch effectively restarts. 
 * @param nga A pointer to the ngadmin structure. 
 * @return ERR_OK when everything is well or an error code otherwise. 
 **/
int ngadmin_defaults (struct ngadmin *nga);


/**
 * Get the port mirroring values. 
 * Retrieves the port mirrorring values. 
 * @note The switch QoS mode should be port based to use this function. 
 * @note You must be logged on a switch. 
 * @param nga A pointer to the ngadmin structure.
 * @param ports A pointer to an array of chars. Must not be NULL. \n
                The first element of the array is the output port (or 0 if port 
                mirroring is disabled), followed by 0 or 1 values for each port 
                if it is present or not. \n
                The array size must be (1+ports_count)*sizeof(char). 
 * @return ERR_OK when everything is well or an error code otherwise. 
 **/
int ngadmin_getMirror (struct ngadmin *nga, char *ports);


/**
 * Set the port mirroring values. 
 * Changes the port mirroring values. 
 * @note The switch QoS mode should be port based to use this function. 
 * @param nga A pointer to the ngadmin structure. 
 * @param ports A pointer to an array of chars. It as the same format as in 
                ngadmin_getMirror. \n
                If it is NULL, port mirroring is disabled. 
 * @return ERR_OK when everything is well or an error code otherwise. 
 **/
int ngadmin_setMirror (struct ngadmin *nga, const char *ports);


/**
 * Get the IGMP configuration. 
 * Retrieves the IGMP & multicast configuration. 
 * @note You must be logged on a switch. 
 * @param nga A pointer to the ngadmin structure. 
 * @param ic A pointer to an igmp_conf structure. Must not be NULL. 
 * @return ERR_OK when everything is well or an error code otherwise. 
 **/
int ngadmin_getIGMPConf (struct ngadmin *nga, struct igmp_conf *ic);


/**
 * Set the IGMP configuration. 
 * Changes the IGMP configuration. 
 * @note You must be logged on a switch. 
 * @param nga A pointer to the ngadmin structure. 
 * @param ic A pointer to an igmp_conf structure. Must not be NULL. 
 * @return ERR_OK when everything is well or an error code otherwise. 
 **/
int ngadmin_setIGMPConf (struct ngadmin *nga, const struct igmp_conf *ic);


/**
 * Perform a cable test. 
 * Performs a cable test on one ore more ports. 
 * @note Results are still raw values. 
 * @note This function takes a very long time. 
 * @note You must be logged on a switch. 
 * @param nga A pointer to the ngadmin structure. 
 * @param ct A pointer to an array of cabletest structures. Must not be NULL. 
 * @param nb The number of elements in the array. 
 * @return ERR_OK when everything is well or an error code otherwise. 
 **/
int ngadmin_cabletest (struct ngadmin *nga, struct cabletest *ct, int nb);


/**
 * Set the network configuration. 
 * Changes the network configuration. 
 * @note You must be logged on a switch. 
 * @param nga A pointer to the ngadmin structure. 
 * @param nc A pointer to a net_conf structure. Must not be NULL. \n
             Only non-zero fields of the structure are taken into account. 
 * @return ERR_OK when everything is well or an error code otherwise. 
 **/
int ngadmin_setNetConf (struct ngadmin *nga, const struct net_conf *nc);


/**
 * Get the VLAN type. 
 * Retrieves the VLAN type. 
 * @note You must be logged on a switch. 
 * @param nga A pointer to the ngadmin structure. 
 * @param t A pointer to an integer which will receive the VLAN type. Must not be NULL. 
 * @return ERR_OK when everything is well or an error code otherwise. 
 **/
int ngadmin_getVLANType (struct ngadmin *nga, int *t);


/**
 * Set the VLAN type. 
 * Changes the VLAN type. 
 * @note You must be logged on a switch. 
 * @param nga A pointer to the ngadmin structure. 
 * @param t An integer which contains the new VLAN type. 
 * @return ERR_OK when everything is well or an error code otherwise. 
 **/
int ngadmin_setVLANType (struct ngadmin *nga, int t);


/**
 * Get the ports VLANs in port mode. 
 * Retrieves the associated VLAN of ports in port mode. 
 * @note The switch should be in port mode. 
 * @note You must be logged on a switch. 
 * @param nga A pointer to the ngadmin structure. 
 * @param ports A pointer to an array of integers which will receive the 
                number of associated VLAN. Must not be NULL. 
 * @return ERR_OK when everything is well or an error code otherwise. 
 **/
int ngadmin_getVLANPortConf (struct ngadmin *nga, unsigned char *ports);


/**
 * Set the ports VLAN in port mode. 
 * Changes the associated VLAN of ports in port mode. 
 * @note The switch should be in port mode. 
 * @note You must be logged on a switch. 
 * @param nga A pointer to the ngadmin structure. 
 * @param ports A pointer to an array of integers which contain the 
                number of associated VLAN. Must not be NULL. 
 * @return ERR_OK when everything is well or an error code otherwise. 
 **/
int ngadmin_setVLANPortConf (struct ngadmin *nga, const unsigned char *ports);


/**
 * Get all the 802.1q VLAN configuration. 
 * Retrieves all the VLAN configuration in 802.1q mode. 
 * @note The switch should be in 802.1q mode. 
 * @note You must be logged on a switch. 
 * @param nga A pointer to the ngadmin structure. 
 * @param vlans A pointer to an array of unsigned shorts which will receive 
 *              VLAN ids. Must not be NULL. \n
 *              The array size must be sizeof(unsigned short)*(*nb). 
 * @param ports A pointer to an array of unsigned chars which will receive the 
                802.1q configuration for each VLAN. Must not be NULL. \n
                The array size must be sizeof(unsigned char)*ports_count*(*nb). 
 * @param nb A pointer to an integer which contains the maximum number of 
             elements allowed in the array. Must not be NULL. \n
             It will receive the actual number of VLAN written in the arrays. 
 * @return ERR_OK when everything is well or an error code otherwise. 
 **/
int ngadmin_getVLANDotAllConf (struct ngadmin *nga, unsigned short *vlans, unsigned char *ports, int *nb);


/**
 * Get the configuration of a VLAN in 802.1q mode. 
 * Retrieves the configuration of a particular VLAN in 802.1q mode. 
 * @note The switch should be in 802.1q mode. 
 * @note You must be logged on a switch. 
 * @param nga A pointer to the ngadmin structure. 
 * @param vlan The VLAN you want to get the configuration. 
 * @param ports A pointer to an array of integers which will receive the 
                configuration. Must not be NULL. 
 * @return ERR_OK when everything is well or an error code otherwise. 
 **/
int ngadmin_getVLANDotConf (struct ngadmin *nga, unsigned short vlan, unsigned char *ports);


/**
 * Set the configuration if a VLAN in 802.1q mode. 
 * Changes the configuration of a particular VLAN in 802.1q mode. 
 * @note The switch should be in 802.1q mode. 
 * @note You must be logged on a switch. 
 * @param nga A pointer to the ngadmin structure. 
 * @param vlan The VLAN you want to change the configuration. 
 * @param ports A pointer to an array of integers which contain the 
                configuration. Must not be NULL. 
 * @return ERR_OK when everything is well or an error code otherwise. 
 **/
int ngadmin_setVLANDotConf (struct ngadmin *nga, unsigned short vlan, const unsigned char *ports);


/**
 * Destroy a VLAN in 802.1q mode. 
 * Destroys a particular VLAN in 802.1q mode. 
 * @note The switch should be in 802.1q mode. 
 * @note You must be logged on a switch. 
 * @param nga A pointer to the ngadmin structure. 
 * @param vlan The VLAN you want to destroy. 
 * @return ERR_OK when everything is well or an error code otherwise. 
 **/
int ngadmin_VLANDestroy (struct ngadmin *nga, unsigned short vlan);


/**
 * Get the PVID values. 
 * Retrieves the PVID values of all the ports. 
 * @note You must be logged on a switch. 
 * @param nga A pointer to the ngadmin structure. 
 * @param ports A pointer to an array of unsigned shorts which will receive the 
 *              PVID values. Must not be NULL. \n
 *              The array size must be sizeof(unsigned short)*ports_count. 
 * @return ERR_OK when everything is well or an error code otherwise. 
 **/
int ngadmin_getAllPVID (struct ngadmin *nga, unsigned short *ports);


/**
 * Set the PVID of one port. 
 * Changes the PVID of one port. 
 * @note You must be logged on a switch. 
 * @param nga A pointer to the ngadmin structure. 
 * @param port The port you want to change PVID. 
 * @param vlan The new PVID value. 
 * @return ERR_OK when everything is well or an error code otherwise. 
 **/
int ngadmin_setPVID (struct ngadmin *nga, unsigned char port, unsigned short vlan);


/**
 * Get the loop detection state. 
 * Retrieves the loop detection state. 
 * @note You must be logged on a switch. 
 * @param nga A pointer to the ngadmin structure. 
 * @param s A pointer to an integer which will receive 0 or 1. 
 * @return ERR_OK when everything is well or an error code otherwise. 
 **/
int ngadmin_getLoopDetectionState (struct ngadmin *nga, int *s);


/**
 * Set the loop detection state. 
 * Changes the loop detection state. 
 * @note You must be logged on a switch. 
 * @param nga A pointer to the ngadmin structure. 
 * @param s An integer with value 0 or 1. 
 * @return ERR_OK when everything is well or an error code otherwise. 
 **/
int ngadmin_setLoopDetectionState (struct ngadmin *nga, int s);


#ifdef __cplusplus
}
#endif



#endif

