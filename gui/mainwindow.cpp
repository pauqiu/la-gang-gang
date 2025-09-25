#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "menuwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setFixedSize(1100, 700);
    ui->usernameMessage->setVisible(false);
    ui->passwordMessage->setVisible(false);
    ui->passwordHelp->setVisible(false);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_logInButton_clicked()
{
    if(ui->usernameInput->text() != "" && ui->passwordInput->text() != ""){
        menuWindow *menu = new menuWindow();
        menu->setLogInWindow(this);
        menu->setUsername(ui->usernameInput->text());
        menu->setUserRole("Admin");
        menu->setUIByRole();
        menu->show();
        close();
    } else{
        if(ui->usernameInput->text() == ""){
            ui->usernameMessage->setVisible(true);
        } else{
            ui->usernameMessage->setVisible(false);
        }
        if(ui->passwordInput->text() == ""){
            ui->passwordMessage->setVisible(true);
        } else {
            ui->passwordMessage->setVisible(false);
        }
    }
}


void MainWindow::on_forgotPasswordButton_clicked()
{
    ui->passwordHelp->setVisible(true);
}
