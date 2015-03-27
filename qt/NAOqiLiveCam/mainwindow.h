#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QTimer>


namespace Ui {
class MainWindow;
}

class AudioOutput;

class MainWindow : public QMainWindow
{
    Q_OBJECT

    QGraphicsScene  *d_cameraViewScene;
    QTimer          *d_cameraIntervalTimer;

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    static void consoleMessage(const QString& msg);

private:
    Ui::MainWindow *ui;

    AudioOutput *audio;

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
