#pragma once

#include <opencv2/opencv.hpp>
#include <fstream>
using namespace std;
using namespace cv;
using namespace cv::dnn;

struct detectionResult
{
	Rect plateRect;
	double confidence;
	int type;
};
vector<string> NetworkSetting(string weight, string cfg, string name);
vector<detectionResult> ObjectDetection(Mat input);
