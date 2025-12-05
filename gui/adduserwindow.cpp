#include "adduserwindow.h"
#include "ui_adduserwindow.h"
#include <QPushButton>
#include <regex>

addUserWindow::addUserWindow(const QList<Role>& roles, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::addUserWindow)
{
    ui->setupUi(this);
    disconnect(ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);

    ui->passwordMessage->setVisible(false);
    ui->usernameMessage->setVisible(false);
    ui->passwordMessage->setText("Password must be at least 8 chars and include uppercase, lowercase, digit and special char.");

    for (const Role& r : roles) {
        ui->rolesComboBox->addItem(r.name);
    }

    connect(ui->buttonBox->button(QDialogButtonBox::Ok), &QPushButton::clicked,
            this, &addUserWindow::validateInputs);
}

addUserWindow::addUserWindow(const QList<Role>& roles,
                             const QString &username,
                             const QString &role,
                             QWidget *parent)
    : addUserWindow(roles, parent)
{
    ui->usernameInput->setText(username);

    int index = ui->rolesComboBox->findText(role);
    if (index >= 0)
        ui->rolesComboBox->setCurrentIndex(index);

    ui->passwordInput->setVisible(false);
    ui->passwordMessage->setVisible(false);
    ui->label_3->setVisible(false);
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

    if (ui->passwordInput->isVisible()) {
        QString pwd = ui->passwordInput->text();
        if (pwd.isEmpty()) {
            ui->passwordMessage->setText("Password is required.");
            ui->passwordMessage->setVisible(true);
            valid = false;
        } else {
            std::regex criteria("^(?=.*[a-z])(?=.*[A-Z])(?=.*\\d)(?=.*[@$!%*?&])[A-Za-z\\d@$!%*?&]{8,}$");
            std::string pwdStd = pwd.toStdString();
            if (!std::regex_match(pwdStd, criteria)) {
                ui->passwordMessage->setText("Password must have (min 8, upper, lower, digit, special)");
                ui->passwordMessage->setVisible(true);
                valid = false;
            } else {
                ui->passwordMessage->setVisible(false);
            }
        }
    } else {
        ui->passwordMessage->setVisible(false);
    }

    if (valid) {
        accept();
    }
}
