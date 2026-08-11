#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>

void pick_fork(int semid, int sem_num)
{
    struct sembuf op;
    op.sem_num=sem_num;
    op.sem_op=-1;
    op.sem_flg=0;
    if (semop(semid, &op, 1) == -1) {
        perror("semop wait");
        exit(1);
    }
}

void release_fork(int semid, int sem_num)
{
    struct sembuf op;
    op.sem_num=sem_num;
    op.sem_op=1;
    op.sem_flg=0;
    if (semop(semid, &op, 1) == -1) {
        perror("semop signal");
        exit(1);
    }
}

void philosopher(int id, int semid)
{
    int left=id;
    int right=(id+1)%5;
    for (int i=0; i<5; i++)
    {
        printf("Philosopher %d is thinking\n", id);
        sleep(1);
        /* HUNGRY */
        printf("Philosopher %d is hungry\n", id);
        if (id % 2 == 0)
        {
            pick_fork(semid, left);
            printf("Philosopher %d picked LEFT fork %d\n", id, left);
            pick_fork(semid, right);
            printf("Philosopher %d picked RIGHT fork %d\n", id, right);
        }
        else
        {
            pick_fork(semid, right);
            printf("Philosopher %d picked RIGHT fork %d\n", id, right);
            pick_fork(semid, left);
            printf("Philosopher %d picked LEFT fork %d\n", id, left);
        }
        printf("***** Philosopher %d is EATING *****\n", id);
        sleep(2);
        release_fork(semid, left);
        release_fork(semid, right);
        printf("Philosopher %d released forks %d and %d\n\n", id, left, right);
    }
    printf("Philosopher %d finished eating and is exiting\n", id);
    exit(0);
}

int main()
{
    int semid;
    pid_t pid;
    semid = semget(IPC_PRIVATE, 5, IPC_CREAT | 0666);
    if (semid == -1)
    {
        perror("semget");
        exit(EXIT_FAILURE);
    }
    printf("Semaphore ID = %d\n", semid);
    for (int i=0; i<5; i++)
    {
        if (semctl(semid, i, SETVAL, 1) == -1)
        {
            perror("semctl SETVAL");
            semctl(semid, 0, IPC_RMID);

            exit(EXIT_FAILURE);
        }
    }

    for (int i=0; i<5; i++)
    {
        pid = fork();

        if (pid == -1)
        {
            perror("fork");
            semctl(semid, 0, IPC_RMID);
            exit(EXIT_FAILURE);
        }

        if (pid == 0)
        {
            philosopher(i, semid);
            exit(0);
        }
    }
    for (int i=0; i<5; i++)
    {
        if (wait(NULL) == -1)
        {
            perror("wait");
        }
    }

    printf("\nAll philosophers have finished.\n");
    if (semctl(semid, 0, IPC_RMID) == -1)
    {
        perror("semctl IPC_RMID");
        exit(EXIT_FAILURE);
    }
    printf("Semaphore set removed.\n");

    return 0;
}
