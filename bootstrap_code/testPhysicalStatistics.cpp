#include<physicalFormulas.cpp>
#include<iostream>

using namespace std;

int main()
{
	double U = 0.02;
	double increment = 0.001*e_minus;

	for(int i = 0; i < 10; i++)
	{
		cout << fermi(0.015*e_minus + increment*i,U) << endl;
	}

	return 0;
}
