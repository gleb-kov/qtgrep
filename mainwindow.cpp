#include "mainwindow.h"
#include "ui_mainwindow.h"

/*
 * TODO:
 * const, &, &&, emplace_back + (always copy tresult in getresult())
 * KMP-algorithm ?
 *
 * tests
 *
 * put in tresult is it near-mod
 * bold name of file on near-mod
 * choose directory or file
 */

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->pushButton->setEnabled(false);
    ui->pushButton_2->setEnabled(false);

    std::function<void()> onLinesButtonEnabler = [this]() {
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

    std::function<void()> onFinishButtonEnabler = [this]() {
        ui->pushButton_2->setEnabled(false);
        if (!ui->lineEdit->text().isEmpty() &&
            !ui->lineEdit_2->text().isEmpty() &&
            !ui->pushButton_2->isEnabled())
        {
            ui->pushButton->setEnabled(true);
        }
    };

    connect(ui->lineEdit, &QLineEdit::textChanged, this, onLinesButtonEnabler);
    connect(ui->lineEdit_2, &QLineEdit::textChanged, this, onLinesButtonEnabler);

    connect(ui->lineEdit, &QLineEdit::textChanged, this, [this] {
        QString path = ui->lineEdit->text();
        QPalette *palette = new QPalette();
        if (QDir(path).exists() || QFile(path).exists()) {
            palette->setColor(QPalette::Text, Qt::black);
        } else {
            palette->setColor(QPalette::Text, Qt::red);
        }
        ui->lineEdit->setPalette(*palette);
    });

    connect(ui->pushButton, &QPushButton::clicked, this, [this] {
        QString path = ui->lineEdit->text();
        QString text = ui->lineEdit_2->text();

        NGrepInfo::TOptions opt(path, text, false, false, false, MAXSHOW);
        opt.Lowcase = ui->checkBox->isChecked();
        opt.Near = ui->checkBox_2->isChecked();
        opt.One = ui->checkBox_3->isChecked();

        bg_thread.SetTask(opt);
        ui->pushButton->setEnabled(false);
        ui->pushButton_2->setEnabled(true);
    });

    connect(ui->pushButton_2, &QPushButton::clicked, this,
            [this, onFinishButtonEnabler]
    {
        bg_thread.Stop();

        onFinishButtonEnabler();
    });

    connect(&bg_thread, &BgThread::ResultChanged, this,
            [this, onFinishButtonEnabler]
    {
        NGrepInfo::TResult res = bg_thread.GetResult();

        QString text;
        for (size_t i = 0; i < res.Size() && i < MAXSHOW; i++) {
            text += res.Filename[i];
            text += QString(":%1").arg(res.Lines[i]);
            text += QString(":%1").arg(res.Index[i]);

            if (!res.Near[i].isEmpty()) {
                text += ':' + CutWidth(res.Near[i], res.Index[i]) + '\n';
            } else {
                text += '\n';
            }
        }

        if (res.Complete) {
            onFinishButtonEnabler();
        } else {
            text += "...\n";
        }

        text += QString("%1 results.").arg(res.Num);

        ui->textBrowser->setPlainText(text);

        QScrollBar *sb = ui->textBrowser->verticalScrollBar();
        sb->setValue(sb->maximum());
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
