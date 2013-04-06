//================================================================================================
// Copyright Nikon Corporation - All rights reserved
//
// View this file in a non-proportional font, tabs = 3
//================================================================================================

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "Maid3.h"
#include "Maid3d1.h"

/////////////////////////////////////////////////////////////////////////////
// Structures

#pragma pack(push, 2)

	typedef struct tagRefObj
	{
		LPNkMAIDObject	pObject;
		SLONG lMyID;
		LPVOID pRefParent;
		ULONG ulChildCount;
		LPVOID pRefChildArray;
		ULONG ulCapCount;
		LPNkMAIDCapInfo pCapArray;
	} RefObj, *LPRefObj;

	typedef struct tagRefCompletionProc
	{
//		BOOL bEnd;
		ULONG* pulCount;
		NKERROR nResult;
//		LPVOID pcProgressDlg;
		LPVOID pRef;
	} RefCompletionProc, *LPRefCompletionProc;

	typedef struct tagRefDataProc
	{
		LPVOID	pBuffer;
		ULONG	ulOffset;
		ULONG	ulTotalLines;
		SLONG	lID;
	} RefDataProc, *LPRefDataProc;

	typedef struct tagPSDFileHeader
	{
		char	type[5];
		char	space11[1];
		char	space01[6];
		short	Planecount; 	//0004 if RGB, this is 0003
		long	rowPixels;
		long	columnPixels;
		short	bits; 			//0008 means 8bit. 16bit also supported
		short	mode; 			//0004 means CMYK, Gray -- 1, RGB -- 3
		char	space02[14];
	} PSDFileHeader, *LPPSDFileHeader;

	typedef struct tagRefSpecialCap
	{
		ULONG ulCapID;
		ULONG ulCapValue;
//		ULONG ulCapType;
		ULONG ulUIID;
	} RefSpecialCap, *LPRefSpecialCap;

#pragma pack(pop)


/////////////////////////////////////////////////////////////////////////////
// Prototype

SLONG	CallMAIDEntryPoint(
		LPMAIDEntryPointProc ep,
		LPNkMAIDObject	pObject,				// module, source, item, or data object
		ULONG				ulCommand,			// Command, one of eNkMAIDCommand
		ULONG				ulParam,				// parameter for the command
		ULONG				ulDataType,			// Data type, one of eNkMAIDDataType
		NKPARAM			data,					// Pointer or long integer
		LPNKFUNC			pfnComplete,		// Completion function, may be NULL
		NKREF				refComplete );		// Value passed to pfnComplete
BOOL	Command_Async( LPMAIDEntryPointProc ep, LPNkMAIDObject pObject);
BOOL	Command_CapSet(  LPMAIDEntryPointProc ep,LPNkMAIDObject pObject, ULONG ulParam, ULONG ulDataType, NKPARAM pData, LPNKFUNC pfnComplete, NKREF refComplete );
BOOL	Command_CapGet(  LPMAIDEntryPointProc ep,LPNkMAIDObject pObject, ULONG ulParam, ULONG ulDataType, NKPARAM pData, LPNKFUNC pfnComplete, NKREF refComplete );
BOOL	Command_CapStart(  LPMAIDEntryPointProc ep,LPNkMAIDObject pObject, ULONG ulParam, LPNKFUNC pfnComplete, NKREF refComplete, SLONG* pnResult );
BOOL	Command_CapGetArray(  LPMAIDEntryPointProc ep,LPNkMAIDObject pObject, ULONG ulParam, ULONG ulDataType, NKPARAM pData, LPNKFUNC pfnComplete, NKREF refComplete );
BOOL	Command_CapGetDefault(  LPMAIDEntryPointProc ep,LPNkMAIDObject pObject, ULONG ulParam, ULONG ulDataType, NKPARAM pData, LPNKFUNC pfnComplete, NKREF refComplete );
BOOL	Command_Abort( LPMAIDEntryPointProc ep,LPNkMAIDObject pobject, LPNKFUNC pfnComplete, NKREF refComplete);
BOOL	Command_Open(  LPMAIDEntryPointProc ep,LPNkMAIDObject pParentObj, NkMAIDObject* pChildObj, ULONG ulChildID );
BOOL	Command_Close(  LPMAIDEntryPointProc ep,LPNkMAIDObject pObject );

void	CALLPASCAL CALLBACK ModEventProc( NKREF refProc, ULONG ulEvent, NKPARAM data );
void	CALLPASCAL CALLBACK SrcEventProc( NKREF refProc, ULONG ulEvent, NKPARAM data );
void	CALLPASCAL CALLBACK ItmEventProc( NKREF refProc, ULONG ulEvent, NKPARAM data );
void	CALLPASCAL CALLBACK DatEventProc( NKREF refProc, ULONG ulEvent, NKPARAM data );
void	CALLPASCAL CALLBACK ProgressProc( ULONG ulCommand, ULONG ulParam, NKREF refProc, ULONG ulDone, ULONG ulTotal );
ULONG	CALLPASCAL CALLBACK UIRequestProc( NKREF ref, LPNkMAIDUIRequestInfo pUIRequest );
void	CALLPASCAL CALLBACK CompletionProc( LPNkMAIDObject pObject, ULONG ulCommand, ULONG ulParam, ULONG ulDataType, NKPARAM data, NKREF refComplete, NKERROR nResult );
NKERROR	CALLPASCAL CALLBACK DataProc( NKREF ref, LPVOID pDataInfo, LPVOID pData );

void	InitRefObj( LPRefObj pRef );
//BOOL	Search_Module( const char* Path );
#ifdef UNICODE && _WINDOWS
BOOL	Load_Module( const unsigned short *Path );
#else
BOOL	Load_Module( const char *Path );
#endif
BOOL	Close_Module(  LPMAIDEntryPointProc ep,LPRefObj pRefMod );
BOOL	EnumCapabilities( LPMAIDEntryPointProc ep, LPNkMAIDObject pobject, ULONG* pulCapCount, LPNkMAIDCapInfo* ppCapArray, LPNKFUNC pfnComplete, NKREF refComplete );
BOOL	EnumChildrten(  LPMAIDEntryPointProc ep,LPNkMAIDObject pobject );
BOOL	AddChild( LPMAIDEntryPointProc ep, LPRefObj pRefParent, SLONG lIDChild );
BOOL	RemoveChild( LPMAIDEntryPointProc ep, LPRefObj pRefParent, SLONG lIDChild );
BOOL	SetProc( LPMAIDEntryPointProc ep, LPRefObj pRefObj );
BOOL	ResetProc( LPMAIDEntryPointProc ep, LPRefObj pRefObj );
BOOL	IdleLoop( LPMAIDEntryPointProc ep, LPNkMAIDObject pObject, ULONG* pulCount, ULONG ulEndCount );
void WaitEvent(void);

BOOL	SourceCommandLoop( LPRefObj pRefMod, ULONG ulSrcID );
BOOL	ItemCommandLoop( LPRefObj pRefSrc, ULONG ulItemID );
BOOL	ImageCommandLoop( LPRefObj pRefItm, ULONG ulDatID );
BOOL	ThumbnailCommandLoop( LPRefObj pRefItm, ULONG ulDatID );
BOOL	SelectSource( LPRefObj pRefMod, ULONG *pulSrcID );
BOOL	SelectItem( LPRefObj pRefSrc, ULONG *pulItemID );
BOOL	SelectData( LPRefObj pRefItm, ULONG *pulDataType );
BOOL	SetUpCamera1( LPRefObj pRefSrc );
BOOL	SetUpCamera2( LPRefObj pRefSrc );
BOOL	SetUpCamera3( LPRefObj pRefSrc );
BOOL	SetShootingMenu( LPMAIDEntryPointProc ep,LPRefObj pRefSrc );
BOOL	SetCustomSettings(LPMAIDEntryPointProc ep, LPRefObj pRefSrc );
BOOL	SetEnumCapability(LPMAIDEntryPointProc ep, LPRefObj pRefObj, ULONG ulCapID );
BOOL	SetEnumUnsignedCapability( LPMAIDEntryPointProc ep,LPRefObj pRefObj, ULONG ulCapID, LPNkMAIDEnum pstEnum );
BOOL	SetEnumPackedStringCapability( LPMAIDEntryPointProc ep,LPRefObj pRefObj, ULONG ulCapID, LPNkMAIDEnum pstEnum );
BOOL	SetEnumStringCapability(LPMAIDEntryPointProc ep, LPRefObj pRefObj, ULONG ulCapID, LPNkMAIDEnum pstEnum );
BOOL	SetFloatCapability( LPMAIDEntryPointProc ep,LPRefObj pRefObj, ULONG ulCapID );
BOOL	SetBoolCapability( LPMAIDEntryPointProc ep,LPRefObj pRefObj, ULONG ulCapID );
BOOL	SetIntegerCapability( LPMAIDEntryPointProc ep,LPRefObj pRefObj, ULONG ulCapID );
BOOL	SetUnsignedCapability(LPMAIDEntryPointProc ep, LPRefObj pRefObj, ULONG ulCapID );
BOOL	SetStringCapability(LPMAIDEntryPointProc ep, LPRefObj pRefObj, ULONG ulCapID );
BOOL	SetSizeCapability(LPMAIDEntryPointProc ep, LPRefObj pRefObj, ULONG ulCapID );
BOOL	SetDateTimeCapability(LPMAIDEntryPointProc ep, LPRefObj pRefObj, ULONG ulCapID );
BOOL	SetRangeCapability(LPMAIDEntryPointProc ep, LPRefObj pRefObj, ULONG ulCapID );
BOOL	SetWBPresetDataCapability(LPMAIDEntryPointProc ep, LPRefObj pRefObj, NkMAIDWBPresetData* pWBPresetData, char* filename, unsigned long ulCameraType );
BOOL	GetWBPresetDataCapability(LPMAIDEntryPointProc ep, LPRefObj pRefObj, NkMAIDWBPresetData* pWBPresetData );
BOOL	GetPreviewImageCapability(LPMAIDEntryPointProc ep, LPRefObj pRefSrc, ULONG ulItmID );
BOOL	DeleteDramCapability(LPMAIDEntryPointProc ep, LPRefObj pRefItem, ULONG ulItmID );
BOOL	GetLiveViewImageCapability(LPMAIDEntryPointProc ep, LPRefObj pRefSrc, unsigned long ulCameraType );
BOOL	PictureControlDataCapability( LPMAIDEntryPointProc ep,LPRefObj pRefSrc );
BOOL	SetPictureControlDataCapability(LPMAIDEntryPointProc ep, LPRefObj pRefObj, NkMAIDPicCtrlData* pPicCtrlData, char* filename );
BOOL	GetPictureControlDataCapability( LPMAIDEntryPointProc ep,LPRefObj pRefObj, NkMAIDPicCtrlData* pPicCtrlData );
BOOL	GetPictureControlInfoCapability( LPMAIDEntryPointProc ep,LPRefObj pRefSrc );
BOOL	DeleteCustomPictureControlCapability( LPMAIDEntryPointProc ep,LPRefObj pRefSrc );
BOOL	ShowArrayCapability(LPMAIDEntryPointProc ep, LPRefObj pRefObj, ULONG ulCapID );
BOOL	SaveArrayCapability(LPMAIDEntryPointProc ep, LPRefObj pRefObj, ULONG ulCapID, char* filename );
BOOL	GetArrayCapability(LPMAIDEntryPointProc ep, LPRefObj pRefObj, ULONG ulCapID, LPNkMAIDArray pstArray );
BOOL	LoadArrayCapability(LPMAIDEntryPointProc ep, LPRefObj pRefObj, ULONG ulCapID, char* filename );
BOOL	SetNewLut(LPMAIDEntryPointProc ep, LPRefObj pRefSrc );
char*	GetEnumString( ULONG ulCapID, ULONG ulValue, char *psString );
char*	GetUnsignedString( ULONG ulCapID, ULONG ulValue, char *psString );
BOOL	IssueProcess( LPMAIDEntryPointProc ep,LPRefObj pRefSrc, ULONG ulCapID );
BOOL	IssueProcessSync(LPMAIDEntryPointProc ep, LPRefObj pRefSrc, ULONG ulCapID );
BOOL	IssueAcquire(LPMAIDEntryPointProc ep, LPRefObj pRefDat );
BOOL	IssueThumbnail( LPMAIDEntryPointProc ep,LPRefObj pRefSrc );

LPNkMAIDCapInfo	GetCapInfo( LPRefObj pRef, ULONG ulID );
BOOL	CheckCapabilityOperation( LPRefObj pRef, ULONG ulID, ULONG ulOperations );
LPRefObj	GetRefChildPtr_Index( LPRefObj pRefParent, ULONG ulIndex );
LPRefObj	GetRefChildPtr_ID( LPRefObj pRefParent, SLONG lIDChild );


/////////////////////////////////////////////////////////////////////////////
// Static variables

extern LPMAIDEntryPointProc	g_pMAIDEntryPoint;
extern UCHAR	g_bFileRemoved;
extern BOOL		g_bFirstCall;	// used in ProgressProc, and DoDeleteDramImage
#ifdef _WINDOWS
	extern HINSTANCE	g_hInstModule;
#else
	extern CFragConnectionID	g_ConnID;
	extern short	g_nModRefNum;
#endif

#ifdef __cplusplus
}
#endif
