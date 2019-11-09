#include "grepinfo.h"

NGrepInfo::TResult::TResult()
    : Complete(true)
    , Items(0)
    , Progress(0)
    , FilesNumber(0)
{}

size_t NGrepInfo::TResult::Shown() {
    return Filename.size();
}

size_t NGrepInfo::TResult::Checked() {
    return Progress;
}

size_t NGrepInfo::TResult::TotalFiles() {
    return FilesNumber;
}

size_t NGrepInfo::TResult::TotalItems() {
    return Items;
}

void NGrepInfo::TResult::Append(QString &name, size_t numLine, size_t numIndex,
                                QString const phrase) {
    Items++;
    Filename.push_back(name);
    Lines.push_back(numLine);
    Index.push_back(numIndex);
    Near.push_back(phrase);
}

void NGrepInfo::TResult::Increase() {
    Items++;
}

void NGrepInfo::TResult::Clear() {
    Complete = true;
    Items = 0;
    Progress = 0;
    FilesNumber = 0;
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
