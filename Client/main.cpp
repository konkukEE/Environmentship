#pragma comment(lib,"ws2_32")

#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>		
#include <sys/types.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <time.h>

#include <stdint.h>
#include <errno.h>
#include <wiringPi.h>
#include <wiringSerial.h>

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <fstream>

#include "stopwatch.h"
#include "network.h"
#include "video.h"

#define PORT 4578
#define PACKET_SIZE 1024

int BLOBSIZE(std::string weight);
int RECVKEY(char key[2], int socket);
void SENDMAT(Mat image, int socket);
void MOVE(char key[2]);

/*
	 <coco dataset>
	 name
	 "../data/name/coco.names"

	 weight
	 "../data/weight/coco_v5n_320.onnx"
	 "../data/weight/coco_v5n_416.onnx"
	 "../data/weight/coco_v5n_640.onnx"

	 "../data/weight/coco_v5s_320.onnx"
	 "../data/weight/coco_v5s_416.onnx"
	 "../data/weight/coco_v5s_640.onnx"

	 "../data/weight/coco_v5m_320.onnx"
	 "../data/weight/coco_v5m_416.onnx"
	 "../data/weight/coco_v5m_640.onnx"

	 "../data/weight/coco_v5l_320.onnx"
	 "../data/weight/coco_v5l_416.onnx"
	 "../data/weight/coco_v5l_640.onnx"

	 "../data/weight/coco_v5x_320.onnx"
	 "../data/weight/coco_v5x_416.onnx"
	 "../data/weight/coco_v5x_640.onnx"


	 <chess dataset>
	 name
	 "../data/name/chess.names"

	 weight
	 "chess_v5n_320.onnx"
	 "chess_v5n_416.onnx"
	 "chess_v5s_416.onnx"
*/

int main()
{
	char SERVER_IP[30];
	printf("IP: ");
	scanf("%s", SERVER_IP);
	//char SERVER_IP[] = "192.168.200.191";
	std::string name = "../data/name/coco.names";
	std::string videofile = "../data/video/ship.mp4";
	std::string imagefile = "../data/image/chess.jpg";
	std::string weight = "../data/weight/coco_v5n_320.onnx";

	int clnt_sock;
	struct sockaddr_in serv_addr;
	char buffer[PACKET_SIZE];
	char message[] = "Client Send";
	clnt_sock = socket(PF_INET, SOCK_STREAM, 0);
	if (clnt_sock == -1)
		printf("socket error\n");
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
	serv_addr.sin_port = htons(PORT);
	if (connect(clnt_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
		printf("connect error\n");
	if (write(clnt_sock, message, sizeof(message)) == -1)
		printf("write error\n");
	if (read(clnt_sock, buffer, PACKET_SIZE) == -1)
		printf("read error\n");
	printf("Message from server :%s\n", buffer);


	//
	printf("%s \n", "Raspberry Startup!");
	fflush(stdout);
	if ((fd = serialOpen(device, baud)) < 0)
	{
		fprintf(stderr, "Unable to open serial device: %s\n", strerror(errno));
		exit(1);
	}
	if (wiringPiSetup() == -1)
	{
		fprintf(stdout, "Unable to start wiringPi: %s\n", strerror(errno));
		exit(1);
	}
	//
	char key[2];
	Mat image;
	VideoCapture capture;
	capture.open(0);
	if (!capture.isOpened())
	{
		std::cout << "Can't open the video" << std::endl;
		return -1;
	}
	
	Netinf ClientNet = NetworkSetting(weight, name, BLOBSIZE(weight));
	while (1)
	{
		capture >> image;
		SENDMAT(image, clnt_sock);
		vshow(image, ClientNet);

		if (RECVKEY(key, clnt_sock))
		{
			MOVE(key);
			break;
		}

		MOVE(key);
	}
	close(clnt_sock);

	return 0;
}
int BLOBSIZE(std::string weight)
{
	int iter = 0;
	int tmp = 0;
	int check = 0;
	int result = 0;

	while (1)
	{
		if (check)
		{
			if (check == 1)
				result += (weight[iter] - '0') * 100;
			else if (check == 2)
				result += (weight[iter] - '0') * 10;
			else if (check == 3)
				result += (weight[iter] - '0');
			else if (check == 4)
				break;

			check++;
		}

		if (weight[iter] == '_')
			if (++tmp == 2)
				check = 1;

		iter++;
	}

	return result;
}
int RECVKEY(char key[2], int socket)
{
	key[0] = key[1] = 'q';
	read(socket, key, 2);

	if (key[0] == 'q')
		return 1;
	else
	{
		if (key[0] != 'n' || key[1] != 'n')
			printf("\n");

		if (key[0] == 'w' || key[0] == 's')
			printf("%c ", key[0]);

		if (key[1] == 'a' || key[1] == 'd')
			printf("%c", key[1]);

		return 0;
	}
}
void SENDMAT(Mat image, int socket)
{
	char rows[4];
	char cols[4];
	int size = image.rows * image.cols * image.channels();
	strcpy(rows, std::to_string(image.rows).c_str());
	strcpy(cols, std::to_string(image.cols).c_str());
	char* buffer = new char[size];

	for (int i = 0; i < 4 - std::to_string(image.rows).size(); i++)
		rows[3 - i] = -1;
	for (int i = 0; i < 4 - std::to_string(image.cols).size(); i++)
		cols[3 - i] = -1;
	memcpy(buffer, image.data, size);


	//write(clnt_sock, msg, sizeof(msg));
	write(socket, rows, 4);
	write(socket, cols, 4);
	write(socket, buffer, size);

	delete[]buffer;
}
void MOVE(char key[2])
{
	if (key[0] == 'q')
	{
		serialPutchar(fd, 113);
	}

	if(key[0]=='w')
	{
		serialPutchar(fd, 119);
	}
	else if(key[0]=='s')
	{
		serialPutchar(fd, 115);
	}

	if (key[1] = 'a')
	{
		serialPutchar(fd, 97);
	}
	else if (key[1] = 'd')
	{
		serialPutchar(fd, 100);
	}
}
