#include "fileutil.h"

QString FileUtil::getCurrentDateTimeString() {
    QDateTime dateTime = dateTime.currentDateTime();
    QString currentDateTimeString = dateTime.toString("yyyy-MM-dd_HHmmss");

    return currentDateTimeString;
}

QString FileUtil::getCurrentDateString() {
    QDateTime dateTime = dateTime.currentDateTime();
    QString currentDateString = dateTime.toString("yyyy-MM-dd");

    return currentDateString;
}

QString FileUtil::getPatientDir() {
    QDir dir(QCoreApplication::applicationDirPath());
    QString patientDir = dir.absolutePath() + "/Patient";
    if (dir.cd(patientDir)) {
    }
    else
    {
        dir.mkdir(patientDir);
        dir.cd(patientDir);
    }
    return patientDir;
}

QString FileUtil::getTempDir() {
    QDir dir(QCoreApplication::applicationDirPath());
    QString patientDir = dir.absolutePath() + "/Temp";
    if (dir.cd(patientDir)) {
    }
    else
    {
        dir.mkdir(patientDir);
        dir.cd(patientDir);
    }
    return patientDir;
}

// Can use format QImage::Format_RGB32 for both rgb and depth images.
bool FileUtil::saveCVImage(cv::Mat image, QString savePath, QImage::Format format)
{
    QImageWriter writer(savePath);
    QImage qImage((uchar*)image.data,
        image.cols,
        image.rows,
        image.step,
        format);
    bool success = writer.write(qImage);
    return success;
}
