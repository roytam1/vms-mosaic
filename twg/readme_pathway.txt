	PathWay Build issues:


If you are building for PathWay you will need to get a new version
of TWG$TCP:[NETDIST.INCLUDE.SYS]TYPES.H by FTPing to TWG.COM (contacting
support) or make the change as indicated below. The rest of the build has been
modified to automagically build for PathWay. If you have any problems drop me a
line... (larry@eco.twg.com).

-Larry.

************
File TWG$COMMON:[NETDIST.INCLUDE.SYS]TYPES.H;6
   41
   42   #if !defined(CADDR_T) && !defined(__CADDR_T)
   43   #define CADDR_T
   44   #define __CADDR_T
   45   typedef char *  caddr_t;
   46   #endif
   47
   48   #ifndef __STAT
******
File TWG$COMMON:[NETDIST.INCLUDE.SYS]TYPES.H;5
   41   typedef char *  caddr_t;
   42   #ifndef __STAT
************
