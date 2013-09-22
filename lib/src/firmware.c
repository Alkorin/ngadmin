
#include <ngadmin.h>

#include <attr.h>
#include <protocol.h>

#include "lib.h"
#include "network.h"


int ngadmin_upgradeFirmware (struct ngadmin *nga, const char *filename)
{
	if (nga == NULL || filename == NULL || *filename == 0)
		return ERR_INVARG;
	else if (nga->current == NULL)
		return ERR_NOTLOG;
	
	/*
	Firmware upgrade is not yet implemented. 
	This would require much more work and the use of a TFTP client. 
	Overall, it could be quite dangerous, as the switch may not check the binary 
	content sent to it. 
	*/
	
	return ERR_NOTIMPL;
}


