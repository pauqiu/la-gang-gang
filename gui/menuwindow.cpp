#include "adduserwindow.h"
#include "addrolewindow.h"
#include "menuwindow.h"
#include "ui_menuwindow.h"

#include <sstream>
#include <QPushButton>
#include <QInputDialog>
#include <QDebug>
#include <vector>

menuWindow::menuWindow(Security * security, RightsValidation * rights, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::menuWindow), security(security), rights(rights)
{
    ui->setupUi(this);
    this->setFixedSize(1100, 700);
    this->rolesAmount = 0;

    // Menu options
    ui->stackedWidget->setCurrentIndex(0);
    connect(ui->homeButton, &QPushButton::clicked, this, [this]() {
        ui->stackedWidget->setCurrentIndex(0);
        setMenuActiveButton(1);
    });
    connect(ui->sensorsButton, &QPushButton::clicked, this, [this]() {
        ui->stackedWidget->setCurrentIndex(1);
        setMenuActiveButton(2);
    });
    connect(ui->accountButton, &QPushButton::clicked, this, [this]() {
        ui->stackedWidget->setCurrentIndex(2);
        setMenuActiveButton(3);
    });

    // Sensors menu options
    ui->stackedWidgetSensorsMenu->setCurrentIndex(0);
    connect(ui->overviewButton, &QPushButton::clicked, this, [this]() {
        ui->stackedWidgetSensorsMenu->setCurrentIndex(0);
        setSensorsMenuActiveButton(1);
    });
    connect(ui->activityButton, &QPushButton::clicked, this, [this]() {
        ui->stackedWidgetSensorsMenu->setCurrentIndex(1);
        setSensorsMenuActiveButton(2);
    });
    connect(ui->reportsButton, &QPushButton::clicked, this, [this]() {
        ui->stackedWidgetSensorsMenu->setCurrentIndex(2);
        setSensorsMenuActiveButton(3);
    });
    connect(ui->adminButton, &QPushButton::clicked, this, [this]() {
        ui->stackedWidgetSensorsMenu->setCurrentIndex(3);
        setSensorsMenuActiveButton(4);
    });
}

menuWindow::~menuWindow()
{
    delete ui;
}

void menuWindow::setLogInWindow(MainWindow *newLogIn)
{
    this->logIn = newLogIn;
}

void menuWindow::setUsername(QString user)
{
    this->username = user;
    ui->welcomeMessage->setText(QString("Welcome, %1").arg(user));
    ui->username->setText(user);
}

void menuWindow::setUserRole(QString role)
{
    this->userRole = role;
    ui->role->setText(role);
}

void menuWindow::on_logOutButton_clicked()
{
    this->logIn->show();
    close();
}

void menuWindow::setMenuActiveButton(int index)
{
    QList<QPushButton*> buttons = { ui->homeButton, ui->sensorsButton, ui->accountButton };
    int buttonIndex = 1;

    for (QPushButton* btn : buttons) {
        if (index == buttonIndex) {
            btn->setStyleSheet("background-color: rgb(16, 84, 160); border-color: rgb(0, 68, 139); color: rgb(255, 255, 255);");
        } else {
            btn->setStyleSheet("background-color: rgb(0, 68, 139); border-color: rgb(0, 68, 139); color: rgb(255, 255, 255);");
        }
        buttonIndex++;
    }
}

void menuWindow::setSensorsMenuActiveButton(int index)
{
    QList<QPushButton*> buttons = { ui->overviewButton, ui->activityButton, ui->reportsButton, ui->adminButton };
    int buttonIndex = 1;

    for (QPushButton* btn : buttons) {
        if (!btn->isEnabled()) {
            buttonIndex++;
            continue;
        } else if (index == buttonIndex) {
            btn->setStyleSheet("color: rgb(0, 65, 119); border-bottom-color: rgb(0, 65, 119); font: 600 11pt Segoe UI;");
        } else {
            btn->setStyleSheet("color: rgb(0, 0, 0); font: 600 11pt Segoe UI;");
        }
        buttonIndex++;
    }
}

void menuWindow::loadRolesTable()
{
    ui->rolesTable->clearContents();
    ui->rolesTable->setRowCount(0);
    ui->rolesTable->setColumnCount(4); // ID, Role Name, Permissions, Action

    QStringList headers = {"ID", "Role Name", "Permissions", "Action"};
    ui->rolesTable->setHorizontalHeaderLabels(headers);

    std::vector<std::string> roleLines = rights->getRoleManager().readRolesFile();
    this->rolesAmount = 0;

    int row = 0;
    for (const auto &line : roleLines) {
        if (line.empty() || line[0] == '#') continue;

        std::istringstream iss(line);
        std::string idStr, roleName, permissions;

        if (std::getline(iss, idStr, ';') &&
            std::getline(iss, roleName, ';')) {
            this->rolesAmount++;
            int roleId = std::stoi(idStr);
            permissions = rights->getPermissions(roleId);

            ui->rolesTable->insertRow(row);
            ui->rolesTable->setItem(row, 0, new QTableWidgetItem(QString::number(roleId)));
            ui->rolesTable->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(roleName)));
            ui->rolesTable->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(permissions)));

            QPushButton *editRoleBtn = new QPushButton("Edit");
            ui->rolesTable->setCellWidget(row, 3, editRoleBtn);
            connect(editRoleBtn, &QPushButton::clicked, this, [this, row]() {
                onEditRoleClicked(row);
            });

            row++;
        }
    }
}

void menuWindow::onEditRoleClicked(int row)
{
    if (row < 0 || row >= ui->rolesTable->rowCount()) return;

    QString roleName = ui->rolesTable->item(row, 1)->text();
    QString roleDesc = ui->rolesTable->item(row, 2)->text();

    addRoleWindow dialog(roleName, roleDesc, this);
    if (dialog.exec() == QDialog::Accepted) {
        QString newRole = dialog.getRole();
        QString newDesc = dialog.getDescription();

        rights->getRoleManager().updateRole(roleName.toStdString(), newRole.toStdString(), newDesc.toStdString());
        loadRolesTable();
    }
}

void menuWindow::setUIByRole()
{
    ui->adminButton->setDisabled(true);
    ui->adminButton->setStyleSheet("font: 600 11pt Segoe UI; color: rgb(145, 145, 145);");
    if (this->userRole == "admin_general") {
        ui->adminButton->setDisabled(false);
        ui->adminButton->setStyleSheet("color: rgb(0, 0, 0); font: 600 11pt Segoe UI;");
        loadRolesTable();
        loadUsersTable();

        ui->reportsButton->setDisabled(true);
        ui->reportsButton->setStyleSheet("font: 600 11pt Segoe UI; color: rgb(145, 145, 145);");

    } else if (this->userRole == "analista_negocios") {
        ui->adminButton->setDisabled(true);
        ui->adminButton->setStyleSheet("font: 600 11pt Segoe UI; color: rgb(145, 145, 145);");

    } else if (this->userRole == "Secretary") {
        ui->reportsButton->setDisabled(true);
        ui->adminButton->setDisabled(true);
        ui->reportsButton->setStyleSheet("font: 600 11pt Segoe UI; color: rgb(145, 145, 145);");
        ui->adminButton->setStyleSheet("font: 600 11pt Segoe UI; color: rgb(145, 145, 145);");
    }
}

void menuWindow::loadUsersTable()
{
    std::vector<std::vector<std::string>> users = security->getUsers();
    ui->usersTable->clearContents();
    ui->usersTable->setRowCount(users.size());
    ui->usersTable->setColumnCount(3);

    QStringList headers = {"Username", "Role", "Action"};
    ui->usersTable->setHorizontalHeaderLabels(headers);

    ui->usersTable->setColumnWidth(0, 200);
    ui->usersTable->setColumnWidth(1, 150);
    ui->usersTable->setColumnWidth(2, 100);

    for (int i = 0; i < users.size(); ++i) {
        const std::vector<std::string> usr = users[i];

        QTableWidgetItem *usernameItem = new QTableWidgetItem(QString::fromStdString(usr[0]));
        usernameItem->setFlags(usernameItem->flags() ^ Qt::ItemIsEditable);
        ui->usersTable->setItem(i, 0, usernameItem);

        QTableWidgetItem *roleItem = new QTableWidgetItem(QString::fromStdString(usr[2]));
        roleItem->setFlags(roleItem->flags() ^ Qt::ItemIsEditable);
        ui->usersTable->setItem(i, 1, roleItem);

        QPushButton *editBtn = new QPushButton("Edit Role");
        ui->usersTable->setCellWidget(i, 2, editBtn);
        connect(editBtn, &QPushButton::clicked, this, [this, i]() {
            onEditUserRoleClicked(i);
        });
    }
}

void menuWindow::on_addUserButton_clicked()
{
    std::vector<std::string> roleLines = rights->getRoleManager().readRolesFile();
    QList<Role> currentRoles;

    for (const auto &line : roleLines) {
        if (line.empty() || line[0] == '#') continue;

        std::istringstream iss(line);
        std::string idStr, roleName;
        if (std::getline(iss, idStr, ';') &&
            std::getline(iss, roleName, ';')) {
            currentRoles.append(Role(QString::fromStdString(roleName), ""));
        }
    }

    addUserWindow dialog(currentRoles);
    if (dialog.exec() == QDialog::Accepted) {
        QString username = dialog.getUsername();
        QString password = dialog.getPassword();
        QString role = dialog.getSelectedRole();

        if (security->registerUser(username, password, role) == 0) {
            qDebug() << "Nuevo usuario:" << username << "Rol:" << role;
            loadUsersTable();
        }
    }
}

void menuWindow::onEditUserRoleClicked(int row)
{
    if (row < 0 || row >= ui->usersTable->rowCount()) return;

    QString username = ui->usersTable->item(row, 0)->text();
    QString role = ui->usersTable->item(row, 1)->text();

    std::vector<std::string> roleLines = rights->getRoleManager().readRolesFile();
    QList<Role> currentRoles;

    for (const auto &line : roleLines) {
        if (line.empty() || line[0] == '#') continue;

        std::istringstream iss(line);
        std::string idStr, roleName;
        if (std::getline(iss, idStr, ';') &&
            std::getline(iss, roleName, ';')) {
            currentRoles.append(Role(QString::fromStdString(roleName), ""));
        }
    }

    addUserWindow dialog(currentRoles, username, role, this);
    if (dialog.exec() == QDialog::Accepted) {
        QString newUsername = dialog.getUsername();
        QString newRole = dialog.getSelectedRole();

        security->updateUser(username, newUsername, newRole);
        loadUsersTable();
    }
}

void menuWindow::on_addRoleButton_clicked()
{
    addRoleWindow dialog;
    if (dialog.exec() == QDialog::Accepted) {
        QString role = dialog.getRole();
        QString description = dialog.getDescription();

        this->rolesAmount++;
        if (rights->addRole(this->rolesAmount, role.toStdString())) {
            qDebug() << "Nuevo rol:" << role;
        }
        if (rights->addPermissions(this->rolesAmount, description.toStdString())) {
            qDebug() << "Permisos añadidos al rol:" << role;
        }
        loadRolesTable();
    }
}
