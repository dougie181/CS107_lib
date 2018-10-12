#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "urlconnection.h"
#include "url.h"
#include <curl/curl.h>

#define DEBUG_URLCONN 0

struct MemoryStruct {
  char *memory;
  size_t size;
};

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{ 
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userp;
  
  mem->memory = realloc(mem->memory, mem->size + realsize + 1);
  if(mem->memory == NULL) {
    /* out of memory! */ 
    if (DEBUG_URLCONN) printf("not enough memory (realloc returned NULL)\n");
    return 0;
  }
  
  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize; 
  mem->memory[mem->size] = EOF;
  
  return realsize;
}

void URLInitialise()
{
  CURLcode res;
  long flags = CURL_GLOBAL_DEFAULT;
  res = curl_global_init(flags);
 
  if(res != CURLE_OK) {
    fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));  
    exit(-1);
  }
}

void URLCleanup()
{
  curl_global_cleanup();
}

void URLConnectionNew(urlconnection* urlconn, const url* u)
{
  assert(urlconn != NULL);
  assert(u != NULL);

  if (DEBUG_URLCONN) printf("URLConnectionNew - entered\n");

  struct MemoryStruct chunk;

  chunk.memory = malloc(1);	// will be grown as needed by the realloc above 
  chunk.size = 0;		// no data at this point

  urlconn->responseCode = 0;

  // set up full url, allocate space, copy from url->fullName and port
  // make sure we cater for the https:// (8 chars) or http:// (7 chars)

  int urllength = 0;

  if (u->port == 80) {
    urllength = strlen(u->fullName) + 7;
  } else {
    urllength = strlen(u->fullName) + 8;
  }

  urlconn->fullUrl = (void *) malloc(urllength + 1);

  assert(urlconn->fullUrl != NULL);
  
  if (u->port == 80) 
    strcpy((void *)urlconn->fullUrl, "http://");
  else
    strcpy((void *)urlconn->fullUrl, "https://");

  strncat((char *)urlconn->fullUrl, u->fullName, urllength - strlen(urlconn->fullUrl));

  // create responseMessage string to be max ERROR size and set it to be empty
  urlconn->responseMessage = (char*) malloc(CURL_ERROR_SIZE);
  assert(urlconn->responseMessage != NULL);

  if (DEBUG_URLCONN) printf("initialising the curl session\n");
  // init the curl session
  CURL *curl = curl_easy_init();
  CURLcode res;

  // specify URL to get
  curl_easy_setopt(curl, CURLOPT_URL, urlconn->fullUrl);
  if (DEBUG_URLCONN) printf("curl_easy_setopt for urlconn: %s\n",urlconn->fullUrl);
 
  // send all data to this function
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

  // we pass our 'chunk' struct to the callback function */
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

  // some servers don't like requests that are made without a user-agent
  // field, so we provide one
  curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");

  /* get it! */
  if (DEBUG_URLCONN) printf("performing curl call\n");
  res = curl_easy_perform(curl);

  /* check for errors */
  if(res != CURLE_OK) 
  {
    if (DEBUG_URLCONN) printf("curl failed for some reason\n");
    // set error buffer to be a response message
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, urlconn->responseMessage);
    if (DEBUG_URLCONN) printf("current responseMessage is: %s and size of %lu\n",urlconn->responseMessage,strlen(urlconn->responseMessage));

    fprintf(stderr, "curl_easy_perform() failed: %s\n",
      curl_easy_strerror(res));

  } else {
    if (DEBUG_URLCONN) printf("Yah!! curl passed\n");
    
    /*
     * Now, our chunk.memory points to a memory block that is chunk.size
     * bytes big and contains the remote file.
     *
     * Do something nice with it!
     */
    if (DEBUG_URLCONN) printf("chunk address of size %zu is located at %p\n",chunk.size,(void *)chunk.memory);

    strcpy((void *)urlconn->responseMessage, "OK!");

    if (DEBUG_URLCONN) printf("current responseMessage is: %s and size of %lu\n",urlconn->responseMessage,strlen(urlconn->responseMessage));

    if (DEBUG_URLCONN) printf("assigning memory to urlconn->datastream of %lu bytes\n",(unsigned long)chunk.size); 

    char *buf = NULL;
    urlconn->dataStream = fmemopen (buf, chunk.size + 1, "w+");
    assert(urlconn->dataStream != NULL);

    for (int i = 0; i < chunk.size; i++) {
      char ch = chunk.memory[i];
      fputc(ch, urlconn->dataStream);
    } 
    if (DEBUG_URLCONN) printf("created dataStream\n");
    rewind(urlconn->dataStream);

    chunk.size = 0;
    free(chunk.memory);

    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &(urlconn->responseCode));

    // set content type
    char *ct = NULL;
    curl_easy_getinfo(curl, CURLINFO_CONTENT_TYPE, &ct);
    urlconn->contentType = NULL;
    if (ct != NULL) {
      urlconn->contentType = strdup(ct);

//    urlconn->contentType = (char*) malloc(strlen(ct) + 1);
//    strcpy((void *)urlconn->contentType, ct);
    }

    if (DEBUG_URLCONN) printf("content type: %s\n",urlconn->contentType);

    // set up newUrl to be NULL at the beginning
    char *newUrl = NULL;
    urlconn->newUrl = NULL;

    if (DEBUG_URLCONN) printf("got here - 1\n");
    curl_easy_getinfo(curl, CURLINFO_REDIRECT_URL, &newUrl);

    if (DEBUG_URLCONN) printf("got here - 2\n");
    if(newUrl) {
      if (DEBUG_URLCONN) printf("Redirect to: %s\n", newUrl);
      if (strlen(newUrl) > 0) {
        urlconn->newUrl = strdup(newUrl);
        //urlconn->newUrl = newUrl;
      }
    }

    if (DEBUG_URLCONN) printf("got here - 3\n");

    // summary of url connection
    if (DEBUG_URLCONN) printf("\n");
    if (DEBUG_URLCONN) printf("summary of url connection\n");
    if (DEBUG_URLCONN) printf("urlconn->responseCode: %d\n",urlconn->responseCode);
    if (DEBUG_URLCONN) printf("urlconn->fullUrl: %s\n",urlconn->fullUrl);
    if (DEBUG_URLCONN) printf("urlconn->contentType: %s\n",urlconn->contentType);
    if (DEBUG_URLCONN) printf("urlconn->newURL: %s\n",urlconn->newUrl);
    if (DEBUG_URLCONN) printf("\n");

  }
  // clean everything curl-related
  if (DEBUG_URLCONN) printf("Cleaning up curl\n");
  curl_easy_cleanup(curl);
  if (DEBUG_URLCONN) printf("Cleaned up curl\n");
}

void URLConnectionDispose(urlconnection* urlconn)
{
  if (DEBUG_URLCONN) printf("freeing up URLConnection\n");
  if (urlconn->fullUrl != NULL) {
    if (DEBUG_URLCONN) printf("urlconn->fullUrl = %s\n",urlconn->fullUrl);
    free((void *)urlconn->fullUrl);
  }

  if (DEBUG_URLCONN) printf(".. 2");
  if (urlconn->responseMessage != NULL) {
    if (DEBUG_URLCONN) printf("urlconn->responseMessage = %s\n",urlconn->responseMessage);
    free((void *)urlconn->responseMessage);
  }
/*
  if (DEBUG_URLCONN) printf(".. 3");
  if (urlconn->newUrl != NULL) {
    if (DEBUG_URLCONN) printf("urlconn->newUrl = %s\n",urlconn->newUrl);
    free((void *)urlconn->newUrl);
  }
*/
  if (DEBUG_URLCONN) printf(" .. closing dataStream\n");
  fclose(urlconn->dataStream);
  if (DEBUG_URLCONN) printf("freed up URL Connection\n");
}
