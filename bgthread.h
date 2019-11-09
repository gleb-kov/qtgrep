#ifndef BGTHREAD_H
#define BGTHREAD_H

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QObject>

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>

#include "grepinfo.h"

struct BgThread : QObject {
private:
    Q_OBJECT

public:
    BgThread();

    void SetTask(NGrepInfo::TOptions options);

    NGrepInfo::TResult GetResult() const;

    void Stop();

    ~BgThread();

signals:
    void ResultChanged();

private:
    void FindWork(QString filePath, NGrepInfo::TOptions copyOptions);

    void QueueCallback();

    void Callback();

private:
    mutable std::mutex Mutex;

    NGrepInfo::TOptions Options;
    NGrepInfo::TResult CurResult;

    bool NewTask; // have new grep task
    bool CallQueue; // result changed
    bool Quit; // delete bgthread

    std::atomic<bool> Cancel; // stop grep task
    std::condition_variable CV;
    std::thread Thread;
};

#endif // BGTHREAD_H
