#include "../config.h"
#ifndef VMS   /* PGE */
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#endif

#include "HTFormat.h"
#include "HTFile.h"
#include "HTUtils.h"
#include "tcp.h"
#include "HText.h"
#include "HTAlert.h"
#include "HTInit.h"
#include "HTFWriter.h"
#include "HTCompressed.h"

#include "../libnut/system.h"

#ifndef DISABLE_TRACE
extern int www2Trace;
#endif

int is_uncompressed = 0;

extern char *uncompress_program;
extern char *gunzip_program;

/* Given a filename of a local compressed file, compress it in place.
 *
 * We assume that the file does not already have a .Z or .z extension
 * at this point -- this is a little weird but it's convenient.
 */
void HTCompressedFileToFile (char *fnam, int compressed)
{
  char *znam, *cmd;
#ifdef VMS
  char *cund, *cp, *cnam;
  int  nund;
#endif /* VMS, BSN */

#ifndef DISABLE_TRACE
  if (www2Trace)
      fprintf(stderr,
	      "[HTCompressedFileToFile] Entered; fnam '%s', compressed %d\n",
              fnam, compressed);
#endif

  /* Punt if we can't handle it. */
  if (compressed != COMPRESSED_BIGZ && compressed != COMPRESSED_GNUZIP)
      return;

  HTProgress("Preparing to uncompress data.");
  
  znam = (char *)malloc((strlen(fnam) + 8) * sizeof(char));

  /* Either compressed or gzipped. */
#ifndef VMS   /* PGE, VMS filenames don't allow multiple '.'s */
  if (compressed == COMPRESSED_BIGZ) {
      sprintf(znam, "%s.Z", fnam);
  } else {
      sprintf(znam, "%s.gz", fnam);
  }
#else
  /*
   * Check if the original file already had an extension and add gz or Z
   * extension in appropriate fashion.
   */
  cnam = fnam;  
  cp = strchr(fnam, ']');
  if (cnam < cp)
      cnam = cp;
  cp = strchr(fnam, ':');
  if (cnam < cp)
      cnam = cp;
  if (cund = strchr(cnam, '.')) {
      nund = -1;
      if (compressed == COMPRESSED_BIGZ) {
          sprintf(znam, "%s_Z", fnam);
      } else {
          sprintf(znam, "%s-gz", fnam);
      }
  } else {
      nund = -1;
      if (compressed == COMPRESSED_BIGZ) {
          sprintf(znam, "%s._Z", fnam);
      } else {
          sprintf(znam, "%s.-gz", fnam);
      }
  }
#endif /* VMS, BSN */

  /* New "mv" function to take care of these /bin/mv things */
  {
      char retBuf[BUFSIZ];

      if (my_move(fnam, znam, retBuf, BUFSIZ, 1) != SYS_SUCCESS) {
	  sprintf(retBuf,
	   "Unable to uncompress compressed data;\nresults may be in error.\n%s",
	   retBuf);
	  application_user_info_wait(retBuf);
	  free(znam);
	  return;
      }
  }

#ifndef DISABLE_TRACE
  if (www2Trace)
      fprintf(stderr, "[HTCompressedFileToFile] Moved '%s' to '%s'\n",
              fnam, znam);
#endif

  if (compressed == COMPRESSED_BIGZ) {
      cmd = (char *)malloc(strlen(uncompress_program) + strlen(znam) + 8);
      sprintf(cmd, "%s %s", uncompress_program, znam);
  } else {
      cmd = (char *)malloc(strlen(gunzip_program) + strlen(znam) + 8);
      sprintf(cmd, "%s %s", gunzip_program, znam);
  }

  HTProgress("Uncompressing data.");

  {
      int status;
      int skip_output = 0;
      char retBuf[BUFSIZ], final[BUFSIZ];

      *retBuf = '\0';

      if ((status = my_system(cmd, retBuf, BUFSIZ)) != SYS_SUCCESS) {
	  char *msg;

	  switch(status) {
	      case SYS_NO_COMMAND:
		  msg = "There was no command to execute.";
		  break;
	      case SYS_FORK_FAIL:
		  msg = "The fork call failed.";
		  break;
	      case SYS_PROGRAM_FAILED:
		  msg = "The program specified was not able to run.";
		  break;
	      case SYS_NO_RETBUF:
		  msg = "There was no return buffer.\n";
		  break;
	      case SYS_FCNTL_FAILED:
		  msg = "Fcntl failed to set non-block on the pipe.";
		  break;
	      default:
		  msg = "Unexpected failure.";
	  }
	  /* Give them the output */
	  if (*retBuf) {
	      sprintf(final, "%s\n%s", msg, retBuf);
	  } else {
	      sprintf(final, "%s", msg);
	  }
      } else if (*retBuf) {
	  /* Give them the output */
	  sprintf(final, "%s", retBuf);
      } else {
	  /* Okay */
	  skip_output = 1;
      }
      if (!skip_output) {
	  application_user_info_wait(final);
	  free(cmd);
	  free(znam);
	  HTProgress("Uncompress failed.");
	  return;
      }
  }

  HTProgress("Data uncompressed.");

  is_uncompressed = 1;

#ifndef DISABLE_TRACE
  if (www2Trace)
      fprintf(stderr,
	      "[HTCompressedFileToFile] Uncompressed '%s' with command '%s'\n",
              znam, cmd);
#endif
  
#ifdef VMS
  if (nund != -1) {
      char retBuf[BUFSIZ];

      znam[strlen(fnam)] = '\0';
      if (my_move(znam, fnam, retBuf, BUFSIZ, 1) != SYS_SUCCESS) {
	  sprintf(retBuf,
	     "Unable to rename uncompressed data file;\nresults may be in error.\n%s",
	     retBuf);
	  application_user_feedback(retBuf);
      }
  }
#endif /* BSN, modified by PGE */
  free(cmd);
  free(znam);

  return;
}


void HTCompressedHText (HText *text, int compressed, int plain)
{
  char *fnam;
  FILE *fp;
  int rv, size_of_data;
  
#ifndef DISABLE_TRACE
  if (www2Trace)
      fprintf(stderr, "[HTCompressedHText] Entered; compressed %d\n",
              compressed);
#endif

  /* Punt if we can't handle it. */
  if (compressed != COMPRESSED_BIGZ && compressed != COMPRESSED_GNUZIP)
      return;

  /* Hmmmmmmmmm, I'm not sure why we subtract 1 here, but it is
   * indeed working... */
  size_of_data = HText_getTextLength(text) - 1;

  if (size_of_data == 0) {
#ifndef DISABLE_TRACE
      if (www2Trace)
          fprintf(stderr, "[HTCompressedHText] size_of_data 0; punting\n");
#endif
      return;
  }
  
  fnam = mo_tmpnam(NULL);
#ifdef VMS
  /* Open file for efficient writes, VaxC RMS defaults are pitiful. PGE */
  fp = fopen(fnam, "w", "shr = nil", "rop = WBH", "mbf = 4",
             "mbc = 32", "deq = 16", "alq = 32", "fop = tef");
#else
  fp = fopen(fnam, "w");
#endif /* VMS, GEC for PGE */
  if (!fp) {
#ifndef DISABLE_TRACE
      if (www2Trace)
          fprintf(stderr, "Could not open temp file '%s'\n", fnam);
#endif
      application_user_feedback(
	     "Unable to uncompress compressed data;\nresults may be in error.");
      free(fnam);
      return;
  }

#ifndef DISABLE_TRACE
  if (www2Trace)
      fprintf(stderr, "[HTCompressedHText] Going to write %d bytes.\n",
              size_of_data);
#endif
  rv = fwrite(HText_getText(text), sizeof(char), size_of_data, fp);
  if (rv != size_of_data) {
#ifndef DISABLE_TRACE
      if (www2Trace)
          fprintf(stderr, "Only wrote %d bytes\n", rv);
#endif
      application_user_feedback(
          "Unable to write compressed data to disk;\nresults may be in error.");
  }
  fclose(fp);

#ifndef DISABLE_TRACE
  if (www2Trace)
      fprintf(stderr, "HTCompressedHText: Calling CompressedFileToFile\n");
#endif
  HTCompressedFileToFile(fnam, compressed);

  HText_clearOutForNewContents(text);

  HText_beginAppend(text);
  
  if (plain) {
#ifndef DISABLE_TRACE
      if (www2Trace)
          fprintf(stderr,
	          "[HTCompressedHText] Throwing in PLAINTEXT token...\n");
#endif
      HText_appendText(text, "<PLAINTEXT>\n");
  }

  if (!(fp = fopen(fnam, "r"))) {
#ifndef DISABLE_TRACE
      if (www2Trace)
          fprintf(stderr, "Could not open temp file for reading '%s'\n", fnam);
#endif
      /* We already get error dialog up above. */
      free(fnam);
      return;
  }

  HTFileCopyToText(fp, text);

#ifndef DISABLE_TRACE
  if (www2Trace)
      fprintf(stderr, "[HTCompressedHText] I think we're done...\n");
#endif

#ifndef VMS
  unlink(fnam);
#else
  remove(fnam);
#endif /* VMS, BSN */
  
  return;
}
