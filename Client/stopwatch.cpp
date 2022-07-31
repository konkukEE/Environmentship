#include "stopwatch.h"

void stopwatch::record(std::string type)
{
	clock_t tmp = clock();
	nameset[type].push_back(tmp);
}
void stopwatch::print(int option)   // Option1: Only the last measured value is displayed
{
	for (std::map<std::string, std::vector<clock_t>>::iterator iter = nameset.begin(); iter != nameset.end(); iter++)
	{
		std::string name = iter->first;
		std::vector<clock_t> data = iter->second;
		double avg = 0;

		std::cout << "******** " << name << " ********" << std::endl;
		for (int i = 1; i < ((int)data.size() & (~1)); i = i + 2)
		{
			if (option)
			{
				std::cout << data[((int)data.size() & (~1)) - 1] - data[((int)data.size() & (~1)) - 2] << "ms" << std::endl;
				return;
			}
			avg += data[i] - data[i - 1];
		}
		avg /= ((int)data.size() & (~1)) / 2;
		std::cout << "measurements: " << data.size() << std::endl;
		std::cout << "average: " << avg << "ms" << std::endl;
	}


}