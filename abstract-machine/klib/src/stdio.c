#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

char* itoa(int num,int base){
  static char buf[32]={};
  int i=30;
  for(;num&&i;i--,num/=base)
    buf[i]="0123456789abcdef"[num%base];
  return &buf[i+1];
}


int printf(const char *fmt, ...) {
  // panic("Not implemented");
  int n=0;
  size_t size=0;

  char* p=NULL;

  va_list ap;
  va_start(ap,fmt);
  while(*fmt){
    if(*fmt!='%'){
      putch(*fmt);
      fmt++;
      n++;
    }
    else{
      fmt++;
      switch(*fmt){
        case 'd':{
          int num=va_arg(ap,int);
          if(num<0){
            num=-num;
            putch('-');
            n++;
          }
          p=itoa(num,10);
          size=strlen(p);
          for(int i=0;i<size;i++){
            putch(p[i]);
            n++;
          }
          fmt++;
          break;
        }
        case 'x':{
          int num=va_arg(ap,int);
          p=itoa(num,16);
          size=strlen(p);
          for(int i=0;i<size;i++){
            putch(p[i]);
            n++;
          }
          fmt++;
          break;
        }
        case 's':{
          char* str=va_arg(ap,char*);
          size=strlen(str);
          for(int i=0;i<size;i++){
            putch(str[i]);
            n++;
          }
          fmt++;
          break;
        }
        case 'c':{
          char ch=va_arg(ap,int);
          putch(ch);
          n++;
          fmt++;
          break;
        }
        default:{
          putch(*fmt);
          n++;
          fmt++;
          break;
        }
      }
    }
  }
  va_end(ap);
  return n;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  int n=0;
  size_t size=0;

  char* p=NULL;
  //实现vsprintf
  while(*fmt){
    if(*fmt!='%'){
      *out=*fmt;
      out++;
      fmt++;
      n++;
    }
    else{
      fmt++;
      switch(*fmt){
        case 'd':{
          int num=va_arg(ap,int);
          if(num<0){
            num=-num;
            *out='-';
            out++;
            n++;
          }
          p=itoa(num,10);
          size=strlen(p);
          for(int i=0;i<size;i++){
            *out=p[i];
            out++;
            n++;
          }
          fmt++;
          break;
        }
        case 'x':{
          int num=va_arg(ap,int);
          p=itoa(num,16);
          size=strlen(p);
          for(int i=0;i<size;i++){
            *out=p[i];
            out++;
            n++;
          }
          fmt++;
          break;
        }
        case 's':{
          char* str=va_arg(ap,char*);
          size=strlen(str);
          for(int i=0;i<size;i++){
            *out=str[i];
            out++;
            n++;
          }
          fmt++;
          break;
        }
        case 'c':{
          char ch=va_arg(ap,int);
          *out=ch;
          out++;
          n++;
          fmt++;
          break;
        }
        default:{
          *out=*fmt;
          out++;
          n++;
          fmt++;
          break;
        }
      }
    }
  }
  *out='\0';
  return n;

}

int sprintf(char *out, const char *fmt, ...) {
  va_list ap;
  va_start(ap,fmt);
  int n=vsprintf(out,fmt,ap);
  va_end(ap);
  return n;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  //实现snprintf
  va_list ap;
  va_start(ap,fmt);
  int tot=vsnprintf(out,n,fmt,ap);
  va_end(ap);
  return tot;
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  int tot=0;
  for(int i=0;fmt[i];i++){
    if(fmt[i]!='%'){
      if(tot<n){
        out[tot]=fmt[i];
        tot++;
      }
    }
    else{
      i++;
      switch(fmt[i]){
        case 'd':{
          int num=va_arg(ap,int);
          char* p=itoa(num,10);
          int size=strlen(p);
          for(int j=0;j<size;j++){
            if(tot<n){
              out[tot]=p[j];
              tot++;
            }
          }
          break;
        }
        case 'x':{
          int num=va_arg(ap,int);
          char* p=itoa(num,16);
          int size=strlen(p);
          for(int j=0;j<size;j++){
            if(tot<n){
              out[tot]=p[j];
              tot++;
            }
          }
          break;
        }
        case 's':{
          char* str=va_arg(ap,char*);
          int size=strlen(str);
          for(int j=0;j<size;j++){
            if(tot<n){
              out[tot]=str[j];
              tot++;
            }
          }
          break;
        }
        case 'c':{
          char ch=va_arg(ap,int);
          if(tot<n){
            out[tot]=ch;
            tot++;
          }
          break;
        }
        default:{
          if(tot<n){
            out[tot]=fmt[i];
            tot++;
          }
          break;
        }
      }
    }
    if(tot>=n)break;
  }
  if(tot<n)out[tot]='\0';
  else out[n-1]='\0';
  return tot;
}

#endif
