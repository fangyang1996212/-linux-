#  基于 linux 的线程池 
从零开始搭建了一个线程，该线程池可以随任务减少线程数量动态的减少

### 线程池的作用：对线程池里面任务队列的管理；对线程池里面线程的管理；

### 步骤 1：
- 设计 condition 结构体，封装条件变量与互斥量，把 condition 结构重命名为 condition_t 类型 。
- 该结构体有两个成员变量：pthread_mutex_t pmutex、pthread_cond_t pcond;

### 步骤 2 ：设计 8 个函数，封装条件变量与互斥量相关的函数。
	• condition_init()
	• condition_lock()
	• condition_unlock()
	• condition_wait()
	• condition_timewait()
	• condition_signal()
	• condition_broadcast()
	• condition_destory()


### 步骤 3 ：设计任务结构体 task，把 task 结构重命名为 task_t 类型 。
	• 该任务结构体内部有 3 个成员变量
		○ 回调函数指针：返回值为 void* ，参数类型为一个void* 的函数指针 ；
			void* (*func)(void* arg)
		○ 回调函数的参数；
		○ 指向下一个任务的 task 类型的指针 next；  // struct  task *next

### 步骤 4：设计线程池结构体 threadpool，里面有 7 个成员变量，把 threadpool结构重命名为 threadpool_t 类型 。
	• int  max_threads; // 线程池可容纳的最大线程数
	• int  counts; // 线程池中当前线程总数
	• int  idle; // 线程池中闲置线程数
	• condition_t  ready; //条件变量，线程池有任务可做或线程池销毁通知到达
	• int  quit; // 线程池销毁时 quit = 1，初始化为 0
	• task_t  *first;  // 线程池中任务链表的头指针
	• task_t  *last;  // 线程池中任务链表的尾指针

### 步骤 5 ：设计 3 个函数，操作线程池。
	• 初始化线程池；
	• 给线程池添加任务；
	• 销毁线程池；

### 步骤 6 ：创建新线程的时候，新线程的函数要做的事情
	• 对共享任务队列加锁；
	• 线程等待的情况
		○ 当任务队列无任务，且没有收到线程池销毁通知的时候，线程等待；
	• 线程不等待的三种情况
		○ 情况1：等待到了任务 。
		○ 情况2：等待到线程池销毁通知 。
		○ 情况3：使用 timewait 等到了超时，timeout  = 1

### 步骤 7 ：销毁线程池
	• 销毁线程池中线程；
  使得 counts == 0
	• 销毁条件变量；

