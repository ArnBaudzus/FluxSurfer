#pragma once
#include<cmath>

#define k_b 1.380649e-23
#define e_minus 1.602176634e-19


/**
* Evaluates the Fermi-Dirac-distribution for given values of E,U,T
*
* @param E Energy value of the evaluation of the distribution function.
* @param U The chemical potential in volts. In Experiments, the fermi niveau of
* e.g. electrons in a contanct is often shifted by a voltage. This parameter
* represents that voltage.
* @param T The temperature in K.
*/
double fermi(
			 double E,		 //in J
			 double U = 0,	 //in V
			 double T = 4	 //in K
			)
{
	return 1 / (1 + exp((E - e_minus*U)/(k_b*T)));
}
