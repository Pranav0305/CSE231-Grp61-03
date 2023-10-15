#include <stdio.h>
#include <stdlib.h>
#include <threads.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <ctype.h>
#include <signal.h>
#include <time.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <pthread.h>

//struct of processes
typedef struct Process
{
    pid_t pid;
    char *name;
    double execTime;
    double waitTime;
    int status;
    int priority;
} Process;

//shared memory 
typedef struct shm_t
{
    sem_t queue_mutex;
    Process ready_queue[100];
    Process process_det[100];
    int ind;
    int st;
    int en;
    double avgRunTime;
    double avgWaitTime;
    // int flag;

} shm_t;

int ncpu;
double tslice;

const char *name = "/shared_mem";
int size = sizeof(shm_t);
int fd;
shm_t *ptr;

char command_arr[100][100];
int command_ind = 0;

void setup()
{
    // size = sizeof(shm_t);
    fd = shm_open(name, O_CREAT | O_RDWR, 0666);
    if (fd == -1)
    {
        perror("shm");
        exit(1);
    }
    if (ftruncate(fd, size) == -1)
    {
        perror("ftruncate");
        exit(1);
    }

    ptr = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    if (ptr == MAP_FAILED)
    {
        perror("map_failed");
        exit(1);
    }

    if (sem_init(&ptr->queue_mutex, 0, 1) == -1)
    {
        perror("sem_init");
        exit(1);
    }

    ptr->st = 0;
    ptr->en = 0;
    // ptr->flag = 0;
    ptr->ind = 0;
    ptr->avgRunTime = 0;
    ptr->avgWaitTime = 0;
}

void cleanup()
{
    if (munmap(ptr, size) == -1)
    {
        perror("munmap");
        exit(1);
    }

    if (close(fd) == -1)
    {
        perror("close");
        exit(1);
    }

    if (shm_unlink(name) == -1)
    {
        perror("shm_unlink");
        exit(1);
    }
}

char dp[100][300];
int dp_ind = 0;
int proc_no = 0;

//function for wait time
double calcWaitTime()
{
    // printf("ind %d\n",ptr->ind);
    int proc = ptr->ind;
    double sum = 0;
    for (int i = 0; i < ptr->ind; i++)
    {
        sum += ptr->process_det[i].waitTime;
    }
    // sum /= proc;
    return sum / proc;
}

//function for execution time
double calcExecTime()
{
    int proc = ptr->ind;
    double sum = 0;
    for (int i = 0; i < ptr->ind; i++)
    {
        sum += ptr->process_det[i].execTime;
    }
    sum /= proc;
    return sum;
}

//printing process history
void print(int ind)
{
    // for(int i = 0; i < command_ind; i++)
    // {
    //     for(int j = 0; j < 100; j++)
    //         printf("%c",command_arr[i][j]);
    //     printf("\n");
    // }
    if(ind == 0)
    {
        return;
    }
    printf("\nProcess Details:\n");
    for (int i = 0; i < ptr->ind; i++)
    {
        printf("Command: %s\n", command_arr[i]);
        printf("PID: %d\n", ptr->process_det[i].pid);
        printf("Execution Time: %lf ms\n", ptr->process_det[i].execTime);
        printf("Wait Time: %lf ms\n", ptr->process_det[i].waitTime);
        printf("\n");
    }
    double avgExec = calcExecTime();
    double avgWait = calcWaitTime();

    printf("Average Wait Time: %lf ms\n", avgWait);
    printf("Average Execution Time: %lf ms\n", avgExec);
}

void trim(char *str)
{
    int start = 0;
    int end = strlen(str) - 1;
    while (str[start] && isspace(str[start]))
    {
        start++;
    }
    while (end >= start && isspace(str[end]))
    {
        end--;
    }
    for (int i = 0; i <= end - start; i++)
    {
        str[i] = str[start + i];
    }

    str[end - start + 1] = '\0';
}

//handling signals -> SIGINT when program ends and displays history
static void sig_handler(int signum)
{
    if (signum == SIGINT)
    {
        int in = ptr->ind;
        char *mssg = "\nExiting the shell.\n";
        print(in);
        cleanup();

        char *command = "make clean";
        int status = system(command);
        write(STDOUT_FILENO, mssg, strlen(mssg));
        if (status == -1)
        {
            perror("system");
            exit(1);
        };
        exit(0);
    }
}

void process()
{

    int org_stdin = dup(STDIN_FILENO);
    struct sigaction sig;
    sig.sa_handler = sig_handler;
    if (sigaction(SIGINT, &sig, NULL) == -1)
    {
        perror("sigaction");
        exit(1);
    }

    do
    {
        printf("[n4m4n_3irl4@archlinux]$ ");
        char command_inp[100];
        char dup_inp[100];
        fgets(command_inp, sizeof(command_inp), stdin);
        // for(int i = 0; i < 100; i++)
        // {
        //     command_arr[command_ind][i] = command_inp[i];
        // }
        command_inp[strcspn(command_inp, "\n")] = '\0';
        for (int i = 0; i < 100; i++)
            dup_inp[i] = command_inp[i];
        // printf("%s",command_inp);


        for(int i = 0; i < 100; i++)
        {
            command_arr[command_ind][i] = dup_inp[i];
        }
        command_ind++;

        trim(command_inp);
        if (strlen(command_inp) == 0)
        {
            continue;
        }

        for (int i = 0; i < 100; i++)
        {
            dp[dp_ind][i] = dup_inp[i];
        }
        dp_ind++;

        /*
        if(strcmp(command_inp,"history") == 0)
        {
            clock_t s=clock();
            time_t start;
            time(&start);
            exec_history[proc_no].process_start[0] = start;
            for(int i = 0; i < 100; i++)
            {
                exec_history[proc_no].command[i] = dup_inp[i];
            }

            int hi_st = fork();
            if(hi_st == 0)exit(0);
            if(hi_st > 0)
            {
                wait(NULL);
                clock_t e=clock();
                time_t end;
                time(&end);
                exec_history[proc_no].process_pid[0] = hi_st;
                exec_history[proc_no].process_end[0] = end;
                exec_history[proc_no].is_pipe = 0;
                exec_history[proc_no].num_commands = 1;
                exec_history[proc_no].process_exec[0]=(double)(e-s)/CLOCKS_PER_SEC;
                proc_no++;

            }
            // printf("YES");
            for(int i = 0; i < dp_ind; i++)
                printf("%d %s\n",i+1,dp[i]);
            continue;
        }
        */

        command_inp[strcspn(command_inp, "\n")] = '\0';
        int pipeCnt = 0;

        for (int i = 0; i < 100; i++)
        {
            if (command_inp[i] == '|')
                pipeCnt++;
        }

        int fd[pipeCnt][2];
        pid_t children[pipeCnt + 1];
        if (pipeCnt == 0)
        {
            clock_t s = clock();
            time_t start;
            time(&start);

            pid_t st = fork();
            if (st < 0)
                printf("Something wrong\n");
            else if (st == 0)
            {
                char *token = strtok(command_inp, " ");
                char *command[20];
                int ind = 0;

                while (token != NULL)
                {
                    command[ind] = token;
                    ind++;
                    token = strtok(NULL, " ");
                }

                // printf("%s\n",command[0]);
                if (strcmp(command[0], "submit") == 0)
                {
                    int pr;
                    if (ind == 2)
                        pr = 1;
                    if (ind == 3)
                        pr = atoi(command[2]);
                    // printf("pr: %d\n", pr);
                    if (pr <= 0 || pr > 4)
                    {
                        printf("Invalid priority\n");
                        continue;
                    }

                    char *args[] = {command[1], NULL};

                    process_call(command[1], args, pr, dup_inp);
                    exit(0);
                }

                command[ind] = NULL;
                char path[100] = "/usr/bin/";
                if (command_inp[0] == '.' && command_inp[1] == '/')
                {
                    char chr_path[100] = "";
                    int chrind = 0;
                    for (int i = 2; i < 100; i++)
                    {
                        chr_path[chrind] = command[0][i];
                        chrind++;
                    }
                    process_call(*chr_path, command, 1, dup_inp);
                }
                else
                {
                    ind = 0;
                    for (int i = 9; i < 100; i++)
                    {
                        path[i] = command[0][ind];
                        ind++;
                    }
                    process_call(*path, command, 1, dup_inp);
                }

                exit(0);
            }
            else
            {
                wait(NULL);
            }
        }

        proc_no++;

    } while (1);
}

void process_call(char *path, char *command[], int pr, char process_name[100])
{
    // printf("command: %s\n", process_name);
    pid_t run_fork = fork();
    if (run_fork == 0)
    {
        pid_t st = fork();
        if (st == 0)
        {
            if (execv(path, command) == -1)
            {
                perror("execv");
                exit(1);
            }
        }
        else
        {
            kill(st, SIGSTOP);
            Process p;
            p.pid = st;
            p.waitTime = 0;
            p.execTime = 0;
            p.status = 0;
            p.priority = pr - 1;


            //SEMAPHORES 
            if (sem_wait(&ptr->queue_mutex) == -1)
            {
                perror("sem_wait");
                exit(1);
            }

            ptr->ready_queue[ptr->en] = p;
            for(int j = 0; j < 100; j++)
            {
                ptr->ready_queue[ptr->en].name[j] = process_name[j];
            }

            // printf("name: %s\n",ptr->ready_queue[ptr->en].name);
            ptr->en++;

            if (sem_post(&ptr->queue_mutex) == -1)
            {
                perror("sem_post");
                exit(1);
            }

            while (1)
            {
                for (int i = ptr->st; i < ptr->en; i++)
                {
                    if (ptr->ready_queue[i].pid == st)
                    {
                        int stat;
                        if (waitpid(st, &stat, WNOHANG) > 0)
                        {
                            // printf("\nTerminated: %d\n ",ptr->ready_queue[i].pid);
                            Process p;
                            p.pid = ptr->ready_queue[i].pid;
                            p.execTime = ptr->ready_queue[i].execTime;
                            p.waitTime = ptr->ready_queue[i].waitTime;
                            p.name = ptr->ready_queue[i].name;
                            ptr->process_det[ptr->ind] = p;
                            ptr->ind++;
                            // printf(" %d\n",ptr->ind);
                            ptr->ready_queue[i].status = 1;
                            exit(0);
                        }
                    }
                }
            }
        }
    }
    else
        return;
}

void schedule()
{
    //priority queues
    Process queues[4][100];
    int st_arr[4] = {0, 0, 0, 0};
    int en_arr[4] = {0, 0, 0, 0};
    while (1)
    {
        // printf("ind: %d\n",ptr->ind);
        
        // printf("[n4m4n_3irl4@archlinux]$ ");
        usleep(10000);

        for (int i = ptr->st; i < ptr->en; i++)
        {
            //SEMAPHORES
            if (sem_wait(&ptr->queue_mutex) == -1)
            {
                perror("sem_post");
                exit(1);
            }

            if (ptr->ready_queue[i].status == 1)
            {
                // printf("Stopped: %d\n",ptr->ready_queue[i].pid);
                ptr->st++;
            }
            else
            {
                // printf("Next Process:\n");
                Process p = ptr->ready_queue[i];
                int ind = p.priority;
                // printf("ind %d\n",ind);
                queues[ind][en_arr[ind]] = p;
                en_arr[ind]++;
                ptr->st++;
            }

            // printf("pr st en: %d %d\n",st_arr[ind],en_arr[ind]);
            // printf("st en: %d %d\n", ptr->st, ptr->en);

            if (sem_post(&ptr->queue_mutex) == -1)
            {
                perror("sem_post");
                exit(1);
            }
            // printf("bc");
        }

        // printf("\nPrinting\n");
        int q;
        for (int i = 0; i < 4; i++)
        {
            int cnt = 0;
            if (st_arr[i] != en_arr[i])
            {
                q = i;
                break;
            }
        }

        printf("\n");
        for (int i = 0; i < 4; i++)
        {
            //Managing the queues
            printf("Priority %d: ", i + 1);
            int cnt = 0;
            for (int j = st_arr[i]; j < en_arr[i]; j++)
            {
                if(i == q)
                {
                    if (cnt < ncpu)
                    {
                        queues[i][j].execTime += tslice;
                    }
                    else
                        queues[i][j].waitTime += tslice;
                    cnt++;
                }
                else
                {
                    queues[i][j].waitTime += tslice;
                }
                    

                printf("%d ", queues[i][j].pid);
            }
            printf("\n");
        }
        printf("\n");


        for (int i = q; i <= q; i++)
        {
            int cnt = 0;

            for (int j = st_arr[i]; j < en_arr[i]; j++)
            {
                if (cnt >= ncpu)
                    break;
                
                //SEMAPHORES
                if (sem_wait(&ptr->queue_mutex) == -1)
                {
                    perror("sem_post");
                    exit(1);
                }

                kill(queues[i][j].pid, SIGCONT);

                if (sem_post(&ptr->queue_mutex) == -1)
                {
                    perror("sem_post");
                    exit(1);
                }
                cnt++;
            }
        }
        
        
        printf("[n4m4n_3irl4@archlinux]$ ");
        usleep(tslice*1000);

        for (int i = q; i <=q; i++)
        {
            int cnt = 0;
            for (int j = st_arr[i]; j < en_arr[i]; j++)
            {
                if (cnt >= ncpu)
                    break;
                if (sem_wait(&ptr->queue_mutex) == -1)
                {
                    perror("sem_post");
                    exit(1);
                }

                kill(queues[i][j].pid, SIGSTOP);

                if (queues[i][j].status == 1)
                {
                    st_arr[i]++;
                }
                else if (queues[i][j].priority < 3)
                {
                    queues[i][j].priority++;
                    ptr->ready_queue[ptr->en] = queues[i][j];
                    ptr->en++;
                    st_arr[i]++;
                }
                else
                {
                    ptr->ready_queue[ptr->en] = queues[i][j];
                    ptr->en++;
                    st_arr[i]++;
                }

                if (sem_post(&ptr->queue_mutex) == -1)
                {
                    perror("sem_post");
                    exit(1);
                }
                cnt++;
            }
        }

        // continue;
    }
}

int main(int argc, char *argv[])
{
    // printf("%s\n",argc);
    printf("%d\n", argc);
    // printf("%lf %lf\n",argv[1],argv[2]);

    // setup shm
    setup();
    if(argc!=3){
        printf("Usage: %s <arg1> <arg2>\n", argv[0]);
        exit(1);
    }
    if (argc == 3)
    {
        ncpu = atoi(argv[1]);
        char *endptr;
        tslice = strtod(argv[2], &endptr);
        if (endptr == argv[1] || *endptr != '\0')
        {
            printf("Error\n");
            exit(1);
        }
        pid_t scheduler_fork = fork();
        if (scheduler_fork < 0)
        {
            perror("fork");
            exit(1);
        }
        else if (scheduler_fork == 0)
        {
            usleep(tslice*1000);
            pid_t bg_fork = fork();
            if (bg_fork < 0)
            {
                perror("fork");
                exit(1);
            }
            else if (bg_fork == 0)
            {
                schedule();
                exit(0);
            }
            else
                exit(0);
            // exit(0);
        }
        else
        {
            // sleep(1);
            process();
        }
    }
}
