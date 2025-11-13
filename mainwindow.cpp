#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , manager(new NodeManager())
{
    ui->setupUi(this);
    on_pushButton_clicked();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    auto states = manager->checkAllNodes();

    // --- Auth ---
    ui->authState->setText(states[0] ? "Encendido" : "Apagado");
    ui->authState->setStyleSheet(states[0] ? "color: green;" : "color: red;");
    ui->authButton->setText(states[0] ? "Apagar" : "Encender");

    // --- Proxy ---
    ui->proxyState->setText(states[1] ? "Encendido" : "Apagado");
    ui->proxyState->setStyleSheet(states[1] ? "color: green;" : "color: red;");
    ui->proxyButton->setText(states[1] ? "Apagar" : "Encender");

    // --- Storage ---
    ui->storageState->setText(states[2] ? "Encendido" : "Apagado");
    ui->storageState->setStyleSheet(states[2] ? "color: green;" : "color: red;");
    ui->storageButton->setText(states[2] ? "Apagar" : "Encender");

    // --- Receptor ---
    ui->receptorState->setText(states[3] ? "Encendido" : "Apagado");
    ui->receptorState->setStyleSheet(states[3] ? "color: green;" : "color: red;");
    ui->receptorButton->setText(states[3] ? "Apagar" : "Encender");
}

void MainWindow::on_authButton_clicked()
{
    QString currentText = ui->authButton->text();

    if (currentText == "Encender") {
        manager->startAuth();
        bool isUp = manager->checkNodeStatus(getAuthIp(), getAuthPort());

        ui->authState->setText(isUp ? "Encendido" : "Error al encender");
        ui->authState->setStyleSheet(isUp ? "color: green;" : "color: red;");
        ui->authButton->setText(isUp ? "Apagar" : "Encender");
    }
    else {
        manager->stopAuth();
        bool isDown = !manager->checkNodeStatus(getAuthIp(), getAuthPort());

        ui->authState->setText(isDown ? "Apagado" : "Error al apagar");
        ui->authState->setStyleSheet(isDown ? "color: red;" : "color: orange;");
        ui->authButton->setText(isDown ? "Encender" : "Apagar");
    }
}

void MainWindow::on_proxyButton_clicked()
{
    QString currentText = ui->proxyButton->text();

    if (currentText == "Encender") {
        manager->startProxy();
        bool isUp = manager->checkNodeStatus(getProxyIp(), getProxyPort());

        ui->proxyState->setText(isUp ? "Encendido" : "Error al encender");
        ui->proxyState->setStyleSheet(isUp ? "color: green;" : "color: red;");
        ui->proxyButton->setText(isUp ? "Apagar" : "Encender");
    } else {
        manager->stopProxy();
        bool isDown = !manager->checkNodeStatus(getProxyIp(), getProxyPort());

        ui->proxyState->setText(isDown ? "Apagado" : "Error al apagar");
        ui->proxyState->setStyleSheet(isDown ? "color: red;" : "color: orange;");
        ui->proxyButton->setText(isDown ? "Encender" : "Apagar");
    }
}


void MainWindow::on_storageButton_clicked()
{
    QString currentText = ui->storageButton->text();

    if (currentText == "Encender") {
        manager->startStorage();
        bool isUp = manager->checkNodeStatus(getStorageIp(), getStoragePort());

        ui->storageState->setText(isUp ? "Encendido" : "Error al encender");
        ui->storageState->setStyleSheet(isUp ? "color: green;" : "color: red;");
        ui->storageButton->setText(isUp ? "Apagar" : "Encender");
    } else {
        manager->stopStorage();
        bool isDown = !manager->checkNodeStatus(getStorageIp(), getStoragePort());

        ui->storageState->setText(isDown ? "Apagado" : "Error al apagar");
        ui->storageState->setStyleSheet(isDown ? "color: red;" : "color: orange;");
        ui->storageButton->setText(isDown ? "Encender" : "Apagar");
    }
}


void MainWindow::on_receptorButton_clicked()
{
    QString currentText = ui->receptorButton->text();

    if (currentText == "Encender") {
        manager->startReceptor();
        bool isUp = manager->checkNodeStatus(getReceptorIp(), getReceptorPort());

        ui->receptorState->setText(isUp ? "Encendido" : "Error al encender");
        ui->receptorState->setStyleSheet(isUp ? "color: green;" : "color: red;");
        ui->receptorButton->setText(isUp ? "Apagar" : "Encender");
    }
    else {
        manager->stopReceptor();
        bool isDown = !manager->checkNodeStatus(getReceptorIp(), getReceptorPort());

        ui->receptorState->setText(isDown ? "Apagado" : "Error al apagar");
        ui->receptorState->setStyleSheet(isDown ? "color: red;" : "color: orange;");
        ui->receptorButton->setText(isDown ? "Encender" : "Apagar");
    }
}

