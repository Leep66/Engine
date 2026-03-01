#pragma once
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <vector>
#include <functional>
#include <atomic>
#include <cstdint>
#include <iostream>

struct Job 
{
	virtual ~Job() = default;
	virtual void Execute() = 0;
};


struct JobSystemConfig 
{
	uint32_t m_workerCount = 0;
	uint32_t m_fileIOWorkerCount = 0;
	uint32_t m_maxExecuting = 0;
};

class JobSystem {
public:

	JobSystem(const JobSystemConfig& cfg);

	void Startup();

	void Shutdown();

	~JobSystem() { Shutdown(); }

	void Enqueue(Job* j);

	void RetrieveCompleted(std::vector<Job*>& out, size_t maxCount = 0);

	void CancelPendingJobs();
	void CancelAllJobs();

private:
	void WorkerLoop();
	
private:
	JobSystemConfig m_config;
	std::vector<std::thread> m_workers;

	std::vector<Job*> m_pending;  
	std::vector<Job*> m_executing;
	std::vector<Job*> m_completed;

	std::mutex              m_mutex;
	std::condition_variable m_cv;
	std::atomic<bool>       m_running;
	size_t                  m_maxExecuting;
};
