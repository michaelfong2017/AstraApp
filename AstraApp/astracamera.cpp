#include "astracamera.h"

AstraCamera::AstraCamera()
	: QThread()
{
	config_ = new AstraConfig;
}

AstraCamera::~AstraCamera()
{}

void AstraCamera::run()
{
	while (camera_running_)
	{
		std::shared_ptr<ob::FrameSet> frameSet = p.waitForFrames(100);

		auto colorFrame = frameSet->colorFrame();
		auto depthFrame = frameSet->depthFrame();

		cv::Mat colorImage;
		cv::Mat depthImage;
		readColorImage(colorImage, colorFrame);
		readDepthImage(depthImage, depthFrame);

		QImage qColor = convertColorCVToQImage(colorImage);
		QImage qDepth = convertDepthCVToColorizedQImage(depthImage);

		emit framesReady(qColor, qDepth);

		// Point Cloud
		pointCloud.setCreatePointFormat(OB_FORMAT_RGB_POINT);
		std::shared_ptr<ob::Frame> rgbPointsFrame = pointCloud.process(frameSet);

		pointCloud.setCreatePointFormat(OB_FORMAT_POINT);
		std::shared_ptr<ob::Frame> pointsFrame = pointCloud.process(frameSet);

		if (rgbPointsFrame != NULL && pointsFrame != NULL) {
			qDebug() << rgbPointsFrame->dataSize();
			qDebug() << pointsFrame->dataSize();
			sendFrames(rgbPointsFrame, pointsFrame);
		}
	}
}

void AstraCamera::startThread()
{
	start();
}

void AstraCamera::stopThread()
{
	camera_running_ = false;
}

void AstraCamera::open()
{
	auto colorProfiles = p.getStreamProfileList(OB_SENSOR_COLOR);
	auto colorProfile = colorProfiles->getVideoStreamProfile(config_->color_width, config_->color_height, OB_FORMAT_MJPG, config_->fps);
	auto depthProfiles = p.getStreamProfileList(OB_SENSOR_DEPTH);
	auto depthProfile = depthProfiles->getVideoStreamProfile(config_->depth_width, config_->depth_height, OB_FORMAT_Y16, config_->fps);
	auto irProfiles = p.getStreamProfileList(OB_SENSOR_IR);
	auto irProfile = irProfiles->getVideoStreamProfile(config_->depth_width, config_->depth_height, OB_FORMAT_Y16, config_->fps);

	qDebug() << "Chosen color width: " << colorProfile->width();
	qDebug() << "Chosen color height: " << colorProfile->height();
	qDebug() << "Chosen depth width: " << depthProfile->width();
	qDebug() << "Chosen depth height: " << depthProfile->height();

	cfg = std::make_shared< ob::Config >();
	cfg->enableStream(colorProfile);
	cfg->enableStream(depthProfile);
	cfg->enableStream(irProfile);

	cfg->setAlignMode(ALIGN_D2C_SW_MODE);

	// Start our pipeline
	p.start(cfg);

	// Point Cloud
	auto cameraParam = p.getCameraParam();
	pointCloud.setCameraParam(cameraParam);

	camera_running_ = true;
}

void AstraCamera::close()
{
	// TODO
}

void AstraCamera::sendFrames(std::shared_ptr<ob::Frame> rgbPointsFrame, std::shared_ptr<ob::Frame> pointsFrame)
{
	rgbPointsFrameWrapper->setCustomObject(rgbPointsFrame);
	pointsFrameWrapper->setCustomObject(pointsFrame);

	emit rgbPointsFrameWrapper->pointsReady(rgbPointsFrameWrapper, pointsFrameWrapper);
	emit pointsFrameWrapper->pointsReady(rgbPointsFrameWrapper, pointsFrameWrapper);
}

void AstraCamera::readColorImage(cv::Mat& colorImage, std::shared_ptr<ob::ColorFrame> colorFrame)
{
	if (colorFrame == NULL) {
		colorImage = cv::Mat{};
		return;
	}

	//Create Format Conversion Filter
	ob::FormatConvertFilter formatConverFilter;
	if (colorFrame->format() == OB_FORMAT_MJPG) {
		formatConverFilter.setFormatConvertType(FORMAT_MJPG_TO_BGRA);
	}
	else {
		qWarning() << "Only color format OB_FORMAT_MJPG is supported!";
		colorImage = cv::Mat{};
		return;
	}
	colorFrame = formatConverFilter.process(colorFrame)->as<ob::ColorFrame>();

	int width = colorFrame->width();
	int height = colorFrame->height();

	//// .clone() is necessary to prevent release in memory before use. Otherwise, later on when this
	//// cv image needs to be used (e.g. cvtColor() or clone()), there will be access violation error
	//// https://stackoverflow.com/questions/45013214/qt-signal-slot-cvmat-unable-to-read-memory-access-violation
	colorImage = cv::Mat(height, width, CV_8UC4, (void*)colorFrame->data(), cv::Mat::AUTO_STEP).clone();
}

void AstraCamera::readDepthImage(cv::Mat& depthImage, std::shared_ptr<ob::DepthFrame> depthFrame)
{
	if (depthFrame == NULL) {
		depthImage = cv::Mat{};
		return;
	}

	int width = depthFrame->width();
	int height = depthFrame->height();

	depthImage = cv::Mat(height, width, CV_16U, (void*)depthFrame->data(), cv::Mat::AUTO_STEP).clone();
}

QImage AstraCamera::convertColorCVToQImage(cv::Mat cvImage) {
	if (cvImage.empty()) {
		// QImage.isNull() will return true
		return QImage();
	}

	cv::Mat temp;

	cvtColor(cvImage, temp, cv::COLOR_BGRA2RGB);

	QImage qImage((const uchar*)temp.data, temp.cols, temp.rows, temp.step, QImage::Format_RGB888);
	qImage.bits();

	return qImage;
}

QImage AstraCamera::convertDepthCVToQImage(cv::Mat cvImage)
{
	if (cvImage.empty()) {
		// QImage.isNull() will return true
		return QImage();
	}

	//cvImage.convertTo(cvImage, CV_16U, 255.0 / 5000.0, 0.0);

	cv::Mat temp;
	cvtColor(cvImage, temp, cv::COLOR_BGRA2RGBA);

	QImage qImage((const uchar*)temp.data, temp.cols, temp.rows, temp.step, QImage::Format_RGBX64);
	qImage.bits();

	return qImage;
}

QImage AstraCamera::convertDepthCVToColorizedQImage(cv::Mat cvImage) {
	if (cvImage.empty()) {
		// QImage.isNull() will return true
		return QImage();
	}

	cvImage.convertTo(cvImage, CV_8U, 255.0 / 5000.0, 0.0);

	/** Colorize depth image */
	cv::Mat temp;
	colorizeDepth(cvImage, temp);
	/** Colorize depth image END */

	QImage qImage((const uchar*)temp.data, temp.cols, temp.rows, temp.step, QImage::Format_RGB888);
	qImage.bits();

	return qImage;
}

void AstraCamera::colorizeDepth(const cv::Mat& gray, cv::Mat& rgb)
{
	double maxDisp = 255;
	float S = 1.f;
	float V = 1.f;

	rgb.create(gray.size(), CV_8UC3);
	rgb = cv::Scalar::all(0);

	if (maxDisp < 1)
		return;

	for (int y = 0; y < gray.rows; y++)
	{
		for (int x = 0; x < gray.cols; x++)
		{
			uchar d = gray.at<uchar>(y, x);

			// show black
			if (d == 0) {
				rgb.at<cv::Point3_<uchar> >(y, x) = cv::Point3_<uchar>(0.f, 0.f, 0.f);
				continue;
			}

			// show gray color
			if (d == 255) {
				rgb.at<cv::Point3_<uchar> >(y, x) = cv::Point3_<uchar>(200.f, 200.f, 200.f);
				continue;
			}

			unsigned int H = 255 - ((uchar)maxDisp - d) * 280 / (uchar)maxDisp;
			//unsigned int H = ((uchar)maxDisp - d) * 280 / (uchar)maxDisp; // red close, purple far
			unsigned int hi = (H / 60) % 6;

			float f = H / 60.f - H / 60;
			float p = V * (1 - S); // 0.f
			float q = V * (1 - f * S); // 1 - f
			float t = V * (1 - (1 - f) * S); // f

			cv::Point3f res;

			//qDebug() << d << " " << H << " " << hi << f;
			if (hi == 0) // R = V, G = t,  B = p
				res = cv::Point3f(p, t, V);
			if (hi == 1) // R = q, G = V,  B = p
				res = cv::Point3f(p, V, q);
			if (hi == 2) // R = p, G = V,  B = t
				res = cv::Point3f(t, V, p);
			if (hi == 3) // R = p, G = q,  B = V
				res = cv::Point3f(V, q, p);
			if (hi == 4) // R = t, G = p,  B = V
				res = cv::Point3f(V, p, t);
			if (hi == 5) // R = V, G = p,  B = q
				res = cv::Point3f(q, p, V);

			uchar b = (uchar)(std::max(0.f, std::min(res.x, 1.f)) * 255.f);
			uchar g = (uchar)(std::max(0.f, std::min(res.y, 1.f)) * 255.f);
			uchar r = (uchar)(std::max(0.f, std::min(res.z, 1.f)) * 255.f);

			rgb.at<cv::Point3_<uchar> >(y, x) = cv::Point3_<uchar>(b, g, r);

		}
	}
}

// Save colored point cloud data to ply
void AstraCamera::saveRGBPointsToPly(std::shared_ptr<ob::Frame> frame, std::string fileName) {
	int   pointsSize = frame->dataSize() / sizeof(OBColorPoint);
	FILE* fp = fopen(fileName.c_str(), "wb+");
	fprintf(fp, "ply\n");
	fprintf(fp, "format ascii 1.0\n");
	fprintf(fp, "element vertex %d\n", pointsSize);
	fprintf(fp, "property float x\n");
	fprintf(fp, "property float y\n");
	fprintf(fp, "property float z\n");
	fprintf(fp, "property uchar red\n");
	fprintf(fp, "property uchar green\n");
	fprintf(fp, "property uchar blue\n");
	fprintf(fp, "end_header\n");

	OBColorPoint* point = (OBColorPoint*)frame->data();
	for (int i = 0; i < pointsSize; i++) {
		fprintf(fp, "%.3f %.3f %.3f %d %d %d\n", point->x, point->y, point->z, (int)point->r, (int)point->g, (int)point->b);
		point++;
	}

	fflush(fp);
	fclose(fp);
}


// Save point cloud data to ply
void AstraCamera::savePointsToPly(std::shared_ptr<ob::Frame> frame, std::string fileName) {
	int   pointsSize = frame->dataSize() / sizeof(OBPoint);
	FILE* fp = fopen(fileName.c_str(), "wb+");
	fprintf(fp, "ply\n");
	fprintf(fp, "format ascii 1.0\n");
	fprintf(fp, "element vertex %d\n", pointsSize);
	fprintf(fp, "property float x\n");
	fprintf(fp, "property float y\n");
	fprintf(fp, "property float z\n");
	fprintf(fp, "end_header\n");

	OBPoint* point = (OBPoint*)frame->data();
	for (int i = 0; i < pointsSize; i++) {
		fprintf(fp, "%.3f %.3f %.3f\n", point->x, point->y, point->z);
		point++;
	}

	fflush(fp);
	fclose(fp);
}
