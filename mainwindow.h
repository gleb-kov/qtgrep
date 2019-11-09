#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QScrollBar>
#include <QTimer>

#include <functional>

#include "bgthread.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);

    ~MainWindow();

private:
    QString CutWidth(QString &line, size_t pos);

private:
    const static size_t PHRASEBOUND = 100;
    const static size_t MAXSHOW = 20;

    Ui::MainWindow *ui;
    BgThread bg_thread;
};

#endif // MAINWINDOW_H
