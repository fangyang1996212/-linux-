//
//  main.cpp
//  Thread_Pool
//
//  Created by FY on 2020/12/8.
//  Copyright © 2020 WH-JS. All rights reserved.
//

#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include<stdlib.h>
#include "threadpool.hpp"

void* mytask(void *arg) // 这是一个指针函数，返回值为 *void， mytask为该函数的首地址
{
    printf("thread %p is working on task %d\n", pthread_self(), *(int*)arg);
    sleep(1); // 执行时间为 1秒
    free(arg); // 执行完任务释放 arg
    return NULL;
}

int main(int argc, const char * argv[]) {

    threadpool_t pool;
    threadpool_init(&pool, 3);
    
    //向线程池中添加 10 个任务
    for(int i=0; i < 10; i++)
    {
        int *arg = (int*)malloc(sizeof(int));
        *arg = i;
        
        threadpool_add_task(&pool, mytask, arg); // 这里 arg 不能替换为 &i，有静态问题
        
        // 静态问题：假设某一刻 i=1，当任务没执行的时候执行循环 i = 2，i 改变了，而任务内部指针指向i变量，任务序号就变咯哒！
        // 简而言之，i 变量不能变，但是却改变了。
    }
    
    //sleep(15);
    threadpool_destroy(&pool);
    
    return 0;
}
