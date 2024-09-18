#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QClipboard>
#include <QFile>
#include <QProcess>
#include <QRegularExpression>
#include <QSettings>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QObject::connect(&proc, &QProcess::readyReadStandardOutput, this, &MainWindow::new_output);
    readSettings();
    getURLFromClipboard();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::new_output()
{
    QString output( proc.readAllStandardOutput() );
    QRegularExpression re;
    re.setPattern("[download].*?(\\d*\\.\\d*)%.*?(\\d*\\.\\d*.*?) at.*?(\\d*\\.\\d*.*?\\/s)");
    QRegularExpressionMatch match = re.match(output);
    QString progress = match.captured(1);
    QString total = match.captured(2);
    QString speed = match.captured(3);
    this->ui->progressBar->setValue(progress.toFloat());
    ui->speedLabel->setText("Speed: " + speed);
    ui->totalLabel->setText("Total: " + total);
}

void MainWindow::on_pushButton_clicked()
{
    QString URL = ui->urlInput->text();
    QStringList params = {URL, "-P", "~"};
    if(ui->splitChaptersCheckbox->isChecked())
        params.append("--split-chapters");
    if(ui->justAudioCheckBox->isChecked())
        params.append("--extract-audio");
    proc.start("yt-dlp", params);
    if(!URL.isEmpty())
        ui->listWidget->addItem(URL);
}

void MainWindow::saveDownloads()
{
    QString filename = "downloads.txt";
    QFile file(filename);
    if (file.open(QIODevice::ReadWrite)) { //ReadWrite
        QTextStream stream(&file);
        for(int i = 0; i < ui->listWidget->count(); ++i)
        {
            QListWidgetItem* item = ui->listWidget->item(i);
            stream << item->text() << Qt::endl;
        }
    }
}

void MainWindow::loadDownloads()
{
    QString filename = "downloads.txt";
    QFile inputFile(filename);

    if (inputFile.open(QIODevice::ReadOnly))
    {
        QTextStream in(&inputFile);
        while (!in.atEnd())
        {
            QString line = in.readLine();
            ui->listWidget->addItem(line);
        }
        inputFile.close();
    }

}

void MainWindow::writeSettings()
{
    QSettings settings("AlexMNSKSoft", "ytdlpgui");

    settings.beginGroup("MainWindow");
    settings.setValue("size", size());
    settings.setValue("pos", pos());
    settings.setValue("justAudio", ui->justAudioCheckBox->isChecked());
    settings.setValue("splitChapters", ui->splitChaptersCheckbox->isChecked());
    settings.endGroup();

    saveDownloads();
}

void MainWindow::readSettings()
{
    QSettings settings("AlexMNSKSoft", "ytdlpgui");

    settings.beginGroup("MainWindow");
    resize(settings.value("size", QSize(400, 400)).toSize());
    move(settings.value("pos", QPoint(200, 200)).toPoint());

    ui->justAudioCheckBox->setChecked(settings.value("justAudio", QVariant(false)).toBool() );
    ui->splitChaptersCheckbox->setChecked(settings.value("splitChapters", QVariant(false)).toBool() );

    settings.endGroup();

    loadDownloads();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    writeSettings();
    QWidget::closeEvent(event);
}

void MainWindow::getURLFromClipboard()
{
    QClipboard *clipboard = QGuiApplication::clipboard();
    ui->urlInput->setText(clipboard->text());
}
