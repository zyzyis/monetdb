/*--------------------------------------------------------------------*/
/*
/* ODBCSetupCons - ODBC Setup Console style program that 
/* version checks, copies and installs ODBC32.DLL and a custom
/* ODBC 32bit driver
/*
/* (c) Copyright 1991- 1999 Microsoft Corporation. All rights reserved.
/*--------------------------------------------------------------------*/

#include "sql_config.h"
#include <windows.h>
#include <sql.h>
#include <sqlext.h>
#include <odbcinst.h>
#include <winver.h>
#include <string.h>
#include <stdio.h>
#include <shlwapi.h>
#include <malloc.h>

static char *DriverName = "MonetDB ODBC Driver";
static char *DataSourceName = "MonetDB";
static char *DriverDLL = "libMonetODBC.dll";
static char *DriverDLLs = "libMonetODBCs.dll";

/* General error handler for installer functions */

static BOOL
ProcessErrorMessages(const char *func)
{
	WORD errnr = 1;
	DWORD errcode;
	char errmsg[300];
	WORD errmsglen;
	int rc;
	BOOL func_rc = FALSE;

	do {
		errmsg[0] = '\0';
		rc = SQLInstallerError(errnr, &errcode, errmsg,
				       sizeof(errmsg), &errmsglen);
		if (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO) {
			fprintf(stderr, "%s in function %s", errmsg, func);
			func_rc = TRUE;
		}
		errnr++;
	} while (rc != SQL_NO_DATA);

	return func_rc;
}

static BOOL
CheckIfFileExists(const char *filepath, const char *filename)
{
	char buf[300];
	LPTSTR b;

	return SearchPath(filepath, filename, NULL, sizeof(buf), buf, &b) > 0;
}

static BOOL
GetFileVersion(const char *filepath, char *version, size_t maxversionlen)
{
	DWORD handle = 0;
	DWORD versioninfosize;
	DWORD error;
	PVOID fileinfo;
	PBYTE versioninfo;
	PDWORD translation = NULL;
	UINT length = 0;
	char string[512] = "";
	LPSTR versionstr;

	versioninfosize = GetFileVersionInfoSize(filepath, &handle);
	if (!versioninfosize) {
		error = GetLastError();
		return FALSE;
	}

	fileinfo = (PVOID) malloc(versioninfosize);
	versioninfo = (PBYTE) malloc(versioninfosize);

	if (!GetFileVersionInfo(filepath, handle, versioninfosize, fileinfo)) {
		error = GetLastError();
		free(fileinfo);
		free(versioninfo);
		return FALSE;
	}

	if (!VerQueryValue(fileinfo, TEXT("\\VarFileInfo\\Translation"),
			   (LPVOID *) &translation, &length)) {
		error = GetLastError();
		free(fileinfo);
		free(versioninfo);
		return FALSE;
	}

	wnsprintf(string, sizeof(string),
		  "\\StringFileInfo\\%04x%04x\\FileVersion",
		  LOWORD(*translation), HIWORD(*translation));

	if (!VerQueryValue(fileinfo, string, (PVOID *) &versionstr, &length)) {
		error = GetLastError();
		free(fileinfo);
		free(versioninfo);
		return FALSE;
	}

	if (lstrlen(versionstr) >= (int) maxversionlen)
		lstrcpyn(version, versionstr, maxversionlen - 1);
	else
		lstrcpy(version, versionstr);

	free(fileinfo);
	free(versioninfo);

	return TRUE;
}

static BOOL
VersionCheckCopyFile(const char *srcpath, const char *dstpath,
		     const char *filename)
{
	char srcfile[512];
	char dstfile[512];
	char srcfileVersion[512];
	char dstfileVersion[512];

	snprintf(srcfile, sizeof(srcfile), "%s\\%s", srcpath, filename);
	snprintf(dstfile, sizeof(dstfile), "%s\\%s", dstpath, filename);

	if (CheckIfFileExists(dstpath, filename)) {
		if (!GetFileVersion(srcfile, srcfileVersion,
				    sizeof(srcfileVersion)) ||
		    !GetFileVersion(dstfile, dstfileVersion,
				    sizeof(dstfileVersion)))
			return FALSE;

		if (strcmp(dstfileVersion, srcfileVersion) >= 0) {
			/* file is up-to-date, so don't copy */
			return TRUE;
		}
		/* file exists but is not up-to-date, so copy */
	}
	/* else file does not exist, so copy */

	if (!CopyFile(srcfile, dstfile, FALSE)) {
		fprintf(stderr, "Unable to copy %s to %s\n", srcfile, dstfile);
		return FALSE;
	}

	return TRUE;
}

static BOOL
CopyODBCCore(const char *redistpath, const char *path)
{
	return VersionCheckCopyFile(redistpath, path, "ds16gt.dll") &&
		VersionCheckCopyFile(redistpath, path, "ds32gt.dll") &&
		VersionCheckCopyFile(redistpath, path, "odbc16gt.dll") &&
		VersionCheckCopyFile(redistpath, path, "odbc32.dll") &&
		VersionCheckCopyFile(redistpath, path, "odbc32gt.dll") &&
		VersionCheckCopyFile(redistpath, path, "odbccp32.cpl") &&
		VersionCheckCopyFile(redistpath, path, "odbccp32.dll") &&
		VersionCheckCopyFile(redistpath, path, "odbccr32.dll") &&
		VersionCheckCopyFile(redistpath, path, "odbcint.dll") &&
		VersionCheckCopyFile(redistpath, path, "odbctrac.dll");
/* 	VersionCheckCopyFile(redistpath, path, "odbcinst.cnt"); */
/* 	VersionCheckCopyFile(redistpath, path, "odbcinst.hlp"); */
}

static BOOL
InstallMyDriver(const char *driverpath)
{
	char driver[300];
	char inpath[301];
	char outpath[301];
	WORD outpathlen;
	DWORD usagecount;
	char *p;

	/* the correct format of driver keywords are
	 * "DriverName\0Driver=xxxxxx.DLL\0Setup=xxxxxx.DLL\0\0" */

	snprintf(driver, sizeof(driver),
		 "%s;Driver=%s;Setup=%s;", DriverName, DriverDLL, DriverDLLs);

	for (p = driver; *p; p++)
		if (*p == ';')
			*p = '\0';

	/* the driver array is filled in before calling
	 * SQLInstallDriverEx so that SQLInstallDriverEx will return
	 * where to install the driver in the inpath */

	SQLInstallDriverEx(driver, NULL, inpath, sizeof(inpath), &outpathlen,
			   ODBC_INSTALL_INQUIRY, &usagecount);

	/* the correct format of driver keywords are
	 * "DriverName\0Driver=c:\winnt\system32\xxxxxx.DLL\0Setup=c:\winnt\system32\xxxxxx.DLL\0\0" */

	snprintf(driver, sizeof(driver),
		 "%s;Driver=%s\\%s;Setup=%s\\%s;APILevel=1;"
		 "ConnectFunctions=YYN;DriverODBCVer=03.51;SQLLevel=3;",
		 DriverName, inpath, DriverDLL, inpath, DriverDLLs);

	for (p = driver; *p; p++)
		if (*p == ';')
			*p = '\0';

	if (!VersionCheckCopyFile(driverpath, inpath, DriverDLL))
		if (ProcessErrorMessages("SQLInstallDriverEx"))
			return FALSE;

	if (!VersionCheckCopyFile(driverpath, inpath, DriverDLLs))
		if (ProcessErrorMessages("SQLInstallDriverEx"))
			return FALSE;

	/* call SQLInstallDriverEx to install the driver in the
	 * registry */
	if (!SQLInstallDriverEx(driver, inpath, outpath, sizeof(outpath),
				&outpathlen, ODBC_INSTALL_COMPLETE,
				&usagecount) &&
	    ProcessErrorMessages("SQLInstallDriverEx"))
		return FALSE;

	/* call SQLConfigDriver to exercise the driver's setup
	 * functions - some drivers have their setup routines built in
	 * and others require a seperate dll */
	if (!SQLConfigDriver(NULL, ODBC_CONFIG_DRIVER, DriverName,
			     "CPTimeout=60\0\0", outpath, sizeof(outpath),
			     &outpathlen))
		ProcessErrorMessages("SQLConfigDriver");

	return TRUE;
}

static BOOL
RemoveMyDriver()
{
	char buf[300];
	char dirname[300];
	WORD len;
	DWORD usagecount;
	char *p;

	if (!SQLRemoveDriver(DriverName, TRUE, &usagecount))
		ProcessErrorMessages("SQLRemoveDriver");
	else if (usagecount == 0) {
		/* figure out where the files were installed */
		snprintf(buf, sizeof(buf), "%s;Driver=%s;Setup=%s;",
			 DriverName, DriverDLL, DriverDLLs);
		for (p = buf; *p; p++)
			if (*p == ';')
				*p = '\0';
		SQLInstallDriverEx(buf, NULL, dirname, sizeof(dirname),
				   &len, ODBC_INSTALL_INQUIRY, &usagecount);
		/* and the delete them */
		snprintf(buf, sizeof(buf), "%s\\%s", dirname, DriverDLL);
		DeleteFile(buf);
		snprintf(buf, sizeof(buf), "%s\\%s", dirname, DriverDLLs);
		DeleteFile(buf);
	}

	return TRUE;
}

static void
CreateAttributeString(char *attrs, size_t len)
{
	snprintf(attrs, len,
		"DSN=%s;Server=localhost;Database=;UID=monetdb;PWD=monetdb;",
		DataSourceName);

	for (; *attrs; attrs++)
		if (*attrs == ';')
			*attrs = '\0';
}

static BOOL
AddMyDSN()
{
	char attrs[200];

	CreateAttributeString(attrs, sizeof(attrs));

	/* I choose to remove the DSN if it already existed */
	SQLConfigDataSource(NULL, ODBC_REMOVE_SYS_DSN, DriverName, attrs);

	/* then create a new DSN */
	if (!SQLConfigDataSource(NULL, ODBC_ADD_SYS_DSN, DriverName, attrs)) {
		ProcessErrorMessages("SQLConfigDataSource");
		return FALSE;
	}

	return TRUE;
}

static BOOL
RemoveMyDSN()
{
	char buf[300];
	WORD len;

	CreateAttributeString(buf, sizeof(buf));

	if (!SQLConfigDataSource(NULL, ODBC_REMOVE_SYS_DSN, DriverName, buf)) {
		ProcessErrorMessages("SQLConfigDataSource");
		return FALSE;
	}

	return TRUE;
}

static BOOL
Install(const char *redistpath, const char *driverpath)
{
	char path[300];
	WORD pathlen;
	BOOL rc;
	char existingversion[300];
	char newversion[300];
	DWORD usagecount;

	/* first, retrieve the path the driver should be installed to
	 * in path */
	if (!SQLInstallDriverManager(path, sizeof(path), &pathlen)) {
		ProcessErrorMessages("SQLInstallDriverManager");
		return FALSE;
	}

	if (redistpath && !CheckIfFileExists(path, redistpath)) {
		fprintf(stderr, "Can't find ODBC SDK in %s\n", redistpath);
		SQLRemoveDriverManager(&usagecount);
		exit(-10);
	}
	rc = TRUE;
	if (CheckIfFileExists(path, "odbc32.dll")) {
		if (redistpath) {
			char existingfile[_MAX_PATH], newfile[_MAX_PATH];

			snprintf(existingfile, sizeof(existingfile),
				 "%s\\odbc32.dll", path);
			snprintf(newfile, sizeof(newfile),
				 "%s\\odbc32.dll", redistpath);

			GetFileVersion(existingfile, existingversion,
				       sizeof(existingversion));
			GetFileVersion(newfile, newversion,
				       sizeof(newversion));

			if (strcmp(newversion, existingversion) > 0) {
#ifndef SKIP_IF_EXISTS
				if (!SQLRemoveDriverManager(&usagecount)) {
					if (ProcessErrorMessages("SQLRemoveDriverManager"))
						return FALSE;
				} else
					rc = CopyODBCCore(redistpath, path);
#endif
			}
		}
	} else if (redistpath == NULL) {
		fprintf(stderr, "You must install MDAC before you can use the ODBC driver\n");
		return FALSE;
	} else
		rc = CopyODBCCore(redistpath, path);

	if (!rc) {
		/* copy failed */
		SQLRemoveDriverManager(&usagecount);
		return FALSE;
	}

	/* if not, install the driver */
	rc = InstallMyDriver(driverpath);

	if (rc) {
		/* after the driver is installed create the new DSN */
		AddMyDSN();
	}

	return TRUE;
}

static BOOL
Uninstall()
{
	DWORD usagecount;

	RemoveMyDSN();
	RemoveMyDriver();
	SQLRemoveDriverManager(&usagecount);
	return TRUE;
}

int
main(int argc, char **argv)
{
	if (argc != 2) {
		fprintf(stderr, "Usage: %s /Install\n   or: %s /Uninstall\n",
			argv[0]);
		exit(1);
	}
	if (strcmp("/Install", argv[1]) == 0) {
		char *buf = malloc(strlen(argv[0]) + 2);
		char *p;
		strcpy(buf, argv[0]);
		if ((p = strrchr(buf, '\\')) != 0 ||
		    (p = strrchr(buf, '/')) != 0)
			*p = 0;
		else
			strcpy(buf, ".");
		if (!Install(NULL, buf)) {
			fprintf(stderr, "%s: ODBC Install Failed\n", argv[0]);
			exit(1);
		}
	} else if (strcmp("/Uninstall", argv[1]) == 0) {
		if (!Uninstall()) {
			printf("ODBC Uninstall Failed\n");
			exit(1);
		}
	} else {
		fprintf(stderr, "Usage: %s /Install\n   or: %s /Uninstall\n",
			argv[0]);
		exit(1);
	}
	return 0;
}
