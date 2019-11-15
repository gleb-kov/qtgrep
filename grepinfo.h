#ifndef GREPINFO_H
#define GREPINFO_H

#include <QObject>

#include <vector>

namespace NGrepInfo {
struct TResult {
    TResult();

    size_t Shown() const;

    size_t Checked() const;

    size_t TotalFiles() const;

    size_t TotalItems() const;

    void Append(QString &name, size_t numLine, size_t numIndex, QString const phrase);

    void Increase();

    void Clear();

    bool Complete;
    size_t Items;
    size_t Progress;
    size_t FilesNumber;

    std::vector<QString> Filename;
    std::vector<QString> Near;
    std::vector<size_t> Lines;
    std::vector<size_t> Index;
};

struct TOptions {
    TOptions();
    TOptions(QString, QString, bool, bool, bool, size_t);

    QString Path;
    QString Substring;

    bool Lowcase;
    bool Near;
    bool One;

    size_t ResultSize;
};
}

#endif // GREPINFO_H
