/**************************************************
 * odbcinstext.h
 *
 **************************************************
 * This code was created by Peter Harvey @ CodeByDesign.
 * Released under LGPL 28.JAN.99
 *
 * Contributions from...
 * -----------------------------------------------
 * Peter Harvey		- pharvey@codebydesign.com
 **************************************************/
#ifndef _ODBCINST_H
#define _ODBCINST_H

#include <unistd.h>
#ifdef HAVE_PWD_H
#include <pwd.h>
#endif
#include <sys/types.h>
#include <stdio.h>

#define ODBCVER 0x0351

#include <ini.h>
#include <log.h>
#include <odbcinst.h>

/********************************************************
 * CONSTANTS WHICH DO NOT EXIST ELSEWHERE
 ********************************************************/
#ifndef TRUE
#define FALSE 0;
#define TRUE 1;
#endif

/********************************************************
 * PUBLIC API
 ********************************************************/

#ifdef __cplusplus
extern "C"
{
#endif

BOOL SQLConfigDataSource(		HWND	hWnd,
								WORD	nRequest,
								LPCSTR	pszDriver,
								LPCSTR	pszAttributes );

BOOL SQLGetConfigMode(          UWORD	*pnConfigMode );

BOOL SQLGetInstalledDrivers(	LPSTR	pszBuf,
								WORD	nBufMax,
								WORD	*pnBufOut );

BOOL SQLInstallDriverEx(		LPCSTR	pszDriver,
								LPCSTR	pszPathIn,
								LPSTR	pszPathOut,
								WORD	nPathOutMax,
								WORD	*nPathOut,
								WORD	nRequest,
								LPDWORD	pnUsageCount );

BOOL SQLInstallDriverManager(	LPSTR	pszPath,
								WORD	nPathMax,
								WORD	*pnPathOut );

RETCODE SQLInstallerError(		WORD	nError,
								DWORD	*pnErrorCode,
								LPSTR	pszErrorMsg,
								WORD	nErrorMsgMax,
								WORD	*nErrorMsg );

BOOL SQLManageDataSources(		HWND	hWnd );

BOOL SQLReadFileDSN(			LPCSTR	pszFileName,
								LPCSTR	pszAppName,
								LPCSTR	pszKeyName,
								LPSTR	pszString,
								WORD	nString,
								WORD	*pnString );

BOOL SQLRemoveDriver(			LPCSTR	pszDriver,
								BOOL	nRemoveDSN,
								LPDWORD	pnUsageCount );

BOOL SQLRemoveDriverManager(	LPDWORD	pnUsageCount );

BOOL SQLRemoveDSNFromIni(		LPCSTR	pszDSN );

BOOL SQLRemoveTranslator(		LPCSTR	pszTranslator,
								LPDWORD	pnUsageCount );

BOOL SQLSetConfigMode(			UWORD	nConfigMode );

BOOL SQLValidDSN(				LPCSTR	pszDSN );

BOOL SQLWriteDSNToIni(			LPCSTR	pszDSN,
								LPCSTR	pszDriver );

BOOL SQLWriteFileDSN(			LPCSTR	pszFileName,
								LPCSTR	pszAppName,
								LPCSTR	pszKeyName,
								LPCSTR	pszString );

BOOL SQLWritePrivateProfileString(
								LPCSTR	pszSection,
								LPCSTR	pszEntry,
								LPCSTR	pszString,
								LPCSTR	pszFileName );



#ifdef __cplusplus
}
#endif


/********************************************************
 * PRIVATE API
 ********************************************************/
#if defined(__cplusplus)
         extern  "C" {
#endif

BOOL _odbcinst_UserINI(
	char *pszFileName,
	BOOL bVerify );

BOOL _odbcinst_SystemINI(		
	char *pszFileName,
	BOOL bVerify );

char * odbcinst_system_file_path( void );

BOOL _odbcinst_ConfigModeINI( 	
	char *pszFileName );

int _odbcinst_GetSections(	
	HINI	hIni,
	LPSTR	pRetBuffer,
	int		nRetBuffer,
	int		*pnBufPos );

int _odbcinst_GetEntries(	
	HINI	hIni,
	LPCSTR	pszSection,
	LPSTR	pRetBuffer,
	int		nRetBuffer,
	int		*pnBufPos );

int _SQLGetInstalledDrivers(	
	LPCSTR	pszSection,
	LPCSTR	pszEntry,
	LPCSTR	pszDefault,
	LPCSTR	pRetBuffer,
	int		nRetBuffer );

BOOL _SQLWriteInstalledDrivers(
	LPCSTR	pszSection,
	LPCSTR	pszEntry,
	LPCSTR	pszString );

int inst_logPushMsg( 
        char *pszModule, 
        char *pszFunctionName, 
        int nLine, 
        int nSeverity, 
        int nCode, 
        char *pszMessage );

int inst_logPopMsg( 
        char *pszMsgHdr, 
        int *pnCode, 
        char *pszMsg );

/*
 * we should look at caching this info, the calls can become expensive
 */

#ifndef DISABLE_INI_CACHING

struct ini_cache
{
    char                *fname;
    char                *section;
    char                *entry;
    char                *value;
    char                *default_value;
    int                 buffer_size;
    int                 ret_value;
    int                 config_mode;
    long                timestamp;
    struct ini_cache    *next;
};

#endif

#ifdef __cplusplus
}
#endif

/*********************************
 * ODBCINST - PROPERTIES
 *********************************
 *
 * PURPOSE:
 *
 * To provide the caller a mechanism to interact with Data Source properties
 * containing Driver specific options while avoiding embedding GUI code in
 * the ODBC infrastructure.
 *
 * DETAILS:
 *
 *	1.	Application calls libodbcinst.ODBCINSTConstructProperties()
 *		- odbcinst will load the driver and call libMyDrvS.ODBCINSTGetProperties() to build a list of all possible properties
 *	2.	Application calls libodbcinst.ODBCINSTSetProperty()
 *		- use, as required, to init values (ie if configuring existing DataSource)
 *		- use libodbcinst.SetConfigMode() & libodbcinst.SQLGetPrivateProfileString() to read existing Data Source info (do not forget to set the mode back)
 *		- do not forget to set mode back to ODBC_BOTH_DSN using SetConfigMode() when done reading
 *		- no call to Driver Setup
 *	3.	Application calls libodbcinst.ODBCINSTValidateProperty()
 *		- use as required (ie on leave widget event)
 *		- an assesment of the entire property list is also done
 *		- this is passed onto the driver setup DLL
 *	4.	Application should refresh widgets in case aPromptData or szValue has changed
 *		- refresh should occur for each property where bRefresh = 1
 *	5.	Application calls libodbcinst.ODBCINSTValidateProperties()
 *		- call this just before saving new Data Source or updating existing Data Source
 *		- should always call this before saving
 *		- use libodbcinst.SetConfigMode() & libodbcinst.SQLWritePrivateProfileString() to save Data Source info
 *		- do not forget to set mode back to ODBC_BOTH_DSN using SetConfigMode() when done saving
 *		- this is passed onto the driver setup DLL
 *	6.	Application calls ODBCINSTDestructProperties() to free up memory
 *		- unload Driver Setup DLL
 *		- frees memory (Driver Setup allocates most of the memory but we free ALL of it in odbcinst)
 *
 * NOTES
 *
 *	1.	odbcinst implements 5 functions to support this GUI config stuff
 *	2.	Driver Setup DLL implements just 3 functions for its share of the work
 *
 *********************************/

#define ODBCINST_SUCCESS				0
#define ODBCINST_WARNING				1
#define ODBCINST_ERROR					2

#define	ODBCINST_PROMPTTYPE_LABEL		0 /* readonly */
#define	ODBCINST_PROMPTTYPE_TEXTEDIT	1
#define	ODBCINST_PROMPTTYPE_LISTBOX		2
#define	ODBCINST_PROMPTTYPE_COMBOBOX	3
#define	ODBCINST_PROMPTTYPE_FILENAME	4
#define	ODBCINST_PROMPTTYPE_HIDDEN	    5 

typedef struct	tODBCINSTPROPERTY
{
	struct tODBCINSTPROPERTY *pNext;				/* pointer to next property, NULL if last property										*/

	char	szName[INI_MAX_PROPERTY_NAME+1];		/* property name																		*/
	char	szValue[INI_MAX_PROPERTY_VALUE+1];		/* property value																		*/
	int		nPromptType;							/* PROMPTTYPE_TEXTEDIT, PROMPTTYPE_LISTBOX, PROMPTTYPE_COMBOBOX, PROMPTTYPE_FILENAME	*/
	char	**aPromptData;							/* array of pointers terminated with a NULL value in array. 							*/
	char	*pszHelp;								/* help on this property (driver setups should keep it short)							*/
	void	*pWidget;								/* CALLER CAN STORE A POINTER TO ? HERE													*/
	int		bRefresh;								/* app should refresh widget ie Driver Setup has changed aPromptData or szValue   		*/
	void 	*hDLL;									/* for odbcinst internal use... only first property has valid one 						*/
} ODBCINSTPROPERTY, *HODBCINSTPROPERTY;


#if defined(__cplusplus)
         extern  "C" {
#endif

/* ONLY IMPLEMENTED IN ODBCINST (not in Driver Setup) */
int ODBCINSTConstructProperties( char *szDriver, HODBCINSTPROPERTY *hFirstProperty );
int ODBCINSTSetProperty( HODBCINSTPROPERTY hFirstProperty, char *pszProperty, char *pszValue );
int ODBCINSTDestructProperties( HODBCINSTPROPERTY *hFirstProperty );
int ODBCINSTAddProperty( HODBCINSTPROPERTY hFirstProperty, char *pszProperty, char *pszValue );

/* IMPLEMENTED IN ODBCINST AND DRIVER SETUP */
int ODBCINSTValidateProperty( HODBCINSTPROPERTY hFirstProperty, char *pszProperty, char *pszMessage );
int ODBCINSTValidateProperties( HODBCINSTPROPERTY hFirstProperty, HODBCINSTPROPERTY hBadProperty, char *pszMessage );

/* ONLY IMPLEMENTED IN DRIVER SETUP (not in ODBCINST) */
int ODBCINSTGetProperties( HODBCINSTPROPERTY hFirstProperty );

#if defined(__cplusplus)
         }
#endif

#endif

