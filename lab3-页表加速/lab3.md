## 实验3
***理清思路，有序渐进地完成任务***

### speed up page table
一些操作系统通过在用户和内核空间共享一个只读区域来加速具体的系统调用
这个利用了在执行这些系统调用的时候需要跨越内核。为了帮助你学习怎样插入
一个映射到页表中，你的第一个任务为getpid()系统调用实现这个优化。

当每个进程在创建时，在**USYSCALL(定义在memlayout.h中的
一个虚拟地址)映射一个制度的页面**（一个页面4096字节)。
 在这个页面的开始，存储一个struct usyscall(也定义在memlayout.h中),初始化它来存储当前进程的PID
  对于这实验，ugetpid()在用户空间被使用，会自动使用USYSCALL映射。
  当运行pgtbltest时如果ugetpid test case 通过，这个实验有全部的分数。

一些提示:
你可以在 kernel/proc.c中的 proc_pagetable()执行映射。
选择合适的权限位允许用户空间只读那个页面。
你会发现mappages()是一个有用的工具。
不要忘记在allocproc()中初始化这个页面。
确保在freeproc()中释放这个页面


涉及的文件
  kernel/proc.c
  kernel/proc.h
  kernel/memlayout.h

利用页表直接将一些信息映射到用户空间中，使得用户可以直接访问，不用通过系统调用获取，加快速度


#### 你可以在 kernel/proc.c中的 proc_pagetable()执行映射。
将一整个物理地址的页面映射到虚拟地址中(映射整个页面)
kernel/proc.c:234
mappages(pagetable, USYSCALL,PGSIZE,(uint64)(p->uc), PTE_R | PTE_U) < 0)

#### 选择合适的权限位允许用户空间只读那个页面。
##### U标记位
控制这用户的地址空间能否对这个虚拟地址进行解释
刚开始我没加上，死活解释都是0
后来加上就正常了

#### 不要忘记在allocproc()中初始化这个页面。
kernel/proc.c:149
先拿到一个页面，然后将本进程id存到这个页面中 kernel/proc.c:149
kalloc() 用来拿到一个空闲页面
    用来在freelist中找到一个空闲页面返回，并将头指针指向下一个
    
#### 确保在freeproc()中释放这个页面
##### proc_freepagetable也要释放
kernel/proc.c:256
需要在释放页表是把对USYSCALL的映射也取消，不然会报错
#####  freeproc();
kernel/proc.c:194
kfree(fp) 归还fp所在的页面


#### Print a page table (easy)
为了帮助您可视化RISC-V页表，并为将来的调试提供帮助，您的第二项任务是编写一个函数，该函数打印页表的内容。

定义一个名为vmprint()的函数。它应该接受一个pagetable_t参数，并按照下面的格式打印该pagetable。在exec.c中的返回argc之前插入if(p->pid==1) vmprint(p->pagetable)，以打印第一个进程的页表。如果您通过了make grade的pte打印测试，您将获得此部分实验的完整分数。
现在，当您启动xv6时，它应该打印类似于以下输出的内容，描述第一个进程在执行init时的页表：

```c
page table 0x0000000087f6b000
 ..0: pte 0x0000000021fd9c01 pa 0x0000000087f67000
 .. ..0: pte 0x0000000021fd9801 pa 0x0000000087f66000
 .. .. ..0: pte 0x0000000021fda01b pa 0x0000000087f68000
 .. .. ..1: pte 0x0000000021fd9417 pa 0x0000000087f65000
 .. .. ..2: pte 0x0000000021fd9007 pa 0x0000000087f64000
 .. .. ..3: pte 0x0000000021fd8c17 pa 0x0000000087f63000
 ..255: pte 0x0000000021fda801 pa 0x0000000087f6a000
 .. ..511: pte 0x0000000021fda401 pa 0x0000000087f69000
 .. .. ..509: pte 0x0000000021fdcc13 pa 0x0000000087f73000
 .. .. ..510: pte 0x0000000021fdd007 pa 0x0000000087f74000
 .. .. ..511: pte 0x0000000020001c0b pa 0x0000000080007000
init: starting sh
```
  
第一行显示vmprint的参数。在此之后，每个PTE都有一行，
包括指向树中更深层页表页的PTE。每个PTE行的缩进由".."
的数量表示其在树中的深度。每个PTE行显示页表页中PTE索引、
pte位和从PTE中提取的物理地址。不要打印无效的PTE。在上面的示例中，
顶级页表页具有条目0和255的映射。下一级的条目0仅映射索引0，而该索引0的底层具有条目0、1和2的映射。
您的代码可能会发出与上面不同的物理地址。条目数和虚拟地址应该相同。

一些提示：

您可以将vmprint()放在kernel/vm.c中。
使用kernel/riscv.h文件末尾的宏。
freewalk函数可能对你会有所启发的。
在kernel/defs.h中定义vmprint的原型，以便您可以从exec.c调用它。
在您的printf调用中使用%p打印出如示例所示的完整64位十六进制PTE和地址。

涉及文件
  kernel/exec.c
  kernel/defs.h
  kernel/vm.c


### 检测哪些页面已被访问（难）
一些垃圾收集器（自动内存管理的一种形式）可以从有关哪些页面已被访问
（读或写）的信息中受益。在本实验的这一部分中，您将在xv6中添加一个
新功能，通过检查RISC-V页表中的访问位来检测并向用户空间报告此信息。 
RISC-V硬件页面走廊每当解决TLB miss时在PTE中标记这些位。

您的任务是实现pgaccess（），这是一个系统调用，它报告哪些页面已被
访问。该系统调用需要三个参数。首先，它需要第一个用户页面的起始虚拟
地址。其次，它需要要检查的页面数。最后，它需要用户地址缓冲区，将结
果存储到位掩码中（使用每页一位的数据结构，其中第一页对应于最低有效
位）。如果运行pgtbltest时通过pgaccess测试用例，则可以获得本实验
的完整分数。

提示:

阅读 user/pgtlbtest.c 中的 pgaccess_test() 来了解 pgaccess 的使用方法。
在 kernel/sysproc.c 中实现 sys_pgaccess()。
使用 argaddr() 和 argint() 解析参数。
对于输出位掩码，在内核中存储一个临时缓冲区并在填充正确的位之后将其复制到用户 (通过 copyout()) 是更简单的。
可以设置扫描页面数的上限。
在 kernel/vm.c 中的 walk() 非常有用于找到正确的 PTEs。
在 kernel/riscv.h 中定义 PTE_A，即访问位。请参考 RISC-V 特权架构手册以确定其值。
在检查设置后请务必清除 PTE_A。否则，无法确定页面是否在上次调用 pgaccess() 之后被访问 (即，位将永远设置)。
vmprint() 可能会在调试页表时非常有用。


涉及文件
  kernel/sysproc.c
  kernel/riscv.h
  user/pgtbltest.c

