#include <iostream>
#include <quantum_system_code.cpp>
#include <dataSet_and_Solver.cpp>
#include <ode_solvers.cpp>
//#include<wanderer_code.cpp>
#include<parallel_processing_code.cpp>
#include<physicalFormulas.cpp>


using namespace std;

/**
* A Simple Quantumsystem that describes two Energy-niveaus that are in
* equilibirum with a larger classical system.
*/
class TwoLevelSystem : public QuantumSystem
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
		std::vector<Edge*> newEdges;

		BinaryNumber OriginOccupation = origin.occupiedLevels();
		

		for(int i = 0;i < Niveaus.size(); i++)
		{
			BinaryNumber indexTargetState = OriginOccupation.bitFlip(i);
			
			double probability = 0.2;
			std::string label = "";

			if(indexTargetState.readBit(i) == false)
			{
				// Tunneling out
				label = "Tunneling Out";
				probability = 1-fermi(Niveaus[i].energy(),
									  BackContactVoltage,
									  Temperature);
			}
			else
			{
				//Tunneling in
				label = "Tunneling In";
				probability =	fermi(Niveaus[i].energy(),
									  BackContactVoltage,
									  Temperature);
			}
			
			newEdges.push_back(new TunnelEdge(allStates[indexTargetState.asDecimal()],probability,label));
		}

		origin.storeEdges(time,newEdges);
	}

	void createEdges(double time, State& origin) override
	{
		createTunnelEdges(time,origin);
	}

	public:

	TwoLevelSystem(
		double initialTime,
		std::string path,
		double p_Temperature,
		double p_BackContactVoltage
	):
		QuantumSystem({Niveau("Level2",20e-3*e_minus,0.5),Niveau("Level1",40e-3*e_minus,0.5)},initialTime,path),
		Temperature(p_Temperature),
		BackContactVoltage(p_BackContactVoltage)
	{}
	
};

class TunnelExperiment : public Experiment
{
	protected:
	std::vector<double> TemperatureValues;
	std::vector<double> VoltageValues;

	//Initialisation Data
	double p_init_time = 0;
	double p_end_time = 10;
	int timeSteps = 1000;
	std::vector<double> initial_occupation={1,0,0,0};
	
	SingleStepScheme* scheme = new EulerForward();

	int NumberOfSystems;
	int SystemsDeployed = 0;
	
	virtual int MeasurementsToCome() override
	{
		return (NumberOfSystems - SystemsDeployed);
	}

	int VoltageIndex = 0;
	int TemperatureIndex = 0;

	std::pair<
		std::vector<std::string>,
		std::pair<QuantumSystem*,Solver*>> NextMeasurement() override
	{
		//Generating the System
		
		double Temperature = TemperatureValues[TemperatureIndex];
		double BCVoltage = VoltageValues[VoltageIndex]; 

		std::string simulationIndex = std::to_string(SystemsDeployed);
		
		while(simulationIndex.length() < 6)
		{
			simulationIndex = '0' + simulationIndex;
		}

		TwoLevelSystem* T = new TwoLevelSystem(0,
											   "TunnelExperimentSaves/"+
												simulationIndex+
											   "_VBC="+
												std::to_string(BCVoltage)+
												"_T="+
												std::to_string(Temperature)+
												".xml",
												Temperature,
												BCVoltage);
		
		//Setting up the Solver
		std::vector<double> keyFrames;
		
		for(int i = 0;i<timeSteps;i++)
		{
			keyFrames.push_back(p_init_time+(p_end_time*((double)i/(double)timeSteps)));
		}
		keyFrames.push_back(p_end_time);

		Solver* adaptiv = new RichardsonSolver(keyFrames,initial_occupation,T,scheme,1e-3,1e-2);
		
		//finalizing
		SystemsDeployed ++;
		VoltageIndex ++;
		if(VoltageIndex >= VoltageValues.size())
		{
			VoltageIndex = 0;
			TemperatureIndex ++;
		}

		auto job = std::make_pair(T,adaptiv);
		
		std::vector<std::string> attributes;
		attributes.push_back(std::to_string(BCVoltage));
		attributes.push_back(std::to_string(Temperature));

		return make_pair(attributes,job);
	}

	public:

	TunnelExperiment
	(
		std::vector<double> pTemperatureValues,
		std::vector<double> pVoltageValues,
		int pNumberThreads
	):
	Experiment(pNumberThreads,pTemperatureValues.size() * pVoltageValues.size(),
				"TunnelExperimentSaves",{"Back Contact Voltage / V","Temperature / K"},
				{},
				"The tunneling from a 2DEG into a 2 Niveau System without spin is simulated. Coulomb interaction, exchange interaction, relaxation and spinflip are neglected. The only way changes in the system state can be introduced is via tunnelling processes over the 2DEG.",
				"This is a Demo project, the main goal is to get warm with the library and the post processors."),
	TemperatureValues(pTemperatureValues),
	VoltageValues(pVoltageValues),
	NumberOfSystems(pTemperatureValues.size() * pVoltageValues.size())
	{}
	
};

int main()
{
	std::vector<double> Voltages;

	double minVoltage = 0;//V
	double maxVoltage = 60e-3;//V
	int numVoltageValues = 100;
	
	for(int i = 0;i<numVoltageValues;i++)
	{
		Voltages.push_back(minVoltage + ((maxVoltage)*((double)i/(double)numVoltageValues)));
	}

	TunnelExperiment experiment(
								{1,4.2,20,30,60,120,300},// Temperatures/K
								Voltages,// Voltages/V
								4  // Worker Threads
								);
	experiment.Conduct();
	
	return 0;
}
