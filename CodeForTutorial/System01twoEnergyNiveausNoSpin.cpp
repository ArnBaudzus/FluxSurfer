#include <iostream>
#include <quantum_system_code.cpp>
#include <dataSet_and_Solver.cpp>
#include <ode_solvers.cpp>
//#include<wanderer_code.cpp>
#include<parallel_processing_code.cpp>

using namespace std;

/**
* A Simple Quantumsystem that describes two Energy-niveaus that are in
* equilibirum with a larger classical system.
*/
class TwoNiveauSystem : public QuantumSystem
{
	protected:

	double Temperature;
	double BackContactVoltage;
	
	bool actualisationNeedet(double time,State& s) override
	{
		return false; //Constant Probabilities
	}
	
	class TunnelEdge : public Edge
	{
		protected:

		virtual void update(double time) override
		{
			//Just do Nothing
		}

		public:

		TunnelEdge(
			State& p_targetState,
			double p_transitionProbabilitie,
			std::string id
		):
		Edge(p_targetState,p_transitionProbabilitie,id)
		{
			
		}

	};
	
	void createTunnelEdges(double time,State& origin)
	{
		
	}

	void createEdges(double time, State& origin) override
	{
		std::vector<Edge*> newEdges;

		for(State& s: allStates)
		{
			if(s.number() == origin.number())
			{
				continue;
			}
			else
			{
				newEdges.push_back(new TunnelEdge(s,0.2,"Tunneling"));
			}
		}

		origin.storeEdges(time,newEdges);
	}

	public:

	TwoNiveauSystem(
		double initialTime,
		std::string path,
		double p_Temperature,
		double p_BackContactVoltage
	):
		QuantumSystem({Niveau("Level2",20e-3,0.5),Niveau("Level1",40e-3,0.5)},initialTime,path),
		Temperature(p_Temperature),
		BackContactVoltage(p_BackContactVoltage)
	{}
	
};

class TestExperiment : public Experiment
{
	protected:
	//Initialisation Data
	double p_init_time = 0;
	double p_end_time = 1;
	std::vector<double> initial_occupation={1,0,0,0};
	
	SingleStepScheme* scheme = new EulerForward();

	int NumberOfSystems;
	int SystemsDeployed = 0;
	
	virtual int MeasurementsToCome() override
	{
		return (NumberOfSystems - SystemsDeployed);
	}

	std::pair<
		std::vector<std::string>,
		std::pair<QuantumSystem*,Solver*>> NextMeasurement() override
	{
		TestSystem* T = new TestSystem(0,"testSystemSaves/System no. "+std::to_string(SystemsDeployed));
		
		std::vector<double> keyFrames;
		
		for(int i = 0;i<10;i++)
		{
			keyFrames.push_back(SystemsDeployed+((double)i/10.0));
		}
		keyFrames.push_back(SystemsDeployed+1);

		Solver* adaptiv = new RichardsonSolver(keyFrames,initial_occupation,T,scheme,1e-3,1e-2);
		SystemsDeployed ++;

		auto job = std::make_pair(T,adaptiv);
		
		std::vector<std::string> attributes;
		attributes.push_back(std::to_string(SystemsDeployed-1));
		
		return make_pair(attributes,job);
	}

	public:

	TestExperiment
	(
		int pNumberOfSystems,
		int pNumberThreads
	):
	Experiment(pNumberThreads,pNumberOfSystems,
				"testSystemSaves",{"MeasurementNumber"},
				{{"T/K","4.2"},{"B/T","3"},{"Mensa@11:30am?","true"}},
				"A Test Experiment with similar measurements of a quantumsystem with identical transition probabilities",
				"The Aim is to see if the code is working as expected."),
	NumberOfSystems(pNumberOfSystems)
	{}
	
};

int main()
{
	

	TestExperiment test(10,4);
	test.Conduct();
	
	return 0;
}
