#include "AstraApp.h"
#include "astracamera.h"
#include "fileutil.h"

AstraApp::AstraApp(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    ui.label->setScaledContents(true);
    ui.label_2->setScaledContents(true);

    ui.pushButton->setEnabled(false);
    ui.pushButton_2->setEnabled(false);

    QObject::connect(ui.plainTextEdit, &QPlainTextEdit::textChanged, [this]() {
        QString t = ui.plainTextEdit->toPlainText();
        patientNumber = t;
        updateFilepath();

        if (t.isEmpty()) {
            ui.pushButton->setEnabled(false);
            ui.pushButton_2->setEnabled(false);
            ui.label_4->setText(QString("Save Location:"));
        }
        else {
            ui.pushButton->setEnabled(true);
            ui.pushButton_2->setEnabled(true);
            ui.label_4->setText("Save Location: " + filepath);
        }
    });

    registerRadioButtonOnClicked(ui.radioButton);
    registerRadioButtonOnClicked(ui.radioButton_2);
    registerRadioButtonOnClicked(ui.radioButton_3);
    registerRadioButtonOnClicked(ui.radioButton_4);
    registerRadioButtonOnClicked(ui.radioButton_5);
    registerRadioButtonOnClicked(ui.radioButton_6);
    registerRadioButtonOnClicked(ui.radioButton_7);
    registerRadioButtonOnClicked(ui.radioButton_8);

    QObject::connect(ui.pushButton, &QPushButton::clicked, [this]() {
        QDir dir(QCoreApplication::applicationDirPath());
        if (dir.cd(filepath)) {
        }
        else
        {
            dir.mkdir(filepath);
            dir.cd(filepath);
        }
        QString colorSavePath = filepath + "/" + patientNumber + "_" + mode + "_RGB_" + FileUtil::getCurrentDateTimeString() + ".png";
        QString depthSavePath = filepath + "/" + patientNumber + "_" + mode + "_D_" + FileUtil::getCurrentDateTimeString() + ".png";
        qColor.save(colorSavePath);
        qDepth.save(depthSavePath);

        // Point Cloud
        std::string rgbPointsSavePath = QString(filepath + "/" + patientNumber + "_" + mode + "_RGB_PLY_" + FileUtil::getCurrentDateTimeString() + ".ply").toStdString();
        std::string pointsSavePath = QString(filepath + "/" + patientNumber + "_" + mode + "_PLY_" + FileUtil::getCurrentDateTimeString() + ".ply").toStdString();
        AstraCamera::saveRGBPointsToPly(rgbPointsFrame, rgbPointsSavePath);
        AstraCamera::savePointsToPly(pointsFrame, pointsSavePath);
    });

    QObject::connect(ui.pushButton_2, &QPushButton::clicked, [this]() {
        QDir dir(QCoreApplication::applicationDirPath());
        if (dir.cd(filepath)) {
        }
        else
        {
            dir.mkdir(filepath);
            dir.cd(filepath);
        }

        QStringList args;

        args << "/select," << QDir::toNativeSeparators(filepath);

        QProcess* process = new QProcess(this);
        process->startDetached("explorer.exe", args);
        });
}


AstraApp::~AstraApp()
{}

void AstraApp::receiveFrame(QImage rgb, QImage depth)
{
    qDebug() << "receiveFrame()";
    if (rgb.width() != 0) {
        ui.label->setPixmap(QPixmap::fromImage(rgb));
        this->qColor = rgb;
    }
    if (depth.width() != 0) {
        ui.label_2->setPixmap(QPixmap::fromImage(depth));
        this->qDepth = depth;
    }
}

void AstraApp::receivePoints(FrameWrapper* rgbPointsFrameWrapper, FrameWrapper* pointsFrameWrapper) {
    qDebug() << "receivePoints()";
    rgbPointsFrame = rgbPointsFrameWrapper->getCustomObject();
    pointsFrame = pointsFrameWrapper->getCustomObject();
}

void AstraApp::registerRadioButtonOnClicked(QRadioButton* radioButton) {
    QString m = radioButton->text();
    QObject::connect(radioButton, &QRadioButton::clicked, [this, m]() {
        mode = m;
    });
};

void AstraApp::updateFilepath() {
    filepath = FileUtil::getPatientDir() + "/" + patientNumber;
}
