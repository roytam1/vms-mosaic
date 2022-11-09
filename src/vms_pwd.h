/*
 * An attempt at a pwd.h file, a subset of Ultrix 4.3 pwd.h to be
 * used with VMS xmosaic
 * Bjorn S. Nilsson, 2-May-1993.
 */
#ifndef _PWD_H_
#define _PWD_H_

struct  passwd { /* see getpwent(3) */
        char    *pw_name;
        char    *pw_passwd;
/*        uid_t   pw_uid; */
/*        short   pad;    */
/*        gid_t   pw_gid; */
/*        short   pad1;   */
        int     pw_quota;       /* ULTRIX, BSD-4.2 */
        char    *pw_comment;
        char    *pw_gecos;
        char    *pw_dir;
        char    *pw_shell;
};
struct passwd *getpwent(), *getpwuid(), *getpwnam();

#endif /* _PWD_H_ */
