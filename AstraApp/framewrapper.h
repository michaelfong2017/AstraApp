#pragma once

#include <QObject>
#include <libobsensor/ObSensor.hpp>

class FrameWrapper  : public QObject
{
	Q_OBJECT

public:
	FrameWrapper();
	~FrameWrapper();
    std::shared_ptr<ob::Frame> getCustomObject() const { return m_customObject; }
    void setCustomObject(std::shared_ptr<ob::Frame> customObject) { m_customObject = customObject; }

signals:
	void pointsReady(FrameWrapper* rgbPointsFrameWrapper, FrameWrapper* pointsFrameWrapper);

private:
    // Wrapped custom object
    std::shared_ptr<ob::Frame> m_customObject;
};
