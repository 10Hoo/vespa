// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
#include <stdio.h>
#include <string.h>

#include <vespa/fastos/app.h>

class BaseTest : public FastOS_Application
{
private:
   BaseTest(const BaseTest&);
   BaseTest &operator=(const BaseTest&);

   int totallen;
   bool _allOkFlag;
public:
   const char *okString;
   const char *failString;

   BaseTest ()
     : totallen(70),
       _allOkFlag(true),
       okString("SUCCESS"),
       failString("FAILURE")
   {
   }

   virtual ~BaseTest() {};

   bool allWasOk() const { return _allOkFlag; }

   void PrintSeparator ()
   {
      for(int i=0; i<totallen; i++) printf("-");
      printf("\n");
   }

   virtual void PrintProgress (char *string)
   {
      printf("%s", string);
   }
#define MAX_STR_LEN 3000
   bool Progress (bool result, const char *str)
   {
      char string[MAX_STR_LEN];
      sprintf(string, "%s: %s\n",
         result ? okString : failString, str);
      PrintProgress(string);
      if (! result) { _allOkFlag = false; }
      return result;
   }

   bool Progress (bool result, const char *str, int d1)
   {
      char string[MAX_STR_LEN];
      sprintf(string, "%s: ", result ? okString : failString);
      sprintf(&string[strlen(string)], str, d1);
      sprintf(&string[strlen(string)], "\n");
      PrintProgress(string);
      if (! result) { _allOkFlag = false; }
      return result;
   }

   bool Progress (bool result, const char *str, int d1, int d2)
   {
      char string[MAX_STR_LEN];
      sprintf(string, "%s: ", result ? okString : failString);
      sprintf(&string[strlen(string)], str, d1, d2);
      sprintf(&string[strlen(string)], "\n");
      PrintProgress(string);
      if (! result) { _allOkFlag = false; }
      return result;
   }

   bool Progress (bool result, const char *str, const char *s1)
   {
      char string[MAX_STR_LEN];
      sprintf(string, "%s: ", result ? okString : failString);
      sprintf(&string[strlen(string)], str, s1);
      sprintf(&string[strlen(string)], "\n");
      PrintProgress(string);
      if (! result) { _allOkFlag = false; }
      return result;
   }

   bool Progress (bool result, const char *str, const FastOS_ThreadInterface *s1)
   {
      char string[MAX_STR_LEN];
      sprintf(string, "%s: ", result ? okString : failString);
      sprintf(&string[strlen(string)], str, s1);
      sprintf(&string[strlen(string)], "\n");
      PrintProgress(string);
      if (! result) { _allOkFlag = false; }
      return result;
   }

   bool Progress (bool result, const char *str, const FastOS_Socket *s1)
   {
      char string[MAX_STR_LEN];
      sprintf(string, "%s: ", result ? okString : failString);
      sprintf(&string[strlen(string)], str, s1);
      sprintf(&string[strlen(string)], "\n");
      PrintProgress(string);
      if (! result) { _allOkFlag = false; }
      return result;
   }

   bool Progress (bool result, const char *str, const char *s1, const char *s2)
   {
      char string[MAX_STR_LEN];
      sprintf(string, "%s: ", result ? okString : failString);
      sprintf(&string[strlen(string)], str, s1, s2);
      sprintf(&string[strlen(string)], "\n");
      PrintProgress(string);
      if (! result) { _allOkFlag = false; }
      return result;
   }

   bool Progress (bool result, const char *str, const char *s1, int d1)
   {
      char string[MAX_STR_LEN];
      sprintf(string, "%s: ", result ? okString : failString);
      sprintf(&string[strlen(string)], str, s1, d1);
      sprintf(&string[strlen(string)], "\n");
      PrintProgress(string);
      if (! result) { _allOkFlag = false; }
      return result;
   }

   bool Progress (bool result, const char *str, int d1, const char *s1)
   {
      char string[MAX_STR_LEN];
      sprintf(string, "%s: ", result ? okString : failString);
      sprintf(&string[strlen(string)], str, d1, s1);
      sprintf(&string[strlen(string)], "\n");
      PrintProgress(string);
      if (! result) { _allOkFlag = false; }
      return result;
   }

   bool ProgressI64 (bool result, const char *str, int64_t val)
   {
      char string[MAX_STR_LEN];
      sprintf(string, "%s: ", result ? okString : failString);
      sprintf(&string[strlen(string)], str, val);
      sprintf(&string[strlen(string)], "\n");
      PrintProgress(string);
      if (! result) { _allOkFlag = false; }
      return result;
   }

   bool ProgressFloat (bool result, const char *str, float val)
   {
      char string[MAX_STR_LEN];
      sprintf(string, "%s: ", result ? okString : failString);
      sprintf(&string[strlen(string)], str, val);
      sprintf(&string[strlen(string)], "\n");
      PrintProgress(string);
      if (! result) { _allOkFlag = false; }
      return result;
   }

   void Ok (const char *string)
   {
      Progress(true, string);
   }

   void Fail (const char *string)
   {
      Progress(false, string);
   }

   void TestHeader (const char *string)
   {
      int len = strlen(string);
      int leftspace = (totallen - len)/2 - 2;
      int rightspace = totallen - 4 - len - leftspace;
      int i;

      printf("\n\n");
      for(i=0; i<totallen; i++) printf("*");
      printf("\n**");
      for(i=0; i<leftspace; i++) printf(" ");   //forgot printf-specifier..
      printf("%s", string);
      for(i=0; i<rightspace; i++) printf(" ");
      printf("**\n");
      for(i=0; i<totallen; i++) printf("*");
      printf("\n");
   }
};
