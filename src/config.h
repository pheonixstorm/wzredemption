/*
 * config.h
 * load and save favourites to the registry.
 */
#ifndef config_h
#define config_h

extern BOOL loadConfig				(BOOL bResourceAvailable);
extern BOOL loadRenderMode			(VOID);
extern BOOL saveConfig				(VOID);
extern BOOL getWarzoneKeyNumeric	(STRING *pName,DWORD *val);
extern BOOL openWarzoneKey			(VOID);
extern BOOL closeWarzoneKey			(VOID);
extern BOOL setWarzoneKeyNumeric	(STRING *pName,DWORD val);

extern BOOL	bAllowSubtitles;

BOOL getWarzoneKeyString	(STRING *pName,STRING *pString);
BOOL getWarzoneKeyBinary	(STRING *pName,UCHAR  *pData,UDWORD *pSize);
BOOL setWarzoneKeyString	(STRING *pName,STRING *pString);
BOOL setWarzoneKeyBinary	(STRING *pName, VOID *pData, UDWORD size);

#endif
