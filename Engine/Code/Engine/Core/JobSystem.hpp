#pragma once


//--------------------------------------------------------------------------------------------------
#include <vector>
#include <thread>
#include <mutex>
#include <queue>


//--------------------------------------------------------------------------------------------------
class JobSystem;


//--------------------------------------------------------------------------------------------------
extern JobSystem* g_theJobSystem;


//--------------------------------------------------------------------------------------------------
enum JobStatus : unsigned char
{
	JOB_STATUS_INVALID = (unsigned char)-1,

	JOB_STATUS_CONSTRUCTED_BUT_NOT_QUEUED = 0,
	JOB_STATUS_QUEUED,
	JOB_STATUS_CLAIMED_AND_EXECUTING,
	JOB_STATUS_COMPLETED,
	JOB_STATUS_RETRIEVED_AND_RETIRED,

	JOB_STATUS_COUNT,
};


//--------------------------------------------------------------------------------------------------
class Job
{
public:
	Job() {};
	virtual ~Job() {};
	virtual void Execute() = 0;

public:
	std::atomic<JobStatus> m_status = JOB_STATUS_CONSTRUCTED_BUT_NOT_QUEUED;
};


//--------------------------------------------------------------------------------------------------
struct JobSystemConfig
{
	int m_numOfWorkerThreads = -1;
};


//--------------------------------------------------------------------------------------------------
class JobWorkerThread
{
public:
	JobWorkerThread(int workerID, JobSystem* jobSystem);
	~JobWorkerThread();

	void ThreadMain();

public:
	std::thread*	m_thread	= nullptr;
	JobSystem*		m_jobSystem = nullptr;
	int				m_workerID	= -1;
};


//--------------------------------------------------------------------------------------------------
class JobSystem
{
	friend class JobWorkerThread;
public:
	JobSystem(JobSystemConfig jobSystemConfig);

	void	Startup();
	void	BeginFrame();
	void	EndFrame();
	void	Shutdown();

	void	QueueNewJob(Job* job);  // Called by main thread to get a Job INTO the system (and give up ownership)
	Job*	RetrieveCompletedJob(); // Called by main thread to get a Job back OUT of the system ( and retake ownership)
	
	void	ClearQueuedJobList();
	void	ClearCompletedJobList();
	void	WaitUntilQueuedJobCompletion();

protected:
	void	CreateNewWorkerThreads(int numWorkerThreads);
	void	DestroyAllWorkers();
	bool	IsQuitting() const;
	Job*	ClaimJob();
	void	ReportCompletedJob(Job* job);

private:
	void	RemoveCompletedJobFromClaimedList(Job* jobToRemove);

private:
	JobSystemConfig					m_config;
	std::atomic<bool>				m_isQuitting = false;
	std::queue<Job*>				m_queuedJobsList;
	std::mutex						m_queuedJobsListMutex;
	std::queue<Job*>				m_completedJobsList;
	std::mutex						m_completedJobsListMutex;
	std::queue<Job*>				m_claimedJobList;
	std::mutex						m_claimedJobListMutex;
	std::vector<JobWorkerThread*>	m_jobWorkerThreads;
	// std::vector<Job*>	m_unclaimedJobsList;
};