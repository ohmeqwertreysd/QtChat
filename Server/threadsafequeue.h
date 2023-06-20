
#ifndef THREADSAFEQUEUE_H
#define THREADSAFEQUEUE_H

#include <QQueue>
#include <QMutex>
#include <QMutexLocker>
#include <QWaitCondition>

template<class T>
class ThreadSafeQueue
{
public:
    ThreadSafeQueue()
    {}
    void enqueue(T newValue);
    bool tryDequeue(T& value);
    void waitAndDequeue(T& value);
    bool empty() const;
private:
    QQueue<T> dataQueue;
    mutable QMutex dataMutex;
    QWaitCondition dataCond;
};

template<class T>
void ThreadSafeQueue<T>::enqueue(T newValue)
{
    QMutexLocker locker(&dataMutex);
    dataQueue.enqueue(newValue);
    dataCond.wakeAll();
}

template<class T>
bool ThreadSafeQueue<T>::tryDequeue(T& value)
{
    QMutexLocker locker(&dataMutex);
    if(dataQueue.empty())
    {
        return false;
    }
    value = dataQueue.dequeue();
    return true;
}

template<class T>
void ThreadSafeQueue<T>::waitAndDequeue(T& value)
{
    QMutexLocker locker(&dataMutex);
    dataCond.wait(&locker);
    value = dataQueue.dequeue();
}

template<class T>
bool ThreadSafeQueue<T>::empty() const
{
    QMutexLocker locker(&dataMutex);
    return dataQueue.empty();
}

#endif // THREADSAFEQUEUE_H
