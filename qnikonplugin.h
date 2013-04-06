#ifndef QNIKONPLUGIN_H
#define QNIKONPLUGIN_H


//#include "nikon/CtrlSample.h"
#include "qnikoncamera_global.h"
#include "qnikoncamera.h"
#include <QObject>

class QNikonCBWrapper : public QObject
{
	Q_OBJECT

public:
	~QNikonCBWrapper();
	static QNikonCBWrapper *getInstance();

	void registercamera(void *src, QNikonCamera *camera);
	void registerdataref(void *refdel, void *dataref);

	void unregisterfordata(void *src);
	void unregisterdataref(void *src);

	QNikonCamera *getcamera(void *src);
	QNikonCamera *getdatacamera(void *src);
	void *QNikonCBWrapper::getdatadev(void *src);

	void registermodule(QString string, HINSTANCE hInst);
	void registerentrypoint(void *hInst, LPMAIDEntryPointProc ep);
	void registerfordata(void *src, QNikonCamera *camera);

	HINSTANCE getModuleInstance(QString file) { return _moduleInst[file]; }
	LPMAIDEntryPointProc getEntryPoint(HINSTANCE inst) { if(_currentEP) return _currentEP; return _moduleEntryPoints[inst]; }
	void setCurrentEntryPoint(LPMAIDEntryPointProc ep) { _currentEP = ep; }
	LPMAIDEntryPointProc getentrypointbymodule(unsigned short *fname);
	LPMAIDEntryPointProc getCurrentEntryPoint() { return _currentEP; }
private:
	QNikonCBWrapper(QObject *parent=0);
	static bool _instancecreated;
	static QNikonCBWrapper *_instance;


	QMap<void *, QNikonCamera *> _registeredCameras;

	QMap<void *, QNikonCamera *> _dataCBCameras;
	QMap<void *, void *> _datadev;

	QMap<void *, LPMAIDEntryPointProc> _moduleEntryPoints;
	QMap<QString, HINSTANCE> _moduleInst;

	LPMAIDEntryPointProc _currentEP;
};


#endif // QNIKONPLUGIN_H
