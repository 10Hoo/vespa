// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
/**
 *
 * @file url.cpp
 * @author Michael Sus�g
 * @date Creation date: 1999-11-23
 * @version $Id$
 *
 * This file contains different URL string functions
 *
 * Copyright (c)        : 1997-2000 Fast Search & Transfer ASA.
 *                        ALL RIGHTS RESERVED
 *
 */

/**
 * Encode and decode a URL
 */

#include <vespa/fastos/fastos.h>
#include "url.h"

void
Fast_URL::decode(const char *encodedURL, char *decodedURL, int bufsize)
{
  const char *tmpPtr;
  unsigned int charVal;
  char *bufend = decodedURL + bufsize;

  tmpPtr = encodedURL;

  /* Parse the whole encodedURL */
  while(decodedURL < bufend && *tmpPtr != '\0') {
    /* Check if an encoded character is the next one */
    if(*tmpPtr == '%') {
      tmpPtr++; /* Skip % character */
      sscanf(tmpPtr,"%02X", &charVal);
      *decodedURL = static_cast<char>(charVal);
      tmpPtr += 2;
    }
    else
    {
      *decodedURL = *tmpPtr;
      tmpPtr++;
    }
    decodedURL++;
  }
  if (decodedURL < bufend)
    *decodedURL = '\0';
}

int Fast_URL::DecodeQueryString(char *queryString)
{
  int numReplaced = 0;

  for(int i=0; queryString[i] != '\0'; i++)
  {
    if (queryString[i] == '+')
    {
      queryString[i] = ' ';
      numReplaced ++;
    }
  }

  return numReplaced;
}
