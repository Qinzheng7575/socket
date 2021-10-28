# reference

https://blog.csdn.net/Pompey_Wang/article/details/81003057?utm_medium=distribute.pc_relevant.none-task-blog-2~default~baidujs_title~default-1.essearch_pc_relevant&spm=1001.2101.3001.4242.2

https://blog.csdn.net/boyixuanbo/article/details/80775807?spm=1001.2101.3001.6650.5&utm_medium=distribute.pc_relevant.none-task-blog-2%7Edefault%7ECTRLIST%7Edefault-5.essearch_pc_relevant&depth_1-utm_source=distribute.pc_relevant.none-task-blog-2%7Edefault%7ECTRLIST%7Edefault-5.essearch_pc_relevant

看这个：
https://blog.csdn.net/lixiaofeng0/article/details/119729152

select与read
https://blog.csdn.net/xing1584114471/article/details/83317976?utm_medium=distribute.pc_relevant.none-task-blog-2~default~baidujs_title~default-0.essearch_pc_relevant&spm=1001.2101.3001.4242.1

非阻塞read的思想（基于open）：
https://www.cnblogs.com/alantu2018/p/8472736.html

# select原理

select，其实就是一个监听的程序，检查文件描述符的改变。看看它的定义

```c
int _stdcall select(int nfds,fd_set*readfds,fd_set*writefds,fd_set*exceptfds,const timeval*timeout)
```

nfds代表最大的文件描述词加1

readfds,writefds,exceptfds是三个fd列表，**正因为键盘标准输入也可以看作一个文件描述符（0），因此可以将键盘和SOCKET端口放在一起监听。**反正我只想监听键盘`输入`，因此我这样写：

```c
retval = select(STDIN+1, &iofds, NULL, NULL, &timeout);//#define STDIN 0
```

但是我在测试的时候，select一直返回`-1`，用`WSAGetLastError()`查看后，发现错误返回值是10038，对应在一个非套接字上尝试了一个操作。

再结合这个[博客](https://blog.csdn.net/dengyejing/article/details/678444)的信息，突然明白了，**根本原因是在Windows上不支持非socket类型的文件描述符，我应该在Linux环境下进行！**

# 下一步改进

> 在之前的结构中，标准服务器流程：
> listen(s,5)//s前面已经定义好，自己的端口啥的
> while(1){
> newsock=accept(s,...)//用刚刚在listen的s来和对端建立连接
> recv接收
> send发送

但是会在`recv`处阻塞，如果`recv()`处一直没有socket发东西，就会阻塞（阻塞状态下）。同时socket可以工作在非阻塞条件下：

> 套接字接口函数是否阻塞，由套接字的状态决定，该状态可通过ioctlsocket( )来设置。
>
> arg = 1;
>
> ioctlsocket( s, FIONBIO,&arg );
>
> select　函数中的timeout,参数至关重要，它可以使select处于三种状态，
> 第一，若将NULL以形参传入，即不传入时间结构，就是将select置于阻塞状态，一定等到监视文件描述符集合中某个文件描述符发生变化为止；
> 第二，若将时间值设为0秒0毫秒，就变成一个纯粹的非阻塞函数，不管文件描述符是否有变化，都立刻返回继续执行，文件无变化返回0，有变化返回一个正值；
> 第三，timeout的值大于0，这就是等待的超时时间，即select在timeout时间内阻塞，超时时间之内有事件到来就返回了，否则在超时后不管怎样一定返回，返回值同上述。 



当套接字处于阻塞状态，执行操作时如果相应事件没有发生，则会阻塞当前进程，直到事件发生或出现错误。如果在非阻塞状态，接口函数始终立即返回，`出现忙等`。

所以通过设置最后一个参数`(&timeout)`解决，





# 显示

![分屏显示](http://life.chinaunix.net/bbsfile/forum/201210/22/1402462yx0wph5b2hy1m1h.jpg)

邓:dog: 非要像这样的效果，找到了一个可以实现之的库ncurses，但是觉得当务之急试试select和其它fd_set的共用能否实现问题