#ifndef __NETDB_LOADED
#define __NETDB_LOADED	1


/*
 * Structures returned by network
 * data base library.  All addresses
 * are supplied in host order, and
 * returned in network order (suitable
 * for use in system calls).
 */
struct	hostent {
	char	*h_name;	/* official name of host */
	char	**h_aliases;	/* alias list */
	int	h_addrtype;	/* host address type */
	int	h_length;	/* length of address */
	char	**h_addr_list;	/* address */
#define h_addr h_addr_list[0]
};

/*
 * Assumption here is that a network number
 * fits in 32 bits -- probably a poor one.
 */
struct	netent {
	char	*n_name;	/* official name of net */
	char	**n_aliases;	/* alias list */
	int	n_addrtype;	/* net address type */
	int	n_net;		/* network # */
};

struct	servent {
	char	*s_name;	/* official service name */
	char	**s_aliases;	/* alias list */
	int	s_port;		/* port # */
	char	*s_proto;	/* protocol to use */
};

struct	protoent {
	char	*p_name;	/* official protocol name */
	char	**p_aliases;	/* alias list */
	int	p_proto;	/* protocol # */
};

struct rpcent {
	char    *r_name;        /* name of server for this rpc program */
	char    **r_aliases;    /* alias list */
	int     r_number;       /* rpc program number */
};

struct hostent *gethostbyaddr( char *addr, int len, int type);
struct hostent *gethostbyname( char *name);
struct hostent *gethostent();
struct netent *getnetbyaddr( long net, int type);
struct netent *getnetbyname( char *name);
struct netent *getnetent();
struct servent *getservbyname( char *name, char *proto);
struct servent *getservbyport(int port, char *proto);
struct servent *getservent();
struct protoent	*getprotobyname(char *name);
struct protoent	*getprotobynumber(int proto);
struct protoent	*getprotoent();

#endif					/* __NETDB_LOADED */
