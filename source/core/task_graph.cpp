/**
\file   task_graph.cpp
\author Andrew Baxter
\date February 18, 2016

Controls the scheduling and execution of per-frame behaviors

\todo Finish

*/

#include "core/task_graph.h"
//#include "core/traits.h"

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