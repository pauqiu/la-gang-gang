#include <QApplication>
#include <QMainWindow>
#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Create main window
    QMainWindow window;
    window.setWindowTitle("La Gang Gang - Qt Application");
    window.resize(800, 600);

    // Create central widget and layout
    QWidget *centralWidget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout();
    
    // Add a welcome label
    QLabel *label = new QLabel("Welcome to La Gang Gang!");
    label->setAlignment(Qt::AlignCenter);
    label->setStyleSheet("font-size: 24px; font-weight: bold; color: #2c3e50; margin: 20px;");
    
    // Add another label with info
    QLabel *infoLabel = new QLabel("This is a Qt application running in Replit!");
    infoLabel->setAlignment(Qt::AlignCenter);
    infoLabel->setStyleSheet("font-size: 16px; color: #34495e; margin: 10px;");
    
    layout->addWidget(label);
    layout->addWidget(infoLabel);
    
    centralWidget->setLayout(layout);
    window.setCentralWidget(centralWidget);

    // Show the window
    window.show();

    return app.exec();
}