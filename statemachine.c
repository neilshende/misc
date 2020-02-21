#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <ctype.h>
int debug = 0;

int issign(char x) {
  return (x =='+' || x == '-');
}

typedef struct machine_context machine_context;
typedef int (*SM) (machine_context *ctx);
typedef struct machine_context {
char *str;
char *ptr;
char *begin;
char *end;
SM   next;
} machine_context;

int start (machine_context *ctx);
int success (machine_context *ctx);
int failure (machine_context *ctx);
int alpha (machine_context *ctx);
int digit (machine_context *ctx);
int sign (machine_context *ctx);

char *string ="Thisisatest++911-600abcd673pqrs";
char *argv1 = NULL;
int start (machine_context *ctx) 
{
   ctx->ptr=argv1?argv1:string;
   char *ptr=ctx->ptr;
   if (isdigit(*ptr)) {ctx->next = &digit; ctx->begin = ptr; return 0;}
   else if (isalpha(*ptr)) ctx->next = &alpha;
   else if (issign(*ptr)) {ctx->next = &sign; ctx->begin = ptr; return 0;}
   else ctx->next=&failure;
   (ctx->ptr)++;
   ctx->begin = ctx->end = NULL;
   return 0;
}

int success (machine_context *ctx)
{
   ctx->next=NULL;
   if (debug) printf("SUCCESS! _%s _%s\n", ctx->begin, ctx->end);
   char *x = (char *)malloc(ctx->end - ctx->begin + 1);
   x[ctx->end - ctx->begin] = '\0';
   memcpy(x, ctx->begin, ctx->end-ctx->begin);
   printf("%s\n",x);
   free(x);
   return 0;
}

int failure (machine_context *ctx)
{
   ctx->next=NULL;
   printf("Failed to find int\n");
   return 0;
}

int alpha (machine_context *ctx)
{
   char *ptr=ctx->ptr;
   if (isdigit(*ptr)) {ctx->next = &digit; ctx->begin = ptr; }
   else if (isalpha(*ptr)) {ctx->next = &alpha; ctx->begin = ctx->end = NULL; }
   else if (issign(*ptr)) {ctx->next = &sign; ctx->begin = ptr ;}
   else ctx->next=&failure;
   (ctx->ptr)++;
   return 0;
}

int digit (machine_context *ctx)
{
   char *ptr=ctx->ptr;
   if (isdigit(*ptr)) {ctx->next = &digit; if (ctx->begin == NULL) ctx->begin = ptr;}
   else {ctx->next=&success; ctx->end = ptr; return 0;}
   (ctx->ptr)++;
   return 0;
}

int sign (machine_context *ctx)
{
   char *ptr=ctx->ptr;
   if (isdigit(*ptr)) {ctx->next = &digit; if (ctx->begin == NULL) ctx->begin = ptr;}
   else if (isalpha(*ptr)) ctx->next = &alpha;
   else if (issign(*ptr)) {ctx->next = &sign; ctx->begin = ptr; }
   else ctx->next=&failure;
   (ctx->ptr)++;
   return 0;
}


int schedule() {
   machine_context CTX;
   memset(&CTX, 0, sizeof(CTX));
   CTX.next = &start;

   while (1) {
      if (CTX.next != NULL) {
         if (debug) {
            printf("ptr=%1s\n",(CTX.ptr));
         }
         CTX.next(&CTX);
      } else {
         if (debug) printf("Ending SM\n");
         break;
      }
   }
   return 0;
}

int main(int argc, char *argv[]) {
   debug = argc > 2;
   argv1 = argv[1];
   schedule();
   return 0;
}
