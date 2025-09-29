#include "addUserWindow.h"
#include "menuwindow.h"
#include "ui_menuwindow.h"

#include <QPushButton>
#include <QInputDialog>
#include <vector>

menuWindow::menuWindow(Security * security, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::menuWindow), security(security)
{
    ui->setupUi(this);
    this->setFixedSize(1100, 700);

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
            btn->setStyleSheet("background-color: rgb(16, 84, 160); border-color: rgb(0, 68, 139);");
        } else {
            btn->setStyleSheet("background-color: rgb(0, 68, 139); border-color: rgb(0, 68, 139);");
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
            // Botón activo
            btn->setStyleSheet("color: rgb(0, 65, 119); border-bottom-color: rgb(0, 65, 119); font: 600 11pt Segoe UI;");
        } else {
            // Botones inactivos
            btn->setStyleSheet("color: rgb(0, 0, 0); font: 600 11pt Segoe UI;");
        }
        buttonIndex++;
    }
}

void menuWindow::loadRolesTable()
{
    ui->rolesTable->clearContents();
    ui->rolesTable->setRowCount(roles.size());

    ui->rolesTable->setColumnCount(3);
    QStringList headers = {"Role", "Description", "Action"};
    ui->rolesTable->setHorizontalHeaderLabels(headers);

    // Tamaño específico de cada columna
    ui->rolesTable->setColumnWidth(0, 150);
    ui->rolesTable->setColumnWidth(1, 300);
    ui->rolesTable->setColumnWidth(2, 80);

    for (int i = 0; i < roles.size(); ++i) {
        QTableWidgetItem *nameItem = new QTableWidgetItem(roles[i].name);
        nameItem->setFlags(nameItem->flags() ^ Qt::ItemIsEditable);
        ui->rolesTable->setItem(i, 0, nameItem);

        QTableWidgetItem *descItem = new QTableWidgetItem(roles[i].description);
        descItem->setFlags(descItem->flags() ^ Qt::ItemIsEditable);
        ui->rolesTable->setItem(i, 1, descItem);

        QPushButton *editBtn = new QPushButton("Edit");
        ui->rolesTable->setCellWidget(i, 2, editBtn);

        // Connect edit button with role
        connect(editBtn, &QPushButton::clicked, this, [this, i]() {
            onEditRoleClicked(i);
        });
    }
}

void menuWindow::onEditRoleClicked(int row)
{
    if (row < 0 || row >= roles.size()) return;
}

void menuWindow::setUIByRole()
{
    if (this->userRole == "Admin") {
        Role adminR("Admin", "Acceso total");
        roles.append(adminR);
        Role userR("User", "Acceso básico");
        roles.append(userR);
        Role guestR("Guest", "Solo lectura");
        roles.append(guestR);
        loadRolesTable();
        loadUsersTable();
        ui->reportsButton->setDisabled(true);
        ui->reportsButton->setStyleSheet("font: 600 11pt Segoe UI; color: rgb(145, 145, 145);");

    } else if (this->userRole == "Analist") {
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
    // TODO (@Paulette: Avoid passing passwords to front-end.

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

        // Pass row index as property or connect with a lambda
        connect(editBtn, &QPushButton::clicked, this, [this, i]() {
            onEditUserRoleClicked(i);
        });
    }
}

void menuWindow::on_addUserButton_clicked()
{
    addUserWindow dialog(this->roles);
    if (dialog.exec() == QDialog::Accepted) {
        QString username = dialog.getUsername();
        QString password = dialog.getPassword();
        QString role = dialog.getSelectedRole();

        if (!security->registerUser(username, password, role)) {
            // TODO: Show in UI the password requirements.
            qDebug() << "Nuevo usuario:" << username << "Rol:" << role;
        }
    }
}

void menuWindow::onEditUserRoleClicked(int row)
{
    qDebug() << "Editing role";
}

