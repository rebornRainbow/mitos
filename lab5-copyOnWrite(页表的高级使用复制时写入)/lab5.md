<style>
span
{
        background: linear-gradient(to right, red, blue);
        -webkit-background-clip: text;
        color: transparent;
}

</style>
### 实现 copy-on-write fork(hard)
在xv6中fork 系统调用复制所有父亲进程的用户空间内存到孩子。
如果父亲非常大， 复制需要很多时间。
更糟糕的是:这通常是没有必要的，fork()通常有exec()接着，放弃了复制的内存，没有使用它。
换句话说，如如果孩子和父母有一个复制的页面，一个或者两者都对其进行写入，这时候复制才是
需要的
The solution
解决方案
在实现cowfork()中你的目标是推迟分配和复制物理页面直到复制方真的需要它的时候。
COW fork() 只为孩子创建了一个页表，用户空间的页表项指向父亲的物理页表
COW fork() 使得孩子和父亲中的所有的页表项都是只读状态。
当任何一个进程试图写入这些复制写入页时，CPU都会发出一个页面错误。
内核页表错误处理器检测到这种情况，为错误进程分配一个物理页面。将原先的页面复制到新的页面，
然后修改在错误进程中的相关的页表项目来定义新的页面，这时候，页表项目标记为可写。
当页面错误处理器返回时候，用户进程将可以写入到它的页面的副本。

COWfork()使得释放用户使用的内存变得有些微妙。
一个给定的物理页面可以是被多个进程的页表引用，只有当最后一个引用消失的时候才可以被释放。
 在像xv6这样简单的内核中这样的记录是相当简单直接的，但是在真实的内核中，这可能是非常困难做到的。
 例如打补丁直到COWs回来

实现 copy-on-write fork(hard)
的任务是在xv6内核中实现一个copy-on-write fork。如果你修改的内核在cowtest和'usertests -q'程序中都可以
成功执行，你就完成了


为了帮助你测试你的实现，我们已经提供了一个xv6程序叫做COWtest(源程序在user/cowtest.c中)。cowtest运行
各种各样的测试，但是在未修改的xv6中一个也通过不了。因此，刚开始，你可能会看到这样的输出:
```c
 cowtest
simple: fork() failed
```
 
这个简单的测试分配了超过可用内存的一半，之后fork()s,这个fork()失败是因为没有足够的空闲物理内存让孩子可以
复制父亲的所有内存

当你完成，你内核应该通过所有的测试，输出如下：
```c
 cowtest
simple: ok
simple: ok
three: zombie!
ok
three: zombie!
ok
three: zombie!
ok
file: ok
ALL COW TESTS PASSED
 usertests -q
...
ALL TESTS PASSED
```
这是开始的一个合理的计划。

1.修改uvmcopy()来映射父亲的物理内存到孩子中，而不是分配一个新的页面。**<span>清除所有孩子和父亲页面中有PTE_W的PTE_W位</span>**。
完成
```c
                    这一位控制了可写信息
                           *
                           *
                           |
100001111111011001100000010111
100001111111011001100000010011
```



修改<span>usertrap()来识别页面错误，当在一个COW页面中出现一个写入页面错误时，用kalloc()分配一个新的页面</span>
复制旧的页面到新的页面，在有PTE_W的页表项中安装这些页面，那些原本只读的应该仍然是只读，在父亲和孩子之间共享
那些试图写入这些页面的进程应该被杀死。

确保在最后一个页表索引消失的时候释放这个物理页面----但是之前不要这么做。一个好的方式是，为每一个物理页面记录一个索引次数。
- 在kalloc()的时候将这个页面的索引次数设置为1.

```c
在vm.c 170行 本文件中开头设置了pa_refer_num[32734]
前提是页面只会被同一个页表修改一次，如果之后修改flags用指针修改。
在mappages时将这个数量增加1。
```
- 当fork导致了一个孩子来共享这个页面的时候增加这个页面的索引次数。
```C
在uvmcopy中成功复制了一页后将这一页面增加1
vm.c: 379行
if(VALPA(pa))
      ++pa_refer_num[PA_GET_INDEX(pa)];
```

- 然后当任何的进程将这个页面从页表中去掉的时候将这个页面的索引次数减去1。
```c
这个过程需要明白什么时候将其去掉
我的理解是在uvmunmap的时候将其去掉。
vm.c 203行
if(VALPA(pa))
{
  --pa_refer_num[PA_GET_INDEX(pa)];
}

这个看做法应该在其中，但是出现了错误，不明白
```
- 只有当当前页面的索引次数为0的时候,kfree()才应该释放这个页面。
```c
我的理解是
kalloc.c 65行
if(VALPA(pa)  && pa_refer_num[PA_GET_INDEX(pa)] != 0)
{//有效且索引为0
  return;
}
```
- 可以将这些计数装在一个固定大小的整数数组中。
你需要弄清楚怎么索引这个数组，怎么选择这个数组的大小。
例如，你可以用这个页面的物理地址除以4096来索引数组，
将kalloc.c中kinit()放置在空闲链表上的任何页的最高物理地址赋值给数组。不用担心地修改kalloc.c（e.g., kalloc() and kfree()）来维持索引计数。
```c
这里我用的是一个固定的数组
在vm.c中定义
其他的模块通过extern int pa_refer_num[]来引用
宏函数
PA_GET_INDEX(pa)//通过地址找到对应的索引
还是有问题不知道怎么回事还是页面不够
太可气了
```

- <span>最后一部分</span>
修改copyout()，使其在遇到COW页时使用与缺页异常相同的方案。


Some hints:

为每个页表项，记录它是否是一个 COW mapping映射。你可以使用在RISC-V PTE中的RSW (reserved for software)位
来达到这个目的。
为页表定义的许多有用的宏在 kernel/riscv.h.的末尾
如果产生页面错误，没有空闲内存，进程应该直接被杀死。




#### 目前在实现了为孩子实现cow后，进程结束时
出现问题
怀疑是因为页面free的问题
在第一个释放这个页面之后，后面的再次访问就会失败
应该不是cow的问题，因为孩子是可以成功打印的，这一点并没有问题。

所以尝试加上索引并使用索引

##### 目前发现
当前我们加上索引的也包括了所有的内核空间的地址
应该是只给用户空间用的地址才可以使用
所以包弄明白哪里开始时用户空间
```c
0x80041cf0 end;但是这个凑不成一页
0x80042000 PGROUNDUP((uint64)end)
0x88000000 PHYSTOP;
```
果然是这个问题
现在可以成功启动

#### 现在测试卡在第三个
意料之中我还有一个没有实现

发现似乎进程序启动后就没有释放过页面，不知道是怎么回事。

尝试当外面的调用强制要求释放的时候将其页面的引用修改为0

似乎不是这个错误

我也晕了
猜测是 其他执行的问题


- 奇怪的是明明有页面的索引为0。
推测
我需要增加一条指令当地址在有效范围内，且索引项目已经为0的时候
直接释放该页面。
睡了明天测试


#### 实验临时总结
以专业的术语来说算是发生了内存泄露的情况，原因不明
可以观察到的是本来应该挂载在freelist上的0-31号块，突然不见了非常的离谱
检测
- 在整个过程中是否有使用过31号块。
- 发现原因是真的把所有的块都用完了
  - 这说明并不是我代码的原因，有人没有释放内存，或者说我没有正确地找到该释放内存的位置。
  - 我似乎找到原因了--并不是在uvmunmap的时候将索引减去1.这样的做法是愚蠢的

- 实验测试通过了，但是usertest -q 没有完成
说明存在泄露的情况


- 各种测试集合太有用了

```c
这是对虚拟地址0的索引
100001111111010000100101011011
```


#### 按照位的运算的操作
检查某两位是否都为1
不能用 &（|）的形式
```c
pa &(PTE_COW|PTE_OW)//表示PTE_COW或者PTE_OW为1
//上面的运算不是检查是否 PTE_COW和PTE_OW位都为1
pa &(PTE_COW)&&pa &(PTE_OW)//才是PTE_COW和PTE_OW位都为1
```

#### 实验小结

1. 修改来映射父亲的物理内存到孩子中，而不是分配一个新的页面。**<span>清除所有孩子和父亲页面中有PTE_W的PTE_W位</span>**
 ```c
    // Given a parent process's page table, copy
    // its memory into a child's page table.
    // Copies both the page table and the
    // physical memory.
    // returns 0 on success, -1 on failure.
    // frees any allocated pages on failure.
    int
    uvmcopy(pagetable_t old, pagetable_t new, uint64 sz)
    {
      pte_t *pte;
      uint64 pa, i;
      uint flags;

      //遍历父亲进程的页面
      for(i = 0; i < sz; i += PGSIZE)
      {
        //检查页面是否存在
        if((pte = walk(old, i, 0)) == 0)
          panic("uvmcopy: pte should exist");
        //检查页面是否有效
        if((*pte & PTE_V) == 0)
          panic("uvmcopy: page not present");

        //将页面标记为cow页面
        *pte = *pte | PTE_COW;

        //检查pte是可写入
        if(*pte & PTE_W)
        { 
          //标记为不可写入
          *pte = *pte ^ PTE_W;
          //标记为原本可以写入页面
          *pte = *pte | PTE_OW;
        }
        //将页表项目转化为物理地址
        pa = PTE2PA(*pte);
        flags = PTE_FLAGS(*pte);

        //更改将孩子和父亲指向同一个屋里地址
          //如果有写入将其标记为没有  
        if(mappages(new, i, PGSIZE, (uint64)pa, flags) != 0)
        {
          kfree((void *)pa);
          goto err;
        }
        //有一个孩子来这里，所以将其增加1
        //将这个物理地址的索引加1
        if(VALPA(pa))
          ++pa_refer_num[PA_GET_INDEX(pa)];
          
      }
      return 0;

    err:
      uvmunmap(new, 0, i / PGSIZE, 1);
      return -1;
    }
```

```c
                    这一位标记了可写信息
                           *
                           *
                           |
100001111111011001100000010111
100001111111011001100000010011
```

  - 因为PTE中后十位是信息位，且其中第十位和第九位时保留位，所以我用了这两个位置来分别标记这是cow页面和这是ow（原先就是写入的来标记可写信息）页面
  定义在rish.c中346行
```c
#define PTE_COW (1L << 8)// 标记这是否是cow页面
#define PTE_OW (1L << 9)//标记这原本是否是可写入页面
```
  - 上面这段函数是fork以后子进程利用父进程的空间实现自己空间的地方，我们先将原先的父进程的PTE改变为PTE_COW
  - 然后检查自己这个页面是否可写，
    - 如果可写需要将标记设置为不可写入，导致页面错误
    - 然后将其标记为原本可写PTE_OW，方便区分
  - 得到其对应的物理页的地址，然后复制标记
  - 将子进程页表对应的页表项也指向相同的物理地址，有相同的标记。

2. 修改<span>usertrap()来识别页面错误，当在一个COW页面中出现一个写入页面错误时，用kalloc()分配一个新的页面</span>
复制旧的页面到新的页面，在有PTE_W的页表项中安装这些页面，那些原本只读的应该仍然是只读，在父亲和孩子之间共享
那些试图写入这些页面的进程应该被杀死。
```c
else if( r_scause() == 15)
  {//标记这是写入错误
    uint64 va = r_stval();//获取导致错误的虚拟地址
    pte_t *pte;//这个虚拟地址所对应的页表项目
    char *mem;
    uint flags;
    if(va >= MAXVA)
    {//这里意思是让我们杀死他
      exit(-1);//杀死
    }
    if((pte = walk(p->pagetable, va, 0)) == 0)
    {
      panic("uvmunmap: walk");
      setkilled(p);
    }
    //只有当是cow页面，且原本是可以写入时才可以执行
    if( (*pte &(PTE_COW))&&(*pte &(PTE_OW)))
    {
      flags = PTE_FLAGS(*pte);
      flags = flags | PTE_W;
      // flags = flags ^ (PTE_COW|PTE_OW);
      uint64 pa = walkaddr(p->pagetable,va);
      //先将原先的这个虚拟地址指向的物理地址解除映射
      uvmunmap(p->pagetable,PGROUNDDOWN(va),1,0);
      // 分配一个新的页面。将原先的内容进行拷贝
      //这里是分配新的内存页
      if((mem = kalloc()) == 0)
      {
        panic("usertrap:kalloc: no pages");
        setkilled(p);
      }
      //复制新的页面
      memmove(mem, (char*)pa, PGSIZE);
      //映射新的页面
      if(mappages(p->pagetable, PGROUNDDOWN(va), PGSIZE, (uint64)mem, flags) != 0){
        panic("usertrap:mappages: fail");
        kfree(mem);
      }
      //将原先页面的索引减少1
      kfree((void *)pa);
    }else
    {
      // 打印错误
      // printf("写入错误usertrap(): unexpected scause %p pid=%d\n", r_scause(), p->pid);
      // printf("            sepc=%p stval=%p\n", r_sepc(), r_stval());
      setkilled(p);
    }
  }else {
    // printf("usertrap(): unexpected scause %p pid=%d\n", r_scause(), p->pid);
    // printf("            sepc=%p stval=%p\n", r_sepc(), r_stval());
    setkilled(p);
  }

```
  -  这个函数是子进程发生中断的地方
  -  p记录了发生中断的进程！
  - scause寄存器记录了导致中断的原因，15=0xf是写入错误
  - r_stval()存放了导致错误的虚拟地址
  - 我们需要对虚拟地址进行检查，如果这个地址超出了MAXVA需要杀死如果没有也要杀死
  - 获取pte 并检查
```c
pa &(PTE_COW|PTE_OW)//表示PTE_COW或者PTE_OW为1
//上面的运算不是检查是否 PTE_COW和PTE_OW位都为1
pa &(PTE_COW)&&pa &(PTE_OW)//才是PTE_COW和PTE_OW位都为1
```
  - 如果是原先可写的PTE_OW 就讲标记更新为可写
  - 得到这时子进程指向的物理地址(这个地址此时同时为父进程和子进程指向)
  - 先将子进程的虚拟地址和这个物理地址解除映射。
  - 从内核获取一空闲的页面，设置为可写，映射到本进程的虚拟地址上
  - 处理各种错误（一旦发生将本进程杀死，然后退出）
    - MAXVA
    - 写入不可写入的页面

3. 确保在最后一个页表索引消失的时候释放这个物理页面----但是之前不要这么做。一个好的方式是，为每一个物理页面记录一个索引次数。
- 在kalloc()的时候将这个页面的索引次数设置为1.

``` c++
我在vm.c中设置了计数器和验证的宏函数
#define VALPA(pa)  ((uint64)pa >=  PGROUNDUP((uint64)end) && (uint64)pa <= PHYSTOP)//这是用来验证地址的宏函数
int pa_refer_num[32734] = {0};//物理页面的索引次数

这是在kalloc.c中的
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r)
    kmem.freelist = r->next;
  release(&kmem.lock);

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  if(VALPA(r))//检查
    pa_refer_num[PA_GET_INDEX(r)] = 1;//kalloc的时候将它设置为1.
  return (void*)r;
}

```


- 当fork导致了一个孩子来共享这个页面的时候增加这个页面的索引次数。

```c
在uvmcopy中成功复制了一页后将这一页面增加1
// Given a parent process's page table, copy
// its memory into a child's page table.
// Copies both the page table and the
// physical memory.
// returns 0 on success, -1 on failure.
// frees any allocated pages on failure.
int
uvmcopy(pagetable_t old, pagetable_t new, uint64 sz)
{
  pte_t *pte;
  uint64 pa, i;
  uint flags;
  // char *mem;

  //遍历父亲进程的页面
  for(i = 0; i < sz; i += PGSIZE)
  {
    //检查页面是否存在
    if((pte = walk(old, i, 0)) == 0)
      panic("uvmcopy: pte should exist");
    //检查页面是否有效
    if((*pte & PTE_V) == 0)
      panic("uvmcopy: page not present");

    //将页面标记为cow页面
    *pte = *pte | PTE_COW;

    //检查pte是可写入
    if(*pte & PTE_W)
    { 
      //标记为不可写入
      *pte = *pte ^ PTE_W;
      //标记为原本可以写入页面
      *pte = *pte | PTE_OW;
    }
    //将页表项目转化为物理地址
    pa = PTE2PA(*pte);
    flags = PTE_FLAGS(*pte);
    // //这里是分配新的内存页
    // if((mem = kalloc()) == 0)
    //   goto err;
    // //复制新的页面
    // memmove(mem, (char*)pa, PGSIZE);
    // if(mappages(new, i, PGSIZE, (uint64)mem, flags) != 0){
    //   kfree(mem);
    //   goto err;
    //  }

    //更改将孩子和父亲指向同一个屋里地址
      //如果有写入将其标记为没有  
    if(mappages(new, i, PGSIZE, (uint64)pa, flags) != 0)
    {
      kfree((void *)pa);
      goto err;
    }
    //有一个孩子来这里，所以将其增加1
    //将这个物理地址的索引加1
    if(VALPA(pa))
      ++pa_refer_num[PA_GET_INDEX(pa)];
      
  }
  return 0;

 err:
  uvmunmap(new, 0, i / PGSIZE, 1);
  return -1;
}
```

- 然后当任何的进程将这个页面从页表中去掉的时候将这个页面的索引次数减去1。
  - <span>这里我需要着重解释</span>
  - 首先页面的页面索引的加减是关于页面分配的方法，最好是全部放到同意个模块（.c/.cpp文件，）中，使用的越散，就越是难以维护
    - 我刚开始就犯了这个错误
    - 犯错了也不要紧，懂得利用git这个工具，在涉及文件不多的时候，vscode中的git插件相当的好用
  - 什么时候应该将索引减去1，当原先没有索引的时候，解除就是在释放的时候。所以在kfree()里面
```c
// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;
  
  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");
  //**********************这里********************
  if(pa_refer_num[PA_GET_INDEX(pa)] > 0)
  {//有效且索引为0
    --pa_refer_num[PA_GET_INDEX(pa)]; 
  }

  if(pa_refer_num[PA_GET_INDEX(pa)] > 0)
  {
    return;
  }
  //********************************************


  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  acquire(&kmem.lock);
  r->next = kmem.freelist;
  kmem.freelist = r;
  release(&kmem.lock);
}

```
  - 但是也不是什么时候都可以减去1的，
  - 刚开始freerange()的时候各个索引次数还是0这时候不应该减去1
  - 之后还应该检查，只有当为0的时候才应该释放

4. 那么在那些时候应该kfree()呢
- 在我写的这几个函数中
  - 在copyout中
    - 出现了映射错误
    - 子进程映射到新的页面后，原先的那个页面索引次数应该减去1
  - 在usertrap中和copyout一样
    - 出现了映射错误
    - 子进程映射到新的页面后，原先的那个页面索引次数应该减去1
  - 在uvmunmap中
    - 如果调用者要求释放内存的话直接释放。
  - uvmcopy中
    - 如果映射失败直接释放
5. 最后在copyout中
  - 这个和usertrap一样
    - 不做详细解释
  
6. 最后这个实验其实没没有完成会随机出错，我推测和同步数据有关系
  - 解决方案加锁
  - 我推测也只有一种可能就是对pa_r_num的同步出错
  - 所以我直接给kalloc和kfree加锁同一时间只允许一个cpu对其进行访问
    - 声明一个freelock;
```c
struct spinlock freelock;
```
    - 在kinit()中初始化，这是内核配置好后第一个调用的函数
```c
void
kinit()
{
  initlock(&kmem.lock, "kmem");
  initlock(&freelock,"freelock");//铸造锁
  freerange(end, (void*)PHYSTOP);
}

```
    - 然后再kfree()中对其整个过程加锁
      - 不要忘了在任何退出的分支中退出前都要释放锁
```c
// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  // intr_off();
  struct run *r;
  
  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");
  acquire(&freelock);//加锁
  if(pa_refer_num[PA_GET_INDEX(pa)] > 0)
  {//有效且索引为0
    --pa_refer_num[PA_GET_INDEX(pa)]; 
  }

  if(pa_refer_num[PA_GET_INDEX(pa)] > 0)
  {
    release(&freelock);//放锁
    return;
  }

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  acquire(&kmem.lock);
  r->next = kmem.freelist;
  kmem.freelist = r;
  release(&kmem.lock);
  release(&freelock);//放锁
  // intr_on();
}
```
  - 最后在kalloc()中也要加锁，否则也会导致错误
    - 不要忘了在任何退出的分支中退出前都要释放锁
```c
// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;
  acquire(&freelock);//加锁
  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r)
    kmem.freelist = r->next;
  release(&kmem.lock);

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  if(VALPA(r))
    pa_refer_num[PA_GET_INDEX(r)] = 1;//kalloc的时候将它设置为1.
  release(&freelock);//放锁
  return (void*)r;
}

```
#### <span>完美结束</span>
