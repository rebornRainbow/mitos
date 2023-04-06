<style>

.be{
  background: linear-gradient(to right, red, blue);
  -webkit-background-clip: text;
  color: transparent;
}
</style>


## 问题1 完成sleep

### 1 文件在user/sleep.c中
仿造这些可以获得参数
user/echo.c, user/grep.c, and user/rm.c)
用户态的程序需要加在makefile中
U 那一段

### 2 总结
完成但是他的atoi函数没有检查错误的机制。

## 问题2 pingpong.c
Write a program that uses UNIX system calls to ''ping-pong'' 
a byte between two processes over a pair of pipes, 
one for each direction. The parent should send a byte to the child; 
the child should print "<pid>: received ping", 
where <pid> is its process ID, write the byte on 
the pipe to the parent, and exit; 
the parent should read the byte from the child, 
print "<pid>: received pong", 
and exit. Your solution should be in the file user/pingpong.c.

### 出现的bug
1 首先是声明的所有变量必须要在fork()之前
不然子程序和父程序之间的文件就没有任何的关系了
尤其是在使用管道的时候，不然会出现你自己都想不到的错误

2 对于所有文件描述符,端口等
在文件前声明一个常量表示，不然如果某一个出错难以发觉。

3 读取之后及时将缓冲区的最后一个加上终止符号
### 总结
注意以上两个问题

## 问题3
primes (moderate)/(hard)
Write a concurrent version of prime sieve using pipes.
 This idea is due to Doug McIlroy, inventor of Unix pipes. 
 The picture halfway down this page and the surrounding 
 text explain how to do it. 
 Your solution should be in the file user/primes.c.

Your goal is to use pipe and fork to set up the pipeline. 
The first process feeds the numbers 2 through 35 into 
the pipeline. For each prime number, you will arrange 
to create one process that reads from its left neighbor 
over a pipe and writes to its right neighbor over 
another pipe. Since xv6 has limited number of 
file descriptors and processes, the first process can stop at 35.

Some hints:
  
### 实验问题 执行中会随机出现的符号
原因，因为我刚开始的设计是以read来控制输出顺序
但是最初的父进程执行完成由于没有read()阻塞或者其他控制
直接结束了，导致子进程成了僵尸进程或者是后台进程，
输出中会出现表示最初的进程结束
办法加上等待子进程结束,
由于是在末尾加上的wait(),不会影响效率。

### 顺利完成


## find 查找文件

### 1 memmove(str1,str2,len)
C 库函数 void *memmove(void *str1, const void *str2, size_t n) 
从 str2 复制 n 个字符到 str1，但是在<span class = "be">重叠内存
</span>块这方面，memmove() 是比 memcpy() 更安全的方法。如果目标区
域和源区域有重叠的话，memmove() 能够保证源串在被覆盖之前将重叠区域
的字节拷贝到目标区域中，复制后源区域的内容会被更改。如果目标区域
与源区域没有重叠，则和 memcpy() 函数功能相同。


### fstat 可以查看自己文件的信息
[fstat参考](https://blog.csdn.net/qq_21792169/article/details/50454475)


## xargs自己完成的命令
xargs (moderate)
Write a simple version of the UNIX xargs program: 
its arguments describe a command to run, it 
reads lines from the standard input, and it 
runs the command for each line, appending the 
line to the command's arguments. Your 
solution should be in the file user/xargs.c.

The following example illustrates xarg's behavior:
     echo hello too | xargs echo bye
    bye hello too
    
  
Note that the command here is "echo bye" and the 
additional arguments are "hello too", making the 
command "echo bye hello too", which outputs "bye hello too".
Please note that xargs on UNIX makes an optimization 
where it will feed more than argument to the 
command at a time. We don't expect you to make 
this optimization. To make xargs on UNIX behave 
the way we want it to for this lab, please 
run it with the -n option set to 1. For instance

     (echo 1 ; echo 2) | xargs -n 1 echo
    1
    2
    
  
Some hints:

Use fork and exec to invoke the command on each 
line of input. Use wait in the parent to wait 
for the child to complete the command.
To read individual lines of input, read a 
character at a time until a newline ('\n') appears.
kernel/param.h declares MAXARG, which may 
be useful if you need to declare an argv array.
Add the program to UPROGS in Makefile.
Changes to the file system persist across 
runs of qemu; to get a clean file system 
run make clean and then make qemu.
xargs, find, and grep combine well:

   find . b | xargs grep hello
  
will run "grep hello" on each file named b 
in the directories below ".".
To test your solution for xargs, run the 
shell script xargstest.sh. Your solution is 
correct if it produces the following output:

   make qemu
  ...
  init: starting sh
   sh < xargstest.sh
        hello
  hello
  hello
   
  
You may have to go back and fix bugs in your 
find program. The output has many  because 
the xv6 shell doesn't realize it is processing 
commands from a file instead of from the 
console, and prints a  for each command in the file.

### 开发流程需要将程序划分为小的步骤，实现一个小的里程碑
完成好每一步骤才能执行好一个好的程序
尤其是面对一个复杂的程序的时候。
以一个正确的程序为基础才能开发一个正确的程序

### 在他开发的risv中有一个漏洞
不要直接使用原先的argv,可以使用指针来指向，但是不可以直接使用会出错。

### 完美实现

== Test sleep, no arguments == 
 make qemu-gdb
sleep, no arguments: OK (4.6s) 
== Test sleep, returns == 
 make qemu-gdb
sleep, returns: OK (0.9s) 
== Test sleep, makes syscall == 
 make qemu-gdb
sleep, makes syscall: OK (1.0s) 
== Test pingpong == 
 make qemu-gdb
pingpong: OK (1.0s) 
== Test primes == 
 make qemu-gdb
primes: OK (1.1s) 
== Test find, in current directory == 
 make qemu-gdb
find, in current directory: OK (1.2s) 
== Test find, recursive == 
 make qemu-gdb
find, recursive: OK (1.2s) 
== Test xargs == 
 make qemu-gdb
xargs: OK (1.1s) 
== Test time == 
time: OK 
Score: 100/100
