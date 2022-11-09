/*		Manage different file formats			HTFormat.c
**		=============================
**
*/
#include "../config.h"

#include "HTMIME.h"
#include "HTFormat.h"

#ifdef HAVE_SSL
#include <openssl/ssl.h>
#endif

PUBLIC float HTMaxSecs = 1e10;		/* No effective limit */
PUBLIC float HTMaxLength = 1e10;	/* No effective limit */

#include "HTTCP.h"
#include "HText.h"
#include "HTAlert.h"
#include "HTList.h"
#include "HTInit.h"
#include "HTFWriter.h"
#include "HTPlain.h"
#include "SGML.h"

/* From gui-documents.c. */
extern int loading_inlined_images;

#ifndef DISABLE_TRACE
extern int www2Trace;
#endif

/* If set, HTCopy does just one read per call */
PUBLIC int HTCopyOneRead = 0;

extern WWW_CONST HTStreamClass HTMIME;

struct _HTStream {
      WWW_CONST HTStreamClass *isa;
      /* ... */
};

/*	Presentation methods
**	--------------------
*/

PUBLIC HTList *HTPresentations = NULL;
PUBLIC HTPresentation *default_presentation = NULL;


/*	Define a presentation system command for a content-type
**	-------------------------------------------------------
*/
PUBLIC void HTSetPresentation (WWW_CONST char *representation,
			       WWW_CONST char *command,
			       float quality,
			       float secs, 
			       float secs_per_byte)
{
    HTPresentation *pres = (HTPresentation *)malloc(sizeof(HTPresentation));
    static init = 0;
    static HTAtom *www_present;
    
    if (!init) {
	www_present = WWW_PRESENT;
	init = 1;
    }
    pres->rep = HTAtom_for(representation);
    pres->rep_out = www_present;		/* Fixed for now ... :-) */
    pres->converter = HTSaveAndExecute;		/* Fixed for now ...     */
    pres->quality = quality;
    pres->secs = secs;
    pres->secs_per_byte = secs_per_byte;
    pres->command = NULL;
    StrAllocCopy(pres->command, command);
    
    if (!HTPresentations)
	HTPresentations = HTList_new();
    
    if (!strcmp(representation, "*")) {
        if (default_presentation)
	    free(default_presentation);
	default_presentation = pres;
    } else {
        HTList_addObjectAtEnd(HTPresentations, pres);
    }
}


/*	Define a built-in function for a content-type
**	---------------------------------------------
*/
PUBLIC void HTSetConversion (WWW_CONST char *representation_in,
			     WWW_CONST char *representation_out,
			     HTConverter *converter,
			     float quality,
			     float secs,
			     float secs_per_byte)
{
    HTPresentation *pres = (HTPresentation *)malloc(sizeof(HTPresentation));
    
    pres->rep = HTAtom_for(representation_in);
    pres->rep_out = HTAtom_for(representation_out);
    pres->converter = converter;
    pres->command = NULL;		/* Fixed */
    pres->quality = quality;
    pres->secs = secs;
    pres->secs_per_byte = secs_per_byte;
    
    if (!HTPresentations)
	HTPresentations = HTList_new();
    
    if (!strcmp(representation_in, "*")) {
        if (default_presentation)
	    free(default_presentation);
	default_presentation = pres;
    } else {
        HTList_addObject(HTPresentations, pres);
    }
}


/*
** Remove a conversion routine from the presentation list.
** The conversion routine must match up with the given args.
*/
PUBLIC void HTRemoveConversion (WWW_CONST char *representation_in,
				WWW_CONST char *representation_out,
				HTConverter *converter)
{
    int numberOfPresentations = HTList_count(HTPresentations);
    HTPresentation *pres;
    HTAtom *rep_in = HTAtom_for(representation_in);
    HTAtom *rep_out = HTAtom_for(representation_out);
    int x;

    for (x = 0; x < numberOfPresentations; x++) {
        pres = HTList_objectAt(HTPresentations, x);
	if (pres && !strcmp(pres->rep->name, rep_in->name) &&
	    !strcmp(pres->rep_out->name, rep_out->name) &&
	    (pres->converter == converter))
	    HTList_removeObject(HTPresentations, pres);
    }
}


/*	File buffering
**	--------------
**
**	The input file is read using the macro which can read from
**	a socket or a file.
**	The input buffer size, if large will give greater efficiency and
**	release the server faster, and if small will save space on PCs, etc.
*/
#ifndef VMS
#define INPUT_BUFFER_SIZE 65536
#else
#define INPUT_BUFFER_SIZE 16384
#endif /* VMS, BSN */
PRIVATE char input_buffer[INPUT_BUFFER_SIZE];
PRIVATE char *input_pointer;
PRIVATE char *input_limit;
PRIVATE int input_file_number;


/*	Set up the buffering
**
**	These routines are public because they are in fact needed by
**	many parsers, and on PCs and Macs we should not duplicate
**	the static buffer area.
*/
PUBLIC void HTInitInput (int file_number)
{
    input_file_number = file_number;
    input_pointer = input_limit = input_buffer;
}

PUBLIC int interrupted_in_htgetcharacter = 0;

PUBLIC char HTGetCharacter (void)
{
  char ch;
  int status;

  interrupted_in_htgetcharacter = 0;

  do {
      if (input_pointer >= input_limit) {
          status = NETREAD(input_file_number, input_buffer, INPUT_BUFFER_SIZE);
          if (status <= 0)  {
              if (!status) 
                  return (char)EOF;
              if (status == HT_INTERRUPTED) {
#ifndef DISABLE_TRACE
                  if (www2Trace)
                      fprintf(stderr,
			      "HTFormat: Interrupted in HTGetCharacter\n");
#endif
                  interrupted_in_htgetcharacter = 1;
                  return (char)EOF;
              }
#ifndef DISABLE_TRACE
              if (www2Trace) 
                  fprintf(stderr, "HTFormat: File read error %d\n", status);
#endif
              return (char)EOF;
          }
          input_pointer = input_buffer;
          input_limit = input_buffer + status;
      }
      ch = *input_pointer++;
  } while (ch == (char) 13);  /* Ignore ASCII carriage return */

  return ch;
}

#ifdef NEED_HTOutputBinary
/*	Stream the data to an ouput file as binary
*/
PUBLIC int HTOutputBinary (int input, FILE *output)
{
  do {
      int status = NETREAD(input, input_buffer, INPUT_BUFFER_SIZE);

      if (status <= 0) {
          if (!status) 
              return 0;
#ifndef DISABLE_TRACE
          if (www2Trace)
	      fprintf(stderr, "HTFormat: File read error %d\n", status);
#endif
          return 2;			/* Error */
      }
#ifndef VMS
      fwrite(input_buffer, sizeof(char), status, output);
#else
      if (fwrite(input_buffer, sizeof(char), status, output) != status) {
          char *str = (char *)malloc(128 * sizeof(char));

          sprintf(str, "VMS I/O error in HTOutputBinary:\n%s\0",
                  strerror(errno, vaxc$errno));
          application_user_feedback(str);
          free(str);
      }
#endif /* VMS, BSN */
  } while (YES);
}
#endif


static int partial_wildcard_matches (HTFormat r1, HTFormat r2)
{
  /* r1 is the presentation format we're currently looking at out
   * of the list we understand.  r2 is the one we need to get to. */
  char *s1, *s2;
  char *subtype1 = NULL;
  char *subtype2 = NULL;
  int res = 0;
  int i;

  if (!r1 || !r2) {
      fprintf(stderr, "Bug detected by partial_wildcard_matches\n");
      return res;
  }
  s1 = HTAtom_name(r1);
  s2 = HTAtom_name(r2);

  if (!s1 || !s2)
      return res;
  
  s1 = strdup(s1);
  s2 = strdup(s2);

  for (i = 0; i < strlen(s1); i++) {
      if (s1[i] == '/') {
          s1[i] = '\0';
          subtype1 = &s1[i + 1];
          /* Now s1 contains the main type and subtype1 contains
           * the subtype. */
          break;
      }
  }
  
  /* Bail if we don't have a wildcard possibility. */
  if (!subtype1 || (*subtype1 != '*'))
      goto done;

  for (i = 0; i < strlen(s2); i++) {
      if (s2[i] == '/') {
          s2[i] = '\0';
          subtype2 = &s2[i + 1];
          /* Now s2 contains the main type and subtype2 contains
           * the subtype. */
          break;
      }
  }

  /* Bail if s1 and s2 aren't the same and s1[0] isn't '*'. */
  if (!subtype2 || (strcmp(s1, s2) && (*s1 != '*')))
      goto done;

  /* OK, so now either we have the same main types or we have a wildcard
   * type for s1.  We also know that we have a wildcard possibility in
   * s1.  Therefore, at this point, we have a match. */
  res = 1;

 done:
  free(s1);
  free(s2);
  return res;
}
  

/*		Create a filter stack
**		---------------------
**
**	If a wildcard match is made, a temporary HTPresentation
**	structure is made to hold the destination format while the
**	new stack is generated.  This is just to pass the out format to
**	MIME so far.  Storing the format of a stream in the stream might
**	be a lot neater.
*/
PUBLIC HTStream *HTStreamStack (HTFormat format_in,
				HTFormat rep_out,
				int compressed,
				HTStream *sink,
				HTParentAnchor *anchor)
{
  HTPresentation temp;
  static HTAtom *wildcard, *www_mime, *www_source;
  static int init = 0;

  /* Inherit force_dump_to_file from mo-www.c. */
  extern int force_dump_to_file;

  if (!init) {
      wildcard = HTAtom_for("*");
      www_mime = WWW_MIME;
      www_source = WWW_SOURCE;
      init = 1;
  }

  if (!format_in) {
      fprintf(stderr, "Bug detected by HTStreamStack\n");
      return NULL;
  }
#ifndef DISABLE_TRACE
  if (www2Trace) {
      fprintf(stderr,
              "[HTStreamStack] Constructing stream stack for %s to %s\n",
              HTAtom_name(format_in), HTAtom_name(rep_out));
      fprintf(stderr, "               Compressed is %d\n", compressed);
  }
#endif
    
  if (rep_out == www_source || rep_out == format_in) {
#ifndef DISABLE_TRACE
      if (www2Trace)
          fprintf(stderr,
            "[HTStreamStack] rep_out = WWW_SOURCE || format_in; return sink\n");
#endif
      return sink;
  }
  
  if (!HTPresentations) 
      HTFormatInit();	/* Set up the list */
  
  if (force_dump_to_file && (format_in != www_mime))
      return HTSaveAndExecute(NULL, anchor, sink, format_in, compressed);

  {
    int n = HTList_count(HTPresentations);
    int i;
    HTPresentation *pres;

    for (i = 0; i < n; i++) {
        pres = HTList_objectAt(HTPresentations, i);
#ifndef DISABLE_TRACE
        if (www2Trace) {
            fprintf(stderr, "HTFormat: looking at pres '%s'\n",
                    HTAtom_name(pres->rep));
            if (pres->command) {
                fprintf(stderr, "HTFormat: pres->command is '%s'\n",
                        pres->command);
            } else {
                fprintf(stderr, "HTFormat: pres->command doesn't exist\n");
	    }
        }
#endif
        if (pres->rep == format_in ||
            partial_wildcard_matches(pres->rep, format_in)) {
            if (pres->command &&
		strstr(pres->command, "mosaic-internal-present")) {
#ifndef DISABLE_TRACE
                if (www2Trace)
                    fprintf(stderr, "[HTStreamStack] Found internal-present\n");
#endif
                return HTPlainPresent(pres, anchor, sink, format_in,
				      compressed);
            }
            if (pres->rep_out == rep_out) {
#ifndef DISABLE_TRACE
                if (www2Trace)
                    fprintf(stderr,
                            "[HTStreamStack] pres->rep_out = rep_out\n");
#endif
                return
		  (*pres->converter)(pres, anchor, sink, format_in, compressed);
            }
            if (pres->rep_out == wildcard) {
#ifndef DISABLE_TRACE
                if (www2Trace)
                    fprintf(stderr,
                            "[HTStreamStack] pres->rep_out = wildcard\n");
#endif
                temp = *pres;  /* Make temp conversion to needed fmt */
                temp.rep_out = rep_out;		/* yuk */
                return
		 (*pres->converter)(&temp, anchor, sink, format_in, compressed);
            }
        }
    }
  }
#ifndef DISABLE_TRACE
  if (www2Trace)
      fprintf(stderr, "[HTStreamStack] Returning NULL at bottom.\n");
#endif
  
  return NULL;
}
	

/*		Find the cost of a filter stack
**		-------------------------------
**
**	Must return the cost of the same stack which StreamStack would set up.
**
** On entry,
**	length	The size of the data to be converted
*/
PUBLIC float HTStackValue (HTFormat format_in,
			   HTFormat rep_out,
			   float initial_value,
			   long int length)
{
    static HTAtom *wildcard, *www_source;
    static int init = 0;

    if (!init) {
        wildcard = HTAtom_for("*");
        www_source = WWW_SOURCE;
        init = 1;
    }
#ifndef DISABLE_TRACE
    if (www2Trace)
	fprintf(stderr,
    	        "HTFormat: Evaluating stream stack for %s worth %.3f to %s\n",
	        HTAtom_name(format_in), initial_value, HTAtom_name(rep_out));
#endif
    if (rep_out == www_source || rep_out == format_in)
	return 0.0;

    if (!HTPresentations)
	HTFormatInit();	 /* Set up the list */

    {
	int n = HTList_count(HTPresentations);
	int i;
	HTPresentation *pres;

	for (i = 0; i < n; i++) {
	    pres = HTList_objectAt(HTPresentations, i);
	    if (pres->rep == format_in &&
	    	(pres->rep_out == rep_out || pres->rep_out == wildcard)) {
	        float value = initial_value * pres->quality;

		if (HTMaxSecs != 0.0)
		    value -= (length * pres->secs_per_byte + pres->secs) /
								      HTMaxSecs;
		return value;
	    }
	}
    }
    return -1e30;		/* Really bad */
}
	

/*	Push data from a socket down a stream
**	-------------------------------------
**
**   This routine is responsible for creating and PRESENTING any
**   graphic (or other) objects described by the file.
**
**   The file number given is assumed to be a TELNET stream containing
**   CRLF at the end of lines which need to be stripped to LF for unix
**   when the format is textual.
**
**   Returns negative for error or total read (or zero if read enough)
*/
PUBLIC int HTCopy (int file_number,
                   HTStream *sink,
                   int bytes_already_read,
		   void *handle,
		   int loading_length)
{
  HTStreamClass targetClass;    
  char line[256];
  char *msg;
  extern int twirl_increment;
  int next_twirl = twirl_increment;
  int total_read = bytes_already_read;
  int rv;
  int left = -1;
  int hdr_done = 0;
  int hdr_len, status, intr, is_mime;

  /*
   * Push the data down the stream
   */
  targetClass = *sink->isa;	/* Copy pointers to procedures */
  
  /* If MIME, get lengths. */
  if (sink->isa == &HTMIME) {
      hdr_len = HTMIME_get_header_length(sink);
      loading_length = HTMIME_get_content_length(sink);
      is_mime = 1;
      if (HTMIME_header_done(sink))
	  hdr_done = 1;
  } else {
      hdr_len = 0;
      is_mime = 0;
  }

  /* Push binary from socket down sink */
  for (;;) {
      if (total_read > next_twirl) {
          intr = HTCheckActiveIcon(1);
          next_twirl += twirl_increment;
      } else {
          intr = HTCheckActiveIcon(0);
      }
      if (intr) {
          HTProgress("Data transfer interrupted.");
	  HTMeter(100, NULL);
          (*targetClass.handle_interrupt)(sink);
          rv = -1;
          goto ready_to_leave;
      }
      if (loading_length == -1) {
	  left = -1;
#ifdef HAVE_SSL
	  if (handle) {
	      status = SSL_read((SSL *)handle, input_buffer, INPUT_BUFFER_SIZE);
	  } else {
	      status = NETREAD(file_number, input_buffer, INPUT_BUFFER_SIZE);
	  }
#else
	  status = NETREAD(file_number, input_buffer, INPUT_BUFFER_SIZE);
#endif
      } else {
	  left = (loading_length + hdr_len) - total_read;
	  if (left > 0) {
#ifdef HAVE_SSL
	      if (handle) {
		  status = SSL_read((SSL *)handle, input_buffer, 
                                    (left > INPUT_BUFFER_SIZE ?
                                     INPUT_BUFFER_SIZE : left));
	      } else {
		  status = NETREAD(file_number, input_buffer, 
                                   (left > INPUT_BUFFER_SIZE ?
                                    INPUT_BUFFER_SIZE : left));
	      }
#else
	      status = NETREAD(file_number, input_buffer, 
                               (left > INPUT_BUFFER_SIZE ?
                                INPUT_BUFFER_SIZE : left));
#endif
          } else {
	      status = 0;
	  }
      }
      if (status > 0) {
	  total_read += status;
      } else if (status < 0) {
          if (status == HT_INTERRUPTED) {
              HTProgress("Data transfer interrupted.");
	      HTMeter(100, NULL);
              (*targetClass.handle_interrupt)(sink);
              rv = -1;
              goto ready_to_leave;
          }
#ifndef MULTINET
          if (errno == ENOTCONN || errno == ECONNRESET || errno == EPIPE) {
#else
          if (socket_errno == ENOTCONN || socket_errno == ECONNRESET ||
              socket_errno == EPIPE) {
#endif /* MULTINET, BSN */
              /* Arrrrgh, HTTP 0/1 compatibility problem, maybe. */
              rv = -2;
              goto ready_to_leave;
          }
          break;
      } else {
	  /* status is zero */
	  total_read = 0;	/* Return zero to signal done to HTMultiLoad */
	  break;
      }

      /* Got at least one byte */
      (*targetClass.put_block)(sink, input_buffer, status);

      /* May not have lengths yet due to long headers */
      if (is_mime && !hdr_done && HTMIME_header_done(sink)) {
	  hdr_done = 1;
          hdr_len = HTMIME_get_header_length(sink);
	  loading_length = HTMIME_get_content_length(sink);
      }

      /* Moved msg stuff here as loading_length may change midstream */
      if (loading_length == -1) {
	  msg = loading_inlined_images ? 
		"Read %d bytes of inlined image data." : 
		"Read %d bytes of data.";
          sprintf(line, msg, total_read);
      } else {
	  msg = loading_inlined_images ? 
		"Read %d of %d bytes of inlined image data." : 
		"Read %d of %d bytes of data.";
          sprintf(line, msg, total_read, loading_length + hdr_len);
	  if (total_read > 10000000) {
	      /* Avoid Integer overflow */
	      HTMeter(total_read / ((loading_length + hdr_len) / 100), NULL);
	  } else {
	      HTMeter((total_read * 100) / (loading_length + hdr_len), NULL);
	  }
      }
      HTProgress(line);
      if (HTCopyOneRead || ((loading_length != -1) &&
	  		    (total_read >= (loading_length + hdr_len))))
	  break;
  }  /* Next bufferload */
  
  /* Success */
  if (HTCopyOneRead && ((loading_length != -1) &&
			(total_read >= (loading_length + hdr_len)))) {
      /* Keep HTMultiLoad from calling again since we are done */
      rv = 0;
  } else {
      rv = total_read;
  }
  if (!HTCopyOneRead || !rv) {
      HTProgress("Data transfer complete.");
      HTMeter(100, NULL);
  }

 ready_to_leave:
  return rv;
}


/*	Push data from a file pointer down a stream
**	-------------------------------------------
**
**   This routine is responsible for creating and PRESENTING any
**   graphic (or other) objects described by the file.
**
*/
PUBLIC void HTFileCopy (FILE *fp, HTStream *sink)
{
    HTStreamClass targetClass;
    int status;

    targetClass = *sink->isa;	 /* Copy pointers to procedures */

    for (;;) {
	status = fread(input_buffer, 1, INPUT_BUFFER_SIZE, fp);
	if (!status) {  /* EOF or error */
	    if (!ferror(fp))
		break;
#ifndef DISABLE_TRACE
	    if (www2Trace)
		fprintf(stderr,
		        "HTFormat: Read error, read returns %d\n", ferror(fp));
#endif
	    break;
	}
	(*targetClass.put_block)(sink, input_buffer, status);
    }  /* Next bufferload */

    fclose(fp);
    return;
}


PUBLIC void HTFileCopyToText (FILE *fp,	HText *text)
{
  int status;

  for (;;) {
      status = fread(input_buffer, 1, INPUT_BUFFER_SIZE, fp);
      if (!status) {  /* EOF or error */
          if (!ferror(fp))
	      break;
#ifndef DISABLE_TRACE
          if (www2Trace)
	      fprintf(stderr,
		      "HTFormat: Read error, read returns %d\n", ferror(fp));
#endif
          break;
      }
      HText_appendBlock(text, input_buffer, status);
  }  /* Next bufferload */
  
  fclose(fp);
  return;
}


/*	Parse a socket given format and file number
**
**   This routine is responsible for creating and PRESENTING any
**   graphic (or other) objects described by the file.
**
**   The file number given is assumed to be a TELNET stream containing
**   CRLF at the end of lines which need to be stripped to LF for unix
**   when the format is textual.
**
*/
PUBLIC int HTParseSocket (HTFormat format_in,
			  HTFormat format_out,
			  HTParentAnchor *anchor,
			  int file_number,
			  HTStream *sink,
			  int compressed)
{
  HTStream *stream;
  HTStreamClass targetClass;    
  int rv;
  
  stream = HTStreamStack(format_in, format_out, compressed, sink, anchor);
  if (!stream) {
      char buffer[1024];

      sprintf(buffer, "Sorry, can't convert from %s to %s.",
              HTAtom_name(format_in), HTAtom_name(format_out));
#ifndef DISABLE_TRACE
      if (www2Trace)
	  fprintf(stderr, "HTParseSocket: %s\n", buffer);
#endif
      HTProgress(buffer);
      return HT_FAILED;
  }
    
  targetClass = *stream->isa;	 /* Copy pointers to procedures */
  rv = HTCopy(file_number, stream, 0, NULL, -1);
  if (rv == -1) {
      /* Handle_interrupt should have been done in HTCopy */
      (*targetClass.free)(stream);
      return HT_INTERRUPTED;
  }

  (*targetClass.end_document)(stream);

  /* New thing: we force close the data socket here, so that if
   * an external viewer gets forked off in the free method below,
   * the connection doesn't remain up until the child exits --
   * which it does if we don't do this. */
  NETCLOSE(file_number);

  (*targetClass.free)(stream);
    
  return HT_LOADED;
}


/*	Parse a file given format and file pointer
**
**   This routine is responsible for creating and PRESENTING any
**   graphic (or other) objects described by the file.
**
**   The file number given is assumed to be a TELNET stream containing
**   CRLF at the end of lines which need to be stripped to LF for unix
**   when the format is textual.
**
*/
PUBLIC int HTParseFile (HTFormat format_in,
			HTFormat format_out,
			HTParentAnchor *anchor,
			FILE *fp,
			HTStream *sink,
			int compressed)
{
    HTStream *stream;
    HTStreamClass targetClass;    
    
    stream = HTStreamStack(format_in, format_out, compressed, sink, anchor);
    if (!stream) {
        char buffer[1024];

	sprintf(buffer, "Sorry, can't convert from %s to %s.",
		HTAtom_name(format_in), HTAtom_name(format_out));
#ifndef DISABLE_TRACE
	if (www2Trace)
	    fprintf(stderr, "HTParseFile: %s\n", buffer);
#endif
	HTProgress(buffer);
        return HT_FAILED;
    }
    targetClass = *stream->isa;  /* Copy pointers to procedures */
    HTFileCopy(fp, stream);
    (*targetClass.end_document)(stream);
    (*targetClass.free)(stream);
    
    return HT_LOADED;
}
