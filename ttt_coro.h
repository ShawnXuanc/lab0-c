#include <setjmp.h>
#include <stdint.h>
#include <unistd.h>
#include "list.h"

typedef int (*algo)(char *, char);

struct task {
    jmp_buf env;
    struct list_head list;
    char task_name[10];
    char turn;
    algo ai_algo;
};

struct arg {
    char turn;
    char *task_name;
    algo ai_algo;
};



static LIST_HEAD(tasklist);
static void (**tasks)(void *);
static struct arg *args;
static int ntasks;
static jmp_buf sched;
static struct task *cur_task;
