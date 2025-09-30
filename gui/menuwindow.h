#ifndef MENUWINDOW_H
#define MENUWINDOW_H

#include <QMainWindow>
#include <QVector>

#include "mainwindow.h"
#include "RightsValidation/RightsValidation.h"
#include "RightsValidation/RolesFileManager.h"
#include "Role.h"
#include "security.h"

namespace Ui {
class menuWindow;
}

class menuWindow : public QMainWindow
{
    Q_OBJECT

public:

    explicit menuWindow(Security * security, RightsValidation * rights, QWidget *parent = nullptr);
    void setLogInWindow(MainWindow *newLogIn);
    void setUsername(QString user);
    void setUserRole(QString user);
    void setUIByRole();
    void loadUsersTable();
    ~menuWindow();

private slots:
    void on_logOutButton_clicked();
    void setMenuActiveButton(int);
    void setSensorsMenuActiveButton(int index);

    void loadRolesTable();
    void onEditRoleClicked(int row);

    void on_addUserButton_clicked();
    void onEditUserRoleClicked(int row);

    void on_addRoleButton_clicked();

private:
    Ui::menuWindow *ui;
    MainWindow *logIn;
    QString username;
    QString userRole;
    QVector<Role> roles;
    Security * security;
    RightsValidation * rights;
    int rolesAmount;

};

#endif // MENUWINDOW_H
