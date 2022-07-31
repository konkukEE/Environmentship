#pragma once
#include <iostream>
#include <map>
#include <vector>

class stopwatch
{
private:
	std::map<std::string, std::vector<clock_t>> nameset;

public:
	void record(std::string type);
	void print(int option = 0);


};

