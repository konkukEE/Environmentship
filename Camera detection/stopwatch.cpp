#include "stopwatch.h"

void stopwatch::record(string type)
{
	clock_t tmp = clock();
	nameset[type].push_back(tmp);
}
void stopwatch::print(int option)
{
	for (map<string, vector<clock_t>>::iterator iter = nameset.begin(); iter != nameset.end(); iter++)
	{
		string name = iter->first;
		vector<clock_t> data = iter->second;
		double avg = 0;

		cout << "******** " << name << " ********" << endl;
		for (int i = 1; i < ((int)data.size() & (~1)); i = i + 2)
		{
			if (option)
				cout << data[i] - data[i - 1] << "ms" << endl;
			avg += data[i] - data[i - 1];
		}
		avg /= ((int)data.size() & (~1)) / 2;
		cout << "measurements: " << data.size() << endl;
		cout << "average: " << avg << "ms" << endl;
	}


}