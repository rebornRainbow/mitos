#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>


void cout(char **argv)
{
  int i = 0;
  while(argv[i])
  {
    printf("%s|",argv[i++]);
  }
  printf("\n");

}

//读取一个参数
char *readp()
{
  static char buffer[1024];
  int bufi = 0;
  
  char tem = ' ';
  while(tem != '\n')
  {
    int num = read(0,&tem,1);
    if(num == EOF || tem == ' ')
    {
      return 0;
    }
    buffer[bufi++] = tem;
    // printf("bufi=%d,%c\n",bufi,tem);
    // sleep(1);
  }
  buffer[--bufi] = '\0';
  return buffer;
}

int xagrs(int argc,char *argv[])
{
  char * p = readp();
  if(p == 0)
    return -1;
  // printf("%s\n",p);
  if(fork() == 0)
  {
    argv[argc] = p;
    argv[argc+1] = 0;
    ++argv;
    // cout(argv);
    int err = execv("/usr/bin/grep",argv);
    printf("fail execv!:%d\n",err);
    exit(-1);
  }else
  {
    wait(0);
  }
 
}


int main(int argc,char*argv[])
{

  int num = 0;
  while(num != -1)
  {
    num = xagrs(argc,argv);
  }
  return 0;

}