#include <stdio.h>
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
	//string cfg = "yolov4.cfg";
	//string weight = "yolov4.weights";
	string cfg = "yolov2-tiny.cfg";
	string weight = "yolov2-tiny.weights";

	start1 = clock();
	Net mnet = readNet(weight, cfg);
	end1 = clock();
	VideoCapture capture(samples::findFile(parser.get<String>("input")));
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