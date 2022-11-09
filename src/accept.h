typedef int	ListenAddress;

extern ListenAddress NetServerInitSocket();
extern PortDescriptor *NetServerAccept();
extern int NetRead();
extern int NetServerWrite();
extern void NetCloseConnection();
extern int NetIsThereInput();

void NetCloseAcceptPort(int);
int NetGetSocketDescriptor(PortDescriptor *);
int NetIsThereAConnection(int);
