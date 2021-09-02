#include <iostream>
#include <quantum_system_code.cpp>
#include <dataSet_and_Solver.cpp>
#include <ode_solvers.cpp>
#include <queue>
#include <thread>
#include <chrono>
#include <mutex>
//#include<wanderer_code.cpp>

using namespace std;





std::mutex commonMutex;


std::queue<std::pair<QuantumSystem*,Solver*>> WorkSource;
std::queue<std::pair<QuantumSystem*,Solver*>> FinishedTasks;
	
bool Running=true;

bool shouldContinue()
{
	std::lock_guard<std::mutex> guard(commonMutex);
	return Running;
}

int jobsPending()
{
	std::lock_guard<std::mutex> guard(commonMutex);
	return WorkSource.size(); 
}

bool thereIsJob()
{
	std::lock_guard<std::mutex> guard(commonMutex);
	return !WorkSource.empty(); 
}

void handleFinishedJob(QuantumSystem* Job,Solver* s)
{
	std::lock_guard<std::mutex> guard(commonMutex);
	FinishedTasks.push(std::make_pair(Job,s));
}

void stopWork()
{
	std::lock_guard<std::mutex> guard(commonMutex);
	Running = false;
}

std::pair<QuantumSystem*,Solver*> getJob()
{
	std::lock_guard<std::mutex> guard(commonMutex);
	std::pair<QuantumSystem*,Solver*> toReturn = WorkSource.front();
	WorkSource.pop();
		
	return toReturn;
}

void run(std::string test)
{
	while(thereIsJob())
	{
		QuantumSystem* problem;
		Solver* s;
	
		std::tie(problem,s) = getJob();
				
		s->solve();
				
		handleFinishedJob(problem,s);
	}
}
	
class Experiment
{
	protected:
	
	int InitialMeasurementCount=0;

	int WorkerCount;

	QuantumSystem* mergedSystem=nullptr;
	
	std::vector<std::thread> Workers;
	
	virtual std::pair<QuantumSystem*,Solver*> NextMeasurement()=0;
	
	virtual bool MeasurementsToCome()=0;

	void WaitForCalculation()
	{
		while(thereIsJob())
		{
			std::cout << '\r' << jobsPending() << " Jobs Pending.";
			std::this_thread::sleep_for(std::chrono::milliseconds(1));	
		}
	}

	public:
	
	Experiment
	(
		int pWorkerCount=4
	)
	:WorkerCount(pWorkerCount)
	{}
	
	void Conduct()
	{
		while(MeasurementsToCome() != 0)
		{	
			WorkSource.push(NextMeasurement());
		}
		
		for(int i =0; i< WorkerCount;i++)
		{
			std::string tIden = "T"+std::to_string(i+1);
			Workers.push_back(std::thread(run,tIden));
		}
		
		

		WaitForCalculation();
		
		for(std::thread& t : Workers)
		{
			t.join();
		}
		
		auto& pair = FinishedTasks.front();
		delete pair.second;
		mergedSystem=pair.first;
		FinishedTasks.pop();

		while(!FinishedTasks.empty())
		{
			auto& p = FinishedTasks.front();
			delete p.second;
			mergedSystem->mergeDataWith(*p.first);
			delete p.first;
			FinishedTasks.pop();
		}
	}

	QuantumSystem* results()
	{
		return mergedSystem;
	}
};

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
	
	virtual bool MeasurementsToCome() override
	{
		return 0<(NumberOfSystems - SystemsDeployed);
	}

	virtual std::pair<QuantumSystem*,Solver*> NextMeasurement() override
	{
		TestSystem* T = new TestSystem(0);
		
		std::vector<double> keyFrames;
		
		for(int i = 0;i<10;i++)
		{
			keyFrames.push_back(SystemsDeployed+((double)i/10.0));
		}
		keyFrames.push_back(SystemsDeployed+1);

		Solver* adaptiv = new RichardsonSolver(keyFrames,initial_occupation,T,scheme,1e-3,1e-2);
		SystemsDeployed ++;

		return std::make_pair(T,adaptiv);
	}

	public:

	TestExperiment
	(
		int pNumberOfSystems,
		int pNumberThreads
	):
	Experiment(pNumberThreads),
	NumberOfSystems(pNumberOfSystems)
	{}
	
};

int main()
{
	

	TestExperiment test(100000,4);
	test.Conduct();
	QuantumSystem* T = test.results();
	T->writeToFile("test.Graphml");
	
	delete T;
	return 0;
}
