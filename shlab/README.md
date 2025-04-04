# tsh lab
通过trace文件一点点完善 tiny shell 的实现。

## trace1 ~ 5
`eval(char *cmdline)` 和 `builtin_cmd(char **argv) ` 两个函数在第三版第八章p525页给出了框架。<br>
只要些许变动即可对应 tshref 要求的输出。

```
void eval(char *cmdline)
{
    char *argv[MAXARGS];
    char buf[MAXLINE];
    int bg;
    pid_t pid;

    strcpy(buf, cmdline);
    bg = parseline(buf, argv);
    if (argv[0] == NULL) return;

    if (!builtin_cmd(argv)) {
        if ((pid = fork()) == 0) {
            if (execve(argv[0], argv, environ) < 0) {
                printf("%s: Command not found. \n", argv[0]);
                exit(0);
            }
        }
        if (!bg) {
            int status;
            if (waitpid(pid, &status, 0) < 0)
                unix_error("waitfg: waitpid error");
        }
        else{
            printf("[%d] (%d) %s", pid2jid(pid), pid, cmdline);
        }
    }
    return;
}
int builtin_cmd(char **argv) 
{
    if (!(strcmp(argv[0], "quit")))
        exit(0);
    if (!(strcmp(argv[0], "jobs"))) {
        listjobs(jobs);
        return 1;
    }
    return 0;     /* not a builtin command */
}
```

## trace 6 ~ 8

这里就不能直接用waitpid了，否则会错过信号。所以要实现 `waitfg` 函数。<br>
trace 6 要求实现 sigint_handler 还有 sigchld_handler 。<br>
trace 7 只是检查目前为止的正确性。<br>
trace 8 要处理 SIGTSTP。
```
void eval(char *cmdline)
{
    ...
    if (!builtin_cmd(argv)) {
        if ((pid = fork()) == 0) {
            setpgid(0, 0); /* 如果不把子进程放到同一个进程组，也会杀死shell*/
            if (execve(argv[0], argv, environ) < 0) {
                printf("%s: Command not found. \n", argv[0]);
                exit(0);
            }
        }
        if (!bg) {
            addjob(jobs, pid, FG, cmdline);
            waitfg(pid);
        }
    ...
    }
    ...
}

void waitfg(pid_t pid)
{
    while (pid == fgpid(jobs)) {
        sleep(1);
    }
    return;
}

void sigchld_handler(int sig) 
{
    int old_errno = errno;
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG | WUNTRACED)) > 0) { /* 非阻塞、能捕捉子程序暂停 */
        if (WIFEXITED(status)) {
            deletejob(jobs, pid);
        } 
        else if (WIFSIGNALED(status)) {
            printf("Job [%d] (%d) terminated by signal %d\n", 
                pid2jid(pid), pid, WTERMSIG(status));
            deletejob(jobs, pid);
        } 
        else if (WIFSTOPPED(status)) {
            printf("Job [%d] (%d) stopped by signal %d\n", pid2jid(pid), pid, WSTOPSIG(status));
            struct job_t *job = getjobpid(jobs, pid);
            job->state = ST;
        }
    }
    errno = old_errno;
    return;
}

void sigint_handler(int sig) 
{
    pid_t pid = fgpid(jobs);
    if (pid != 0) {
        kill(-pid, SIGINT);
    }
}

void sigtstp_handler(int sig) 
{
    pid_t pid = fgpid(jobs);
    if (pid != 0) {
        kill(-pid, SIGTSTP);
    }
    return;
}

```

# trace 9 ~ 13

就是让我们实现 `do_bgfg` ，来处理内置 `fg` 和 `bg` 命令。
```
void do_bgfg(char **argv) 
{
    int jid;
    struct job_t *job;
    jid = atoi(&argv[1][1]);
    job = getjobjid(jobs, jid);
    if (!(strcmp(argv[0], "bg"))) {
        job->state = BG;
        kill(-(job->pid), SIGCONT);
        printf("[%d] (%d) %s", jid, job->pid, job->cmdline);
    }
    else if (!(strcmp(argv[0], "fg"))) {
        job->state = FG;
        kill(-(job->pid), SIGCONT);
        waitfg(job->pid);
    }
    return;
}

```

# trace 14 ~ 16

其中 `trace 14` 要求 `do_bgfg` 的错误处理, `trace 15` 和 `trace 16` 就是综合测试了。

```
void do_bgfg(char **argv) 
{
    int jid;
    struct job_t *job;

    if (argv[1] == NULL) {
        printf("%s command requires PID or %%jobid argument\n", argv[0]);
        return;
    }

    if (argv[1][0] == '%') {
        jid = atoi(&argv[1][1]);
        job = getjobjid(jobs, jid);
        if (job == NULL) {
            printf("%%%d: No such job\n", jid);
            return;
        }
    } else if (isdigit(argv[1][0])) {
        jid = atoi(argv[1]);
        job = getjobpid(jobs, jid);
        if (job == NULL) {
            printf("(%d): No such process\n", jid);
            return;
        }
    }
    else {
        printf("%s: argument must be a PID or %%jobid\n", argv[0]);
        return;
    }

    if (!(strcmp(argv[0], "bg"))) {
        kill(-(job->pid), SIGCONT);
        job->state = BG;
        printf("[%d] (%d) %s", jid, job->pid, job->cmdline);
    }
    else if (!(strcmp(argv[0], "fg"))) {
        kill(-(job->pid), SIGCONT);
        job->state = FG;
        waitfg(job->pid);
    }
    return;
}
```
