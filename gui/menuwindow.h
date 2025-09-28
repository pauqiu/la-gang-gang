#ifndef MENUWINDOW_H
#define MENUWINDOW_H

#include <QMainWindow>
#include <QVector>
#include "mainwindow.h"

namespace Ui {
class menuWindow;
}

struct Role {
    QString name;
    QString description;

    Role(QString name, QString description){
        this->name = name;
        this->description = description;
    }
};

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

private:
    Ui::menuWindow *ui;
    MainWindow *logIn;
    QString username;
    QString userRole;
    QVector<Role> roles;

};

#endif // MENUWINDOW_H
