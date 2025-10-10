#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "menuwindow.h"
#include "../sockets/nodeClient.h"
#include <QDebug>

MainWindow::MainWindow(Security * security, RightsValidation * rights, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow), security(security), rights(rights)
{
    ui->setupUi(this);
    this->setFixedSize(1100, 700);
    ui->usernameMessage->setVisible(false);
    ui->passwordMessage->setVisible(false);
    ui->passwordHelp->setVisible(false);
    ui->authErrorMessage->setVisible(false);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_logInButton_clicked()
{
    // Validar campos vacíos
    if(ui->usernameInput->text() == ""){
        ui->usernameMessage->setVisible(true);
        ui->passwordMessage->setVisible(false);
        return;
    } else{
        ui->usernameMessage->setVisible(false);
    }

    if(ui->passwordInput->text() == ""){
        ui->passwordMessage->setVisible(true);
        return;
    } else {
        ui->passwordMessage->setVisible(false);
    }

    // CAMBIO: Usar NodeClient para autenticación remota
    NodeClient client;
    client.sendAuthentication(
        ui->usernameInput->text().toStdString(),
        ui->passwordInput->text().toStdString(),
        0  // intentos fallidos
        );

    // Verificar respuesta del servidor
    if (client.hasValidToken()) {
        qDebug() << "Login successful!";
        ui->authErrorMessage->setVisible(false);

        // Obtener rol del token (viene del servidor)
        uint8_t roleNumber = client.getRole();
        QString userRole = roleNumberToString(roleNumber);

        // Abrir ventana de menú
        menuWindow *menu = new menuWindow(this->security, this->rights);
        menu->setLogInWindow(this);
        menu->setUsername(ui->usernameInput->text());
        menu->setUserRole(userRole);
        menu->setUIByRole();
        menu->show();
        close();
    }
    else {
        // Mostrar error de autenticación
        qDebug() << "Login failed!";
        ui->authErrorMessage->setVisible(true);
        ui->authErrorMessage->setText("Credenciales incorrectas");
    }
}

// Función auxiliar para convertir número de rol a string
QString MainWindow::roleNumberToString(int role) {
    // Mapear según tu sistema de roles (1-7)
    switch(role) {
    case 1: return "admin_sistema";
    case 2: return "tecnico_sensores";
    case 3: return "oficial_seguridad";
    case 4: return "supervisor_seguridad";
    case 5: return "analista_negocios";
    case 6: return "admin_general";
    case 7: return "auditor";
    default: return "unknown";
    }
}


void MainWindow::on_forgotPasswordButton_clicked()
{
    ui->passwordHelp->setVisible(true);
}
