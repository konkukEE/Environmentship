#pragma once
#include "network.h"

void PrintDetection(Mat input, vector<detectionResult> output, vector<string>names, int frame = -1, int windowsize = 1000);
int vread(VideoCapture& capture, int camera = 0);
int vread(VideoCapture& capture, string sourcefile);
void vshow(VideoCapture& capture, vector<string>names);