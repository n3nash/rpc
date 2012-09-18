#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <curl/curl.h>
#include <rpc/rpc.h>
#include "proxy.h"


struct wd_in
{
size_t size;
size_t len;
char *data;
};



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


char **
proxyserver_1_svc(urlc,client)
char **urlc;struct svc_req *client;
{
 CURL *curl;
  CURLcode res;
  struct wd_in wdi;
  char *url = *urlc;
  static char *result;
  memset(&wdi, 0, sizeof(wdi));

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

  result = wdi.data;
  free(wdi.data);
  
  return (&result);
}

