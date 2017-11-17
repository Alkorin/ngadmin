
#include <string.h>
#include <ctype.h>

#include <nsdp/str.h>


const char passwordKey[] = "NtgrSmartSwitchRock";


const char* const speed_str_tab[] = {
	[SPEED_DOWN] = "down",
	[SPEED_10_HD] = "10M half-duplex",
	[SPEED_10_FD] = "10M full-duplex",
	[SPEED_100_HD] = "100M half-duplex",
	[SPEED_100_FD] = "100M full-duplex",
	[SPEED_1000_FD] = "1000M full-duplex",
	NULL
};


const char* const vlan_type_str_tab[] = {
	[VLAN_DISABLED] = "disabled",
	[VLAN_PORT_BASIC] = "basic port based",
	[VLAN_PORT_ADV] = "advanced port based",
	[VLAN_DOT_BASIC] = "basic 802.1Q",
	[VLAN_DOT_ADV] = "advanced 802.1Q",
	NULL
};


const char* const vlan_code_str_tab[] = {
	[VLAN_NO] = "no",
	[VLAN_UNTAGGED] = "untagged",
	[VLAN_TAGGED] = "tagged",
	NULL
};


const char* const qos_type_str_tab[] = {
	[QOS_PORT] = "port",
	[QOS_DOT] = "802.1p",
	NULL
};


const char* const qos_prio_str_tab[] = {
	[PRIO_HIGH] = "high",
	[PRIO_MED] = "medium",
	[PRIO_NORM] = "normal",
	[PRIO_LOW] = "low",
	NULL
};


const char* const bitrate_str_tab[] = {
	[BITRATE_NOLIMIT] = "nl",
	[BITRATE_512K] = "512K",
	[BITRATE_1M] = "1M",
	[BITRATE_2M] = "2M",
	[BITRATE_4M] = "4M",
	[BITRATE_8M] = "8M",
	[BITRATE_16M] = "16M",
	[BITRATE_32M] = "32M",
	[BITRATE_64M] = "64M",
	[BITRATE_128M] = "128M",
	[BITRATE_256M] = "256M",
	[BITRATE_512M] = "512M",
	NULL
};


const char* const code_str_tab[] = {
	[CODE_READ_REQ] = "read request",
	[CODE_READ_REP] = "read reply",
	[CODE_WRITE_REQ] = "write request",
	[CODE_WRITE_REP] = "write reply",
	NULL
};


const char* const error_str_tab[] = {
	[ERROR_NONE] = "none",
	[ERROR_READONLY] = "read only",
	[ERROR_INVALID_VALUE] = "invalid value",
	[ERROR_DENIED] = "access denied",
	NULL
};


const char* const cable_test_result_str_tab[] = {
	[CABLETEST_OK] = "OK",
	[CABLETEST_NO_CABLE] = "No Cable",
	[CABLETEST_OPEN_CABLE] = "Open Cable",
	[CABLETEST_SHORT_CIRCUIT] = "Short Circuit",
	[CABLETEST_FIBER_CABLE] = "Fiber Cable",
	[CABLETEST_SHORTED_CABLE] = "Shorted cable",
	[CABLETEST_UNKNOWN] = "Unknown",
	[CABLETEST_CROSSTALK] = "Crosstalk",
	NULL
};

int parseValueStr (const char* const* tab, unsigned char mini, unsigned char maxi, const char *str)
{
	unsigned char i;
	
	for (i = mini; i <= maxi; i++) {
		if (tab[i] != NULL && strcasecmp(str, tab[i]) == 0)
			return (int)i;
	}
	
	return -1;
}


bool isStringPrintable (const char *str, unsigned int len)
{
	const char *p;
	
	for (p = str; len > 0; len--) {
		if (!isprint(*p++))
			return false;
	}
	
	return true;
}


void passwordEndecode (char *buf, unsigned int len)
{
	const char *k = passwordKey;
	unsigned int i;
	
	if (buf == NULL || len <= 0)
		return;
	
	for (i = 0; i < len; i++) {
		if (*k == '\0')
			k = passwordKey;
		buf[i] ^= *k++;
	}
}


int trim (char *txt, int start)
{
	char *p;
	
	if (txt == NULL)
		return 0;
	
	p = txt + start;
	for (p--; p >= txt && (*p == ' ' || *p == '\n'); *p-- = '\0');
	
	return p - txt + 1;
}


