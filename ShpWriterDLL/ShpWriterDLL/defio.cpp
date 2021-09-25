#include "shapefil.h"

#include <math.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#ifdef USE_CPL
#include "cpl_string.h"
#else

#if defined(_MSC_VER)
# if _MSC_VER < 1900
#     define snprintf _snprintf
# endif
#elif defined(WIN32) || defined(_WIN32)
#  ifndef snprintf
#     define snprintf _snprintf
#  endif
#endif

#define CPLsprintf sprintf
#define CPLsnprintf snprintf
#endif

//SHP_CVSID("$Id: dbfopen.c,v 1.92 2016-12-05 18:44:08 erouault Exp $")

#ifndef FALSE
#  define FALSE		0
#  define TRUE		1
#endif

/* File header size */
#define XBASE_FILEHDR_SZ         32

#define HEADER_RECORD_TERMINATOR 0x0D

/* See http://www.manmrk.net/tutorials/database/xbase/dbf.html */
#define END_OF_FILE_CHARACTER    0x1A

#ifdef USE_CPL
CPL_INLINE static void CPL_IGNORE_RET_VAL_INT(CPL_UNUSED int unused) {}
#else
#define CPL_IGNORE_RET_VAL_INT(x) x
#endif

/************************************************************************/
/*                             SfRealloc()                              */
/*                                                                      */
/*      A realloc cover function that will access a NULL pointer as     */
/*      a valid input.                                                  */
/************************************************************************/
static void * SfRealloc(void * pMem, int nNewSize)
{
	if (pMem == NULL)
		return((void *)malloc(nNewSize));
	else
		return((void *)realloc(pMem, nNewSize));
}

/************************************************************************/
/*                           DBFWriteHeader()                           */
/*                                                                      */
/*      This is called to write out the file header, and field          */
/*      descriptions before writing any actual data records.  This      */
/*      also computes all the DBFDataSet field offset/size/decimals     */
/*      and so forth values.                                            */
/************************************************************************/
static void DBFWriteHeader(DBFHandle psDBF)
{
	unsigned char	abyHeader[XBASE_FILEHDR_SZ] = { 0 };

	if (!psDBF->bNoHeader)
		return;

	psDBF->bNoHeader = FALSE;

	/* -------------------------------------------------------------------- */
	/*	Initialize the file header information.				*/
	/* -------------------------------------------------------------------- */
	abyHeader[0] = 0x03;		/* memo field? - just copying 	*/

	/* write out update date */
	abyHeader[1] = (unsigned char)psDBF->nUpdateYearSince1900;
	abyHeader[2] = (unsigned char)psDBF->nUpdateMonth;
	abyHeader[3] = (unsigned char)psDBF->nUpdateDay;

	/* record count preset at zero */

	abyHeader[8] = (unsigned char)(psDBF->nHeaderLength % 256);
	abyHeader[9] = (unsigned char)(psDBF->nHeaderLength / 256);

	abyHeader[10] = (unsigned char)(psDBF->nRecordLength % 256);
	abyHeader[11] = (unsigned char)(psDBF->nRecordLength / 256);

	abyHeader[29] = (unsigned char)(psDBF->iLanguageDriver);

	/* -------------------------------------------------------------------- */
	/*      Write the initial 32 byte file header, and all the field        */
	/*      descriptions.                                     		*/
	/* -------------------------------------------------------------------- */
	psDBF->sHooks.FSeek(psDBF->fp, 0, 0);
	psDBF->sHooks.FWrite(abyHeader, XBASE_FILEHDR_SZ, 1, psDBF->fp);
	psDBF->sHooks.FWrite(psDBF->pszHeader, XBASE_FLDHDR_SZ, psDBF->nFields,
		psDBF->fp);

	/* -------------------------------------------------------------------- */
	/*      Write out the newline character if there is room for it.        */
	/* -------------------------------------------------------------------- */
	if (psDBF->nHeaderLength > XBASE_FLDHDR_SZ*psDBF->nFields +
		XBASE_FLDHDR_SZ)
	{
		char	cNewline;

		cNewline = HEADER_RECORD_TERMINATOR;
		psDBF->sHooks.FWrite(&cNewline, 1, 1, psDBF->fp);
	}

	/* -------------------------------------------------------------------- */
	/*      If the file is new, add a EOF character.                        */
	/* -------------------------------------------------------------------- */
	if (psDBF->nRecords == 0 && psDBF->bWriteEndOfFileChar)
	{
		char ch = END_OF_FILE_CHARACTER;

		psDBF->sHooks.FWrite(&ch, 1, 1, psDBF->fp);
	}
}

/************************************************************************/
/*                           DBFFlushRecord()                           */
/*                                                                      */
/*      Write out the current record if there is one.                   */
/************************************************************************/
static int DBFFlushRecord(DBFHandle psDBF)
{
	SAOffset	nRecordOffset;

	if (psDBF->bCurrentRecordModified && psDBF->nCurrentRecord > -1)
	{
		psDBF->bCurrentRecordModified = FALSE;

		nRecordOffset =
			psDBF->nRecordLength * (SAOffset)psDBF->nCurrentRecord
			+ psDBF->nHeaderLength;

		if (psDBF->sHooks.FSeek(psDBF->fp, nRecordOffset, 0) != 0
			|| psDBF->sHooks.FWrite(psDBF->pszCurrentRecord,
			psDBF->nRecordLength,
			1, psDBF->fp) != 1)
		{
			char szMessage[128];
			snprintf(szMessage, sizeof(szMessage), "Failure writing DBF record %d.",
				psDBF->nCurrentRecord);
			psDBF->sHooks.Error(szMessage);
			return FALSE;
		}

		if (psDBF->nCurrentRecord == psDBF->nRecords - 1)
		{
			if (psDBF->bWriteEndOfFileChar)
			{
				char ch = END_OF_FILE_CHARACTER;
				psDBF->sHooks.FWrite(&ch, 1, 1, psDBF->fp);
			}
		}
	}

	return TRUE;
}

/************************************************************************/
/*                           DBFLoadRecord()                            */
/************************************************************************/
static int DBFLoadRecord(DBFHandle psDBF, int iRecord)

{
	if (psDBF->nCurrentRecord != iRecord)
	{
		SAOffset nRecordOffset;

		if (!DBFFlushRecord(psDBF))
			return FALSE;

		nRecordOffset =
			psDBF->nRecordLength * (SAOffset)iRecord + psDBF->nHeaderLength;

		if (psDBF->sHooks.FSeek(psDBF->fp, nRecordOffset, SEEK_SET) != 0)
		{
			char szMessage[128];
			snprintf(szMessage, sizeof(szMessage), "fseek(%ld) failed on DBF file.",
				(long)nRecordOffset);
			psDBF->sHooks.Error(szMessage);
			return FALSE;
		}

		if (psDBF->sHooks.FRead(psDBF->pszCurrentRecord,
			psDBF->nRecordLength, 1, psDBF->fp) != 1)
		{
			char szMessage[128];
			snprintf(szMessage, sizeof(szMessage), "fread(%d) failed on DBF file.",
				psDBF->nRecordLength);
			psDBF->sHooks.Error(szMessage);
			return FALSE;
		}

		psDBF->nCurrentRecord = iRecord;
	}

	return TRUE;
}

/************************************************************************/
/*                          DBFUpdateHeader()                           */
/************************************************************************/
void SHPAPI_CALL DBFUpdateHeader(DBFHandle psDBF)

{
	unsigned char		abyFileHeader[XBASE_FILEHDR_SZ];

	if (psDBF->bNoHeader)
		DBFWriteHeader(psDBF);

	if (!DBFFlushRecord(psDBF))
		return;

	psDBF->sHooks.FSeek(psDBF->fp, 0, 0);
	psDBF->sHooks.FRead(abyFileHeader, sizeof(abyFileHeader), 1, psDBF->fp);

	abyFileHeader[1] = (unsigned char)psDBF->nUpdateYearSince1900;
	abyFileHeader[2] = (unsigned char)psDBF->nUpdateMonth;
	abyFileHeader[3] = (unsigned char)psDBF->nUpdateDay;
	abyFileHeader[4] = (unsigned char)(psDBF->nRecords % 256);
	abyFileHeader[5] = (unsigned char)((psDBF->nRecords / 256) % 256);
	abyFileHeader[6] = (unsigned char)((psDBF->nRecords / (256 * 256)) % 256);
	abyFileHeader[7] = (unsigned char)((psDBF->nRecords / (256 * 256 * 256)) % 256);

	psDBF->sHooks.FSeek(psDBF->fp, 0, 0);
	psDBF->sHooks.FWrite(abyFileHeader, sizeof(abyFileHeader), 1, psDBF->fp);

	psDBF->sHooks.FFlush(psDBF->fp);
}

/************************************************************************/
/*                       DBFSetLastModifiedDate()                       */
/************************************************************************/
void SHPAPI_CALL DBFSetLastModifiedDate(DBFHandle psDBF, int nYYSince1900, int nMM, int nDD)
{
	psDBF->nUpdateYearSince1900 = nYYSince1900;
	psDBF->nUpdateMonth = nMM;
	psDBF->nUpdateDay = nDD;
}

/************************************************************************/
/*                              DBFOpen()                               */
/*                                                                      */
/*      Open a .dbf file.                                               */
/************************************************************************/

DBFHandle SHPAPI_CALL DBFOpen(const char * pszFilename, const char * pszAccess)

{
	SAHooks sHooks;

	SASetupDefaultHooks(&sHooks);

	return DBFOpenLL(pszFilename, pszAccess, &sHooks);
}

/************************************************************************/
/*                              DBFOpen()                               */
/*                                                                      */
/*      Open a .dbf file.                                               */
/************************************************************************/
DBFHandle SHPAPI_CALL DBFOpenLL(const char * pszFilename, const char * pszAccess, SAHooks *psHooks)

{
	DBFHandle		psDBF;
	SAFile		pfCPG;
	unsigned char	*pabyBuf;
	int			nFields, nHeadLen, iField, i;
	char		*pszBasename, *pszFullname;
	int                 nBufSize = 500;
	size_t              nFullnameLen;

	/* -------------------------------------------------------------------- */
	/*      We only allow the access strings "rb" and "r+".                  */
	/* -------------------------------------------------------------------- */
	if (strcmp(pszAccess, "r") != 0 && strcmp(pszAccess, "r+") != 0
		&& strcmp(pszAccess, "rb") != 0 && strcmp(pszAccess, "rb+") != 0
		&& strcmp(pszAccess, "r+b") != 0)
		return(NULL);

	if (strcmp(pszAccess, "r") == 0)
		pszAccess = "rb";

	if (strcmp(pszAccess, "r+") == 0)
		pszAccess = "rb+";

	/* -------------------------------------------------------------------- */
	/*	Compute the base (layer) name.  If there is any extension	*/
	/*	on the passed in filename we will strip it off.			*/
	/* -------------------------------------------------------------------- */
	pszBasename = (char *)malloc(strlen(pszFilename) + 5);
	strcpy(pszBasename, pszFilename);
	for (i = (int)strlen(pszBasename) - 1;
		i > 0 && pszBasename[i] != '.' && pszBasename[i] != '/'
		&& pszBasename[i] != '\\';
	i--) {
	}

	if (pszBasename[i] == '.')
		pszBasename[i] = '\0';

	nFullnameLen = strlen(pszBasename) + 5;
	pszFullname = (char *)malloc(nFullnameLen);
	snprintf(pszFullname, nFullnameLen, "%s.dbf", pszBasename);

	psDBF = (DBFHandle)calloc(1, sizeof(DBFInfo));
	psDBF->fp = psHooks->FOpen(pszFullname, pszAccess);
	memcpy(&(psDBF->sHooks), psHooks, sizeof(SAHooks));

	if (psDBF->fp == NULL)
	{
		snprintf(pszFullname, nFullnameLen, "%s.DBF", pszBasename);
		psDBF->fp = psDBF->sHooks.FOpen(pszFullname, pszAccess);
	}

	snprintf(pszFullname, nFullnameLen, "%s.cpg", pszBasename);
	pfCPG = psHooks->FOpen(pszFullname, "r");
	if (pfCPG == NULL)
	{
		snprintf(pszFullname, nFullnameLen, "%s.CPG", pszBasename);
		pfCPG = psHooks->FOpen(pszFullname, "r");
	}

	free(pszBasename);
	free(pszFullname);

	if (psDBF->fp == NULL)
	{
		free(psDBF);
		if (pfCPG) psHooks->FClose(pfCPG);
		return(NULL);
	}

	psDBF->bNoHeader = FALSE;
	psDBF->nCurrentRecord = -1;
	psDBF->bCurrentRecordModified = FALSE;

	/* -------------------------------------------------------------------- */
	/*  Read Table Header info                                              */
	/* -------------------------------------------------------------------- */
	pabyBuf = (unsigned char *)malloc(nBufSize);
	if (psDBF->sHooks.FRead(pabyBuf, XBASE_FILEHDR_SZ, 1, psDBF->fp) != 1)
	{
		psDBF->sHooks.FClose(psDBF->fp);
		if (pfCPG) psDBF->sHooks.FClose(pfCPG);
		free(pabyBuf);
		free(psDBF);
		return NULL;
	}

	DBFSetLastModifiedDate(psDBF, pabyBuf[1], pabyBuf[2], pabyBuf[3]);

	psDBF->nRecords =
		pabyBuf[4] + pabyBuf[5] * 256 + pabyBuf[6] * 256 * 256 + (pabyBuf[7] & 0x7f) * 256 * 256 * 256;

	psDBF->nHeaderLength = nHeadLen = pabyBuf[8] + pabyBuf[9] * 256;
	psDBF->nRecordLength = pabyBuf[10] + pabyBuf[11] * 256;
	psDBF->iLanguageDriver = pabyBuf[29];

	if (psDBF->nRecordLength == 0 || nHeadLen < XBASE_FILEHDR_SZ)
	{
		psDBF->sHooks.FClose(psDBF->fp);
		if (pfCPG) psDBF->sHooks.FClose(pfCPG);
		free(pabyBuf);
		free(psDBF);
		return NULL;
	}

	psDBF->nFields = nFields = (nHeadLen - XBASE_FILEHDR_SZ) / XBASE_FLDHDR_SZ;

	psDBF->pszCurrentRecord = (char *)malloc(psDBF->nRecordLength);

	/* -------------------------------------------------------------------- */
	/*  Figure out the code page from the LDID and CPG                      */
	/* -------------------------------------------------------------------- */

	psDBF->pszCodePage = NULL;
	if (pfCPG)
	{
		size_t n;
		memset(pabyBuf, 0, nBufSize);
		psDBF->sHooks.FRead(pabyBuf, nBufSize - 1, 1, pfCPG);
		n = strcspn((char *)pabyBuf, "\n\r");
		if (n > 0)
		{
			pabyBuf[n] = '\0';
			psDBF->pszCodePage = (char *)malloc(n + 1);
			memcpy(psDBF->pszCodePage, pabyBuf, n + 1);
		}
		psDBF->sHooks.FClose(pfCPG);
	}
	if (psDBF->pszCodePage == NULL && pabyBuf[29] != 0)
	{
		snprintf((char *)pabyBuf, nBufSize, "LDID/%d", psDBF->iLanguageDriver);
		psDBF->pszCodePage = (char *)malloc(strlen((char*)pabyBuf) + 1);
		strcpy(psDBF->pszCodePage, (char *)pabyBuf);
	}

	/* -------------------------------------------------------------------- */
	/*  Read in Field Definitions                                           */
	/* -------------------------------------------------------------------- */

	pabyBuf = (unsigned char *)SfRealloc(pabyBuf, nHeadLen);
	psDBF->pszHeader = (char *)pabyBuf;

	psDBF->sHooks.FSeek(psDBF->fp, XBASE_FILEHDR_SZ, 0);
	if (psDBF->sHooks.FRead(pabyBuf, nHeadLen - XBASE_FILEHDR_SZ, 1,
		psDBF->fp) != 1)
	{
		psDBF->sHooks.FClose(psDBF->fp);
		free(pabyBuf);
		free(psDBF->pszCurrentRecord);
		free(psDBF);
		return NULL;
	}

	psDBF->panFieldOffset = (int *)malloc(sizeof(int) * nFields);
	psDBF->panFieldSize = (int *)malloc(sizeof(int) * nFields);
	psDBF->panFieldDecimals = (int *)malloc(sizeof(int) * nFields);
	psDBF->pachFieldType = (char *)malloc(sizeof(char) * nFields);

	for (iField = 0; iField < nFields; iField++)
	{
		unsigned char		*pabyFInfo;

		pabyFInfo = pabyBuf + iField*XBASE_FLDHDR_SZ;

		if (pabyFInfo[11] == 'N' || pabyFInfo[11] == 'F')
		{
			psDBF->panFieldSize[iField] = pabyFInfo[16];
			psDBF->panFieldDecimals[iField] = pabyFInfo[17];
		}
		else
		{
			psDBF->panFieldSize[iField] = pabyFInfo[16];
			psDBF->panFieldDecimals[iField] = 0;

			/*
			** The following seemed to be used sometimes to handle files with long
			** string fields, but in other cases (such as bug 1202) the decimals field
			** just seems to indicate some sort of preferred formatting, not very
			** wide fields.  So I have disabled this code.  FrankW.
			psDBF->panFieldSize[iField] = pabyFInfo[16] + pabyFInfo[17]*256;
			psDBF->panFieldDecimals[iField] = 0;
			*/
		}

		psDBF->pachFieldType[iField] = (char)pabyFInfo[11];
		if (iField == 0)
			psDBF->panFieldOffset[iField] = 1;
		else
			psDBF->panFieldOffset[iField] =
			psDBF->panFieldOffset[iField - 1] + psDBF->panFieldSize[iField - 1];
	}

	DBFSetWriteEndOfFileChar(psDBF, TRUE);

	return(psDBF);
}

/************************************************************************/
/*                              DBFClose()                              */
/************************************************************************/
void SHPAPI_CALL DBFClose(DBFHandle psDBF)
{
	if (psDBF == NULL)
		return;

	/* -------------------------------------------------------------------- */
	/*      Write out header if not already written.                        */
	/* -------------------------------------------------------------------- */
	if (psDBF->bNoHeader)
		DBFWriteHeader(psDBF);

	CPL_IGNORE_RET_VAL_INT(DBFFlushRecord(psDBF));

	/* -------------------------------------------------------------------- */
	/*      Update last access date, and number of records if we have	*/
	/*	write access.                					*/
	/* -------------------------------------------------------------------- */
	if (psDBF->bUpdated)
		DBFUpdateHeader(psDBF);

	/* -------------------------------------------------------------------- */
	/*      Close, and free resources.                                      */
	/* -------------------------------------------------------------------- */
	psDBF->sHooks.FClose(psDBF->fp);

	if (psDBF->panFieldOffset != NULL)
	{
		free(psDBF->panFieldOffset);
		free(psDBF->panFieldSize);
		free(psDBF->panFieldDecimals);
		free(psDBF->pachFieldType);
	}

	if (psDBF->pszWorkField != NULL)
		free(psDBF->pszWorkField);

	free(psDBF->pszHeader);
	free(psDBF->pszCurrentRecord);
	free(psDBF->pszCodePage);

	free(psDBF);
}

/************************************************************************/
/*                             DBFCreate()                              */
/*                                                                      */
/* Create a new .dbf file with default code page LDID/87 (0x57)         */
/************************************************************************/
DBFHandle SHPAPI_CALL DBFCreate(const char * pszFilename)
{
	return DBFCreateEx(pszFilename, "LDID/87"); // 0x57
}

/************************************************************************/
/*                            DBFCreateEx()                             */
/*                                                                      */
/*      Create a new .dbf file.                                         */
/************************************************************************/
DBFHandle SHPAPI_CALL DBFCreateEx(const char * pszFilename, const char* pszCodePage)
{
	SAHooks sHooks;
	SASetupDefaultHooks(&sHooks);
	return DBFCreateLL(pszFilename, pszCodePage, &sHooks);
}

/************************************************************************/
/*                             DBFCreate()                              */
/*                                                                      */
/*      Create a new .dbf file.                                         */
/************************************************************************/
DBFHandle SHPAPI_CALL DBFCreateLL(const char * pszFilename, const char * pszCodePage, SAHooks *psHooks)
{
	DBFHandle	psDBF;
	SAFile	fp;
	char	*pszFullname, *pszBasename;
	int		i, ldid = -1;
	char    chZero = '\0';
	size_t  nFullnameLen;

	/* -------------------------------------------------------------------- */
	/*	Compute the base (layer) name.  If there is any extension	*/
	/*	on the passed in filename we will strip it off.			*/
	/* -------------------------------------------------------------------- */
	pszBasename = (char *)malloc(strlen(pszFilename) + 5);
	strcpy(pszBasename, pszFilename);
	for (i = (int)strlen(pszBasename) - 1; i > 0 && pszBasename[i] != '.' && pszBasename[i] != '/' && pszBasename[i] != '\\'; i--) {}

	if (pszBasename[i] == '.')
		pszBasename[i] = '\0';

	nFullnameLen = strlen(pszBasename) + 5;
	pszFullname = (char *)malloc(nFullnameLen);
	snprintf(pszFullname, nFullnameLen, "%s.dbf", pszBasename);

	/* -------------------------------------------------------------------- */
	/*      Create the file.                                                */
	/* -------------------------------------------------------------------- */
	fp = psHooks->FOpen(pszFullname, "wb");
	if (fp == NULL)
	{
		free(pszBasename);
		free(pszFullname);
		return(NULL);
	}

	psHooks->FWrite(&chZero, 1, 1, fp);
	psHooks->FClose(fp);

	fp = psHooks->FOpen(pszFullname, "rb+");
	if (fp == NULL)
	{
		free(pszBasename);
		free(pszFullname);
		return(NULL);
	}

	snprintf(pszFullname, nFullnameLen, "%s.cpg", pszBasename);
	if (pszCodePage != NULL)
	{
		if (strncmp(pszCodePage, "LDID/", 5) == 0)
		{
			ldid = atoi(pszCodePage + 5);
			if (ldid > 255)
				ldid = -1; // don't use 0 to indicate out of range as LDID/0 is a valid one
		}
		if (ldid < 0)
		{
			SAFile fpCPG = psHooks->FOpen(pszFullname, "w");
			psHooks->FWrite((char*)pszCodePage, strlen(pszCodePage), 1, fpCPG);
			psHooks->FClose(fpCPG);
		}
	}
	if (pszCodePage == NULL || ldid >= 0)
	{
		psHooks->Remove(pszFullname);
	}

	free(pszBasename);
	free(pszFullname);

	/* -------------------------------------------------------------------- */
	/*	Create the info structure.					*/
	/* -------------------------------------------------------------------- */
	psDBF = (DBFHandle)calloc(1, sizeof(DBFInfo));

	memcpy(&(psDBF->sHooks), psHooks, sizeof(SAHooks));
	psDBF->fp = fp;
	psDBF->nRecords = 0;
	psDBF->nFields = 0;
	psDBF->nRecordLength = 1;
	psDBF->nHeaderLength = XBASE_FILEHDR_SZ + 1; /* + 1 for HEADER_RECORD_TERMINATOR */

	psDBF->panFieldOffset = NULL;
	psDBF->panFieldSize = NULL;
	psDBF->panFieldDecimals = NULL;
	psDBF->pachFieldType = NULL;
	psDBF->pszHeader = NULL;

	psDBF->nCurrentRecord = -1;
	psDBF->bCurrentRecordModified = FALSE;
	psDBF->pszCurrentRecord = NULL;

	psDBF->bNoHeader = TRUE;

	psDBF->iLanguageDriver = ldid > 0 ? ldid : 0;
	psDBF->pszCodePage = NULL;
	if (pszCodePage)
	{
		psDBF->pszCodePage = (char *)malloc(strlen(pszCodePage) + 1);
		strcpy(psDBF->pszCodePage, pszCodePage);
	}
	DBFSetLastModifiedDate(psDBF, 95, 7, 26); /* dummy date */

	DBFSetWriteEndOfFileChar(psDBF, TRUE);

	return(psDBF);
}

/************************************************************************/
/*                        DBFGetNullCharacter()                         */
/************************************************************************/
static char DBFGetNullCharacter(char chType)
{
	switch (chType)
	{
	case 'N':
	case 'F':
		return '*';
	case 'D':
		return '0';
	case 'L':
		return '?';
	default:
		return ' ';
	}
}

/************************************************************************/
/*                            DBFAddField()                             */
/*                                                                      */
/*      Add a field to a newly created .dbf file before any records     */
/*      are written.                                                    */
/************************************************************************/
int SHPAPI_CALL DBFAddNativeFieldType(DBFHandle psDBF, const char * pszFieldName, char chType, int nWidth, int nDecimals)
{
	char	*pszFInfo;
	int		i;
	int         nOldRecordLength, nOldHeaderLength;
	char        *pszRecord;
	char        chFieldFill;
	SAOffset    nRecordOffset;

	/* make sure that everything is written in .dbf */
	if (!DBFFlushRecord(psDBF))
		return -1;

	if (psDBF->nHeaderLength + XBASE_FLDHDR_SZ > 65535)
	{
		char szMessage[128];
		snprintf(szMessage, sizeof(szMessage),
			"Cannot add field %s. Header length limit reached "
			"(max 65535 bytes, 2046 fields).",
			pszFieldName);
		psDBF->sHooks.Error(szMessage);
		return -1;
	}

	/* -------------------------------------------------------------------- */
	/*      Do some checking to ensure we can add records to this file.     */
	/* -------------------------------------------------------------------- */
	if (nWidth < 1)
		return -1;

	if (nWidth > XBASE_FLD_MAX_WIDTH)
		nWidth = XBASE_FLD_MAX_WIDTH;

	if (psDBF->nRecordLength + nWidth > 65535)
	{
		char szMessage[128];
		snprintf(szMessage, sizeof(szMessage),
			"Cannot add field %s. Record length limit reached "
			"(max 65535 bytes).",
			pszFieldName);
		psDBF->sHooks.Error(szMessage);
		return -1;
	}

	nOldRecordLength = psDBF->nRecordLength;
	nOldHeaderLength = psDBF->nHeaderLength;

	/* -------------------------------------------------------------------- */
	/*      SfRealloc all the arrays larger to hold the additional field      */
	/*      information.                                                    */
	/* -------------------------------------------------------------------- */
	psDBF->nFields++;

	psDBF->panFieldOffset = (int *)
		SfRealloc(psDBF->panFieldOffset, sizeof(int) * psDBF->nFields);

	psDBF->panFieldSize = (int *)
		SfRealloc(psDBF->panFieldSize, sizeof(int) * psDBF->nFields);

	psDBF->panFieldDecimals = (int *)
		SfRealloc(psDBF->panFieldDecimals, sizeof(int) * psDBF->nFields);

	psDBF->pachFieldType = (char *)
		SfRealloc(psDBF->pachFieldType, sizeof(char) * psDBF->nFields);

	/* -------------------------------------------------------------------- */
	/*      Assign the new field information fields.                        */
	/* -------------------------------------------------------------------- */
	psDBF->panFieldOffset[psDBF->nFields - 1] = psDBF->nRecordLength;
	psDBF->nRecordLength += nWidth;
	psDBF->panFieldSize[psDBF->nFields - 1] = nWidth;
	psDBF->panFieldDecimals[psDBF->nFields - 1] = nDecimals;
	psDBF->pachFieldType[psDBF->nFields - 1] = chType;

	/* -------------------------------------------------------------------- */
	/*      Extend the required header information.                         */
	/* -------------------------------------------------------------------- */
	psDBF->nHeaderLength += XBASE_FLDHDR_SZ;
	psDBF->bUpdated = FALSE;

	psDBF->pszHeader = (char *)SfRealloc(psDBF->pszHeader,
		psDBF->nFields*XBASE_FLDHDR_SZ);

	pszFInfo = psDBF->pszHeader + XBASE_FLDHDR_SZ * (psDBF->nFields - 1);

	for (i = 0; i < XBASE_FLDHDR_SZ; i++)
		pszFInfo[i] = '\0';

	strncpy(pszFInfo, pszFieldName, XBASE_FLDNAME_LEN_WRITE);

	pszFInfo[11] = psDBF->pachFieldType[psDBF->nFields - 1];

	if (chType == 'C')
	{
		pszFInfo[16] = (unsigned char)(nWidth % 256);
		pszFInfo[17] = (unsigned char)(nWidth / 256);
	}
	else
	{
		pszFInfo[16] = (unsigned char)nWidth;
		pszFInfo[17] = (unsigned char)nDecimals;
	}

	/* -------------------------------------------------------------------- */
	/*      Make the current record buffer appropriately larger.            */
	/* -------------------------------------------------------------------- */
	psDBF->pszCurrentRecord = (char *)SfRealloc(psDBF->pszCurrentRecord,
		psDBF->nRecordLength);

	/* we're done if dealing with new .dbf */
	if (psDBF->bNoHeader)
		return(psDBF->nFields - 1);

	/* -------------------------------------------------------------------- */
	/*      For existing .dbf file, shift records                           */
	/* -------------------------------------------------------------------- */

	/* alloc record */
	pszRecord = (char *)malloc(sizeof(char) * psDBF->nRecordLength);

	chFieldFill = DBFGetNullCharacter(chType);

	for (i = psDBF->nRecords - 1; i >= 0; --i)
	{
		nRecordOffset = nOldRecordLength * (SAOffset)i + nOldHeaderLength;

		/* load record */
		psDBF->sHooks.FSeek(psDBF->fp, nRecordOffset, 0);
		psDBF->sHooks.FRead(pszRecord, nOldRecordLength, 1, psDBF->fp);

		/* set new field's value to NULL */
		memset(pszRecord + nOldRecordLength, chFieldFill, nWidth);

		nRecordOffset = psDBF->nRecordLength * (SAOffset)i + psDBF->nHeaderLength;

		/* move record to the new place*/
		psDBF->sHooks.FSeek(psDBF->fp, nRecordOffset, 0);
		psDBF->sHooks.FWrite(pszRecord, psDBF->nRecordLength, 1, psDBF->fp);
	}

	if (psDBF->bWriteEndOfFileChar)
	{
		char ch = END_OF_FILE_CHARACTER;

		nRecordOffset =
			psDBF->nRecordLength * (SAOffset)psDBF->nRecords + psDBF->nHeaderLength;

		psDBF->sHooks.FSeek(psDBF->fp, nRecordOffset, 0);
		psDBF->sHooks.FWrite(&ch, 1, 1, psDBF->fp);
	}

	/* free record */
	free(pszRecord);

	/* force update of header with new header, record length and new field */
	psDBF->bNoHeader = TRUE;
	DBFUpdateHeader(psDBF);

	psDBF->nCurrentRecord = -1;
	psDBF->bCurrentRecordModified = FALSE;
	psDBF->bUpdated = TRUE;

	return(psDBF->nFields - 1);
}

/************************************************************************/
/*                         DBFWriteAttribute()                          */
/*								                                    	*/
/*	Write an attribute record to the file.			                	*/
/************************************************************************/
static int DBFWriteAttribute(DBFHandle psDBF, int hEntity, int iField, void * pValue)
{
	int	       	i, j, nRetResult = TRUE;
	unsigned char	*pabyRec;
	char	szSField[XBASE_FLD_MAX_WIDTH + 1], szFormat[20];

	/* -------------------------------------------------------------------- */
	/*	Is this a valid record?						*/
	/* -------------------------------------------------------------------- */
	if (hEntity < 0 || hEntity > psDBF->nRecords)
		return(FALSE);

	if (psDBF->bNoHeader)
		DBFWriteHeader(psDBF);

	/* -------------------------------------------------------------------- */
	/*      Is this a brand new record?                                     */
	/* -------------------------------------------------------------------- */
	if (hEntity == psDBF->nRecords)
	{
		if (!DBFFlushRecord(psDBF))
			return FALSE;

		psDBF->nRecords++;
		for (i = 0; i < psDBF->nRecordLength; i++)
			psDBF->pszCurrentRecord[i] = ' ';

		psDBF->nCurrentRecord = hEntity;
	}

	/* -------------------------------------------------------------------- */
	/*      Is this an existing record, but different than the last one     */
	/*      we accessed?                                                    */
	/* -------------------------------------------------------------------- */
	if (!DBFLoadRecord(psDBF, hEntity))
		return FALSE;

	pabyRec = (unsigned char *)psDBF->pszCurrentRecord;

	psDBF->bCurrentRecordModified = TRUE;
	psDBF->bUpdated = TRUE;

	/* -------------------------------------------------------------------- */
	/*      Translate NULL value to valid DBF file representation.          */
	/*                                                                      */
	/*      Contributed by Jim Matthews.                                    */
	/* -------------------------------------------------------------------- */
	if (pValue == NULL)
	{
		memset((char *)(pabyRec + psDBF->panFieldOffset[iField]),
			DBFGetNullCharacter(psDBF->pachFieldType[iField]),
			psDBF->panFieldSize[iField]);
		return TRUE;
	}

	/* -------------------------------------------------------------------- */
	/*      Assign all the record fields.                                   */
	/* -------------------------------------------------------------------- */
	switch (psDBF->pachFieldType[iField])
	{
	case 'D':
	case 'N':
	case 'F':
	{
		int		nWidth = psDBF->panFieldSize[iField];

		if ((int) sizeof(szSField) - 2 < nWidth)
			nWidth = sizeof(szSField) - 2;

		snprintf(szFormat, sizeof(szFormat), "%%%d.%df",
			nWidth, psDBF->panFieldDecimals[iField]);
		CPLsnprintf(szSField, sizeof(szSField), szFormat, *((double *)pValue));
		if ((int)strlen(szSField) > psDBF->panFieldSize[iField])
		{
			szSField[psDBF->panFieldSize[iField]] = '\0';
			nRetResult = FALSE;
		}
		strncpy((char *)(pabyRec + psDBF->panFieldOffset[iField]),
			szSField, strlen(szSField));
		break;
	}

	case 'L':
		if (psDBF->panFieldSize[iField] >= 1 &&
			(*(char*)pValue == 'F' || *(char*)pValue == 'T'))
			*(pabyRec + psDBF->panFieldOffset[iField]) = *(char*)pValue;
		break;

	default:
		if ((int)strlen((char *)pValue) > psDBF->panFieldSize[iField])
		{
			j = psDBF->panFieldSize[iField];
			nRetResult = FALSE;
		}
		else
		{
			memset(pabyRec + psDBF->panFieldOffset[iField], ' ',
				psDBF->panFieldSize[iField]);
			j = (int)strlen((char *)pValue);
		}

		strncpy((char *)(pabyRec + psDBF->panFieldOffset[iField]),
			(char *)pValue, j);
		break;
	}

	return(nRetResult);
}

/************************************************************************/
/*                      DBFWriteIntegerAttribute()                      */
/*                                                                      */
/*      Write a integer attribute.                                      */
/************************************************************************/
int SHPAPI_CALL DBFWriteIntegerAttribute(DBFHandle psDBF, int iRecord, int iField, int nValue)
{
	double	dValue = nValue;
	return(DBFWriteAttribute(psDBF, iRecord, iField, (void *)&dValue));
}

/************************************************************************/
/*                         DBFGetRecordCount()                          */
/*                                                                      */
/*      Return the number of records in this table.                     */
/************************************************************************/
int SHPAPI_CALL DBFGetRecordCount(DBFHandle psDBF)
{
	return(psDBF->nRecords);
}

/************************************************************************/
/*                    DBFSetWriteEndOfFileChar()                        */
/************************************************************************/
void SHPAPI_CALL DBFSetWriteEndOfFileChar(DBFHandle psDBF, int bWriteFlag)
{
	psDBF->bWriteEndOfFileChar = bWriteFlag;
}