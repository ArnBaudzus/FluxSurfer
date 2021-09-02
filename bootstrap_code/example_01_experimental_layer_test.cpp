#include<iostream>
#include<quantum_system_code.cpp>
#include<dataSet_and_Solver.cpp>
#include<ode_solvers.cpp>
//#include<wanderer_code.cpp>

using namespace std;

/**
* A Simple Quantumsystem that describes two Energy-niveaus that are in
* equilibirum with a larger classical system.
*/
class TestSystem : public QuantumSystem
{
	protected:
	
	bool actualisationNeedet(double time,State& s) override
	{
		return false; // Constant Probabilities
	}

	void calculateProbabilities(double time,Edge& e,State& origin) override
	{
		e.transitionProbabilitie = 0.2;
	}

	void createEdges(double time, State& origin) override
	{
		std::vector<Edge> newEdges;

		for(State& s: allStates)
		{
			if(s.number() == origin.number())
			{
				continue;
			}
			else
			{
				newEdges.push_back(Edge(s,0.2));
			}
		}

		origin.storeEdges(time,newEdges);

	}

	public:

	TestSystem(
		double initialTime
	):
		QuantumSystem(4,initialTime)
	{}
	
};

int main()
{
	//Initialisation
	double p_init_time = 0;
	double p_end_time = 1;
	
	std::vector<double> initial_occupation={1,0,0,0};

	TestSystem T(0);
	TestSystem U(0);
	
	T.logMoment(0,initial_occupation);
	U.logMoment(1,initial_occupation);

	SingleStepScheme* scheme = new EulerForward();
	
	RichardsonSolver adaptiv1(0,1,initial_occupation,&T,scheme,1e-3,1e-2);
	RichardsonSolver adaptiv2(0,1,initial_occupation,&U,scheme,1e-3,1e-2);
	
	adaptiv1.solve();
	adaptiv2.solve();

	T.mergeDataWith(U);
	T.writeToFile("test.Graphml");

//	SingleStepScheme* scheme = new EulerForward();
	
	//Fixes Stepwidth
//	Solver* fixed = new
//	FixedStepwidthSolver(p_init_time,p_end_time,initial_occupation,&T,scheme,10);	
	
//	DataSet fixed_solution = fixed->solution();
	
//	fixed_solution.writeToJSON("Test_fixed_step.json");
	
	// Adaptive Stepwidth
//
	
//	DataSet adaptiv_solution = adaptiv->solution();
	
//	adaptiv_solution.writeToJSON("Test_adaptiv_step.json");
	
	// Monte Carlo

//	StdRandom rand;
//	
//	Solver* wanderer = new 	
//	MonteCarloWanderer(p_init_time,p_end_time,initial_occupation,&T,&rand,10,1000000);
	
//	DataSet monte_Charlo_solution = wanderer->solution();
	
//	monte_Charlo_solution.writeToJSON("Test_montecarlo_step.json");
	
	//Freeing Memory
	
//	delete adaptiv;
//	delete fixed;
//	delete wanderer;
//	delete scheme;

	return 0;
}
