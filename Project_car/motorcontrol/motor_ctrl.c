#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/kthread.h>
#include <linux/delay.h>

#define DEVICE_NAME "motor_ctrl"
#define CLASS_NAME  "motor"

#define CMD_FORWARD  'F'
#define CMD_BACKWARD 'B'
#define CMD_LEFT     'L'
#define CMD_RIGHT    'R'
#define CMD_STOP     'S'

#define GPIO_OFFSET  0xD0000
#define RIO_OFFSET   0xE0000
#define PAD_OFFSET   0xF0000
#define MAP_SIZE     (64 * 1024 * 1024)

static dev_t dev_num;
static struct class *motor_class;
static struct cdev motor_cdev;
static struct task_struct *pwm_task;
static int duty_cycle = 50;

uint32_t *PERIBase;
uint32_t *GPIOBase;
uint32_t *RIOBase;
uint32_t *PADBase;
uint32_t *pad;

typedef struct {
    uint32_t status;
    uint32_t ctrl;
} GPIOregs;

typedef struct {
    uint32_t Out;
    uint32_t OE;
} rioregs;

#define GPIO ((volatile GPIOregs *) GPIOBase)
#define rioSET ((volatile rioregs *) (RIOBase + 0x2000 / 4))
#define rioCLR ((volatile rioregs *) (RIOBase + 0x3000 / 4))

#define SET(pin) rioSET->Out = (1 << (pin))
#define CLR(pin) rioCLR->Out = (1 << (pin))
#define OUT(pin) rioSET->OE = (1 << (pin))

static const uint32_t A_IN1 = 23, A_IN2 = 24, A_ENA = 25;
static const uint32_t B_IN1 = 26, B_IN2 = 27, B_ENB = 22;

static void stop_all(void) {
    CLR(A_IN1); CLR(A_IN2);
    CLR(B_IN1); CLR(B_IN2);
}

static void init_gpio(uint32_t pin, uint32_t fn) {
    GPIO[pin].ctrl = fn;
    pad[pin] = 0x0f;
    OUT(pin);         
}

static void handle_cmd(char cmd) {
    stop_all();
    duty_cycle = 50; 

    switch (cmd) {
        case CMD_FORWARD:
            SET(A_IN1); CLR(A_IN2);
            SET(B_IN1); CLR(B_IN2);
            duty_cycle = 100;
            break;
        case CMD_BACKWARD:
            CLR(A_IN1); SET(A_IN2);
            CLR(B_IN1); SET(B_IN2);
            duty_cycle = 50;
            break;
        case CMD_LEFT:
            CLR(A_IN1); SET(A_IN2);
            SET(B_IN1); CLR(B_IN2);
            duty_cycle = 80;
            break;
        case CMD_RIGHT:
            SET(A_IN1); CLR(A_IN2);
            CLR(B_IN1); SET(B_IN2);
            duty_cycle = 80;
            break;
        case CMD_STOP:
						stop_all();
						duty_cycle = 50;
						break; 
        default:
            duty_cycle = 50;
            break;
    }
}

static ssize_t motor_write(struct file *file, const char __user *buf, size_t len, loff_t *off) {
    char cmd;
    if (len < 1) return -EINVAL;
    if (copy_from_user(&cmd, buf, 1)) return -EFAULT;

    handle_cmd(cmd);
    return len;
}

static int pwm_thread(void *data) {
    while (!kthread_should_stop()) {
        if (duty_cycle > 0) {
            SET(A_ENA);
            SET(B_ENB);
            usleep_range(duty_cycle * 100, duty_cycle * 100 + 10);
            CLR(A_ENA);
            CLR(B_ENB);
            usleep_range((100 - duty_cycle) * 100, (100 - duty_cycle) * 100 + 10);
        } else {
            CLR(A_ENA);
            CLR(B_ENB);
            msleep(10);
        }
    }
    return 0;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .write = motor_write,
};

static int __init motor_init(void) {
    PERIBase = ioremap(0x1f00000000, MAP_SIZE);
    if (!PERIBase) {
        printk("ioremap failed\n");
        return -ENOMEM;
    }

    GPIOBase = PERIBase + GPIO_OFFSET / 4;
    RIOBase  = PERIBase + RIO_OFFSET  / 4;
    PADBase  = PERIBase + PAD_OFFSET  / 4;
    pad      = PADBase + 1;

    if (alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME) < 0) {
        printk("alloc_chrdev_region failed\n");
        return -1;
    }

    cdev_init(&motor_cdev, &fops);
    if (cdev_add(&motor_cdev, dev_num, 1) < 0) {
        printk("cdev_add failed\n");
        return -1;
    }

    motor_class = class_create(CLASS_NAME);
    if (IS_ERR(motor_class)) {
        printk("class_create failed\n");
        return PTR_ERR(motor_class);
    }

    device_create(motor_class, NULL, dev_num, NULL, DEVICE_NAME);

    uint32_t fn = 5;
    init_gpio(A_IN1, fn); init_gpio(A_IN2, fn); init_gpio(A_ENA, fn);
    init_gpio(B_IN1, fn); init_gpio(B_IN2, fn); init_gpio(B_ENB, fn);

    pwm_task = kthread_run(pwm_thread, NULL, "pwm_thread");

    printk("motor_ctrl with software PWM loaded\n");
    return 0;
}

static void __exit motor_exit(void) {
    if (pwm_task) kthread_stop(pwm_task);
    stop_all();
    CLR(A_ENA); CLR(B_ENB);
    device_destroy(motor_class, dev_num);
    class_destroy(motor_class);
    cdev_del(&motor_cdev);
    unregister_chrdev_region(dev_num, 1);
    iounmap(PERIBase);
    printk("motor_ctrl module unloaded\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("H");
MODULE_DESCRIPTION("motor driver");
module_init(motor_init);
module_exit(motor_exit);

