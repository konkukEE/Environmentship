#include <stdio.h>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <algorithm>
#include <time.h>

#define TEST_FORWARD_INTERVAL 1

using namespace std;
using namespace cv;
using namespace cv::dnn;

struct detectionResult
{
	cv::Rect plateRect;
	double confidence;
	int type;
};
void NMS(std::vector<detectionResult>& vResultRect);
const char* params
= "{ help h         |           | Print usage }"
"{ input          | video2.mp4 | Path to a video or a sequence of image }"
"{ algo           | MOG2      | Background subtraction method (KNN, MOG2) }";
clock_t start1, start2, start3, end1, end2, end3;
void main(int argc, char* argv[])
{
	CommandLineParser parser(argc, argv, params);
	Mat input;// = imread("dog.jpg");
	string cfg = "yolov2-tiny.cfg";
	string weight = "yolov2-tiny.weights";

	start1 = clock();
	Net mnet = readNet(weight, cfg);
	end1 = clock();
	//VideoCapture capture(samples::findFile(parser.get<String>("input")));
	VideoCapture capture(0);
	if (!capture.isOpened())
	{
		cerr << "Unable to open: " << parser.get<String>("input") << endl;
		return;
	}

	while (1)
	{
		/////////////////////////////////////////////////////////////////////////**
		capture >> input;
		if (input.empty())
			break;
		rectangle(input, cv::Point(10, 2), cv::Point(100, 20), cv::Scalar(255, 255, 255), -1);      //get the frame number and write it on the current frame
		stringstream ss;
		ss << capture.get(CAP_PROP_POS_FRAMES);     // CAP_PROP_POS_FRAMES : 동영상의 현재 프레임 수
		string frameNumberString = ss.str();
		putText(input, frameNumberString.c_str(), cv::Point(15, 15), FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0));   // 영상에 프레임 번호를 넣는 명령어
		namedWindow("original", 0);
		resizeWindow("original", 1000, 1000);
		imshow("original", input);
		waitKey(10);
		//////////////////////////////////////////////////////////////////////////**
		Mat input_blob = blobFromImage(input, 1 / 255.F, cv::Size(416, 416), cv::Scalar(), true, false);
		mnet.setInput(input_blob);
		start2 = clock();
		Mat output;
		for (int i = 0; i < TEST_FORWARD_INTERVAL; i++)
			output = mnet.forward();
		end2 = clock();
		//cout << output.rows << endl << output.cols << endl;

		////////////////////////////////////////////////////////////
		start3 = clock();
		std::vector<detectionResult> vResultRect;
		for (int i = 0; i < output.rows; i++)
		{
			const int probability_index = 5;
			const int probability_size = output.cols - probability_index;
			float* prob_array_ptr = &output.at<float>(i, probability_index);
			size_t objectClass = std::max_element(prob_array_ptr, prob_array_ptr + probability_size) - prob_array_ptr;
			float confidence = output.at<float>(i, (int)objectClass + probability_index);
			if (confidence > 0.24f)
			{
				float x_center = output.at<float>(i, 0) * (float)input.cols;
				float y_center = output.at<float>(i, 1) * (float)input.rows;
				float width = output.at<float>(i, 2) * (float)input.cols;
				float height = output.at<float>(i, 3) * (float)input.rows;
				cv::Point2i p1(round(x_center - width / 2.f), round(y_center - height / 2.f));
				cv::Point2i p2(round(x_center + width / 2.f), round(y_center + height / 2.f));
				cv::Rect2i object(p1, p2);

				detectionResult tmp;
				tmp.plateRect = object;
				tmp.confidence = confidence;
				tmp.type = objectClass;
				vResultRect.push_back(tmp);
			}
		}
		NMS(vResultRect);
		end3 = clock();
		if (vResultRect.size() > 0)
			for (int i = 0; i < vResultRect.size(); i++)
			{
				cv::rectangle(input, vResultRect[i].plateRect, cv::Scalar(0, 0, 255), 2);
				printf("index: %d, confidence: %g\n", vResultRect[i].type, vResultRect[i].confidence);
			}
		namedWindow("after", 0);
		resizeWindow("after", 1000, 1000);
		imshow("after", input);
		imshow("after", input);
	}
	cv::imshow("input", input);
	cv::waitKey();

	///////////////////////////////////////////////////////////


	cout << endl << endl << (end1 - start1) << endl << (end2 - start2) / TEST_FORWARD_INTERVAL << endl << (end3 - start3) << endl;


	return;
}

void NMS(std::vector<detectionResult>& vResultRect)
{
	if (vResultRect.size() <= 1)
		return;

	for (int i = 0; i < vResultRect.size() - 1; i++)
	{
		for (int j = i + 1; j < vResultRect.size(); j++)
		{
			double IOURate = (double)(vResultRect[i].plateRect & vResultRect[j].plateRect).area() / (vResultRect[i].plateRect | vResultRect[j].plateRect).area();
			if (IOURate >= 0.5)
			{
				if (vResultRect[i].confidence > vResultRect[j].confidence) {
					vResultRect.erase(vResultRect.begin() + j);
					j--;
				}
				else {
					vResultRect.erase(vResultRect.begin() + i);
					i--;
					break;
				}
			}
		}
	}
}

// 영상처리 주석
/*
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
		stringstream ss;
		ss << capture.get(CAP_PROP_POS_FRAMES);     // CAP_PROP_POS_FRAMES : 동영상의 현재 프레임 수
		string frameNumberString = ss.str();
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