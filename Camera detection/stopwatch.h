#pragma once
#include <iostream>
#include <map>
#include <vector>
using namespace std;

class stopwatch
{
private:
	map<string, vector<clock_t>> nameset;

public:
	void record(string type);
	void print(int option = 0);


};

