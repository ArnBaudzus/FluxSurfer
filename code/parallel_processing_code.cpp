
#include <queue>
#include <thread>
#include <chrono>
#include <mutex>

/**
* This is the Mutex that is used to coordinate the access on shared memory used
* by the workerthreads and the coordination thread.
*/
std::mutex commonMutex;

/**
* Systems that are not simulated and their corresponding solvers are stored in
* this queue to be processed by the workerthreads.
*/
std::queue<std::pair<QuantumSystem*,Solver*>> WorkSource;

/**
* Multithreading in this program works as follows: The Thread that was created
* to run the main method manages the work that is done by any number of worker
* threads. The coordination of the work is done using shared memory. The Shared
* memory consists of a queue that contains the quantumsystems that should be
* simulated and the solver that should be used and a boolean that indicates if
* there is more work to come or not. The workerthreads source work from the
* queue if the queue is not empty. If it is empty the workerthreads wait until
* the boolean that indicates whether there is work to come or not turns false.
* The workerthreads process the system and save the data as .graphml. Additional
* information is saved in a METADATA.csv file.
* This Boolean indicates if the system should continue to run and wait for work
* or if it should stop.
*/	
bool Running=true;

/**
* This method provides access to the Running variable.
*/
bool shouldContinue()
{
	std::lock_guard<std::mutex> guard(commonMutex);
	return Running;
}

/**
* This method returns the number of pending jobs stored in WorkSource.
*/
int jobsInPendingQueue()
{
	std::lock_guard<std::mutex> guard(commonMutex);
	return WorkSource.size(); 
}

/**
* This Method asks if there are unprocessed systems in the WorkSource queue. It
* is used by the Workerthreads to source new Work.
*/
bool thereIsJob()
{
	std::lock_guard<std::mutex> guard(commonMutex);
	return !WorkSource.empty(); 
}

/**
* This Mehthod sets the variable that tells the workerthreads that there is
* more work to come to true. It is used to start the workerthreads. This is only
* important, when one program is running multible experiments.
*/
void startWork()
{
	std::lock_guard<std::mutex> guard(commonMutex);
	Running = true;
}

/**
* This Mehthode sets the variable that tells the workerthreads that there is
* more work to come to false. It is used to stop the workerthreads so that they
* can be joined with the management thread.
*/
void stopWork()
{
	std::lock_guard<std::mutex> guard(commonMutex);
	Running = false;
}

/**
* This Method reads the Running boolean. It is used by the Workerthreads.
*/
bool isRunning()
{
	std::lock_guard<std::mutex> guard(commonMutex);
	return Running;
}

/**
* This Method appends a new job to the back of the WorkSource queue. It is used
* by the management-thread to distribute new work.
*/
void pushJob(std::pair<QuantumSystem*,Solver*> job)
{
	std::lock_guard<std::mutex> guard(commonMutex);
	WorkSource.push(job);
}

/**
* This Method pops one job from the worksource queue and return it. It is used
* by the workerthreads to source new work.
*/
std::pair<QuantumSystem*,Solver*> getJob()
{
	std::lock_guard<std::mutex> guard(commonMutex);
	std::pair<QuantumSystem*,Solver*> toReturn = WorkSource.front();
	WorkSource.pop();
		
	return toReturn;
}

/**
* This Method is executed by the workerthreads and defines their behaviour. 
*/
void run(std::string test)
{
	
	while(isRunning())
	{
		while(thereIsJob())
		{
			QuantumSystem* problem;
			Solver* s;
	
			std::tie(problem,s) = getJob();
					
			s->solve();
				
			delete s;
			problem->writeToFile();
			delete problem;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1));	
	}
}

/**
* The Experiment class defines a set of systems that should be simulated. The
* idea behind the name is, that one Simulation is similar to a measurement in a
* experiment and the experiment is a set of measurements with corresponding
* metadata.//
* This is formalized here because the experiment class is used by the program to
* manage multithreading and the storage of the data. It can also be used for the
* programmer to organize the workflow. //
* The Programmer needs to override this class to make use of it.
*/	
class Experiment
{
	protected:
	
	/**
	* This Class represents the Metadata associated with this experiment.
	*/
	class MetaDataSet
	{
		protected:
		
		/**
		* The description of the experiment. Includes hints for you or other
		* people.
		*/
		std::string Description;
		
		/**
		* Motivation for this experiment.
		*/
		std::string Motivation;

		/**
		* In every experiment there are a number of constant physical parameters
		* that don't change during all measurements. The Name of these
		* Parameters and their value are stored here. They are written in the
		* metadata file.
		*/
		std::vector<std::pair<std::string,std::string>> ConstantParameters;
		
		/**
		* The folder where all the data generated by the experiment and the
		* Metadata should be saved.
		*/
		std::string ProjectFolder;
		
		/**
		* In every experiment there are parameters that are constant during one
		* measurement but change from measurement to measurement. The Name of
		* the changing parameters are saved here.//
		* In the Metadata file is a table of these parameters, their values and
		* the file for the experimental data associated with these values.
		*/
		std::vector<std::string> AttributeNames;
		
		/**
		* Each entry of these Vector represents a column in the table of
		* measurements in the metadata file. The first entrie in the pair
		* should be the file where the simulation data for a experiment is
		* stored. The vector in the second entrie of the pair holds the values
		* of the parameters specified in attributes.
		*/
		std::vector<std::pair<std::string,std::vector<std::string>>> Column;
		
		/**
		* Writes the metadata file for this experiment.
		*/
		void writeRecordData()
		{
			char delimiter = '\t';
			char newLine = '\n';

			std::ofstream file(ProjectFolder+"/METADATA.csv");

			file << "RecordFile" << delimiter;
			for(std::string& s : AttributeNames)
			{
				file << s << delimiter;
			}
			file << newLine;

			for(auto& p : Column)
			{
				file << p.first << delimiter;

				for(std::string& s: p.second)
				{
					file << s << delimiter;
				}

				file << newLine;
			}
		
			file.close();
		}
		
		void writeHumanReadableData()
		{
			Json::Value toSave;

			Json::Value Constants;
			for(auto p : ConstantParameters)
			{
				Constants[p.first] = p.second;
			}

			toSave["Motivation"] = Motivation;
			toSave["Description"] = Description;
			toSave["SystemConstants"] = Constants;
			
			std::ofstream file(ProjectFolder+"/METADATA.json");
			file << toSave;
			file.close();
		}

		public:
		
		MetaDataSet
		(
			std::string pProjectFolder,
			std::vector<std::string> pAttributeNames,
			std::vector<std::pair<std::string,std::string>> constantParameters,
			std::string description,
			std::string motivation
		):
		ProjectFolder(pProjectFolder),
		AttributeNames(pAttributeNames),
		ConstantParameters(constantParameters),
		Description(description),
		Motivation(motivation)
		{}
		
		/**
		* Adds one entire to the column of records.
		*/
		void logRecord(std::string PathToRecord, std::vector<std::string> attributeValues)
		{
			if(AttributeNames.size() != attributeValues.size())
			{
				throw std::runtime_error("Number of Attributes in metadata logging don't match.");
			}
			Column.push_back(std::make_pair(PathToRecord,attributeValues));
		}
		
		/**
		* Saves the metadata in a .csv file in the project folder.
		*/
		void writeToFiles()
		{
			
			writeRecordData();
			writeHumanReadableData();
		}
		
		std::string projectFolder()
		{
			return ProjectFolder;
		}
	};
	
	/**
	* The number of systems that should be simulated during the coures of this
	* experiment.
	*/
	int InitialMeasurementCount;
	
	/**
	* The number of workerthreads that work parallel on the simulations in this
	* experiment.
	*/
	int WorkerCount;
	
	/**
	* The Metadatafile of this experiment.
	*/
	MetaDataSet MetaData;
	
	/**
	* The Workerthreads.
	*/
	std::vector<std::thread> Workers;
	
	/**
	* This method must be defined by the user. It returns a measurement that
	* should be simulated and the constant parameters in this simulation.
	* In every experiment there are physical quantities that stay constant
	* during the whole experiment and there are quantities that change from
	* measurement to measurement. This Method returns the system that defines
	* the measurement and its solver and the quantities that are constant for
	* this measurement but change from measurement to measurement.
	*/
	virtual std::pair<
				std::vector<std::string>,
				std::pair<QuantumSystem*,Solver*>
				>	NextMeasurement()=0;
	
	/**
	* This method returns the number of measurements that is not saved in the
	* WorkSource queue or already processed. It is used in conjunction with
	* InitialMeasurementCount to log the progress of the simulation to the terminal.
	*/
	virtual int MeasurementsToCome()=0;
	
	/**
	* This method refills the WorkSource queue if the number of jobs there,
	* available for the workerthreads is running low.
	*/
	void managePendingJobs()
	{
		if(jobsInPendingQueue() < WorkerCount * 3 && MeasurementsToCome() != 0)
		{
			while(jobsInPendingQueue() < WorkerCount * 5)
			{
				auto attributesNjob = NextMeasurement();
				
				MetaData.logRecord(attributesNjob.second.first->pathToSave(),attributesNjob.first);
				pushJob(attributesNjob.second);

				if(MeasurementsToCome() <= 0)
				{
					break;
				}
			}
		}
	}

	/**
	* Logs the progress in the simulation to the terminal.
	*/
	void logStatusToTerminal(int jobsPending)
	{
		std::string statusLine = "Simulation Running on "+std::to_string(WorkerCount)+" Threads. ";
		std::string jobLine = std::to_string(jobsPending) +" Jobs Pending. ";
		
		std::cout << "\r" <<
		statusLine <<
		jobLine;
	}

	/**
	* This Method is used by the management thread and defines its behaviour. It
	* is used parallel to the workerthreads.
	*/
	void WaitForCalculation()
	{
		int jobsPending = jobsInPendingQueue() + MeasurementsToCome();
		
		int loopCount = 0;

		while(jobsPending > 0)
		{
			managePendingJobs();
			
			loopCount ++;

			if(loopCount >= 100)
			{
				logStatusToTerminal(jobsPending);
				loopCount = 0;
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(1));	
			jobsPending = jobsInPendingQueue() + MeasurementsToCome();
		}
	}

	public:
	
	Experiment
	(
		int pWorkerCount,
		int pInitialMeasurementCount,
		std::string projectFolder,
		std::vector<std::string> attributeNames,
		std::vector<std::pair<std::string,std::string>> constantParameters,
		std::string description,
		std::string motivation
	)
	:WorkerCount(pWorkerCount),
	 InitialMeasurementCount(pInitialMeasurementCount),
	 MetaData(projectFolder,attributeNames,constantParameters,description,motivation)
	{}
	
	/**
	* Starts the parallel calculation of the simulations that make up this
	* experiment. 
	*/
	void Conduct()
	{
		startWork();
		
		for(int i =0; i< WorkerCount;i++)
		{
			std::string tIden = "T"+std::to_string(i+1);
			Workers.push_back(std::thread(run,tIden));
		}
		

		WaitForCalculation();
		
		stopWork();

		for(std::thread& t : Workers)
		{
			t.join();
		}
		
		std::cout << std::endl;

		MetaData.writeToFiles();
	}
};


