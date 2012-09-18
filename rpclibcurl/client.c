#include <stdio.h>
#include <rpc/rpc.h>
#include "proxy.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>


main(argc, argv)
int argc;
char *argv[];
{
CLIENT *cl;
int *result;
char *server;
char *url;
char **win;
if (argc != 3)
{
fprintf(stderr,
"usage: %s host message\n", argv[0]);
exit(1);
}

server = argv[1];
url = argv[2];

cl = clnt_create(server, MESSAGEPROG, MESSAGEVERS, "tcp");
if (cl == NULL)
{
clnt_pcreateerror (server);
exit(1);
}

win = proxyserver_1(&url, cl);
if (win == NULL)
{
clnt_perror (cl, server);
exit(1);
}

printf("%s", *win);
exit(0);
}

