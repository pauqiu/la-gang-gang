#pragma once
#include <QDialog>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QScrollArea>
#include <vector>
#include <map>
#include "permissions_constants.h"

namespace Ui {
class addRoleWindow;
}

class addRoleWindow : public QDialog
{
    Q_OBJECT

public:
    explicit addRoleWindow(QWidget *parent = nullptr);
    addRoleWindow(const QString &role, const QString &permissions, QWidget *parent = nullptr);
    ~addRoleWindow();

    QString getRole() const;
    QString getPermissions() const;

private slots:
    void validateInputs();

private:
    Ui::addRoleWindow *ui;
    std::map<std::string, QCheckBox*> permissionCheckboxes;
    QScrollArea* scrollArea;

    void setupPermissionsUI();
    void loadPermissionsFromString(const QString& permissions);
};
