#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_AstraApp.h"
#include <QLabel>
#include <libobsensor/ObSensor.hpp>

#include "framewrapper.h"

class AstraApp : public QMainWindow
{
    Q_OBJECT

public:
    AstraApp(QWidget *parent = nullptr);
    ~AstraApp();

public slots:
    // Slot that will receive frames from the camera
    void receiveFrame(QImage rgb, QImage depth, QImage depth16bit);
    void receivePoints(FrameWrapper* rgbPointsFrameWrapper, FrameWrapper* pointsFrameWrapper);

private:
    Ui::AstraAppClass ui;

    QString patientNumber;
    QString mode = "Back_120_ArmAside";
    QString filepath;

    QImage qColor;
    QImage qDepth;
    QImage qDepth16bit;

    std::shared_ptr<ob::Frame> rgbPointsFrame;
    std::shared_ptr<ob::Frame> pointsFrame;

    void registerRadioButtonOnClicked(QRadioButton* radioButton);
    void updateFilepath();
};
