#include <linux/init.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <linux/module.h>
#include <linux/interrupt.h>

#define INTERVAL 60
#define KEYBOARD_IRQ 1
static int res = 0, irq = KEYBOARD_IRQ, dev_id, end = 0;
struct timer_list print_res;

static irqreturn_t kb_handler(int irq, void* dev) {
    res++;
    return IRQ_NONE;
}

void show_res(struct timer_list *timer) {
    printk(KERN_INFO "current amount of typings = %d\n", res);

    if (end == 0) {
        mod_timer(&print_res, jiffies + INTERVAL * HZ);
    }
}

static int init_mod(void) {
    if (request_irq(irq, kb_handler, IRQF_SHARED, "keyboard", &dev_id)) {
        return -1;
    }
    timer_setup(&print_res, show_res, 0);
    mod_timer(&print_res, jiffies + INTERVAL * HZ);
    return 0;
}

static void cleanup_mod(void) {
    synchronize_irq(irq);
    free_irq(irq, &dev_id);
    end = 1;
    del_timer_sync(&print_res);
}

module_init(init_mod);
module_exit(cleanup_mod);
MODULE_LICENSE("GPL");