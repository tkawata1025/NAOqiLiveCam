#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "NAOqi/nao_interface/nao_interface.h"

#include <QMutex>
#include <QQueue>

#include "audiooutput.h"

static bool s_isConnected = false;
static QMutex s_consoleMutex;
static QQueue<QString> s_pendingConsoleMessages;
static MainWindow *s_window = NULL;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->disconnectButton->setEnabled(false);
    ui->connectButton->setEnabled(true);
    ui->console->setMaximumBlockCount(100);
    ui->console->setReadOnly(true);
    QFont monofont("Courier");
    monofont.setStyleHint(QFont::Monospace);
    ui->console->setFont(monofont);

    connect(this, SIGNAL(consoleUpdated()), this, SLOT(update()), Qt::AutoConnection);
    connect(ui->connectButton, SIGNAL(clicked()), this, SLOT(connectButtonClicked()));
    connect(ui->disconnectButton, SIGNAL(clicked()), this, SLOT(disconnectButtonClicked()));

    d_cameraIntervalTimer = new QTimer(this);
    d_cameraIntervalTimer->setInterval(100);
    connect(d_cameraIntervalTimer, SIGNAL(timeout()), this, SLOT(updateCameraView()));

    s_window = this;


    d_audio = new AudioOutput();

}

MainWindow::~MainWindow()
{
    if (d_cameraIntervalTimer)
        d_cameraIntervalTimer->stop();

    if (d_audio)
        delete d_audio;

    delete ui;

}

//static
void MainWindow::consoleMessage(const QString &msg)
{
    QMutexLocker lock(&s_consoleMutex);

    bool hasPendingMessage = s_pendingConsoleMessages.length() > 0;
    s_pendingConsoleMessages.append(msg);
    if (s_window)
    {
        if (hasPendingMessage)
        {
            emit s_window->consoleUpdated();
        }
    }
}

void MainWindow::connectButtonClicked()
{
    s_isConnected = false;
    try
    {
        d_cameraIntervalTimer->stop();
        QString msg = "connecting to ";
        msg.append(ui->naoIp->text());
        msg.append("...");
        ui->console->appendPlainText(msg);
        NaoInterface::instance()->setNaoIp(ui->naoIp->text().toStdString());
        ui->console->appendPlainText("connected!");
        ui->connectButton->setEnabled(false);
        ui->disconnectButton->setEnabled(true);
        ui->naoIp->setReadOnly(true);
        s_isConnected = true;

        d_cameraIntervalTimer->start();
        d_audio->startPlay();
    }
    catch ( std::string exceptionMsg )
    {
        ui->console->appendPlainText("ERROR: connction failed.");
        QString msg = QString::fromStdString(exceptionMsg);
        ui->console->appendPlainText(msg);
    }
}

void MainWindow::disconnectButtonClicked()
{
    try
    {
        d_cameraIntervalTimer->stop();
        d_audio->stopPlay();
        QString msg = "disconnect from ";
        msg.append(ui->naoIp->text());
        msg.append("...");
        ui->console->appendPlainText(msg);
        NaoInterface::instance()->disconnect();
        ui->console->appendPlainText("disconnected!");
        ui->connectButton->setEnabled(true);
        ui->disconnectButton->setEnabled(false);
        ui->naoIp->setReadOnly(false);
        s_isConnected = false;
    }
    catch ( std::string exceptionMsg )
    {
        ui->console->appendPlainText("ERROR: connction failed.");
        QString msg = QString::fromStdString(exceptionMsg);
        ui->console->appendPlainText(msg);
    }
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    QMutexLocker lock(&s_consoleMutex);

    while (!s_pendingConsoleMessages.isEmpty())
    {
        ui->console->appendPlainText(s_pendingConsoleMessages.dequeue());
    }
    QMainWindow::paintEvent(event);
}

void MainWindow::updateCameraView()
{
    QImage img(NaoInterface::instance()->updateCameraView(),320,240,QImage::Format_RGB888);
    ui->cameraView->setPixmap(QPixmap::fromImage(img));
}
