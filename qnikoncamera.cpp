#include "qnikoncamera.h"
#include "qnikonplugin.h"

#include "dcrimage.h"

static void * g_pNikonCamera = NULL;

QNikonCamera::QNikonCamera()
{
	imageData = NULL;
	_properties = NULL;
	_imagecount = 0;
	_imagedestdir = QDir::currentPath();
	g_bFileRemoved = false;
}

QNikonCamera::~QNikonCamera()
{

}

int QNikonCamera::QCConnect(){

   bool success = false;

    qDebug(qPrintable("Trying to connect to Nikon"));


	//g_pMAIDEntryPoint = pMAIDEntryPoint;
    //if(success) {
        pRefSrc = GetRefChildPtr_ID(pRefMod, cameraID);
        if(!pRefSrc) {
            // create source object and RefSrc structure
            if(AddChild(pMAIDEntryPoint, pRefMod, cameraID)) {
                qDebug(qPrintable("Opened source object"));
            } else {
                qDebug(qPrintable("Couldn't open source object"));
                return false;
            }
            pRefSrc = GetRefChildPtr_ID(pRefMod, cameraID );
			QNikonCBWrapper::getInstance()->registercamera(pRefSrc, this);
        }

        // get CameraType
        if(CheckCapabilityOperation(pRefSrc, kNkMAIDCapability_CameraType, kNkMAIDCapOperation_Get)) {
            success = Command_CapGet(pMAIDEntryPoint, pRefSrc->pObject, kNkMAIDCapability_CameraType, kNkMAIDDataType_UnsignedPtr, (NKPARAM)&ulCameraType, NULL, NULL);
            if(!success) {
                qDebug(qPrintable("Failed to get camera type"));
                //return false;
            }
        }

        NkMAIDString name;
        success = Command_CapGet(pMAIDEntryPoint, pRefSrc->pObject, kNkMAIDCapability_Name, kNkMAIDDataType_StringPtr, (NKPARAM)&name, NULL, NULL);
        //if(success) {
        //    cameraName = QString("Nikon ") + QString((const char *) name.str);
        //} else {
        //    qDebug(qPrintable("Failed to get camera name"));
        //    return false;
        //}

        /*
            Header Size
            -----------
            D7000/D5100: 384
            D3/D3S/D3x/D300/D700/D300S: 64
            D90: 128
        */
        QString model = QString((const char *) name.str);

		if(model.compare(QString("D7000"), Qt::CaseInsensitive)
           || model.compare(QString("D5100"), Qt::CaseInsensitive)) {
            vfHeaderSize = 384;
        } else if(model.compare(QString("D90"), Qt::CaseInsensitive)) {
            vfHeaderSize = 64;
        } else {
            vfHeaderSize = 128;
        }

		// get async rate

		if(GetCapInfo(pRefMod, kNkMAIDCapability_AsyncRate) != NULL) {
			qDebug(qPrintable("Async rate capability supported"));
		} else {
			qDebug(qPrintable("Async rate capability not supported"));
		}


		unsigned long asyncRate = 0;
		success = Command_CapGet(pMAIDEntryPoint, pRefMod->pObject, kNkMAIDCapability_AsyncRate, kNkMAIDDataType_UnsignedPtr, (NKPARAM)&asyncRate, NULL, NULL );
		if(success) {
			qDebug(qPrintable("Async rate: %u ms"), asyncRate);
			if(asyncRate == 0) {
				asyncRate = 100;
			}
			_asynctimer = new QTimer(this);
			connect(_asynctimer, &QTimer::timeout, this, &QNikonCamera::on_asynctimer_timeout);
			_asynctimer->start(asyncRate);
		} else {
			qDebug(qPrintable("Failed to get async rate"));
		}
    //}

	_properties = getallproperties();
    cameraConnected = success;
	if(success) 	g_pNikonCamera = this;

}

int QNikonCamera::QCDisconnect() {
	bool success = false;

    if(pRefMod) {
        success = RemoveChild(pMAIDEntryPoint,pRefMod, cameraID);
        if(success) {
            qDebug(qPrintable("Removed source object"));
        } else {
            qDebug(qPrintable("Failed to remove source object"));
        }

        success = Close_Module(pMAIDEntryPoint, pRefMod );
        if(success) {
            qDebug(qPrintable("Closed module"));
        } else {
            qDebug(qPrintable("Failed to close module"));
        }

		pRefSrc = NULL;
    }

	return success;
}

void QNikonCamera::on_asynctimer_timeout() {
     Command_Async(pMAIDEntryPoint, pRefSrc->pObject);
}

void QNikonCamera::capture(int seconds) {
    //capHandler = receiver;
    bool success = true;

//    if(viewfinderOn) {
//       SetLiveView(false);
//	}
    success = IssueProcess(pMAIDEntryPoint, pRefSrc, kNkMAIDCapability_Capture);
	
	//if(success) AcquireImage(0);
}
void QNikonCamera::setbulbmode(bool bulb) {}

QImage QNikonCamera::getImage() { return QImage();}
QString QNikonCamera::identifier() { return QString("%0 (%1)").arg(_model).arg(cameraID); }
void QNikonCamera::setSelected() {}


QCameraProperties *QNikonCamera::getCameraProperties() { 
	if(_properties)
		delete _properties;
	_properties = getallproperties();

	return _properties;
}

QCameraProperties *QNikonCamera::getallproperties()
{

	//if(!Connected(_index))
	//	QCConnect();

	QCameraProperties *props = new QCameraProperties();
	int count = 0;

	props->addProperty(QCameraProperties::ResolutionMode, GetQualityList());

	QCameraProperty *camProp = new QCameraProperty("FNumbers");

	QList<QString> fnumber = GetList(kNkMAIDCapability_Aperture);
	for(int i=0; i < fnumber.count(); i++)
	{
		camProp->appendValue(fnumber.at(i),fnumber.at(i));
	}
	camProp->setCurrentValue(fnumber.at(GetValueIndex(kNkMAIDCapability_Aperture)));

	props->addProperty(QCameraProperties::Aperture, camProp);

	camProp = new QCameraProperty("Exposures");

	QList<QString> exposures = GetModeList(kNkMAIDCapability_ExposureMode);
	for(int i=0; i < exposures.count(); i++)
	{
		camProp->appendValue(exposures.at(i),exposures.at(i));
	}
	if(exposures.count() > 0)
		camProp->setCurrentValue(exposures.at(GetValueIndex(kNkMAIDCapability_ExposureMode)));

	props->addProperty(QCameraProperties::ExposureMode, camProp);

	camProp = new QCameraProperty("ISOs");
	QList<QString> isos = GetList(kNkMAIDCapability_Sensitivity);
	for(int i=0; i < isos.count(); i++)
	{
		camProp->appendValue(isos.at(i),isos.at(i));
	}
	camProp->setCurrentValue(isos.at(GetValueIndex(kNkMAIDCapability_Sensitivity)));

	props->addProperty(QCameraProperties::Iso, camProp);


	camProp = new QCameraProperty("ExposureTimes");
	QList<QString> shutter = GetList(kNkMAIDCapability_ShutterSpeed);
	for(int i=0; i < shutter.count(); i++ )
	{
		camProp->appendValue(shutter.at(i),shutter.at(i));
	}

	camProp->setCurrentValue(shutter.at(GetValueIndex(kNkMAIDCapability_ShutterSpeed)));
	props->addProperty(QCameraProperties::ExposureTimes, camProp);

	camProp = new QCameraProperty("WhiteBalance");
	QList<QString> wb = GetList(kNkMAIDCapability_WBMode);
	for(int i=0; i < wb.count(); i++ )
	{
		camProp->appendValue(wb.at(i),wb.at(i));
	}

	camProp->setCurrentValue(wb.at(GetValueIndex(kNkMAIDCapability_WBMode)));
	props->addProperty(QCameraProperties::WhiteBalanceMode, camProp);

	return props;
}



QCameraProperty *QNikonCamera::getCameraProperty(QCameraProperties::QCameraPropertyTypes prop) { return _properties->getCameraProperty(prop); }

void QNikonCamera::setCameraProperty(QCameraProperties::QCameraPropertyTypes prop, QVariant value) {
	QCameraProperty *camProp = _properties->getCameraProperty(prop);

	switch(prop){
		case QCameraProperties::QCameraPropertyTypes::Aperture:
			SetValue(kNkMAIDCapability_Aperture,camProp->values().count(), camProp->values().indexOf(value));
			break;
		case QCameraProperties::QCameraPropertyTypes::ExposureTimes:
			SetValue(kNkMAIDCapability_ShutterSpeed,camProp->values().count(), camProp->values().indexOf(value));
			break;
		case QCameraProperties::QCameraPropertyTypes::Iso:
			SetValue(kNkMAIDCapability_Sensitivity,camProp->values().count(), camProp->values().indexOf(value));
			break;
		case QCameraProperties::QCameraPropertyTypes::ResolutionMode:
			SetQuality(value.toString());
			break;
		default:
			break;
	}

}
	
	

void QNikonCamera::toggleLiveView(bool onoff) {}
	
int QNikonCamera::batteryLevel() {
	int batterylevel = 0;


	if(GetCapInfo(pRefMod, kNkMAIDCapability_BatteryLevel) != NULL) {
		qDebug(qPrintable("Battery Level supported"));
	} else {
		qDebug(qPrintable("Battery Level not supported"));
	}

	bool success = Command_CapGet(pMAIDEntryPoint,pRefMod->pObject, kNkMAIDCapability_BatteryLevel, kNkMAIDDataType_IntegerPtr, (NKPARAM)&batterylevel, NULL, NULL );
	if(success) {
		return (int)batterylevel;
	} else {
		qDebug(qPrintable("Failed to get battery level"));
	}
	return 0;
}

bool QNikonCamera::hasBulbMode() {return false;}
bool QNikonCamera::canSetBulbMode() {return false;}
bool QNikonCamera::hasLiveView() {return false;}
bool QNikonCamera::canStreamLiveView() {return false;}

void QNikonCamera::initializeLiveView(){}
QPixmap *QNikonCamera::getLiveViewImage() { return 0;}
void QNikonCamera::endLiveView() {}

void QNikonCamera::lockUI() {}
void QNikonCamera::unlockUI(){}

void QNikonCamera::notifypropertychanged(QCameraProperties::QCameraPropertyTypes prop, QVariant value) {}
bool QNikonCamera::SelectItem(tagRefObj* pRefObj, unsigned long *pulItemID)
{
	BOOL	bRet;
	NkMAIDEnum	stEnum;
	ULONG	i;

	LPNkMAIDCapInfo pCapInfo = GetCapInfo( pRefObj, kNkMAIDCapability_Children );
	if ( pCapInfo == NULL ) return false;

	// check data type of the capability
	if ( pCapInfo->ulType != kNkMAIDCapType_Enum ) return false;
	// check if this capability suports CapGet operation.
	if ( !CheckCapabilityOperation( pRefObj, kNkMAIDCapability_Children, kNkMAIDCapOperation_Get ) ) return false;

	bRet = Command_CapGet(pMAIDEntryPoint, pRefObj->pObject, kNkMAIDCapability_Children, kNkMAIDDataType_EnumPtr, (NKPARAM)&stEnum, NULL, NULL );
	if( bRet == false ) return false;

	// check the data of the capability.
	if ( stEnum.ulElements == 0 ) {
	    qDebug(qPrintable("No images returned"));
	    return false;
	}

	// check the data of the capability.
	if ( stEnum.wPhysicalBytes != 4 ) return false;
    printf("3\n");
	// allocate memory for array data
	stEnum.pData = malloc( stEnum.ulElements * stEnum.wPhysicalBytes );
	if ( stEnum.pData == NULL ) return false;
	// get array data
	bRet = Command_CapGetArray(pMAIDEntryPoint, pRefObj->pObject, kNkMAIDCapability_Children, kNkMAIDDataType_EnumPtr, (NKPARAM)&stEnum, NULL, NULL );
	if( bRet == false ) {
		free( stEnum.pData );
		return false;
	}

	// show the list of selectable Items
//	for ( i = 0; i < stEnum.ulElements; i++ )
//        qDebug(to_wxStr(i) + _T(". Internal ID = ") + to_wxStr(((ULONG*)stEnum.pData)[i]));

    *pulItemID = ((ULONG*)stEnum.pData)[0];
    free( stEnum.pData );
	return true;
}
bool QNikonCamera::AcquireImage(unsigned long ulItemID) {
    bool success = true;
    //wxMutexLocker locker(cameraMutex);   // lock camera
	//QString filename;

    success = this->SelectItem(pRefSrc, &ulItemID);
    if(!success) {
        qDebug(qPrintable("Image item not found."));
        return false;
    }

    LPRefObj pRefItm = GetRefChildPtr_ID(pRefSrc, ulItemID);
    if (pRefItm == NULL)
    {
        if (!AddChild(pMAIDEntryPoint,pRefSrc, ulItemID)) {
            qDebug(qPrintable("Item object can't be opened."));
            return false;
        }
        pRefItm = GetRefChildPtr_ID(pRefSrc, ulItemID);
    }

    LPRefObj pRefDat = GetRefChildPtr_ID( pRefItm, kNkMAIDDataObjType_Image );
    if (pRefDat == NULL)
    {
        if (!AddChild(pMAIDEntryPoint,pRefItm, kNkMAIDDataObjType_Image)) {
            qDebug(qPrintable("Image object can't be opened."));
            return false;
        }
        pRefDat = GetRefChildPtr_ID(pRefItm, kNkMAIDDataObjType_Image);
    }

    qDebug(qPrintable("Issue image acquire"));

    g_bFileRemoved = false;

	QNikonCBWrapper::getInstance()->registerfordata(pRefDat, this);

    success = IssueAcquire(pMAIDEntryPoint, pRefDat);
    if (!success || !imageData) {
        qDebug(qPrintable("Unable to acquire image."));
        return false;
    } else {
        qDebug(qPrintable("Image acquired"));
    }


	QNikonCBWrapper::getInstance()->unregisterfordata(pRefDat);
    //SendImageDataEvent(capHandler, imageData, imageDataSize);


	_imagecount++;
	char exiftag[] = {0xff,0xd8, 0xff, 0xe1,0xff,0xfe,0x45,0x78,0x69,0x66};
	QFile file;
	if(strncmp(imageData, exiftag,10) != 0)
		file.setFileName(QString("%1/%2_%4.%3").arg(_imagedestdir).arg(_nameprefix).arg("nef").arg(_imagecount,4,10,QChar('0')));
	else
		file.setFileName(QString("%1/%2_%4.%3").arg(_imagedestdir).arg(_nameprefix).arg("jpg").arg(_imagecount,4,10,QChar('0')));

	file.open(QIODevice::WriteOnly);
	QDataStream ds( &file);

	ds.writeRawData((const char *)imageData, imageDataSize);

	file.close();

	QImage image;
	if(strncmp(imageData, exiftag,10) != 0)
	{
		//Do raw processing
		DcRImage dcraw;



		//incase the of long shutter exposure and camera hasn't finished writing and it grabs the pic before and it turns out to be jpg
		if(dcraw.isRaw(QString("%1/%2_%4.%3").arg(_imagedestdir).arg(_nameprefix).arg("nef").arg(_imagecount,4,10,QChar('0')))){
			//if(_usedarkframe && QFile::exists(_darkframe))
			//{
			//	QStringList args;
			//	args += "dcrawqt";
			//	args += "-T";
			//	args += "-c";
			//	args += QString("-K %1").arg(_darkframe); 

			//	dcraw.load(filename, args);
			//}
			//else
				dcraw.loadthumbnail(QString("%1/%2_%4.%3").arg(_imagedestdir).arg(_nameprefix).arg("nef").arg(_imagecount,4,10,QChar('0')));

			//QByteArray *image =dcraw.GetImage(previewFile.absoluteFilePath());

			image = dcraw.getthumbimage(); //.loadFromData(*image);
		}
	}
	else
	{
		image.load(QString("%1/%2_%4.%3").arg(_imagedestdir).arg(_nameprefix).arg("jpg").arg(_imagecount,4,10,QChar('0')));
	}

	emit image_captured(image);





    free(imageData); imageData = NULL;
    imageDataSize = 0;

    success = RemoveChild( pMAIDEntryPoint,pRefItm, kNkMAIDDataObjType_Image );
	if ( !success ) {
		 qDebug(qPrintable("Failed to remove data object"));
	}

    // if the image data was stored in DRAM, the item has been removed after reading image
    if ( g_bFileRemoved ) {
        success = RemoveChild(pMAIDEntryPoint,pRefSrc, ulItemID);
        pRefItm = NULL;
    }

	if ( pRefItm != NULL ) {
		// if the item object remains, close it and remove from parent link.
		success = RemoveChild(pMAIDEntryPoint,pRefSrc, ulItemID);
	}

    return success;
}

//extern "C" void ModEventCBWrapper(void * refProc, unsigned long ulEvent, unsigned long data) {
//    ((QNikonCamera*)(g_pNikonCamera))->ModEventCB(refProc, ulEvent, data);
//}

void QNikonCamera::ModEventCB(void * refProc, unsigned long ulEvent, unsigned long data ) {
	bool bRet;
	LPRefObj pRefParent = (LPRefObj)refProc, pRefChild = NULL;

	switch(ulEvent) {
		case kNkMAIDEvent_AddChild:
            puts("Mod Event_AddChild\n");
			bRet = AddChild(pMAIDEntryPoint, pRefParent, (SLONG)data );
			if ( bRet == false ) return;
			pRefChild = GetRefChildPtr_ID( pRefParent, (SLONG)data );
			// Enumerate children(Item and Data Objects) and open them.
			bRet = EnumChildrten( pMAIDEntryPoint,pRefChild->pObject );
			if ( bRet == false ) return;
			break;
		case kNkMAIDEvent_RemoveChild:
            puts("Mod Event_RemoveChild\n");
			bRet = RemoveChild( pMAIDEntryPoint,pRefParent, (SLONG)data );
			if ( bRet == false ) return;

			//Disconnect();
			//CameraDisconnected();
			break;
		case kNkMAIDEvent_WarmingUp:
			// The Type0001/2/3/D40/D80/D200 Module does not use this event.
			puts( "Event_WarmingUp to Module object is not supported.\n" );
			break;
		case kNkMAIDEvent_WarmedUp:
			// The Type0001/2/3/D40/D80/D200 Module does not use this event.
			puts( "Event_WarmedUp to Module object is not supported.\n" );
			break;
		case kNkMAIDEvent_CapChange:
            puts("Event_CapChange\n");
			// re-enumerate the capabilities
			if ( pRefParent->pCapArray != NULL ) {
				free( pRefParent->pCapArray );
				pRefParent->ulCapCount = 0;
				pRefParent->pCapArray = NULL;
			}
			bRet = EnumCapabilities(pMAIDEntryPoint, pRefParent->pObject, &(pRefParent->ulCapCount), &(pRefParent->pCapArray), NULL, NULL );
			if ( bRet == false ) return;
			// ToDo: Execute a process when the property of a capability was changed.
			qDebug(qPrintable("Mod: the capability list (CapID=0x%X) was changed"), (int) data);
			break;
		case kNkMAIDEvent_CapChangeValueOnly:
			// ToDo: Execute a process when the value of a capability was changed.
			qDebug(qPrintable("Mod: the value of capability (CapID=0x%X) was changed"), (int) data );
			break;
		case kNkMAIDEvent_OrphanedChildren:
			// ToDo: Close children(Source Objects).
			break;
		default:
			puts( "Detected unknown Event to the Module object.\n" );
    }
}

//extern "C" void SrcEventCBWrapper(void * refProc, unsigned long ulEvent, unsigned long data) {
//    ((QNikonCamera*)(g_pNikonCamera))->SrcEventCB(refProc, ulEvent, data);
//}

void QNikonCamera::ItmEventProcCB(void *refProc, unsigned long ulEvent, unsigned long data)
{
	BOOL bRet;
	LPRefObj pRefParent = (LPRefObj)refProc;

	switch(ulEvent){
		case kNkMAIDEvent_AddChild:
            puts("Itm Event_AddChild\n");
			bRet = AddChild(pMAIDEntryPoint, pRefParent, (SLONG)data );
			if ( bRet == false ) return;
			break;
		case kNkMAIDEvent_RemoveChild:
            puts("Itm Event_RemoveChild\n");
			bRet = RemoveChild( pMAIDEntryPoint,pRefParent, (SLONG)data );
			if ( bRet == false ) return;
			break;
		case kNkMAIDEvent_WarmingUp:
			// The Type0001/2/3/D40/D80/D200 Module does not use this event.
			puts( "Event_WarmingUp to Item object is not supported.\n" );
			break;
		case kNkMAIDEvent_WarmedUp:
			// The Type0001/2/3/D40/D80/D200 Module does not use this event.
			puts( "Event_WarmedUp to Item object is not supported.\n" );
			break;
		case kNkMAIDEvent_CapChange:
            puts("Event_CapChange\n");
			// re-enumerate the capabilities
			if ( pRefParent->pCapArray != NULL ) {
				free( pRefParent->pCapArray );
				pRefParent->ulCapCount = 0;
				pRefParent->pCapArray = NULL;
			}
			bRet = EnumCapabilities(pMAIDEntryPoint, pRefParent->pObject, &(pRefParent->ulCapCount), &(pRefParent->pCapArray), NULL, NULL );
			if ( bRet == false ) return;
			// ToDo: Execute a process when the property of a capability was changed.
			break;
		case kNkMAIDEvent_CapChangeValueOnly:
			// ToDo: Execute a process when the value of a capability was changed.
			printf( "The value of Capability(CapID=0x%X) was changed.\n", (int) data );
			break;
		case kNkMAIDEvent_OrphanedChildren:
			// ToDo: Close children(Data Objects).
			break;
		default:
			puts( "Detected unknown Event to the Item object.\n" );
		}
	}

void QNikonCamera::SrcEventCB(void * refProc, unsigned long ulEvent, unsigned long data) {
	BOOL bRet;
	LPRefObj pRefParent = (LPRefObj)refProc, pRefChild = NULL;
	QCameraProperty *camProp;

	switch(ulEvent){
		case kNkMAIDEvent_AddChild:
            qDebug(qPrintable("Event_AddChild"));
			bRet = AddChild( pMAIDEntryPoint,pRefParent, (SLONG)data );
			if ( bRet == false ) return;
			pRefChild = GetRefChildPtr_ID( pRefParent, (SLONG)data );
			// Enumerate children(Data Objects) and open them.
			bRet = EnumChildrten(pMAIDEntryPoint, pRefChild->pObject );
			if ( bRet == false ) return;

			AcquireImage(0);
			//if(viewfinderOn) {
			//    SetLiveView(true);
			//}

			break;
		case kNkMAIDEvent_RemoveChild:
            qDebug(qPrintable("Event_RemoveChild"));
			bRet = RemoveChild(pMAIDEntryPoint, pRefParent, (SLONG)data );
			if ( bRet == false ) return;
			break;
		case kNkMAIDEvent_WarmingUp:
			// The Type0001/2/3/D40/D80/D200 Module does not use this event.
			qDebug(qPrintable("Event_WarmingUp to Source object is not supported"));
			break;
		case kNkMAIDEvent_WarmedUp:
			// The Type0001/2/3/D40/D80/D200 Module does not use this event.
            qDebug(qPrintable("Event_WarmedUp to Source object is not supported"));
			break;
		case kNkMAIDEvent_CapChange:
            qDebug(qPrintable("Event_CapChange"));
			// re-enumerate the capabilities
			if ( pRefParent->pCapArray != NULL ) {
				free( pRefParent->pCapArray );
				pRefParent->ulCapCount = 0;
				pRefParent->pCapArray = NULL;
			}
			bRet = EnumCapabilities(pMAIDEntryPoint, pRefParent->pObject, &(pRefParent->ulCapCount), &(pRefParent->pCapArray), NULL, NULL );
			if ( bRet == false ) return;
			// ToDo: Execute a process when the property of a capability was changed.
			qDebug(qPrintable("Src: the capability list (CapID=0x%X) was changed"), (int) data);
			getCameraProperties();
            switch(data) {
                case kNkMAIDCapability_ExposureMode:
                    //UpdateModeList(GetModeList(kNkMAIDCapability_ExposureMode));
                    //UpdateMode(GetValue(kNkMAIDCapability_ExposureMode));
                    break;
                case kNkMAIDCapability_Aperture:
                    //UpdateAvList(GetList(kNkMAIDCapability_Aperture));
                    //UpdateAv(GetValue(kNkMAIDCapability_Aperture));
                    break;
                case kNkMAIDCapability_ShutterSpeed:
                    //UpdateTvList(GetList(kNkMAIDCapability_ShutterSpeed));
                   // UpdateTv(GetValue(kNkMAIDCapability_ShutterSpeed));
                    break;
                case kNkMAIDCapability_Sensitivity:
                    //UpdateISOList(GetList(kNkMAIDCapability_Sensitivity));
                    //UpdateISO(GetValue(kNkMAIDCapability_Sensitivity));
                    break;
                case kNkMAIDCapability_WBMode:
                    //UpdateWBList(GetList(kNkMAIDCapability_WBMode));
                    //UpdateWB(GetValue(kNkMAIDCapability_WBMode));
                    break;
                case kNkMAIDCapability_ImageSize:
                case kNkMAIDCapability_CompressionLevel:
                    //UpdateQualityList(GetQualityList());
                    //UpdateQuality(GetQuality());
					camProp=GetQualityList();
					emit camera_property_changed(QCameraProperties::QCameraPropertyTypes::ResolutionMode, camProp->value());
                    break;
            }

			break;
		case kNkMAIDEvent_CapChangeValueOnly:
			// ToDo: Execute a process when the value of a capability was changed.
			qDebug(qPrintable("Src: the value of capability (CapID=0x%X) was changed"), (int) data);
			//getCameraProperties();
            switch(data) {
                case kNkMAIDCapability_ExposureMode:
                    //UpdateMode(GetValue(kNkMAIDCapability_ExposureMode));
					camProp = this->getCameraProperty(QCameraProperties::QCameraPropertyTypes::ExposureMode);
					camProp->setCurrentValue(camProp->values().at(GetValueIndex(kNkMAIDCapability_ExposureMode)));
					emit camera_property_changed(QCameraProperties::QCameraPropertyTypes::ExposureMode, camProp->value());
                    break;
                case kNkMAIDCapability_Aperture:
                   // UpdateAv(GetValue(kNkMAIDCapability_Aperture));
					camProp = this->getCameraProperty(QCameraProperties::QCameraPropertyTypes::Aperture);
					camProp->setCurrentValue(camProp->values().at(GetValueIndex(kNkMAIDCapability_Aperture)));
					emit camera_property_changed(QCameraProperties::QCameraPropertyTypes::Aperture, camProp->value());
                    break;
                case kNkMAIDCapability_ShutterSpeed:
                   // UpdateTv(GetValue(kNkMAIDCapability_ShutterSpeed));
					camProp = this->getCameraProperty(QCameraProperties::QCameraPropertyTypes::ExposureTimes);
					camProp->setCurrentValue(camProp->values().at(GetValueIndex(kNkMAIDCapability_ShutterSpeed)));
					emit camera_property_changed(QCameraProperties::QCameraPropertyTypes::ExposureTimes, camProp->value());
                    break;
                case kNkMAIDCapability_Sensitivity:
                   // UpdateISO(GetValue(kNkMAIDCapability_Sensitivity));
					camProp = this->getCameraProperty(QCameraProperties::QCameraPropertyTypes::Iso);
					camProp->setCurrentValue(camProp->values().at(GetValueIndex(kNkMAIDCapability_Sensitivity)));
					emit camera_property_changed(QCameraProperties::QCameraPropertyTypes::Iso, camProp->value());
                    break;
                case kNkMAIDCapability_WBMode:
                   // UpdateWB(GetValue(kNkMAIDCapability_WBMode));
                    break;
                case kNkMAIDCapability_ImageSize:
                case kNkMAIDCapability_CompressionLevel:
                    //UpdateQuality(GetQuality());
					camProp=GetQualityList();
					emit camera_property_changed(QCameraProperties::QCameraPropertyTypes::ResolutionMode, camProp->value());
                    break;
            }

			break;
		case kNkMAIDEvent_OrphanedChildren:
			// ToDo: Close children(Item Objects).
			break;
		case kNkMAIDEvent_AddPreviewImage:
            qDebug(qPrintable("Event_AddPreviewImage"));
            {
                ULONG ulItemID = (ULONG) data;
                qDebug(qPrintable("Item: 0x%x"), ulItemID);
            }
			break;
		case kNkMAIDEvent_CaptureComplete:
			qDebug(qPrintable("Event_CaptureComplete"));
			{
                ULONG ulItemID = (ULONG) data;
                AcquireImage(ulItemID);
			}
			break;
		default:
			qDebug(qPrintable("Detected unknown Event to the Source object."));
    }
}


void QNikonCamera::DatEventProcCB( NKREF refProc, ULONG ulEvent, NKPARAM data )
{
	BOOL bRet;
	LPRefObj	pRefParent = (LPRefObj)refProc;

	switch(ulEvent){
		case kNkMAIDEvent_AddChild:
			// In data object, the Type0001/2/3/D40/D80/D200 Module does not use this event.
			puts( "Event_AddChild to Data object is not supported.\n" );
			break;
		case kNkMAIDEvent_RemoveChild:
			// The Type0001/2/3/D40/D80/D200 Module does not use this event.
			puts( "Event_RemoveChild to Data object is not supported.\n" );
			break;
		case kNkMAIDEvent_WarmingUp:
			// The Type0001/2/3/D40/D80/D200 Module does not use this event.
			puts( "Event_WarmingUp to Data object is not supported.\n" );
			break;
		case kNkMAIDEvent_WarmedUp:
			// The Type0001/2/3/D40/D80/D200 Module does not use this event.
			puts( "Event_WarmedUp to Data object is not supported.\n" );
			break;
		case kNkMAIDEvent_CapChange:// module notify that a capability is changed.
            puts("Event_CapChange\n");
			// re-enumerate the capabilities
			if ( pRefParent->pCapArray != NULL ) {
				free( pRefParent->pCapArray );
				pRefParent->ulCapCount = 0;
				pRefParent->pCapArray = NULL;
			}
			bRet = EnumCapabilities(pMAIDEntryPoint, pRefParent->pObject, &(pRefParent->ulCapCount), &(pRefParent->pCapArray), NULL, NULL );
			if ( bRet == false ) return;
			// ToDo: Execute a process when the property of a capability was changed.
		case kNkMAIDEvent_CapChangeValueOnly:
			// ToDo: Execute a process when the value of a capability was changed.
			printf( "The value of Capability(CapID=0x%X) was changed.\n", (int) data );
			break;
		case kNkMAIDEvent_OrphanedChildren:
			// The Type0001/2/3/D40/D80/D200 Module does not use this event.
			puts( "Event_OrphanedChildren to Data object is not supported.\n" );
			break;
		default:
			puts( "Detected unknown Event to the Data object.\n" );
	}
}

//extern "C" long DataCBWrapper(void * ref, void * pInfo, void * pData) {
//    return ((QNikonCamera*)(g_pNikonCamera))->DataCB(ref, pInfo, pData);
//}

long QNikonCamera::DataCB(void * ref, void * pInfo, void * pData) {
	LPNkMAIDDataInfo pDataInfo = (LPNkMAIDDataInfo)pInfo;
	LPNkMAIDImageInfo pImageInfo = (LPNkMAIDImageInfo)pInfo;
	LPNkMAIDFileInfo pFileInfo = (LPNkMAIDFileInfo)pInfo;
	ULONG ulTotalSize, ulOffset;
	LPVOID pCurrentBuffer;
	ULONG ulByte;

	qDebug(qPrintable("DataProc called"));

	if ( pDataInfo->ulType & kNkMAIDDataObjType_File ) {
		if( ((LPRefDataProc)ref)->ulOffset == 0 && ((LPRefDataProc)ref)->pBuffer == NULL )
			((LPRefDataProc)ref)->pBuffer = malloc( pFileInfo->ulTotalLength );
		if ( ((LPRefDataProc)ref)->pBuffer == NULL ) {
			qDebug(qPrintable("There is not enough memory."));
			return kNkMAIDResult_OutOfMemory;
		}
		ulOffset = ((LPRefDataProc)ref)->ulOffset;
		pCurrentBuffer = (LPVOID)((ULONG)((LPRefDataProc)ref)->pBuffer + ((LPRefDataProc)ref)->ulOffset);
		memmove( pCurrentBuffer, pData, pFileInfo->ulLength);
		ulOffset += pFileInfo->ulLength;

		if( ulOffset < pFileInfo->ulTotalLength ) {
			// We have not finished the delivery.
			((LPRefDataProc)ref)->ulOffset = ulOffset;
		} else {
		    /*
			if (((LPRefDataProc)ref)->cameraContext)
			{
                ImageData::Ptr imageData = ImageData::New((char*)((LPRefDataProc)ref)->pBuffer, pFileInfo->ulTotalLength, pFileInfo->ulFileDataType);
                ((LPRefDataProc)ref)->cameraContext->SetImageData(imageData);
			}
			else free(((LPRefDataProc)ref)->pBuffer);
			*/

            qDebug(qPrintable("Image data available: 0x%x %u"), ((LPRefDataProc)ref)->pBuffer, pFileInfo->ulTotalLength);
			this->imageData = (char *) ((LPRefDataProc)ref)->pBuffer;
			////imageData = (char *) malloc(pFileInfo->ulTotalLength);
			//memcpy(imageData, ((LPRefDataProc)ref)->pBuffer, pFileInfo->ulTotalLength);
			this->imageDataSize = pFileInfo->ulTotalLength;
			//free(((LPRefDataProc)ref)->pBuffer);

			((LPRefDataProc)ref)->pBuffer = NULL;
			((LPRefDataProc)ref)->ulOffset = 0;
			// If the flag of fRemoveObject in NkMAIDFileInfo structure is true, we should remove this item.
			if ( pFileInfo->fRemoveObject && (pDataInfo->ulType & kNkMAIDDataObjType_Image) )
				g_bFileRemoved = true;
		}
	} else {
		ulTotalSize = pImageInfo->ulRowBytes * pImageInfo->szTotalPixels.h;
		if( ((LPRefDataProc)ref)->ulOffset == 0 && ((LPRefDataProc)ref)->pBuffer == NULL )
			((LPRefDataProc)ref)->pBuffer = malloc( ulTotalSize );
		if ( ((LPRefDataProc)ref)->pBuffer == NULL ) {
			//wxLogMessage(_T("There is not enough memory."));
			return kNkMAIDResult_OutOfMemory;
		}
		ulOffset = ((LPRefDataProc)ref)->ulOffset;
		pCurrentBuffer = (LPVOID)((ULONG)((LPRefDataProc)ref)->pBuffer + ulOffset);
		ulByte = pImageInfo->ulRowBytes * pImageInfo->rData.h;
		memmove( pCurrentBuffer, pData, ulByte );
		ulOffset += ulByte;

		if( ulOffset < ulTotalSize ) {
			// We have not finished the delivery.
			((LPRefDataProc)ref)->ulOffset = ulOffset;
		} else {
			// We have finished the delivery. We will save this file.
			FILE *stream;
			char filename[256], Prefix[16];
			UWORD i = 0;
			if ( pDataInfo->ulType & kNkMAIDDataObjType_Image )
				strcpy(Prefix,"Image");
			else if ( pDataInfo->ulType & kNkMAIDDataObjType_Thumbnail )
				strcpy(Prefix,"Thumb");
			else
				strcpy(Prefix,"Unknown");
			while( true ) {
				sprintf( filename, "%s%03d.raw", Prefix, ++i );
				if ( (stream = fopen(filename, "r") ) != NULL )
					fclose(stream);
				else
					break;
			}
			if ( (stream = fopen(filename, "wb") ) == NULL)
				return kNkMAIDResult_UnexpectedError;
			fwrite(((LPRefDataProc)ref)->pBuffer, 1, ulTotalSize, stream);
			fclose(stream);
			free(((LPRefDataProc)ref)->pBuffer);
			((LPRefDataProc)ref)->pBuffer = NULL;
			((LPRefDataProc)ref)->ulOffset = 0;
			// If the flag of fRemoveObject in NkMAIDFileInfo structure is true, we should remove this item.
			if ( pImageInfo->fRemoveObject && (pDataInfo->ulType & kNkMAIDDataObjType_Image) )
				g_bFileRemoved = true;
		}
	}
	return kNkMAIDResult_NoError;
}

QList<QString> QNikonCamera::GetList(unsigned long capID) {
    bool success;
    bool settable = false;
    QList<QString> values;
    //wxMutexLocker locker(cameraMutex);   // lock camera

    qDebug(qPrintable("GetList()"));

    if (!CheckCapabilityOperation(pRefSrc, capID, kNkMAIDCapOperation_Get)) {
        qDebug(qPrintable("Capability not supported"));
        return values;
    }

    if (CheckCapabilityOperation(pRefSrc, capID, kNkMAIDCapOperation_Set)) {
        qDebug(qPrintable("Capability is settable"));
        settable = true;
    } else {
        qDebug(qPrintable("Capability is not settable"));
        settable = false;
    }

    NkMAIDEnum	stEnum;
    success = Command_CapGet(pMAIDEntryPoint, pRefSrc->pObject, capID, kNkMAIDDataType_EnumPtr, (NKPARAM)&stEnum, NULL, NULL );
    if(!success) {
        qDebug(qPrintable("Failed to get capability"));
        return values;
    }
    stEnum.pData = malloc(stEnum.ulElements * stEnum.wPhysicalBytes);

    success = Command_CapGetArray(pMAIDEntryPoint, pRefSrc->pObject, capID, kNkMAIDDataType_EnumPtr, (NKPARAM)&stEnum, NULL, NULL );

    if(!success) {
        qDebug(qPrintable("Failed to get capability array"));
        free(stEnum.pData);
        return values;
    } else {
        char	    *psStr;
        unsigned	ulCount = 0;
        for (unsigned i = 0; i < stEnum.ulElements; ) {
            psStr = (char*)((ULONG)stEnum.pData + i);
            //qDebug(qPrintable("%0: %1"));
			values.append(psStr);
            i += strlen(psStr) + 1;
            ulCount++;
        }
        //qDebug(qPrintable("Current Setting: %2d: " + values[stEnum.ulValue], (int) stEnum.ulValue);
        //settingLists[capID] = values;
    }
    free(stEnum.pData);

    return settable? values : QList<QString>();
}

unsigned long QNikonCamera::GetValueIndex(unsigned long capID) {
    //wxMutexLocker locker(cameraMutex);   // lock camera
    bool success;
    //std::map<unsigned long, wxArrayString>::iterator it = settingLists.find(capID);
    //if(it == settingLists.end()) {
    //    return wxEmptyString;
    //}


    NkMAIDEnum	stEnum;
    success = Command_CapGet(pMAIDEntryPoint, pRefSrc->pObject, capID, kNkMAIDDataType_EnumPtr, (NKPARAM)&stEnum, NULL, NULL );
    if(!success) {
        qDebug(qPrintable("Failed to get capability"));
        return -1;
    }

    return stEnum.ulValue;
}

QList<QString> QNikonCamera::GetModeList(unsigned long capID) {
    bool success;
    bool settable = false;

	QCameraProperty *camProp = new QCameraProperty("Modes");

    QList<QString> values;
//    wxMutexLocker locker(cameraMutex);   // lock camera

    qDebug(qPrintable("GetList()"));

    if (!CheckCapabilityOperation(pRefSrc, capID, kNkMAIDCapOperation_Get)) {
        qDebug(qPrintable("Capability not supported"));
        return values;
    }

    if (CheckCapabilityOperation(pRefSrc, capID, kNkMAIDCapOperation_Set)) {
        qDebug(qPrintable("Capability is settable"));
        settable = true;
    } else {
        qDebug(qPrintable("Capability is not settable"));
        settable = false;
    }

    NkMAIDEnum	stEnum;
    success = Command_CapGet(pMAIDEntryPoint, pRefSrc->pObject, capID, kNkMAIDDataType_EnumPtr, (NKPARAM)&stEnum, NULL, NULL );
    if(!success) {
        qDebug(qPrintable("Failed to get capability"));
        return values;
    }
    stEnum.pData = malloc(stEnum.ulElements * stEnum.wPhysicalBytes);

    success = Command_CapGetArray(pMAIDEntryPoint, pRefSrc->pObject, capID, kNkMAIDDataType_EnumPtr, (NKPARAM)&stEnum, NULL, NULL );

    if(!success) {
        qDebug(qPrintable("Failed to get capability array"));
        free(stEnum.pData);
        return values;
    } else {
        char psString[32];
        for (unsigned i = 0; i < stEnum.ulElements; i++) {
            GetEnumString(capID, ((ULONG*)stEnum.pData)[i], psString);
            //qDebug(qPrintable("%i: ") + wxString::FromAscii(psString), i);
            values.append(QString(psString));
        }
        //qDebug(qPrintable("Current Setting: %2d: ") + values[stEnum.ulValue], (int) stEnum.ulValue);
        //settingLists[capID] = values;
    }
    free(stEnum.pData);

    return settable? values : QList<QString>();
}

QCameraProperty *QNikonCamera::GetQualityList() {
    QList<QString> sizes = GetList(kNkMAIDCapability_ImageSize);
    QList<QString> compressions = GetList(kNkMAIDCapability_CompressionLevel);

    QList<QString> qualities;

	QCameraProperty *camProp = new QCameraProperty("Resolutions");

	if(sizes.count() > 0){
		for(unsigned i = 0; i < sizes.count(); i++) {
			for(unsigned j = 0; j < compressions.count(); j++) {
				camProp->appendValue(QString("%1 %2").arg(sizes[i]).arg(compressions[j]),QString("%1|%2").arg(sizes[i]).arg(compressions[j]));
			}
		}

		camProp->setCurrentValue(QString("%1|%2").arg(sizes[GetValueIndex(kNkMAIDCapability_ImageSize)]).arg(compressions[GetValueIndex(kNkMAIDCapability_CompressionLevel)]));
	}
	else{
		for(unsigned j = 0; j < compressions.count(); j++) {
			camProp->appendValue(QString("%1").arg(compressions[j]),QString("|%1").arg(compressions[j]));
		}
		camProp->setCurrentValue(QString("|%1").arg(compressions[GetValueIndex(kNkMAIDCapability_CompressionLevel)]));
	}
    return camProp;
}

bool QNikonCamera::SetValue(unsigned long capID, unsigned long count, unsigned long index) {
//    wxMutexLocker locker(cameraMutex);   // lock camera
    bool success;
    NkMAIDEnum stEnum;

    stEnum.ulType = kNkMAIDArrayType_PackedString;
    stEnum.ulElements = count;
    stEnum.ulValue = index;
    stEnum.ulDefault = 0;
    stEnum.wPhysicalBytes = 4;
    stEnum.pData = 0;
    success = Command_CapSet(pMAIDEntryPoint, pRefSrc->pObject, capID, kNkMAIDDataType_EnumPtr, (NKPARAM)&stEnum, NULL, NULL );
    if(!success) {
        qDebug(qPrintable("Failed to set capability"));
    }

    return true;
}

bool QNikonCamera::SetQuality (QString value) {
    QList<QString> sizes = GetList(kNkMAIDCapability_ImageSize);
    QList<QString> compressions = GetList(kNkMAIDCapability_CompressionLevel);

	QStringList sizecomp = value.split("|");

	if(sizecomp.count() == 2 && (sizes.count() == 0 || sizes.indexOf(sizecomp.at(0)) > -1) && compressions.indexOf(sizecomp.at(1)) > -1 )
		return (sizes.count() > 0 ? SetValue(kNkMAIDCapability_ImageSize, sizes.count(), sizes.indexOf(sizecomp.at(0))) : true) && SetValue(kNkMAIDCapability_CompressionLevel, compressions.count(), compressions.indexOf(sizecomp.at(1)));

    return false;
}