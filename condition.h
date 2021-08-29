//
//  condition.h
//  Thread_Pool
//
//  Created by FY on 2020/12/8.
//  Copyright © 2020 WH-JS. All rights reserved.
//

#ifndef condition_h
#define condition_h
#include <pthread.h>

// 条件变量总是和互斥锁配合使用
typedef struct condition
{
    pthread_mutex_t pmutex;
    pthread_cond_t pcond;
    
}condition_t;

// 八个函数
int condition_init(condition_t *cond);
int condition_lock(condition_t *cond);
int condition_unlock(condition_t *cond);
int condition_wait(condition_t *cond);
int condition_timedwait(condition_t *cond, const struct timespec *abstime);
int condition_signal(condition_t *cond);
int condition_broadcast(condition_t *cond);
int condition_destroy(condition_t *cond);

#endif /* condition_h */
