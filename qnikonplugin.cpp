#include "qnikonplugin.h"

QNikonCBWrapper::QNikonCBWrapper(QObject *parent)
	: QObject(parent)
{

}

QNikonCBWrapper::~QNikonCBWrapper()
{

}

QNikonCBWrapper *QNikonCBWrapper::getInstance(){
	if(!_instancecreated){
		_instance = new QNikonCBWrapper();
		_instancecreated = true;
	}

	return _instance;
}


void QNikonCBWrapper::registercamera(void *src, QNikonCamera *camera){
	_registeredCameras.insert(src, camera);
}


QNikonCamera *QNikonCBWrapper::getcamera(void *src){

	return _registeredCameras[src];
}

void QNikonCBWrapper::registerfordata(void *src, QNikonCamera *camera){
	_dataCBCameras.insert(src, camera);
}

void QNikonCBWrapper::registerdataref(void *refdel, void *dataref){
	_datadev.insert(refdel, dataref);
}

QNikonCamera *QNikonCBWrapper::getdatacamera(void *src){
	return _dataCBCameras[src];
}

void *QNikonCBWrapper::getdatadev(void *src){
	return _datadev[src];
}

void QNikonCBWrapper::unregisterfordata(void *src){
	_dataCBCameras.remove(src);
}

void QNikonCBWrapper::unregisterdataref(void *src){
	_datadev.remove(src);
}

LPMAIDEntryPointProc QNikonCBWrapper::getentrypointbymodule(unsigned short *fname){
	return getEntryPoint(getModuleInstance(QString::fromUtf16(fname)));
}

void QNikonCBWrapper::registermodule(QString string, HINSTANCE hInst){
	_moduleInst.insert(string, hInst);
}

void QNikonCBWrapper::registerentrypoint(void *hInst, LPMAIDEntryPointProc ep){
	_moduleEntryPoints.insert(hInst, ep);
}

extern "C" void ModEventCBWrapper(void * refProc, unsigned long ulEvent, unsigned long data) {
	tagRefObj *p = (tagRefObj *)refProc;
	if(QNikonCBWrapper::getInstance()->getcamera(refProc)==0) {
		p=(tagRefObj *)p->pRefParent;
	}

    QNikonCBWrapper::getInstance()->getcamera(p)->ModEventCB(refProc, ulEvent, data);
}



extern "C" void SrcEventCBWrapper(void * refProc, unsigned long ulEvent, unsigned long data) {

	tagRefObj *p = (tagRefObj *)refProc;
	if(QNikonCBWrapper::getInstance()->getcamera(refProc) == 0) {
		p=(tagRefObj *)p->pRefParent;
	}

    QNikonCBWrapper::getInstance()->getcamera(p)->SrcEventCB(refProc, ulEvent, data);
}

extern "C" void ItmEventProcCBWrapper(void * refProc, unsigned long ulEvent, unsigned long data) {
	tagRefObj *p = (tagRefObj *)refProc;
	if(QNikonCBWrapper::getInstance()->getcamera(refProc) == 0) {
		p=(tagRefObj *)p->pRefParent;
	}

	QNikonCBWrapper::getInstance()->getcamera(p)->ItmEventProcCB(refProc, ulEvent, data);
}

extern "C" void DatEventCBWrapper(void * refProc, unsigned long ulEvent, unsigned long data){
	tagRefObj *p = (tagRefObj *)refProc;
	if(QNikonCBWrapper::getInstance()->getcamera(refProc)==0) {
		p=(tagRefObj *)p->pRefParent;
	}


    QNikonCBWrapper::getInstance()->getcamera(p)->DatEventProcCB(refProc, ulEvent, data);
}

extern "C" long DataCBWrapper(void * ref, void * pInfo, void * pData) {
	return QNikonCBWrapper::getInstance()->getdatacamera(QNikonCBWrapper::getInstance()->getdatadev(ref))->DataCB(ref, pInfo, pData);
}

extern "C" void registerdataref(void *ref, void *dataref){
	QNikonCBWrapper::getInstance()->registerdataref(ref, dataref);
}

extern "C" void unregisterdataref(void *ref){
	QNikonCBWrapper::getInstance()->unregisterdataref(ref);
}

extern "C" void registermoduleinst(unsigned short *string, HINSTANCE hInst){
	QNikonCBWrapper::getInstance()->registermodule(QString::fromUtf16(string), hInst);
}

extern "C" void registerentrypoint(void *inst, LPMAIDEntryPointProc ep){
	QNikonCBWrapper::getInstance()->registerentrypoint(inst, ep);
}

extern "C" LPMAIDEntryPointProc getentrypointbymodule(unsigned short *fname){
	return QNikonCBWrapper::getInstance()->getentrypointbymodule(fname);
}

extern "C" LPMAIDEntryPointProc getentrypointbyref(void *ref){
	if(QNikonCBWrapper::getInstance()->getCurrentEntryPoint()) return QNikonCBWrapper::getInstance()->getCurrentEntryPoint();
	if(ref == 0) return QNikonCBWrapper::getInstance()->getEntryPoint(0);
	return QNikonCBWrapper::getInstance()->getcamera(ref)->pMAIDEntryPoint;
}

bool QNikonCBWrapper::_instancecreated = false;
QNikonCBWrapper *QNikonCBWrapper::_instance = NULL;




