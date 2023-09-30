/* chat.c: Example char device module.
 *
 */
 /* Kernel Programming */
#define MODULE
#define LINUX
#define __KERNEL__

#include <linux/kernel.h>  	
#include <linux/module.h>
#include <linux/fs.h>       		
#include <asm/uaccess.h>
#include <linux/errno.h>  
#include <asm/segment.h>
#include <asm/current.h>
#include <linux/slab.h>
#include "slot.h"

#define MY_DEVICE "slot"

MODULE_AUTHOR("Anonymous");
MODULE_LICENSE("GPL");

/* globals */
int my_major = 0; /* will hold the major # of my device driver */
struct slot_machine* slot_machines;

static struct file_operations my_fops = {
    .open = my_open,
    .release = my_release,
    .write = my_write,
    .read = my_read,
    .ioctl = my_ioctl,
};

int my_open(struct inode* inode, struct file* filp){
int minor = MINOR(inode->i_rdev);
    pid_t pid = current->pid;

    // Check if the slot machine has been initialized
    if (slot_machines[minor].initialized == 0) {
        // Initialize the slot machine (set initial values)
        slot_machines[minor].cash = 0;
        slot_machines[minor].operator_pid = pid;
        slot_machines[minor].condition.type = CONDITION_NEVER;
        slot_machines[minor].condition.argument = 0;
        slot_machines[minor].initialized = 1;

        // Initialize the reference count to 1
        slot_machines[minor].ref_count = 1;

    }
    else {

            slot_machines[minor].ref_count++;   
    }
    return 0;
}

int my_release(struct inode* inode, struct file* filp) {
    int minor = MINOR(inode->i_rdev);
    struct slot_machine* machine = &slot_machines[minor];

    // Decrement the reference count
    machine->ref_count--;

    // Check if this is the last file descriptor
    if (machine->ref_count <= 0) {
  
        // Reset the machine to uninitialized state
        machine->cash = 0;
        machine->operator_pid = 0;
        machine->condition.type = CONDITION_NEVER;
        machine->condition.argument = 0;
        machine->initialized = 0; // Using 0 for false, to be consistent with your previous code
        // Reset the reference count for the next open
        machine->ref_count = 0;
    }

    return 0;
}


ssize_t my_write(struct file* filp, const char* buf, size_t count, loff_t* f_pos) {
    int minor = MINOR(filp->f_dentry->d_inode->i_rdev);
    unsigned int amount;


    if (buf == NULL) {
        return -EFAULT; // Invalid buffer
    }

    if (count != sizeof( int)) {
        return -EINVAL; // Invalid write operation
    }


    if (copy_from_user(&amount, buf, sizeof( int))) {
        return -EFAULT; // Error copying data from user space
    }
   
    // Add the amount to the slot machine's cash
    slot_machines[minor].cash += amount;

    return sizeof( int);
}

ssize_t my_read(struct file* filp, char* buf, size_t count, loff_t* f_pos){
    int minor = MINOR(filp->f_dentry->d_inode->i_rdev);
    unsigned int winnings = 0;

    if (buf == NULL) {
        return -EFAULT; // Invalid buffer
    }

    if (count != sizeof( int)) {
        return -EINVAL; // Invalid write operation
    }



    if (slot_machines[minor].condition.type == CONDITION_NEVER) {
        // Always loses
        winnings = 0;
    }
    else if (slot_machines[minor].condition.type == CONDITION_SEQ) {
        if (slot_machines[minor].condition.argument > 0) {
            winnings = slot_machines[minor].cash;
            slot_machines[minor].cash = 0; // Reset the cash
            slot_machines[minor].condition.argument = slot_machines[minor].condition.argument - 1;

        }
        else
        {
            winnings = 0;
        }
    }
    else if (slot_machines[minor].condition.type == CONDITION_PID) {
        // Wins if the process PID matches the argument
        int current_pid = current->pid;
        if (current_pid == slot_machines[minor].condition.argument) {
            winnings = slot_machines[minor].cash;
            slot_machines[minor].cash = 0; // Reset the cash
        }
        else {
            // Doesn't match the PID, so it loses
            winnings = 0;
        }
    }
    else {
        // Any other case: Default condition
        winnings = 0;
        return -EACCES;
    }


    if (copy_to_user(buf, &winnings, sizeof( int))) {
        return -EFAULT; // Error copying data to user space
    }

    return sizeof(int);
}



int my_ioctl(struct inode* inode, struct file* filp, unsigned int cmd_id, unsigned long arg){

    int minor = MINOR(filp->f_dentry->d_inode->i_rdev);
   
    switch (cmd_id) {
    case GET_CONDITION: {
        if (current->pid != slot_machines[minor].operator_pid)
        {
            return -EACCES;
        }
        // Get the current win condition
        if (copy_to_user((struct win_condition*)arg, &slot_machines[minor].condition, sizeof(struct win_condition))) {
            return -EFAULT; // Failed to copy data to user space
        }
        return 0;
    }

    case SET_CONDITION: {
        if (current->pid != slot_machines[minor].operator_pid)
        {
            return -EACCES;
        }
        // Set a new win condition
        if (copy_from_user(&slot_machines[minor].condition, (struct win_condition*)arg, sizeof(struct win_condition))) {
            return -EFAULT; // Invalid write operation
        }
        return 0;
    }

    case PRIZE_AMOUNT: {
        // Get the current cash amount in the slot machine
        if (copy_to_user((int*)arg, &slot_machines[minor].cash, sizeof(int))) {
            return -EFAULT; // Failed to copy data to user space
        }
        return 0;
    }

    default:
        return -ENOTTY; // Invalid ioctl command
    }
}

int init_module(void)
{
    // This function is called when inserting the module using insmod

    my_major = register_chrdev(my_major, MY_DEVICE, &my_fops);

    if (my_major < 0)
    {
        printk(KERN_WARNING "can't get dynamic major\n");
        return my_major;
    }


    // Allocate memory for slot machines
    slot_machines = kmalloc(sizeof(struct slot_machine) * MAX_MINOR_DEVICES, GFP_KERNEL);

    if (!slot_machines) {
        // Handle memory allocation failure
        printk(KERN_ERR "Failed to allocate memory for slot_machines.\n");
        return -ENOMEM;
    }

    int minor = 0;
    for (minor = 0; minor < MAX_MINOR_DEVICES; minor++) {
        slot_machines[minor].initialized = 0;
        slot_machines[minor].cash = 0; // Initialize other fields as needed
        slot_machines[minor].operator_pid = 0;
        slot_machines[minor].ref_count = 0;
    }

    printk("Device driver registered - called from insmod\n");
    return 0;
}


void cleanup_module(void) {
    printk("Before calling device driver unregister - called from rmmod\n");
    unregister_chrdev(my_major, MY_DEVICE);
    kfree(slot_machines);

}
