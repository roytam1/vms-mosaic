/* <lib$routines.h>
 *
 *	General run-time library routines.
 */
#ifndef _LIB$ROUTINES_H
#define _LIB$ROUTINES_H
# ifdef __cplusplus
extern "C" {
# endif

/*	LIB$ADAWI		-- add aligned word with interlock */
unsigned long lib$adawi(const short *,short *,short *);
/*	LIB$ADD_TIMES		-- add two quadword times */
unsigned long lib$add_times(const void *,const void *,void *);
/*	LIB$ADDX		-- add two multiple-precision binary numbers */
unsigned long lib$addx(const void *,const void *,void *,...);
/*	LIB$ANALYZE_SDESC	-- analyze string descriptors */
unsigned long lib$analyze_sdesc(const void *,unsigned short *,void *);
/*	LIB$ASCII_TO_UID	-- convert text to UID */
unsigned long lib$ascii_to_uid();
/*	LIB$ASN_WTH_MBX		-- assign channel with mailbox */
unsigned long lib$asn_wth_mbx(const void *,const long *,const long *,
			      unsigned short *,unsigned short *);
/*	LIB$AST_IN_PROG		-- AST in progress */
int lib$ast_in_prog(void);		/*(boolean)*/
/*	LIB$ATTACH		-- attach terminal to process */
unsigned long lib$attach(const unsigned long *);
/*	LIB$BBCCI		-- test and clear or set bit with interlock */
int lib$bbcci(const int *,void *), lib$bbssi(const int *,void *);  /*(boolean)*/
/*	LIB$CALLG		-- call routine with general argument list */
unsigned long lib$callg(const void *,const unsigned long (*)()); /*(arbitrary)*/
/*	LIB$CHAR		-- transform byte to first character of string */
unsigned long lib$char(void *,const char *);
/*	LIB$COMPARE_UID		-- compare two UIDs */
unsigned long lib$compare_uid();
/*	LIB$CONVERT_DATE_STRING -- convert date string to quadword */
unsigned long lib$convert_date_string(const void *,void *,...);
/*	LIB$CRC			-- calculate a cyclic redundancy check (CRC) */
unsigned long lib$crc(const void *,const long *,const void *);
/*	LIB$CRC_TABLE		-- construct a cyclic redundancy check (CRC) table */
void lib$crc_table(const unsigned long *,void *);
/*	LIB$CREATE_DIR		-- create a directory */
unsigned long lib$create_dir(const void *,...);
/*	LIB$CREATE_USER_VM_ZONE -- create user-defined storage zone */
unsigned long lib$create_user_vm_zone(unsigned long *,...);
/*	LIB$CREATE_VM_ZONE	-- create a new zone */
unsigned long lib$create_vm_zone(unsigned long *,...);
/*	LIB$CRF_INS_KEY		-- insert key in cross-reference table */
void lib$crf_ins_key(const void *,const void *,const long *,const unsigned long *);
/*	LIB$CRF_INS_REF		-- insert reference to a key in the cross-reference table */
void lib$crf_ins_ref(const void *,const long *,const void *,long *,const long *);
/*	LIB$CRF_OUTPUT		-- output cross-reference table information */
void lib$crf_output(const void *,const long *,const long *,const long *,const long *,const long *);
/*	LIB$CURRENCY		-- get system currency symbol */
unsigned long lib$currency(void *,...);
/*	LIB$CVT_DTB,LIB$CVT_HTB,LIB$CVT_OTB -- convert numeric text to binary */
unsigned long lib$cvt_dtb(int,const char *,long *),
	      lib$cvt_htb(int,const char *,long *),
	      lib$cvt_otb(int,const char *,long *);
/*	LIB$CVT_DX_DX		-- general data type conversion routine */
unsigned long lib$cvt_dx_dx(const void *,void *,...);
/*	LIB$CVT_FROM_INTERNAL_TIME -- convert internal time to external time */
unsigned long lib$cvt_from_internal_time(const unsigned long *,long *,...);
/*	LIB$CVTF_FROM_INTERNAL_TIME -- convert internal time to external time (F-floating point value) */
unsigned long lib$cvtf_from_internal_time(const unsigned long *,float *,const void *);
/*	LIB$CVT_TO_INTERNAL_TIME -- convert external time to internal time */
unsigned long lib$cvt_to_internal_time(const unsigned long *,const long *,void *);
/*	LIB$CVTF_TO_INTERNAL_TIME -- convert external time to internal time (F-floating point value) */
unsigned long lib$cvtf_to_internal_time(const unsigned long *,const float *,void *);
/*	LIB$CVT_VECTIM		-- convert seven-word vector to internal time */
unsigned long lib$cvt_vectim(const void *,void *);
/*	LIB$DATE_TIME		-- date and time returned as a string */
unsigned long lib$date_time(void *);
/*	LIB$DAY			-- day number returned as a longword integer */
unsigned long lib$day(long *,...);
/*	LIB$DAY_OF_WEEK		-- show numeric day of week */
unsigned long lib$day_of_week(const void *,int *);
/*	LIB$DEC_OVER		-- enable or disable decimal overflow detection */
unsigned long lib$dec_over(const unsigned long *);
/*	LIB$DECODE_FAULT	-- decode instruction stream during fault */
unsigned long lib$decode_fault(const void *,const void *,...);
/*	LIB$DELETE_FILE		-- delete one or more files */
unsigned long lib$delete_file(const void *,...);
/*	LIB$DELETE_LOGICAL	-- delete logical name */
unsigned long lib$delete_logical(const void *,...);
/*	LIB$DELETE_SYMBOL	-- delete CLI symbol */
unsigned long lib$delete_symbol(const void *,...);
/*	LIB$DELETE_VM_ZONE	-- delete virtual memory zone */
unsigned long lib$delete_vm_zone(const unsigned long *);
/*	LIB$DIGIT_SEP		-- get digit separator symbol */
unsigned long lib$digit_sep(void *,...);
/*	LIB$DISABLE_CTRL	-- disable CLI interception of control characters */
unsigned long lib$disable_ctrl(const unsigned long *,...);
/*	LIB$DO_COMMAND		-- execute command */
unsigned long lib$do_command(const void *);
/*	LIB$EDIV		-- extended-precision divide */
unsigned long lib$ediv(const long *,const void *,long *,long *);
/*	LIB$EMODD,LIB$EMODF,LIB$EMODG,LIB$EMODH -- extended multiply and integerize */
unsigned long lib$emodd(const double *,const unsigned char *,const double *,long *,double *),
	      lib$emodf(const float *,const unsigned char *,const float *,long *,float *),
	      lib$emodg(const double *,const unsigned short *,const double *,long *,double *),
	      lib$emodh(const void *,const unsigned short *,const void *,long *,void *);
/*	LIB$EMUL		-- extended-precision multiply */
unsigned long lib$emul(const long *,const long *,const long *,void *);
/*	LIB$ENABLE_CTRL		-- enable CLI interception of control characters */
unsigned long lib$enable_ctrl(const unsigned long *,...);
/*	LIB$ESTABLISH		-- establish a condition handler */
unsigned long (*lib$establish(const unsigned long (*)(void *,void *)))(void *,void *);
/*	LIB$EXTV		-- extract a field and sign-extend */
long lib$extv(const int *,const unsigned char *,const void *);
/*	LIB$EXTZV		-- extract a zero-extended field */
long lib$extzv(const int *,const unsigned char *,const void *);
/*	LIB$FFC,LIB$FFS		-- find first clear or set bit */
unsigned long lib$ffc(const int *,const int *,const void *,int *),
	      lib$ffs(const int *,const int *,const void *,int *);
/*	LIB$FID_TO_NAME		-- convert device and file ID to file specification */
unsigned long lib$fid_to_name(const void *,const void *,void *,...);
/*	LIB$FILE_SCAN		-- file scan */
struct FAB;	/* avoid warning, in case neither <fab.h> nor <rms.h> present */
unsigned long lib$file_scan(const struct FAB *,const unsigned long (*)(struct FAB *),
			    const unsigned long (*)(struct FAB *),...);
/*	LIB$FILE_SCAN_END	-- end-of-file scan */
unsigned long lib$file_scan_end();
/*	LIB$FIND_FILE		-- find file */
unsigned long lib$find_file(const void *,void *,unsigned long *,...);
/*	LIB$FIND_FILE_END	-- end of find file */
unsigned long lib$find_file_end(const unsigned long *);
/*	LIB$FIND_IMAGE_SYMBOL	-- find universal symbol in shareable image file */
unsigned long lib$find_image_symbol(const void *,const void *,void *,...);
/*	LIB$FIND_VM_ZONE	-- return the next valid zone identifier */
unsigned long lib$find_vm_zone(unsigned long *,unsigned long *);
/*	LIB$FIXUP_FLT		-- fix floating reserved operand */
unsigned long lib$fixup_flt(const void *,const void *,...);
/*	LIB$FLT_UNDER		-- floating-point underflow detection */
unsigned long lib$flt_under(const unsigned long *);
/*	LIB$FORMAT_DATE_TIME	-- format date and/or time */
unsigned long lib$format_date_time(void *,...);
/*	LIB$FREE_DATE_TIME_CONTEXT -- free context area used when formatting dates and times for input or output */
unsigned long lib$free_date_time_context();
/*	LIB$FREE_EF		-- free event flag */
unsigned long lib$free_ef(const unsigned long *);
/*	LIB$FREE_LUN		-- free logical unit number */
unsigned long lib$free_lun(const long *);
/*	LIB$FREE_TIMER		-- free timer storage */
unsigned long lib$free_timer(void *);
/*	LIB$FREE_VM		-- free virtual memory from program region */
unsigned long lib$free_vm(const long *,const void *,...);
/*	LIB$FREE_VM_PAGE	-- free virtual memory page */
unsigned long lib$free_vm_page(const long *,const void *);
/*	LIB$GET_COMMAND		-- get line from SYS$COMMAND */
unsigned long lib$get_command(void *,...);
/*	LIB$GET_COMMON		-- get string from common */
unsigned long lib$get_common(void *,...);
/*	LIB$GET_DATE_FORMAT	-- get the user's date input format */
unsigned long lib$get_date_format(void *,...);
/*	LIB$GET_EF		-- get event flag */
unsigned long lib$get_ef(unsigned long *);
/*	LIB$GET_FOREIGN		-- get foreign command line */
unsigned long lib$get_foreign(void *,...);
/*	LIB$GET_INPUT		-- get line from SYS$INPUT */
unsigned long lib$get_input(void *,...);
/*	LIB$GET_LUN		-- get logical unit number */
unsigned long lib$get_lun(long *);
/*	LIB$GET_MAXIMUM_DATE_LENGTH -- retrieve the maximum length of a date/time string */
unsigned long lib$get_maximum_date_length(long *,...);
/*	LIB$GET_SYMBOL		-- get value of CLI symbol */
unsigned long lib$get_symbol(const void *,void *,...);
/*	LIB$GET_USERS_LANGUAGE	-- return the user's language */
unsigned long lib$get_users_language(void *);
/*	LIB$GET_VM		-- allocate virtual memory */
unsigned long lib$get_vm(const long *,void *,...);
/*	LIB$GET_VM_PAGE		-- get virtual memory page */
unsigned long lib$get_vm_page(const long *,void *);
/*	LIB$GETDVI		-- get device/volume information */
unsigned long lib$getdvi(const long *,const unsigned short *,const void *,long *,...);
/*	LIB$GETJPI		-- get job/process information */
unsigned long lib$getjpi(const long *,const unsigned long *,const void *,long *,...);
/*	LIB$GETQUI		-- get queue information */
unsigned long lib$getqui(const long *,const long *,const long *,const void *,
			 const unsigned long *,long *,...);
/*	LIB$GETSYI		-- get systemwide information */
unsigned long lib$getsyi(const long *,void *,...);
/*	LIB$ICHAR		-- convert first character of string to integer */
int lib$ichar(const void *);
/*	LIB$INDEX		-- index to relative position of substring */
int lib$index(const void *,const void *);
/*	LIB$INIT_DATE_TIME_CONTEXT -- initialize context area used in formatting dates and times for input or output */
unsigned long lib$init_date_time_context(unsigned long *,const long *,const void *);
/*	LIB$INIT_TIMER		-- initialize times and counts */
unsigned long lib$init_timer();
/*	LIB$INSERT_TREE		-- insert entry in a balanced binary tree */
long int lib$insert_tree(void *,void *,const unsigned long *,
			 const int (*)(void *,void *,void *),
			 const void (*)(void *,void *,void *),...);
/*	LIB$INSQHI		-- insert entry at head of queue */
unsigned long lib$insqhi(void *,void *,...);
/*	LIB$INSQTI		-- insert entry at tail of queue */
unsigned long lib$insqti(void *,void *,...);
/*	LIB$INSV		-- insert a variable bit field */
void lib$insv(const int *,const int *,const unsigned char *,void *);
/*	LIB$INT_OVER		-- integer overflow detection */
unsigned long lib$int_over(const unsigned long *);
/*	LIB$LEN			-- length of string returned as longword value */
unsigned short lib$len(const void *);
/*	LIB$LOCC		-- locate a character */
int lib$locc(const void *,const void *);
/*	LIB$LOOKUP_KEY		-- look up keyword in table */
unsigned long lib$lookup_key(const void *,const void *,void *,...);
/*	LIB$LOOKUP_TREE		-- look up an entry in a balanced binary tree */
unsigned long lib$lookup_tree(const void *,void *,const int (*)(void *,void *),void *);
/*	LIB$LP_LINES		-- lines on each printer page */
long lib$lp_lines(void);
/*	LIB$MATCHC		-- match characters, return relative position */
int lib$matchc(const void *,const void *);
/*	LIB$MATCH_COND		-- match condition values */
int lib$match_cond(const unsigned long *,const unsigned long *,...);
/*	LIB$MOVC3		-- move characters */
void lib$movc3(const unsigned short *,const void *,void *);
/*	LIB$MOVC5		-- move characters with fill */
void lib$movc5(const unsigned short *,const void *,const char *,
	       const unsigned short *,void *);
/*	LIB$MOVTC		-- move translated characters */
unsigned long lib$movtc(const void *,const void *,const void *,void *);
/*	LIB$MOVTUC		-- move translated until character */
int lib$movtuc(const void *,const void *,const void *,void *,...);
/*	LIB$MULT_DELTA_TIME	-- multiply delta time by scalar */
unsigned long lib$mult_delta_time(const long *,void *);
/*	LIB$MULTF_DELTA_TIME	-- multiply delta time by an F_floating scalar */
unsigned long lib$multf_delta_time(const float *,void *);
/*	LIB$PAUSE		-- pause program execution */
unsigned long lib$pause(void);
/*	LIB$POLYD,LIB$POLYF,LIB$POLYG,LIB$POLYH -- evaluate polynomials */
unsigned long lib$polyd(const double *,const short *,const double *,double *),
	      lib$polyf(const float *,const short *,const float *,float *),
	      lib$polyg(const double *,const short *,const double *,double *),
	      lib$polyh(const void *,const short *,const void *,void *);
/*	LIB$PUT_COMMON		-- put string to common */
unsigned long lib$put_common(const void *,...);
/*	LIB$PUT_OUTPUT		-- put line to SYS$OUTPUT */
unsigned long lib$put_output(const void *);
/*	LIB$RADIX_POINT		-- radix point symbol */
unsigned long lib$radix_point(void *,...);
/*	LIB$REMQHI		-- remove entry from head of queue */
unsigned long lib$remqhi(void *,void *,...);
/*	LIB$REMQTI		-- remove entry from tail of queue */
unsigned long lib$remqti(void *,void *,...);
/*	LIB$RENAME_FILE		-- rename one or more files */
unsigned long lib$rename_file(const void *,const void *,...);
/*	LIB$RESERVE_EF		-- reserve event flag */
unsigned long lib$reserve_ef(const unsigned long *);
/*	LIB$RESET_VM_ZONE	-- reset virtual memory zone */
unsigned long lib$reset_vm_zone(const unsigned long *);
/*	LIB$REVERT		-- revert to the handler of the routine activator */
unsigned long (*lib$revert(void))(void *,void *);
/*	LIB$RUN_PROGRAM		-- run new program */
unsigned long lib$run_program(const void *);
/*	LIB$SCANC		-- scan for characters, return relative position */
int lib$scanc(const void *,const unsigned char *,const unsigned char *);
/*	LIB$SCOPY_DXDX		-- copy source string passed by descriptor to destination */
unsigned long lib$scopy_dxdx(const void *,void *);
/*	LIB$SCOPY_R_DX		-- copy source string passed by reference to destination string */
unsigned long lib$scopy_r_dx(const unsigned short *,const void *,void *);
/*	LIB$SET_LOGICAL		-- set logical name */
unsigned long lib$set_logical(const void *,const void *,...);
/*	LIB$SET_SYMBOL		-- set value of CLI symbol */
unsigned long lib$set_symbol(const void *,const void *,...);
/*	LIB$SFREE1_DD		-- free one dynamic string */
unsigned long lib$sfree1_dd(void *);
/*	LIB$SFREEN_DD		-- free one or more dynamic strings */
unsigned long lib$sfreen_dd(const unsigned *,void *);
/*	LIB$SGET1_DD		-- get one dynamic string */
unsigned long lib$sget1_dd(const unsigned short *,void *);
/*	LIB$SHOW_TIMER		-- show accumulated times and counts */
unsigned long lib$show_timer();
/*	LIB$SHOW_VM		-- show virtual memory statistics */
unsigned long lib$show_vm();
/*	LIB$SHOW_VM_ZONE	-- return information about a zone */
unsigned long lib$show_vm_zone(const unsigned long *,...);
/*	LIB$SIGNAL		-- signal exception condition */
void lib$signal(unsigned long,...);
/*	LIB$SIG_TO_RET		-- signal converted to a return status */
unsigned long lib$sig_to_ret(void *,void *);
/*	LIB$SIG_TO_STOP		-- convert a signaled condition to a signaled stop */
unsigned long lib$sig_to_stop(void *,void *);
/*	LIB$SIM_TRAP		-- simulate floating trap */
unsigned long lib$sim_trap(void *,void *);
/*	LIB$SKPC		-- skip equal characters */
int lib$skpc(const void *,const void *);
/*	LIB$SPANC		-- skip selected characters */
int lib$spanc(const void *,const unsigned char *,const unsigned char *);
/*	LIB$SPAWN		-- spawn subprocess */
unsigned long lib$spawn();
/*	LIB$STAT_TIMER		-- statistics, return accumulated times and counts */
unsigned long lib$stat_timer(const int *,void *,...);
/*	LIB$STAT_VM		-- return virtual memory statistics */
unsigned long lib$stat_vm(const int *,void *);
/*	LIB$STAT_VM_ZONE	-- return information about a zone */
unsigned long lib$stat_vm_zone(const unsigned long *,const int *,void *);
/*	LIB$STOP		-- stop execution and signal the condition */
void lib$stop(unsigned long,...);
/*	LIB$SUB_TIMES		-- subtract two quadword times */
unsigned long lib$sub_times(const void *,const void *,void *);
/*	LIB$SUBX		-- multiple-precision binary subtraction */
unsigned long lib$subx(const void *,const void *,void *,...);
/*	LIB$SYS_ASCTIM		-- invoke $ASCTIM to convert binary time to ASCII string */
unsigned long lib$sys_asctim(unsigned short *,void *,...);
/*	LIB$SYS_FAO		-- invoke $FAO system service to format output */
unsigned long lib$sys_fao(const void *,unsigned short *,void *,...);
/*	LIB$SYS_FAOL		-- invoke $FAOL system service to format output */
unsigned long lib$sys_faol(const void *,unsigned short *,void *,const void *);
/*	LIB$SYS_GETMSG		-- invoke $GETMSG system service to get message text */
unsigned long lib$sys_getmsg(const unsigned long *,unsigned short *,void *,...);
/*	LIB$SYS_TRNLOG		-- invoke $TRNLOG system service to translate logical name */
unsigned long lib$sys_trnlog(const void *,unsigned short *,void *,...);
/*	LIB$TPARSE		-- table-driven finite-state parser */
unsigned long lib$tparse(void *,const void *,const void *);
/*	LIB$TRA_ASC_EBC		-- translate ASCII to EBCDIC */
unsigned long lib$tra_asc_ebc(const void *,void *);
/*	LIB$TRA_EBC_ASC		-- translate EBCDIC to ASCII */
unsigned long lib$tra_ebc_asc(const void *,void *);
/*	LIB$TRAVERSE_TREE	-- traverse a balanced binary tree */
unsigned long lib$traverse_tree(const void *,const unsigned long (*)(void *,void *),...);
/*	LIB$TRIM_FILESPEC	-- fit long file specification into fixed field */
unsigned long lib$trim_filespec(const void *,void *);
/*	LIB$UID_TO_ASCII	-- convert a UID to text */
unsigned long lib$uid_to_ascii();
/*	LIB$VERIFY_VM_ZONE	-- verify a zone */
unsigned long lib$verify_vm_zone(const unsigned long *);
/*	LIB$WAIT		-- wait a specified period of time */
unsigned long lib$wait(float);

# ifdef __cplusplus
}
# endif
#endif	/*_LIB$ROUTINES_H*/
