#pragma once

#include <mutex>
#include <queue>
#include <condition_variable>
#include <future>
#include <vector>

//указатель на функцию, которая является эталоном для функций задач.
using func_type = void(*)(std::vector<int>&, long, long);
typedef std::packaged_task<void()> task_type;
typedef std::future<void> res_type;

template<class T> class BlockedQueue {
	std::mutex mLocker_;
	std::queue<T> mTaskQueue_;										//очередь задач
	std::condition_variable mNotifier_;								//уведомитель

public:
	void push(T& item) {
		std::lock_guard<std::mutex> locker(mLocker_);
		mTaskQueue_.push(std::move(item));
        //делаем оповещение, чтобы поток, вызвавший
        //pop проснулся и забрал элемент из очереди
		mNotifier_.notify_one();
	}

	//блокирующий метод получения элемента из очереди
	void pop(T& item) {
		std::unique_lock<std::mutex> locker(mLocker_);
		if (mTaskQueue_.empty())
			mNotifier_.wait(locker, [this] {return !mTaskQueue_.empty(); });	//ждем, пока вызовут push
		item = std::move(mTaskQueue_.front());
		mTaskQueue_.pop();
	}

	//неблокирующий метод получения элемента из очереди
    //возвращает false, если очередь пуста
	bool fastPop(T& item) {
		std::lock_guard<std::mutex> locker(mLocker_);
		if (mTaskQueue_.empty())
			return false;
		item = std::move(mTaskQueue_.front());						//забираем элемент
		mTaskQueue_.pop();
		return true;
	}
};

//пул потоков
class ThreadPool {	
	long mThreadCount_ = 0;											//количество потоков	
	long mIndex_ = 0;												//для равномерного распределения задач
	std::vector<std::thread> mThreads_;								//вектор потоков
	std::vector<BlockedQueue<task_type>> mThreadQueues_;			//очереди задач для потоков

public:
	ThreadPool();
	~ThreadPool() = default;

	//запуск пула
	void start();
	//останавка пула
	void stop();
	//добавление задачи в очередь задач
	res_type pushTask(func_type f, std::vector<int>& arr, long l, long r);
	//функция потока
	void threadFunc(long qIndex);
};

//обработчик задач 
class RequestHandler {
	ThreadPool threadPool_;											//пул потоков

public:
	RequestHandler();
	~RequestHandler();

	//проброска задачи на выполнение
	res_type pushRequest(func_type f, std::vector<int>& arr, long l, long r);
};