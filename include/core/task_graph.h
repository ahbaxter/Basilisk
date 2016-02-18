/**
\file task_graph.h
\author Andrew Baxter
\date February 17, 2016

Distributes tasks among an arbitrary number of persistent threads

*/



#include "../common.h"
#include <thread>
#include <future>
#include <mutex>


//namespace Basilisk
//{
//
//	/**
//	\brief Schedules and executes tasks among a set number of threads
//
//	Cannot be exported to a DLL since members of the `std` namespace have to be passed as parameters
//	*/
//	class TaskGraph
//	{
//	public:
//		TaskGraph(size_t numThreads);
//		~TaskGraph();
//
//		/**
//		\brief Add a task to the graph
//		Tasks are not sorted until `Execute()` is called
//
//		\template ResultType The return type of the provided task
//		\param[in]task The task to complete
//
//		\return A `std::future`
//		*/
//		template<typename ResultType>
//			const std::future<ResultType>&
//			Add(std::packaged_task<ResultType(void)> &task);
//
//		void Add(std::packaged_task<void(void)> &task);
//
//		/**
//		\brief Launch the threads
//		If they are already launched, nothing happens
//
//		*/
//		void Launch();
//		/**
//
//		*/
//		bool Clear();
//		/**
//
//		*/
//		bool Reset();
//		/**
//		Wait for all worker threads to finish
//		*/
//		void Join();
//	private:
//		void WorkerFunction();
//		bool SortTasks();
//		bool StartWorkers();
//
//
//		std::vector< std::packaged_task<void(void)>& > m_tasks;
//		std::mutex m_tasksLock;
//		std::atomic<bool> m_terminate;
//
//		std::thread *m_workers;
//	};
//}