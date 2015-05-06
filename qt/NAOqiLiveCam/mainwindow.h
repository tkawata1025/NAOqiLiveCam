/**
 * @author Takuji Kawata
 * Updated 2015/05/07
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>

namespace Ui {
class MainWindow;
}

class AudioOutput;

class MainWindow : public QMainWindow
{
    Q_OBJECT

    QTimer          *d_cameraIntervalTimer;

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    static void consoleMessage(const QString& msg);

private:
    Ui::MainWindow  *ui;
    AudioOutput     *d_audio;

protected:
    virtual void paintEvent(QPaintEvent *event );

private slots:
    void connectButtonClicked();
    void disconnectButtonClicked();
    void updateCameraView();

signals:
    void consoleUpdated();

};

#endif // MAINWINDOW_H
