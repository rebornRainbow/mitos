Lab: Multithreading
这个实验将让你熟悉多线程。你将在用户级线程包中实现线程之间的切换，使用多个线程加速程序，并实现一个屏障。


在编写代码之前，您应该确保已经阅读了xv6书中的“第7章：调度”并学习了相应的代码。


To start the lab, switch to the thread branch:

  \$ git fetch
  \$ git checkout thread
  \$ make clean
  
Uthread: switching between threads (moderate)
在这个练习中，您将为用户级线程系统设计上下文切换机制，然后实现它。为了让您开始，您的xv6有两个文件user/uthread.c和user/uthread\_switch.S，以及Makefile中的一条规则来构建uthread程序。uthread.c包含大部分用户级线程包和三个简单测试线程的代码。线程包缺少一些创建线程和在线程之间切换的代码。


你的工作是想出一个计划来创建线程并保存/恢复寄存器来在线程之间切换,并实现这个计划。当你完成之后，make grade应该会说你的解决方案通过了uthread测试。


Once you've finished, you should see the following output when you run uthread on xv6 (the three threads might start in a different order):
\$ make qemu

\$ uthread
thread_a started
thread_b started
thread_c started
thread_c 0
thread_a 0
thread_b 0
thread_c 1
thread_a 1
thread_b 1

thread_c 99
thread_a 99
thread_b 99
thread_c: exit after 100
thread_a: exit after 100
thread_b: exit after 100
thread_schedule: no runnable threads
\$
这个输出来自三个测试线程，每个线程都有一个循环，打印一行然后将CPU让给其他线程。


然而，在这个时候，如果没有上下文切换代码，你将看不到任何输出。


您需要在user/uthread.c中的thread\_create()和thread\_schedule()中添加代码，以及user/uthread\_switch.S中的thread\_switch。其中一个目标是确保当thread\_schedule()第一次运行给定线程时，该线程在其自己的堆栈上执行传递给thread\_create()的函数。另一个目标是确保thread\_switch保存正在切换的线程的寄存器，恢复正在切换到的线程的寄存器，并返回后者线程指令的上次离开位置。您需要决定在哪里保存/恢复寄存器;将寄存器保存在struct thread中是一个好的计划。您需要在thread\_schedule中添加一个thread\_switch调用，您可以传递任何需要的参数给thread\_switch，但是目的是从线程t切换到next\_thread。


Some hints:

thread\_switch只需要保存/恢复被调用者保存的寄存器。为什么?

您可以在user/uthread.asm中查看uthread的汇编代码，这对调试可能很有帮助。

要测试您的代码，可能有助于使用riscv64-linux-gnu-gdb逐行检查您的thread\_switch。您可以以这种方式开始：

(gdb) file user/_uthread
Reading symbols from user/_uthread
（gdb）b uthread.c:60


这会在uthread.c的第60行设置断点。在运行uthread之前，断点可能（或可能不）被触发。怎么会发生这种情况？


一旦你的xv6 shell运行，输入 "uthread"，gdb将在第60行处停止。如果您从另一个进程中触发了断点，请继续直到在uthread进程中触发断点。现在，您可以输入以下命令来检查uthread的状态：


  (gdb) p/x *next_thread
With "x", you can examine the content of a memory location:
  (gdb) x/x next_thread->stack
You can skip to the start of thread_switch thus:

   (gdb) b thread_switch
   (gdb) c
You can single step assembly instructions using:

   (gdb) si
gdb的在线文档在这里。


完成




Using threads (moderate)
在这个作业中，您将使用线程和锁来探索哈希表的并行编程。您应该在真正的Linux或MacOS计算机上完成此作业（不是xv6，不是qemu），并且具有多个内核。大多数最新的笔记本电脑都有多核处理器。


这个作业采用UNIX pthread线程库。您可以从手册页面中获取有关它的信息，使用man pthreads查看，并可以在Web上查看，例如在这里、这里和这里。


文件notxv6/ph.c包含一个简单的哈希表，如果从单个线程使用是正确的，但在多线程使用时是不正确的。在您的主要xv6目录中（可能是~/xv6-labs-2021），键入此命令：


\$ make ph
\$ ./ph 1

请注意，要构建ph，Makefile使用您的OS的gcc，而不是6.S081工具。ph的参数指定在哈希表上执行put和get操作的线程数。运行一段时间后，ph 1将产生类似于此的输出：

100000 次puts,3.991秒, 25056次puts/秒

0: 0 keys missing
100000次gets,3.981秒，25118次gets/秒

您看到的数字可能会与这个样本输出相差两倍或更多，这取决于您的计算机的速度，是否具有多核，以及它是否正在做其他事情。


ph运行两个基准测试。首先，它通过调用put()向哈希表中添加大量键并打印出每秒实现的puts速率。然后，它使用get()从哈希表中获取键。它打印应该作为puts的结果在哈希表中的键的数量，但缺失了（在这种情况下为零），并打印它实现的每秒gets数。


您可以通过给ph一个大于1的参数来让它同时从多个线程使用它的哈希表。试试ph 2:


\$ ./ph 2

100000次puts,1.885秒,53044次puts/秒

1: 16579 keys missing
0: 16579 keys missing
200000次gets,4.322秒,46274次gets/秒

ph 2输出的第一行表明，当两个线程同时向哈希表中添加条目时，它们实现了每秒53,044次插入的总速率。这大约是运行ph 1单线程速率的两倍。这是一个出色的“并行加速”约2倍，是人们可能期望的最大值（即，核心数量翻倍，单位时间工作量翻倍）

然而，说16579个键缺失的两行表明应该在哈希表中的大量键不在那里。也就是说，puts应该将这些键添加到哈希表中，但出了问题。请查看notxv6/ph.c，尤其是put()和insert()。


当使用2个线程时为什么有丢失的键，而使用1个线程时没有？识别使用2个线程时可能导致键丢失的事件序列。请在answers-thread.txt中提交你的序列并附上简短的解释。

在notxv6/ph.c中的put和get中插入lock和unlock语句，以便在使用两个线程时始终保持丢失键数为0。相关的pthread调用是：


pthread_mutex_t lock;            // declare a lock
pthread_mutex_init(&lock, NULL); // initialize the lock
pthread_mutex_lock(&lock);       // acquire lock
pthread_mutex_unlock(&lock);     // release lock
这个实验将让你熟悉多线程。你将在用户级线程包中实现线程间切换，使用多线程加速程序，并实现障碍。在编写代码之前，你应该确保你已经阅读了xv6书中的“第7章：调度”并研究了相应的代码。在这个练习中，你将设计用户级线程系统的上下文切换机制，然后实现它。为了让你入门，你的xv6有两个文件user / uthread.c和user / uthread\_switch.S，以及Makefile中的一条规则来构建uthread程序。uthread.c包含大部分用户级线程包和三个简单测试线程的代码。线程包缺少创建线程和在线程之间切换的一些代码。你的任务是制定一个计划来


在进行编码之前，您应该确保已经阅读了xv6书中的“第7章：调度”并研究了相应的代码。

在这个练习中，您将设计用户级线程系统的上下文切换机制，然后实现它。为了让您开始，您的xv6有两个文件user/uthread.c和user/uthread\_switch.S以及Makefile中的一条规则来构建一个uthread程序。uthread.c包含大部分用户级线程包，以及三个简单测试线程的代码。线程包缺少创建线程和在线程之间切换的代码。

您的工作是制定一个计划来创建线程并在线程之间保存/恢复寄存器来切换，并实现该


ph.c中有些情况下并发的put()操作在哈希表中的读写没有重叠，因此不需要锁来保护彼此。您是否可以更改ph.c来利用这种情况来获得某些put()操作的并行加速？提示：每个哈希桶如何？


修改您的代码，使一些put操作在并行运行，同时保持正确性。完成后，当make grade显示您的代码通过ph\_safe和ph\_fast测试时，您就完成了。ph\_fast测试要求两个线程至少获得1.25倍于一个线程的puts/second。


Barrier(moderate)
在这个作业中，您将实现一个屏障：应用程序中所有参与线程必须等待直到其他所有参与线程也到达该点的一点。您将使用pthread条件变量，它们是与xv6的sleep和wakeup类似的序列协调技术。


在这个作业中，您将实现一个障碍：应用程序中所有参与线程必须等待其他所有参与线程到达该点的点。您将使用 pthread 条件变量，它是一种类似于 xv6 的 sleep 和 wakeup 的序列协调技术。您应该在真正的电脑上完成此作业（不是 xv6，不是 qemu）。


notxv6/barrier.c 中包含一个有问题的屏障。


\$ make barrier
\$ ./barrier 2（在命令行中运行 barrier 程序并指定使用 2 个线程）

barrier: notxv6/barrier.c:42: 线程: 断言“i == t”失败。

barrier.c 中的 2 指定了在屏障上同步的线程数量(barrier.c中的nthread)。每个线程执行一个循环。在每次循环迭代中，线程调用 barrier() 并随机睡眠几微秒。断言触发，因为一个线程在另一个线程到达屏障之前离开了屏障。期望的行为是每个线程在 barrier() 中阻塞，直到所有 nthreads 都调用了 barrier()。

你的目标是实现所需的屏障行为。除了你在ph作业中看到的锁原语之外，你还需要以下新的pthread原语；请在此处和此处查看详细信息。


pthread_cond_wait(&cond, &mutex);  // go to sleep on cond, releasing lock mutex, acquiring upon wake up
pthread_cond_broadcast(&cond);     // wake up every thread sleeping on cond
在确保你的解决方案通过 make grade 的 barrier 测试之后。


pthread\_cond\_wait释放了互斥锁当调用，并在返回之前重新获取互斥锁。

给你了 barrier\_init()。你的工作是实现 barrier()，使得不会发生 panic。我们已经定义了 struct barrier，它的字段可供你使用。


There are two issues that complicate your task:

在这个作业中，你将实现一个屏障：在应用程序中的一个点，所有参与线程都必须等待，直到所有其他参与线程也到达该点。你将使用 pthread 条件变量，这是一种类似于 xv6 的 sleep 和 wakeup 的序列协调技术。你应该在真实计算机上完成此作业（不是 xv6，不是 qemu）。文件notxv6/barrier.c包含一个破损的屏障。不要忘记调用 pthread\_cond\_wait。

在本作业中，你将实现一个屏障：在应用程序中的一个点，所有参与线程都必须等待，直到所有其他参与线程也到达了该点。你将使用 pthread 条件变量，它是一种类似于 xv6 的 sleep 和 wakeup 的序列协调技术。

在一个线程，两个线程和多于两个线程的情况下测试你的代码。



Submit the lab
这完成了实验。确保您通过所有 make grade 测试。如果这个实验有问题，不要忘记将您的答案写入 answers-lab-name.txt 中。提交您的更改（包括添加 answers-lab-name.txt）并在实验目录中键入 make handin 以交出您的实验。


Time spent
创建一个新文件time.txt，并在其中放入一个整数，表示你花在实验室的时间（小时）。不要忘记使用git add和git commit提交该文件。


Submit
创建一个新文件time.txt，并在其中添加一个整数，表示你花费在实验室上的小时数。不要忘记使用git add和git commit提交该文件。使用提交网站提交作业。在提交任何作业或实验之前，您需要在提交网站上申请一次API密钥。

经过提交你的最终更改后，键入 make handin 来提交你的实验。


\$ git commit -am "ready to submit my lab"
[util c2e3c8b] ready to submit my lab
 2 files changed, 18 insertions(+), 2 deletions(-)

\$ make handin
tar: Removing leading `/' from member names
访问<https://6828.scripts.mit.edu/2022/handin.py/> 为自己获取API密钥。

Please enter your API key: XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
  % Total    % Received % Xferd  Average Speed   Time    Time     Time  Current
                                 Dload  Upload   Total   Spent    Left  Speed
100 79258  100   239  100 79019    853   275k --:--:-- --:--:-- --:--:--  276k
\$
访问<https://6828.scripts.mit.edu/2022/handin.py/> 为自己获取API密钥。

If you run make handin and you have either uncomitted changes or untracked files, you will see output similar to the following:

M hello.c 代表在版本控制系统中，文件hello.c被标记为修改状态。

?? bar.c 的意思是，这个文件（bar.c）没有被追踪，并且当前不存在于工作目录中。

?? foo.pyc 意思是询问本地目录中是否存在名为 foo.pyc 的文件，此文件可能是文件系统中不存在或是未被跟踪的文件。

Untracked files 将不会交给。继续吗？ [y/N]

在上面的行中检查，确保你的实验解决方案需要的所有文件都被跟踪，即不在以??开头的行中列出。 您可以使用git add filename使git跟踪新创建的文件。

如果 make handin 不能正常工作，请尝试使用 curl 或 Git 命令修复问题。或者，您可以运行 make tarball。这将为您创建一个 tar 文件，然后您可以通过我们的 web 接口上传。


Please run `make grade` to ensure that your code passes all of the tests
Commit any modified source code before running `make handin`
您可以在[https://6828.scripts.mit.edu/2022/handin.py/中检查您提交的状态并下载已提交的代码。](https://6828.scripts.mit.edu/2022/handin.py/%E4%B8%AD%E6%A3%80%E6%9F%A5%E6%82%A8%E6%8F%90%E4%BA%A4%E7%9A%84%E7%8A%B6%E6%80%81%E5%B9%B6%E4%B8%8B%E8%BD%BD%E5%B7%B2%E6%8F%90%E4%BA%A4%E7%9A%84%E4%BB%A3%E7%A0%81%E3%80%82)

Optional challenges for uthread
用户级线程包在多种方面与操作系统交互不良。例如，如果一个用户级线程在系统调用中阻塞，另一个用户级线程将不会运行，因为用户级线程调度程序不知道它的一个线程已经被 xv6 调度程序取消调度。另一个例子是，两个用户级线程不会在不同的核上同时运行，因为 xv6 调度程序不知道有多个线程可以并行运行。请注意，如果两个用户级线程真的并行运行，这种实现将不工作，因为有多个竞争（例如，两个处理器上的线程可以同时调用 thread\_schedule，选择相同的可运行线程，并在不同的处理器上运行它）。


在 xv6 中解决这些问题有几种方法。一种是使用调度器激活，另一种是为每个用户级线程使用一个内核线程（就像 Linux 内核一样）。在 xv6 中实现其中一种方法。这不是容易正确的; 例如，您需要在更新多线程用户进程的页表时实现 TLB 射击。


在你的线程包中添加锁、条件变量、障碍等。

