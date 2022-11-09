/* Copyright (C) 2006 - The VMS Mosaic Project */

#ifndef __CCISERVER_H__
#define __CCISERVER_H__

#include "cci.h"

extern int MCCIServerInitialize();
extern MCCIPort MCCICheckAndAcceptConnection();
extern int MCCISendResponseLine();
extern int MCCIIsThereInput();
extern int MCCIReadInputMessage();

typedef struct {
	MCCIPort client;
	int status;
	char *url;
} cciStat;			/* ejb 03/09/95 */

int MCCICloseAcceptPort(void);
int MCCIFormQueryToClient(MCCIPort, char *, char *, char *, char *);
int MCCIHandleInput(MCCIPort);
int MCCIReturnListenPortSocketDesc(void);
int MCCISendAnchorHistory(MCCIPort, char *);
int MCCISendBrowserViewOutput(MCCIPort, char *, char *, char *, int);
int MCCISendEventOutput(MCCIPort, CCI_events);
int MCCISendMouseAnchorOutput(MCCIPort, char *);
int MCCISendOutputFile(MCCIPort, char *, char *);
int MCCIGetSocketDescriptor(MCCIPort);

#endif
