#include "OptimizeThreadPool.h"

ThreadPool::ThreadPool() : mThreadCount_(std::thread::hardware_concurrency()
	!= 0 ? std::thread::hardware_concurrency() : 4),
	mThreadQueues_(mThreadCount_) {}

//запуск пула
void ThreadPool::start() {
	for (auto i = 0; i < mThreadCount_; ++i)
		mThreads_.emplace_back(&ThreadPool::threadFunc, this, i);
}

//останавка пула
void ThreadPool::stop() {
	for (auto i = 0; i < mThreadCount_; ++i) 	{
		task_type emptyTask;
		mThreadQueues_[i].push(emptyTask);
	}
	for (auto& th : mThreads_)
		if (th.joinable())
			th.join();
}

//добавление задачи в очередь задач
res_type ThreadPool::pushTask(func_type f, std::vector<int>& arr, long l, long r) {
	int queue_to_push = mIndex_++ % mThreadCount_;
	task_type task([=, &arr] {f(arr, l, r); });
	auto res = task.get_future();
	mThreadQueues_[queue_to_push].push(task);
	return res;
}

//функция потока 
void ThreadPool::threadFunc(long qIndex) {
	while (true) {													//обработка очередной задачи
		task_type task_to_do;
		bool isGotTask = false;
		auto i = 0;

		for (; i < mThreadCount_; ++i)
			//попытка быстро забрать задачу из любой очереди, начиная со своей
			if (isGotTask = mThreadQueues_[(static_cast<ptrdiff_t>(qIndex) + i) %
				mThreadCount_].fastPop(task_to_do))
				break;

		if (!isGotTask)
			mThreadQueues_[qIndex].pop(task_to_do);					//вызов блокирующего получения очереди
		else if (!task_to_do.valid())
			//чтобы не допустить зависания потока кладем обратно
			mThreadQueues_[(static_cast<ptrdiff_t>(qIndex) + i) % mThreadCount_].push(task_to_do);

		if (!task_to_do.valid())
			return;

		task_to_do();												//выполнение задачи
	}
}

RequestHandler::RequestHandler() {
	threadPool_.start();
}

RequestHandler::~RequestHandler() {
	threadPool_.stop();
}

//проброска задачи на выполнение
res_type RequestHandler::pushRequest(func_type f, std::vector<int>& arr, long l, long r) {
	return threadPool_.pushTask(f, arr, l, r);
}