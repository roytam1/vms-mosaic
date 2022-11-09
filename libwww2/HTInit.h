/*  */

/*              Initialisation module                   HTInit.h
**
**      This module resisters all the plug&play software modules which
**      will be used in the program.  This is for a browser.
**
**      To override this, just copy it and link in your version
**      befoe you link with the library.
*/

#include "HTUtils.h"

extern void HTReInit NOPARAMS;
extern void HTFileInit NOPARAMS;

extern int HTLoadExtensionsConfigFile (char *fn);
extern int HTLoadTypesConfigFile (char *fn);

/*
HTFormatInit: Set up default presentations and conversions

   These are defined in HTInit.c or HTSInit.c if these have been
replaced. If you don't call this routine, and you don't define any
presentations, then this routine will automatically be called the
first time a conversion is needed. However, if you explicitly add some
conversions (eg using HTLoadRules) then you may want also to
explicitly call this to get the defaults as well.
*/
extern void HTFormatInit NOPARAMS;

