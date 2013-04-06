//================================================================================================
// Copyright Nikon Corporation - All rights reserved
//
// View this file in a non-proportional font, tabs = 3
//================================================================================================

#include <stdlib.h>
#include <stdio.h>
#ifdef _WINDOWS
	#include <windows.h>
	#include <mmsystem.h>
#endif
#include "Maid3.h"
#include "maid3d1.h"
#include "CtrlSample.h"

#ifdef _WINDOWS
ULONG g_ulProgressValue;// used in only ProgressProc
#else
ULONG g_ulProgressValue;// used in only ProgressProc
#endif
BOOL	g_bFirstCall = true;// used in ProgressProc, and DoDeleteDramImage

//------------------------------------------------------------------------------------------------------------------------------------
extern void ModEventCBWrapper(void * refProc, unsigned long ulEvent, unsigned long data);

void CALLPASCAL CALLBACK ModEventProc( NKREF refProc, ULONG ulEvent, NKPARAM data )
{
    ModEventCBWrapper(refProc, ulEvent, data);
}
/*
void CALLPASCAL CALLBACK ModEventProc( NKREF refProc, ULONG ulEvent, NKPARAM data )
{
	BOOL bRet;
	LPRefObj pRefParent = (LPRefObj)refProc, pRefChild = NULL;

	switch(ulEvent){
		case kNkMAIDEvent_AddChild:
            puts("Mod Event_AddChild\n");
			bRet = AddChild( pRefParent, (SLONG)data );
			if ( bRet == false ) return;
			pRefChild = GetRefChildPtr_ID( pRefParent, (SLONG)data );
			// Enumerate children(Item and Data Objects) and open them.
			bRet = EnumChildrten( pRefChild->pObject );
			if ( bRet == false ) return;
			break;
		case kNkMAIDEvent_RemoveChild:
            puts("Mod Event_RemoveChild\n");
			bRet = RemoveChild( pRefParent, (SLONG)data );
			if ( bRet == false ) return;
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
			bRet = EnumCapabilities( pRefParent->pObject, &(pRefParent->ulCapCount), &(pRefParent->pCapArray), NULL, NULL );
			if ( bRet == false ) return;
			// ToDo: Execute a process when the property of a capability was changed.
			break;
		case kNkMAIDEvent_CapChangeValueOnly:
			// ToDo: Execute a process when the value of a capability was changed.
			printf( "The value of Capability(CapID=0x%X) was changed.\n", (int) data );
			break;
		case kNkMAIDEvent_OrphanedChildren:
			// ToDo: Close children(Source Objects).
			break;
		default:
			puts( "Detected unknown Event to the Module object.\n" );
		}
}
*/
//------------------------------------------------------------------------------------------------------------------------------------

extern void SrcEventCBWrapper(void * refProc, unsigned long ulEvent, unsigned long data);

void CALLPASCAL CALLBACK SrcEventProc( NKREF refProc, ULONG ulEvent, NKPARAM data )
{
    SrcEventCBWrapper(refProc, ulEvent, data);
}
/*
void CALLPASCAL CALLBACK SrcEventProc( NKREF refProc, ULONG ulEvent, NKPARAM data )
{
	BOOL bRet;
	LPRefObj pRefParent = (LPRefObj)refProc, pRefChild = NULL;

	switch(ulEvent){
		case kNkMAIDEvent_AddChild:
			bRet = AddChild( pRefParent, (SLONG)data );
			if ( bRet == false ) return;
			pRefChild = GetRefChildPtr_ID( pRefParent, (SLONG)data );
			// Enumerate children(Data Objects) and open them.
			bRet = EnumChildrten( pRefChild->pObject );
			if ( bRet == false ) return;
			break;
		case kNkMAIDEvent_RemoveChild:
			bRet = RemoveChild( pRefParent, (SLONG)data );
			if ( bRet == false ) return;
			break;
		case kNkMAIDEvent_WarmingUp:
			// The Type0001/2/3/D40/D80/D200 Module does not use this event.
			puts( "Event_WarmingUp to Source object is not supported.\n" );
			break;
		case kNkMAIDEvent_WarmedUp:
			// The Type0001/2/3/D40/D80/D200 Module does not use this event.
			puts( "Event_WarmedUp to Source object is not supported.\n" );
			break;
		case kNkMAIDEvent_CapChange:
			// re-enumerate the capabilities
			if ( pRefParent->pCapArray != NULL ) {
				free( pRefParent->pCapArray );
				pRefParent->ulCapCount = 0;
				pRefParent->pCapArray = NULL;
			}
			bRet = EnumCapabilities( pRefParent->pObject, &(pRefParent->ulCapCount), &(pRefParent->pCapArray), NULL, NULL );
			if ( bRet == false ) return;
			// ToDo: Execute a process when the property of a capability was changed.
			break;
		case kNkMAIDEvent_CapChangeValueOnly:
			// ToDo: Execute a process when the value of a capability was changed.
			printf( "The value of Capability(CapID=0x%X) was changed.\n", (int) data );
			break;
		case kNkMAIDEvent_OrphanedChildren:
			// ToDo: Close children(Item Objects).
			break;
		case kNkMAIDEvent_AddPreviewImage:
			{
				// A Preview data is a specification deleted from the inside of the camera after reading a main image is finished.
				// Current module is a specification that immediately begins the acquisition of a main image regardless of
				// the request of the client when the creation completion notification of a main image is received from the camera.
				// After module read the image, the image data is deleted from the inside of the camera.
				// Therefore, the acquisition of a Preview data  is not a function secured 100%.
				// However, if client could read a Preview data even once, the module generates the Preview cache internally,
				// so the client can get data after receiving kNkMAIDEvent_AddChild even, until Item object is closed.

				ULONG	ulSel;

			printf( "\n\nReceived AddPreviewImage Event.\n PreviewID[0x%08X] is ready.\n\n", (int) data );

				do {
					ULONG ulItmID = (ULONG)data;
					char	buf[256], HeaderFileName[256], ImageFileName[256];
					FILE*	hFileHeader = NULL;		// Preview Image file name
					FILE*	hFileImage = NULL;		// Preview header file name
					ULONG	ulHeaderSize = 32;		//The header size of PreviewImage
					NkMAIDArray	stArray;
						int i = 0;
					unsigned char* pucData = NULL;	// Preview data pointer

					printf( "\nSelect (1-3, 0)\n" );
					printf( " 1. Get Preview Low.\n" );
					printf( " 2. Get Preview Normal.\n" );
					printf( " 3. Delete image.\n" );
					printf( " 0. Exit\n>" );
					scanf( "%s", buf );
					ulSel = atol( buf );

					switch( ulSel )
					{
					case 1:// Get PreviewImage Low
					case 2:// Get PreviewImage Normal
						// Set Preview ID
						bRet = Command_CapSet( pRefParent->pObject, kNkMAIDCapability_CurrentPreviewID, kNkMAIDDataType_Unsigned, (NKPARAM)ulItmID, NULL, NULL );
						if (bRet)
						{
							if (ulSel == 1 )
							{	// Preview Low
								bRet = GetArrayCapability( pRefParent, kNkMAIDCapability_GetPreviewImageLow, &stArray );
							}
							else
							{	// Preview Normal
								bRet = GetArrayCapability( pRefParent, kNkMAIDCapability_GetPreviewImageNormal, &stArray );
							}
							if ( bRet == false ) break;

							// create file name
							while( true )
							{
								sprintf( HeaderFileName, "Preview%03d_H.%s", ++i, "dat" );
								sprintf( ImageFileName, "Preview%03d.%s", i, "jpg" );
								if ( (hFileHeader = fopen(HeaderFileName, "r") ) != NULL ||
										(hFileImage  = fopen(ImageFileName, "r") )  != NULL    )
								{
									// this file name is already used.
									if (hFileHeader)
									{
										fclose( hFileHeader );
										hFileHeader = NULL;
									}
									if (hFileImage)
									{
										fclose( hFileImage );
										hFileImage = NULL;
									}
								}
								else
								{
									break;
								}
							}

							// open file
							hFileHeader = fopen( HeaderFileName, "wb" );
							if ( hFileHeader == NULL )
							{
								printf("file open error.\n");
								break;
							}
							hFileImage = fopen( ImageFileName, "wb" );
							if ( hFileImage == NULL )
							{
								printf("file open error.\n");
								break;
							}

							// Get data pointer
							pucData = (unsigned char*)stArray.pData;

							// write file
							if ( hFileHeader && hFileImage )
							{
								fwrite( pucData, 1, ulHeaderSize, hFileHeader );
								fwrite( pucData+ulHeaderSize, 1, (stArray.ulElements-ulHeaderSize), hFileImage );
								printf("\n%s was saved.\n", HeaderFileName);
								printf("%s was saved.\n", ImageFileName);
							}
						}
						break;

					case 3: // Delete Dram Image (Delete timing No.1)
						// Set Preview ID
						bRet = Command_CapSet( pRefParent->pObject, kNkMAIDCapability_CurrentPreviewID, kNkMAIDDataType_Unsigned, (NKPARAM)ulItmID, NULL, NULL );
						if (bRet)
						{
							// Delete DRAM
							bRet = Command_CapStart( pRefParent->pObject, kNkMAIDCapability_DeleteDramImage, NULL, NULL, NULL );
							if (bRet)
							{
								printf("\nPreview ID [0x%08X] was deleted.\n", (int) ulItmID );
							}
						}
						break;

					default:
						ulSel = 0;
					}
					if ( bRet == false )
					{
						printf( "An Error occured. \n" );
					}
				} while( ulSel > 0 );
			}
			break;

		case kNkMAIDEvent_CaptureComplete:
			// ToDo: Show the image transfer finished.
			break;

		default:
			puts( "Detected unknown Event to the Source object.\n" );
		}
}
*/
//------------------------------------------------------------------------------------------------------------------------------------
extern void ItmEventProcCBWrapper(void * refProc, unsigned long ulEvent, unsigned long data);

void CALLPASCAL CALLBACK ItmEventProc( NKREF refProc, ULONG ulEvent, NKPARAM data )
{
	return ItmEventProcCBWrapper(refProc, ulEvent, data);
}
	//BOOL bRet;
	//LPRefObj pRefParent = (LPRefObj)refProc;

	//switch(ulEvent){
	//	case kNkMAIDEvent_AddChild:
 //           puts("Itm Event_AddChild\n");
	//		bRet = AddChild( pRefParent, (SLONG)data );
	//		if ( bRet == false ) return;
	//		break;
	//	case kNkMAIDEvent_RemoveChild:
 //           puts("Itm Event_RemoveChild\n");
	//		bRet = RemoveChild( pRefParent, (SLONG)data );
	//		if ( bRet == false ) return;
	//		break;
	//	case kNkMAIDEvent_WarmingUp:
	//		// The Type0001/2/3/D40/D80/D200 Module does not use this event.
	//		puts( "Event_WarmingUp to Item object is not supported.\n" );
	//		break;
	//	case kNkMAIDEvent_WarmedUp:
	//		// The Type0001/2/3/D40/D80/D200 Module does not use this event.
	//		puts( "Event_WarmedUp to Item object is not supported.\n" );
	//		break;
	//	case kNkMAIDEvent_CapChange:
 //           puts("Event_CapChange\n");
	//		// re-enumerate the capabilities
	//		if ( pRefParent->pCapArray != NULL ) {
	//			free( pRefParent->pCapArray );
	//			pRefParent->ulCapCount = 0;
	//			pRefParent->pCapArray = NULL;
	//		}
	//		bRet = EnumCapabilities( pRefParent->pObject, &(pRefParent->ulCapCount), &(pRefParent->pCapArray), NULL, NULL );
	//		if ( bRet == false ) return;
	//		// ToDo: Execute a process when the property of a capability was changed.
	//		break;
	//	case kNkMAIDEvent_CapChangeValueOnly:
	//		// ToDo: Execute a process when the value of a capability was changed.
	//		printf( "The value of Capability(CapID=0x%X) was changed.\n", (int) data );
	//		break;
	//	case kNkMAIDEvent_OrphanedChildren:
	//		// ToDo: Close children(Data Objects).
	//		break;
	//	default:
	//		puts( "Detected unknown Event to the Item object.\n" );
	//	}
	//}
//------------------------------------------------------------------------------------------------------------------------------------
extern void DatEventCBWrapper(void * refProc, unsigned long ulEvent, unsigned long data);

void CALLPASCAL CALLBACK DatEventProc( NKREF refProc, ULONG ulEvent, NKPARAM data ){
	return DatEventCBWrapper(refProc, ulEvent, data);
}
//{
//	BOOL bRet;
//	LPRefObj	pRefParent = (LPRefObj)refProc;
//
//	switch(ulEvent){
//		case kNkMAIDEvent_AddChild:
//			// In data object, the Type0001/2/3/D40/D80/D200 Module does not use this event.
//			puts( "Event_AddChild to Data object is not supported.\n" );
//			break;
//		case kNkMAIDEvent_RemoveChild:
//			// The Type0001/2/3/D40/D80/D200 Module does not use this event.
//			puts( "Event_RemoveChild to Data object is not supported.\n" );
//			break;
//		case kNkMAIDEvent_WarmingUp:
//			// The Type0001/2/3/D40/D80/D200 Module does not use this event.
//			puts( "Event_WarmingUp to Data object is not supported.\n" );
//			break;
//		case kNkMAIDEvent_WarmedUp:
//			// The Type0001/2/3/D40/D80/D200 Module does not use this event.
//			puts( "Event_WarmedUp to Data object is not supported.\n" );
//			break;
//		case kNkMAIDEvent_CapChange:// module notify that a capability is changed.
//            puts("Event_CapChange\n");
//			// re-enumerate the capabilities
//			if ( pRefParent->pCapArray != NULL ) {
//				free( pRefParent->pCapArray );
//				pRefParent->ulCapCount = 0;
//				pRefParent->pCapArray = NULL;
//			}
//			bRet = EnumCapabilities( pRefParent->pObject, &(pRefParent->ulCapCount), &(pRefParent->pCapArray), NULL, NULL );
//			if ( bRet == false ) return;
//			// ToDo: Execute a process when the property of a capability was changed.
//		case kNkMAIDEvent_CapChangeValueOnly:
//			// ToDo: Execute a process when the value of a capability was changed.
//			printf( "The value of Capability(CapID=0x%X) was changed.\n", (int) data );
//			break;
//		case kNkMAIDEvent_OrphanedChildren:
//			// The Type0001/2/3/D40/D80/D200 Module does not use this event.
//			puts( "Event_OrphanedChildren to Data object is not supported.\n" );
//			break;
//		default:
//			puts( "Detected unknown Event to the Data object.\n" );
//	}
//}
//------------------------------------------------------------------------------------------------------------------------------------
extern long DataCBWrapper(void * ref, void * pInfo, void * pData);
NKERROR CALLPASCAL CALLBACK DataProc( NKREF ref, LPVOID pInfo, LPVOID pData )
{
    return DataCBWrapper(ref, pInfo, pData);
}

// copy the delivered data
/*
NKERROR CALLPASCAL CALLBACK DataProc( NKREF ref, LPVOID pInfo, LPVOID pData )
{
	LPNkMAIDDataInfo pDataInfo = (LPNkMAIDDataInfo)pInfo;
	LPNkMAIDImageInfo pImageInfo = (LPNkMAIDImageInfo)pInfo;
	LPNkMAIDFileInfo pFileInfo = (LPNkMAIDFileInfo)pInfo;
	ULONG ulTotalSize, ulOffset;
	LPVOID pCurrentBuffer;
	ULONG ulByte;

	if ( pDataInfo->ulType & kNkMAIDDataObjType_File ) {
		if( ((LPRefDataProc)ref)->ulOffset == 0 && ((LPRefDataProc)ref)->pBuffer == NULL )
			((LPRefDataProc)ref)->pBuffer = malloc( pFileInfo->ulTotalLength );
		if ( ((LPRefDataProc)ref)->pBuffer == NULL ) {
			puts( "There is not enough memory." );
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
			// We have finished the delivery. We will save this file.
			FILE *stream;
			char filename[256], Prefix[16], Ext[16];
			UWORD i = 0;
			if ( pDataInfo->ulType & kNkMAIDDataObjType_Image )
				strcpy(Prefix,"Image");
			else if ( pDataInfo->ulType & kNkMAIDDataObjType_Thumbnail )
				strcpy(Prefix,"Thumb");
			else
				strcpy(Prefix,"Unknown");
			switch( pFileInfo->ulFileDataType ) {
				case kNkMAIDFileDataType_JPEG:
					strcpy(Ext,".jpg");
					break;
				case kNkMAIDFileDataType_TIFF:
					strcpy(Ext,".tif");
					break;
				case kNkMAIDFileDataType_NIF:
					strcpy(Ext,".nef");
					break;
				case kNkMAIDFileDataType_NDF:
					strcpy(Ext,".ndf");
					break;
				default:
					strcpy(Ext,".dat");
			}
			while( true ) {
				sprintf( filename, "%s%03d%s", Prefix, ++i, Ext );
				if ( (stream = fopen(filename, "r") ) != NULL )
					fclose(stream);
				else
					break;
			}
			if ( (stream = fopen(filename, "wb") ) == NULL)
				return kNkMAIDResult_UnexpectedError;
			fwrite(((LPRefDataProc)ref)->pBuffer, 1, pFileInfo->ulTotalLength, stream);
			fclose(stream);
			free(((LPRefDataProc)ref)->pBuffer);
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
			puts( "There is not enough memory." );
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
*/
//------------------------------------------------------------------------------------------------------------------------------------

void CALLPASCAL CALLBACK CompletionProc(
  			LPNkMAIDObject	pObject,			// module, source, item, or data object
			ULONG				ulCommand,		// Command, one of eNkMAIDCommand
			ULONG				ulParam,			// parameter for the command
			ULONG				ulDataType,		// Data type, one of eNkMAIDDataType
			NKPARAM			data,				// Pointer or long integer
			NKREF				refComplete,	// Reference set by client
			NKERROR			nResult )		// One of eNkMAIDResult)
{
	((LPRefCompletionProc)refComplete)->nResult = nResult;
	(*((LPRefCompletionProc)refComplete)->pulCount) ++;

	// if the Command is CapStart acquire, we terminate RefDeliver.
	if(ulCommand == kNkMAIDCommand_CapStart && ulParam == kNkMAIDCapability_Acquire) {
		LPRefDataProc pRefDeliver = (LPRefDataProc)((LPRefCompletionProc)refComplete)->pRef;
		if ( pRefDeliver != NULL ) {
			if ( pRefDeliver->pBuffer != NULL )
				free( pRefDeliver->pBuffer );
			free( pRefDeliver );
		}
	}
	// terminate refComplete.
	if ( refComplete != NULL )
		free( refComplete );

}
//------------------------------------------------------------------------------------------------------------------------------------

void CALLPASCAL CALLBACK ProgressProc(
		ULONG				ulCommand,			// Command, one of eNkMAIDCommand
		ULONG				ulParam,				// parameter for the command
		NKREF				refProc,				// Reference set by client
		ULONG				ulDone,				// Numerator
		ULONG				ulTotal )			// Denominator
{
	ULONG	ulNewProgressValue, ulCount;
	puts("ProgressProc\n");
	if ( ulTotal == 0 ) {
		// when we don't know how long this process is, we show such as barber's pole.
		if ( ulDone == 1 ) {
		#ifdef _WINDOWS
			ulNewProgressValue = timeGetTime();
			if( (ulNewProgressValue < g_ulProgressValue) || (ulNewProgressValue > g_ulProgressValue + 500) ) {
				printf( "c" );
				g_ulProgressValue = ulNewProgressValue;
			}
		#else
			ulNewProgressValue = TickCount();
			if( (ulNewProgressValue < g_ulProgressValue) || (ulNewProgressValue > g_ulProgressValue + 30) ) {
				printf( "c" );
				g_ulProgressValue = ulNewProgressValue;
			}
		#endif
		} else if ( ulDone == 0 ) {
				printf( "o" );
		}
	} else {
		// when we know how long this process is, we show progress bar.
		if ( ulDone == 0 ) {
			if ( g_bFirstCall == true ) {
				g_ulProgressValue = 0;
				g_bFirstCall = false;
				printf("\n0       20        40        60        80        100");
				printf("\n---------+---------+---------+---------+---------+\n");
			}
		} else {
			// show progress bar
			ulNewProgressValue = (50 * ulDone + ulTotal - 1) / ulTotal;
			ulCount = ulNewProgressValue - g_ulProgressValue;
			while ( ulCount-- )
				printf( "]" );
			g_ulProgressValue = ulNewProgressValue;
			if ( ulDone == ulTotal ) {
				printf( "\n" );
				g_bFirstCall = true;
			}
		}
	}
}
//------------------------------------------------------------------------------------------------------------------------------------

ULONG CALLPASCAL CALLBACK UIRequestProc( NKREF ref, LPNkMAIDUIRequestInfo pUIRequest )
{
	short	 nRet = kNkMAIDUIRequestResult_None;
	char	sAns[256];

	// display message
	if (pUIRequest->lpPrompt)
		printf( "\n%s\n", pUIRequest->lpPrompt );
	if (pUIRequest->lpDetail)
		printf( "\n%s\n", pUIRequest->lpDetail );

	// get an answer
	switch( pUIRequest->ulType ){
		case kNkMAIDUIRequestType_Ok:
			do {
				printf("\nPress 'O' key. ('O': OK)\n>");
				scanf( "%s", sAns );
			} while ( *sAns != 'o' && *sAns != 'O' );
			nRet = kNkMAIDUIRequestResult_Ok;
			break;
		case kNkMAIDUIRequestType_OkCancel:
			do {
				printf("\nPress 'O' or 'C' key. ('O': OK   'C': Cancel)\n>");
				scanf( "%s", sAns );
			} while ( *sAns != 'o' && *sAns != 'O' && *sAns != 'c' && *sAns != 'C' );
			if ( *sAns == 'o' || *sAns == 'O' )
				nRet = kNkMAIDUIRequestResult_Ok;
			else if ( *sAns == 'c' || *sAns == 'C' )
				nRet = kNkMAIDUIRequestResult_Cancel;
			break;
		case kNkMAIDUIRequestType_YesNo:
			do {
				printf("\nPress 'Y' or 'N' key. ('Y': Yes   'N': No)\n>");
				scanf( "%s", sAns );
			} while ( *sAns != 'y' && *sAns != 'Y' && *sAns != 'n' && *sAns != 'N' );
			if ( *sAns == 'y' || *sAns == 'Y' )
				nRet = kNkMAIDUIRequestResult_Yes;
			else if ( *sAns == 'n' || *sAns == 'N' )
				nRet = kNkMAIDUIRequestResult_No;
			break;
		case kNkMAIDUIRequestType_YesNoCancel:
			do {
				printf("\nPress 'Y' or 'N' or 'C' key. ('Y': Yes   'N': No   'C': Cancel)\n>");
				scanf( "%s", sAns );
			} while ( *sAns != 'y' && *sAns != 'Y' && *sAns != 'n' && *sAns != 'N' && *sAns != 'c' && *sAns != 'C' );
			if ( *sAns == 'y' || *sAns == 'Y' )
				nRet = kNkMAIDUIRequestResult_Yes;
			else if ( *sAns == 'n' || *sAns == 'N' )
				nRet = kNkMAIDUIRequestResult_No;
			else if ( *sAns == 'c' || *sAns == 'C' )
				nRet = kNkMAIDUIRequestResult_Cancel;
			break;
		default:
			nRet = kNkMAIDUIRequestResult_None;
	}

	return nRet;
}
//------------------------------------------------------------------------------------------------------------------------------------
