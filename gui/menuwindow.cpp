#include "menuwindow.h"
#include "ui_menuwindow.h"

menuWindow::menuWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::menuWindow)
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
        if (index == buttonIndex) {
            btn->setStyleSheet("color: rgb(0, 65, 119); border-bottom-color: rgb(0, 65, 119); font: 600 11pt Segoe UI;");
        } else {
            btn->setStyleSheet("color: rgb(0, 0, 0); font: 600 11pt Segoe UI;");
        }
        buttonIndex++;
    }
}
