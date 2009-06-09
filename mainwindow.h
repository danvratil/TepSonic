#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include <QtGui/QActionGroup>
#include <QtGui/QSystemTrayIcon>

namespace Ui
{
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

    bool canClose;

private:
    Ui::MainWindow *ui;

    QActionGroup *randomPlaybackGroup;
    QActionGroup *repeatPlaybackGroup;

    QSystemTrayIcon *trayIcon;
    QIcon *appIcon;

protected:
        void closeEvent(QCloseEvent*);

private slots:
    void on_actionLoad_file_triggered();
    void on_actionQuit_TepSonic_triggered();
    void trayClicked(QSystemTrayIcon::ActivationReason);
    void on_actionReport_a_bug_triggered();
    void on_actionAbout_TepSonic_triggered();
    void on_actionAbout_Qt_triggered();

};

#endif // MAINWINDOW_H
