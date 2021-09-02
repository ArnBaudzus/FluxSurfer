#include"job_supervisor_code.cpp"

using namespace std;

class testJob : Job
{
	protected:
	
	int num = 0;
	
	void doStuff() override
	{
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}

	public:
	
	testJob(int jobNum)
	:num(jobNum)
	{}

	void initialize() override
	{
		cout << "Hello Finished " << num << endl;
	}	
};

class testJobGenerator : JobGenerator
{
	protected:
	
	int num_jobs;

	int generated=0;

	public:

	testJobGenerator(int jobs)
	:num_jobs(jobs)
	{}

	Job* nextJob() override
	{
		generated ++;
		return new testJob(generated);
	}

	bool hasJob() override
	{
		return generated <= num_jobs;
	}

	int getJobsPending() override
	{
		return num_jobs - generated;
	}

	bool pendingJobNumberIsExact() override
	{
		return true;
	}
	
};

int main()
{
	cout << "Hi" << endl;
	return 0;
}
