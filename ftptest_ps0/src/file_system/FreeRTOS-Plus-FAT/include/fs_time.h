#ifndef _FS_TIME_H_
#define _FS_TIME_H_

#include <sys/time.h>

/* _HT_
The following declarations and functions may be moved to a common directory?
 */
typedef struct xTIME_STRUCT
{
	int tm_sec;   /* Seconds */
	int tm_min;   /* Minutes */
	int tm_hour;  /* Hour (0--23) */
	int tm_mday;  /* Day of month (1--31) */
	int tm_mon;   /* Month (0--11) */
	int tm_year;  /* Year (calendar year minus 2000) */
	int tm_wday;  /* Weekday (0--6; Sunday = 0) */
	int tm_yday;  /* Day of year (0--365) */
	int tm_isdst; /* 0 if daylight savings time is not in effect) */
} FS_TimeStruct_t;

/* Equivalent of mktime() : calculates the number of seconds after 1-1-2000. */
time_t FS_mktime( const FS_TimeStruct_t *pxTimeBuf );
//
///* Equivalent of gmtime_r() : Fills a 'struct tm'. */
//FS_TimeStruct_t *FreeRTOS_gmtime_r( const time_t *pxTime, FS_TimeStruct_t *pxTimeBuf );

/**
 *	@public
 *	@brief	A TIME and DATE object for FreeRTOS+FAT. A FreeRTOS+FAT time driver must populate these values.
 *
 **/
typedef struct
{
	uint16_t Year;		/* Year	(e.g. 2009). */
	uint16_t Month;		/* Month(e.g. 1 = Jan, 12 = Dec). */
	uint16_t Day;		/* Day	(1 - 31). */
	uint16_t Hour;		/* Hour	(0 - 23). */
	uint16_t Minute;	/* Min	(0 - 59). */
	uint16_t Second;	/* Second	(0 - 59). */
} FS_SystemTime_t;

/*---------- PROTOTYPES */

int32_t	FS_GetSystemTime(FS_SystemTime_t *pxTime);

#endif /* _FS_TIME_H_*/

