#include "network.h"


// onnx weight
Netinf::Netinf(string onnx, vector<string> name, int blobsize)
{
	this->mnet = readNet(onnx);
	this->names = name;
	this->BLOBSIZE = blobsize;
	this->NAME_SIZE = name.size();
	this->OUTPUT_SIZE = this->NAME_SIZE + 5;

	if (blobsize == 320)
		this->TOTALBOX = 6300;
	else if (blobsize == 416)
		this->TOTALBOX = 10647;
	else if (blobsize == 640)
		this->TOTALBOX = 25200;
	else
		cout << "BLOB SIZE NOT SUPPORTED" << endl;
}
Netinf NetworkSetting(string onnx, string name, int blobsize)
{
	vector<string> tmpv;

	ifstream file(name);
	if (file.is_open())
	{
		string tmp;
		while (getline(file, tmp))
			tmpv.push_back(tmp);

		file.close();
	}
	else
		cout << "Unable to open file";

	Netinf network(onnx, tmpv, blobsize);

	return network;
}
void ObjectDetection(Mat input, Netinf net, vector<detectionResult>& result)  // Process the input image using net to produce the result
{
	// Data for forwarding
	Mat blob;
	vector<Mat> output;
	float* data;

	// Data before NMS
	vector<int> class_ids;      // Detected Class 
	vector<float> confidences;  // Detected Score
	vector<Rect> boxes;         // Detected Rect

	// Variable for Data Collecting
	float x_factor = (float)input.cols / net.BLOBSIZE;
	float y_factor = (float)input.rows / net.BLOBSIZE;
	float cx, cy, w, h;
	Point class_id;
	float confidence;
	float* score;
	double max_score;

	// After NMS index
	vector<int> indices;

	// Iterator
	int i, idx;
	detectionResult tmp;

	blobFromImage(input, blob, 1 / 255.F, Size(net.BLOBSIZE, net.BLOBSIZE), Scalar(), true, false);
	net.mnet.setInput(blob);
	net.mnet.forward(output, net.mnet.getUnconnectedOutLayersNames());
	data = (float*)output[0].data;

	for (i = 0; i < net.TOTALBOX; i++)
	{
		confidence = data[4];
		if (confidence >= CONFIDENCE_THRESHOLD)
		{
			score = data + 5;
			Mat scores(1, net.NAME_SIZE, CV_32FC1, score);
			minMaxLoc(scores, 0, &max_score, 0, &class_id);
			if (max_score > CLASS_THRESHOLD)
			{
				confidences.push_back(confidence);
				class_ids.push_back(class_id.x);
				cx = data[0];
				cy = data[1];
				w = data[2];
				h = data[3];

				Point2i p1(round((cx - w / 2.F) * x_factor), round((cy - h / 2.F) * y_factor));
				Point2i p2(round((cx + w / 2.F) * x_factor), round((cy + h / 2.F) * y_factor));
				Rect2i object(p1, p2);

				boxes.push_back(object);
			}
		}
		data += net.OUTPUT_SIZE;
	}

	NMSBoxes(boxes, confidences, CLASS_THRESHOLD, NMS_THRESHOLD, indices);
	for (i = 0; i < indices.size(); i++)
	{
		idx = indices[i];
		tmp.plateRect = boxes[idx];
		tmp.confidence = confidences[idx];
		tmp.type = class_ids[idx];
		result.push_back(tmp);

	}

	return;
}

// darknet weight
Netinf::Netinf(string weight, string cfg)
{
	this->mnet = readNet(weight, cfg);
	this->BLOBSIZE = -1;
}
Netinf NetworkSetting(string weight, string cfg, string name)
{
	Netinf network(weight, cfg);


	ifstream file(name);
	if (file.is_open())
	{
		string tmp;
		while (getline(file, tmp))
			network.names.push_back(tmp);

		file.close();
	}
	else
		cout << "Unable to open file";

	return network;
}
void ObjectDetection(Mat input, Netinf net, vector<detectionResult>& result, int trigger)
{
	Mat input_blob = blobFromImage(input, 1 / 255.F, Size(416, 416), Scalar(), true, false);
	net.mnet.setInput(input_blob);
	Mat output = net.mnet.forward();

	for (int i = 0; i < output.rows; i++)
	{
		const int probability_index = 5;
		const int probability_size = output.cols - probability_index;
		float* prob_array_ptr = &output.at<float>(i, probability_index);
		size_t objectClass = max_element(prob_array_ptr, prob_array_ptr + probability_size) - prob_array_ptr;
		float confidence = output.at<float>(i, (int)objectClass + probability_index);
		if (confidence > 0.5)
		{
			float x_center = output.at<float>(i, 0) * (float)input.cols;
			float y_center = output.at<float>(i, 1) * (float)input.rows;
			float width = output.at<float>(i, 2) * (float)input.cols;
			float height = output.at<float>(i, 3) * (float)input.rows;
			Point2i p1(round(x_center - width / 2.f), round(y_center - height / 2.f));
			Point2i p2(round(x_center + width / 2.f), round(y_center + height / 2.f));
			Rect2i object(p1, p2);

			detectionResult tmp;
			tmp.plateRect = object;
			tmp.confidence = confidence;
			tmp.type = objectClass;
			result.push_back(tmp);
		}
	}
	for (int i = 0; i < (int)result.size() - 1; i++)
	{
		for (int j = i + 1; j < result.size(); j++)
		{
			double IOURate = (double)(result[i].plateRect & result[j].plateRect).area() / (result[i].plateRect | result[j].plateRect).area();
			if (IOURate >= 0.5)
			{
				if (result[i].confidence > result[j].confidence)
				{
					result.erase(result.begin() + j);
					j--;
				}
				else
				{
					result.erase(result.begin() + i);
					i--;
					break;
				}
			}
		}
	}

}