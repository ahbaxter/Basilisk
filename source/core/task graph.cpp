//#include "../include/core/task graph.h"
//#include "../include/core/traits.h"
//
//using namespace Basilisk;
//
//TaskGraph::TaskGraph(size_t numThreads)
//{
//	m_workers = new std::thread[numThreads];
//}
//
//TaskGraph::~TaskGraph()
//{
//	Join();
//	delete[] m_workers;
//}
//
//
//template<typename ResultType>
//void TaskGraph::Add(std::packaged_task<ResultType(void)> task)
//{
//	
//}
//
//void TaskGraph::Add(std::packaged_task< void(void) > task)
//{
//	
//}
//
//bool TaskGraph::Execute()
//{
//	return (ScheduleTasks() && StartWorkers());
//}