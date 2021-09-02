#include<vector>
#include<random>

/**
* This abstract Class describes how a source of random numbers used by the
* montecarlo-algorithms in this project should look like. All montecarlo
* algorithms are using this baseclass. This way sources for random numbers can
* be changed on the fly.
*/
class RandomSource
{
	public:
	
	/**
	* Returns a random number between 0 and 1.
	*/
	virtual double number()=0;
};

/**
* Generates a random Number between 0 and 1 useing std::rand();
*/
class StdRandom : public RandomSource
{
	public:

	double number() override
	{
		double rand = (double)std::rand()/(double)RAND_MAX;
		return rand;
	}
};

/**
* This Solver calculates the timeevolution of the system by "exploreing" its
* graph. For each step the Solver samples a random initial state from the probabilities
* calculated in the last step. It than randomly chooses a transition with
* respect to the probabilitie that a transition occurs given by the
* Quantumsystem.  This is made many times per step. The final states after the
* step are saved in a histogram. The Occupation is than calculated by
* normalizing the integral. Staying in the initial state is a trivial transition
* in this context. I'm relatively sure that this is equal to solving the
* Masterequation with some sort of single step montecarlo integrator for the
* integration steps.<br>
* The great advantage this algorithm has over a plain Masterequation solver is,
* that only the probabilities and transitions for states that are actually
* occupied by the system must be calculated.
*/
class MonteCarloWanderer : public Solver
{
	protected:
	
	/**
	* Random numbers used by this solver are sampled form here.
	*/	
	RandomSource* Random;	
	
	double StepWidth;
	
	/**
	* The number of random transitions in one timestep. Note that staying in the
	* initial state is a trivial transition.
	*/
	int TransitionsPerStep;
	
	/**
	* The in simulation time corresponding to the last calculated occupation.
	*/	
	double CurrentTime;
	
	/**
	* The Occupation that was calculated in the last step or the initial
	* occupation.
	*/
	std::vector<double> CurrentOccupation; 

	public:
	
	MonteCarloWanderer(
		double p_initialTime,
		double p_destinationTime,
		std::vector<double> p_initialOccupation,
		QuantumSystem* p_problem,
		RandomSource* p_Random,
		int p_Steps,
		int p_TransitionEvents
	):
		Solver(p_initialTime,p_destinationTime,p_initialOccupation,p_problem),
		Random(p_Random),
		StepWidth((p_destinationTime-p_initialTime)/p_Steps),
		TransitionsPerStep(p_TransitionEvents/p_Steps),
		CurrentTime(p_initialTime)
	{
		if(TransitionsPerStep==0)
		{
			TransitionsPerStep=1;
		}
		for(double& d : p_initialOccupation)
		{
			CurrentOccupation.push_back(d);
		}
	}

	/**
	* The wandering process that is described in the classdescription of this
	* class.
	*/
	DataSet solution() override
	{
		int States = CurrentOccupation.size();
		int* histogram = new int[States]();
		
		while(CurrentTime < DestinationTime)
		{
			//reset Histogram
			for(int i=0;i<States;i++)
			{
				histogram[i] =0;
			}
			
			for(int i=0;i<TransitionsPerStep;i++)
			{

			//rolling on the Outgoing state
			
			double random = Random->number();
			double cummulation=0;
			int StateNumber = 0;
		
			for(double& d : CurrentOccupation)
			{
				cummulation += d;
				if(cummulation > random)
				{
					cummulation = 0;
					break;
				}
				else
				{
					StateNumber++;
				}
			}

			//rolling on the Transition
			random = Random->number();
			
			std::vector<std::pair<int,double>> transitions =
				Problem->possibleTransitions(StateNumber,CurrentTime);

			for(auto& transition : transitions)
			{
				cummulation += transition.second*StepWidth;
				if(cummulation > random)
				{
					StateNumber = transition.first;
					break;
				}
			}
			
			histogram[StateNumber]++;
			
			}

			//Saveing Data
			for(int i=0;i<States;i++)
			{
				CurrentOccupation[i] = 
					((double)histogram[i]/(double)TransitionsPerStep);
			}
			CurrentTime+=StepWidth;
			Data.push_back(CurrentTime,CurrentOccupation);
		}
		
		delete[] histogram;

		return Data;
	}
};
