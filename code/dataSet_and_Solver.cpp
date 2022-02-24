#include<armadillo>
#include<vector>
#include<fstream>
#include<jsoncpp/json/writer.h> // Ubuntu
//#include<json/writer.h> // Arch

/**
* This class defines the minimal methods and variables all Solvers should
* implement/ have to be used in the context of this class. The rest of the code
* uses this class to interface with all solvers.
*/
class Solver
{
	protected:
	
	/**
	* This is a series of time-values. If this time is reached by the
	* simulation, the data of the quantumsystem is saved.
	*/
	std::vector<double> KeyFrameTime;

	/**
	* The System whichs time evolution should be calculated i.e. the problem
	* that should be solved.
	*/
	QuantumSystem* Problem;

	public:

	Solver(
		std::vector<double> p_KeyFrameTime,
		std::vector<double> p_initialOccupation,
		QuantumSystem* p_problem
	):
		KeyFrameTime(p_KeyFrameTime),
		Problem(p_problem)
	{}
	
	/**
	* This method calculates the time evolution of the system (i.e. the solution
	* of the problem) and stores it in Data. It the returns Data.
	*/
	virtual void solve() = 0;
};
