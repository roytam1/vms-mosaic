/*		FILE WRITER				HTFWrite.h
**		===========
**
**	This version of the stream object just writes to a C file.
**	The file is assumed open and left open.
**
**	Bugs:
**		strings written must be less than buffer size.
*/
#include "../config.h"
#include <string.h>

#include "HTFWriter.h"

#include "HTFormat.h"
#include "HTAlert.h"
#include "HTFile.h"
#include "HText.h"
#include "tcp.h"
#include "../src/mosaic.h"
#ifdef VMS
#include <starlet.h>
#include "../src/child.h"
#endif
#include "HTCompressed.h"
#include "../src/gui-dialogs.h"
#include "../src/mo-www.h"

extern int loading_length;
extern int force_image_load;
extern char *image_file_fnam;
extern char *currentURL;

int imageViewInternal = 0;

#ifndef DISABLE_TRACE
extern int www2Trace;
#endif

/*		Stream Object
**		------------
*/

struct _HTStream {
	WWW_CONST HTStreamClass *isa;
	
	FILE *fp;
        char *fnam;
	char *end_command;
        int compressed;
        int interrupted;
        int write_error;
	char *mime_type;
};

/* We now pick up some external variables, handled in src/mo-www.c: */
extern int force_dump_to_file;
extern char *force_dump_filename;
/* If force_dump_to_file is high, we know we want to dump the
 *  data into a file already named by force_dump_filename and not
 *  do anything else. */

/* If this is high, then we just want to dump the thing to a file;
 * the file is named by force_dump_filename. */
extern int binary_transfer;

/*_________________________________________________________________________
**
**			A C T I O N 	R O U T I N E S
*/

#ifdef VMS
#include <descrip.h>
#define $NEW_DESCRIPTOR(name) \
   struct dsc$descriptor_s name = { \
            0, DSC$K_DTYPE_T, DSC$K_CLASS_S, 0}

typedef struct {
	char *command;
	int ret_status;
	} ExtCmd;

PRIVATE int system_vms_error ARGS1(XtPointer, cld)
{
  ExtCmd *cdata = (ExtCmd *)cld;
  char *str, *msg_str;
  unsigned short int string_end;
  extern mo_window *current_win;
  $NEW_DESCRIPTOR (msg_desc);

  msg_str = (char *)calloc(256, sizeof(char));
  msg_desc.dsc$w_length = 255;
  msg_desc.dsc$a_pointer = msg_str;
  sys$getmsg(cdata->ret_status, &string_end, &msg_desc, 15, 0);
  msg_str[string_end] = '\0';
  str = (char *)calloc(1024, sizeof(char));
  sprintf(str, "External viewer command:  %s\n\nExited with error:\n%s",
          cdata->command, msg_str);
  XmxMakeErrorDialog(current_win->base, str, "External Viewer Error");
  XtManageChild(Xmx_w);
  free(str);
  free(msg_str);

  free(cdata->command);
  free(cdata);

  /* Required to keep this routine from repeating */
  return 1;
}

PRIVATE void system_vms_exit ARGS2(ExtCmd *, cdata, int, pid)
{
  extern XtAppContext app_context; /* From GUI.C */

  if (cdata->ret_status != 1) {
    XtAppAddWorkProc(app_context, (XtWorkProc)system_vms_error,
		     (XtPointer)cdata);
  } else {
    free(cdata->command);
    free(cdata);
  }
}

/*
 * A VMS work-around: spawn a subprocess executing args.  This may be a
 * list of commands separated by " ; "
 * If the list ends with &, do not wait for completion.
 * Mimicks UNIX: (arg1 ; arg2 ...) &
 */
PRIVATE int system_VMS ARGS1(char *, args)
{
  FILE *fpc;
  char *fcname, *argsi, *cp;
  char cmd[64];
  static char null_dev[] = "NL:";
  $DESCRIPTOR(cmd_desc, NULL);
  $DESCRIPTOR(null_dev_desc, NULL);
  int status, flags, pid;
  ExtCmd *cdata;
  extern int child_count;

  cmd_desc.dsc$a_pointer = cmd;
  null_dev_desc.dsc$a_pointer = null_dev;
  fcname = mo_tmpnam((char *) 0);
  strcat(fcname, ".COM");

  fpc = fopen(fcname, "w");
  if (!fpc) {
    fprintf(stderr,
            "\nVMS scratch file open error: %s\n", strerror(errno, vaxc$errno));
    return(0);
  }

  argsi = strdup(args);
  if (cp = strstr(argsi, "&")) {
    flags = 1;
    *cp = '\0';
  } else {
    flags = 0;
  }

  cdata = (ExtCmd *) malloc(sizeof(ExtCmd));
  cdata->command = strdup(argsi);
  if (cp = strstr(cdata->command, "\n"))
    *cp = '\0';

  while (NULL != (cp = strstr(argsi, " ; "))) {
    cp++;
    *cp = '\n';
  }
  fprintf(fpc, "$ Set NoVerify\n");
  fprintf(fpc, "$ On Warning Then GoTo Error\n");
  fprintf(fpc, "$ Excode=1\n");
  fprintf(fpc, "%s", argsi);
  fprintf(fpc, "$End:\n");
  fprintf(fpc, "$ Delete$$/NoConfirm/NoLog %s;\n", fcname);
  fprintf(fpc, "$Exit Excode\n");
  fprintf(fpc, "$Error:\n");
  fprintf(fpc, "$ Excode=$status\n");
  fprintf(fpc, "$ Set Noon\n");
  fprintf(fpc, "$ GoTo End\n");
  fclose(fpc);

  sprintf(cmd, "@%s", fcname);
  cmd_desc.dsc$w_length = strlen(cmd);
  child_count = child_count + 1;

  sys$setast(0);
  status = lib$spawn(&cmd_desc, &null_dev_desc, &null_dev_desc, &flags, 0,
	             &pid, &cdata->ret_status, 0, ChildTerminated, child_count);
  AddChildProcessHandler(pid, system_vms_exit, cdata);
  sys$setast(1);

  free(fcname);
  free(argsi);

  return (status);
}

#endif /* VMS, BSN, GEC */

/*	Character handling
**	------------------
*/

PRIVATE void HTFWriter_put_character ARGS2(HTStream *, me, char, c)
{
  int rv;

  if (me->write_error)
      return;

  /* Make sure argument to putc is in range 0-255, to avoid weirdness
   * with rv == -1 == EOF when it's not supposed to. */
  rv = putc((int)(unsigned char)c, me->fp);

  if (rv == EOF) {
      HTProgress("Error writing to temporary file.");
      me->write_error = 1;
  }
}


/*	String handling
**	---------------
**
**	Strings must be smaller than this buffer size.
*/         
PRIVATE void HTFWriter_put_string ARGS2(HTStream *, me, WWW_CONST char *, s)
{
  int rv;

  if (me->write_error)
      return;

  rv = fputs(s, me->fp);
  if (rv == EOF) {
      HTProgress("Error writing to temporary file.");
      me->write_error = 1;
  }
}


/*	Buffer write.  Buffers can (and should!) be big.
**	------------
*/
PRIVATE void HTFWriter_write ARGS3(HTStream *, me, WWW_CONST char *, s, int, l)
{
  int rv;

  if (me->write_error)
      return;

  rv = fwrite(s, 1, l, me->fp); 
  if (rv != l) {
      HTProgress("Error writing to temporary file.");
      me->write_error = 1;
  }
}

static char *supportedTypes[] = {
        "image/bmp",
        "image/x-bmp",
        "image/x-ms-bmp",
        "image/gif",
        "image/jpeg",
        "image/pjpeg",
        "image/jpg",
        "image/png",
        "image/x-png",
        "image/x-pcd-jpeg",
        "image/x-pcd-jycc",
	"image/xpm",
	"image/xbm",
	"image/x-xbm",
	"image/xpixmap",
	"image/xbitmap",
	"image/x-xpixmap",
	"image/x-xbitmap",
        "\n"
};

PRIVATE int supportedImageType(char *mt)
{
    int i;

    if (!mt || !*mt)
	return(0);

    for (i = 0; supportedTypes[i][0] != '\n'; i++) {
	if (!strcmp(supportedTypes[i], mt))
	    return(1);
    }

    return(0);
}


/*	Free an HTML object
**	-------------------
**
**	Note that the SGML parsing context is freed, but the created
**	object is not,
**	as it takes on an existence of its own unless explicitly freed.
*/
PRIVATE void HTFWriter_free ARGS1(HTStream *, me)
{
  HText *text;
  static char *envbuf1 = NULL;
  static char *envbuf2 = NULL;
  char *tmp;

  /* I dunno if this is necessary... */
  if (me->interrupted) {
      free(me->fnam);
      free(me);
      return;
  }

  if (me->write_error) {
#ifndef VMS
      unlink(me->fnam);
#else
      remove(me->fnam);
#endif /* VMS, BSN */

      HTProgress("Insufficient temporary disk space; could not transfer data.");

      free(me->fnam);
      free(me);
      return;
  }

  fflush(me->fp);
  fclose(me->fp);

  /* We do want to be able to handle compressed inlined images,
   * but we don't want transparent uncompression to take place
   * in binary transfer mode.
   */
  if (!binary_transfer && (me->compressed != COMPRESSED_NOT)) {
#ifndef DISABLE_TRACE
      if (www2Trace)
          fprintf(stderr,
		  "[HTFWriter] Hi there; compressed is %d, fnam is '%s'\n",
                  me->compressed, me->fnam);
#endif
      HTCompressedFileToFile(me->fnam, me->compressed);
  }

  if (force_dump_to_file && !binary_transfer) {
      goto done;
  }

  /* Now, me->end_command can either be something starting with
     "<mosaic-internal-reference" or it can be a real command.
     Deal with appropriately. */
  if (me->end_command) {
	/* Check for forced dump condition.  The left paren comes
		from the construction of me->end_command as a compound shell
		command below. */
	if (strstr(me->end_command, "mosaic-internal-dump")) {
		rename_binary_file(me->fnam);
        } else if (!strstr(me->end_command, "mosaic-internal-reference")) {
		if (imageViewInternal && supportedImageType(me->mime_type)) {
			char *buf;
#ifndef VMS
			char *newHTML =
			    "<html>\n<head>\n<title>Mosaic's Internal Image Display</title>\n</head>\n<body>\n<center><img src=\"%s\">\n</body>\n</html>\n";

			buf = (char *)calloc((strlen(currentURL) +
			    strlen(newHTML) + 5), sizeof(char));
			sprintf(buf, newHTML, currentURL);
#else
			char *newHTML =
			    "<html>\n<head>\n<title>Mosaic's Internal Image Display</title>\n</head>\n<body>\n<center><img src=\"file://localhost/%s\">\n</body>\n</html>\n";

			buf = (char *)calloc((strlen(me->fnam) +
			    strlen(newHTML) + 5), sizeof(char));
			sprintf(buf, newHTML, me->fnam);
			/* Keep file name to delete later */
			image_file_fnam = strdup(me->fnam);
#endif
			text = HText_new();
			HText_beginAppend(text);
			HText_appendText(text, buf);
			HText_endAppend(text);

			free(buf);

			force_image_load = 1;	/* img.c will reset */
#ifndef DISABLE_TRACE
			if (www2Trace)
			    fprintf(stderr,
			       "[HTFWriter] Set up internal image viewer\n");
#endif
			goto done;
		}

		HTProgress("Spawning external viewer.");

#ifndef VMS   /* PGE, Skip setting environment variable for VMS */
		/*
		 * Have to dance around putenv since it makes "envbuf*" part
		 *   of the actual environment string...*sigh* What a mess!
		 */
		if (envbuf1) {
			envbuf2 = (char *)calloc((strlen(currentURL) +
						 strlen("MOSAIC_CURRENT_URL=") +
						 2),
					        sizeof(char));
			sprintf(envbuf2, "MOSAIC_CURRENT_URL=%s", currentURL);
			putenv(envbuf2);
			free(envbuf1);
			envbuf1 = NULL;
		} else if (envbuf2) {
			envbuf1 = (char *)calloc((strlen(currentURL) +
						 strlen("MOSAIC_CURRENT_URL=") +
						 2),
					        sizeof(char));
			sprintf(envbuf1, "MOSAIC_CURRENT_URL=%s", currentURL);
			putenv(envbuf1);
			free(envbuf2);
			envbuf2 = NULL;
		} else { /* Likely it is the first time */
			envbuf1 = (char *)calloc((strlen(currentURL) +
						 strlen("MOSAIC_CURRENT_URL=") +
						 2),
					        sizeof(char));
			sprintf(envbuf1, "MOSAIC_CURRENT_URL=%s", currentURL);
			putenv(envbuf1);
		}
#endif   /* VMS, PGE */

#ifndef VMS
		system(me->end_command);
#else
		/* Add '&' so spawn doesn't wait; avoids hangs */
		tmp = malloc(strlen(me->end_command) + 2);
		tmp = strdup(me->end_command);
		tmp = strcat(tmp, "&");
		system_VMS(tmp);
		free(tmp);
#endif

#ifndef VMS   /* PGE, Skip setting environment variable for VMS */
		if (envbuf1) {
			envbuf2 = (char *)calloc((strlen("MOSAIC_CURRENT_URL=")+
						  2),
					         sizeof(char));
			sprintf(envbuf2, "MOSAIC_CURRENT_URL=");
			putenv(envbuf2);
			free(envbuf1);
			envbuf1 = NULL;
		} else if (envbuf2) {
			envbuf1 = (char *)calloc((strlen("MOSAIC_CURRENT_URL=")+
						  2),
					         sizeof(char));
			sprintf(envbuf1, "MOSAIC_CURRENT_URL=");
			putenv(envbuf1);
			free(envbuf2);
			envbuf2 = NULL;
		} else { /* Likely it is the first time */
			envbuf1 = (char *)calloc((strlen("MOSAIC_CURRENT_URL=")+
						  2),
					         sizeof(char));
			sprintf(envbuf1, "MOSAIC_CURRENT_URL=");
			putenv(envbuf1);
		}
#endif   /* VMS, PGE */
	} else {
          /* Internal reference, aka HDF file.  Just close output file. */
	}
    } else {
      /* No me->end_command; just close the file. */
    }

  /* Construct dummy HText thingie so Mosaic knows
   * not to try to access this "document". */
  text = HText_new();
  HText_beginAppend(text);
  /* If it's a real internal reference, tell Mosaic. */
  if (me->end_command) {
      if (strstr(me->end_command, "mosaic-internal-reference")) {
          HText_appendText(text, me->end_command);
      } else {
	  HText_appendText(text, "<mosaic-access-override>\n");
      }
      free(me->end_command);
  } else {
      /* No me->end_command; just override the access. */
      HText_appendText(text, "<mosaic-access-override>\n");
  }
  HText_endAppend(text);

 done:
  if (binary_transfer)
      rename_binary_file(me->fnam);

  free(me->fnam);
  if (me->mime_type)
      free(me->mime_type);
  free(me);

  return;
}

/*	End writing
*/

PRIVATE void HTFWriter_end_document ARGS1(HTStream *, me)
{
  if (me->interrupted || me->write_error)
      return;

  fflush(me->fp);
}

PRIVATE void HTFWriter_handle_interrupt ARGS1(HTStream *, me)
{
  if (me->write_error)
      goto outtahere;

  /* Close the file, then kill it. */
  fclose(me->fp);

#ifndef VMS
  unlink(me->fnam);
#else
  remove(me->fnam);
#endif /* VMS, BSN */

#ifndef DISABLE_TRACE
  if (www2Trace)
      fprintf(stderr, "*** HTFWriter interrupted; killed '%s'\n", me->fnam);
#endif
  
 outtahere:
  me->interrupted = 1;

  return;
}


/*	Structured Object Class
**	-----------------------
*/
PRIVATE WWW_CONST HTStreamClass HTFWriter = /* As opposed to print etc */
{		
	"FileWriter",
	HTFWriter_free,
	HTFWriter_end_document,
	HTFWriter_put_character, 	HTFWriter_put_string,
	HTFWriter_write,
        HTFWriter_handle_interrupt
}; 


/*	Take action using a system command
**	----------------------------------
**
**	Creates temporary file, writes to it, executes system command
**	on end-document.  The suffix of the temp file can be given
**	in case the application is fussy, or so that a generic opener can
**	be used.
**
**      WARNING: If force_dump_to_file is high, pres may be NULL
**      (as we may get called directly from HTStreamStack).
*/
PUBLIC HTStream* HTSaveAndExecute ARGS5(
	HTPresentation *,	pres,
	HTParentAnchor *,	anchor,	/* Not used */
	HTStream *,		sink,
        HTFormat,               format_in,
        int,                    compressed)	/* Not used */
{
  char *command;
  WWW_CONST char *suffix;
  HTStream *me;

  me = (HTStream *)malloc(sizeof(*me));
  me->isa = &HTFWriter;  
  me->interrupted = 0;
  me->write_error = 0;
  me->fnam = NULL;
  me->end_command = NULL;
  me->compressed = compressed;
  if (!format_in || !format_in->name || !*(format_in->name)) {
      me->mime_type = NULL;
  } else {
      if (!strncmp(format_in->name, "image", 5)) {
	  me->mime_type = strdup(format_in->name);
      } else {
	  me->mime_type = NULL;
      }
  }

#ifndef DISABLE_TRACE
  if (www2Trace)
    fprintf(stderr, "[HTSaveAndExecute] me->compressed is '%d'\n",
            me->compressed);
#endif
  
  /* Save the file under a suitably suffixed name */
  if (!force_dump_to_file) {
      suffix = HTFileSuffix(pres->rep);
      
      me->fnam = mo_tmpnam(anchor->address);
      if (suffix) {
          char *freeme = me->fnam;
         
          me->fnam = (char *)malloc(strlen(me->fnam) + strlen(suffix) + 8);
          strcpy(me->fnam, freeme);
          strcat(me->fnam, suffix);
          free(freeme);
      }
  } else {
      me->fnam = strdup(force_dump_filename);
  }

#ifdef VMS
  if (loading_length != -1) {
      char alq[64];

      sprintf(alq, "alq = %d", (loading_length + 511) / 512);
      me->fp = fopen(me->fnam, "w", "shr = get", "rop = WBH", "mbf = 4",
                     "mbc = 32", "deq = 16", alq, "fop=tef");
  } else {
      me->fp = fopen(me->fnam, "w", "shr = get", "rop = WBH", "mbf = 4",
                     "mbc = 32", "deq = 16", "fop = tef");
  }
#else
  me->fp = fopen(me->fnam, "w");
#endif
  if (!me->fp) {
      HTProgress("Can't open temporary file -- serious problem.");
      me->write_error = 1;
      return me;
  }

  /* If force_dump_to_file is high, we're done here. */
  if (!force_dump_to_file) {
      if (!strstr(pres->command, "mosaic-internal-reference")) {
          /* If there's a "%s" in the command, or if the command is magic... */
#ifndef DISABLE_TRACE
          if (www2Trace)
              fprintf(stderr, "HTFWriter: pres->command is '%s'\n",
                      pres->command);
#endif
          if (strstr(pres->command, "%s") ||
              strstr(pres->command, "mosaic-internal")) {
              /* Make command to process file */
              command = (char *)malloc(
			   (strlen(pres->command) + 10 + 3 * strlen(me->fnam)) *
                           sizeof(char));
              
              /* Cute.  pres->command will be something like "xv %s"; me->fnam
               * gets filled in as many times as appropriate.  */
              sprintf(command, pres->command, me->fnam, me->fnam, me->fnam);
              
              me->end_command = (char *)malloc(
				     (strlen(command) + 32 + strlen(me->fnam)) *
				     sizeof(char));
#ifndef VMS
              sprintf(me->end_command, "(%s ; /bin/rm -f %s) &",
#else
              sprintf(me->end_command, "$ %s\n$ErrEnd:\n$ Delete$$ %s;\n&",
#endif /* VMS, BSN, GEC */
                      command, me->fnam);

              free(command);
          } else {
              /* Make command to process file -- but we have to cat
               * to the viewer's stdin. */
              me->end_command = (char *)malloc(
                         (strlen(pres->command) + 64 + (2 * strlen(me->fnam))) *
                         sizeof(char));
#ifndef VMS
              sprintf(me->end_command, "((cat %s | %s); /bin/rm -f %s) &",
                      me->fnam, pres->command, me->fnam);
#else
              HTProgress("Too bad, you entered a part not ported to VMS.");
#endif /* VMS, not ported, BSN */
          }
      } else {
          /* Overload me->end_command to be what we should write out as text
           * to communicate back to client code. */
          me->end_command = (char *)malloc(strlen("mosaic-internal-reference") +
					   strlen(me->fnam) + 32);
#ifndef VMS
          sprintf(me->end_command, "<%s \"%s\">\n",
		  "mosaic-internal-reference", me->fnam);
#else
          HTProgress("Too bad, you entered another part not ported to VMS.");
#endif /* VMS, not ported, BSN */
      }
  }
  
  return me;
}
