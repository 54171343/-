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
//����DS18B20�˿ڿ���
#define   DS18B20_L    	gpio_set_value(DS18B20_Pin, 0)    	 	//���������ߵ�ƽ
#define   DS18B20_H    	gpio_set_value(DS18B20_Pin, 1)      		//���������ߵ�ƽ
#define   DS18B20_OUT	gpio_direction_output(DS18B20_Pin, 0)			//����������Ϊ���
#define   DS18B20_IN		gpio_direction_input(DS18B20_Pin)		//����������Ϊ����
#define   DS18B20_STU	gpio_get_value(DS18B20_Pin)       	//����״̬

typedef struct
{
	unsigned char  humi_high8bit;		//ԭʼ���ݣ�ʪ�ȸ�8λ
	unsigned char  humi_low8bit;	 	//ԭʼ���ݣ�ʪ�ȵ�8λ
	unsigned char  temp_high8bit;	 	//ԭʼ���ݣ��¶ȸ�8λ
	unsigned char  temp_low8bit;	 	//ԭʼ���ݣ��¶ȵ�8λ
	unsigned char  check_sum;	 	    //У���
  	int    humidity;        //ʵ��ʪ��
  	int    temperature;     //ʵ���¶�  
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
	//��ʼ�ź�
	DS18B20_L;
	//20ms
	mdelay(20);
	DS18B20_H;
	udelay(30);
	DHT11_Init(READ);
	//�ȴ��͵�ƽ
	while(DS18B20_STU);
	//83us�͵�ƽ�������ȴ��ߵ�ƽ
	while(!DS18B20_STU);
	for(i=0;i<5;i++){
		for(j=0;j<8;j++){
			s=0;
			//�ȵȴ��͵�ƽ����
			while(DS18B20_STU && s<100)
			{
				s++;
				udelay(1);
			}
			//printk("s = %d\n",s);
			s = 0;
			//54us�͵�ƽ����,
			//�ȴ��ߵ�ƽ����
			while(!DS18B20_STU && s<100)
			{
				s++;
				udelay(1);
			}
			//printk("s = %d\n",s);
			//��ʱ27us����,�ж�����λ,0��1
			udelay(40);
			//�Ȱ���������1λ
			data[i] = data[i] << 1;
			//�ж�40us���Ƿ�Ϊ�ߵ�ƽ,������Ϊ1,��Ϊ0
			if(DS18B20_STU)
				data[i] |= 0x01;
			//һλ���ݽ������
			//�ȴ��ߵ�ƽ����,��һ�����ݵ͵�ƽ�ĵ���
		}
	}
	//�ж�У��λ
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
	copy_to_user(buf, &DHT11_Data, sizeof(DHT11_Data_TypeDef));    //����ȡ�õ�DS18B20��ֵ���Ƶ��û���

	return 0;
}

static struct file_operations sencod_drv_fops = {
	.owner		= THIS_MODULE,    /* ����һ���꣬�������ģ��ʱ�Զ�������__this_module���� */
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

