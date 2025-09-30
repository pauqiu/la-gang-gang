#include "addrolewindow.h"
#include "ui_addrolewindow.h"

addRoleWindow::addRoleWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::addRoleWindow)
{
    ui->setupUi(this);
    disconnect(ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);

    ui->roleMessage->setVisible(false);
    ui->roleDescriptionMessage->setVisible(false);

    connect(ui->buttonBox->button(QDialogButtonBox::Ok), &QPushButton::clicked,
            this, &addRoleWindow::validateInputs);
}

addRoleWindow::addRoleWindow(const QString &role, const QString &description, QWidget *parent)
    : addRoleWindow(parent)
{
    ui->roleInput->setText(role);
    ui->descriptionInput->setText(description);
}

addRoleWindow::~addRoleWindow()
{
    delete ui;
}

QString addRoleWindow::getRole() const
{
    return ui->roleInput->text();
}

QString addRoleWindow::getDescription() const
{
    return ui->descriptionInput->text();
}

void addRoleWindow::validateInputs()
{
    bool valid = true;

    if (ui->roleInput->text().isEmpty()) {
        ui->roleMessage->setVisible(true);
        valid = false;
    } else {
        ui->roleMessage->setVisible(false);
    }

    if (ui->descriptionInput->text().isEmpty()) {
        ui->roleDescriptionMessage->setVisible(true);
        valid = false;
    } else {
        ui->roleDescriptionMessage->setVisible(false);
    }

    if (valid) {
        accept();
    }
}
