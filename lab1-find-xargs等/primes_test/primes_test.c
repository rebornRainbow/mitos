#include <stdio.h>
#include <stdlib.h>


#define R 0
#define W 1


char w_num[35][3] ={
    "2","3","4","5","6","7","8","9",
    "10","11","12","13","14","15","16",
    "17","18","19","20","21","22","23",
    "24","25","26","27","28","29","30",
    "31","32","33","34","35","36",};


const int W_N = 2;
const int R_N = 2;

void create(int * p2c)
{
  close(p2c[W]);
  char buffer[35];
  int target = 0;
  int wnum = read(p2c[R],buffer,R_N);
  // printf("读取到的%d\n",wnum);
  if(wnum != 0 && wnum != -1)
  {
    buffer[wnum] = '\0';
    target = atoi(buffer);
    printf("prime %d\n",target);
    
  }else
  {
    close(p2c[R]);
    return;
  }
  int tem[2];
  pipe(tem);

  if(fork() != 0)
  { 
    close(tem[R]);
    while(wnum != 0 && wnum != -1)
    {
      int wnum = read(p2c[R],buffer,R_N);
      if(wnum == 0 || wnum == -1)
      {
        // close(p2c[R]);
        break;
      }
      buffer[wnum] = '\0';
      int rnum = atoi(buffer);
      if(rnum % target != 0)
      {
        write(tem[W],buffer,W_N);
      }
    }
    close(p2c[R]); 
    close(tem[W]);
    wait(0);

  }else
  {
    close(p2c[R]);
    create(tem);
  }
}

int main()
{
  int p2c[2];
  pipe(p2c);
  
  if(fork() != 0)
  { 
    close(p2c[R]);
    for(int i = 2;i < 35;++i)
    {
      if(i < 10)
      {
        
        write(p2c[W],w_num[i-2],W_N);
      }else
      {
        write(p2c[W],w_num[i-2],W_N);
      }
    }
    close(p2c[W]);
    wait(0);
  }else
  {
    create(p2c);
  }
  
  return 0;
}