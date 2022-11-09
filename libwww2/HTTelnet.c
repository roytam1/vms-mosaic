/*		Telnet Access, Rlogin, etc.			HTAccess.c
**		===========================
**
** Authors
**	TBL	Tim Berners-Lee timbl@info.cern.ch
**	JFG	Jean-Francois Groff jgh@next.com
**	DD	Denis DeLaRoca (310) 825-4580  <CSP1DWD@mvs.oac.ucla.edu>
** History
**       8 Jun 92 Telnet hopping prohibited as telnet is not secure. (TBL)
**	26 Jun 92 When over DECnet, suppressed FTP, Gopher and News. (JFG)
**	 6 Oct 92 Moved HTClientHost and logfile into here. (TBL)
**	17 Dec 92 Tn3270 added, bug fix. (DD)
**	 2 Feb 93 Split from HTAccess.c.  Registration. (TBL)
*/

#include "../config.h"
#include "HTTelnet.h"

#include <signal.h>
#include "HTParse.h"
#include "HTAnchor.h"
#include "HTFile.h"
#include "HTTP.h"
#include "HTAlert.h"
#include <errno.h>
#include <stdio.h>

#include "../libnut/str-tools.h"

#ifdef VMS
#include <descrip.h>
#endif

#ifndef DISABLE_TRACE
extern int www2Trace;
#endif

/*	Make a string secure for passage to the
**	system() command.  Make it contain only alphanumneric
**	characters, or the characters '.', '-', '_', '+'.
**	Also remove leading '-' or '+'.
**	-----------------------------------------------------
*/
PRIVATE void make_system_secure (char *str)
{
	char *ptr1, *ptr2;

	if (!str || !*str)
		return;

	/*
	 * Remove leading '-' or '+' by making it into whitespace that
	 * will be stripped later.
	 */
	if ((*str == '-') || (*str == '+'))
		*str = ' ';

	ptr1 = ptr2 = str;

	while (*ptr1) {
		if (!isalpha((int)*ptr1) && !isdigit((int)*ptr1) &&
			(*ptr1 != '.') && (*ptr1 != '_') &&
			(*ptr1 != '+') && (*ptr1 != '-')) {
			ptr1++;
		} else {
			*ptr2++ = *ptr1++;
		}
	}
	*ptr2 = *ptr1;
}


#ifndef VMS
PRIVATE void run_a_command (char *command)
{
	char **argv;
	char *str;
	int argc = 0;
	int alen = 10;
	int i;

	argv = (char **)malloc(10 * sizeof(char *));
	if (!argv)
		return;

	str = strtok(command, " \t\n");
	while (str) {
		argv[argc] = strdup(str);
		if (++argc >= alen) {
			char **tmp_av;

			tmp_av = (char **)malloc((alen + 10) * sizeof(char *));
			if (!tmp_av)
				return;
			for (i = 0; i < alen; i++)
				tmp_av[i] = argv[i];
			alen += 10;
			free((char *)argv);
			argv = tmp_av;
		}
		str = strtok(NULL, " \t\n");
	}
	argv[argc] = NULL;

	if (!fork()) {
		execvp(argv[0], argv);
	} else {
		/*
		 * The signal handler in main.c will clean this child
		 * up when it exits.
		 */
		for (i = 0; i < argc; i++) {
			if (argv[i])
				free(argv[i]);
		}
		free((char *)argv);
	}
#else
/*
 * Spawn a subprocess, but do not wait for completion so that the
 * application_user_feedback can pop up before we need it.
 * Also, do not close the terminal window after logout until the user
 * has hit Return.
 * Clean up this later.
 */
PRIVATE void run_a_command (char *command, char *xterm_str)
{
	char cmd[256];
	char *fcname;
	char null_dev[] = "NL:";
	FILE *fpc;
	int flags = 1;
        $DESCRIPTOR(cmd_desc, NULL);
        $DESCRIPTOR(null_dev_desc, NULL);

        cmd_desc.dsc$a_pointer = cmd;
        null_dev_desc.dsc$a_pointer = null_dev;

	fcname = mo_tmpnam(NULL);
	strcat(fcname, ".COM");

	if (!(fpc = fopen(fcname, "w"))) {
	    fprintf(stderr, "\nVMS scratch file open error: %s\n",
		    strerror(errno, vaxc$errno));
	    return;
	}
	fprintf(fpc, "$ Set NoVerify\n$ On Error Then GoTo End\n");
	fprintf(fpc, "$ Define/User SYS$Input SYS$Command\n");
	fprintf(fpc, "$ %s\n$End:\n$ Write SYS$Output \" \"\n", command);
	fprintf(fpc,
	    "$ Read/Prompt=\"Hit Return to close window: \" SYS$Command ANS\n");
	fprintf(fpc, "$ Set NoOn\n$ Delete$$/NoConfirm/NoLog %s;\n$ Exit\n",
		fcname);
	fclose(fpc);
	sprintf(cmd, "%s @%s", xterm_str, fcname);
	cmd_desc.dsc$w_length = strlen(cmd);

	lib$spawn(&cmd_desc, &null_dev_desc, &null_dev_desc, &flags);
	free(fcname);
#endif /* VMS, BSN, GEC for PE */
}


/*	Telnet or "rlogin" access
**	-------------------------
*/
PRIVATE int remote_session (char *access, char *host)
{
  char *user, *hostname, *port;
  char command[256];
  char *xterm_str;
  int portnum;
  enum _login_protocol { telnet, rlogin, tn3270 } login_protocol;
  extern char *global_xterm_str;

  if (!access || !host) {
      application_user_feedback (
                      "Cannot open remote session, because\nURL is malformed.");
      return HT_NO_ACCESS;
  }
  login_protocol = !my_strcasecmp(access, "rlogin") ? rlogin :
      		   !my_strcasecmp(access, "tn3270") ? tn3270 : telnet;

  /* Make sure we won't overrun the size of command with a huge host string */
  if (strlen(host) > 200)
      host[200] = '\0';
  
  hostname = strchr(host, '@');
  port = strchr(host, ':');
  
  if (hostname) {
      *hostname++ = '\0';	/* Split */
      user = host;
  } else {
      hostname = host;
      user = NULL;		/* No user specified */
  }
  if (port) {
      *port++ = '\0';		/* Split */
      portnum = atoi(port);
  }

  /*
   * Make user and hostname secure by removing leading '-' or '+'
   * and allowing only alphanumeric, '.', '_', '+', and '-'.
   */
  make_system_secure(user);
  make_system_secure(hostname);
  
  xterm_str = global_xterm_str;
  
  if (login_protocol == rlogin) {
#if !defined(VMS) || defined(WIN_TCP)
      /* For rlogin, we should use -l user. */
      if (port && (portnum > 0) && (portnum < 63336)) {
          sprintf(command, "%s -e %s %s %d %s %s", xterm_str, access,
                  hostname, portnum,
                  user ? "-l" : "",
                  user ? user : "");
      } else {
          sprintf(command, "%s -e %s %s %s %s", xterm_str, access, hostname,
                  user ? "-l" : "",
                  user ? user : "");
      }
#else
#ifdef MULTINET
      sprintf(command, "%s%s%s%s%s %s", access,
              port ? "/PORT=" : "",
              port ? port : "",
              user ? "/USER=" : "",
              user ? user : "",
              hostname);
#else /* UCX */
      sprintf(command, "%s%s%s %s", access,
              user ? "/USER=" : "",
              user ? user : "",
              hostname);
#endif /* MULTINET - UCX, BSN */
#endif /* VMS, BSN */
  } else {
      /* For telnet, -l isn't safe to use at all -- most platforms
       * don't understand it. */
#ifndef VMS
      if (port && (portnum > 0) && (portnum < 63336)) {
          sprintf(command, "%s -e %s %s %d", xterm_str, access,
                  hostname, portnum);
      } else {
          sprintf(command, "%s -e %s %s", xterm_str, access, hostname);
      }
#else
      /*
       * For VMS we assume that the xterm_str is a Create/Terminal/NoDetach
       * which starts as a subprocess.
       */
      if (getenv("MULTINET_ROOT") || getenv("CMUTEK_ROOT") ||
          (getenv("TWG$TCP") && (!getenv("TELNET") ||
	   (*getenv("TELNET") != '$')))) {
          if ((login_protocol == tn3270) && !getenv("CMUTEK_ROOT")) {
              sprintf(command, "TELNET/TN3270 %s%s %s",
                      port ? "/PORT=" : "",
                      port ? port : "",
                      hostname);
          } else {
              sprintf(command, "%s%s%s %s", access,
                      port ? "/PORT=" : "",
                      port ? port : "",
                      hostname);
          }
      } else {
          sprintf(command, "%s %s %s", access, hostname,
                  port ? port : "");
      }
#endif /* VMS, BSN, GEC */
  }
  
#ifndef DISABLE_TRACE
  if (www2Trace)
      fprintf(stderr, "HTTelnet: Command is: %s\n", command);
#endif
#ifndef VMS
  run_a_command(command);
#else
  run_a_command(command, xterm_str);
#endif /* VMS, BSN */

  /* No need for application feedback if we're rlogging directly in... */
  if (user && login_protocol != rlogin) {
      char str[200];

      /* Sleep to let the xterm get up first.
       * Otherwise, the popup will get buried. */
      sleep(2);
      sprintf(str, "When you are connected, log in as '%s'.", user);
      application_user_feedback(str);
  }
  return HT_NO_DATA;		/* Ok - it was done but no data */
}

/*	"Load a document" -- establishes a session
**	------------------------------------------
**
** On entry,
**	addr	   must point to the fully qualified hypertext reference.
**
** On exit,
**	returns	   HT_NO_ACCESS	   Error.
**		   HT_NO_DATA	   Success.
**
*/
#ifndef VMS
PRIVATE int HTLoadTelnet (
#else
PUBLIC int HTLoadTelnet (
#endif /* VMS, BSN */
	WWW_CONST char *addr,
	HTParentAnchor *anchor,
	HTFormat format_out,
	HTStream *sink)
{
    char *access, *host;
    int status;
    
    if (sink) {
        HTAlert("Can't output a live session -- it has to be interactive");
	return HT_NO_ACCESS;
    }
    access =  HTParse(addr, "file:", PARSE_ACCESS);
    host = HTParse(addr, "", PARSE_HOST);
    status = remote_session(access, host);

    free(host);	
    free(access);
    return status;
}

PUBLIC HTProtocol HTTelnet = { "telnet", HTLoadTelnet, NULL };
PUBLIC HTProtocol HTRlogin = { "rlogin", HTLoadTelnet, NULL };
PUBLIC HTProtocol HTTn3270 = { "tn3270", HTLoadTelnet, NULL };
