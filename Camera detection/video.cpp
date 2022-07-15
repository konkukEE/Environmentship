#pragma once
#include "video.h"
#include "stopwatch.h"

void PrintDetection(Mat input, vector<detectionResult> output, vector<string> names, int frame, int windowsize)
{
	rectangle(input, Point(0, 0), Point(100, 30), cv::Scalar(255, 255, 255), -1);      //get the frame number and write it on the current frame
	putText(input, to_string(frame), Point(10, 25), 0, 1, Scalar(0, 0, 0), 2);   // 영상에 프레임 번호를 넣는 명령어
	namedWindow("before", 0);
	resizeWindow("before", windowsize, windowsize);
	imshow("before", input);
	waitKey(1);

	for (int i = 0; i < (int)output.size(); i++)
	{
		rectangle(input, output[i].plateRect, Scalar(0, 0, 255), 2);
		putText(input, names[output[i].type], Point2i(output[i].plateRect.x, output[i].plateRect.y), 0, 1.7, Scalar(0, 255, 0), 2.8);
		cout << "frame:" << frame << "  name: " << names[output[i].type] << ", confidence: " << output[i].confidence << endl;
	}
	namedWindow("after", 0);
	resizeWindow("after", windowsize, windowsize);
	imshow("after", input);
}
int vread(VideoCapture& capture, int camera)
{
	capture.open(camera);

	if (!capture.isOpened())
	{
		cerr << "Unable to open camera #: " << camera << endl;
		return -1;
	}

	return 1;
}
int vread(VideoCapture& capture, string sourcefile)
{
	capture.open(sourcefile);

	if (!capture.isOpened())
	{
		cerr << "Unable to open: " << sourcefile << endl;
		return -1;
	}

	return 1;
}
void vshow(VideoCapture& capture, vector<string> names)
{
	Mat input;
	int frame = 0;
	stopwatch speedtest;
	while (1)
	{
		capture >> input;
		if (input.empty())
		{
			waitKey();
			break;
		}

		speedtest.record("forward");
		vector<detectionResult> vResultRect;
		ObjectDetection(input, vResultRect);
		//ObjectDetection(input, vResultRect, 1);
		speedtest.record("forward");
		speedtest.record("processing");
		PrintDetection(input, vResultRect, names, ++frame);
		speedtest.record("processing");
	}
	speedtest.print();
}