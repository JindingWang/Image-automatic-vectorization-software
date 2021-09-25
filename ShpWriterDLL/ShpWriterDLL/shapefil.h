#ifndef SHAPEFILE_H_INCLUDED
#define SHAPEFILE_H_INCLUDED

#include <stdio.h>

#ifndef SHPAPI_CALL
#  if defined(USE_GCC_VISIBILITY_FLAG)
#    define SHPAPI_CALL     __attribute__ ((visibility("default")))
#    define SHPAPI_CALL1(x) __attribute__ ((visibility("default")))     x
#  else
#    define SHPAPI_CALL
#  endif
#endif

#ifndef SHPAPI_CALL1
#  define SHPAPI_CALL1(x)      x SHPAPI_CALL
#endif

/* -------------------------------------------------------------------- */
/*      IO/Error hook functions.                                        */
/* -------------------------------------------------------------------- */
typedef int *SAFile;
typedef unsigned long SAOffset;

typedef struct {
	SAFile(*FOpen) (const char *filename, const char *access);
	SAOffset(*FRead) (void *p, SAOffset size, SAOffset nmemb, SAFile file);
	SAOffset(*FWrite)(void *p, SAOffset size, SAOffset nmemb, SAFile file);
	SAOffset(*FSeek) (SAFile file, SAOffset offset, int whence);
	SAOffset(*FTell) (SAFile file);
	int(*FFlush)(SAFile file);
	int(*FClose)(SAFile file);
	int(*Remove) (const char *filename);

	void(*Error) (const char *message);
	double(*Atof)  (const char *str);
} SAHooks;

void SHPAPI_CALL SASetupDefaultHooks(SAHooks *psHooks);
/************************************************************************/
/*                             SHP Support.                             */
/************************************************************************/
typedef struct tagSHPObject SHPObject;

typedef struct
{
	SAHooks sHooks;

	SAFile      fpSHP;
	SAFile      fpSHX;

	int         nShapeType;  /* SHPT_* */

	unsigned int nFileSize;  /* SHP file */

	int         nRecords;
	int         nMaxRecords;
	unsigned int*panRecOffset;
	unsigned int *panRecSize;

	double      adBoundsMin[4];
	double      adBoundsMax[4];

	int         bUpdated;

	unsigned char *pabyRec;
	int         nBufSize;

	int            bFastModeReadObject;
	unsigned char *pabyObjectBuf;
	int            nObjectBufSize;
	SHPObject*     psCachedObject;
} SHPInfo;

typedef SHPInfo * SHPHandle;

/* -------------------------------------------------------------------- */
/*      Shape types (nSHPType)                                          */
/* -------------------------------------------------------------------- */
#define SHPT_NULL       0
#define SHPT_POINT      1
#define SHPT_ARC        3
#define SHPT_POLYGON    5
#define SHPT_MULTIPOINT 8
#define SHPT_POINTZ     11
#define SHPT_ARCZ       13
#define SHPT_POLYGONZ   15
#define SHPT_MULTIPOINTZ 18
#define SHPT_POINTM     21
#define SHPT_ARCM       23
#define SHPT_POLYGONM   25
#define SHPT_MULTIPOINTM 28
#define SHPT_MULTIPATCH 31

/* -------------------------------------------------------------------- */
/*      Part types - everything but SHPT_MULTIPATCH just uses           */
/*      SHPP_RING.                                                      */
/* -------------------------------------------------------------------- */
#define SHPP_TRISTRIP   0
#define SHPP_TRIFAN     1
#define SHPP_OUTERRING  2
#define SHPP_INNERRING  3
#define SHPP_FIRSTRING  4
#define SHPP_RING       5

/* -------------------------------------------------------------------- */
/*      SHPObject - represents on shape (without attributes) read       */
/*      from the .shp file.                                             */
/* -------------------------------------------------------------------- */
struct tagSHPObject
{
	int    nSHPType;

	int    nShapeId;  /* -1 is unknown/unassigned */

	int    nParts;
	int    *panPartStart;
	int    *panPartType;

	int    nVertices;
	double *padfX;
	double *padfY;
	double *padfZ;
	double *padfM;

	double dfXMin;
	double dfYMin;
	double dfZMin;
	double dfMMin;

	double dfXMax;
	double dfYMax;
	double dfZMax;
	double dfMMax;

	int    bMeasureIsUsed;
	int    bFastModeReadObject;
};

/* -------------------------------------------------------------------- */
/*      SHP API Prototypes                                              */
/* -------------------------------------------------------------------- */
/* If setting bFastMode = TRUE, the content of SHPReadObject() is owned by the SHPHandle. */
/* So you cannot have 2 valid instances of SHPReadObject() simultaneously. */
/* The SHPObject padfZ and padfM members may be NULL depending on the geometry */
/* type. It is illegal to free at hand any of the pointer members of the SHPObject structure */
void SHPAPI_CALL SHPSetFastModeReadObject(SHPHandle hSHP, int bFastMode);

SHPHandle SHPAPI_CALL SHPCreate(const char * pszShapeFile, int nShapeType);
SHPHandle SHPAPI_CALL SHPCreateLL(const char * pszShapeFile, int nShapeType, SAHooks *psHooks);
void SHPAPI_CALL SHPGetInfo(SHPHandle hSHP, int * pnEntities, int * pnShapeType, double * padfMinBound, double * padfMaxBound);

SHPObject SHPAPI_CALL1(*) SHPReadObject(SHPHandle hSHP, int iShape);
int SHPAPI_CALL SHPWriteObject(SHPHandle hSHP, int iShape, SHPObject * psObject);

void SHPAPI_CALL SHPDestroyObject(SHPObject * psObject);
void SHPAPI_CALL SHPComputeExtents(SHPObject * psObject);
SHPObject SHPAPI_CALL1(*) SHPCreateObject(int nSHPType, int nShapeId, int nParts,
										  const int * panPartStart, const int * panPartType,
									      int nVertices,
										  const double * padfX, const double * padfY,
										  const double * padfZ, const double * padfM);
SHPObject SHPAPI_CALL1(*) SHPCreateSimpleObject(int nSHPType, int nVertices,
												const double * padfX,
												const double * padfY,
												const double * padfZ);

int SHPAPI_CALL SHPRewindObject(SHPHandle hSHP, SHPObject * psObject);

void SHPAPI_CALL SHPClose(SHPHandle hSHP);
void SHPAPI_CALL SHPWriteHeader(SHPHandle hSHP);

const char SHPAPI_CALL1(*) SHPTypeName(int nSHPType);
const char SHPAPI_CALL1(*) SHPPartTypeName(int nPartType);

/************************************************************************/
/*                             DBF Support.                             */
/************************************************************************/
	typedef struct
	{
		SAHooks sHooks;

		SAFile      fp;

		int         nRecords;

		int         nRecordLength; /* Must fit on uint16 */
		int         nHeaderLength; /* File header length (32) + field
								   descriptor length + spare space.
								   Must fit on uint16 */
		int         nFields;
		int         *panFieldOffset;
		int         *panFieldSize;
		int         *panFieldDecimals;
		char        *pachFieldType;

		char        *pszHeader; /* Field descriptors */

		int         nCurrentRecord;
		int         bCurrentRecordModified;
		char        *pszCurrentRecord;

		int         nWorkFieldLength;
		char        *pszWorkField;

		int         bNoHeader;
		int         bUpdated;

		union
		{
			double      dfDoubleField;
			int         nIntField;
		} fieldValue;

		int         iLanguageDriver;
		char        *pszCodePage;

		int         nUpdateYearSince1900; /* 0-255 */
		int         nUpdateMonth; /* 1-12 */
		int         nUpdateDay; /* 1-31 */

		int         bWriteEndOfFileChar; /* defaults to TRUE */
	} DBFInfo;

	typedef DBFInfo * DBFHandle;

	typedef enum {
		FTString,
		FTInteger,
		FTDouble,
		FTLogical,
		FTInvalid
	} DBFFieldType;

	/* Field descriptor/header size */
#define XBASE_FLDHDR_SZ         32
	/* Shapelib read up to 11 characters, even if only 10 should normally be used */
#define XBASE_FLDNAME_LEN_READ  11
	/* On writing, we limit to 10 characters */
#define XBASE_FLDNAME_LEN_WRITE 10
	/* Normally only 254 characters should be used. We tolerate 255 historically */
#define XBASE_FLD_MAX_WIDTH     255

	DBFHandle SHPAPI_CALL
		DBFOpen(const char * pszDBFFile, const char * pszAccess);
	DBFHandle SHPAPI_CALL
		DBFOpenLL(const char * pszDBFFile, const char * pszAccess,
		SAHooks *psHooks);
	DBFHandle SHPAPI_CALL
		DBFCreate(const char * pszDBFFile);
	DBFHandle SHPAPI_CALL
		DBFCreateEx(const char * pszDBFFile, const char * pszCodePage);
	DBFHandle SHPAPI_CALL
		DBFCreateLL(const char * pszDBFFile, const char * pszCodePage, SAHooks *psHooks);

	int SHPAPI_CALL
		DBFGetFieldCount(DBFHandle psDBF);
	int SHPAPI_CALL
		DBFGetRecordCount(DBFHandle psDBF);
	int SHPAPI_CALL
		DBFAddField(DBFHandle hDBF, const char * pszFieldName,
		DBFFieldType eType, int nWidth, int nDecimals);

	int SHPAPI_CALL
		DBFAddNativeFieldType(DBFHandle hDBF, const char * pszFieldName,
		char chType, int nWidth, int nDecimals);

	int SHPAPI_CALL
		DBFDeleteField(DBFHandle hDBF, int iField);

	int SHPAPI_CALL
		DBFReorderFields(DBFHandle psDBF, int* panMap);

	int SHPAPI_CALL
		DBFAlterFieldDefn(DBFHandle psDBF, int iField, const char * pszFieldName,
		char chType, int nWidth, int nDecimals);

	DBFFieldType SHPAPI_CALL
		DBFGetFieldInfo(DBFHandle psDBF, int iField,
		char * pszFieldName, int * pnWidth, int * pnDecimals);

	int SHPAPI_CALL
		DBFGetFieldIndex(DBFHandle psDBF, const char *pszFieldName);

	int SHPAPI_CALL
		DBFReadIntegerAttribute(DBFHandle hDBF, int iShape, int iField);
	double SHPAPI_CALL
		DBFReadDoubleAttribute(DBFHandle hDBF, int iShape, int iField);
	const char SHPAPI_CALL1(*)
		DBFReadStringAttribute(DBFHandle hDBF, int iShape, int iField);
	const char SHPAPI_CALL1(*)
		DBFReadLogicalAttribute(DBFHandle hDBF, int iShape, int iField);
	int SHPAPI_CALL
		DBFIsAttributeNULL(DBFHandle hDBF, int iShape, int iField);

	int SHPAPI_CALL
		DBFWriteIntegerAttribute(DBFHandle hDBF, int iShape, int iField,
		int nFieldValue);
	int SHPAPI_CALL
		DBFWriteDoubleAttribute(DBFHandle hDBF, int iShape, int iField,
		double dFieldValue);
	int SHPAPI_CALL
		DBFWriteStringAttribute(DBFHandle hDBF, int iShape, int iField,
		const char * pszFieldValue);
	int SHPAPI_CALL
		DBFWriteNULLAttribute(DBFHandle hDBF, int iShape, int iField);

	int SHPAPI_CALL
		DBFWriteLogicalAttribute(DBFHandle hDBF, int iShape, int iField,
		const char lFieldValue);
	int SHPAPI_CALL
		DBFWriteAttributeDirectly(DBFHandle psDBF, int hEntity, int iField,
		void * pValue);
	const char SHPAPI_CALL1(*)
		DBFReadTuple(DBFHandle psDBF, int hEntity);
	int SHPAPI_CALL
		DBFWriteTuple(DBFHandle psDBF, int hEntity, void * pRawTuple);

	int SHPAPI_CALL DBFIsRecordDeleted(DBFHandle psDBF, int iShape);
	int SHPAPI_CALL DBFMarkRecordDeleted(DBFHandle psDBF, int iShape,
		int bIsDeleted);

	DBFHandle SHPAPI_CALL
		DBFCloneEmpty(DBFHandle psDBF, const char * pszFilename);

	void SHPAPI_CALL
		DBFClose(DBFHandle hDBF);
	void    SHPAPI_CALL
		DBFUpdateHeader(DBFHandle hDBF);
	char SHPAPI_CALL
		DBFGetNativeFieldType(DBFHandle hDBF, int iField);

	const char SHPAPI_CALL1(*)
		DBFGetCodePage(DBFHandle psDBF);

	void SHPAPI_CALL
		DBFSetLastModifiedDate(DBFHandle psDBF, int nYYSince1900, int nMM, int nDD);

	void SHPAPI_CALL DBFSetWriteEndOfFileChar(DBFHandle psDBF, int bWriteFlag);

#endif /* ndef SHAPEFILE_H_INCLUDED */