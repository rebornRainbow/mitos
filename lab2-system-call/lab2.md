## lab2

<style>
  .title{
     background: -webkit-linear-gradient(#05649b, #02e297);
    -webkit-background-clip: text;
    -webkit-text-fill-color: transparent;
  }
  body{
    background:grey;
  }
</style>

### system call
System call tracing (moderate)
In this assignment you will add a system call tracing
feature that may help you when debugging later labs. 
You'll create a new trace system call that will control 
tracing. It should take one argument, an integer "mask", 
whose bits specify which system calls to trace. For 
example, to trace the fork system call, a program 
calls trace(1 << SYS_fork), where SYS_fork is a 
syscall number from kernel/syscall.h. You have to 
modify the xv6 kernel to print out a line when 
each system call is about to return, if the system 
call's number is set in the mask. <span class= "title">The line should 
contain **the process id**, **the name of the system call** 
and the **return value**;</span> you don't need to print the 
system call arguments. The trace system call should 
enable tracing for the process that calls it and any 
children that it subsequently forks, but should not 
affect other processes.
We provide a trace user-level program that runs 
another program with tracing enabled (see user/trace.c). 
When you're done, you should see output like this:

 trace 32 grep hello README
3: syscall read -> 1023
3: syscall read -> 966
3: syscall read -> 70
3: syscall read -> 0

 trace 2147483647 grep hello README
4: syscall trace -> 0
4: syscall exec -> 3
4: syscall open -> 3
4: syscall read -> 1023
4: syscall read -> 966
4: syscall read -> 70
4: syscall read -> 0
4: syscall close -> 0

 grep hello README

 trace 2 usertests forkforkfork
usertests starting
test forkforkfork: 407: syscall fork -> 408
408: syscall fork -> 409
409: syscall fork -> 410
410: syscall fork -> 411
409: syscall fork -> 412
410: syscall fork -> 413
409: syscall fork -> 414
411: syscall fork -> 415
...
   
In the first example above, trace invokes grep 
tracing just the read system call. 
The 32 is 1 << SYS_read. In the second example, 
trace runs grep while tracing all system calls; 
the 2147483647 has all 31 low bits set. In the third 
example, the program isn't traced, so no trace output 
is printed. In the fourth example, the fork system calls 
of all the descendants of the forkforkfork test in 
usertests are being traced. Your solution is correct 
if your program behaves as shown above (though the 
process IDs may be different).

Some hints:

Add U/_trace to UPROGS in Makefile


步骤记录
  用户层
  - 在user/user.h中为一个函数添加一个系统调用。
  - 在user/usys.pl中添加一个桩
  - 在kernel/syscall.h添加一个系统调用的数子
  user/usys.pl生成user/usys.S
  使用 ecall 进入内核

the system call to user/user.h
Run make qemu and you will see that the compiler cannot 
compile user/trace.c, because the user-space stubs for 
the system call don't exist yet: add a prototype for 
the system call to user/user.h, a stub to user/usys.pl, 
and a syscall number to kernel/syscall.h. The Makefile 
invokes the perl script user/usys.pl, which produces 
user/usys.S, the actual system call stubs, which use 
the RISC-V ecall instruction to transition to the kernel. 
Once you fix the compilation issues, run trace 32 grep hello 
README; it will fail because you haven't implemented 
the system call in the kernel yet.

内核层
  - 在kernel/sysproc.c中添加一个sys_trace()函数
  - implements the new system call by remembering 
    its argument in a new variable in the proc structure 
    (see kernel/proc.h).
  - 内核空间获取用户空间的参数的函数在kernel/syscall.c中
    使用例子在kernel/sysproc.c中
  学习到的是 
    - 寄存器a0-a5保存参数
    - 寄存器a7保存调用函数的索引
  - Modify fork() (see kernel/proc.c) to copy the 
    trace mask from the parent to the child process.
    - fork()将父进程中获取到的参数也传递到子进程中
  - Modify the syscall() function in kernel/syscall.c 
    to print the trace output. You will need to add 
    an array of syscall names to index into.
  
Add a sys_trace() function in kernel/sysproc.c 
that implements the new system call by remembering 
its argument in a new variable in the proc structure 
(see kernel/proc.h). The functions to retrieve system 
call arguments from user space are in kernel/syscall.c, 
and you can see examples of their use in kernel/sysproc.c.


Modify fork() (see kernel/proc.c) to copy the 
trace mask from the parent to the child process.

Modify the syscall() function in kernel/syscall.c 
to print the trace output. You will need to add 
an array of syscall names to index into.


### sysinfo
Sysinfo (moderate)
In this assignment you will add a system call, 
sysinfo, that collects information about the 
running system. The system call takes one 
argument: a pointer to a struct sysinfo 
(see kernel/sysinfo.h). The kernel should fill 
out the fields of this struct: <span class="title">the freemem 
field should be set to **the number of bytes 
of free memory**, and the nproc field should 
be set to **the number of processes whose state 
is not UNUSED**.</span> We provide a test program 
sysinfotest; you pass this assignment if 
it prints "sysinfotest: OK".
Some hints:

Add $U/_sysinfotest to UPROGS in Makefile

Run make qemu; user/sysinfotest.c will fail 
to compile. Add the system call sysinfo, 
following the same steps as in the previous 
assignment. To declare the prototype for 
sysinfo() in user/user.h you need predeclare 
the existence of struct sysinfo:

    struct sysinfo;
    int sysinfo(struct sysinfo *);
  
Once you fix the compilation issues, run sysinfotest; 
it will fail because you haven't implemented 
the system call in the kernel yet.
sysinfo needs to copy a struct sysinfo back 
to user space; see sys_fstat() (kernel/sysfile.c) 
and filestat() (kernel/file.c) for examples of 
how to do that using copyout().

To collect the amount of free memory, 
add a function to kernel/kalloc.c

To collect the number of processes, add a 
function to kernel/proc.c

