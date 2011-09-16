#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/times.h>

//______________________________________________________________________________
// Prototypes
int getsocket(struct hostent *h, int);
void printhelp();
int recvn(int sock, void *buffer, int length);
int sendn(int sock, const void *buffer, int length);

//______________________________________________________________________________
// The client request structure
typedef struct {
   int first;
   int second;
   int third;
   int fourth;
   int fifth;
} clnt_HS_t;

//______________________________________________________________________________
// The body received after the first handshake's header
typedef struct {
   int msglen;
   int protover;
   int msgval;
} srv_HS_t;

static int verbose = 0;

//______________________________________________________________________________
int main(int argc, char **argv)
{
   // Non-blocking check for a PROOF service at 'url'
   // Return
   //        0 if a XProofd (or Xrootd) daemon is listening at 'url'
   //        1 if nothing is listening on the port (connection cannot be open)
   //        2 if something is listening but not XProofd (or Xrootd)
   //        3 PROOFD (or ROOTD) is listening at 'url' 

   // Check arguments
   if (argc <= 1) {
      fprintf(stderr,"%s: at least one argument must be given!\n", argv[0]);
      printhelp();
      exit(1);
   }

   // Extract URL and port from the URL, if any
   int port = -1;
   char *sport = 0, *url = 0;
   bool chk_xpd = 1;
   for (int i = 1; i < argc; i++) {
      if (argv[i]) {
         if (!strcmp(argv[i], "xproofd") || !strcmp(argv[i], "xpd")) {
            chk_xpd = 1;
         } else if (!strcmp(argv[i], "xrootd") || !strcmp(argv[i], "xrd")) {
            chk_xpd = 0;
         } else if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
            printhelp();
            exit(0);
         } else if (!strcmp(argv[i], "-v")) {
            verbose = 1;
         } else if (!strcmp(argv[i], "-nv")) {
            verbose = -1;
         } else if (!strcmp(argv[i], "-p")) {
            if (argv[i+1]) {
               i++;
               port = atoi(argv[i]);
            }
         } else {
            // This the URL
            url = new char[strlen(argv[i])+1];
            strcpy(url, argv[i]);
            sport = (char *) strchr(url, ':');
            if (sport) {
               *sport = 0;
               sport++;
            }
         }
      }
   }

   if (port < 0) {
      if (sport) {
         port = atoi(sport);
      } else if (chk_xpd) {
         port = 1093;
      } else {
         port = 1094;
      }
   }

   if (verbose > 0) fprintf(stderr, "Testing service at %s:%u ...\n", url, port);

   struct hostent *h = gethostbyname(url);
   if (!h) {
      if (verbose > -1) fprintf(stderr,"%s: unknown host '%s'\n",argv[0],url);
      exit(1);
   }

   // Get socket and listen to it
   int sd = getsocket(h,port);
   if (sd == -1) {
      if (verbose > -1) fprintf(stderr,"%s: problems creating socket ... exit\n",argv[0]);
      exit(1);
   }

   // Send the first bytes
   clnt_HS_t initHS;
   memset(&initHS, 0, sizeof(initHS));
   int len = sizeof(initHS);
   if (chk_xpd) {
      initHS.third  = (int)htonl((int)1);
      if (sendn(sd, &initHS, len) != len) {
         if (verbose > -1) fprintf(stderr,"%s: problems sending first set of handshake bytes\n",argv[0]);
         exit(2);
      }
      
      // These 8 bytes are need by 'rootd/proofd' and discarded by XRD/XPD
      int dum[2];
      dum[0] = (int)htonl((int)4);
      dum[1] = (int)htonl((int)2012);
      if (sendn(sd, &dum[0], sizeof(dum)) != sizeof(dum)) {
         if (verbose > -1) fprintf(stderr,"%s: problems sending second set of handshake bytes\n",argv[0]);
         exit(2);
      }
   } else {
      initHS.fourth  = (int)htonl((int)4);
      initHS.fifth  = (int)htonl((int)2012);
      if (sendn(sd, &initHS, len) != len) {
         if (verbose > -1) fprintf(stderr,"%s: problems sending handshake bytes\n",argv[0]);
         exit(2);
      }
   }
   
   // Read first server response
   int type;
   len = sizeof(type);
   int nr = 0;
   if ((nr = recvn(sd, &type, len)) != len) { // 4 bytes
      if (verbose > -1)
         fprintf(stderr, "%s: 1st: wrong number of bytes read: %d (expected: %d)\n",
                        argv[0], nr, len);
      exit(2);
   }
   
   // To host byte order
   type = ntohl(type);
   // Check if the server is the eXtended proofd
   if (type == 0) {
      srv_HS_t xbody;
      len = sizeof(xbody);
      if ((nr = recvn(sd, &xbody, len)) != len) { // 12(4+4+4) bytes
         if (verbose > -1)
            fprintf(stderr, "%s: 2nd: wrong number of bytes read: %d (expected: %d)\n",
                           argv[0], nr, len);
         exit(2);
      }
      xbody.protover = ntohl(xbody.protover);
      xbody.msgval = ntohl(xbody.msglen);
      xbody.msglen = ntohl(xbody.msgval);

   } else if (type == 8) {
      // Standard proofd
      if (verbose > -1)
         fprintf(stderr, "%s: server is %s\n", argv[0], (chk_xpd ? "PROOFD" : "ROOTD"));
      exit(3);
   } else {
      // We don't know the server type
      if (verbose > -1)
         fprintf(stderr, "%s: unknown server type: %d\n", argv[0], type);
      exit(2);
   }
   // Done
   exit(0);
}

// Auxilliary functions

//______________________________________________________________________________
int getsocket(struct hostent *h, int port)
{
   int sd, rc;
   struct sockaddr_in localAddr, servAddr;

   servAddr.sin_family = h->h_addrtype;
   memcpy((char *) &servAddr.sin_addr.s_addr, h->h_addr_list[0], h->h_length);
   servAddr.sin_port = htons(port);

   /* create socket */
   sd = socket(AF_INET, SOCK_STREAM, 0);
   if(sd < 0) {
      if (verbose > -1) perror("cannot open socket ");
      return -1;
   }

   /* bind any port number */
   localAddr.sin_family = AF_INET;
   localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
   localAddr.sin_port = htons(0);
  
   rc = bind(sd, (struct sockaddr *) &localAddr, sizeof(localAddr));
   if(rc < 0) {
      if (verbose > -1) perror("error ");
      return -1;
   }
                                
   /* connect to server */
   rc = connect(sd, (struct sockaddr *) &servAddr, sizeof(servAddr));
   if(rc < 0) {
      if (verbose > -1) perror("cannot connect ");
      return -1;
   }

   return sd;

}

//______________________________________________________________________________
int sendn(int sock, const void *buffer, int length)
{
   // Send exactly length bytes from buffer.

   if (sock < 0) return -1;

   int n, nsent = 0;
   const char *buf = (const char *)buffer;

   for (n = 0; n < length; n += nsent) {
      if ((nsent = send(sock, buf+n, length-n, 0)) <= 0) {
         if (verbose > -1) perror("problems sending ");
         return nsent;
      }
   }

   return n;
}

//______________________________________________________________________________
int recvn(int sock, void *buffer, int length)
{
   // Receive exactly length bytes into buffer. Returns number of bytes
   // received. Returns -1 in case of error.

   if (sock < 0) return -1;

   int n, nrecv = 0;
   char *buf = (char *)buffer;

   for (n = 0; n < length; n += nrecv) {
      while ((nrecv = recv(sock, buf+n, length-n, 0)) == -1 && errno == EINTR)
         errno = 0;   // probably a SIGCLD that was caught
      if (nrecv < 0) {
         if (verbose > -1) perror("problems receiving ");
         return nrecv;
      } else if (nrecv == 0)
         break;         // EOF
   }

   return n;
}

//______________________________________________________________________________
void printhelp()
{
   // Help function
   fprintf(stderr, "\n");
   fprintf(stderr, "   xrdping: ping xproofd (or xrootd) service on remote host\n");
   fprintf(stderr, "\n");
   fprintf(stderr, "   Syntax:\n");
   fprintf(stderr, "            xrdping host[:port] [xrootd|xrd] [-p port] [-v|-nv] [-h|--help]\n");
   fprintf(stderr, "\n");
   fprintf(stderr, "   host:    hostname where service should be running\n");
   fprintf(stderr, "   xrd, xrootd: check for an xrootd service instead of xproofd\n");
   fprintf(stderr, "   port:    port (at hostname) where service should be listening; port specification\n");
   fprintf(stderr, "            via the '-p' switch has priority\n");
   fprintf(stderr, "   -v:      switch-on some verbosity\n");
   fprintf(stderr, "   -nv:     switch-off all verbosity (errors included)\n");
   fprintf(stderr, "   -h, --help: print this screen\n");
   fprintf(stderr, "\n");
   fprintf(stderr, "   Return:  0   the required service accepts connections at given host and port\n");
   fprintf(stderr, "            1   no service running at the given host and port\n");
   fprintf(stderr, "            2   a service accepts connections at given host and port but most likely\n");
   fprintf(stderr, "                is not of the required type (failure occured during handshake)\n");
   fprintf(stderr, "            3   service at given host and port is PROOFD (or ROOTD)\n");
   fprintf(stderr, "\n");
}


