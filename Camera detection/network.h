#pragma once

#include <opencv2/opencv.hpp>
#include <fstream>
#include <opencv2/imgproc.hpp>

using namespace cv;
using namespace cv::dnn;

#define CONFIDENCE_THRESHOLD 0.375F
#define CLASS_THRESHOLD 0.5F
#define NMS_THRESHOLD 0.4375F

class Netinf
{
public:
	Netinf(std::string onnx, std::vector<std::string>name, int blobsize);
	Netinf(std::string weight, std::string cfg);
	std::vector<std::string> names;
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
Netinf NetworkSetting(std::string onnx, std::string name, int blobsize);
void ObjectDetection(Mat input, Netinf net, std::vector<detectionResult>& result);

// darknet weight
Netinf NetworkSetting(std::string weight, std::string cfg, std::string name);
void ObjectDetection(Mat input, Netinf net, std::vector<detectionResult>& result, int trigger);