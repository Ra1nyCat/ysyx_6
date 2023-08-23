/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <common.h>
#include "monitor/sdb/sdb.h"

void init_monitor(int, char *[]);
void am_init_monitor();
void engine_start();
int is_exit_status_bad();

void test_expr()
{
  //读取文件
  FILE *fp = fopen("/home/yanxy/ysyx/ysyx-workbench/nemu/tools/gen-expr/build/input", "r");
  assert(fp != NULL);
  char buf[65536]={};

  int y=0,n=0;
  while(true)
  {
    memset(buf,0,65536);
    char *eof=fgets(buf, 65536, fp);
    if(eof==NULL&&feof(fp))break;

     //读取结果
    char *res=strtok(buf," ");
    assert(res!=NULL);
    uint32_t ans;
    sscanf(res,"%u",&ans);

    char *expr_str=buf+strlen(res)+1;
    int len=strlen(expr_str);
    if(len>0&&expr_str[len-1]=='\n')expr_str[len-1]=' ';
    bool success=true;
    uint32_t ans2=expr(expr_str,&success);

    if(success){
      if(ans==ans2)y++;
      else {
        puts("=========================");
        printf("wrong expr:\n---%s\n",expr_str);
        printf("%u %u\n",ans,ans2);
        puts("=========================\n");
        n++;
      }
    }else{
      printf("Expr has bug\n");
    }
  }
  printf("test result: %d %d\n",y,n);
  fclose(fp);
  return;
}

int main(int argc, char *argv[]) {
  /* Initialize the monitor. */
#ifdef CONFIG_TARGET_AM
  am_init_monitor();
#else
  init_monitor(argc, argv);
#endif

// #ifdef EXPR_TEST
//   test_expr();
//   return 0;
// #else
//   /* Start engine. */
//   engine_start();
//   return is_exit_status_bad();
// #endif
    /* Start engine. */
  engine_start();
  return is_exit_status_bad();
}
