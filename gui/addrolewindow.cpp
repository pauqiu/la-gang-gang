#include "addrolewindow.h"
#include "ui_addrolewindow.h"
#include <QPushButton>
#include <sstream>

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

    setupCategoriesUI();

    connect(ui->buttonBox->button(QDialogButtonBox::Ok), &QPushButton::clicked,
            this, &addRoleWindow::validateInputs);
}

addRoleWindow::addRoleWindow(const QString &role, const QString &permissions, QWidget *parent)
    : addRoleWindow(parent)
{
    ui->roleInput->setText(role);
    loadCategoriesFromPermissions(permissions);
}

addRoleWindow::~addRoleWindow()
{
    delete ui;
}

void addRoleWindow::setupCategoriesUI()
{
    scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setMinimumHeight(400);
    scrollArea->setMaximumHeight(500);

    QWidget* scrollWidget = new QWidget();
    QVBoxLayout* mainLayout = new QVBoxLayout(scrollWidget);
    mainLayout->setSpacing(15);

    // Añadir label explicativo
    QLabel* infoLabel = new QLabel("Select the permission categories for this role:");
    infoLabel->setStyleSheet("font-weight: bold; color: #333; margin-bottom: 10px;");
    mainLayout->addWidget(infoLabel);

    // Crear checkbox para cada categoría
    for (const auto& category : SystemPermissions::ALL_CATEGORIES) {
        QGroupBox* groupBox = new QGroupBox(
            QString::fromStdString(category.displayName)
            );
        groupBox->setCheckable(true);
        groupBox->setChecked(false);
        groupBox->setStyleSheet(
            "QGroupBox { "
            "   font-weight: bold; "
            "   border: 2px solid #cccccc; "
            "   border-radius: 5px; "
            "   margin-top: 10px; "
            "   padding: 15px; "
            "} "
            "QGroupBox::title { "
            "   subcontrol-origin: margin; "
            "   left: 10px; "
            "   padding: 0 5px; "
            "}"
            );

        // Layout para descripción
        QVBoxLayout* groupLayout = new QVBoxLayout();

        QLabel* descLabel = new QLabel(QString::fromStdString(category.description));
        descLabel->setStyleSheet("font-weight: normal; font-style: italic; color: #666;");
        descLabel->setWordWrap(true);
        groupLayout->addWidget(descLabel);

        groupBox->setLayout(groupLayout);

        // Guardar referencia
        categoryCheckboxes[category.name] = groupBox;

        mainLayout->addWidget(groupBox);
    }

    mainLayout->addStretch();
    scrollWidget->setLayout(mainLayout);
    scrollArea->setWidget(scrollWidget);

    // Añadir al layout del diálogo
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

void addRoleWindow::loadCategoriesFromPermissions(const QString& permissions)
{
    if (permissions.isEmpty()) return;

    // Convertir permisos a lista
    QStringList permList = permissions.split(',', Qt::SkipEmptyParts);
    std::vector<std::string> perms;
    for (const QString& perm : permList) {
        perms.push_back(perm.trimmed().toStdString());
    }

    // Verificar qué categorías están presentes
    for (const auto& category : SystemPermissions::ALL_CATEGORIES) {
        // Parsear permisos de la categoría
        std::istringstream iss(category.permissions);
        std::string catPerm;
        bool hasAllPerms = true;

        while (std::getline(iss, catPerm, ',')) {
            catPerm.erase(0, catPerm.find_first_not_of(" \t"));
            catPerm.erase(catPerm.find_last_not_of(" \t") + 1);

            if (std::find(perms.begin(), perms.end(), catPerm) == perms.end()) {
                hasAllPerms = false;
                break;
            }
        }

        // Si tiene todos los permisos de la categoría, marcarla
        if (hasAllPerms) {
            auto it = categoryCheckboxes.find(category.name);
            if (it != categoryCheckboxes.end()) {
                it->second->setChecked(true);
            }
        }
    }
}

QString addRoleWindow::getRole() const
{
    return ui->roleInput->text();
}

QString addRoleWindow::getPermissions() const
{
    // Recopilar categorías seleccionadas
    std::vector<std::string> selectedCategories;

    for (const auto& pair : categoryCheckboxes) {
        if (pair.second->isChecked()) {
            selectedCategories.push_back(pair.first);
        }
    }

    // Convertir categorías a permisos
    std::string categoryNames;
    for (size_t i = 0; i < selectedCategories.size(); i++) {
        if (i > 0) categoryNames += ",";
        categoryNames += selectedCategories[i];
    }

    std::string permissions = SystemPermissions::categoryNamesToPermissions(categoryNames);
    return QString::fromStdString(permissions);
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

    // Validar que al menos una categoría esté seleccionada
    bool hasCategories = false;
    for (const auto& pair : categoryCheckboxes) {
        if (pair.second->isChecked()) {
            hasCategories = true;
            break;
        }
    }

    if (!hasCategories) {
        ui->roleDescriptionMessage->setText("Select at least one category");
        ui->roleDescriptionMessage->setVisible(true);
        valid = false;
    } else {
        ui->roleDescriptionMessage->setVisible(false);
    }

    if (valid) {
        accept();
    }
}
