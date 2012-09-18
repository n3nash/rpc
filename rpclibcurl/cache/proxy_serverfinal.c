#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <curl/curl.h>
#include <rpc/rpc.h>
#include "proxy.h"
#include <rpc/pmap_clnt.h>
#include <memory.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "hashfucntion.h"

extern void messageprog_1(struct svc_req *rqstp, register SVCXPRT *transp);
//#include "hashfunction.h"
char *crp;
unsigned int DEKHash(char* str, unsigned int len)
{
   unsigned int hash = len;
   unsigned int i    = 0;

   for(i = 0; i < len; str++, i++)
   {
      hash = ((hash << 5) ^ (hash >> 27)) ^ (*str);
   }
   return hash;
}

struct wd_in
{
size_t size;
size_t len;
char *data;
};

struct hashtab
{
char *value;
int count;
};

struct map{
hashtab1 *hashtab[cachesize];
int size;
};
int flag=1;


static int recently_count=0;
int hashsize = 2<<6; //Size of hash table
static struct hashtab table[64];
int size_hash=0; //Number of entries in table

static size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp) {
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


char **crplrc(char **urlc)
{

unsigned int value;
 CURL *curl;
  CURLcode res;
  struct wd_in wdi;
  char *url = *urlc;
  static char *result;
  
value = (DEKHash(url,strlen(url))&hashsize)-1;
if(table[value].value == NULL)
{
  if((size_hash/hashsize)*100 > 80){
	int i=0,key=0;
	int h = table[0].count;
   for(i=0;i<hashsize;i++){
	if(table[i].count > h){
		h = table[i].count;
		key = value;}
	}
	free(table[key].value);
	size_hash--;}
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
   table[value].value = (char *) malloc(strlen(wdi.data) + 3);
   table[value].value = wdi.data;
   table[value].count = recently_count;
	
  result = wdi.data;
  free(wdi.data);
 } 

else{
recently_count++;
table[value].count = recently_count;
result = table[value].value;
}
 return (&result);
}

char **crplfu(char **urlc)
{
unsigned int value;
  CURL *curl;
  CURLcode res;
  struct wd_in wdi;
  char *url = *urlc;
  static char *result;
  
value = (DEKHash(url,strlen(url))&hashsize)-1;
if(table[value].value == NULL)
{
  if((size_hash/hashsize)*100 > 80){
	int i=0,key=0;
	int h = table[0].count;
   for(i=0;i<hashsize;i++){
	if(table[value].count > h){
		h = table[value].count;
		key = value;}
	}
	free(table[key].value);
	size_hash--;}
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
   table[value].value = (char *) malloc(strlen(wdi.data) + 3);
   table[value].value = wdi.data;
   table[value].count = recently_count;
	
  result = wdi.data;
  free(wdi.data);
 } 

else{
recently_count++;
table[value].count = recently_count;
result = table[value].value;
}
  return (&result);
}

char **crprand(char **urlc)
{

}

char **
proxyserver_1_svc(urlc, client)
char **urlc; struct svc_req *client; 
{
	if(!strcmp(crp,"LRU"))
	return crplrc(urlc);

	else if(!strcmp(crp, "LFU"))
	return crplfu(urlc);

	else if(!strcmp(crp, "RAND"))
	return crprand(urlc);


}



void main(int argc, char **argv)
{
	if(argc!=2)
	{
	printf("Usage: %s <cache_replacement_policy>\nLRU = Least Recently Used\nRAND = Random\nLFU = Least Frequently Used\n",argv[0]);
	exit(-1);
	}
        char *arg = *argv;  
	if(!strcmp(&arg[1],"LRU") || !strcmp(&arg[1],"LFU") || !strcmp(&arg[1],"RAND"))
	{
	printf("Invalid cache replacement policy\n");
	exit(-1);
	}

	crp = &arg[1];

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

