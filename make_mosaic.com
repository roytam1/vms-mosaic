$ Ident = "4.2"
$!
$! Create Mosaic version 4.2 on VMS.
$!
$! Copyright (C) 2000, 2003, 2004, 2005, 2006, 2007 - The VMS Mosaic Project
$!
$! This command procedure compiles and links Mosaic with MMS or MMK if
$! either is available, otherwise it just compiles and links in one go.
$!
$! The debugger is enabled if P1 is set to DEBUG.  Trace dumps are enabled
$! if P1 is set to TRACE.  P2 can be used to specify the TCP/IP package
$! (CMU, MULTINET, PATHWAY, SOCKETSHR, TCPWARE or UCX).  MMS parameters
$! can be added in P3 (e.g. /IGNORE=WARNING to make MMS ignore compilation
$! warnings).  Additional arguments (e.g. NOVMSLOGO, VAXC, NOMMS, NOUCX)
$! can be specified in P4 thru P8.
$!
$! Björn S. Nilsson, 25-Nov-1993
$! Motif 1.2 sensitivity added 2-June-1994
$! Mosaic 2.4 with WAIS Dec. 1994
$! Mosaic 2.6, Oct. 1995, George Cook
$! Combined build procedure, Nov. 1995, George Cook
$! VAX GNU C support, Sept. 1996, George Cook
$! OpenSSL support, Aug. 1999, George Cook
$! HP SSL support, Nov. 2003, George Cook
$! Motif 1.3 support, Dec. 2003, George Cook
$! IA64 and Motif 1.4 support, Jan. 2004, George Cook
$! LIBTIFF support, Jul. 2006, George Cook
$! LIBOPENJPEG support, May 2007, George Cook
$!
$!---------------------------------------------------------------------------
$!
$! Fallbacks for home page, default print command, mail prefix, etc.
$! ***
$! *** Local configuration info should be placed in the file local.config
$! *** The following values are only used if they are not in local.config
$! ***
$!----------------------------------------
$ HOME_PAGE     ="http://wvnvms.wvnet.edu/vmswww/vms_mosaic.html"
$ PRINT_COMMAND ="Print/Name=\""""""""From Mosaic\""""""""/Notify/Identify/Delete"
$ MAIL_PREFIX   ="in%"
$! MAIL_PREFIX   ="smtp%"	! Native UCX
$ EDIT_COMMAND  ="Edit"
$ NEWS_HOST     =""		! The name of the default news host
$ POSTSCRIPT_VIEWER ="View/interface=decwindows/format=ps %s"
$! POSTSCRIPT_VIEWER ="gv %s"	! Ghostview
$!----------------------------------------
$!
$ If F$Search("local.config") .NES. ""
$  Then
$   Open/read/err=End_conf loc_conf local.config
$Read_conf:
$   Read/end=End_conf/err=End_conf loc_conf Line
$   Line = F$Edit(Line, "TRIM,COMPRESS,UNCOMMENT")
$   If Line .NES. ""
$    Then
$     item  = F$Element(0, "#", Line)
$     value = F$Element(1, "#", Line)
$     'item' = 'value'
$    Endif
$   Goto Read_conf
$End_conf:
$   If F$Trnlnm("loc_conf") .NES. "" Then Close loc_conf
$  Endif
$!
$!---------------------------------------------------------------------------
$!
$ Verify = F$Verify(0)
$ On Error Then Goto The_End
$ On Control_Y Then Goto EndY
$ Set Symbol/Scope = (NoGlobal, NoLocal)
$! Get rid of <>
$ Set Default []
$ pwd = F$Environment("Default")
$ Set Default 'F$Element(0, "]", F$Environment("PROCEDURE"))']
$!
$ If (P3 .EQS. "?") .OR. (P3 .EQS. "HELP")
$  Then
$   Write sys$output "Additional MMS parameters (e.g. /IGNORE=WARNING) in P3."
$   Goto The_End
$  Endif
$ If (P2 .EQS. "?") .OR. (P2 .EQS. "HELP")
$  Then
$   Write sys$output -
	"Specify UCX, CMU, MULTINET, PATHWAY, SOCKETSHR or TCPWARE."
$   Goto The_End
$  Endif
$ If (P1 .EQS. "?") .OR. (P1 .EQS. "HELP")
$  Then
$   Write sys$output "Specify DEBUG in P1 to build with debugging."
$   Write sys$output "Specify TRACE in P1 to enable trace dumps."
$   Write sys$output -
 "The TCP/IP package (UCX, CMU, MULTINET, PATHWAY, SOCKETSHR or TCPWARE) in P2."
$   Write sys$output "Additional MMS parameters (e.g. /IGNORE=WARNING) in P3."
$   Write sys$output -
       "Additional arguments (e.g. VAXC, GNUC, NOMMS, NOVMSLOGO) in P4 thru P8."
$   Goto The_End
$  Endif
$ If (P1 .NES. "TRACE") .AND. (P1 .NES. "DEBUG") .AND. (P1 .NES. "")
$  Then
$   Write sys$output "Invalid P1 parameter.  Specify either TRACE or DEBUG."
$   Goto The_End
$  Endif
$ If (P2 .NES. "") .AND. (P2 .NES. "UCX") .AND. (P2 .NES. "CMU") .AND. -
     (P2 .NES. "MULTINET") .AND. (P2 .NES. "PATHWAY") .AND. -
     (P2 .NES. "SOCKETSHR") .AND. (P2 .NES. "TCPWARE")
$  Then
$   Write sys$output "Invalid TCP/IP package specified in P2."
$   Write sys$output -
	"Specify UCX, CMU, MULTINET, PATHWAY, SOCKETSHR or TCPWARE."
$   Goto The_End
$  Endif
$!
$ Args = ""
$ If P4 .NES. "" then Args = P4 + ","
$ If P5 .NES. "" then Args = Args + P5 + ","
$ If P6 .NES. "" then Args = Args + P6 + ","
$ If P7 .NES. "" then Args = Args + P7 + ","
$ If P8 .NES. "" then Args = Args + P8 + ","
$ Args = F$Edit(Args, "COLLAPSE,UPCASE")
$!
$ Macro = "(IDENT=''Ident',"
$ If F$Locate("NOWAIS", Args) .NE. F$Length(Args) -
	Then Macro = Macro + "NOWAIS=1,"
$ If F$Locate("NOLINK", Args) .NE. F$Length(Args) -
	Then Macro = Macro + "NOLINK=1,"
$ If F$Locate("NOTIFF", Args) .NE. F$Length(Args)
$  Then
$   Macro = Macro + "NOTIFF=1,"
$   Tiff = 0
$  Else
$   Tiff = 1
$  Endif
$ If F$Locate("CCI", Args) .NE. F$Length(Args)
$  Then CCI = 1
$  Else CCI = 0
$  Endif
$!                              
$ VMS_Version = F$Extract(1, 3, F$GetSYI("Version"))
$!
$ On Warning Then Platform = "VAX"
$ If (VMS_Version .EQS. "5.4") .OR. (VMS_Version .EQS. "5.5")
$  Then Platform = "VAX"
$  Else Platform = F$GetSYI("Arch_Name")
$  Endif
$ On Error Then Goto The_End
$ If Platform .EQS. "Alpha"
$  Then
$   Macro = Macro + "ALPHA=1,"
$   Work = "A"
$  Else If Platform .EQS. "IA64"
$   Then Work = "I"
$   Else Work = "V"
$   Endif
$  Endif
$!
$ If F$Locate("NOTRACE", Args) .NE. F$Length(Args)
$  Then
$   Disable_Trace = 1
$   Work = "N" + Work
$  Else
$   Disable_Trace = 0
$  Endif
$!
$ If ((Platform .EQS. "Alpha") .AND. (VMS_Version .GES. "7.2")) .OR. -
      (Platform .EQS. "IA64")
$  Then
$   Write sys$output "Setting DCL parse style to TRADITIONAL."
$   Set Process/Parse=Traditional
$  Endif
$!
$ CC = "CC"
$ Compiler = ""
$ DECCP = 0
$ VAXCP = 0
$ GNUCP = 0
$ If F$Locate("DECC", Args) .NE. F$Length(Args) Then DECCP = 1
$ If F$Locate("VAXC", Args) .NE. F$Length(Args) Then VAXCP = 1
$ If F$Locate("GNUC", Args) .NE. F$Length(Args) Then GNUCP = 1
$ If (DECCP .AND. VAXCP) .OR. (DECCP .AND. GNUCP) .OR. (VAXCP .AND. GNUCP)
$  Then
$   Write sys$output "Multiple C compilers specified.  Please specify only one."
$   Write sys$output "Aborting."
$   Goto The_End
$  Endif
$!
$ If (F$Search("SYS$System:DECC$Compiler.Exe") .EQS. "") .AND. DECCP
$  Then
$   Write sys$output "DEC C does not appear to be installed on this system."
$   Write sys$output "Aborting."
$   Goto The_End
$  Endif
$ If (F$Search("SYS$System:VAXC.Exe") .EQS. "") .AND. VAXCP
$  Then
$   Write sys$output "VAX C does not appear to be installed on this system."
$   Write sys$output "Aborting."
$   Goto The_End
$  Endif
$ If (F$Trnlnm("GNU_CC_VERSION") .EQS. "") .AND. GNUCP
$  Then
$   Write sys$output "GNU C does not appear to be installed on this system."
$   Write sys$output "Aborting."
$   Goto The_End
$  Endif
$!
$ If F$Search("SYS$System:DECC$Compiler.Exe") .NES. ""
$  Then
$   If (Platform .EQS. "VAX") .AND. -
	(F$Trnlnm("DECC$LIBRARY_INCLUDE") .EQS. "") .AND. -
	(F$Trnlnm("DECC$CC_DEFAULT") .EQS. "") .AND. (.NOT. VAXCP) .AND. -
        (.NOT. GNUCP)
$    Then
$     Write sys$output "DEC C does not appear to be properly installed on"
$     Write sys$output "this system.  Check if SYS$STARTUP:CCXX$STARTUP.COM"
$     If DECCP
$      Then
$       Write sys$output "was executed.  Aborting."
$       Goto The_End
$      Else
$       Write sys$output "was executed.  Will attempt to use VAX or GNU C."
$      Endif
$    Else
$     Compiler = "DECC"
$    Endif
$  Endif
$!
$ If (Compiler .EQS. "") .AND. (F$Search("SYS$System:VAXC.Exe") .NES. "")
$  Then
$   Compiler = "VAXC"
$  Endif
$!
$ If (Compiler .EQS. "") .AND. (F$Trnlnm("GNU_CC_VERSION") .NES. "")
$  Then                                            
$   Compiler = "GNUC"
$  Endif
$!
$ If Compiler .EQS. ""
$  Then
$   Write sys$output "C does not appear to be installed on this system."
$   Write sys$output "DEC C, VAX C or GNU C is required.  Aborting."
$   Goto The_End
$  Endif
$!
$ If Compiler .EQS. "DECC"
$  Then
$   If VAXCP
$    Then
$     Compiler = "VAXC"
$     Macro = Macro + "DECCVAXC=1,VAXC=1,"
$     COpt = "/VAXC/PRECISION=SINGLE"
$     COpt_NoVAXC = COpt
$    Endif
$  Else
$   If ((Compiler .EQS. "VAXC") .OR. VAXCP) .AND. (.NOT. GNUCP)
$    Then
$     Compiler = "VAXC"
$     Macro = Macro + "VAXC=2,"
$     COpt = ""
$     COpt_NoVAXC = "/PRECISION=SINGLE"
$    Endif
$  Endif
$!
$ If (Compiler .EQS. "GNUC") .OR. GNUCP
$  Then
$   Compiler = "GNUC"
$   If Platform .EQS. "Alpha"
$    Then CC = "GCC/Names=Upper/Float=ieee/cc1=""-fno-exceptions -g0"""
$    Else CC = "GCC/Names=Upper"
$    Endif
$   Macro = Macro + "GNUC=1,"
$   COpt = ""
$   COpt_NoVAXC = ""
$  Endif
$!
$ If Compiler .EQS. "DECC"
$  Then
$   Macro = Macro + "DECC=1,"
$   COpt = "/DECC/Standard=VAXC/PRECISION=SINGLE"
$   If F$Locate("UNUSED", Args) .NE. F$Length(Args)
$    Then COpt_NoVAXC = -
	"/DECC/WARNING=(ENABLE=UNUSED,DISABLE=(UNUSEDINCL,NESTINCL))"
$    Else If F$Locate("QUESTCODE", Args) .NE. F$Length(Args)
$     Then COpt_NoVAXC = -
	"/DECC/WARNING=(ENABLE=QUESTCODE,DISABLE=(UNKNOWNMACRO,INTCONSTSIGN))"
$     Else If F$Locate("CHECK", Args) .NE. F$Length(Args)
$      Then COpt_NoVAXC = "/DECC/CHECK"
$      Else COpt_NoVAXC = "/DECC"
$      Endif
$     Endif
$    Endif
$  Endif
$!
$ If (Compiler .EQS. "VAXC") .AND. Tiff
$  Then
$   Write sys$output "DEC C or GNU C is required to build with TIFF support."
$   Write sys$output "Continuing build without including TIFF image support."
$   Macro = Macro + "NOTIFF=1,"
$   Tiff = 0
$  Endif
$!
$! OpenSSL
$! 
$ SSL = 1
$ SSL_Log = 0
$ If F$Locate("NOSSL", Args) .NE. F$Length(Args)
$  Then
$   Write sys$output "Building without OpenSSL support."
$   SSL = 0 
$  Else If F$Trnlnm("OPENSSL") .EQS. ""
$   Then
$    SSL = 3
$   Endif
$  Endif
$!
$! Check for HP OpenSSL
$ If (SSL .EQ. 1) .AND. (F$Trnlnm("SSL$ROOT") .NES. "") .AND. -
     (F$Search("SYS$SHARE:SSL$LIBSSL_SHR32.EXE") .NES. "") .AND. -
     (F$Locate("NOHPSSL", Args) .EQ. F$Length(Args))
$  Then
$   SSL = 2
$   If F$Trnlnm("OPENSSL") .EQS. "SSLINCLUDE:"
$    Then
$     Define OPENSSL SSL$INCLUDE:
$     SSL_Log = 1
$    Endif
$  Endif
$!
$! Check for non-HP OpenSSL
$ If (SSL .EQ. 1) .AND. (F$Trnlnm("SSLLIB") .EQS. "")
$  Then
$   SSL = 3
$  Endif
$!
$ If SSL .EQ. 3
$  Then
$   Write sys$output "OpenSSL appears to not be installed on this system."
$   Write sys$output "Continuing build without including OpenSSL."
$   SSL = 0
$  Endif
$ If (SSL .GT. 0) .AND. (Compiler .NES. "DECC")
$  Then
$   Write sys$output "DEC C is required to build with OpenSSL."
$   Write sys$output "Continuing build without including OpenSSL."
$   SSL = 0
$  Endif
$ If (SSL .EQ. 0) Then Macro = Macro + "NOSSL=1,"
$ If (SSL .EQ. 2) Then Macro = Macro + "HPSSL=1,"
$!
$!
$ If F$Locate("CLEAN", P3) .NE. F$Length(P3)
$  Then Extras = " " + P3
$  Else Extras = P3
$  Endif
$ If F$Locate("CLEAN", Args) .NE. F$Length(Args) Then Extras = Extras + " CLEAN"
$ If F$GetJPI("","CLINAME") .EQS. "DCL_RECALL" Then Extras = Extras + "/CLI=DCL"
$!
$ If (F$Locate("DEBUG", P1) .EQ. F$Length(P1)) .OR. -
     (F$Locate("NODEBUG", P1) .NE. F$Length(P1))
$  Then
$   COpt = COpt + "/Optim"
$   COpt_NoVAXC = COpt_NoVAXC + "/Optim"
$   If F$Locate("TRACE", P1) .EQ. F$Length(P1)
$    Then
$     LOpt = "/NoTrace"
$     VMS_Debug = 0
$    Else
$     LOpt = "/Trace"
$     VMS_Debug = 1
$     Macro = Macro + "TRACE=1,"
$    Endif
$  Else
$   COpt = COpt + "/NoOptim/Debug"
$   COpt_NoVAXC = COpt_NoVAXC + "/NoOptim/Debug"
$   LOpt = "/Debug"
$   VMS_Debug = 1
$   Macro = Macro + "DEBUG=1,"
$   Work = "D" + Work
$  EndIf
$ If Compiler .EQS. "VAXC" Then Work = Work + "C"
$ If Compiler .EQS. "GNUC" Then Work = Work + "G"
$!
$ If P2 .EQS. ""
$ Then
$  IP = "UCX"
$  If F$Trnlnm("MULTINET") .NES. ""
$   Then
$    IP = "MULTINET_UCX"
$    Goto ip_done
$   Endif
$  If F$Trnlnm("TWG$TCP") .NES. ""
$   Then
$    IP = "PATHWAY_UCX"
$    Goto ip_done
$   EndIf
$  If F$Trnlnm("INET$DEVICE") .NES. ""
$   Then
$    IP = "CMU"
$    Goto ip_done
$   Endif
$  If F$Trnlnm("SOCKETSHR") .NES. ""
$   Then
$    IP = "SOCKETSHR"
$    Goto ip_done
$   EndIf
$  If F$Trnlnm("TCPWARE") .NES. ""
$   Then
$    IP = "TCPWARE"
$    Goto ip_done
$   EndIf
$ Else
$  IP = ""
$  IPX = F$Edit(P2, "UPCASE")
$  If F$Locate("UCX", IPX) .NE. F$Length(IPX) Then IP ="UCX"
$  If F$Locate("CMU", IPX) .NE. F$Length(IPX) Then IP ="CMU"
$  If F$Locate("MULTINET", IPX) .NE. F$Length(IPX) Then IP ="MULTINET_UCX"
$  If F$Locate("PATHWAY", IPX) .NE. F$Length(IPX) Then IP ="PATHWAY_UCX"
$  If F$Locate("SOCKETSHR", IPX) .NE. F$Length(IPX) Then IP ="SOCKETSHR"
$  If F$Locate("TCPWARE", IPX) .NE. F$Length(IPX) Then IP ="TCPWARE"
$  If IP .EQS. ""
$   Then
$    Write sys$output "Invalid TCP/IP package specified in P2."
$    Write sys$output -
	"Must be UCX, CMU, MULTINET, PATHWAY, SOCKETSHR or TCPWARE."
$    Goto The_End
$   Endif
$ Endif
$Ip_done:
$ If F$Locate("NOUCX", Args) .NE. F$Length(Args) Then IP = IP - "_UCX"
$ If IP .EQS. "MULTINET_UCX"
$  Then
$   Set Noon
$   Set Symbol/Scope = (Global, NoLocal)
$   Define/User sys$output tmpip.lis
$   Define/User sys$error tmpip.lis
$   Multinet Show/Version
$   Set Symbol/Scope = (NoGlobal, NoLocal)
$   Define/User sys$output nl:
$   Define/User sys$error nl:
$   Search tmpip.lis 3.1,3.2,3.3
$   Tmp_Err = $Severity
$   Delete tmpip.lis;*
$   Set On
$   If Tmp_Err .EQ. 1
$    Then
$     Write sys$output "The MultiNet on this system is a version prior to V3.4."
$     Write sys$output "Versions prior to V3.4 do not correctly support UCX"
$     Write sys$output "compatibility mode.  Proceeding with non-UCX build."
$     Write sys$output " "
$     IP = IP - "_UCX"
$    Endif
$  Endif
$ If IP .EQS. "MULTINET"
$  Then
$   Set Noon
$   Set Symbol/Scope = (Global, NoLocal)
$   Define/User sys$output tmpip.lis
$   Define/User sys$error tmpip.lis
$   Multinet Show/Version
$   Set Symbol/Scope = (NoGlobal, NoLocal)
$   Define/User sys$output nl:
$   Define/User sys$error nl:
$   Search tmpip.lis 4.2,4.3,4.4,4.5,5.0,5.1,5.2,5.3,5.4,5.5,6.0,6.1
$   Tmp_Err = $Severity
$   Delete tmpip.lis;*
$   Set On
$   If Tmp_Err .EQ. 1
$    Then
$     Write sys$output "The MultiNet on this system is a version after V4.1."
$     Write sys$output "Direct MultiNet builds are not supported on versions"
$     Write sys$output "after V4.1.  Proceeding with UCX compatible build."
$     Write sys$output " "
$     IP = "MULTINET_UCX"
$    Endif
$  Endif
$ Macro = Macro + IP + "=1,"
$ If (IP .EQS. "UCX") .OR. (IP .EQS. "MULTINET_UCX") .OR. (IP .EQS. "CMU") -
     .OR. (IP .EQS. "PATHWAY_UCX") .OR. (IP .EQS. "TCPWARE")
$  Then Work = Work + "U"
$  Else
$   If Compiler .EQS. "GNUC"
$    Then
$     Write sys$output "SOCKETSHR and non-UCX compatible builds are not"
$     Write sys$output "supported using the GNU C compiler.  Aborting."
$     Goto The_End
$    Endif
$  Endif
$ If IP .EQS. "MULTINET" Then Work = Work + "M"
$ If IP .EQS. "PATHWAY"
$  Then
$   Work = Work + "P"
$   @[.twg]def.com      ! define the location of PathWay include files
$  Endif
$ If IP .EQS. "SOCKETSHR" Then Work = Work + "S"
$!
$! Test build
$ If F$Locate("TEST", Args) .NE. F$Length(Args) Then Work = "TEST_" + Work
$!
$ Macro = Macro + "WORK=" + Work + ","
$!
$ If Compiler .EQS. "DECC"
$  Then
$   If IP .EQS. "MULTINET"
$    Then
$     COpt = COpt + "/PREFIX=ANSI"
$     COpt_NoVAXC = COpt_NoVAXC + "/PREFIX=ANSI"
$    Else
$     If VMS_Version .GES. "7.0"
$      Then 
$	COpt = COpt + "/PREFIX=(ALL,EXCEPT=(GETPWUID,IOCTL))"
$	COpt_NoVAXC = COpt_NoVAXC + "/PREFIX=(ALL,EXCEPT=(GETPWUID,IOCTL))"
$      Else
$	COpt = COpt + "/PREFIX=ALL"
$	COpt_NoVAXC = COpt_NoVAXC + "/PREFIX=ALL"
$      Endif
$    Endif
$  Endif
$!
$ If IP .EQS. "SOCKETSHR"
$  Then
$   Define Socketshr_Files -
     "''f$parse(f$trnlnm("SOCKETSHR"),,,"DEVICE")'''f$parse(f$trnlnm("SOCKETSHR"),,,"DIRECTORY")"
$   Write sys$output "Will look for the SOCKETSHR include files in the same"
$   Write sys$output "location as the SOCKETSHR shareable library."
$   Write sys$output " "
$  Endif
$!
$ Motif12 = ""
$ Motif123 = ""
$ Motif124 = ""
$ Motif125 = ""
$ Motif126 = ""
$ Motif13 = ""
$ Motif130 = ""
$ Motif131 = ""
$ Motif14 = ""
$ Motif141 = ""
$ Motif15 = ""
$ Motif16 = ""
$ If F$Search("SYS$Update:DECW$Get_Image_Version.COM") .NES. ""
$  Then
$   Set Symbol/Scope = (Global, Local)
$   @SYS$Update:DECW$Get_Image_Version SYS$Share:DECW$XLibshr.EXE DECW$Version
$   If (F$Length(DECW$Version) .NE. 0) .AND. -
       (F$Locate("V1.1", DECW$Version) .NE. F$Length(DECW$Version))
$    Then Goto Motif11
$    Endif
$   If (F$Length(DECW$Version) .NE. 0) .AND. -
       ((F$Locate("V1.2", DECW$Version) .NE. F$Length(DECW$Version)) .OR. -
        (F$Locate("T1.2", DECW$Version) .NE. F$Length(DECW$Version)))
$    Then
$     Macro = Macro + "MOTIF1_2=1,"
$     Motif12 = "1"
$     If (F$Locate("V1.2-395", DECW$Version) .NE. F$Length(DECW$Version)) .OR. -
         (F$Locate("V1.2-394", DECW$Version) .NE. F$Length(DECW$Version)) .OR. -
         (F$Locate("V1.23", DECW$Version) .NE. F$Length(DECW$Version))
$      Then
$       Write sys$output "Your system has Motif 1.2-3 installed.  Due to a bug"
$       Write sys$output "in Motif 1.2-3, the hotlist dialog windows may act oddly."
$       Write sys$output " "
$       Motif123 = "1"
$      Else
$!	39602 is MOTF07, 39604 is DWMW01, 39801 is MOTF08, 39711 is DWMW02
$       If (F$Locate("V1.2-3960", DECW$Version) .NE. F$Length(DECW$Version)) -
	  .OR. (F$Locate("V1.2-3971", DECW$Version) .NE. F$Length(DECW$Version)) -
	  .OR. (F$Locate("V1.2-3980", DECW$Version) .NE. F$Length(DECW$Version))
$        Then Motif123 = "7"
$        Endif
$      Endif
$     If F$Locate("1.2-4", DECW$Version) .NE. F$Length(DECW$Version)
$      Then Motif124 = "1"
$      Endif
$     If F$Locate("1.2-5", DECW$Version) .NE. F$Length(DECW$Version)
$      Then Motif125 = "1"
$      Endif
$     If F$Locate("1.2-6", DECW$Version) .NE. F$Length(DECW$Version)
$      Then Motif126 = "1"
$      Endif
$     If (Motif123 .EQS. "") .AND. (Motif124 .EQS. "") .AND. -
	(Motif125 .EQS. "") .AND. (Motif126 .EQS. "")
$      Then
$       Write sys$output "Your system appears to have Motif 1.2 installed.  Due"
$       Write sys$output "to various problems with the Motif 1.2 header files,"
$       Write sys$output "the build of LIBXMX may fail with warning messages."
$       Write sys$output "If no other errors are encountered, the build can"
$       Write sys$output "safely be continued.  It is recommended, however,"
$       Write sys$output "that you upgrade your system to Motif 1.2-3 and"
$       If Platform .EQS. "Alpha"
$        Then
$	  Write sys$output "install patch kit ALPMOTF08_U3012 (for systems"
$	  Write sys$output "with Motif Worldwide Support installed, the kit"
$	  Write sys$output "is ALPDWMW02_U3012)."
$        Else
$	  Write sys$output "install patch kit VAXMOTF08_U3012 (for systems"
$	  Write sys$output "with Motif Worldwide Support installed, the kit"
$	  Write sys$output "is VAXDWMW02_U3012)."
$        Endif
$       Write sys$output " "
$      Endif
$    Else If (F$Length(DECW$Version) .NE. 0) .AND. -
            ((F$Locate("V1.3", DECW$Version) .NE. F$Length(DECW$Version)) .OR. -
            (F$Locate("T1.3", DECW$Version) .NE. F$Length(DECW$Version)))
$     Then
$      Macro = Macro + "MOTIF1_2=1,"
$      Motif12 = "1"
$      Motif13 = "1"
$      If F$Locate("1.3-0", DECW$Version) .NE. F$Length(DECW$Version)
$       Then Motif130 = "1"
$       Endif
$      If F$Locate("1.3-1", DECW$Version) .NE. F$Length(DECW$Version)
$       Then Motif131 = "1"
$       Endif
$     Else If (F$Length(DECW$Version) .NE. 0) .AND. -
            ((F$Locate("V1.4", DECW$Version) .NE. F$Length(DECW$Version)) .OR. -
            (F$Locate("T1.4", DECW$Version) .NE. F$Length(DECW$Version)))
$      Then
$       Macro = Macro + "MOTIF1_2=1,"
$       Motif12 = "1"
$       Motif13 = "1"
$       Motif14 = "1"
$       If F$Locate("1.4-1", DECW$Version) .NE. F$Length(DECW$Version)
$        Then Motif141 = "1"
$        Endif
$      Else If (F$Length(DECW$Version) .NE. 0) .AND. -
               (F$Locate("1.5", DECW$Version) .NE. F$Length(DECW$Version))
$       Then
$        Macro = Macro + "MOTIF1_2=1,"
$        Motif12 = "1"
$        Motif13 = "1"
$        Motif14 = "1"
$        Motif15 = "1"
$       Else If (F$Length(DECW$Version) .NE. 0) .AND. -
                (F$Locate("1.6", DECW$Version) .NE. F$Length(DECW$Version))
$        Then
$         Macro = Macro + "MOTIF1_2=1,"
$         Motif12 = "1"
$         Motif13 = "1"
$         Motif14 = "1"
$         Motif15 = "1"
$         Motif16 = "1"
$        Else
$         Write sys$output "You either have DECW Motif installed incorrectly or"
$         Write sys$output "you have an unsupported version of it."
$         Goto Err
$        EndIf
$       EndIf
$      Endif
$     Endif
$    Endif
$Motif11:
$   Set Symbol/Scope = (NoGlobal, NoLocal)
$  Else
$   Write sys$output "You do not seem to have DECW Motif installed correctly!"
$   Goto Err
$  EndIf
$!
$ If F$Trnlnm("XMU") .EQS. ""
$  Then   
$   Write sys$output -
     "You cannot build Mosaic without the XMU auxiliary library.  It comes as"
$   Write sys$output -
     "an optional subset with DECWindows Motif, so you should install that now."
$   Goto Err
$  EndIf
$!
$ If F$Trnlnm("LNK$LIBRARY") .NES. ""
$  Then
$   Write sys$output -
     "Logical LNK$LIBRARY is currently defined.  If the link of Mosaic fails,"
$   Write sys$output -
     "please deassign all LNK$LIBRARY logicals and rerun the build procedure."
$   Write sys$output " "
$  EndIf
$!
$ If F$Trnlnm("BITMAPS") .NES. ""
$  Then
$   Write sys$output -
     "Logical BITMAPS is currently defined.  If the compile of Mosaic fails,"
$   Write sys$output -
     "please deassign all BITMAPS logicals and rerun the build procedure."
$   Write sys$output " "
$  EndIf
$!
$!
$! Create config.h, first the VMS invariant parts.
$!
$ Type$$ sys$input /output=test_config.h_'work'
/* config.h.  Generated automatically on VMS by MAKE_MOSAIC.COM.  */
/*            Do not edit this file, it will just be overwritten  */
/*            during the build. */

/* Define to empty if the keyword does not work.  */
/* #undef const */

#define XMOSAIC 1
/*#define PRERELEASE 1*/

/* Defines for system types */
/* #undef ALPHA */
/* #undef _BSD */
/* #undef DGUX */
/* #undef NEXT */
/* #undef _HPUX_SOURCE */
/* #undef MO_IRIX5 */
/* #undef SVR4 */
/* #undef SOLARIS */
/* #undef SOLARIS23 */
/* #undef SOLARIS24 */
/* #undef SOLARIS24X86 */
/* #undef SCO */
/* #undef SUN */
/* #undef CONVEX */
/* #undef MOTOROLA */
/* #undef BROKEN_MOTIF */

/* External software packages */
#define HAVE_JPEG 1
#define HAVE_PNG 1
#define HAVE_WAIS 1

/* Define to figure Motif version (does anybody use 1.0?) */
#define MOTIF 1
#define MOTIF1_1 1

/* Include the build specific config */
#ifdef __GNUC__
#include MOSAIC_CONFIG
#else
#include "mosaic_config"
#endif
$!
$!    The rest of the config has to be customized for each site
$!    and build configuration.  It is placed in a build specific
$!    include file.
$!        
$ Open/write config_file test_config_'work'.h
$ If Motif12 .EQS. ""
$  Then Write config_file "/* #undef MOTIF1_2 */"
$  Else Write config_file "#define MOTIF1_2 1"
$ Endif
$ Write config_file "/* #undef MOTIF2_0 */"
$ Write config_file ""
$ Write config_file ""
$ Write config_file "/* These are VMS port specific */"
$ If Motif123 .EQS. ""
$  Then Write config_file "/* #undef MOTIF1_23 */"         
$  Else
$   If Motif123 .EQS. "7"
$    Then Write config_file "#define MOTIF1_23 7"
$    Else Write config_file "#define MOTIF1_23 1"
$   Endif
$  Endif
$ If Motif124 .EQS. ""
$  Then Write config_file "/* #undef MOTIF1_24 */"
$  Else Write config_file "#define MOTIF1_24 1"
$  Endif
$ If Motif125 .EQS. ""
$  Then Write config_file "/* #undef MOTIF1_25 */"
$  Else Write config_file "#define MOTIF1_25 1"
$  Endif
$ If Motif126 .EQS. ""
$  Then Write config_file "/* #undef MOTIF1_26 */"
$  Else Write config_file "#define MOTIF1_26 1"
$  Endif
$ If Motif13 .EQS. ""
$  Then Write config_file "/* #undef MOTIF1_3 */"
$  Else Write config_file "#define MOTIF1_3 1"
$  Endif
$ If Motif130 .EQS. ""
$  Then Write config_file "/* #undef MOTIF1_30 */"
$  Else Write config_file "#define MOTIF1_30 1"
$  Endif
$ If Motif131 .EQS. ""
$  Then Write config_file "/* #undef MOTIF1_31 */"
$  Else Write config_file "#define MOTIF1_31 1"
$  Endif
$ If Motif14 .EQS. ""
$  Then Write config_file "/* #undef MOTIF1_4 */"
$  Else Write config_file "#define MOTIF1_4 1"
$  Endif
$ If Motif141 .EQS. ""
$  Then Write config_file "/* #undef MOTIF1_41 */"
$  Else Write config_file "#define MOTIF1_41 1"
$  Endif
$ If Motif15 .EQS. ""
$  Then Write config_file "/* #undef MOTIF1_5 */"
$  Else Write config_file "#define MOTIF1_5 1"
$  Endif
$ If Motif16 .EQS. ""
$  Then Write config_file "/* #undef MOTIF1_6 */"
$  Else Write config_file "#define MOTIF1_6 1"
$  Endif
$!
$ Write config_file "#define HOME_PAGE_DEFAULT ""''HOME_PAGE'"""
$ DUMMY_PRINT = "''PRINT_COMMAND'"
$ Write config_file "#define PRINT_DEFAULT ""''DUMMY_PRINT'"""
$ Write config_file "#define MAIL_PREFIX_DEFAULT ""''MAIL_PREFIX'"""
$ Write config_file "#define EDITOR_DEFAULT ""''EDIT_COMMAND'"""
$ Write config_file "#define DEFAULT_NEWS_HOST ""''NEWS_HOST'"""
$ Write config_file "#define DEFAULT_PS_VIEWER ""''POSTSCRIPT_VIEWER'"""
$!
$ If (IP .EQS. "UCX") .OR. (IP .EQS. "MULTINET_UCX") .OR. (IP .EQS. "CMU") -
	.OR. (IP .EQS. "PATHWAY_UCX") .OR. (IP .EQS. "TCPWARE")
$  Then Write config_file "#define UCX 1"
$  Else Write config_file "/* #undef UCX */"
$  Endif
$ If IP .EQS. "MULTINET"
$  Then Write config_file "#define MULTINET 1"
$  Else Write config_file "/* #undef MULTINET */"
$  Endif
$ If IP .EQS. "PATHWAY"
$  Then Write config_file "#define WIN_TCP 1"
$  Else Write config_file "/* #undef WIN_TCP */"
$  Endif
$ If IP .EQS. "SOCKETSHR"
$  Then Write config_file "#define SOCKETSHR 1"
$  Else Write config_file "/* #undef SOCKETSHR */"
$  Endif
$ If IP .EQS. "TCPWARE"
$  Then Write config_file "#define TCPWARE 1"
$  Else Write config_file "/* #undef TCPWARE */"
$  Endif
$ If (IP .EQS. "MULTINET_UCX") .OR. (IP .EQS. "CMU") .OR. (IP .EQS. "TCPWARE") -
	.OR. (IP .EQS. "PATHWAY_UCX")
$  Then Write config_file "#define UCX_COMPAT 1"
$  Else Write config_file "/* #undef UCX_COMPAT */"
$  Endif
$ If F$Locate("NOVMSLOGO", Args) .NE. F$Length(Args)
$  Then Write config_file "/* #undef VMSLOGO */"
$  Else Write config_file "#define VMSLOGO 1"
$  Endif
$ If Disable_Trace .EQ. 0
$  Then Write config_file "/* #undef DISABLE_TRACE */"
$  Else Write config_file "#define DISABLE_TRACE 1"
$  Endif
$ If CCI .EQ. 0
$  Then Write config_file "/* #undef CCI */"
$  Else Write config_file "#define CCI 1"
$  Endif
$ If SSL .GT. 0
$  Then Write config_file "#define HAVE_SSL 1"
$  Else Write config_file "/* #undef HAVE_SSL */"
$  Endif
$ If Tiff .EQ. 1
$  Then Write config_file "#define HAVE_TIFF 1"
$  Else Write config_file "/* #undef HAVE_TIFF */"
$  Endif
$ Close config_file
$!
$ Open/write config_file Built_'work'.h
$ Write config_file -
   "/* built_xx.h  Generated automatically on VMS by MAKE_MOSAIC.COM.  */"
$ Write config_file -
   "/*             Do not edit this file, it will just be overwritten  */"
$ Write config_file -
   "/*             during the build. */"
$ Write config_file " "
$ Write config_file "#define BUILD_TIME ""''f$time()'"""
$ Write config_file "#define IDENT_VER ""''Ident'"""
$ If VMS_Debug .EQ. 1
$  Then Write config_file "#define DEBUGVMS 1"
$  Else Write config_file "/* #undef DEBUGVMS */"
$  Endif
$ If SSL .EQ. 2
$  Then Write config_file "#define HAVE_HPSSL 1"
$  Else Write config_file "/* #undef HAVE_HPSSL */"
$  Endif  
$ Close config_file
$ Purge Built_'work'.h
$!
$! This file is only referenced by MMS descript files
$ Open/write config_file test_SSL_'work'.h
$ Write config_file "/* Dummy file to force recompile of SSL stuff as needed */"
$ If SSL .EQ. 0 Then Write config_file "/* No SSL support */"
$ If SSL .EQ. 1 Then Write config_file "/* OpenSSL support */"
$ If SSL .EQ. 2 Then Write config_file "/* HP SSL support */"
$ Close config_file
$!
$! Don't want to create a new config.h on every build or MMS will rebuild
$! everything.  Update only if it changed or RECOMPILE specified.
$ If F$Search("config.h") .EQS. ""
$  Then Rename test_config.h_'work' config.h
$  Else
$   Set noon
$   Differ/out=nl: test_config.h_'work' config.h
$   Diff_err = $severity
$   Set on
$   If (diff_err .EQ. 1) .AND. -
       (F$Locate("RECOMPILE", Args) .EQ. F$Length(Args))
$    Then Delete test_config.h_'work';*
$    Else Rename test_config.h_'work' config.h
$    Endif
$  Endif
$!
$ If F$Search("config_''work'.h") .EQS. ""
$  Then Rename test_config_'work'.h config_'work'.h
$  Else
$   Set noon
$   Differ/out=nl: test_config_'work'.h config_'work'.h
$   Diff_err = $severity
$   Set on
$   If diff_err .EQ. 1
$    Then Delete test_config_'work'.h;*
$    Else Rename test_config_'work'.h config_'work'.h
$    Endif
$  Endif
$!
$ If F$Search("SSL_''work'.h") .EQS. ""
$  Then Rename test_SSL_'work'.h SSL_'work'.h
$  Else
$   Set noon
$   Differ/out=nl: test_SSL_'work'.h SSL_'work'.h
$   Diff_err = $severity
$   Set on
$   If diff_err .EQ. 1
$    Then Delete test_SSL_'work'.h;*
$    Else Rename test_SSL_'work'.h SSL_'work'.h
$    Endif
$  Endif
$!
$ Top_Dir = F$Environment("default")
$ If Compiler .NES. "GNUC"
$  Then
$   Define mosaic_config "''Top_Dir'config_''work'.h"
$   Define mosaic_built "''Top_Dir'built_''work'.h"
$   GNUC_Def = ""
$  Else
$   GNUC_Def = "/Define=(mosaic_config=<''Top_Dir'config_''work'.h>," + -
	"mosaic_built=<''Top_dir'built_''work'.h>)/Include=(gcc_include:)"
$   If Platform .EQS. "Alpha"
$    Then
$     If F$Locate("NOMMS", Args) .EQ. F$Length(Args)
$      Then GNUC_Def = GNUC_Def + "/Undefine=(""""alpha"""")"
$      Else GNUC_Def = GNUC_Def + "/Undefine=(""alpha"")"
$     Endif
$    Endif
$   If F$Locate("NOMMS", Args) .EQ. F$Length(Args)
$    Then 
$     If Platform .EQS. "Alpha"
$      Then Define GCC_Defines -
	"/Names=Upper/Float=ieee/cc1=""-fno-exceptions -g0""''GNUC_Def'"
$      Else Define GCC_Defines "/Names=Upper''GNUC_Def'"
$      Endif
$    Endif
$   Top_Root = Top_Dir - "]"
$   If Platform .EQS. "VAX"
$    Then
$     Define/Translation=Conceal gcc_include -
	"''Top_Root'.libvms.gcc_include_vax.]", "''Top_Root'.src.]", -
	"''Top_Root'.libwww2.]", "''Top_Root'.freewais-0_5.ir.]", -
	"''Top_Root'.libpng.]", "''Top_Root'.zlib.]", "''Top_Root'.libtiff.]", -
	"''Top_Root'.libjpeg.]", "''Top_Root'.libopenjpeg.]", -
	"''Top_Root'.libnut.]", "''Top_Root'.libxmx.]"
$    Else
$     Define/Translation=Conceal gcc_include -
	"''Top_Root'.libvms.gcc_include_alpha.]", -
	"''Top_Root'.libvms.gcc_include_alpha.vms.]", "''Top_Root'.src.]", -
	"''Top_Root'.libwww2.]", "''Top_Root'.freewais-0_5.ir.]", -
	"''Top_Root'.libpng.]", "''Top_Root'.zlib.]", "''Top_Root'.libtiff.]", -
	"''Top_Root'.libjpeg.]", "''Top_Root'.libopenjpeg.]", -
	"''Top_Root'.libnut.]", "''Top_Root'.libxmx.]"
$    Endif
$  Endif
$!
$ If (F$Locate("NOMMS", Args) .NE. F$Length(Args)) .AND. -
     (F$Locate("MMK", Args) .EQ. F$Length(Args)) Then Goto No_MMS
$ If (F$Search("SYS$System:MMS.Exe") .NES. "") .AND. -
     (F$Locate("MMK", Args) .EQ. F$Length(Args))
$  Then
$   Set Noon
$   Set Symbol/Scope = (Global, NoLocal)
$   Define/User sys$output tmpmms.lis
$   Define/User sys$error tmpmms.lis
$   MMS/Ident
$   MMS_Err = $Severity
$   Set Symbol/Scope = (NoGlobal, NoLocal)
$   If MMS_Err .NE. 1
$    Then
$     Write sys$output "MMS does not appear to be available."
$     Write sys$output "Proceeding with complete (re)build."
$     Write sys$output " "
$     Delete tmpmms.lis;*
$     Set On
$     Goto No_MMS
$    Endif
$   Define/User sys$output nl:
$   Define/User sys$error nl:
$   Search tmpmms.lis V3.8
$   If $Severity .NE. 3
$    Then
$     Define/User sys$output nl:
$     Define/User sys$error nl:
$     Search tmpmms.lis V3.8-01
$     If $Severity .EQ. 3
$      Then
$       Write sys$output "The MMS on this system is V3.8."
$       Write sys$output "The Mosaic build will not work if V3.8 was installed"
$       Write sys$output "with Extended File Specification support."
$       Write sys$output " "
$      Endif
$    Endif
$   Delete tmpmms.lis;*
$   Set On
$   Command = "MMS"
$  Else
$   If F$Search("MMK_DIR:MMK.Exe") .NES. ""
$    Then
$     Set Noon
$     Set Symbol/Scope = (Global, NoLocal)
$     Define/User sys$output tmpmmk.lis
$     Define/User sys$error tmpmmk.lis
$     MMK/Ident
$     MMK_Err = $Severity
$     Set Symbol/Scope = (NoGlobal, NoLocal)
$     Set On
$     If MMK_Err .NE. 1
$      Then
$       Write sys$output "Neither MMS or MMK appears to be available."
$       Write sys$output "Proceeding with complete (re)build."
$       Write sys$output " "
$       Delete tmpmmk.lis;*
$       Goto No_MMS
$      Endif
$     Set Noon
$     Define/User sys$output nl:
$     Define/User sys$error nl:
$     Search tmpmmk.lis V3.4,V3.5,V3.6,V3.7,V3.8,V3.9,V4.0,V4.1,V4.2
$     MMK_Err = $Severity
$     Delete tmpmmk.lis;*
$     Set On
$     If MMK_Err .EQ. 1
$      Then
$       Macro = Macro + "USE_MMK=1,"
$       Command = "MMK"
$      Else
$       Write sys$output "The MMK on this system is a version prior to V3.4"
$       Write sys$output "The Mosaic build needs V3.4 or later."
$       Write sys$output "Proceeding with complete (re)build."
$       Write sys$output " "
$       Goto No_MMS
$      Endif
$    Else
$     Write sys$output "Neither MMS or MMK appears to be available."
$     Write sys$output "Proceeding with complete (re)build."
$     Write sys$output " "
$     Goto No_MMS
$    EndIf
$  EndIf
$!
$! Do MMS or MMK build
$!
$ Quals = Extras
$ If Macro .NES. "("
$  Then
$   Quals = Quals + "/Skip/Macro=" + Macro + ")"
$   Quals = Quals - ",)"
$   Quals = Quals + ")"
$  EndIf
$! MMS needs global symbols enabled
$ Set Symbol/Scope = (Global, NoLocal)
$! Define before deleting to avoid warning messages
$ Alpha:==
$ Cmu:==
$ Multinet:==
$ Pathway:==
$ Socketshr:==
$ Tcpware:==
$ Delete:==
$ Edit:==
$ Delete/symbol/global Alpha
$ Delete/symbol/global Cmu
$ Delete/symbol/global Edit
$ Delete/symbol/global Multinet
$ Delete/symbol/global Pathway
$ Delete/symbol/global Socketshr
$ Delete/symbol/global Tcpware
$ Delete/symbol/local Macro
$ Delete/symbol/global Delete
$!
$! Run MMS or MMK now
$!
$ Write sys$output "''Command' will be invoked as ''Command'''Quals'"
$ If Command .EQS. "MMS"
$  Then MMS'Quals'
$  Else MMK'Quals'
$  EndIf
$ Goto The_End
$!
$No_MMS:
$ If F$Locate("DEB", Args) .NE. F$Length(Args) Then Show Symbol/Local *
$ If F$Locate("CLEAN", Args) .NE. F$Length(Args) Then Goto No_mms_clean
$ If IP .NES. "PATHWAY"
$  Then
$   If Compiler .EQS. "DECC"
$    Then
$     If Platform .EQS. "VAX"
$      Then If F$Trnlnm("DECC$LIBRARY_INCLUDE") .NES. "" Then Define Sys DECC$Library_Include
$      Else If F$Trnlnm("ALPHA$LIBRARY") .NES. "" Then Define Sys Alpha$Library
$      Endif
$    Else
$     If Compiler .EQS. "GNUC"
$      Then Define Sys GNU_CC_Include
$      Else Define Sys SYS$Library
$      Endif
$    Endif
$  Endif
$ Topdir = F$Environment("Default") - "]" + "."
$ If F$Locate("END", Args) .NE. F$Length(Args)
$  Then
$   End = F$Extract(F$Locate("END", Args) + 4, 20, Args)
$   If F$Locate(",", End) .NE. F$Length(End)
$    Then End = F$Extract(0, F$Locate(",", End), End)
$    Endif
$  Else End = ""
$  Endif
$ If F$Locate("START", Args) .NE. F$Length(Args)
$  Then
$   Start = F$Extract(F$Locate("START", Args) + 6, 20, Args)
$   If F$Locate(",", Start) .NE. F$Length(Start)
$    Then Start = F$Extract(0, F$Locate(",", Start), Start)
$    Endif
$   xxx = F$Verify(1)
$   Goto 'Start'
$  Endif
$!
$ xxx = F$Verify(1)
$Begin:
$Wais:
$ If F$Locate("NOWAIS", Args) .NE. F$Length(Args) Then Goto After_WAIS
$!
$! Create [.freeWAIS-0_5.ir]libWAIS.olb
$!
$ Set Default [.freeWAIS-0_5.ir]
$ If F$Search("libWAIS.olb") .NES. "" Then Delete libWAIS.olb;* 
$ Library/Create/Log libWAIS.olb
$ Comp = CC + COpt_NoVAXC + "/Define=''IP'"
$ 'Comp' cutil.c
$ 'Comp' futil.c
$ 'Comp' panic.c
$ 'Comp' ui.c
$ 'Comp' wmessage.c
$ 'Comp' wprot.c
$ 'Comp' wutil.c
$ 'Comp' zprot.c
$ 'Comp' ztype1.c
$ 'Comp' zutil.c
$ Library/Replace/Log libWAIS.olb *.obj
$ Purge *.obj
$ Set Default [-.-]
$After_WAIS:
$ If End .EQS. "WAIS" Then Goto The_End
$!
$! Create [.libwww2]libwww.olb       
$!
$Libwww2:
$ Set Default [.libwww2]
$ If F$Search("libwww.olb") .NES. "" Then Delete libwww.olb;*
$ Library/Create/Log libwww.olb
$ Comp = CC + COpt_NoVAXC + GNUC_Def
$ 'Comp' HTAABrow.c
$ 'Comp' HTAAUtil.c
$ 'Comp' HTAccess.c
$ 'Comp' HTAlert.c
$ 'Comp' HTAnchor.c
$ 'Comp' HTAssoc.c
$ 'Comp' HTAtom.c
$ 'Comp' HTBTree.c
$ 'Comp' HTChunk.c
$ 'Comp' HTCompressed.c
$ 'Comp' HTCookie.c
$ 'Comp' HTFile.c
$ 'Comp' HTFinger.c
$ 'Comp' HTFormat.c
$ 'Comp' HTFTP.c
$ 'Comp' HTFWriter
$ 'Comp' HTGopher.c
$ 'Comp' HTIcon.c
$ 'Comp' HTInit.c
$ 'Comp' HTList.c
$ 'Comp' HTMailto.c
$ 'Comp' HTMIME.c
$ 'Comp' HTML.c
$ 'Comp' HTMLDTD.c
$ 'Comp' HTMLGen
$ 'Comp' HTMosaicHTML.c
$ 'Comp' HTNews.c
$ 'Comp' HTParse.c
$ 'Comp' HTPlain.c
$ 'Comp' HTSort.c
$ 'Comp' HTString.c
$ 'Comp' HTTCP.c
$ 'Comp' HTTelnet.c
$ 'Comp' HTTP.c
$ 'Comp' HTUU.c
$ 'Comp' HTVMSUtils.c
$ 'Comp' HTWriter.c
$ 'Comp' SGML.c
$ If F$Locate("NOWAIS", Args) .EQ. F$Length(Args)
$  Then
$   'Comp' HTWSRC.c
$   Define WAIS_IR 'Topdir'freeWAIS-0_5.ir]
$   If Compiler .EQS. "DECC"
$    Then
$     Define DECC$User_Include 'F$Environment("Default")',WAIS_IR
$     Define DECC$System_Include 'F$Environment("Default")',WAIS_IR
$    Else
$     Define VAXC$Include 'F$Environment("Default")',WAIS_IR,SYS$Library
$     Define C$Include 'F$Environment("Default")',WAIS_IR
$    EndIf
$   'Comp' HTWAIS.c
$  EndIf
$ Library/Replace/Log libwww.olb *.obj
$ Purge *.obj
$ Set Default [-]
$ If End .EQS. "LIBWWW2" Then Goto The_End
$!
$! Create [.libhtmlw]libhtmlw.olb
$!
$Libhtmlw:
$ Set Default [.libhtmlw]
$ If F$Search("libhtmlw.olb") .NES. "" Then Delete libhtmlw.olb;*
$ Library/Create/Log libhtmlw.olb
$ Comp = CC + COpt_NoVAXC + GNUC_Def
$ 'Comp' HTML-PSformat.c
$ 'Comp' HTML.c
$ 'Comp' HTMLformat.c
$ 'Comp' HTMLimages.c
$ 'Comp' HTMLlists.c
$ 'Comp' HTMLparse.c
$ 'Comp' HTMLtable.c
$ 'Comp' HTMLwidgets.c
$ 'Comp' LIST.c
$ 'Comp' HTMLapplet
$ 'Comp' HTMLfont
$ 'Comp' HTMLform
$ 'Comp' HTMLframe
$ 'Comp' HTMLimagemap.c
$ 'Comp' HTMLtext
$ Library/Replace/Log libhtmlw.olb *.obj
$ Purge *.obj
$ Set Default [-]
$ If End .EQS. "LIBHTMLW" Then Goto The_End
$!
$! Create [.libxmx]libxmx.olb
$!
$LibXmx:
$ Set Default [.libXmx]
$ If F$Search("libXmx.olb") .NES. "" Then Delete libXmx.olb;*
$ Library/Create/Log libXmx.olb
$ Comp = CC + COpt_NoVAXC + GNUC_Def
$ 'Comp' Xmx.c
$ 'Comp' Xmx2.c
$ Library/Replace/Log libXmx.olb *.obj
$ Purge *.obj
$ Set Default [-]
$ If End .EQS. "LIBXMX" Then Goto The_End
$!
$! Create [.libjpeg]libjpeg.olb
$!
$Libjpeg:
$ Set Default [.libjpeg]
$ @Build 'Macro'
$ Purge *.obj,*.olb
$ Set Default [-]
$ If End .EQS. "LIBJPEG" Then Goto The_End
$!
$! Create [.libopenjpeg]libopenjpeg.olb
$!
$Libopenjpeg:
$ Set Default [.libopenjpeg]
$ If F$Search("libopenjpeg.olb") .NES. "" Then Delete libopenjpeg.olb;*
$ Library/Create/Log libopenjpeg.olb
$ Comp = CC + COpt_NoVAXC + GNUC_Def
$ 'Comp' bio.c
$ 'Comp' cio.c
$ 'Comp' dwt.c
$ 'Comp' event.c
$ 'Comp' image.c
$ 'Comp' j2k.c
$ 'Comp' j2k_lib.c
$ 'Comp' jp2.c
$ 'Comp' jpt.c
$ 'Comp' mct.c
$ 'Comp' mqc.c
$ 'Comp' openjpeg.c
$ 'Comp' pi.c
$ 'Comp' raw.c
$ 'Comp' t1.c
$ 'Comp' t2.c
$ 'Comp' tcd.c
$ 'Comp' tgt.c
$ Library/Replace/Log libopenjpeg.olb *.obj
$ Purge *.obj
$ Set Default [-]
$ If End .EQS. "LIBOPENJPEG" Then Goto The_End
$!
$! Create [.libnut]libnut.olb
$!
$Libnut:
$ Set Default [.libnut]
$ If F$Search("libnut.olb") .NES. "" Then Delete libnut.olb;*
$ Library/Create/Log libnut.olb
$ Comp = CC + COpt_NoVAXC + GNUC_Def
$ 'Comp' ellipsis.c
$ 'Comp' mm.c
$ 'Comp' str-tools.c
$ 'Comp' system.c
$ 'Comp' url-utils.c
$ Library/Replace/Log libnut.olb *.obj
$ Purge *.obj
$ Set Default [-]
$ If End .EQS. "LIBNUT" Then Goto The_End
$!
$! Create [.libpng]libpng.olb
$!
$Libpng:
$ Set Default [.libpng]
$ If F$Search("libpng.olb") .NES. "" Then Delete libpng.olb;*
$ Library/Create/Log libpng.olb
$ Comp = CC + COpt_NoVAXC + "/INCLUDE=[-.ZLIB]"
$ 'Comp' png.c
$ 'Comp' pngpread.c
$ 'Comp' pngget.c
$ 'Comp' pngset.c
$ 'Comp' pngrutil.c
$ 'Comp' pngtrans.c
$ 'Comp' pngwutil.c
$ 'Comp' pngread.c
$ 'Comp' pngmem.c
$ 'Comp' pngwrite.c
$ 'Comp' pngrtran.c
$ 'Comp' pngwtran.c
$ 'Comp' pngrio.c
$ 'Comp' pngwio.c
$ 'Comp' pngerror.c
$ Library/Replace/Log libpng.olb *.obj
$ Purge *.obj
$ Set Default [-]
$ If End .EQS. "LIBPNG" Then Goto The_End
$!
$! Create [.zlib]libz.olb
$!
$Zlib:
$ Set Default [.zlib]
$ If F$Search("libz.olb") .NES. "" Then Delete libz.olb;*
$ Library/Create/Log libz.olb
$ Comp = CC + COpt_NoVAXC
$ 'Comp' adler32.c
$ 'Comp' compress.c
$ 'Comp' crc32.c
$ 'Comp' deflate.c
$ 'Comp' gzio.c
$ 'Comp' infback.c
$ 'Comp' inffast.c
$ 'Comp' inflate.c
$ 'Comp' inftrees.c
$ 'Comp' trees.c
$ 'Comp' uncompr.c
$ 'Comp' zutil.c
$ Library/Replace/Log libz.olb *.obj
$ Purge *.obj
$ Set Default [-]
$ If End .EQS. "ZLIB" Then Goto The_End
$!
$! Create [.libpng]libtiff.olb
$!
$Libtiff:
$ If Tiff .EQ. 0 Then Goto After_TIFF
$ Set Default [.libtiff]
$ If F$Search("libtiff.olb") .NES. "" Then Delete libtiff.olb;*
$ Library/Create/Log libtiff.olb
$ Comp = CC + COpt_NoVAXC + "/INCLUDE=([-.ZLIB],[-.LIBJPEG])"
$ 'Comp' tif_aux.c
$ 'Comp' tif_close.c
$ 'Comp' tif_codec.c
$ 'Comp' tif_color.c
$ 'Comp' tif_compress.c
$ 'Comp' tif_dir.c
$ 'Comp' tif_dirinfo.c
$ 'Comp' tif_dirread.c
$ 'Comp' tif_dirwrite.c
$ 'Comp' tif_dumpmode.c
$ 'Comp' tif_error.c
$ 'Comp' tif_extension.c
$ 'Comp' tif_fax3.c
$ 'Comp' tif_fax3sm.c
$ 'Comp' tif_getimage.c
$ 'Comp' tif_jpeg.c
$ 'Comp' tif_ojpeg.c
$ 'Comp' tif_flush.c
$ 'Comp' tif_luv.c
$ 'Comp' tif_lzw.c
$ 'Comp' tif_next.c
$ 'Comp' tif_open.c
$ 'Comp' tif_packbits.c
$ 'Comp' tif_pixarlog.c
$ 'Comp' tif_predict.c
$ 'Comp' tif_print.c
$ 'Comp' tif_read.c
$ 'Comp' tif_swab.c
$ 'Comp' tif_strip.c
$ 'Comp' tif_thunder.c
$ 'Comp' tif_tile.c
$ 'Comp' tif_version.c
$ 'Comp' tif_warning.c
$ 'Comp' tif_write.c
$ 'Comp' tif_zip.c
$ 'Comp' tif_vms.c
$ 'Comp' lfind.c
$ Library/Replace/Log libtiff.olb *.obj
$ Purge *.obj
$ Set Default [-]
$After_TIFF:
$ If End .EQS. "LIBTIFF" Then Goto The_End
$!
$! Create [.libliteclue]libliteclue.olb
$!
$Libliteclue:
$ Set Default [.libliteclue]
$ If F$Search("libliteclue.olb") .NES. "" Then Delete libliteclue.olb;*
$ Library/Create/Log libliteclue.olb
$ Comp = CC + COpt_NoVAXC + GNUC_Def
$ 'Comp' liteclue.c
$ Library/Replace/Log libliteclue.olb *.obj
$ Purge *.obj
$ Set Default [-]
$ If End .EQS. "LIBLITECLUE" Then Goto The_End
$!
$! Create [.libvms]libvms.olb
$!
$Libvms:
$ Set Default [.libvms]
$ If F$Search("libvms.olb") .NES. "" Then Delete libvms.olb;*
$ Library/Create/Log libvms.olb
$ Comp = CC + COpt_NoVAXC + GNUC_Def
$ 'Comp' cmdline.c
$ Set Command/Object mosaic_cld.cld
$ Library/Replace/Log libvms.olb *.obj
$ Edit/Tpu/Nosection/Nodisplay/Command=cvthelp.tpu mosaic.help
$ Runoff/Output=[-]mosaic.hlp mosaic.rnh
$ If (IP .EQS. "CMU") .AND. (Compiler .NES. "DECC")
$  Then
$   If F$Search("ucx$ipc.olb") .EQS. "" Then Library/Create/Log ucx$ipc.olb
$   Macro/Object=ucx$crtlibxfr.obj ucx$crtlibxfr.mar
$   Library/Replace/Log ucx$ipc.olb ucx$crtlibxfr.obj
$  Endif
$ Purge *.obj
$ Set Default [-]
$ If End .EQS. "LIBVMS" Then Goto The_End
$!
$! Compile the [.src] modules.
$!
$Src:
$ Set Default [.src]
$ Comp = CC + COpt_NoVAXC + GNUC_Def
$ If F$Search("src.olb") .NES. "" Then Delete src.olb;*
$ Library/Create/Log src.olb
$ If Compiler .NES. "GNUC"
$  Then
$   Define LIBWWW2     'Topdir'LIBWWW2]
$   Define LIBXMX      'Topdir'LIBXMX]
$   Define LIBHTMLW    'Topdir'LIBHTMLW]
$   Define LIBTIFF     'Topdir'LIBTIFF]
$   Define LIBJPEG     'Topdir'LIBJPEG]
$   Define LIBOPENJPEG 'Topdir'LIBOPENJPEG]
$   Define LIBNUT      'Topdir'LIBNUT]
$   Define LIBPNG      'Topdir'LIBPNG]
$   Define ZLIB        'Topdir'ZLIB]
$   Define LIBVMS      'Topdir'LIBVMS]
$   If Compiler .EQS. "DECC"
$    Then
$     Define/nolog DECC$User_Include 'F$Environment("Default")', -
        LIBWWW2,LIBHTMLW,LIBXMX,LIBTIFF,LIBJPEG,LIBOPENJPEG,LIBPNG,ZLIB,SYS
$     Define/nolog DECC$System_Include 'F$Environment("Default")', -
        LIBWWW2,LIBHTMLW,LIBXMX,LIBTIFF,LIBJPEG,LIBOPENJPEG,LIBPNG,ZLIB,SYS
$    Else  
$     Define/nolog VAXC$Include 'F$Environment("Default")', -
        LIBWWW2,LIBHTMLW,LIBXMX,LIBJPEG,LIBOPENJPEG,LIBPNG,ZLIB,SYS$Library
$     Define/nolog C$Include 'F$Environment("Default")', -
        LIBWWW2,LIBHTMLW,LIBXMX,LIBJPEG,LIBOPENJPEG,LIBPNG,ZLIB,SYS$Library
$    EndIf
$  EndIf
$ If CCI .EQ. 1
$  Then
$   'Comp' accept.c
$   'Comp' bla.c
$   'Comp' ccibindings.c
$   'Comp' ccibindings2.c
$   'Comp' cciserver.c
$   'Comp' support.c
$  Endif
$ 'Comp' annotate.c
$ 'Comp' audan.c
$ 'Comp' child.c
$ 'Comp' colors.c
$ 'Comp' comment.c
$ 'Comp' fsdither.c
$ 'Comp' gifread.c
$ 'Comp' globalhist.c
$ 'Comp' grpan-www.c
$ 'Comp' grpan.c
$ 'Comp' gui-dialogs.c
$ 'Comp' gui-documents.c
$ 'Comp' gui-extras.c
$ 'Comp' gui-ftp.c
$ 'Comp' gui-menubar.c
$ 'Comp' gui-news.c
$ 'Comp' gui-popup.c
$ 'Comp' gui.c
$ 'Comp' history.c
$ 'Comp' hotfile.c
$ 'Comp' hotlist.c
$ 'Comp' img.c
$ 'Comp' libtarga.c
$ 'Comp' mailto.c
$ 'Comp' main.c
$ 'Comp' md5.c
$ 'Comp' medcut.c
$ 'Comp' mo-www.c
$ 'Comp' newsrc.c
$ 'Comp' pan.c
$ 'Comp' picread.c
$ 'Comp' pixmaps.c
$ 'Comp' prefs.c
$ 'Comp' proxy-dialogs.c
$ 'Comp' proxy-misc.c
$ 'Comp' quantize.c
$ 'Comp' readbmp.c
$ 'Comp' readjpeg.c
$ 'Comp' readj2k.c
$ 'Comp' readpng.c
$ 'Comp' readsun.c
$ 'Comp' readtga.c
$ 'Comp' readtiff.c
$ 'Comp' readxwd.c
$ 'Comp' xpmhash.c
$ 'Comp' xpmread.c
$ Library/Replace/Log src.olb *.obj
$ Purge *.obj
$ Set Default [-]
$ If End .EQS. "SRC" Then Goto The_End
$!
$! Finally link ...
$!
$Link:
$ If F$Locate("NOLINK", Args) .NE. F$Length(Args) Then Goto The_End
$ Set Default [.src]
$ Open/Write libraries_file libraries.opt
$ Write libraries_file "Identification=""Mosaic ''Ident'"""
$ Write libraries_file "src.olb/Inc=(main)"
$ Write libraries_file "src.olb/Lib/Inc=(mailto,md5)"
$ Write libraries_file "[-.libhtmlw]libhtmlw.olb/Lib"
$ Write libraries_file "[-.libxmx]libxmx.olb/Lib"
$ Write libraries_file "[-.libwww2]libwww.olb/Lib"
$ If Tiff .EQ. 1
$  Then Write libraries_file "[-.libtiff]libtiff.olb/Lib"
$  Endif
$ Write libraries_file "[-.libjpeg]libjpeg.olb/Lib"
$ Write libraries_file "[-.libopenjpeg]libopenjpeg.olb/Lib"
$ Write libraries_file "[-.libnut]libnut.olb/Lib"
$ Write libraries_file "[-.libpng]libpng.olb/Lib"
$ Write libraries_file "[-.zlib]libz.olb/Lib"
$ Write libraries_file "[-.libliteclue]libliteclue.olb/Lib"
$ Write libraries_file "[-.libvms]libvms.olb/Lib"
$ If F$Locate("NOWAIS", Args) .EQ. F$Length(Args)
$  Then Write libraries_file "[-.freeWAIS-0_5.ir]libWAIS.olb/Lib"
$  Endif
$ If SSL .EQ. 1
$  Then
$   Write libraries_file "ssllib:libssl.olb/Lib"
$   Write libraries_file "ssllib:libcrypto.olb/Lib"
$  Endif
$ If SSL .EQ. 2
$  Then
$   Write libraries_file "sys$share:ssl$libssl_shr32/Share"
$   Write libraries_file "sys$share:ssl$libcrypto_shr32/Share"
$  Endif
$ If Compiler .EQS. "GNUC"
$  Then
$   If Platform .EQS. "VAX"
$    Then
$     Write libraries_file "GNU_CC:[000000]gcclib.olb/Lib"
$    Else
$     Write libraries_file "GNU:[000000]LIBGCC.Olb/Lib"
$!     Write libraries_file "SYS$Library:VaxCRTL.Olb/Lib"
$     Write libraries_file "GNU:[000000]CRT0.Obj"
$    Endif
$  Endif
$ If Motif12 .NES. ""
$  Then
$   Write libraries_file "SYS$Library:DECW$DXmLibShr12.Exe/Share"
$   Write libraries_file "SYS$Library:DECW$XmuLibShrR5.Exe/Share"
$   Write libraries_file "SYS$Library:DECW$XmLibShr12.Exe/Share"
$   Write libraries_file "SYS$Library:DECW$XtLibShrR5.Exe/Share"
$  Else
$   Write libraries_file "SYS$Library:DECW$DXmLibShr.Exe/Share"
$   Write libraries_file "SYS$Library:DECW$XmuLibShr.Exe/Share"
$   Write libraries_file "SYS$Library:DECW$XmLibShr.Exe/Share"
$   Write libraries_file "SYS$Library:DECW$XtShr.Exe/Share"
$  Endif
$ Write libraries_file "SYS$Library:DECW$XLibShr.Exe/Share"
$ Write libraries_file "SYS$Library:DECW$XExtLibShr.Exe/Share"
$!
$ If (IP .EQS. "UCX") .AND. (Compiler .NES. "DECC")
$  Then Write libraries_file "SYS$Library:UCX$IPC.Olb/Lib"
$  EndIf
$ If (IP .EQS. "CMU") .AND. (Compiler .NES. "DECC")
$  Then Write libraries_file "''Topdir'libvms]UCX$IPC.Olb/Lib"
$  Endif
$ If IP .EQS. "MULTINET"
$  Then Write libraries_file "Multinet_Socket_Library/Share"
$  EndIf
$ If (IP .EQS. "MULTINET_UCX") .AND. (Compiler .NES. "DECC")
$  Then Write libraries_file "Multinet_root:[multinet.library]UCX$IPC.Olb/Lib"
$  EndIf
$ If IP .EQS. "PATHWAY"
$  Then Write libraries_file "SYS$Library:TWGLib/Share"
$  EndIf
$ If (IP .EQS. "PATHWAY_UCX") .AND. (Compiler .NES. "DECC")
$  Then Write libraries_file "TWG$ETC:[000000]UW$IPC.Olb/Lib"
$  EndIf
$ If IP .EQS. "SOCKETSHR"
$  Then Write libraries_file "Socketshr/Share"
$  EndIf
$ If (IP .EQS. "TCPWARE") .AND. (Compiler .NES. "DECC")
$  Then Write libraries_file "TCPWARE:UCX$IPC.Olb/Lib"
$  EndIf
$!
$ If Compiler .EQS. "VAXC"
$  Then Write libraries_file "SYS$Library:VaxCRTL.Exe/Share"
$  EndIf
$ Close libraries_file
$ Link'LOpt'/Exe=mosaic.exe libraries.opt/opt
$ xxx = F$Verify(Verify)
$ Purge *.opt
$ Set Default [-]
$ Write sys$output "Linking done.  Welcome to VMS Mosaic " + Ident
$ Write sys$output "The executable is [.src]Mosaic.exe"
$ Write sys$output "Please complete the Comment Card (see Help menu)" 
$ Dir/Date=Modif/Size=All [.src]Mosaic.exe
$ Goto The_End
$!
$No_mms_clean:
$ Purge Mosaic.hlp
$ Purge *.h
$ Purge [.src]Mosaic.exe
$ Delete [.src]*.obj;*
$ Delete [.src]*.olb;*
$ Delete [.freewais-0_5.ir]*.obj;*
$ Delete [.freewais-0_5.ir]*.olb;*
$ Delete [.libwww2]*.obj;*
$ Delete [.libwww2]*.olb;*
$ Delete [.libxmx]*.obj;*
$ Delete [.libxmx]*.olb;*
$ Delete [.libhtmlw]*.obj;*
$ Delete [.libhtmlw]*.olb;*
$ Delete [.libnut]*.obj;*
$ Delete [.libnut]*.olb;*
$ Delete [.libtiff]*.obj;*
$ Delete [.libtiff]*.olb;*
$ Delete [.libjpeg]*.obj;*
$ Delete [.libjpeg]*.olb;*
$ Delete [.libopenjpeg]*.obj;*
$ Delete [.libopenjpeg]*.olb;*
$ Delete [.libpng]*.obj;*
$ Delete [.libpng]*.olb;*
$ Delete [.zlib]*.obj;*
$ Delete [.zlib]*.olb;*
$ Delete [.libvms]*.obj;*
$ Delete [.libvms]*.olb;*
$ Delete [.libvms]*.rnh;*
$ Write sys$output "Cleanup completed."
$ Goto The_End
$EndY:
$ Write sys$output "Execution interrupted..."
$Err:
$The_End:
$ xxx = F$Verify(Verify)
$ If (F$Trnlnm("OPENSSL", "LNM$PROCESS_TABLE") .NES. "") .AND. -
     (F$Type(SSL_Log) .NES. "") Then If (SSL_Log .EQ. 1) -
   Then Deassign/Process OPENSSL
$ If F$Trnlnm("SYS", "LNM$PROCESS_TABLE") .NES. "" Then -
   Deassign/Process SYS
$ If F$Trnlnm("NETINET", "LNM$PROCESS_TABLE") .NES. "" Then -
   Deassign/Process NETINET
$ If F$Trnlnm("LIBHTMLW", "LNM$PROCESS_TABLE") .NES. "" Then -
   Deassign/Process LIBHTMLW
$ If F$Trnlnm("LIBWWW2", "LNM$PROCESS_TABLE") .NES. "" Then -
   Deassign/Process LIBWWW2
$ If F$Trnlnm("LIBXMX", "LNM$PROCESS_TABLE") .NES. "" Then -
   Deassign/Process LIBXMX
$ If F$Trnlnm("LIBTIFF", "LNM$PROCESS_TABLE") .NES. "" Then -
   Deassign/Process LIBTIFF
$ If F$Trnlnm("LIBJPEG", "LNM$PROCESS_TABLE") .NES. "" Then -
   Deassign/Process LIBJPEG
$ If F$Trnlnm("LIBOPENJPEG", "LNM$PROCESS_TABLE") .NES. "" Then -
   Deassign/Process LIBOPENJPEG
$ If F$Trnlnm("LIBNUT", "LNM$PROCESS_TABLE") .NES. "" Then -
   Deassign/Process LIBNUT
$ If F$Trnlnm("LIBPNG", "LNM$PROCESS_TABLE") .NES. "" Then -
   Deassign/Process LIBPNG
$ If F$Trnlnm("ZLIB", "LNM$PROCESS_TABLE") .NES. "" Then -
   Deassign/Process ZLIB
$ If F$Trnlnm("LIBVMS", "LNM$PROCESS_TABLE") .NES. "" Then -
   Deassign/Process LIBVMS
$ If F$Trnlnm("DECC$User_Include", "LNM$PROCESS_TABLE") .NES. "" Then -
   Deassign/Process DECC$User_Include
$ If F$Trnlnm("DECC$System_Include", "LNM$PROCESS_TABLE") .NES. "" Then -
   Deassign/Process DECC$System_Include
$ If F$Trnlnm("VAXC$Include", "LNM$PROCESS_TABLE") .NES. "" Then -
   Deassign/Process VAXC$Include
$ If F$Trnlnm("C$Include", "LNM$PROCESS_TABLE") .NES. "" Then -
   Deassign/Process C$Include
$ If F$Trnlnm("WAIS_IR", "LNM$PROCESS_TABLE") .NES. "" Then -
   Deassign/Process WAIS_IR
$ If F$Trnlnm("SOCKETSHR_FILES", "LNM$PROCESS_TABLE") .NES. "" Then -
   Deassign/Process SOCKETSHR_FILES
$ If F$Trnlnm("MOSAIC_CONFIG", "LNM$PROCESS_TABLE") .NES. "" Then -
   Deassign/Process MOSAIC_CONFIG
$ If F$Trnlnm("MOSAIC_BUILT", "LNM$PROCESS_TABLE") .NES. "" Then -
   Deassign/Process MOSAIC_BUILT
$ If F$Trnlnm("GCC_Include", "LNM$PROCESS_TABLE") .NES. "" Then -
   Deassign/Process GCC_Include
$ If F$Trnlnm("GCC_Defines", "LNM$PROCESS_TABLE") .NES. "" Then -
   Deassign/Process GCC_Defines
$ Set Default 'pwd'
$ Exit
