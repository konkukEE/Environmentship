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
vector<detectionResult> ObjectDetection(Mat input)
{
	Mat input_blob = blobFromImage(input, 1 / 255.F, Size(416, 416), Scalar(), true, false);
	mnet.setInput(input_blob);
	Mat output = mnet.forward();

	vector<detectionResult> result;
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
	}     // 

	return result;
}