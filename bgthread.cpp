#include "bgthread.h"

BgThread::BgThread()
    : NewTask(false)
    , Quit(false)
    , CallProgress(false)
    , CallResult(false)
    , ProgressModulo(0)
    , Cancel(false)
    , Thread([this] {
        while (true) {
            std::unique_lock<std::mutex> lg(Mutex);
            CV.wait(lg, [this]
            {
                return Quit || NewTask;
            });

            if (Quit) {
                break;
            }

            NGrepInfo::TOptions opt = Options;
            CurResult.Clear();
            CurResult.Complete = false;
            NewTask = false;

            if (QDir(opt.Path).exists()) {
                QDirIterator it(opt.Path, QDir::Files,
                                QDirIterator::Subdirectories);
                while (it.hasNext()) {
                    it.next();
                    CurResult.FilesNumber++;
                }
            } else if (QFile(opt.Path).exists()) {
                CurResult.FilesNumber = 1;
            }

            Refresh();
            lg.unlock();

            if (QDir(opt.Path).exists()) {
                QDirIterator it(opt.Path, QDir::Files,
                                QDirIterator::Subdirectories);
                while (it.hasNext()) {
                    FindWork(it.next(), opt);
                    if (Cancel.load()) {
                        break;
                    }
                }
            } else if (QFile(opt.Path).exists()) {
                FindWork(opt.Path, opt);
            }

            lg.lock();

            CurResult.Complete = true;
            Refresh();
            Cancel.store(false);
        }
    })
{}

NGrepInfo::TResult BgThread::GetResult() const {
    std::unique_lock<std::mutex> lg(Mutex);
    return CurResult;
}

void BgThread::SetTask(NGrepInfo::TOptions options) {
    std::unique_lock<std::mutex> lg(Mutex);
    this->Options = options;
    NewTask = true;
    if (options.Lowcase) {
        this->Options.Substring = this->Options.Substring.toLower();
    }
    CV.notify_all();
}

void BgThread::Stop() {
    std::unique_lock<std::mutex> lg(Mutex);
    Cancel.store(true);
}

BgThread::~BgThread() {
    Cancel.store(true);
    {
        std::unique_lock<std::mutex> lg(Mutex);
        Quit = true;
        CV.notify_all();
    }
    Thread.join();
}

void BgThread::FindWork(QString filePath, NGrepInfo::TOptions copyOptions) {
    QFile file(filePath);

    if (file.exists() &&
        file.open(QFile::ReadOnly | QFile::Text) &&
        !QFileInfo(file).isExecutable())
    {
        size_t lineNumber = 1;
        bool found = false;

        while (!file.atEnd() && !found) {
            if (Cancel.load()) {
                return;
            }
            QByteArray line = file.readLine();

            int position = -1;
            if (copyOptions.Lowcase) {
                position = line.toLower().indexOf(copyOptions.Substring);
            } else {
                position = line.indexOf(copyOptions.Substring);
            }

            if (position != -1) {
                if (copyOptions.One) {
                    found = true;
                }

                {
                    std::unique_lock<std::mutex> lg(Mutex);
                    if (copyOptions.ResultSize > CurResult.Shown()) {
                        CurResult.Append(filePath, lineNumber, position,
                                        (copyOptions.Near ? line : ""));
                    } else {
                        CurResult.Increase();
                    }
                    QueueSignalResult();
                }
            }
            lineNumber++;
        }
    }

    {
        std::unique_lock<std::mutex> lg(Mutex);
        CurResult.Progress++;
        ProgressModulo += 100;
        if (ProgressModulo >= CurResult.FilesNumber) {
            ProgressModulo %= CurResult.FilesNumber;
            QueueSignalProgress();
        }
    }
}

void BgThread::Refresh() {
    QueueSignalProgress();
    QueueSignalResult();
}

void BgThread::SignalProgress() {
    {
        std::unique_lock<std::mutex> lg(Mutex);
        CallProgress = false;
    }

    emit ProgressChanged();
}

void BgThread::SignalResult() {
    {
        std::unique_lock<std::mutex> lg(Mutex);
        CallResult = false;
    }

    emit ResultChanged();
}

void BgThread::QueueSignalProgress() {
    if (CallProgress) return;

    CallProgress = true;
    QMetaObject::invokeMethod(this, &BgThread::SignalProgress,
                              Qt::QueuedConnection);
}

void BgThread::QueueSignalResult() {
    if (CallResult) return;

    CallResult = true;
    QMetaObject::invokeMethod(this, &BgThread::SignalResult,
                              Qt::QueuedConnection);
}
