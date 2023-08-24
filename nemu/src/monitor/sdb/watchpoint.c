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

#include "sdb.h"

#define NR_WP 32

#define WHAT_LEN 256

typedef struct watchpoint {
  int NO;
  bool Enb; //是否启用
  char What[WHAT_LEN];
  word_t val;
  
  struct watchpoint *next;

  /* TODO: Add more members if necessary */
} WP;

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
  }

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */
void print_wp(){
  WP *p=head;
  while(p!=NULL){
    printf("watchpoint %d: enbale:%d  expr:%s\n",p->NO,p->Enb,p->What);
    p=p->next;
  }
}

WP* new_wp(){
  if(free_==NULL){
    assert(0);
    return NULL;
  }
  WP *p=free_;
  free_=free_->next;
  p->next=head;
  head=p;
  return p;
}

void free_wp(WP* wp){
  if(wp==NULL)return;
  WP *p=head;
  if(p==wp){
    head=head->next;
    wp->next=free_;
    free_=wp;
    return;
  }
  while(p->next!=NULL){
    if(p->next==wp){
      p->next=wp->next;
      wp->next=free_;
      free_=wp;
      return;
    }
    p=p->next;
  }
}

void free_wp_idx(int n)
{
  WP *p=head;
  while(p!=NULL){
    if(p->NO==n){
      free_wp(p);
      return;
    }
    p=p->next;
  }
  printf("No such watchpoint!\n");
}

void set_wp(char *e)
{
  word_t val=0;
  int status=check_expr(e,&val);
  if(status==-1){
    printf("Invalid expression:%s!\n",e);
    return;
  }else if(status==0){
    printf("Cannot watch constant value %s!\n",e);
    return;
  }
  WP *p=new_wp();
  strncpy(p->What,e,strlen(e));
  p->Enb=true;
  p->val=val;
}

bool watchpoint_check()
{
  WP *p=head;
  bool flag=false;
  while(p!=NULL){
    if(p->Enb){
      word_t val=expr(p->What,NULL);
      if(val!=p->val){
        printf("Watchpoint %d: %s\n",p->NO,p->What);
        printf("Old value = %d\n",p->val);
        printf("New value = %d\n",val);
        p->val=val;
        flag=true;
      }
    }
    p=p->next;
  }
  return flag;
}
