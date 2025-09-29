#ifndef MENUWINDOW_H
#define MENUWINDOW_H

#include <QMainWindow>
#include <QVector>
#include "mainwindow.h"
#include "addUserWindow.h"
#include "Role.h"

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
    void setUIByRole();
    ~menuWindow();

private slots:
    void on_logOutButton_clicked();
    void setMenuActiveButton(int);
    void setSensorsMenuActiveButton(int index);

    void loadRolesTable();
    void onEditRoleClicked(int row);

    void on_addUserButton_clicked();

private:
    Ui::menuWindow *ui;
    MainWindow *logIn;
    QString username;
    QString userRole;
    QVector<Role> roles;

};

#endif // MENUWINDOW_H
