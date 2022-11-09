/*              Initialization module                   HTInit.h
**
**      This module resisters all the plug&play software modules which
**      will be used in the program.  This is for a browser.
**
**      To override this, just copy it and link in your version
**      before you link with the library.
*/

extern void HTReInit();
extern void HTFileInit();

extern int HTLoadExtensionsConfigFile(char *fn);
extern int HTLoadTypesConfigFile(char *fn);

/*
 *  HTFormatInit: Set up default presentations and conversions
 *
 *  These are defined in HTInit.c.  If you don't call this routine, and
 *  you don't define any presentations, then this routine will automatically
 *  be called the first time a conversion is needed.  However, if you
 *  explicitly add some conversions (e.g. using HTLoadRules) then you may
 *  want also to explicitly call this to get the defaults as well.
 */
extern void HTFormatInit();

