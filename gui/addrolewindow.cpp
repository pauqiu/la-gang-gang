#include "addrolewindow.h"
#include "ui_addrolewindow.h"
#include <QPushButton>

addRoleWindow::addRoleWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::addRoleWindow)
{
    ui->setupUi(this);

    disconnect(ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    ui->roleMessage->setVisible(false);
    ui->roleDescriptionMessage->setVisible(false);

    if (ui->descriptionInput) {
        ui->descriptionInput->setVisible(false);
    }

    setupPermissionsUI();

    connect(ui->buttonBox->button(QDialogButtonBox::Ok), &QPushButton::clicked,
            this, &addRoleWindow::validateInputs);
}

addRoleWindow::addRoleWindow(const QString &role, const QString &permissions, QWidget *parent)
    : addRoleWindow(parent)
{
    ui->roleInput->setText(role);
    loadPermissionsFromString(permissions);
}

addRoleWindow::~addRoleWindow()
{
    delete ui;
}

void addRoleWindow::setupPermissionsUI()
{
    scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setMinimumHeight(350);
    scrollArea->setMaximumHeight(450);

    QWidget* scrollWidget = new QWidget();
    QVBoxLayout* mainLayout = new QVBoxLayout(scrollWidget);
    mainLayout->setSpacing(10);

    // Agrupar permisos por categoría
    for (int cat = SystemPermissions::SYSTEM_ADMIN; cat <= SystemPermissions::AUDIT; cat++) {
        SystemPermissions::Category category = static_cast<SystemPermissions::Category>(cat);

        QGroupBox* groupBox = new QGroupBox(
            QString::fromStdString(SystemPermissions::getCategoryName(category))
            );
        groupBox->setStyleSheet(
            "QGroupBox { "
            "   font-weight: bold; "
            "   border: 2px solid #cccccc; "
            "   border-radius: 5px; "
            "   margin-top: 10px; "
            "   padding-top: 10px; "
            "} "
            "QGroupBox::title { "
            "   subcontrol-origin: margin; "
            "   left: 10px; "
            "   padding: 0 5px 0 5px; "
            "}"
            );

        QVBoxLayout* groupLayout = new QVBoxLayout();
        groupLayout->setSpacing(5);

        // Obtener permisos de esta categoría
        auto perms = SystemPermissions::getPermissionsByCategory(category);

        for (const auto& perm : perms) {
            QCheckBox* checkbox = new QCheckBox(
                QString::fromStdString(perm.displayName)
                );
            checkbox->setObjectName(QString::fromStdString(perm.id));
            checkbox->setStyleSheet("QCheckBox { font-weight: normal; padding: 2px; }");
            permissionCheckboxes[perm.id] = checkbox;
            groupLayout->addWidget(checkbox);
        }

        groupBox->setLayout(groupLayout);
        mainLayout->addWidget(groupBox);
    }

    mainLayout->addStretch();
    scrollWidget->setLayout(mainLayout);
    scrollArea->setWidget(scrollWidget);

    QVBoxLayout* dialogLayout = qobject_cast<QVBoxLayout*>(this->layout());
    if (dialogLayout) {
        dialogLayout->insertWidget(2, scrollArea);
    } else {
        QVBoxLayout* newLayout = new QVBoxLayout(this);
        newLayout->addWidget(ui->roleInput);
        newLayout->addWidget(scrollArea);
        newLayout->addWidget(ui->buttonBox);
        setLayout(newLayout);
    }
}

void addRoleWindow::loadPermissionsFromString(const QString& permissions)
{
    if (permissions.isEmpty()) return;

    QStringList permList = permissions.split(',', Qt::SkipEmptyParts);

    for (const QString& perm : permList) {
        QString trimmed = perm.trimmed();
        std::string permId = trimmed.toStdString();

        auto it = permissionCheckboxes.find(permId);
        if (it != permissionCheckboxes.end()) {
            it->second->setChecked(true);
        }
    }
}

QString addRoleWindow::getRole() const
{
    return ui->roleInput->text();
}

QString addRoleWindow::getPermissions() const
{
    QStringList selectedPerms;

    for (const auto& pair : permissionCheckboxes) {
        if (pair.second->isChecked()) {
            selectedPerms.append(QString::fromStdString(pair.first));
        }
    }

    return selectedPerms.join(",");
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

    bool hasPermissions = false;
    for (const auto& pair : permissionCheckboxes) {
        if (pair.second->isChecked()) {
            hasPermissions = true;
            break;
        }
    }

    if (!hasPermissions) {
        ui->roleDescriptionMessage->setText("Select at least one permission");
        ui->roleDescriptionMessage->setVisible(true);
        valid = false;
    } else {
        ui->roleDescriptionMessage->setVisible(false);
    }

    if (valid) {
        accept();
    }
}
