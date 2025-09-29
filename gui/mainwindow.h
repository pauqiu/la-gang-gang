#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "security.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(Security * security, QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_logInButton_clicked();

    void on_forgotPasswordButton_clicked();

private:
    Ui::MainWindow *ui;
    Security * security;
};
#endif // MAINWINDOW_H
