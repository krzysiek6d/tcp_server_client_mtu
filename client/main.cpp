#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

void error(const char *msg) {
    perror(msg);
    exit(1);
}

void readDataFromSocket(int fd)
{
    int total = 0;	
    while(true)
    {
        unsigned READ_BUFSIZE = 5000;
        char buf[READ_BUFSIZE] = {};
        int readlen = read(fd, buf, READ_BUFSIZE);
        if (readlen < 0)
            error("ERROR reading from socket");
        printf("client received %d bytes\n", readlen);
	if(readlen > 0 && 'K' == buf[readlen-1])
	{
	    total+=readlen;	
	    printf("K received, stop reading\n");
            break;
        }
	else if(readlen==0)
	{
	    printf("readlen == 0\n");
	    break;
        }
        else
	{
	    total+=readlen;
        }	    
    }
    printf("total data size is %d\n", total);
}

void sendData(int fd, int len)
{
	char* buf = new char[len];
	for(int i = 0; i<len; i++)
		buf[i]='b';
	buf[len-1]='K';
	int writtenlen = write(fd, buf, len);
	if (writtenlen <0 ) error("ERROR writing to socket");
	delete[] buf;
}

void setDFbit(int fd, bool bit)
{
    int val = IP_PMTUDISC_DO;
    if(val == false) val = IP_PMTUDISC_DONT;
    if(setsockopt(fd, IPPROTO_IP, IP_MTU_DISCOVER, &val, sizeof(val)))
    {
        printf("Cant set DF\n");
    }
    else
    {
        printf("setting DF=%d ok...\n", bit);
    }
}

int getMtuSize(int fd)
{
    int socket_mtu;
    socklen_t size = sizeof( socket_mtu );
    if(getsockopt(fd,IPPROTO_IP,IP_MTU,(char *)&socket_mtu, &size))
    {
        printf("unable to retreive MTU\n");
	return 0;
    }
    else {
        printf("MTU is %d\n", socket_mtu);
	return socket_mtu;
    }
}

void readMSS(int fd)
{
	int mss;
	socklen_t len = sizeof(mss);
	if(getsockopt(fd, IPPROTO_TCP, TCP_MAXSEG, &mss, &len))
		printf("ERR: can't read mss value\n");
	else
		printf("MSS is %d\n", mss);
}

void setSocketMss( int i_sd, int mss )
{
    int res = 0;
    socklen_t len = sizeof( mss );

    res = setsockopt( i_sd, IPPROTO_TCP, TCP_MAXSEG, &mss, len );
    if ( res < 0 )
    {
        printf("error: cannot configure mss for socket %d \n");
    }
    else
    {
        printf("setting MSS to %d succeeded\n", mss);
    }
}

int main(int argc, char **argv) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) error("ERROR opening socket");

    int optval = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int));
    //setSocketMss(fd, 1400);
    setDFbit(fd, 1);

    struct sockaddr_in serveraddr = {};
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(0x0A00000B );
    serveraddr.sin_port = htons((unsigned short)8888);

    if (connect(fd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0) error("ERROR on connect");

    getMtuSize(fd);
    //setDFbit(fd);
    readMSS(fd);

    readDataFromSocket(fd);
    sendData(fd, 2000); 
    close(fd);
}

