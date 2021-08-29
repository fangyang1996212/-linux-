//
//  threadpool.hpp
//  Thread_Pool
//
//  Created by FY on 2020/12/29.
//  Copyright © 2020 WH-JS. All rights reserved.
//

#ifndef threadpool_hpp
#define threadpool_hpp
#include "condition.h"

// 任务结构体（单链表），把任务放入任务队列，由线程池中线程来执行
typedef struct task
{
    void* (*run)(void *arg); // 任务回调函数
    void *arg; // 回调函数参数
    struct task *next;
}task_t;

// 线程池结构体（共 7 个成员）
typedef struct threadpool
{
    condition_t ready; //条件变量(已经被封装了)，任务准备就绪（有任务到达）或者线程销毁通知
    task_t *first; // 任务队列头指针
    task_t *last; // 任务队列尾指针
    int counter;// 线程池中当前线程数
    int idle; //闲置的线程数（等待的线程数）
    int max_threads; //线程池的线程容量（阈值）
    int quit; //销毁线程池的时候置 1（是否销毁线程池？销毁就执行 quit = 1）
    
}threadpool_t;

// 初始化线程池
void threadpool_init(threadpool_t *pool, int threads); // 最多创建 threads 个线程

// 往线程池中添加任务
void threadpool_add_task(threadpool_t *pool, void* (*run)(void *arg), void *arg);

// 销毁线程池
void threadpool_destroy(threadpool_t *pool);

#endif /* threadpool_hpp */
