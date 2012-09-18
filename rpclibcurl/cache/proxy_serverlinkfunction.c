#include <stdio.h>
#include<malloc.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <curl/curl.h>
#include <rpc/rpc.h>
#include "proxy.h"
#include "hashfunction.h"
#include <rpc/pmap_clnt.h>
#include <memory.h>
#include <sys/socket.h>
#include <netinet/in.h>

extern void messageprog_1(struct svc_req *rqstp, register SVCXPRT *transp);
char *crp;

struct wd_in
{
size_t size;
size_t len;
char *data;
};

struct map{
hashtab1 *hashtab[cachesize];
int size;
};
int flag=1;
static int recently_count=0;
int hashsize = 2<<6;
//static hashtab table[64];  //is this the size of the cache.
int size_hash=0;

static size_t write_data(void *buffer, size_t size,
                         size_t nmemb, void *userp) {
  struct wd_in *wdi = userp;
  while(wdi->len + (size * nmemb) >= wdi->size) {
    /* check for realloc failing in real code. */
    wdi->data = realloc(wdi->data, wdi->size*2);
    wdi->size*=2;
  }

  memcpy(wdi->data + wdi->len, buffer, size * nmemb);
  wdi->len+=size*nmemb;

  return size * nmemb;
}

struct map *handle;

char **crplfu(char **urlc)
{

if(flag)
{
handle = (struct map *)malloc((sizeof(struct map)));
handle->size = 0;
flag= 0;
}
unsigned int value;
 CURL *curl;
  CURLcode res;
  struct wd_in wdi;
  char *url = *urlc;
  static char *result;
  char choice[10] = "LRU";  
value = (DEKHash(url,strlen(url)))&(hashsize-1);
struct listofall *use;
if(handle->hashtab[value] == NULL)
{

  if((handle->size/hashsize)*100 > 80){
	int i=0,key=0;
	int h = 0;
   struct listofall *evict;
   for(i=0;i<hashsize;i++){
        struct listofall *me = (struct listofall *)handle->hashtab[i];
	while(me!=NULL){
	
	if(me->frequency > h){
		h = me->frequency;
		evict = me;}
         me=me->nxt;
	}
        struct listofall *change;
        change = evict->prev;
        evict->nxt->prev = change;
        evict->prev->nxt = evict->nxt;
	free(evict);
        handle->size = handle->size - 1;
	size_hash--;}
}


//struct listofall *temp = (struct listofall *)malloc(sizeof(struct listofall));

  memset(&wdi, 0, sizeof(wdi));
  recently_count++;
  curl = curl_easy_init();

  if(NULL != curl) {
    wdi.size = 1024;
    wdi.data = malloc(wdi.size);

    curl_easy_setopt(curl, CURLOPT_URL, url);

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);

    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &wdi);

    res = curl_easy_perform(curl);

    curl_easy_cleanup(curl);
  }
  else {
    fprintf(stderr, "Error: could not get CURL handle.\n");
    exit(EXIT_FAILURE);
  } 
   size_hash += 1; 
   use = (struct listofall *)handle->hashtab[value];
   struct listofall *temp = (struct listofall*)malloc((sizeof(struct listofall)));
   temp->value = (char *) malloc(strlen(wdi.data) + 3);
   temp->count = recently_count;
   temp->frequency += 1;
   temp->value = wdi.data;
   temp->nxt = NULL;
   if(use == NULL){
        temp->prev = use;
	use = temp;}
   if(use!=NULL){
	while(use->nxt==NULL)
		use = use->nxt;
    temp->prev = use;
    use->nxt = temp;
    }
   handle->size += 1;
   strcat(temp->value,"FU");
   result = temp->value;
  free(wdi.data);
}
else{
recently_count++;
printf("value is inserted");
use = (struct listofall *)handle->hashtab[value];
while(use!=NULL)
{
if(use->url == url)
{
result = use->value;
use->frequency +=1;
use->count = recently_count;
}
use = use->nxt;
} 
}
  return (&result);
	
}


char **crplru(char **urlc)
{

}


char **crprand(char **urlc)
{

}


char **
proxyserver_1_svc(urlc, client)
char **urlc; struct svc_req *client; 
{


if(!strcmp(crp,"LRU"))
return crplru(urlc);

if(!strcmp(crp, "LFU"))
return crplfu(urlc);

if(!strcmp(crp, "RAND"))
return crprand(urlc);

}


void main(int argc, char **argv)
{
	if(argc!=2)
	{
	printf("Usage: %s <cache_replacement_policy>\nLRU = Least Recently Used\nRAND = Random\nLFU = Least Frequently Used\n",argv[0]);
	exit(-1);
	}
        crp = argv[1];  
	if(strcmp(crp,"LRU") && strcmp(crp,"LFU") && strcmp(crp,"RAND"))
	{
	printf("Invalid cache replacement policy\n");
	exit(-1);
	}

	register SVCXPRT *transp;

	pmap_unset (MESSAGEPROG, MESSAGEVERS);

	transp = svcudp_create(RPC_ANYSOCK);
	if (transp == NULL) {
		fprintf (stderr, "%s", "cannot create udp service.");
		exit(1);
	}
	if (!svc_register(transp, MESSAGEPROG, MESSAGEVERS, messageprog_1, IPPROTO_UDP)) {
		fprintf (stderr, "%s", "unable to register (MESSAGEPROG, MESSAGEVERS, udp).");
		exit(1);
	}

	transp = svctcp_create(RPC_ANYSOCK, 0, 0);
	if (transp == NULL) {
		fprintf (stderr, "%s", "cannot create tcp service.");
		exit(1);
	}
	if (!svc_register(transp, MESSAGEPROG, MESSAGEVERS, messageprog_1, IPPROTO_TCP)) {
		fprintf (stderr, "%s", "unable to register (MESSAGEPROG, MESSAGEVERS, tcp).");
		exit(1);
	}

	svc_run ();
	fprintf (stderr, "%s", "svc_run returned");
	exit (1);


}

