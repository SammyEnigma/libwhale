#include "ocilib.h"
#include <stdio.h>

int main()
{
	OCI_Connection * cn = NULL;

	if ( !OCI_Initialize(NULL, NULL, OCI_ENV_DEFAULT) )
	{
		return -1;
	}

	cn = OCI_ConnectionCreate("192.168.7.111:1521/orcl1","comm", "comm", OCI_SESSION_DEFAULT);
	if ( cn != NULL )
	{
		printf("%s\n", OCI_GetVersionServer(cn));

		OCI_ConnectionFree(cn);
	}
	
	OCI_Cleanup();

	return 0;
}