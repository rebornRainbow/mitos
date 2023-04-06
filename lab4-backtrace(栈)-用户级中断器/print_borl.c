#include <stdio.h>
#include <stdlib.h>


int main()
{
  unsigned int i = 0x00646c72;
  unsigned int tem = 4096;
  // 0 ,d,l,r
	printf("H%x Wo%s\n", 57616, &i);
  printf("%p\n",~(tem-1));
  exit(0);
}