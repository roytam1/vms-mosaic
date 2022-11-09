$!------------------------------------------------------------------------------
$! Copyright 1983, 1984, 1985, 1986, 1987, 1988, 1989, 1990, 1991, 1992, 1993
$! The Wollongong Group, Inc. All Rights Reserved.  This program 
$! contains confidential and trade secret information of The Wollongong 
$! Group, Inc.  Copyright notice is precautionary only and does not imply 
$! publication.
$!------------------------------------------------------------------------------
$!+ ---
$! DEF.COM
$!
$! Defines the location of each include directory provided with PathWay API
$!- ---
$!
$ define/nolog arpa		twg$tcp:[netdist.include.arpa]
$ define/nolog machine		twg$tcp:[netdist.include.machine]
$ define/nolog net		twg$tcp:[netdist.include.net]
$ define/nolog netdist		twg$tcp:[netdist.include]
$ define/nolog netimp		twg$tcp:[netdist.include.netimp]
$ define/nolog netinet		twg$tcp:[netdist.include.netinet]
$ define/nolog netns		twg$tcp:[netdist.include.netns]
$ define/nolog nfs		twg$tcp:[netdist.include.nfs]
$ define/nolog protocols	twg$tcp:[netdist.include.protocols]
$ define/nolog sys		twg$tcp:[netdist.include.sys],sys$library:
$ define/nolog vaxif		twg$tcp:[netdist.include.vaxif]
$ define/nolog vms		twg$tcp:[netdist.include.vms]
$ define/nolog rpc		twg$tcp:[netdist.include.rpc]
$ define/nolog nb		twg$tcp:[netdist.include.netbios]
$!
$! The next definition sets up a default search order which favors common
$! PathWay API include files over those provided with the DEC C compiler
$!
$
$ if F$GETSYI("Arch_Name") .nes. "Alpha" 
$ then
$ define vaxc$include		twg$tcp:[netdist.include], 	-
				twg$tcp:[netdist.include.sys],  -
				twg$tcp:[netdist.include.netinet], -
				sys$library
$ endif
$!
$ define decc$system_include	twg$tcp:[netdist.include], 	-
				twg$tcp:[netdist.include.sys],  -
				twg$tcp:[netdist.include.netinet], -
				sys$library
$!
$ exit 1
