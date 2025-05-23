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
static struct task_struct *ws2812_task;
static int duty_cycle = 50;

uint32_t *PERIBase;
uint32_t *GPIOBase;
uint32_t *RIOBase;
uint32_t *PADBase;
uint32_t *pad;

void send_pulse_1(void);
void send_pulse_0(void);

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

//ws2812
static const uint32_t WS2812_GPIO = 13;

void send_pulse_1(void){
	SET(WS2812_GPIO);
	ndelay(900); //580~1600
	CLR(WS2812_GPIO);
	ndelay(350); //220~420
  }
void send_pulse_0(void){
	SET(WS2812_GPIO);
	ndelay(300); //220~380 
	CLR(WS2812_GPIO); 
	ndelay(1100); //580~1600
}
void send_bit(uint8_t bit){
	if(bit){
  	send_pulse_1(); 
	}else{
  	send_pulse_0();
  }
}
void send_bits(const uint8_t *bits ,size_t len){
	for(size_t i=0;i<len;i++){
   	send_bit(bits[i]); 
	}
}
const uint8_t command_F_bits[]={
   1,1,1,1,1,1,1,1,
   1,1,1,1,1,1,1,1,
   1,1,1,1,1,1,1,1
};
const uint8_t command_B_bits[]={
  0,0,0,0,0,0,0,0,
  1,1,1,1,1,1,1,1,
  0,0,0,0,0,0,0,0
};
const uint8_t command_D_bits[]={
  1,0,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,
  0,0,0,0,0,0,0,0
};
const uint8_t command_S_bits[]={
  0,0,0,0,0,0,0,0,
  0,0,0,0,1,1,1,1,
  0,0,0,0,0,0,0,0
};
const uint8_t command_C_bits[]={
  0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0
};

void send_command_F(void){
  send_bits(command_F_bits,sizeof(command_F_bits));
}
void send_command_B(void){
  send_bits(command_B_bits,sizeof(command_B_bits));
}
void send_command_D(void){
  send_bits(command_D_bits,sizeof(command_D_bits));
}
void send_command_S(void){
  send_bits(command_S_bits,sizeof(command_S_bits));
}
void send_command_C(void){
  send_bits(command_C_bits,sizeof(command_C_bits));
}





static void stop_all(void) {
    CLR(A_IN1); CLR(A_IN2);
    CLR(B_IN1); CLR(B_IN2);
}

static void init_gpio(uint32_t pin, uint32_t fn) {
    GPIO[pin].ctrl = fn;
    pad[pin] = 0x0f;
    OUT(pin);         
}

//ws2812 To display which mode
static int ws2812_mode = 0;

static void handle_cmd(char cmd) {
    stop_all();
    duty_cycle = 70; 

    switch (cmd) {
        case CMD_FORWARD:
            SET(A_IN1); CLR(A_IN2);
            SET(B_IN1); CLR(B_IN2);
            duty_cycle = 100;
            ws2812_mode = 1;
            break;
        case CMD_BACKWARD:
            CLR(A_IN1); SET(A_IN2);
            CLR(B_IN1); SET(B_IN2);
            duty_cycle = 70;
            ws2812_mode = 2; 
            break;
        case CMD_LEFT:
            CLR(A_IN1); SET(A_IN2);
            SET(B_IN1); CLR(B_IN2);
            duty_cycle = 100;
            ws2812_mode = 3;
            break;
        case CMD_RIGHT:
            SET(A_IN1); CLR(A_IN2);
            CLR(B_IN1); SET(B_IN2);
            duty_cycle = 100;
            ws2812_mode = 4;
            break;
        case CMD_STOP:
			stop_all();
			duty_cycle = 70;
			ws2812_mode =5;
			break; 
        default:
            duty_cycle = 70;
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

//ws2812
static int ws2812_mode_last = 0; 

static int ws2812_thread(void *data){
	while(!kthread_should_stop()){
		switch (ws2812_mode){
				case 1:
					if(ws2812_mode_last!=1){
						for(int i=0;i<8;i++){
						send_command_S();}
						udelay(350);
						for(int i=0;i<8;i++){
						send_command_S();}
						udelay(350);
						ws2812_mode_last=1;
					}
					udelay(500);
					break;
				case 2:
					if(ws2812_mode_last!=2){
						for(int i=0;i<2;i++){
						udelay(350);
						for(int i=0;i<8;i++){
						send_command_B();}
						udelay(350);
						ws2812_mode_last=2;
						}
					}
					udelay(500);
					break;
				case 3:
					udelay(350);
					for(int i=0;i<4;i++){
					send_command_D();}
					for(int i=0;i<4;i++){	
					send_command_S();}
					udelay(350);
					for(int i=0;i<4;i++){
					send_command_D();}
					for(int i=0;i<4;i++){	
					send_command_S();}
					udelay(350);
					msleep(500);
					for(int i=0;i<8;i++){
						send_command_S();}
					udelay(350);
					for(int i=0;i<8;i++){
						send_command_S();}
					udelay(350);
					msleep(500);
					if(ws2812_mode_last!=3){
						ws2812_mode_last=3;
					}
					break;
				case 4:
					udelay(350);
					for(int i=0;i<4;i++){
					send_command_S();}
					for(int i=0;i<4;i++){
					send_command_D();}
					udelay(350);
					for(int i=0;i<4;i++){
					send_command_S();}
					for(int i=0;i<4;i++){
					send_command_D();}
					udelay(350);
					msleep(500);
					for(int i=0;i<8;i++){
						send_command_S();}
					udelay(350);
					for(int i=0;i<8;i++){
						send_command_S();}
					udelay(350);
					msleep(500);
					if(ws2812_mode_last!=4){
						ws2812_mode_last=4;
					}
					break;
				case 5:				
					if(ws2812_mode_last!=5){
						udelay(350);
						if(ws2812_mode_last==2){
							for(int i=0;i<8;i++){
							send_command_S();}
							udelay(350);
						}else{
						for(int i=0;i<8;i++){
							send_command_B();}
						udelay(350);
						msleep(250);
						for(int i=0;i<8;i++){
							send_command_S();}
						udelay(350);
						msleep(250);
						for(int i=0;i<8;i++){
							send_command_B();}
						udelay(350);
						msleep(250);
						for(int i=0;i<8;i++){
						send_command_S();}
						udelay(350);
						
						for(int i=0;i<8;i++){
						send_command_S();}
						udelay(350);
						}
						ws2812_mode_last=5;
					}
					udelay(500);
					break;
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
	init_gpio(WS2812_GPIO, fn);

    pwm_task = kthread_run(pwm_thread, NULL, "pwm_thread");
    ws2812_task = kthread_run(ws2812_thread, NULL, "ws2812_thread");

    printk("motor_ctrl with software PWM loaded\n");
    return 0;
}

static void __exit motor_exit(void) {
    if (pwm_task) kthread_stop(pwm_task);
    if (ws2812_task) kthread_stop(ws2812_task);
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

