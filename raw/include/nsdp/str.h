
#ifndef DEF_STR
#define DEF_STR


#include <nsdp/protocol.h>
#include <ngadmin.h> /* FIXME */


extern const char passwordKey[];

extern const char* const speed_str_tab[];
extern const char* const vlan_type_str_tab[];
extern const char* const vlan_code_str_tab[];
extern const char* const qos_type_str_tab[];
extern const char* const qos_prio_str_tab[];
extern const char* const bitrate_str_tab[];
extern const char* const code_str_tab[];
extern const char* const error_str_tab[];
extern const char* const cable_test_result_str_tab[];


static inline const char* safeStr (const char *s)
{
	return (s == NULL) ? "unknown" : s;
}


static inline const char* getValueStr (const char* const* tab, unsigned char mini, unsigned char maxi, unsigned char value)
{
	return (value >= mini && value <= maxi) ? tab[value] : NULL;
}


#define getSpeedStr(speed)	getValueStr(speed_str_tab, SPEED_DOWN, SPEED_1000_FD, speed)
#define getVlanTypeStr(type)	getValueStr(vlan_type_str_tab, VLAN_DISABLED, VLAN_DOT_ADV, type)
#define getVlanCodeStr(code)	getValueStr(vlan_code_str_tab, VLAN_NO, VLAN_TAGGED, code)
#define getQosTypeStr(type)	getValueStr(qos_type_str_tab, QOS_PORT, QOS_DOT, type)
#define getQosPrioStr(prio)	getValueStr(qos_prio_str_tab, PRIO_HIGH, PRIO_LOW, prio)
#define getBitrateStr(bitrate)	getValueStr(bitrate_str_tab, BITRATE_NOLIMIT, BITRATE_512M, bitrate)
#define getCodeStr(code)	getValueStr(code_str_tab, CODE_READ_REQ, CODE_WRITE_REP, code)
#define getErrorStr(error)	getValueStr(error_str_tab, ERROR_NONE, ERROR_DENIED, error)
#define getCableTestResultStr(error)	getValueStr(cable_test_result_str_tab, ERROR_NONE, ERROR_DENIED, error)


int parseValueStr (const char* const* tab, unsigned char mini, unsigned char maxi, const char *str);


#define parseSpeedStr(str)	parseValueStr(speed_str_tab, SPEED_DOWN, SPEED_1000_FD, str)
#define parseVlanTypeStr(str)	parseValueStr(vlan_type_str_tab, VLAN_DISABLED, VLAN_DOT_ADV, str)
#define parseVlanCodeStr(str)	parseValueStr(vlan_code_str_tab, VLAN_NO, VLAN_TAGGED, str)
#define parseQosTypeStr(str)	parseValueStr(qos_type_str_tab, QOS_PORT, QOS_DOT, str)
#define parseQosPrioStr(str)	parseValueStr(qos_prio_str_tab, PRIO_HIGH, PRIO_LOW, str)
#define parseBitrateStr(str)	parseValueStr(bitrate_str_tab, BITRATE_NOLIMIT, BITRATE_512M, str)
#define parseCodeStr(str)	parseValueStr(code_str_tab, CODE_READ_REQ, CODE_WRITE_REP, str)
#define parseErrorStr(str)	parseValueStr(error_str_tab, ERROR_NONE, ERROR_DENIED, str)


bool isStringPrintable (const char *str, unsigned int len);


void passwordEndecode (char *buf, unsigned int len);


int trim (char *txt, int start);


static inline int min (int a, int b)
{
	return a < b ? a : b;
}


#endif

