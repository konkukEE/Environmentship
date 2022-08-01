#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <iostream>
#include <string>

#include <WinSock2.h>
#include <fstream>
#pragma comment(lib,"ws2_32")
#define PORT 4578
#define PACKET_SIZE 1024

#include "stopwatch.h"
#include "network.h"
#include "video.h"


int SENDKEY(SOCKET hClient);
int RECVKEY(SOCKET hSocket, char key[2]);
void SENDMAT(Mat image, SOCKET hClient);
Mat RECVMAT(SOCKET hSocket);

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
int BLOBSIZE(std::string weight);
int main()
{
	std::string name = "../data/name/coco.names";
	std::string videofile = "../data/video/ship.mp4";
	std::string imagefile = "../data/image/chess.jpg";
	std::string weight = "../data/weight/coco_v5n_320.onnx";

	Netinf ServerNet = NetworkSetting(weight, name, BLOBSIZE(weight));
	VideoCapture capture;
	vread(capture, 0);
	vshow(capture, ServerNet);


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
int SENDKEY(SOCKET hSocket)
{
	char key[2] = { 'n','n' };

	if (GetAsyncKeyState('Q') & 0x8000)
	{
		key[0] = key[1] = 'q';

		send(hSocket, key, 2, 0);
		return 1;
	}
	else
	{
		if (GetAsyncKeyState('W') & 0x8000)
			key[0] = 'w';
		else if (GetAsyncKeyState('S') & 0x8000)
			key[0] = 's';

		if (GetAsyncKeyState('A') & 0x8000)
			key[1] = 'a';
		else if (GetAsyncKeyState('D') & 0x8000)
			key[1] = 'd';


		if (key[0] != 'n' || key[1] != 'n')
		{
			printf("%c ", key[0]);
			printf("%c\n", key[1]);
		}
		send(hSocket, key, 2, 0);
		return 0;
	}
}
int RECVKEY(SOCKET hSocket, char key[2])
{
	key[0] = key[1] = 'q';
	recv(hSocket, key, 2, 0);

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
void SENDMAT(Mat image, SOCKET hSocket)
{
	char rows[4];
	char cols[4];
	int size = image.rows * image.cols * image.channels();
	strcpy_s(rows, std::to_string(image.rows).c_str());
	strcpy_s(cols, std::to_string(image.cols).c_str());
	char* buffer = new char[size];

	for (int i = 0; i < 4 - std::to_string(image.rows).size(); i++)
		rows[3 - i] = -1;
	for (int i = 0; i < 4 - std::to_string(image.cols).size(); i++)
		cols[3 - i] = -1;
	memcpy(buffer, image.data, size);
	send(hSocket, rows, 4, 0);
	send(hSocket, cols, 4, 0);
	send(hSocket, buffer, size, 0);
	delete[]buffer;
}
Mat RECVMAT(SOCKET hSocket)
{
	char rows[4];
	char cols[4];
	char* buffer;
	int size, recvbytes, riter, citer, row, col;
	riter = citer = 1;
	recvbytes = row = col = 0;

	recv(hSocket, rows, 4, 0);
	recv(hSocket, cols, 4, 0);
	for (int i = 0; i < 4; i++)
	{
		if (rows[3 - i] != -1)
		{
			row += (rows[3 - i] - '0') * riter;
			riter *= 10;
		}
		if (cols[3 - i] != -1)
		{
			col += (cols[3 - i] - '0') * citer;
			citer *= 10;
		}
	}

	size = row * col * 3;
	buffer = new char[size];
	for (int i = 0; i < size; i += recvbytes)
	{
		if ((recvbytes = recv(hSocket, buffer + i, row * col * 3 - i, 0)) == -1)
			break;
	}
	recvbytes = 0;
	Mat  image = Mat::zeros(row, col, CV_8UC3);
	for (int i = 0; i < row; i++)
	{
		for (int j = 0; j < col; j++)
		{
			image.at<cv::Vec3b>(i, j) = cv::Vec3b(buffer[recvbytes + 0], buffer[recvbytes + 1], buffer[recvbytes + 2]);
			recvbytes = recvbytes + 3;
		}
	}

	delete[]buffer;
	return image;
}
/* 영상처리 주석
#include <iostream>
#include <sstream>
#include <opencv2/imgcodecs.hpp>// 기본 이미지 코덱이 들어있음 (코덱 : 영상 장비에서 촬영한 영상 파일이 너무 큰 것을 작게 만들거나, 즉 압축하거나 푸는 것)
#include <opencv2/imgproc.hpp>   // image processing을 위한 다양한 기능을 포함
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>   // 윈도우 화면, UI처리(슬라이더, 버튼 등) 및 마우스 제어 가능
#include <opencv2/video.hpp>    // 비디오 추적 및 배경 segmentation과 관련된 루틴을 포함

using namespace cv;            // OpenCV의 모든 함수와 클래스는 cv namespace안에 있음
using namespace std;

const char* params
= "{ help h         |           | Print usage }"
"{ input          | video4.mp4 | Path to a video or a sequence of image }"
"{ algo           | MOG2      | Background subtraction method (KNN, MOG2) }";


int main(int argc, char* argv[])
{
	CommandLineParser parser(argc, argv, params);   // argc: 명령행 매개 변수의 수량 argv: 명령행 매개 변수의 그룹 params: 받아들일 수 있는 명령줄 파라미터를 설명하는 문자열
	parser.about("This program shows how to use background subtraction methods provided by "
		" OpenCV. You can process both videos and images.\n");  // help h 설명문
	if (parser.has("help"))
	{
		parser.printMessage();  // help h 설명문 출력
	}

	//create Background Subtractor objects
	Ptr<BackgroundSubtractor> pBackSub;
	if (parser.get<String>("algo") == "MOG2")
		pBackSub = createBackgroundSubtractorMOG2();    // 배경 차분 알고리즘 1 createBackgroundSubtractorMOG2(히스토리 길이(기본500), 마할라노비스 거리, 그림자 검출여부(기본True)) >> 링크 참조
	else
		pBackSub = createBackgroundSubtractorKNN();     // 배경 차분 알고리즘 2 KNN 알고리즘을 기반으로 배경 추출기를 만듦
	VideoCapture capture(samples::findFile(parser.get<String>("input")));   // VideoCapture '변수 이름'( "동영상 이름" ) : '변수 이름'은 "동영상 이름"에 해당하는 동영상 화면(frame)을 읽어온다.
	if (!capture.isOpened())        // isOpened( ) 함수를 이용하여 동영상 파일이 제대로 읽혔는지 확인
	{
		//error in opening the video input
		cerr << "Unable to open: " << parser.get<String>("input") << endl;
		return 0;
	}

	int count = 0;
	Mat frame, fgMask;      // 행렬을 표현하는 클래스 Mat, 2차원 영상데이터를 저장하고 처리하는 용도. frame, fgMask, wave, dst 영상을 선언.
	Mat wave, dst;
	while (true)
	{
		capture >> frame;
		if (frame.empty())
			break;

		pBackSub->apply(frame, fgMask);     //update the background model

		rectangle(frame, cv::Point(10, 2), cv::Point(100, 20), cv::Scalar(255, 255, 255), -1);      //get the frame number and write it on the current frame
		std::stringstream ss;
		ss << capture.get(CAP_PROP_POS_FRAMES);     // CAP_PROP_POS_FRAMES : 동영상의 현재 프레임 수
		std::string frameNumberString = ss.str();
		putText(frame, frameNumberString.c_str(), cv::Point(15, 15), FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0));   // 영상에 프레임 번호를 넣는 명령어

		////////////////////////////// WAVE KILLING //////////////////////////////
		if (count++ == 0)
			threshold(fgMask, dst, 0, 0, THRESH_BINARY);        // threshold(input image, output image, 임계값, 임계값을 넘었을 때의 value, int type(threshold type_아래 링크 참조))

		threshold(fgMask, wave, 200, 1, THRESH_BINARY);
		add(wave, dst, dst);            // 영상 산술연산 함수 중 덧셈 연산 add 함수 : add(imageA, imageB, resultC); // c[i]=a[i]+b[i]
		if (count == 20)
		{
			threshold(dst, dst, 8, 255, THRESH_BINARY);
			imshow("WAVE_KILLNG", dst);
			count = 0;
		}
		////////////////////////////// WAVE KILLING //////////////////////////////


		//show the current frame and the fg masks
		imshow("Frame", frame);     // 영상 보여주는 함수
		imshow("FG Mask", fgMask);

		int keyboard = waitKey(10);     // 0 넣으면 스페이스바로 재생, 정지 가능 >> 이건 어쩌다 알게 됨
		if (keyboard == 'q' || keyboard == 27)
			break;
	}

	return 0;
}

// header file : https://velog.io/@cjh1995-ros/Opencv-C-basic

// using namespace cv : https://m.blog.naver.com/PostView.naver?isHttpsRedirect=true&blogId=ledzefflin&logNo=220502447505

// CommandLineParser parser(argc, argv, params) : https://intrepidgeeks.com/tutorial/commandlineparser-class-command-line-check-class

// putText 함수 : https://eehoeskrap.tistory.com/278

// Mat 클래스 : https://velog.io/@nayeon_p00/OpenCV-Mat-%ED%81%B4%EB%9E%98%EC%8A%A4-%EC%A0%95%EB%A6%AC

// thershold 함수 : https://jesus-never-fail.tistory.com/3

THRESH_BINARY : 픽셀 값이 threshold 보다 크면 maxval(255 흰색), 아니면 0(검정)

THRESH_BINARY_INV :픽셀 값이 threshold 보다 크면 0, 아니면 maxval

THRESH_TRUNC : 픽셀 값이 threshold 보다 크면 임계값, 아니면 픽셀 값 그대로 할당

THRESH_TOZERO : 픽셀 값이 threshold 보다 크면 픽셀값 그대로, 작으면 0 할당

THRESH_TOZERO_INV : 픽셀 값이 threshold 보다 크면 0, 작으면 픽셀값 그대로

// VideoCapture 클래스: https://diyver.tistory.com/57

// add 함수 : http://hongkwan.blogspot.com/2013/01/opencv-2-6-example.html
// 영상 산술연산 관련 : https://deep-learning-study.tistory.com/115

// 배경 삭제 코드 전체적인 설명. https://docs.opencv.org/3.4/d1/dc5/tutorial_background_subtraction.html 아래 explanation에 더욱 자세한 설명이 있는 링크를 걸어둠

// CAP_PROP_POS_FRAMES : https://076923.github.io/posts/Python-opencv-4/ 파이썬이긴하지만 함수에 대한 설명은 있음

// 배경 차분 알고리즘
// createBackgroundSubtractorMOG2 클래스 : https://mldlcvmjw.tistory.com/67
// createBackgroundSubtractorKNN 클래스 : https://www.codetd.com/ko/article/12588236


*/