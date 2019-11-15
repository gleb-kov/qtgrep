#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->pushButton->setEnabled(false);
    ui->pushButton_2->setEnabled(false);
    ui->progressBar->setVisible(false);
    ui->label->setVisible(false);

    std::function<void()> onLinesButtonEnabler = [this]
    {
        if (ui->lineEdit->text().isEmpty() ||
            ui->lineEdit_2->text().isEmpty())
        {
            ui->pushButton->setEnabled(false);
        }

        if (!ui->lineEdit->text().isEmpty() &&
            !ui->lineEdit_2->text().isEmpty() &&
            !ui->pushButton_2->isEnabled())
        {
            ui->pushButton->setEnabled(true);
        }
    };

    std::function<void()> onFinishButtonEnabler = [this]
    {
        ui->pushButton_2->setEnabled(false);
        if (!ui->lineEdit->text().isEmpty() &&
            !ui->lineEdit_2->text().isEmpty() &&
            !ui->pushButton_2->isEnabled())
        {
            ui->pushButton->setEnabled(true);
        }
    };

    std::function<void(size_t, size_t)> updateProgress = [this](size_t val, size_t total)
    {
        ui->progressBar->setValue(total > 0 ? val * 100 / total : 100);
    };

    connect(ui->lineEdit, &QLineEdit::textChanged, this, onLinesButtonEnabler);
    connect(ui->lineEdit_2, &QLineEdit::textChanged, this, onLinesButtonEnabler);

    connect(ui->lineEdit, &QLineEdit::textChanged, this, [this]
    {
        QString path = ui->lineEdit->text();
        QPalette *palette = new QPalette();
        if (QDir(path).exists() || QFile(path).exists()) {
            palette->setColor(QPalette::Text, Qt::black);
        } else {
            palette->setColor(QPalette::Text, Qt::red);
        }
        ui->lineEdit->setPalette(*palette);
    });

    connect(ui->pushButton, &QPushButton::clicked, this, [this]
    {
        QString path = ui->lineEdit->text();
        QString text = ui->lineEdit_2->text();

        NGrepInfo::TOptions opt(path, text, false, false, false, MAXSHOW);
        opt.Lowcase = ui->checkBox->isChecked();
        opt.Near = ui->checkBox_2->isChecked();
        opt.One = ui->checkBox_3->isChecked();

        bg_thread.SetTask(opt);

        ui->listWidget->clear();
        ui->progressBar->setVisible(true);
        ui->label->setVisible(true);

        ui->pushButton->setEnabled(false);
        ui->pushButton_2->setEnabled(true);
    });

    connect(ui->pushButton_2, &QPushButton::clicked, this, [this, onFinishButtonEnabler]
    {
        bg_thread.Stop();
        onFinishButtonEnabler();
    });

    connect(&bg_thread, &BgThread::ProgressChanged, this, [this, updateProgress]
    {
        NGrepInfo::TResult res = bg_thread.GetResult();
        updateProgress(res.Checked(), res.TotalFiles());
    });

    connect(&bg_thread, &BgThread::ResultChanged, this, [this, onFinishButtonEnabler]
    {
        NGrepInfo::TResult res = bg_thread.GetResult();

        for (size_t i = ui->listWidget->count(); i < res.Shown(); i++) {
            QString text;
            text = res.Filename[i];
            text += QString(":%1").arg(res.Lines[i]);
            text += QString(":%1").arg(res.Index[i]);
            if (!res.Near[i].isEmpty()) {
                text += ':' + CutWidth(res.Near[i], res.Index[i]) + '\n';
            }
            ui->listWidget->addItem(text);
        }

        if (res.Complete) {
            onFinishButtonEnabler();
        }

        ui->label->setText(QString("Total: %1").arg(res.TotalItems()));
    });
}

MainWindow::~MainWindow() {
    delete ui;
}

QString MainWindow::CutWidth(QString &line, size_t pos) {
    size_t half = PHRASEBOUND / 2;
    size_t start = (pos > half ? pos - half : 0);

    // okay with length and remove \tnvfr characters
    return line.mid(start, PHRASEBOUND).trimmed();
}
