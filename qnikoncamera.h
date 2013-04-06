#ifndef QNIKONCAMERA_H
#define QNIKONCAMERA_H

#include <Windows.h>
#include <qcamera.h>
#include "nikon/CtrlSample.h"
#include "qnikoncamera_global.h"


#include <QString>
#include <QDir>


struct tagRefObj;

class QNIKONCAMERA_EXPORT QNikonCamera : public QCamera
{
public:
	QNikonCamera();
	~QNikonCamera();

	 int QCConnect() ;
	 int QCDisconnect() ;

	 void capture(int seconds=0) ;
	 void setbulbmode(bool bulb) ;

	 QImage getImage() ;
	 QString identifier() ;
	 QString model() { return _model; }
	 void setSelected() ;


	 QCameraProperties *getCameraProperties() ;
	 QCameraProperty *getCameraProperty(QCameraProperties::QCameraPropertyTypes prop) ;
	 void setCameraProperty(QCameraProperties::QCameraPropertyTypes prop, QVariant value) ;
	
	void setImageDirectory(QString dir){_imagedestdir = dir;}
	void setImageFilePrefix(QString imagenameprefix){ _nameprefix = imagenameprefix; }


	 void toggleLiveView(bool onoff) ;
	
	 int batteryLevel() ;

	 bool hasBulbMode() ;
	 bool canSetBulbMode() ;
	 bool hasLiveView() ;
	 bool canStreamLiveView() ;

	 void initializeLiveView();
	 QPixmap *getLiveViewImage() ;
	 void endLiveView();

	 void lockUI() ;
	 void unlockUI();

	 tagRefObj *getRefMod() { return pRefMod; }
	 void setRefMod(tagRefObj *refMod) { pRefMod = refMod; }

	 tagRefObj *getRefSrc() { return pRefSrc; }
	 void setRefSrc(tagRefObj *refSrc) { pRefSrc = refSrc; }

	 unsigned long getCameraId() { return cameraID; }
	 void setCameraId(unsigned long id) { cameraID = id; }

	 void setModel(QString mdl){ _model = mdl;}

	bool AcquireImage(unsigned long ulItemID);
	bool SelectItem(tagRefObj* pRefObj, unsigned long *pulItemID);
    void SrcEventCB(void * refProc, unsigned long ulEvent, unsigned long data);
    long DataCB(void * ref, void * pInfo, void * pData);
    void ModEventCB(void * refProc, unsigned long ulEvent, unsigned long data);
	void ItmEventProcCB(void *refProc, unsigned long ulEvent, unsigned long data);
	void DatEventProcCB( NKREF refProc, ULONG ulEvent, NKPARAM data );

	HINSTANCE hInstModule;
	LPMAIDEntryPointProc pMAIDEntryPoint;
protected slots:
	 void notifypropertychanged(QCameraProperties::QCameraPropertyTypes prop, QVariant value) ;
	 void on_asynctimer_timeout();
private:
	QString _model;
    volatile bool cameraConnected;
	unsigned long cameraID;
    unsigned int vfHeaderSize;
	ULONG	ulCameraType;
	tagRefObj* pRefMod;
	tagRefObj* pRefSrc;
	char * imageData;
	size_t imageDataSize;
	QString _imagedestdir;
	QString _nameprefix;
	int _imagecount;
	QTimer *_asynctimer;
	UCHAR g_bFileRemoved;
	bool emitPropChanged;
	ULONG whichProp;

	QCameraProperties *_properties;

	QCameraProperties *QNikonCamera::getallproperties();
	QList<QString> GetList(unsigned long capID);
	QCameraProperty *GetQualityList();
	unsigned long GetValueIndex(unsigned long capID);
	QList<QString> GetModeList(unsigned long capID);

	bool SetValue(unsigned long capID, unsigned long count, unsigned long index);
	bool SetQuality (QString value);
};

#endif // QNIKONCAMERA_H
