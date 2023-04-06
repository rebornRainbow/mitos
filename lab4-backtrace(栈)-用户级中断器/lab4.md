## 实验4

<style>
  .hei{
        background: linear-gradient(to right, red, blue);
        -webkit-background-clip: text;
        color: transparent;
    }
  .box{
    border:4px solid blue;
    
  }
</style>
## 实验1
RISC-V assembly (easy)
It will be important to understand a bit of 
RISC-V assembly, which you were exposed to 
in 6.1910 (6.004). There is a file user/call.c 
in your xv6 repo. make fs.img compiles it and 
also produces a readable assembly version of 
the program in user/call.asm.

Read the code in call.asm for the functions g, 
f, and main. The instruction manual for RISC-V 
is on the reference page. Here are some questions 
that you should answer (store the answers in a 
file answers-traps.txt):

Which registers contain arguments to functions? 
For example, which register holds 13 
in main's call to printf?
Where is the call to function f in the assembly 
code for main? Where is the call to g? 
(Hint: the compiler may inline functions.)
At what address is the function printf located?
What value is in the register ra just after the 
jalr to printf in main?
Run the following code.

	unsigned int i = 0x00646c72;
	printf("H%x Wo%s", 57616, &i);
      
What is the output? Here's an ASCII table that 
maps bytes to characters.
The output depends on that fact that the RISC-V is 
little-endian. If the RISC-V were instead big-endian 
what would you set i to in order to yield the same 
output? Would you need to change 57616 to a 
different value?

Here's a description of little- and big-endian 
and a more whimsical description.

In the following code, what is going to be 
printed after 'y='? (note: the answer is 
not a specific value.) Why does this happen?

	printf("x=%d y=%d", 3);
      

### 这个主要是汇编代码
包括 sd,ld,li,jalr,auipc,
现在不知道61c有没有包括，或者说查看文档

auipc	a0,0x0: 将a0 = pc+0<<12
  
jalr t1(t2): 将ra设置为下条指令，然后跳转到t1+t2的地方



### 问答题目


### 问题2 Backtrace (moderate) 
为了调试，通常有一个backtrace回溯是有用的：在错误出现的
点以上的栈上的函数调用的列表为了帮助backtrace回溯。编译器
产生了维持在栈上对应于当前系统调用链上的每个函数的栈帧的机器码。
每个栈帧有**返回地址**和**调用此函数的调用者的栈帧的栈指针**组成。
寄存器s0含有一个指向当前的栈帧的指针(它实际上指向的是保存的在
栈上的返回地址加上8的地址)。

你的回溯backtrace应该使用栈指针来遍历栈并且打印在每个栈帧上的保存返回地址

在kernel/printf.c.实现一个backtrace()函数。在sys_sleep
中插入对这个函数的一个调用，并且然后运行bbtest,这个实例调用了
sys_sleep。你的输出应该是这种形式的返回地址的链表
(但是数字可能是不同的):

    backtrace:
    0x0000000080002cda
    0x0000000080002bb6
    0x0000000080002898

Some hints:

Add the prototype for your backtrace() to kernel/defs.h so that you can invoke backtrace in sys_sleep.
The GCC compiler stores the frame pointer of the currently executing function in the register s0. Add the following function to kernel/riscv.h:
static inline uint64
r_fp()
{
  uint64 x;
  asm volatile("mv %0, s0" : "=r" (x) );
  return x;
}
and call this function in backtrace to read the current frame pointer. r_fp() uses in-line assembly to read s0.
These lecture notes have a picture of the layout of stack frames. Note that the return address lives at a fixed offset (-8) from the frame pointer of a stackframe, and that the saved frame pointer lives at fixed offset (-16) from the frame pointer.
Your backtrace() will need a way to recognize that it has seen the last stack frame, and should stop. A useful fact is that the memory allocated for each kernel stack consists of a single page-aligned page, so that all the stack frames for a given stack are on the same page. You can use PGROUNDDOWN(fp) (see kernel/riscv.h) to identify the page that a frame pointer refers to.


#### 栈的结构

所有的栈内容都在一个页面上面

栈帧的结构
``` c
字节号 | 内容(*addr)   |  
-----------------------------                
0     |下一个栈帧的地址 |               
1     |              |                        
2     |              |                        
3     |              |                    
4     |              |                  
5     |              |                  
6     |              |                  
7     |              |                  
8     |这里是返回地址  |                 
9     |              |                                        
10    |              |                              
11    |              |                              
12    |              |                              
13    |              |                              
14    |              |                              
16    |起始           |<--- fp
```

#### 完成


### 问题三 Alarm (hard)
在这个练习中，你将会为xv6添加一个特性，当进程使用CPU时间时，阶段性地通知它。
对于计算范围的进程这是很有用的，想要限制他们使用CPU的时间，或者对于那些想要计算但是也想要采取一些阶段性行为的进程。
 更加一般地说，你将会实现一个原始形式的用户级中断和错误处理器。例如你可以会在应用中使用相似的东西来处理页表错误
 如果你的答案通过了alarmtest和usertests -q，你的答案就是正确的。

你应该添加一个新的sigalarm（阶段处理器） 系统调用，如果一个应用调用了sigalarm(n,fn),
那么在应用消耗了n'ticks'的CPU时间后，内核会调用应用函数fn。
当fn返回后，应用应该恢复到它离开的地方。
一个tick在xv6是一个相当随机的时间单位，有一个硬件多么频繁地产生一个中断决定。
如果一个应用调用了sigalarm(0,0)，那么内核应该停止产生阶段性地调用。

你应该在你的xv6仓库中可以找到一个文件user/alarmtest.c。
将它添加到Makefile中，它将不会被正确地编译直到你添加了sigalarm和sigreturn系统调用。

alarmtest 在test0中调用了sigalarm(2,periodic)来让内核每两个ticks 调用一次periodic()。之后占用一会。
你可以在user/alarmtest.asm中看到alarmtest的汇编代码，这对于你之后的debug可能是有用的。
当alarmtest 产生像这样的输出并且usertests -q 也正确运行的时候答案是正确的
``` python
\$ alarmtest
test0 start
........alarm!
test0 passed
test1 start
...alarm!
..alarm!
...alarm!
..alarm!
...alarm!
..alarm!
...alarm!
..alarm!
...alarm!
..alarm!
test1 passed
test2 start
................alarm!
test2 passed
test3 start
test3 passed
\$ usertest -q
...
ALL TESTS PASSED
\$
```
当你完成，你的答案将只有几行代码，但是让它正确可能是非常巧妙的。
我们将用原仓库中的alarmtest.c测试你的代码
你可以修改alarmtest.c来帮助你debug，但是确保通过原版本的所有测试案例
test0: invoke handler
通过修改内核跳转到在用户空间的alarm处理器。这会导致test0打印alarm
现在先不要担心在“alarm”之后会发生什么.现在如果你的程序在打印了"alarm!"之后崩掉是可以的。这是一些提示

你将会需要修改Makefile来让alarmtest.c被编译换成xv6用户程序
**放到user/user.h中的正确的声明**是:
```c
    int sigalarm(int ticks, void (*handler)());
    int sigreturn(void);
```
**更新user/usys.pl(这产生了user/usys.S),kernel/syscall.h, 和 kernel/syscall.c**，来让alarmtest
调用sigalarm 和 sigreturn系统调用。
**函数在kernel/sysproc.c中**
**现在,你的sys_sigreturn应该只返回0。**
你的sys_sigalarm()，应该**存储alarm interval和指向处理器函数的指针**
到一个 ^**proc结构体中 kernel/proc.h的一些新的属性**
你将会需要**记录从上次的调用开始过去了多少的ticks**。(或者说距离下次调用还剩多少tick)
你也会需要为了这个往struct proc 中添加一个新的属性.**你可以在allocproc()中初始化这些属性**。
每次tick，硬件中都对**强制一个中断，这个将会在usertrap() 在kernel/trap.c中处**。
如果有一个时间中断，你只会想要操作一个进程的alarm ticks;你会想要类似这样的东西
```c
    if(which_dev == 2) ...
```
Only invoke the alarm function if the process has a timer outstanding. 
只有在进程有定时器未完成时，才调用报警函数。
注意用户的**alarm function的地址可能是0**

这里需要注意系统中得到的是用户态的函数的虚拟地址，转化为物理地址也无法执行函数？？？
<div class ="box">
<h6>不知道怎么解决<h6>
首先我们要清楚，trap进来之后，<br>
内核可以通过<span class="hei">设置进程trampframe的epc寄存器来<br>
设置返回时执行指令的位置</span>。（p->trampframe->epc）;<br>
所以这里我们可以简单地<span class="hei">通过更改<br>
p->trampframe->epc让用户进程序回到用户态之后执行用户的处理器函数，<br>
用户的处理器函数结束又会调用一个sigreturn的系统调用。<br>
然我们返回之前执行的状态</span>。
这里补充一个用户态跳转到内核态执行系统调用，然后返回的过程。
</div>
 (e.g., in user/alarmtest.asm, periodic is at address 0).
你将需要修改usertrap()使得当一个进程的 alarm interval expires过去时，
用户的进程需要执行那个处理函数。
但一个trap在RISC-V中返回用户空间是，什么决定了在用户空间代码恢复执行的指令？

这个很容易用gdb查看，如果你告诉qemu只使用一个可以运行的CPU，
    make CPUS=1 qemu-gdb
You've succeeded if alarmtest prints "alarm!"
如果alarmtest prints "alarm!".你已经给成功了
test1/test2()/test3(): 恢复中断的代码
很有可能，alarmtest在test0或test1中打印“alarm!”后崩溃，
或者alarmtest(最终)打印“test1 failed”，
或者alarmtest退出时没有打印“test1“ passed
要解决这个问题，必须确保在警报处理程序完成时，控制返回到用户程序最初被定时器中断时的指令。
必须确保寄存器内容恢复到中断时的值，以便用户程序在报警后可以继续不受干扰。
最后，应该在每次报警计数器发出警报后“重新武装”它，以便定期调用处理程序。

作为一个起点，我们已经为你做出了一个设计决策:
用户alarm处理器当他们完成的时候需要调用sigreturn系统调用
看一看在alarmtest.c中的periodic作为一个例子。
这意味着你可以往usertrap和sys_sigreturn里添加代码，两者协让用户的进程在处理完alarm后
正确恢复

一些提示:

你的答案<span class = "hei">需要你保存和恢复寄存器---那些寄存器需要你在保存和恢复来正确地恢复中断的代码</span>（这可能有很多！）
在定时器结束时，让usertrap在struct proc中保存足够的状态，使sigreturn能够正确地返回到被中断的用户代码。
防止对处理器的二次调用---如果处理器现在还没有返回，内核不应该再次调用它。

Make sure to restore a0. sigreturn is a system call, and its return value is stored in a0.
确保恢复a0。Sigreturn是一个系统调用，它的返回值存储在a0中
一旦通过了test0、test1、test2和test3，就运行usertests -q，以确保没有破坏内核的任何其他部分。


#### 实验总结
趁现在我对实验还有记忆
对整个实验过程进行一个回顾，同时对risc-v的系统调用过程进行一个演练。
首先是在user/user.h中添加系统调用的声明
在user/usys.pl中添加对新系统调用的进入项
之后因为测试的桩进程在user/alarmtest.c中所以需要在makefile中加上这个桩函数
实验中有两个系统调用
sigalarm(ticks,&hander);每次经过ticks个时间片段时调用一次这个函数 
  这个函数是在用户空间当中的，所以执行完这个函数之后必须调用sigreturn 返回原来代码终端的地方
  如果发生了hander()的话需要在返回用户的hander之前进行现场保护，因为执行用户的代码段
  必然会改变用户寄存器的状态
sigreturn();用来返回原先执行hander的代码段
  恢复现场，恢复用户代码段的执行，
  同时注意需要返回的事trapframe->a0;
  因为系统调用的函数返回值是放在a0寄存器当中

需要按照说明的添加上两个系统调用的声明在
**更新user/usys.pl(这产生了user/usys.S),kernel/syscall.h, 和 kernel/syscall.c**
因为我们需要保存很多东西
要在kernel/proc.h中添加上这些变量的声明
```c
  //这是为了问题三添加的属性
  int ticks;//每隔多少个ticks执行一次hander;
  uint64 hander_addr;//处理器的指针
  int ticks_p;//已经经过了多少个ticks
  //寄存器的信息
  uint64 s_ra;
  uint64 s_sp;
  uint64 s_gp;
  uint64 s_tp;
  uint64 s_t0;
  uint64 s_t1;
  uint64 s_t2;
  uint64 s_s0;
  uint64 s_s1;
  uint64 s_a0;
  uint64 s_a1;
  uint64 s_a2;
  uint64 s_a3;
  uint64 s_a4;
  uint64 s_a5;
  uint64 s_a6;
  uint64 s_a7;
  uint64 s_s2;
  uint64 s_s3;
  uint64 s_s4;
  uint64 s_s5;
  uint64 s_s6;
  uint64 s_s7;
  uint64 s_s8;
  uint64 s_s9;
  uint64 s_s10;
  uint64 s_s11;
  uint64 s_t3;
  uint64 s_t4;
  uint64 s_t5;
  uint64 s_t6;
  uint64 s_epc;           // saved user program counter

  int isuse; //检查是否在调用中
```
sys_sigalrm()
这个函数需要获取用户发送过来的参数
一个ticks 和一个 hander_addr
我们还需要一个变量来统计过去的ticks数量ticks_p
初始化为0
将其保存
``` c
//处理alarm的调用
uint64 sys_sigalarm(void)
{ 
  struct proc*proc = myproc();
  int ticks = 0;
  argint(0,&ticks);
  uint64 hander_addr = 0;
  argaddr(1,&hander_addr);//取得的是用户的虚拟地址
  proc->isuse = 0;//没有在使用
  proc->ticks = ticks;
  proc->hander_addr = hander_addr;
  //保存当前寄存器的信息
  proc->ticks_p = 0;
  // proc->s_epc = proc->trapframe->epc;


  // printf("%x",proc->trapframe->a0);

  return 0;
}
```

然后是在kernel/trap.c中当
计算机产生时间中断时需要对其进行判定
然后进行处理
这里有一个微妙的地方就是
如果这个函数正在处理中不应该再次进入
所以需要一个isused进行判断
  进入执行之后需要将其设置为1防止二次进入
之后让ticks_p增加
然后需要对此时用户的寄存器信息进行保存
之后用epc寄存器控制返回用户态时执行的语句
```c
 // ok 2表示时间中断 1表示其他设备
    if(which_dev == 2&&p->isuse == 0)
    {
      ++p->ticks_p ;
      { 
        if(p->ticks && (p->ticks_p == p->ticks))
        {
          intr_off();
          p->isuse = 1;
          p->ticks_p = 0;
          p->s_epc = p->trapframe->epc;
          p->s_ra =  p->trapframe->ra ;
          p->s_sp =  p->trapframe->sp ;
          p->s_gp =  p->trapframe->gp ;
          p->s_tp =  p->trapframe->tp ;
          p->s_t0 =  p->trapframe->t0 ;
          p->s_t1 =  p->trapframe->t1 ;
          p->s_t2 =  p->trapframe->t2 ;
          p->s_s0 =  p->trapframe->s0 ;
          p->s_s1 =  p->trapframe->s1 ;
          p->s_a0 =  p->trapframe->a0 ;
          p->s_a1 =  p->trapframe->a1 ;
          p->s_a2 =  p->trapframe->a2 ;
          p->s_a3 =  p->trapframe->a3 ;
          p->s_a4 =  p->trapframe->a4 ;
          p->s_a5 =  p->trapframe->a5 ;
          p->s_a6 =  p->trapframe->a6 ;
          p->s_a7 =  p->trapframe->a7 ;
          p->s_s2 =  p->trapframe->s2 ;
          p->s_s3 =  p->trapframe->s3 ;
          p->s_s4 =  p->trapframe->s4 ;
          p->s_s5 =  p->trapframe->s5 ;
          p->s_s6 =  p->trapframe->s6 ;
          p->s_s7 =  p->trapframe->s7 ;
          p->s_s8 =  p->trapframe->s8 ;
          p->s_s9 =  p->trapframe->s9 ;
          p->s_s10 = p->trapframe->s10;
          p->s_s11 = p->trapframe->s11;
          p->s_t3 =  p->trapframe->t3 ;
          p->s_t4 =  p->trapframe->t4 ;
          p->s_t5 =  p->trapframe->t5 ;
          p->s_t6 =  p->trapframe->t6 ;
          p->trapframe->epc = p->hander_addr;
          intr_off();
        }
      }
```

在sigreturn中需要将用户的执行恢复
返回原先中断的位置
并且设置其不再使用
注意这里返回值应该是p->trapframe->a0
因为系统调用用 a0 传递返回值
标记其不再使用
```c
//处理alaem的返回
uint64 sys_sigreturn(void)
{
  struct proc*proc = myproc();
  //结束调用
  // intr_off();
  // proc->ticks = 0;
  // proc->hander_addr = 0;
  // proc->ticks_p = 0;
  //恢复寄存器信息
  proc->trapframe->ra = proc->s_ra;
  proc->trapframe->sp = proc->s_sp;
  proc->trapframe->gp = proc->s_gp;
  proc->trapframe->tp = proc->s_tp;
  proc->trapframe->t0 = proc->s_t0;
  proc->trapframe->t1 = proc->s_t1;
  proc->trapframe->t2 = proc->s_t2;
  proc->trapframe->s0 = proc->s_s0;
  proc->trapframe->s1 = proc->s_s1;
  proc->trapframe->a0 = proc->s_a0;
  proc->trapframe->a1 = proc->s_a1;
  proc->trapframe->a2 = proc->s_a2;
  proc->trapframe->a3 = proc->s_a3;
  proc->trapframe->a4 = proc->s_a4;
  proc->trapframe->a5 = proc->s_a5;
  proc->trapframe->a6 = proc->s_a6;
  proc->trapframe->a7 = proc->s_a7;
  proc->trapframe->s2 = proc->s_s2;
  proc->trapframe->s3 = proc->s_s3;
  proc->trapframe->s4 = proc->s_s4;
  proc->trapframe->s5 = proc->s_s5;
  proc->trapframe->s6 = proc->s_s6;
  proc->trapframe->s7 = proc->s_s7;
  proc->trapframe->s8 = proc->s_s8;
  proc->trapframe->s9 = proc->s_s9;
  proc->trapframe->s10 =proc->s_s10;
  proc->trapframe->s11 =proc->s_s11;
  proc->trapframe->t3 = proc->s_t3;
  proc->trapframe->t4 = proc->s_t4;
  proc->trapframe->t5 = proc->s_t5;
  proc->trapframe->t6 = proc->s_t6;
  //恢复epc
  proc->trapframe->epc = proc->s_epc;
  proc->isuse = 0;//没有在使用

  // intr_on(); 
  return proc->trapframe->a0;//最后这里需要返回a0的值，因为a0是原来的返回值
}
```

至于系统调用的过程看我的笔记中有详细的解释



