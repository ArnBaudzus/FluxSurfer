#include <iostream>
#include <quantum_system_code.cpp>
#include <dataSet_and_Solver.cpp>
#include <ode_solvers.cpp>
#include<parallel_processing_code.cpp>

using namespace std;

class TwoLevelSystem : QuantumSystem
{
	
	protected:
	
	bool actualisationNeedet(double time,State& s) override
	{
		return false;
	}

	void calculateProbabilities(double time,Edge& e,State& origin) override
	{
		
	}

	void addTunnelEdges(double time,State& Origin, std::vector<Edge> & edges)
	{
	
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
				newEdges.push_back(Edge(s,0.2,"Tunneling"));
			}
		}

		origin.storeEdges(time,newEdges);
	}
	
	public TwoLevelSystem(
		double initialTime,
		std::string path,
		double temperature,
		double fermiLevel
	):	
	{} 
};



