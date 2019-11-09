#include "bgthread.h"

BgThread::BgThread()
    : NewTask(false)
    , CallQueue(false)
    , Quit(false)
    , Cancel(false)
    , Thread([this] {
        while(true) {
            std::unique_lock<std::mutex> lg(Mutex);
            CV.wait(lg, [this] {
                return Quit || NewTask;
            });

            if (Quit) {
                break;
            }

            NGrepInfo::TOptions opt = Options;
            CurResult.Clear();
            CurResult.Complete = false;
            NewTask = false;

            QueueCallback();
            lg.unlock();
            if (QDir(opt.Path).exists()) {
                QDirIterator it(opt.Path, QDir::Files,
                                QDirIterator::Subdirectories);
                while (it.hasNext()) {
                    FindWork(it.next(), opt);
                }
            } else if (QFile(opt.Path).exists()) {
                FindWork(opt.Path, opt);
            }
            lg.lock();

            CurResult.Complete = true;
            QueueCallback();
            Cancel.store(false);
        }
    })
{}

void BgThread::SetTask(NGrepInfo::TOptions options) {
    std::unique_lock<std::mutex> lg(Mutex);
    this->Options = options;
    NewTask = true;
    if (options.Lowcase) {
        this->Options.Substring = this->Options.Substring.toLower();
    }
    CV.notify_all();
}

NGrepInfo::TResult BgThread::GetResult() const {
    std::unique_lock<std::mutex> lg(Mutex);
    return CurResult;
}

void BgThread::Stop() {
    std::unique_lock<std::mutex> lg(Mutex); // unique_lock?
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
                    if (copyOptions.ResultSize > CurResult.Size()) {
                        CurResult.Append(filePath, lineNumber, position,
                                        (copyOptions.Near ? line : ""));
                    } else {
                        CurResult.Increase();
                    }
                    QueueCallback();
                }
            }
            lineNumber++;
        }
    }
}

void BgThread::QueueCallback() {
    if (CallQueue) {
        return;
    }

    CallQueue = true;
    QMetaObject::invokeMethod(this, &BgThread::Callback,
                              Qt::QueuedConnection);
}

void BgThread::Callback() {
    {
        std::unique_lock<std::mutex> lg(Mutex);
        CallQueue = false;
    }

    emit ResultChanged();
}
