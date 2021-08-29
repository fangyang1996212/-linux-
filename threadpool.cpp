//
//  threadpool.cpp
//  Thread_Pool
//
//  Created by FY on 2020/12/29.
//  Copyright © 2020 WH-JS. All rights reserved.
//

#include "threadpool.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <errno.h>

void* thread_routine(void *arg)
{
    struct timespec abstime;
    int timeout;
    
    printf("thread %p is starting.\n ", pthread_self());
    
    threadpool_t *pool = (threadpool_t *)arg;
    
    while(1)
    {
        timeout = 0;
        //condition_lock(&pool->ready); // 加两把锁，模拟死锁
        condition_lock(&pool->ready); // 1   对共享任务队列加锁
        
        pool->idle++; // 正在等待，空闲线程数加一
        // 等待队列有任务到来，或线程池销毁通知，当这两个条件不满足的时候等待
        while(pool->first == NULL && !pool->quit)
        {
            printf("thread %p is waiting.\n ", pthread_self());
            // 内部做了三件事：1、对互斥锁解锁。2、在条件变量上等待。3、等待返回的时候对互斥锁重新加锁。
            //condition_wait(&pool->ready);
            clock_gettime(_CLOCK_REALTIME, &abstime); //将当前时间放到 abstime 中
            abstime.tv_sec += 2; // 表示超时时间是两秒
            int status = condition_timedwait(&pool->ready, &abstime);
            if(status == ETIMEDOUT) // 如果超时了
            {
                printf("thread %p is wait timed out.\n ", pthread_self());
                timeout = 1; // 超时标志置为1
                break; // 因为超时了，就跳出循环
            }
        }
        pool->idle--; // 等到了任务（或销毁通知），空闲线程数减一,处于工作状态
        
        // ----------------------------------------------  情况 1
        if(pool->first != NULL) // 如果是等到了任务
        {
            // 从队头取出任务处理
            task_t *t = pool->first;
            pool->first = t->next;
            
            // 执行任务需要一定的时间所以要先解锁,以便生产者线程能够往队列任务添加新任务
            // 以便其他消费者线程能够能够进入等待任务
            /* 解锁访问共享队列资源 */
            condition_unlock(&pool->ready);
            t->run(t->arg);
            free(t);
            condition_lock(&pool->ready);
            
        }
        
        // ----------------------------------------------  情况 2
        if(pool->quit && pool->first==NULL) // 如果是等到了销毁线程通知，且任务都执行完毕
        {
            pool->counter--;
            
            if(pool->counter == 0)
            {
                condition_signal(&pool->ready);// 对销毁线程池的 wait函数 发起通知
                printf("thread %p 对销毁线程池的 wait函数发起通知.\n ", pthread_self());
            }
                //condition_signal(&pool->ready);// 对销毁线程池的 wait函数 发起通知
            
            condition_unlock(&pool->ready); // 当前处于锁1的状态，一定要解锁才能跳出循环哦！
            break; // 跳出 while（1）这个循环
        }
        
        // ----------------------------------------------  情况 3
        if(timeout && pool->first==NULL) // 如果是等到了超时，且任务都执行完毕
        {
            pool->counter--;
            condition_unlock(&pool->ready); // 当前处于锁1的状态，一定要解锁才能跳出循环哦！
            break; // 跳出 while（1）这个循环
        }
        
        condition_unlock(&pool->ready); // 1
    }

    printf("thread %p is exiting.\n ", pthread_self());
    return NULL;
}

// 初始化线程池
void threadpool_init(threadpool_t *pool, int threads)
{
    // 对线程池中各个字段初始化
    condition_init(&pool->ready);
    pool->first = NULL;
    pool->last = NULL;
    pool->counter = 0;
    pool->idle = 0;
    pool->max_threads = threads;
    pool->quit = 0; // 当线程池销毁时 quit=1，默认quit=0
}

// 往线程池中添加任务
void threadpool_add_task(threadpool_t *pool, void *(*run)(void *arg), void *arg)
{
    // 生成新任务
    task_t *newtask = (task_t *)malloc(sizeof(task_t));
    newtask->run = run;
    newtask->arg = arg;
    newtask->next = NULL;
    
    // -- ************* ------------------------- 访问线程池这个共享资源，所以要加锁
    
    condition_lock(&pool->ready);
    // 将任务添加到队列
    if(pool->first == NULL)
        pool->first = newtask;
    else
        pool->last->next = newtask;
    pool->last = newtask;
    
    // 如果有等待线程则唤醒一个
    if(pool->idle > 0)
    {
        condition_signal(&pool->ready);
    }
    else if(pool->counter < pool->max_threads)
    {
        // 没有等待线程，且当前线程数不超过最大线程数，则创建一个新线程
        pthread_t tid;
        pthread_create(&tid, NULL, thread_routine, pool);
        pool->counter++;
    }
    
    condition_unlock(&pool->ready);
    
    // -- ************* ------------------------- 访问共享资源结束，解锁
}

// 销毁线程池
void threadpool_destroy(threadpool_t *pool)
{
    if(pool->quit)
        return;
    
    condition_lock(&pool->ready);
    pool->quit = 1;
    if(pool->counter > 0)
    {
        if(pool->idle) // 处于等待状态中的线程
            condition_broadcast(&pool->ready);// 对等待队列中的线程发起广播
        
        // 处于执行任务状态的线程，不会收到线程销毁的广播
        // 线程池需要等待执行任务的线程全部退出
        while(pool->counter > 0) //阻塞等待线程池中线程全部 exist 再销毁线程池
            condition_wait(&pool->ready);
    }
    condition_unlock(&pool->ready);
    condition_destroy(&pool->ready);
}
