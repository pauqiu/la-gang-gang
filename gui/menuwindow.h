#ifndef MENUWINDOW_H
#define MENUWINDOW_H

#include <QMainWindow>
#include "mainwindow.h"

namespace Ui {
class menuWindow;
}

class menuWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit menuWindow(QWidget *parent = nullptr);
    void setLogInWindow(MainWindow *newLogIn);
    void setUsername(QString user);
    void setUserRole(QString user);
    ~menuWindow();

private slots:
    void on_logOutButton_clicked();
    void setMenuActiveButton(int);
    void setSensorsMenuActiveButton(int index);

private:
    Ui::menuWindow *ui;
    MainWindow *logIn;
    QString username;
    QString userRole;
};

#endif // MENUWINDOW_H
