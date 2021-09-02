#include<armadillo>

/**
* This class defines how a numerical single step scheme for the integration of a
* differential equation should be implemented in this program. All Solvers that
* use single step schemes work with this class. This way, if a scheme is
* implemented it can be used with all solvers. 
*/
class SingleStepScheme
{
	public:
	
	/**
	* The convergence order of the method. This information is of special
	* importance when solvers with adaptive time should be implemented. 
	*/
	virtual int convergenceOrder()=0;
	
	/**
	* This Method calculates one solver step with the scheme. See the
	* EulerForeward scheme for examples.
	*/
	virtual arma::Col<double> step(double t_n,arma::Col<double>& x_n,QuantumSystem* problem,double h)=0;
};

/**
* A implementation of the simple euler foreward scheme.
*/
class EulerForward : public SingleStepScheme
{
	public:
	
	arma::Col<double> step(double t_n,arma::Col<double>& x_n,QuantumSystem* problem,double h) override
	{
		return x_n + h*problem->ODE(t_n,x_n);
	}
	
	int convergenceOrder() override
	{
		return 1;
	}
};

/**
* This Class proveides a abstract implementation of a SingleStepSolver. It
* incooperates the similaritys a Fixed stepwidth and a adaptive stepwidth solver
* have.
*/
class SingleStepODESolver : public Solver
{
	protected:
	
	/**
	* The current in simulation time.
	*/
	double CurrentTime;
	
	/**
	* The Value of the last step calculated or the initial value
	*/
	arma::Col<double> CurrentValue;
	
	/**
	* The numeric integration scheme to calculate the integration steps.
	*/
	SingleStepScheme* Scheme;
	
	/**
	* This method returns the stepwidth for the next integration step. For the
	* fixed stepwidth solver this simply means  that it returns a constant
	* stepwidth. Whereas in the adaptive stepwidth solver the stepwidth for the
	* next step is calculated here.
	*/
	virtual double stepWidth()=0;

	public:
	
	SingleStepODESolver(
		std::vector<double> p_KeyFrameTime,
		std::vector<double> p_initialOccupation,
		QuantumSystem* p_problem,
		SingleStepScheme* p_Scheme
	):
		Solver(p_KeyFrameTime,p_initialOccupation,p_problem),
		CurrentTime(p_KeyFrameTime.front()),
		CurrentValue(arma::vec(p_initialOccupation.size(),arma::fill::zeros)),
		Scheme(p_Scheme)
	{
		for(int i=0;i<p_initialOccupation.size();i++)
		{
			CurrentValue(i) = p_initialOccupation[i];
		}
	}

	void solve() override
	{
		while(!KeyFrameTime.empty())
		{
			double h = stepWidth();

			CurrentValue = Scheme->step(CurrentTime,CurrentValue,Problem,h);
			CurrentTime += h;
			
			if(CurrentTime >= KeyFrameTime.front())
			{
				std::vector<double> toLog;
			
				for(int i =0; i< Problem->numberOfStates();i++)
				{
					toLog.push_back(CurrentValue(i));
				}
				
				
				Problem->logMoment(CurrentTime,toLog);
				KeyFrameTime.erase(KeyFrameTime.begin());
			}
		}
	}
};

class FixedStepwidthSolver : public SingleStepODESolver
{
	protected:
	
	double theStepWidth;
	
	double stepWidth() override
	{
		return theStepWidth;
	}

	public:

	FixedStepwidthSolver(
		std::vector<double> p_KeyFrameTime,
		std::vector<double> p_initialOccupation,
		QuantumSystem* p_problem,
		SingleStepScheme* p_Scheme,
		int number_of_Steps
	):
		SingleStepODESolver(p_KeyFrameTime,p_initialOccupation,p_problem,p_Scheme),
		theStepWidth((p_KeyFrameTime.front()-p_KeyFrameTime.back())/number_of_Steps)
	{}
};

/**
* This is a adaptive stepwidth solver that uses a estimation of the error
* derived from the richardson extrapolation.
*/
class RichardsonSolver : public SingleStepODESolver
{
	protected:
	
	/**
	* The width of the last integration step.
	*/	
	double LastStepWidth;
	
	/**
	* The maximum error per step.
	*/
	double Precision;
	
	/**
	* The Upper limit of the stepwidth. This is used to keep the solver stable.
	* Unlimitid stepwidth groth can result in unstable behaviour.
	*/
	double MaximalStepWidth;

	/**
	* The Minimal stepwidth. If this gets hit its a sign that the solver is not
	* capable of solveing the problem with the desired minimum error (assuming
	* this parameter is choosen wisely). This is a savety feature to prevent
	* that the solver makes steps very colse to zero and eventually gets stuck
	* on some point.<br>
	* If this limit gets hit it may be helpful to try another scheme.
	*/
	double MinimalStepWidth;
	
	/**
	* Is true when the minimal stepwidth got hit.
	*/
	bool MinimalStepWidthReached = false;
	
	/**
	* This Factor limits the rate at which the stepwidth can get smaller between
	* two steps. Its a savety feature that should gain more stability.
	*/
	double ShrinkRate;

	/**
	* This Factor limits the rate at which the stepwidth can get bigger between
	* two steps. It is a savety feature that should gain more stability.
	*/
	double GrothRate;

	/**
	* The Estimation of the stepwidht in this solver works as follows. One
	* Solution gets approximated useing two stepwidth. Than the first term of
	* the taylor-series that characterizes the error is calculated as a
	* estimated value for the error made in the next step depending on a given
	* stepwidth.\\
	* Substeps is the number of steps that should be made with the smaller
	* stepwidth to get to the solution.
	*/
	int SubSteps;
	

	/**
	* The Estimation of the stepwidht in this solver works as follows. One
	* Solution gets approximated useing two stepwidth. Than the first term of
	* the taylor-series that characterizes the error is calculated as a
	* estimated value for the error made in the next step depending on a given
	* stepwidth.\\
	*/
	double stepWidth() override
	{
		//Calculating Error
		arma::Col<double> SolutionLargeH = Scheme->step(CurrentTime,CurrentValue,Problem,LastStepWidth);
		
		double SmallerH = LastStepWidth/SubSteps;
		arma::Col<double> SolutionSmallerH = CurrentValue;
		double TimeSmallerH = CurrentTime;

		for(int i = 0;i<SubSteps; i++)
		{
			SolutionSmallerH =
			Scheme->step(TimeSmallerH,SolutionSmallerH,Problem,SmallerH);
			TimeSmallerH += SmallerH;	
		}
		

		arma::Col<double> deviation = SolutionLargeH-SolutionSmallerH;
		double NormDeviation = sqrt(arma::dot(deviation,deviation));
		int order = Scheme->convergenceOrder();
		double err = pow((Precision*(pow(SubSteps,order)-1))/(NormDeviation),1/order);
		
		//choosing optimal stepwidth	
		double toReturn = err*ShrinkRate;

		if(toReturn > LastStepWidth*GrothRate)
		{
			toReturn = LastStepWidth*GrothRate;
		}
		if(toReturn > MaximalStepWidth)
		{
			toReturn = MaximalStepWidth;
		}
		if(toReturn < MinimalStepWidth)
		{
			MinimalStepWidthReached= true;
			toReturn = MinimalStepWidth;
		}
		if(toReturn > (KeyFrameTime.front() - CurrentTime))
		{
			toReturn = (KeyFrameTime.front() -CurrentTime);
		}

		LastStepWidth = toReturn;

		return toReturn;
	}

	public:
	
	RichardsonSolver(
		std::vector<double> p_KeyFrameTime,
		std::vector<double> p_initialOccupation,
		QuantumSystem* p_problem,
		SingleStepScheme* p_Scheme,
		double p_InitialStep,
		double p_Precision,
		double p_MaximalStepWidth = 1e-1,
		double p_MinimalStepWidth = 1e-6,
		double p_ShrinkRate = 0.9,
		double p_GrothRate = 1.6,
		int p_SubSteps = 2
	):
		SingleStepODESolver(p_KeyFrameTime,p_initialOccupation,p_problem,p_Scheme),
		LastStepWidth(p_InitialStep),
		Precision(p_Precision),
		MaximalStepWidth(p_MaximalStepWidth),
		MinimalStepWidth(p_MinimalStepWidth),
		ShrinkRate(p_ShrinkRate),
		GrothRate(p_GrothRate),
		SubSteps(p_SubSteps)
	{}
};
