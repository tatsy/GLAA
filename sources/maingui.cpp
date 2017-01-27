#include "maingui.h"

#include <QtCore/qelapsedtimer.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qlineedit.h>
#include <QtWidgets/qpushbutton.h>

#include "radiobuttongroup.h"

class MainGui::Ui : public QWidget {
public:
    explicit Ui(QWidget *parent = nullptr)
        : QWidget(parent) {
        layout = new QVBoxLayout(this);
        layout->setAlignment(Qt::AlignTop);
        setLayout(layout);

        aaTypeRadios = new RadioButtonGroup("AA type", this);
        aaTypeRadios->addRadioButton("None", true);
        aaTypeRadios->addRadioButton("SSAA", false);
        aaTypeRadios->addRadioButton("MSAA", false);
        layout->addWidget(aaTypeRadios);

        subsampleLabel = new QLabel("Subsample", this);
        layout->addWidget(subsampleLabel);

        subsampleEdit = new QLineEdit("2", this);
        layout->addWidget(subsampleEdit);

        updateButton = new QPushButton("Update", this);
        layout->addWidget(updateButton);
    }

    virtual ~Ui() {
    }

    RadioButtonGroup *aaTypeRadios;
    QLabel *subsampleLabel;
    QLineEdit *subsampleEdit;
    QPushButton *updateButton;

private:
    QVBoxLayout *layout;
};

MainGui::MainGui(QWidget *parent)
    : QMainWindow(parent) {
    setFont(QFont("Meiryo UI"));

    mainWidget = new QWidget(this);
    mainLayout = new QGridLayout(mainWidget);
    mainWidget->setLayout(mainLayout);
    setCentralWidget(mainWidget);

    viewer = new OpenGLViewer(mainWidget);
    mainLayout->addWidget(viewer, 0, 0);
    mainLayout->setColumnStretch(0, 4);

    ui = new Ui(mainWidget);
    mainLayout->addWidget(ui, 0, 1);
    mainLayout->setColumnStretch(1, 1);

    connect(viewer, SIGNAL(frameSwapped()), this, SLOT(onFrameSwapped()));
    connect(ui->updateButton, SIGNAL(clicked()), this, SLOT(onUpdateButtonClicked()));
}

MainGui::~MainGui() {
}

void MainGui::onFrameSwapped() {
    static bool isStarted = false;
    static QElapsedTimer timer;
    static long long lastTime;
    
    if (!isStarted) {
        isStarted = true;
        timer.start();
    } else if (timer.elapsed() > 500) {
        long long currentTime = timer.elapsed();
        double fps = 1000.0 / (currentTime - lastTime);
        setWindowTitle(QString("FPS: %1").arg(QString::number(fps, 'f', 2)));
        
        timer.restart();
    }
    lastTime = timer.elapsed();
}

void MainGui::onUpdateButtonClicked() {
    viewer->setAAMethod(ui->aaTypeRadios->selectedIndex(),
                        ui->subsampleEdit->text().toInt());
}
