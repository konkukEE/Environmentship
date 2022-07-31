#pragma once
#include "network.h"
#include "stopwatch.h"

void PrintDetection(Mat input, std::vector<detectionResult> output, std::vector<std::string>names, int frame = -1, int windowsize = 1000);
int vread(VideoCapture& capture, int camera = 0);
int vread(VideoCapture& capture, std::string sourcefile);
void vshow(VideoCapture& capture, Netinf network);
void vshow(Mat input, Netinf network);