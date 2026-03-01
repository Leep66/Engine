#include "Engine/Core/JobSystem.hpp"
#include <algorithm>
#include <cassert>

JobSystem::JobSystem(const JobSystemConfig& cfg)
	: m_running(false)
	, m_maxExecuting(1)
	, m_config(cfg)
{

}

void JobSystem::Startup()
{
	if (m_running.load()) return;

	m_running.store(true);

	uint32_t hc = std::max(1u, std::thread::hardware_concurrency());
	uint32_t workerCount = m_config.m_workerCount ? m_config.m_workerCount
		: (hc > 1 ? hc - 1 : 1);

	m_maxExecuting = (m_config.m_maxExecuting != 0) ? m_config.m_maxExecuting
		: static_cast<size_t>(workerCount);

	m_workers.reserve(workerCount);
	for (uint32_t i = 0; i < workerCount; ++i) 
	{
		m_workers.emplace_back(&JobSystem::WorkerLoop, this);
	}
}

void JobSystem::Enqueue(Job* j)
{
	assert(j != nullptr);
	{
		std::scoped_lock<std::mutex> g(m_mutex);
		m_pending.push_back(j);
	}
	m_cv.notify_one();
}

void JobSystem::RetrieveCompleted(std::vector<Job*>& out, size_t maxCount)
{
	std::scoped_lock<std::mutex> g(m_mutex);

	const size_t nAvail = m_completed.size();
	const size_t n = (maxCount == 0) ? nAvail : std::min(maxCount, nAvail);

	out.reserve(out.size() + n);
	for (size_t i = 0; i < n; ++i) 
	{
		out.push_back(m_completed[i]);
	}
	if (n > 0) 
	{
		m_completed.erase(m_completed.begin(), m_completed.begin() + static_cast<long>(n));
		m_cv.notify_all();
	}
}

void JobSystem::WorkerLoop()
{
	while(true)
	{
		Job* job = nullptr;

		{
			std::unique_lock<std::mutex> lk(m_mutex);
			m_cv.wait(lk, [this] {
				return !m_running.load() || !m_pending.empty();
				});

			if (!m_running.load() && m_pending.empty() && m_executing.empty()) 
			{
				return;
			}

			if (!m_pending.empty() && m_executing.size() < m_maxExecuting) 
			{
				job = m_pending.back();
				m_pending.pop_back();
				m_executing.push_back(job);
			}
			else {
				continue;
			}
		}

		if (job) 
		{
			job->Execute();
		}

		{
			std::scoped_lock<std::mutex> g(m_mutex);
			auto it = std::find(m_executing.begin(), m_executing.end(), job);
			if (it != m_executing.end()) {
				m_executing.erase(it);
			}
			m_completed.push_back(job);
		}

		m_cv.notify_all();
	}
}

void JobSystem::Shutdown()
{
	if (!m_running.load()) return;

	{
		std::scoped_lock<std::mutex> g(m_mutex);
		m_running.store(false);
	}
	m_cv.notify_all();

	for (auto& t : m_workers) 
	{
		if (t.joinable()) t.join();
	}
	m_workers.clear();

	{
		std::scoped_lock<std::mutex> g(m_mutex);
		for (Job* j : m_pending)   delete j;
		for (Job* j : m_executing) delete j; 
		for (Job* j : m_completed) delete j;

		m_pending.clear();
		m_executing.clear();
		m_completed.clear();
	}
}


void JobSystem::CancelPendingJobs()
{
	std::scoped_lock lock(m_mutex);

	for (Job* job : m_pending) {
		delete job;
	}
	m_pending.clear();

	m_cv.notify_all();
}

void JobSystem::CancelAllJobs()
{
	std::scoped_lock lock(m_mutex);

	for (Job* job : m_pending) {
		delete job;
	}
	m_pending.clear();

	for (Job* job : m_completed) {
		delete job;
	}
	m_completed.clear();

	m_cv.notify_all();
}
