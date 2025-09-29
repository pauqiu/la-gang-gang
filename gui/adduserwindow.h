#pragma once
#include <QDialog>
#include "Role.h"

namespace Ui {
class addUserWindow;
}

class addUserWindow : public QDialog
{
    Q_OBJECT

public:
    explicit addUserWindow(const QList<Role>& roles, QWidget *parent = nullptr);
    ~addUserWindow();

    QString getUsername() const;
    QString getPassword() const;
    QString getSelectedRole() const;

private:
    Ui::addUserWindow *ui;

private slots:
    void validateInputs();
};
