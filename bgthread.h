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

    NGrepInfo::TResult GetResult() const;

    void SetTask(NGrepInfo::TOptions options);

    void Stop();

    ~BgThread();

signals:
    void ProgressChanged(); // integer percentage changed

    void ResultChanged(); // new item found

private:
    void FindWork(QString filePath, NGrepInfo::TOptions copyOptions);

    void Refresh(); // invoke both signals

    void SignalProgress();

    void SignalResult();

    void QueueSignalProgress();

    void QueueSignalResult();

private:
    mutable std::mutex Mutex;

    NGrepInfo::TOptions Options;
    NGrepInfo::TResult CurResult; // shared_ptr with locking setters getters?

    bool NewTask; // have new grep task
    bool Quit; // delete bgthread
    bool CallProgress; // queue progress signal
    bool CallResult; // queue result signal

    size_t ProgressModulo; // criteria for progress signal

    std::atomic<bool> Cancel; // stop grep task
    std::condition_variable CV;
    std::thread Thread;
};

#endif // BGTHREAD_H
