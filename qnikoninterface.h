#ifndef QNIKONINTERFACE_H
#define QNIKONINTERFACE_H

#include <Windows.h>
#include <qcamerainterface.h>

//#include "nikon/CtrlSample.h"

#include "qnikoncamera_global.h"
#include "qnikoncamera.h"

class QNIKONCAMERA_EXPORT QNikonInterface : public QCameraInterface
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID "com.ctphoto.QCameraInterface/0.1");

	Q_INTERFACES(QCameraInterface)

public:
	QNikonInterface();
	~QNikonInterface();


	void initialize();
	void unload();
	QList<QCamera *> getcameras();
	QString name();
	QCamera * selectedCamera();

private:
	QNikonCamera *LoadModule(QString filename);
    bool CloseModule();
	QList<QCamera *> nikoncameras;


};

#endif // QNIKONINTERFACE_H
