#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/ioctl.h>

#define SERVER_PORT 8080
#define BACKLOG     10


#define TRUE    1
#define FALSE   0

int fd;
int temp_fd;
typedef struct
{
		unsigned char  humi_high8bit;		//原始数据：湿度高8位
		unsigned char  humi_low8bit;	 	//原始数据：湿度低8位
		unsigned char  temp_high8bit;	 	//原始数据：温度高8位
		unsigned char  temp_low8bit;	 	//原始数据：温度低8位
		unsigned char  check_sum;	 	    //校验和
		int    humidity;        //实际湿度
		int    temperature;     //实际温度
		//long  flag; 
} DHT11_Data_TypeDef;

static void get_temperature(int *SocketClient)
{
	int cishu = 2;
	char ucSendBuf[100] = {0};
	int iSendLen;
	DHT11_Data_TypeDef DHT11_Data;

	while(cishu--){
	read(temp_fd, &DHT11_Data , sizeof(DHT11_Data_TypeDef));

	printf("Humidity = %d, Temperature = %d\n", DHT11_Data.humidity, DHT11_Data.temperature);
	sleep(2);
	sprintf(ucSendBuf, "Humidity = %d, Temperature = %d\n", DHT11_Data.humidity, DHT11_Data.temperature);

	iSendLen = send(*SocketClient, ucSendBuf, strlen(ucSendBuf), 0);
	if (iSendLen <= 0)
	{
		printf("send temperature error\n");
		close(*SocketClient);
		return;
	}
	}
}

int main(int argc, char **argv)
{
	pthread_t Duty_Cycle_Id;
	int result;

	int iSocketServer;
	int iSocketClient;
	struct sockaddr_in tSocketServerAddr;
	struct sockaddr_in tSocketClientAddr;
	int iRet;
	int iAddrLen;
	
	int iRecvLen;
	unsigned char ucRecvBuf[100],sendbuf[50];

	int iClientNum = -1;

	
	signal(SIGCHLD,SIG_IGN);
	
	iSocketServer = socket(AF_INET, SOCK_STREAM, 0);
	if (-1 == iSocketServer)
	{
		printf("socket error!\n");
		return -1;
	}

	tSocketServerAddr.sin_family      = AF_INET;
	tSocketServerAddr.sin_port        = htons(SERVER_PORT);  /* host to net, short */
 	tSocketServerAddr.sin_addr.s_addr = INADDR_ANY;
	memset(tSocketServerAddr.sin_zero, 0, 8);
	
	iRet = bind(iSocketServer, (const struct sockaddr *)&tSocketServerAddr, sizeof(struct sockaddr));
	if (-1 == iRet)
	{
		printf("bind error!\n");
		return -1;
	}

	iRet = listen(iSocketServer, BACKLOG);
	if (-1 == iRet)
	{
		printf("listen error!\n");
		return -1;
	}


	temp_fd = open("/dev/ds18b20", O_RDWR);
	if (fd < 0)
	{
		printf("error, can't open /dev/motor\n");
		return 0;
	}

	while (1)
	{
		iAddrLen = sizeof(struct sockaddr);
		strcpy(sendbuf,"temp");

		iSocketClient = accept(iSocketServer, (struct sockaddr *)&tSocketClientAddr, &iAddrLen);
		if (-1 != iSocketClient)
		{
			iClientNum++;
			printf("Get connect from client %d : %s\n",  iClientNum, inet_ntoa(tSocketClientAddr.sin_addr));
			if (!fork())
			{

				while (1)
				{
					get_temperature(&iSocketClient);
				}		
			}
		}
	}

	close(iSocketServer);
	return 0;
}

