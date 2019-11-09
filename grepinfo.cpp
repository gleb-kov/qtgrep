#include "grepinfo.h"

NGrepInfo::TResult::TResult()
    : Complete(true)
    , Num(0)
{}

size_t NGrepInfo::TResult::Size() {
    return Num;
}

void NGrepInfo::TResult::Append(QString &name, size_t numLine, size_t numIndex,
                                QString const phrase) {
    Num++;
    Filename.push_back(name);
    Lines.push_back(numLine);
    Index.push_back(numIndex);
    Near.push_back(phrase);
}

void NGrepInfo::TResult::Increase() {
    Num++;
}

void NGrepInfo::TResult::Clear() {
    Complete = true;
    Num = 0;
    Filename.clear();
    Lines.clear();
    Index.clear();
    Near.clear();
}

NGrepInfo::TOptions::TOptions() {
    Lowcase = false;
    Near = false;
    One  = false;
    ResultSize = std::numeric_limits<size_t>::max();
}

NGrepInfo::TOptions::TOptions(QString path, QString substring,
                              bool lowcase, bool near, bool one, size_t resultsize)
    : Path(path)
    , Substring(substring)
    , Lowcase(lowcase)
    , Near(near)
    , One(one)
    , ResultSize(resultsize)
{}
