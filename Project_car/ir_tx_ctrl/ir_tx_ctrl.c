#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/delay.h>

#define DEVICE_NAME "ir_tx_ctrl"
#define CLASS_NAME  "ir_tx"

#define GPIO_OFFSET  0xD0000
#define RIO_OFFSET   0xE0000
#define PAD_OFFSET   0xF0000
#define MAP_SIZE     (64 * 1024 * 1024)

static dev_t dev_num;
static struct class *ir_tx_class;
static struct cdev ir_tx_cdev;

uint32_t *PERIBase;
uint32_t *GPIOBase;
uint32_t *RIOBase;
uint32_t *PADBase;
uint32_t *pad;


void send_leader_code(void);
void send_pulse_1(void);
void send_pulse_0(void);
void send_bit(uint8_t bit);
void send_bits(const uint8_t *bits, size_t len); 
void send_address(void);
void send_command_F(void);
void send_command_B(void);
void send_command_L(void);
void send_command_R(void);
void send_command_S(void);

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

static const uint32_t IR_TX_GPIO = 17;

static void init_gpio(uint32_t pin, uint32_t fn) {
    GPIO[pin].ctrl = fn;
    pad[pin] = 0x0f; 
    OUT(pin);       
}

//NEC
void send_leader_code(void){
	SET(IR_TX_GPIO);
	udelay(9000);
	CLR(IR_TX_GPIO);
	udelay(4500);
}
void send_pulse_1(void){
	SET(IR_TX_GPIO);
	udelay(560);
	CLR(IR_TX_GPIO);
	udelay(1690);
}
void send_pulse_0(void){
	SET(IR_TX_GPIO);
	udelay(560);
	CLR(IR_TX_GPIO);
	udelay(560);
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

const uint8_t address_bits[]={
	1,1,0,0,1,1,0,0,
	0,0,1,1,0,0,1,1
};

const uint8_t command_F_bits[]={
	1,1,1,1,0,0,0,0,
	0,0,0,0,1,1,1,1,
	1
};
const uint8_t command_B_bits[]={
	0,0,0,0,1,1,1,1,
	1,1,1,1,0,0,0,0,
	1
};
const uint8_t command_L_bits[]={
	1,1,0,0,0,0,1,1,
	0,0,1,1,1,1,0,0,
	1
};
const uint8_t command_R_bits[]={
	0,0,1,1,1,1,0,0,
	1,1,0,0,0,0,1,1,
	1
};
const uint8_t command_S_bits[]={
	0,1,0,1,0,1,0,1,
	1,0,1,0,1,0,1,0,
	1
};

void send_address(void){
	send_bits(address_bits,sizeof(address_bits));
}

void send_command_F(void){
	send_bits(command_F_bits,sizeof(command_F_bits));
}

void send_command_B(void){
	send_bits(command_B_bits,sizeof(command_B_bits));
}

void send_command_L(void){
	send_bits(command_L_bits,sizeof(command_L_bits));
}

void send_command_R(void){
	send_bits(command_R_bits,sizeof(command_R_bits));
}

void send_command_S(void){
	send_bits(command_S_bits,sizeof(command_S_bits));
}


static void handle_cmd(char cmd) {
    switch (cmd) {
		    case 'F':
						//for(int i=0;i<3;i++){
							send_leader_code();
							send_address();
							send_command_F();
						//	if(i!=2){
								msleep(110);
						//	}
						//}
            break;
        case 'B':
						//for(int i=0;i<3;i++){
							send_leader_code();
							send_address();
							send_command_B();
						//	if(i!=2){
								msleep(110);
						//	}
						//}
						break;
        case 'L':
						//for(int i=0;i<3;i++){
							send_leader_code();
							send_address();
							send_command_L();
						//	if(i!=2){
								msleep(110);
						//	}
						//}
            break;
        case 'R':
						//for(int i=0;i<3;i++){
							send_leader_code();
							send_address();
							send_command_R();
						//	if(i!=2){
							msleep(110);
						//	}
						//}
            break;
        case 'S':
							send_leader_code();
							send_address();
							send_command_S();
							msleep(110);
						break;
        default:
            CLR(IR_TX_GPIO);
            break;
    }
}

static ssize_t ir_tx_write(struct file *file, const char __user *buf, size_t len, loff_t *off) {
    char cmd;
    if (len < 1) return -EINVAL;
    if (copy_from_user(&cmd, buf, 1)) return -EFAULT;

    handle_cmd(cmd);
    return len;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .write = ir_tx_write,
};

static int __init ir_tx_init(void) {
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

    cdev_init(&ir_tx_cdev, &fops);
    if (cdev_add(&ir_tx_cdev, dev_num, 1) < 0) {
        printk("cdev_add failed\n");
        return -1;
    }

    ir_tx_class = class_create(CLASS_NAME);
    if (IS_ERR(ir_tx_class)) {
        printk("class_create failed\n");
        return PTR_ERR(ir_tx_class);
    }

    device_create(ir_tx_class, NULL, dev_num, NULL, DEVICE_NAME);

    uint32_t fn = 5;
    init_gpio(IR_TX_GPIO, fn);

    printk("ir_tx_ctrl module loaded\n");
    return 0;
}

static void __exit ir_tx_exit(void) {
    CLR(IR_TX_GPIO);
    device_destroy(ir_tx_class, dev_num);
    class_destroy(ir_tx_class);
    cdev_del(&ir_tx_cdev);
    unregister_chrdev_region(dev_num, 1);
    iounmap(PERIBase);
    printk("ir_tx_ctrl module unloaded\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("H");
MODULE_DESCRIPTION("ir_tx");

module_init(ir_tx_init);
module_exit(ir_tx_exit);

