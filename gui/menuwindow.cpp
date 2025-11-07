#include "adduserwindow.h"
#include "addrolewindow.h"
#include "menuwindow.h"
#include "ui_menuwindow.h"
#include "../sockets/nodeProxy.h"
#include "../sockets/endpoints.h"

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
    ui->filterErrorMsg->setVisible(false);

    std::memset(sessionToken, 0, 32);

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

void menuWindow::initialize() {
        loadAvailableSensors();
        populateSensorsDropdown();
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
    loadUserPermissions();
}

void menuWindow::loadUserPermissions()
{
    userPermissions.clear();

    // Obtener ID del rol por nombre
    int roleId = rights->getRoleManager().getRoleIdByName(userRole.toStdString());

    if (roleId == -1) {
        qDebug() << "Error: Rol no encontrado:" << userRole;
        return;
    }

    // Obtener permisos del rol
    std::string permissions = rights->getPermissions(roleId);

    if (permissions.empty()) {
        qDebug() << "Advertencia: El rol" << userRole << "no tiene permisos asignados";
        return;
    }

    // Parsear permisos (separados por coma)
    std::istringstream iss(permissions);
    std::string perm;

    while (std::getline(iss, perm, ',')) {
        // Trim espacios
        perm.erase(0, perm.find_first_not_of(" \t"));
        perm.erase(perm.find_last_not_of(" \t") + 1);

        if (!perm.empty()) {
            userPermissions.push_back(perm);
        }
    }

    qDebug() << "Permisos cargados para" << userRole << ":" << userPermissions.size();
}

// Implementa hasPermission
bool menuWindow::hasPermission(const std::string& permission) const
{
    return std::find(userPermissions.begin(), userPermissions.end(), permission)
    != userPermissions.end();
}

void menuWindow::on_logOutButton_clicked()
{
    this->logIn->clearInputs();
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

    QString roleId = ui->rolesTable->item(row, 0)->text();
    QString roleName = ui->rolesTable->item(row, 1)->text();
    QString currentPermissions = ui->rolesTable->item(row, 2)->text();

    addRoleWindow dialog(roleName, currentPermissions, this);
    if (dialog.exec() == QDialog::Accepted) {
        QString newRoleName = dialog.getRole();
        QString newPermissions = dialog.getPermissions();

        rights->getRoleManager().updateRole(
            roleName.toStdString(),
            newRoleName.toStdString(),
            newPermissions.toStdString()
            );

        loadRolesTable();
    }
}

void menuWindow::setUIByRole()
{
    // Admin button - requiere permisos de gestión
    if (hasPermission("Manage_users") || hasPermission("manage_roles")) {
        ui->adminButton->setDisabled(false);
        ui->adminButton->setStyleSheet("color: rgb(0, 0, 0); font: 600 11pt Segoe UI;");
        loadRolesTable();
        loadUsersTable();
    } else {
        ui->adminButton->setDisabled(true);
        ui->adminButton->setStyleSheet("font: 600 11pt Segoe UI; color: rgb(145, 145, 145);");
    }

    // Reports button
    if (hasPermission("generate_attendance_reports") ||
        hasPermission("generate_incident_reports") ||
        hasPermission("generate_activity_reports")) {
        ui->reportsButton->setDisabled(false);
        ui->reportsButton->setStyleSheet("color: rgb(0, 0, 0); font: 600 11pt Segoe UI;");
    } else {
        ui->reportsButton->setDisabled(true);
        ui->reportsButton->setStyleSheet("font: 600 11pt Segoe UI; color: rgb(145, 145, 145);");
    }

    // Activity button - alertas y monitoreo
    if (hasPermission("View_realtime_alerts") ||
        hasPermission("View_alert_history") ||
        hasPermission("view_active_alarms")) {
        ui->activityButton->setDisabled(false);
        ui->activityButton->setStyleSheet("color: rgb(0, 0, 0); font: 600 11pt Segoe UI;");
    } else {
        ui->activityButton->setDisabled(true);
        ui->activityButton->setStyleSheet("font: 600 11pt Segoe UI; color: rgb(145, 145, 145);");
    }

    // Overview button - sensores
    if (hasPermission("View_sensor_status") ||
        hasPermission("view_sensor_health") ||
        hasPermission("view_raw_sensor_data")) {
        ui->overviewButton->setDisabled(false);
        ui->overviewButton->setStyleSheet("color: rgb(0, 0, 0); font: 600 11pt Segoe UI;");
    } else {
        ui->overviewButton->setDisabled(true);
        ui->overviewButton->setStyleSheet("font: 600 11pt Segoe UI; color: rgb(145, 145, 145);");
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
    addRoleWindow dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        QString role = dialog.getRole();
        QString permissions = dialog.getPermissions();

        this->rolesAmount++;

        if (rights->addRole(this->rolesAmount, role.toStdString())) {
            qDebug() << "Nuevo rol creado:" << role;

            if (rights->addPermissions(this->rolesAmount, permissions.toStdString())) {
                qDebug() << "Permisos asignados:" << permissions;
            }
        }

        loadRolesTable();
    }
}

void menuWindow::setSessionToken(const uint8_t token[32]) {
    std::memcpy(sessionToken, token, 32);
}

void menuWindow::loadAvailableSensors() {
    availableSensors.clear();

    int sock = connect_to(getProxyIp(), getProxyPort());
    if (sock < 0) {
        qDebug() << "Error conectando con Proxy";
        return;
    }

    ListSensorRequest request;
    request.message_id = MSG_LIST_SENSOR_REQUEST;
    std::memcpy(request.token, sessionToken, 32);

    auto data = request.serialize();
    if (!send_message(sock, data.data(), data.size())) {
        qDebug() << "Error enviando petición";
        return;
    }

    std::vector<uint8_t> response(4096);
    ssize_t bytes = recv_message(sock, response.data(), response.size());

    if (bytes <= 0) {
        qDebug() << "Error recibiendo respuesta";
        return;
    }

    response.resize(bytes);
    auto listResponse = ListSensorResponse::deserialize(response);

    for (const auto& sensorId : listResponse.sensorIds) {
        QString sensor = QString::fromUtf8(sensorId.data(), strnlen(sensorId.data(), 16));
        availableSensors.append(sensor);
    }

    qDebug() << "Sensores cargados:" << availableSensors;
}

void menuWindow::populateSensorsDropdown() {
    ui->sensorIdInput->clear();
    ui->sensorIdInput->addItems(availableSensors);
}

void menuWindow::on_filterButton_clicked()
{
    ui->filterErrorMsg->setVisible(false);

    if (ui->sensorIdInput->currentText().isEmpty()) {
        ui->filterErrorMsg->setText("Select a sensor");
        ui->filterErrorMsg->setVisible(true);
        return;
    }

    QDate startDate = ui->startDateInput->date();
    QDate endDate = ui->endDateInput->date();

    if (startDate > endDate) {
        ui->filterErrorMsg->setText("Incorrect dates");
        ui->filterErrorMsg->setVisible(true);
        return;
    }

    uint64_t startDateInt = startDate.toString("yyyyMMdd").toULongLong();
    uint64_t endDateInt = endDate.toString("yyyyMMdd").toULongLong();

    QString sensorId = ui->sensorIdInput->currentText();

    loadSensorData(sensorId, startDateInt, endDateInt);
}

void menuWindow::loadSensorData(const QString& sensorId, uint64_t startDate, uint64_t endDate) {
    int sock = connect_to(getProxyIp(), getProxyPort());
    if (sock < 0) {
        qDebug() << "Error conectando con Proxy";
        ui->filterErrorMsg->setText("Connection error");
        ui->filterErrorMsg->setVisible(true);
        return;
    }

    DataRequest request;
    request.message_id = MSG_DATA_REQUEST;
    std::memcpy(request.token, sessionToken, 32);

    std::memset(request.sensor_id, 0, 16);
    std::memcpy(request.sensor_id,
                sensorId.toStdString().c_str(),
                std::min(static_cast<size_t>(sensorId.length()), static_cast<size_t>(16)));

    request.startDate = startDate;
    request.endDate = endDate;

    auto data = request.serialize();
    if (!send_message(sock, data.data(), data.size())) {
        qDebug() << "Error enviando petición";
        return;
    }

    std::vector<uint8_t> response(8192);
    ssize_t bytes = recv_message(sock, response.data(), response.size());

    if (bytes <= 0) {
        qDebug() << "Error recibiendo respuesta";
        return;
    }

    response.resize(bytes);
    auto dataResponse = DataResponse::deserialize(response);

    ui->sensorTable->clearContents();
    ui->sensorTable->setRowCount(dataResponse.entriesCount);
    ui->sensorTable->setColumnCount(5);

    QStringList headers = {"Sensor", "Date", "Time", "Data", "Status"};
    ui->sensorTable->setHorizontalHeaderLabels(headers);

    for (int i = 0; i < dataResponse.entriesCount; i++) {
        const auto& entry = dataResponse.entries[i];

        QString sensor = QString::fromUtf8(entry.sensor_id, strnlen(entry.sensor_id, 16));
        ui->sensorTable->setItem(i, 0, new QTableWidgetItem(sensor));

        QString dateStr = QString::number(entry.date);
        QString formattedDate = dateStr.mid(0, 4) + "-" + dateStr.mid(4, 2) + "-" + dateStr.mid(6, 2);
        ui->sensorTable->setItem(i, 1, new QTableWidgetItem(formattedDate));

        QString timeStr = QString::number(entry.time).rightJustified(6, '0');
        QString formattedTime = timeStr.mid(0, 2) + ":" + timeStr.mid(2, 2) + ":" + timeStr.mid(4, 2);
        ui->sensorTable->setItem(i, 2, new QTableWidgetItem(formattedTime));

        ui->sensorTable->setItem(i, 3, new QTableWidgetItem(QString::number(entry.data_value, 'f', 2)));

        QString status = QString::fromUtf8(entry.status, strnlen(entry.status, 8));
        QTableWidgetItem* statusItem = new QTableWidgetItem(status);

        if (status == "ALERT") {
            statusItem->setBackground(QColor(255, 200, 200));  // Rojo
        } else {
            statusItem->setBackground(QColor(200, 255, 200));  // Verde
        }

        ui->sensorTable->setItem(i, 4, statusItem);
    }

    qDebug() << "Tabla cargada con" << dataResponse.entriesCount << "registros";
}
