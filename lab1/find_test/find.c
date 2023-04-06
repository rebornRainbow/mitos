#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>  
#include <uninstd.h>

#define DIRSIZ 14


char* fmtname(char *path)
{
  static char buf[DIRSIZ+1];
  char *p;

  // Find first character after last slash.
  for(p=path+strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;

  // Return blank-padded name.
  if(strlen(p) >= DIRSIZ)
    return p;
  memmove(buf, p, strlen(p));
  memset(buf+strlen(p), '\0', DIRSIZ-strlen(p));
  return buf;
}

void find(char *path, char *target)
{
  char buf[512], *p;
  int fd;
  struct dirent de;
  struct stat st;

  if((fd = open(path, 0)) < 0){
    fprintf(2, "ls: cannot open %s\n", path);
    return;
  }

  if(fstat(fd, &st) < 0){
    fprintf(2, "ls: cannot stat %s\n", path);
    close(fd);
    return;
  }

  switch(st.type){
  case T_DEVICE:
  case T_FILE:
    //如果遇到的是文件
    if(strcmp(fmtname(path),target) == 0)
    {
      printf("%s\n",path);
    }
    break;

  case T_DIR:
    //文件夹
    if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
      printf("ls: path too long\n");
      break;
    }
    strcpy(buf, path);
    p = buf+strlen(buf);
    *p++ = '/';
    
    while(read(fd, &de, sizeof(de)) == sizeof(de)){
      if(de.inum == 0)
        continue;
      memmove(p, de.name, DIRSIZ);
      p[DIRSIZ] = 0;
      if(stat(buf, &st) < 0){
        printf("ls: cannot stat %s\n", buf);
        continue;
      }
      char *name_t = fmtname(buf);
      if(strcmp(name_t, ".")== 0 || strcmp(name_t, "..") == 0)
      {
        continue;
      }
      find(buf,target);
    }
    break;
  }
  close(fd);

}


int main(int argc,char*argv[])
{
  if(argc != 3)
    printf("参数错误find <路径> <目标>\n");
  find(argv[1],argv[2]);
  return 0;
}