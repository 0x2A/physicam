#pragma once



class AsyncGLWorker
{
	friend class Renderer;

protected:
	void AddToGLQueue(int mode, void* params = nullptr);

	virtual void DoAsyncWork(int mode, void *params) = 0;

	static Renderer* renderer;
};


struct AsyncGLWork_t
{
	AsyncGLWorker* worker;
	int mode;
	void *params;
};