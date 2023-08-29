#ifndef FILEUTIL
#define FILEUTIL

#include "QtWidgets"
#include <opencv2/opencv.hpp>

class FileUtil {
public:
	static QString getCurrentDateTimeString();
	static QString getCurrentDateString();
	static QString getPatientDir();
	static QString getTempDir();
	static bool saveCVImage(cv::Mat image, QString savePath, QImage::Format format);
};

#endif