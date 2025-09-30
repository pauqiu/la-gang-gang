#ifndef ADDROLEWINDOW_H
#define ADDROLEWINDOW_H

#include <QDialog>
#include <QPushButton>

namespace Ui {
class addRoleWindow;
}

class addRoleWindow : public QDialog
{
    Q_OBJECT

    public:
        explicit addRoleWindow(QWidget *parent = nullptr);
        addRoleWindow(const QString &role, const QString &description, QWidget *parent);
        ~addRoleWindow();

        QString getRole() const;
        QString getDescription() const;

    private:
        Ui::addRoleWindow *ui;

    private slots:
        void validateInputs();
};

#endif // ADDROLEWINDOW_H
