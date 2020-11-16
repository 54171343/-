#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/device.h>

#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>

#include <platform.h>
#include <loongson1.h>
#include <linux/gpio.h>

static struct class *class;
static struct device *device;

#define DS18B20_Pin 60
//定义DS18B20端口控制
#define   DS18B20_L    	gpio_set_value(DS18B20_Pin, 0)    	 	//拉低数据线电平
#define   DS18B20_H    	gpio_set_value(DS18B20_Pin, 1)      		//拉高数据线电平
#define   DS18B20_OUT	gpio_direction_output(DS18B20_Pin, 0)			//数据线设置为输出
#define   DS18B20_IN		gpio_direction_input(DS18B20_Pin)		//数据线设置为输入
#define   DS18B20_STU	gpio_get_value(DS18B20_Pin)       	//数据状态

typedef struct
{
	unsigned char  humi_high8bit;		//原始数据：湿度高8位
	unsigned char  humi_low8bit;	 	//原始数据：湿度低8位
	unsigned char  temp_high8bit;	 	//原始数据：温度高8位
	unsigned char  temp_low8bit;	 	//原始数据：温度低8位
	unsigned char  check_sum;	 	    //校验和
  	int    humidity;        //实际湿度
  	int    temperature;     //实际温度  
} DHT11_Data_TypeDef;


#define READ 0
#define SEND 1

static void DHT11_Init(unsigned char select)
{
	if(select == READ){
		DS18B20_IN;
	}else if(select == SEND){
		DS18B20_OUT;
	}
}

void DHT11_Get(DHT11_Data_TypeDef *DHT11_Data)
{
	unsigned char data[5] = {0};
	unsigned char i,j;
	int s=0;	
	DHT11_Init(SEND);
	DS18B20_H;
	//1s
	mdelay(3000);
	//起始信号
	DS18B20_L;
	//20ms
	mdelay(20);
	DS18B20_H;
	udelay(30);
	DHT11_Init(READ);
	//等待低电平
	while(DS18B20_STU);
	//83us低电平到来，等待高电平
	while(!DS18B20_STU);
	for(i=0;i<5;i++){
		for(j=0;j<8;j++){
			s=0;
			//先等待低电平到来
			while(DS18B20_STU && s<100)
			{
				s++;
				udelay(1);
			}
			//printk("s = %d\n",s);
			s = 0;
			//54us低电平到来,
			//等待高电平到来
			while(!DS18B20_STU && s<100)
			{
				s++;
				udelay(1);
			}
			//printk("s = %d\n",s);
			//延时27us以上,判断数据位,0或1
			udelay(40);
			//先把数据左移1位
			data[i] = data[i] << 1;
			//判断40us后是否还为高电平,是数据为1,否为0
			if(DS18B20_STU)
				data[i] |= 0x01;
			//一位数据接收完毕
			//等待高电平结束,下一个数据低电平的到来
		}
	}
	//判断校验位
	if(data[4] == (data[0]+data[1]+data[2]+data[3])){
		DHT11_Data->humidity = data[0];
		DHT11_Data->temperature = data[2];
	//	printk("kernel Humidity = %d,kernel Temperature = %d\n", DHT11_Data->humidity, DHT11_Data->temperature);
	}
	else{
		printk("check failed\r\n");
	}
}


static int dht1_control_open(struct inode *inode, struct file *file)
{
	//DHT11_Data_TypeDef DHT11_Data;
	DS18B20_OUT;
	//DHT11_Get(&DHT11_Data);
	return 0;
}

static ssize_t dht1_control_read(struct file *file, char __user *buf, size_t size, loff_t *ppos)
{
	//unsigned char temp;
	DHT11_Data_TypeDef DHT11_Data = {0};
	
	DHT11_Get(&DHT11_Data);
	//mdelay(2000);
	copy_to_user(buf, &DHT11_Data, sizeof(DHT11_Data_TypeDef));    //将读取得的DS18B20数值复制到用户区

	return 0;
}

static struct file_operations sencod_drv_fops = {
	.owner		= THIS_MODULE,    /* 这是一个宏，推向编译模块时自动创建的__this_module变量 */
	.open		= dht1_control_open,
	.read		= dht1_control_read,
};

static int major;
static int dht1_control_init(void)
{
	major = register_chrdev(0, "dht1_control", &sencod_drv_fops);

	class = class_create(THIS_MODULE, "dht1_control");
	device = device_create(class, NULL, MKDEV(major, 0), NULL, "ds18b20"); /* /dev/ds18b20 */

	return 0;
}

static void dht1_control_exit(void)
{
	unregister_chrdev(major, "dht1_control");
	device_unregister(device);
	class_destroy(class);
}

module_init(dht1_control_init);
module_exit(dht1_control_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("JinXinHao");

