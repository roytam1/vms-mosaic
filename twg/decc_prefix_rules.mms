!
! Setup some common prefixing needed to build against NATIVE routines on
! Alpha platforms.
!
.IFDEF ALPHA

CC_ALL = ALL_ENTRIES
!
! Don't prefix these
!
CC_SOCKET_ACTIONS	= "accept","bind","connect","listen","select","shutdown","socket"
CC_SOCKET_IO		= "recv","send","recvfrom","recvmsg","sendto","sendmsg","readv","writev"
CC_SOCKET_GETSET	= "gethostname","sethostname","getpeername","getsockname","getsockopt","setsockopt"
CC_SOCKET		= $(CC_SOCKET_ACTIONS),$(CC_SOCKET_IO),$(CC_SOCKET_GETSET)
!
CC_COMMON_NET		= "getnetbyaddr","getnetbyname","getnetent","setnetent","endnetent"
CC_COMMON_PROTO		= "getprotobyname","getprotobynumber","getprotoent","setprotoent","endprotoent"
CC_COMMON_SERV		= "getservbyname","getservbyport","getservent","setservent","endservent"
CC_COMMON_HOST		= "gethostbyname","gethostbyaddr","gethostent","sethostent","endhostent"
CC_COMMON		= $(CC_COMMON_NET),$(CC_COMMON_PROTO),$(CC_COMMON_SERV),$(CC_COMMON_HOST)
!
CC_INET			= "inet_addr","inet_lnaof","inet_makeaddr","inet_netof","inet_ntoa","inet_network"
CC_MISC			= "htonl","htons","ntohl","ntohs"
!
CC_SIN			= "sin"
!
CC_TWG			= $(CC_SOCKET),$(CC_COMMON),$(CC_INET),$(CC_MISC),$(CC_SIN)
CC_TWG_NO_SIN		= $(CC_SOCKET),$(CC_COMMON),$(CC_INET),$(CC_MISC)
!
CC_PREFIX		= /PREFIX=($(CC_ALL), EXCEPT=($(CC_TWG)))
CC_PREFIX_NO_SIN	= /PREFIX=($(CC_ALL), EXCEPT=($(CC_TWG_NO_SIN)))

.ENDIF
