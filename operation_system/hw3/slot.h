#ifndef _SLOT_H_
#define _SLOT_H_

#include <linux/ioctl.h>
#include <linux/types.h>

#define MAX_MINOR_DEVICES 256
#define CONDITION_NEVER 0
#define CONDITION_SEQ 1
#define CONDITION_PID 2

int my_open(struct inode*, struct file*);

int my_release(struct inode*, struct file*);
ssize_t my_write(struct file* file, const char* buf, size_t count, loff_t* f_pos);

ssize_t my_read(struct file*, char*, size_t, loff_t*);

int my_ioctl(struct inode* inode, struct file* filp, unsigned int cmd, unsigned long arg);
struct win_condition {
    int type;
    int argument;
};
struct slot_machine {
    unsigned int cash;
    pid_t operator_pid;
    struct win_condition condition;
    int initialized;
    int ref_count;
    int argument;
};
#define MY_MAGIC 'r'
#define GET_CONDITION _IOR(MY_MAGIC, 0, struct win_condition)
#define SET_CONDITION _IOW(MY_MAGIC, 1, struct win_condition)
#define PRIZE_AMOUNT  _IO(MY_MAGIC, 2)

#endif
