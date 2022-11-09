/*
                                 WAIS PROTOCOL INTERFACE
                                             
   This module does not actually perform the WAIS protocol directly, but it
   does using one or more library of the freeWAIS distribution.  The ui.a
   library came with the old free WAIS from TMC, the client.a and wais.a
   libraries are needed from the freeWAIS from CNIDR.
   
   If you include this module in the library, you must also
   
      Register the HTWAIS protocol at initialisation (e.g. HTInit or HTSInit)
      by compiling it with -DDIRECT_WAIS
      
      Link with the libraries
      
   The wais source files are parsed by a separate and independent module,
   HTWSRC.  You can include HTWSRC without including direct wais using this
   module, and your WWW code will be able to read source files, and access
   WAIS indexes through a gateway.
   
   A WAIS-WWW gateway is just a normal W3 server with a libwww compiled with
   this module.
   
   Anyways, this interface won't change much:
   
 */
#ifndef HTWAIS_H
#define HTWAIS_H

#ifndef HTACCESS_H
#include "HTAccess.h"
#endif

extern HTProtocol HTWAIS;

#endif

/* Tim BL  */
