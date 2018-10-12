#include "url.h"
#include "assert.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define DEBUG_URL 0


int find(const char *substring, const char *str, int index)
{
  assert(str != NULL && substring != NULL);

  if (index > strlen(str)) {
    return -1;
  }
   
  char *result = strstr(str + index, substring);
  if (result == NULL)
    return -1;
  int position = result - str;
  return position;
}


char* substring(const char* str, int begin, int len) 
{ 
  assert(str != NULL);

  if (strlen(str) == 0 || strlen(str) < begin || strlen(str) < (begin+len)) 
    return NULL; 

  return strndup(str + begin, len); 
} 


void URLNewAbsolute(url *u, const char *absolutePath)
{
  char *tempString = NULL;
  int pos1 = 0, pos2 = 0;

  assert(absolutePath != NULL);
  assert(strlen(absolutePath) > 2);

  if (DEBUG_URL) printf("building url from %s\n",absolutePath);

  // determine the port number (default to be 80)
  u->port = 80;
  pos1 = find(":", absolutePath, 0);
  if (pos1 > 0) {
    tempString = substring(absolutePath, 0, pos1);
    assert(tempString != NULL);

    if (DEBUG_URL) printf("tempString: %s\n",tempString);
    if (strcmp(tempString,"https") == 0) {
      u->port = 443;
    }
    free(tempString);
  } else {
    pos1 = -3;
  }
  if (DEBUG_URL) printf("    pos1 = %d\n",pos1);

  // extract the fullName
  tempString = substring(absolutePath, pos1+3, strlen(absolutePath)-(pos1+3));
  if (tempString == NULL) {
    if (DEBUG_URL) printf("pos1: %d, length: %lu, absolutePath: %s\n",pos1, strlen(absolutePath), absolutePath);
  }
  assert(tempString != NULL);
  u->fullName = strdup(tempString);
  free(tempString);

  // extract the serverName
  pos2 = find("/", absolutePath, pos1 + 3);
  if (pos2 == -1)
    pos2 = strlen(absolutePath);
  if (DEBUG_URL) printf("    pos2 = %d\n",pos2);
  
  tempString = substring(absolutePath, pos1 + 3, pos2 - (pos1 + 3));
  assert(tempString != NULL);
  u->serverName = strdup(tempString);
  free(tempString);

  // extract the fileName
  tempString = substring(absolutePath, pos2, strlen(absolutePath)-(pos2));
  assert(tempString != NULL);
  u->fileName = strdup(tempString);
  free(tempString);

  if (DEBUG_URL) printf("    strlen(absolutePath) = %lu\n",strlen(absolutePath));

  if (DEBUG_URL) printf("fullName: %s\n",u->fullName);
  if (DEBUG_URL) printf("fileName: %s\n",u->fileName);
  if (DEBUG_URL) printf("serverName: %s\n",u->serverName);
  if (DEBUG_URL) printf("port: %d\n",u->port);
}


void URLNewRelative(url *u, const url *parentURL, const char *relativePath)
{
  // determine if relativePath is actually an absolutePath
  // ie does it have a "." in the path
  if (DEBUG_URL) printf("entered URLNewRelative\n");
  int pos = find(".", relativePath, 0);
  if (DEBUG_URL) printf("pos: %d\n",pos);
  if (pos > 0) {
    if (DEBUG_URL) printf("creating NewAbsolute\n");
    URLNewAbsolute(u, relativePath);
  } else {
    if (DEBUG_URL) printf("creating NewAbsolute\n");

    char *newURL = malloc(strlen(parentURL->serverName) + strlen(relativePath));
    strcpy(newURL, parentURL->serverName);

    if (DEBUG_URL) printf("strlen(newURL) = %lu\n",strlen(newURL));
    if (DEBUG_URL) printf("sizeof(newURL) = %lu\n",sizeof(newURL));

    strncat(newURL, relativePath, sizeof(newURL) - strlen(newURL) - 1);

    URLNewAbsolute(u, relativePath);
  }
}


void URLDispose(url *u)
{
  if (u->fileName != NULL)
    free((void *)u->fileName);
  if (u->serverName != NULL)
    free((void *)u->serverName);
  if (u->fullName != NULL)
    free((void *)u->fullName);
}
