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

class Netinf
{
public:
	Netinf(string onnx, vector<string>name, int blobsize);
	Netinf(string weight, string cfg);
	vector<string> names;
	Net mnet;
	int BLOBSIZE;
	int NAME_SIZE;
	int OUTPUT_SIZE;
	int TOTALBOX;
};
struct detectionResult
{
	Rect plateRect;
	double confidence;
	int type;
};

// onnx weight
Netinf NetworkSetting(string onnx, string name, int blobsize);
void ObjectDetection(Mat input, Netinf net, vector<detectionResult>& result);

// darknet weight
Netinf NetworkSetting(string weight, string cfg, string name);
void ObjectDetection(Mat input, Netinf net, vector<detectionResult>& result, int trigger);