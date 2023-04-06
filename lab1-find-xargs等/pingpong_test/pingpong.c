#include <stdio.h>
#include <stdlib.h>


#define R 0
#define W 1

const int W_N = 4;
const int R_N = 5;


void smhand()
{
  for(int i = 0;i < 10;++i)
  {
    printf(".");
    sleep(1);
  }
  printf("\n");

}

int main(int agrc,char *argv[])
{

  int p2c[2];
  int c2p[2];
  char buffer[32];
  pipe(p2c);
  pipe(c2p);

  int pid = fork();
  if(pid == 0)
  {//孩子
    close(p2c[W]);
    int num = read(p2c[R],buffer,R_N);
    buffer[num]='\0';
    printf("child read:%d\n",num);
    printf("child%d:%s\n",getpid(),buffer);

    close(c2p[R]);
    write(c2p[W],"pong",W_N);
    
    // smhand();

    num = read(p2c[R],buffer,R_N);
    buffer[num]='\0';
    printf("child read:%d\n",num);
    printf("child%d:%s\n",getpid(),buffer);
    close(p2c[R]);

    write(c2p[W],"pon2",W_N);
    close(c2p[W]);
    exit(0);
  }else
  {//父亲

    close(p2c[R]);
    write(p2c[W],"ping",W_N);

    close(c2p[W]);
    int num = read(c2p[R],buffer,R_N);
    buffer[num]='\0';
    printf("parent read:%d\n",num);
    printf("parent%d:%s\n",getpid(),buffer);

    
    
    write(p2c[W],"pin2",W_N);
    close(p2c[W]);

    num = read(c2p[R],buffer,R_N);
    buffer[num]='\0';
    printf("parent read:%d\n",num);
    close(c2p[R]);
    printf("parent%d:%s\n",getpid(),buffer);
  }
  return 0;
}