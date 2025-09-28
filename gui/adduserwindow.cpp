#include "adduserwindow.h"
#include "ui_adduserwindow.h"
#include <QPushButton>

addUserWindow::addUserWindow(const QList<Role>& roles, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::addUserWindow)
{
    ui->setupUi(this);
    disconnect(ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);

    ui->passwordMessage->setVisible(false);
    ui->usernameMessage->setVisible(false);

    for (const Role& r : roles) {
        ui->rolesComboBox->addItem(r.name);
    }

    connect(ui->buttonBox->button(QDialogButtonBox::Ok), &QPushButton::clicked,
            this, &addUserWindow::validateInputs);
}

addUserWindow::~addUserWindow()
{
    delete ui;
}

QString addUserWindow::getUsername() const
{
    return ui->usernameInput->text();
}

QString addUserWindow::getPassword() const
{
    return ui->passwordInput->text();
}

QString addUserWindow::getSelectedRole() const
{
    return ui->rolesComboBox->currentText();
}

void addUserWindow::validateInputs()
{
    bool valid = true;

    if (ui->usernameInput->text().isEmpty()) {
        ui->usernameMessage->setVisible(true);
        valid = false;
    } else {
        ui->usernameMessage->setVisible(false);
    }

    if (ui->passwordInput->text().isEmpty()) {
        ui->passwordMessage->setVisible(true);
        valid = false;
    } else {
        ui->passwordMessage->setVisible(false);
    }

    if (valid) {
        accept();
    }
}
