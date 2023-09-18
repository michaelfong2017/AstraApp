#pragma once

#include <QtWidgets>
#include <QThread>

#include <libobsensor/ObSensor.hpp>
#include "libobsensor/hpp/Pipeline.hpp"
#include "libobsensor/hpp/Error.hpp"

#include <opencv2/opencv.hpp>

#include "framewrapper.h"


struct AstraConfig {
	int color_width = 1920;
	int color_height = 1080;
	int depth_width = 640;
	int depth_height = 480;
	int fps = 30;
};

class AstraCamera  : public QThread
{
	Q_OBJECT

public:
	FrameWrapper* rgbPointsFrameWrapper;
	FrameWrapper* pointsFrameWrapper;

	AstraCamera();
	~AstraCamera();

	void startThread();
	void stopThread();
	void open();
	void close();

	static void saveRGBPointsToPly(std::shared_ptr<ob::Frame> frame, std::string fileName);
	static void savePointsToPly(std::shared_ptr<ob::Frame> frame, std::string fileName);


private:
	bool camera_running_ = false;
	AstraConfig* config_;
	std::shared_ptr< ob::Config > cfg;
	ob::Pipeline p;
	ob::PointCloudFilter pointCloud;

	// Member function that handles thread iteration
	void run() override;

	void sendFrames(std::shared_ptr<ob::Frame> rgbPointsFrame, std::shared_ptr<ob::Frame> pointsFrame);

	void readColorImage(cv::Mat& colorImage, std::shared_ptr<ob::ColorFrame> colorFrame = NULL);
	void readDepthImage(cv::Mat& depthImage, std::shared_ptr<ob::DepthFrame> depthFrame = NULL);
	QImage convertColorCVToQImage(cv::Mat);
	QImage convertDepthCVToQImage(cv::Mat);
	QImage convertDepthCVToColorizedQImage(cv::Mat);
	void colorizeDepth(const cv::Mat& gray, cv::Mat& rgb);

signals:
	// A signal sent by our class to notify that there are frames that need to be processed
	void framesReady(QImage frameRGB, QImage frameDepth, QImage frameDepth16bit);
};
