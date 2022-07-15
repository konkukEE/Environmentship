#pragma once

#include <opencv2/opencv.hpp>
#include <fstream>
#include <opencv2/imgproc.hpp>
using namespace std;
using namespace cv;
using namespace cv::dnn;

#define CONFIDENCE_THRESHOLD 0.375F
#define CLASS_THRESHOLD 0.5F
#define NMS_THRESHOLD 0.4375F
#define BLOBSIZE 416
#define TOTALBOX 10647

#define CLASS_SIZE 85
#define CLASS_NAME_SIZE CLASS_SIZE-5


struct detectionResult
{
	Rect plateRect;
	double confidence;
	int type;
};
vector<string> NetworkSetting(string weight, string cfg, string name);
vector<string> NetworkSetting(string onnx, string name);
void ObjectDetection(Mat input, vector<detectionResult>& result);
void ObjectDetection(Mat input, vector<detectionResult>& result, int trigger);