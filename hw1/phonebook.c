#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <uapi/linux/string.h>

#define DEVICE_NAME "my_numbers"
#define BUFF_SIZE 512
#define DATA_SIZE 32

static dev_t first;
static unsigned int count = 1;
static int major = 700, minor = 0;
static struct cdev *module_cdev;
static char kbuf[BUFF_SIZE] = "start\n";
struct list_head head;

void do_command(void);
static int init_mod(void);
static void cleanup_mod(void);

size_t minimum(size_t x, size_t y) {
    if (x > y) {
	    return y;
    }
    return x;
}

struct Pearson {
    char f_name[DATA_SIZE];
    char s_name[DATA_SIZE];
    int age;
    char phone[DATA_SIZE];
    char email[DATA_SIZE];
};

struct Node {
    struct list_head head;
    struct Pearson p;
};

static int device_open(struct inode* inode, struct file* file) {
    printk(KERN_INFO "Opening device\n");
    count++;
    return 0;
}

static int device_release(struct inode* inode, struct file* file) {
    printk(KERN_INFO "Releasing device\n");
    count--;
    return 0;
}

static ssize_t device_read(struct file* file, char* __user buf, size_t len, loff_t* ppos) {
    size_t data_len = strlen(kbuf);
    len = minimum(len, data_len - *ppos); 
    copy_to_user(buf, kbuf + *ppos, len);
    *ppos += len;

    printk(KERN_INFO "Read some data\n");
    return len;
}

static ssize_t device_write(struct file* file, const char* __user buf, size_t len, loff_t* ppos) {
    len = minimum(len, BUFF_SIZE - *ppos); 
    copy_from_user(kbuf + *ppos, buf, len);
    *ppos += len;

    printk(KERN_INFO "Write some data\n");
    kbuf[*ppos] = '\0';
    do_command();
    return len;
}

static const struct file_operations fops = {
    write: device_write,
    read: device_read,
    open: device_open,
    release: device_release
};

static int init_mod() {
    printk(KERN_INFO "Let's start\n");
    first = MKDEV(major, minor);
    register_chrdev_region(first, count, DEVICE_NAME);

    module_cdev = cdev_alloc();
    cdev_init(module_cdev, &fops);
    cdev_add(module_cdev, first, count);
    INIT_LIST_HEAD(&head);

    return 0;
}

static void cleanup_mod() {
    if (module_cdev){
        cdev_del(module_cdev);
    }

    unregister_chrdev_region(first, count);
}

long get_person(const char* surname, unsigned int len, struct Pearson* res) {
    struct list_head *ptr;
    struct Node *node;

    if (list_empty(&head)) {
        return -1;
    }

    list_for_each(ptr, &head) {
        node = list_entry(ptr, struct Node, head);
        if (strncmp(node->p.s_name, surname, len) == 0) {
            memcpy(res, &node->p, sizeof(struct Pearson));
            return 0;
        }
    }

    return -1;
}

long add_person(struct Pearson* curr) {
    struct Node *new_node = kmalloc(sizeof(struct Node), GFP_KERNEL);
    memcpy(&new_node->p, curr, sizeof(struct Pearson));
    list_add(&new_node->head, &head);

    return 0;
}

long del_person(const char* surname, unsigned int len) {
    struct list_head *ptr;
    struct Node *node;

    list_for_each(ptr, &head) {
        node = list_entry(ptr, struct Node, head);
        if (strncmp(node->p.s_name, surname, len) == 0) {
            list_del(&node->head);
            return 0;
        }
    }

    return -1;
}

void do_command() {
    struct Pearson p;
    char s_name[DATA_SIZE];
    memset(&p, 0, sizeof(struct Pearson));

    if (kbuf[0] == '+') {
        sscanf(kbuf + 2, "%s %s %d %s %s", p.f_name, p.s_name, &p.age, p.phone, p.email);
        add_person(&p);

    } else if (kbuf[0] == '?') {
        int len = 0;
        sscanf(kbuf + 2, "%s%n", s_name, &len);

        if (-1 == get_person(s_name, len, &p)) {
            memcpy(kbuf, "no such person\n\0", 16);
        } else {
            snprintf(kbuf, BUFF_SIZE, "name: %s\nsurname: %s\nage: %d\nphone: %s\nemail: %s\n", 
            p.f_name, p.s_name, p.age, p.phone, p.email);
        }

    } else if (kbuf[0] == '-') {
        int len = 0;
        sscanf(kbuf + 2, "%s%n", s_name, &len);
        del_person(s_name, len);

    } else {
        memcpy(kbuf, "no such command\n\0", 17);
    }
}

module_init(init_mod);
module_exit(cleanup_mod);

MODULE_LICENSE("GPL");
