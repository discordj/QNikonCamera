#include "qnikoninterface.h"
#include "qnikonplugin.h"

static void * g_pNikonCamera = NULL;

//LPMAIDEntryPointProc	g_pMAIDEntryPoint = NULL;



#ifdef _WINDOWS
//	HINSTANCE	        g_hInstModule = NULL;
//	static HINSTANCE    g_hInstModules[8] = {0};
//	LPMAIDEntryPointProc g_pMAIDEntryPoints[8] = {0};
//	static tagRefObj*   g_pRefMods[8] = {0};
//	#define LOAD_CONTEXT(id) g_hInstModule = g_hInstModules[id]; g_pMAIDEntryPoint = g_pMAIDEntryPoints[id]; 
//    #define SAVE_CONTEXT(id) g_hInstModules[id] = g_hInstModule; g_pMAIDEntryPoints[id] = g_pMAIDEntryPoint;
#else
	CFragConnectionID	g_ConnID = 0;
	short	            g_nModRefNum = -1;
	#define LOAD_CONTEXT(id)
	#define SAVE_CONTEXT(id)
#endif


#ifdef _WINDOWS
static const char * const NikonModules[] = {"Type0001.md3",
											"Type0002.md3",
											"Type0003.md3",
											"Type0004.md3",
                                            "Type0009.md3",
											"D40_Mod.md3",
											"D80_Mod.md3",
											"D200_Mod.md3"};
#else
static const char * const NikonModules[] = {"Type0001 Module.bundle",
											"Type0002 Module.bundle",
											"Type0003 Module.bundle",
											"Type0004 Module.bundle",
                                            "Type0009 Module.bundle",
											"D40 Module.bundle",
											"D80 Module.bundle",
											"D200 Module.bundle"};
#endif

QNikonInterface::QNikonInterface()
	: QCameraInterface()
{

}

QNikonInterface::~QNikonInterface()
{

}


void QNikonInterface::initialize(){
		for(int i = 0; i < 8; i++) {
		QNikonCamera *camera = LoadModule(QString(NikonModules[i]));

		if(0 != camera){
			nikoncameras.append(camera);
		}
	}
}
void QNikonInterface::unload(){}

QList<QCamera *> QNikonInterface::getcameras(){

	return nikoncameras;
}

QString QNikonInterface::name(){ return QString("Nikon Interface"); }
QCamera * QNikonInterface::selectedCamera(){ return 0;}

QNikonCamera *QNikonInterface::LoadModule(QString filename) {
	QNikonCamera *camera;// = new QNikonCamera();
    bool success = true;
		tagRefObj *pRefObj;
		QString modulePath = QDir(QDir::currentPath()+"/Debug/").absoluteFilePath(filename);

#ifdef _WINDOWS
		HINSTANCE hInstModule = QNikonCBWrapper::getInstance()->getModuleInstance(filename);
    if(!hInstModule) { // WINDOWS HACK
        hInstModule = NULL;
#endif
		success = Load_Module(filename.utf16());
        if(success) {
            qDebug(qPrintable(("Loaded ") + filename));
        } else {
            qDebug(qPrintable("Failed to load " + modulePath.toLatin1()));
			QNikonCBWrapper::getInstance()->setCurrentEntryPoint(0);
            return 0;
        }

		QNikonCBWrapper::getInstance()->setCurrentEntryPoint(QNikonCBWrapper::getInstance()->getentrypointbymodule((ushort *)filename.utf16()));
		//g_hInstModule = NULL;
		//g_pMAIDEntryPoint = NULL;

       pRefObj = (LPRefObj) malloc(sizeof(RefObj));
		InitRefObj(pRefObj);
       pRefObj->pObject = (LPNkMAIDObject) malloc(sizeof(NkMAIDObject));

        // load the module file
        ULONG ulModID = 0;
       pRefObj->pObject->refClient = (NKREF) pRefObj;
        qDebug(qPrintable("Opening module object..."));

		//TODO: reg entry point
		success = Command_Open(QNikonCBWrapper::getInstance()->getentrypointbymodule((ushort *)filename.utf16()), NULL,				// When Module_Object will be opend, "pParentObj" is "NULL".
                               pRefObj->pObject,	// Pointer to Module_Object
                               ulModID );			// Module object ID set by Client
        if(success) {
            qDebug(qPrintable("Module object opened"));
        } else {
            qDebug(qPrintable("Couldn't open module object"));
            CloseModule();
			QNikonCBWrapper::getInstance()->setCurrentEntryPoint(0);
            return 0;
        }


        // enumerate the capabilities that the module has
		//TODO: reg entry point
        success = EnumCapabilities(QNikonCBWrapper::getInstance()->getentrypointbymodule((ushort *)filename.utf16()), pRefObj->pObject, &(pRefObj->ulCapCount), &(pRefObj->pCapArray), NULL, NULL );
        if(success) {
            qDebug(qPrintable("Capabilities enumerated"));
        } else {
            qDebug(qPrintable("Failed to enumerate capabilities"));
            CloseModule();
			QNikonCBWrapper::getInstance()->setCurrentEntryPoint(0);
            return 0;
        }

        // set the callback functions (ProgressProc, EventProc and UIRequestProc)
        success = SetProc(QNikonCBWrapper::getInstance()->getentrypointbymodule((ushort *)filename.utf16()), pRefObj);
        if(success) {
            qDebug(qPrintable("Set callback functions"));
        } else {
            qDebug(qPrintable("Failed to set callback functions"));
            CloseModule();
			QNikonCBWrapper::getInstance()->setCurrentEntryPoint(0);
            return 0;
        }

        //	Set the kNkMAIDCapability_ModuleMode.
        if(CheckCapabilityOperation(pRefObj, kNkMAIDCapability_ModuleMode, kNkMAIDCapOperation_Set)) {
            success = Command_CapSet(QNikonCBWrapper::getInstance()->getentrypointbymodule((ushort *)filename.utf16()), pRefObj->pObject, kNkMAIDCapability_ModuleMode, kNkMAIDDataType_Unsigned,
                                     (NKPARAM)kNkMAIDModuleMode_Controller, NULL, NULL);
            if(success) {
                qDebug(qPrintable("Set capability module mode"));
            } else {
                qDebug(qPrintable("Failed to set capability module mode"));
                CloseModule();
				QNikonCBWrapper::getInstance()->setCurrentEntryPoint(0);
                return 0;
            }
        }
#ifdef _WINDOWS
    }
#endif

	//
	// Get connected cameras
	//

	NkMAIDEnum	stEnum;

	LPNkMAIDCapInfo pCapInfo = GetCapInfo( pRefObj, kNkMAIDCapability_Children );
	if ( pCapInfo == NULL ) {
	    qDebug(qPrintable("Failed to get capability info"));
	    CloseModule();
		QNikonCBWrapper::getInstance()->setCurrentEntryPoint(0);
	    return 0;
	}

	// check data type of the capability
	if ( pCapInfo->ulType != kNkMAIDCapType_Enum ) {
	    qDebug(qPrintable("Failed to get capability info"));
	    CloseModule();
		QNikonCBWrapper::getInstance()->setCurrentEntryPoint(0);
	    return 0;
	}
	// check if this capability suports CapGet operation.
	if ( !CheckCapabilityOperation( pRefObj, kNkMAIDCapability_Children, kNkMAIDCapOperation_Get ) ) {
	    qDebug(qPrintable("Doesn't support CapGet operation"));
	    CloseModule();
        return 0;
	}

	success = Command_CapGet(QNikonCBWrapper::getInstance()->getentrypointbymodule((ushort *)filename.utf16()), pRefObj->pObject, kNkMAIDCapability_Children, kNkMAIDDataType_EnumPtr, (NKPARAM)&stEnum, NULL, NULL );
	if(!success || stEnum.wPhysicalBytes != 4) {
	    qDebug(qPrintable("Failed to CapGet"));
	    CloseModule();
		QNikonCBWrapper::getInstance()->setCurrentEntryPoint(0);
	    return 0;
	}

	if ( stEnum.ulElements == 0 ) {
		qDebug(qPrintable("No cameras found"));
		CloseModule();
		QNikonCBWrapper::getInstance()->setCurrentEntryPoint(0);
		return 0;
	}

	// allocate memory for array data
	stEnum.pData = malloc(stEnum.ulElements * stEnum.wPhysicalBytes);
	// get array data
	success = Command_CapGetArray(QNikonCBWrapper::getInstance()->getentrypointbymodule((ushort *)filename.utf16()), pRefObj->pObject, kNkMAIDCapability_Children, kNkMAIDDataType_EnumPtr, (NKPARAM)&stEnum, NULL, NULL );
	if(!success) {
	    qDebug(qPrintable("Failed to get list of cameras"));
		free(stEnum.pData);
		CloseModule();
		QNikonCBWrapper::getInstance()->setCurrentEntryPoint(0);
		return 0;
	}

	// show the list of selectable Sources
	for (unsigned int i = 0; i < stEnum.ulElements; i++ ) {
		qDebug(qPrintable("Camera %d: ID = %d"), (int) i + 1, (int) ((ULONG*)stEnum.pData)[i] );
	}
	camera = new QNikonCamera();
	camera->hInstModule = QNikonCBWrapper::getInstance()->getModuleInstance(filename);
	camera->pMAIDEntryPoint = QNikonCBWrapper::getInstance()->getEntryPoint(camera->hInstModule);// g_pMAIDEntryPoint;

	camera->setCameraId(((ULONG*)stEnum.pData)[0]); // select first camera
	camera->setRefMod(pRefObj);
	free(stEnum.pData);

	success = true;

        NkMAIDString name;
		success = Command_CapGet(QNikonCBWrapper::getInstance()->getentrypointbymodule((ushort *)filename.utf16()), camera->getRefMod()->pObject, kNkMAIDCapability_Name, kNkMAIDDataType_StringPtr, (NKPARAM)&name, NULL, NULL);
        if(success) {
			camera->setModel(QString("Nikon ") + QString((const char *) name.str));
        } else {
            qDebug(qPrintable("Failed to get camera name"));
			QNikonCBWrapper::getInstance()->setCurrentEntryPoint(0);
            return false;
        }
	QNikonCBWrapper::getInstance()->setCurrentEntryPoint(0);
	QNikonCBWrapper::getInstance()->registercamera(camera->getRefMod(), camera);

    return camera;
}

bool QNikonInterface::CloseModule() {
    bool success = true;

#ifndef _WINDOWS
    if(pRefMod) {
        success = RemoveChild(pRefMod, cameraID);
        if(success) {
            qDebug(qPrintable("Removed source object"));
        } else {
            qDebug(qPrintable("Failed to remove source object"));
        }

        success = Close_Module( pRefMod );
        if(success) {
            qDebug(qPrintable("Closed module"));
        } else {
            qDebug(qPrintable("Failed to close module"));
        }

		pRefSrc = NULL;
    }
#endif

	// unload module
#ifdef _WINDOWS
    // unloading the library and loading a new one doesn't work, so don't unload them...
    /*
    if(g_hInstModule) {
        success = FreeLibrary( g_hInstModule );
        if(success) {
            qDebug(qPrintable("Freed library"));
        } else {
            qDebug(qPrintable("Failed to free library"));
        }
        g_hInstModule = NULL;
    }
    */
#else
    if(g_ConnID) {
        CloseConnection( &g_ConnID );
        g_ConnID = 0;
        g_nModRefNum = -1;
    }
#endif

#ifndef _WINDOWS
	 // Free memory blocks allocated in this function.
	if(pRefMod->pObject != NULL)	{
		free(pRefMod->pObject);
		pRefMod->pObject = NULL;
	}
    if(pRefMod) {
        free(pRefMod);
        pRefMod = NULL;
    }

    g_pMAIDEntryPoint = NULL;
#endif

    return success;
 }

Q_PLUGIN_METADATA(IID "com.ctphoto.QCameraInterface/0.1");
