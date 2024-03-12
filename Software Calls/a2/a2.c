#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/types.h>
#include "a2_helper.h"
#include <fcntl.h>

sem_t sem4, sem2, sem, semaux, semaux2;
sem_t *sem53, *sem63, *sem61;

pthread_mutex_t lock;
pthread_cond_t cond, cond2;
const int max = 4;
int value = 0, value2 = 0;
int count = 0;

int p9()
{
    info(BEGIN, 9, 0);
    info(END, 9, 0);
    return 0;
}
void *thFunction5(void *arg)
{
    sem53 = sem_open("/sem53", O_EXCL);
    sem63 = sem_open("/sem63", O_EXCL);
    sem61 = sem_open("/sem61", O_EXCL);
    int *val = (int *)arg;
    if (*val == 4)
    {
        sem_wait(&sem4);
        info(BEGIN, 5, 4);
        info(END, 5, 4);
        sem_post(&sem2);
    }
    else if (*val == 2)
    {
        info(BEGIN, 5, 2);
        sem_post(&sem4);
        sem_wait(&sem2);
        info(END, 5, 2); 
    }
    else if (*val == 3)
    {
        sem_wait(sem63);
        info(BEGIN, 5, 3);
        info(END, 5, 3);
        sem_post(sem53);
    }
    else
    {
        info(BEGIN, 5, *val);
        info(END, 5, *val);
    }
    pthread_exit(0);
}
void *thFunction6(void *arg)
{
    sem53 = sem_open("/sem53", O_EXCL);
    sem63 = sem_open("/sem63", O_EXCL);
    sem61 = sem_open("/sem61", O_EXCL);
    int *val = (int *)arg;
    if (*val == 4)
    {
        sem_wait(&sem4);
        info(BEGIN, 6, 4);
        info(END, 6, 4);
        sem_post(&sem2);
    }
    else if (*val == 2)
    {
        info(BEGIN, 6, 2);
        sem_post(&sem4);
        sem_wait(&sem2);
        info(END, 6, 2); 
    }
    else if (*val == 1)
    {
        sem_wait(sem53);
        info(BEGIN, 6, 1);
        info(END, 6, 1);
    }
    else if (*val == 3)
    {
        info(BEGIN, 6, 3);
        info(END, 6, 3);
        sem_post(sem63);
    }
    else
    {
        info(BEGIN, 6, *val);
        info(END, 6, *val);
    }
    pthread_exit(0);
}
void *thFunction7(void *arg)
{
    int *val = (int *)arg;
    pthread_mutex_lock(&lock);
    while (*val != 13 && count == 0)
    {
        pthread_cond_wait(&cond, &lock);
    }
    pthread_mutex_unlock(&lock);
    sem_wait(&sem);
    info(BEGIN, 7, *val);
    if (*val == 13)
    {
        count = 1;
        pthread_cond_broadcast(&cond);
    }
    sem_getvalue(&sem, &value);
    if (value == 0)
    {
        sem_post(&semaux);
    }
    if (*val == 13)
    {
        sem_wait(&semaux);
    }
    pthread_mutex_lock(&lock);
    while (*val != 13 && count == 1)
    {
        pthread_cond_wait(&cond2, &lock);
    }
    pthread_mutex_unlock(&lock);
    info(END, 7, *val);
    if (*val == 13)
    {
        count = 2;
        pthread_cond_broadcast(&cond2);
    }
    sem_post(&sem);

    pthread_exit(0);
}
int p5()
{
    info(BEGIN, 5, 0);
    int childPid;
    int status9;
    childPid = fork();

    if (childPid == 0)
    {
        status9 = p9();
        exit(status9);
    }
    sem_init(&sem4, 1, 0);
    sem_init(&sem2, 1, 0);
    pthread_t th[5];
    int th_array[5];
    for (int i = 0; i < 5; i++)
    {
        th_array[i] = i + 1;
        pthread_create(&th[i], NULL, thFunction5, &th_array[i]);
    }
    for (int i = 0; i < 5; i++)
    {
        pthread_join(th[i], NULL);
    }
    wait(&status9);
    info(END, 5, 0);
    sem_destroy(&sem4);
    sem_destroy(&sem2);
    return 0;
}
int p4()
{
    info(BEGIN, 4, 0);
    int childPid;
    int status5;
    childPid = fork();

    if (childPid == 0)
    {
        status5 = p5();
        exit(status5);
    }
    wait(&status5);
    info(END, 4, 0);
    return 0;
}
int p6()
{
    info(BEGIN, 6, 0);
    pthread_t th[5];
    int th_array[5];
    for (int i = 0; i < 5; i++)
    {
        th_array[i] = i + 1;
        pthread_create(&th[i], NULL, thFunction6, &th_array[i]);
    }
    for (int i = 0; i < 5; i++)
    {
        pthread_join(th[i], NULL);
    }
    info(END, 6, 0);
    return 0;
}
int p7()
{
    info(BEGIN, 7, 0);
    sem_init(&sem, 1, 4);
    sem_init(&semaux, 1, 0);
    sem_init(&semaux2, 1, 0);
    pthread_t th[44];
    int th_array[44];
    for (int i = 0; i < 44; i++)
    {
        
            th_array[i] = i + 1;
            pthread_create(&th[i], NULL, thFunction7, &th_array[i]);
        
    }
    for (int i = 0; i < 44; i++)
    {
        pthread_join(th[i], NULL);
    }
    info(END, 7, 0);
    sem_destroy(&sem);
    sem_destroy(&semaux);
    sem_destroy(&semaux2);
    return 0;
}

int p8()
{
    info(BEGIN, 8, 0);
    info(END, 8, 0);
    return 0;
}
int p3()
{
    info(BEGIN, 3, 0);
    int childPid;
    int status8;
    childPid = fork();

    if (childPid == 0)
    {
        status8 = p8();
        exit(status8);
    }
    wait(&status8);
    info(END, 3, 0);
    return 0;
}

int p2()
{
    info(BEGIN, 2, 0);
    int childPid;
    int status3, status4;
    childPid = fork();

    if (childPid == 0)
    {
        status4 = p4();
        exit(status4);
    }
    childPid = fork();

    if (childPid == 0)
    {
        status3 = p3();
        exit(status3);
    }

    wait(&status4);
    wait(&status3);
    info(END, 2, 0);
    return 0;
}

int main(int argc, char **argv)
{
    init();
    info(BEGIN, 1, 0);
    sem53 = sem_open("/sem53", O_CREAT, 0644, 0);
    sem63 = sem_open("/sem63", O_CREAT, 0644, 0);
    sem61 = sem_open("/sem61", O_CREAT, 0644, 0);
    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&cond, NULL);
    pthread_cond_init(&cond2, NULL);

    int status2, status6, status7;
    int childPid;

    childPid = fork();

    if (childPid == 0)
    {
        status2 = p2();
        exit(status2);
    }

    childPid = fork();

    if (childPid == 0)
    {
        status6 = p6();
        exit(status6);
    }

    childPid = fork();

    if (childPid == 0)
    {
        status7 = p7();
        exit(status7);
    }

    wait(&status2);
    wait(&status6);
    wait(&status7);
    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&cond);
    pthread_cond_destroy(&cond2);
    sem_close(sem53);
    sem_close(sem63);
    sem_close(sem61);
    info(END, 1, 0);
    return 0;
}