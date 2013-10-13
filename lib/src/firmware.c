
#include <ngadmin.h>

#include <nsdp/attr.h>
#include <nsdp/protocol.h>

#include "lib.h"
#include "network.h"


int ngadmin_upgradeFirmware (struct ngadmin *nga, const char *filename)
{
	if (nga == NULL || filename == NULL || *filename == 0)
		return ERR_INVARG;
	else if (nga->current == NULL)
		return ERR_NOTLOG;
	
	/* firmware upgrade is not yet implemented
	 * this would require much more work and the use of a TFTP client
	 * overall, it could be quite dangerous, as the switch may not check
	 * the binary content sent to it
	 */
	
	return ERR_NOTIMPL;
}


