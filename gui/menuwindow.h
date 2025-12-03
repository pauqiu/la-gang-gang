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
    void initialize();
    void setLogInWindow(MainWindow *newLogIn);
    void setUsername(QString user);
    void setUserRole(QString user);
    void setUIByRole();
    void loadUsersTable();
    void setSessionToken(const uint8_t token[32]);
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

    void on_filterButton_clicked();
    void on_logFilterButton_clicked();

    void loadAvailableSensors();
    void loadSensorData(const QString& sensorId, uint64_t startDate, uint64_t endDate);
    void populateSensorsDropdown();
    
    void populateNodeSelector();
    void loadNodeLogs(uint8_t nodeType, uint64_t startDate, uint64_t endDate);
    //QString roleNumberToString(int role);

    void loadUserPermissions();
    bool hasPermission(const std::string& permission) const;

private:
    Ui::menuWindow *ui;
    MainWindow *logIn;
    QString username;
    QString userRole;
    QVector<Role> roles;
    Security * security;
    RightsValidation * rights;
    int rolesAmount;
    uint8_t sessionToken[32];
    QStringList availableSensors;
    std::vector<std::string> userPermissions;

};

#endif // MENUWINDOW_H
