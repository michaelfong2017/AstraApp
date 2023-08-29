#include "AstraApp.h"
#include <QtWidgets/QApplication>
#include <QtWidgets>
#include "astracamera.h"
#include "framewrapper.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    AstraApp w;
    w.show();

    AstraCamera* camera = new AstraCamera();
    camera->open();
    camera->startThread();

    camera->rgbPointsFrameWrapper = new FrameWrapper;
    camera->pointsFrameWrapper = new FrameWrapper;

    // Connect the signal from the camera to the slot of the window
    QApplication::connect(camera, &AstraCamera::framesReady, &w, &AstraApp::receiveFrame);
    QApplication::connect(camera->rgbPointsFrameWrapper, &FrameWrapper::pointsReady, &w, &AstraApp::receivePoints);
    QApplication::connect(camera->pointsFrameWrapper, &FrameWrapper::pointsReady, &w, &AstraApp::receivePoints);

    return a.exec();
}
