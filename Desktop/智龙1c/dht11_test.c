#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <signal.h>

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

int main(int argc, char **argv)
{
	int fd;
	int cnt = 2;
	int val = 1;
	DHT11_Data_TypeDef DHT11_Data;
	
	fd = open("/dev/ds18b20", O_RDWR);
	if (fd < 0)
	{
		printf("can't open\n");
		return -1;
	}
	
	while(cnt--){	
		val = read(fd, &DHT11_Data, sizeof(DHT11_Data_TypeDef));
		if(val < 0)
		{
			printf("read failed\r\n");
			continue;
		}
		else{
			sleep(1);
			printf("Humidity = %d, Temperature = %d\n", DHT11_Data.humidity, DHT11_Data.temperature);
		}
	}

	close(fd);
	return 0;
}


