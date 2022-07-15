#include "network.h"

Net mnet;
vector<string> NetworkSetting(string weight, string cfg, string name)
{
	mnet = readNet(weight, cfg);
	vector<string> names;
	ifstream file(name);
	if (file.is_open())
	{
		int name_index = 0;
		string tmp;
		while (getline(file, tmp))
			names.push_back(tmp);
		file.close();
	}
	else
		cout << "Unable to open file";

	return names;
}
vector<string> NetworkSetting(string onnx, string name)
{
	mnet = readNet(onnx);
	vector<string> names;
	ifstream file(name);
	if (file.is_open())
	{
		int name_index = 0;
		string tmp;
		while (getline(file, tmp))
			names.push_back(tmp);
		file.close();
	}
	else
		cout << "Unable to open file";
	return names;
}
void ObjectDetection(Mat input, vector<detectionResult>& result)
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
	float x_factor = (float)input.cols / BLOBSIZE;
	float y_factor = (float)input.rows / BLOBSIZE;
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

	blobFromImage(input, blob, 1 / 255.F, Size(BLOBSIZE, BLOBSIZE), Scalar(), true, false);
	mnet.setInput(blob);
	mnet.forward(output, mnet.getUnconnectedOutLayersNames());
	data = (float*)output[0].data;

	for (i = 0; i < TOTALBOX; i++)
	{
		confidence = data[4];
		if (confidence >= CONFIDENCE_THRESHOLD)
		{
			score = data + 5;
			Mat scores(1, CLASS_NAME_SIZE, CV_32FC1, score);
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
		data += CLASS_SIZE;
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
void ObjectDetection(Mat input, vector<detectionResult>& result, int trigger)
{
	Mat input_blob = blobFromImage(input, 1 / 255.F, Size(416, 416), Scalar(), true, false);
	mnet.setInput(input_blob);
	Mat output = mnet.forward();

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