#include <stdio.h>
#include <stdlib.h>

typedef unsigned long int uint64;

#define PGSIZE 4096 // bytes per page
#define PGSHIFT 12  // bits of offset within a page

uint64 PGROUNDUP(uint64 sz)  
{
  return (((sz)+PGSIZE-1) & ~(PGSIZE-1));
}
#define k(a) (((a)) & ~(PGSIZE-1))
uint64 PGROUNDDOWN(uint64 a) 
{
  return (((a)) & ~(PGSIZE-1));
}
uint64 PA_GET_INDEX(uint64 pa) 
{
  return  ( PGROUNDDOWN ( (uint64) pa )/4096 - 524322);
}

#define KERNBASE 0x80000000L
#define PHYSTOP (KERNBASE + 128*1024*1024)
#define PTE_COW (1L << 8)// 标记这是否是cow页面
#define PTE_OW (1L << 9)//标记这原本是否是可写入页面
int main()
{

  uint64 num = 31;
  // while(1)
  {
    // scanf("%llu",&31);
    printf("%llu,index:%p\n",num, (num + 524322)*4096);
    // printf("%d,down:%d\n",num,PGROUNDDOWN(12));
    printf("%d\n",(pa&(PTE_OW|PTE_COW)));
  }
  return 0;
}