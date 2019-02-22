#include "includes.h"

static long _start_sec = 0;

void time_init()
{
	struct timespec start_time;
	clock_gettime(CLOCK_MONOTONIC, &start_time);
	_start_sec = (start_time.tv_sec + (start_time.tv_nsec / (1000 * 1000 * 1000)));
}

unsigned int get_system_sec(void)
{
	struct timespec now;
	long sec = 0;
	long current = 0;

	clock_gettime(CLOCK_MONOTONIC, &now);
	current = (now.tv_sec + (now.tv_nsec / (1000 * 1000 * 1000)));
	sec = current - _start_sec;
	return (unsigned int)sec;
}

unsigned long GetTimestrip_us()
{
	struct timeval tCurr;
	gettimeofday(&tCurr, NULL);
	return (1000000L * tCurr.tv_sec + tCurr.tv_usec);
}

void GetSystemTime(int *Year, int *Mon, int *Data, int *Hour, int *Min, int *Sec)
{
	time_t now;          //ʵ����time_t�ṹ

	struct tm *timenow;  //ʵ����tm�ṹָ��

	time(&now);

    //time������ȡ���ڵ�ʱ��(���ʱ�׼ʱ��Ǳ���ʱ��)��Ȼ��ֵ��now
    timenow = localtime(&now);	

	//�ֱ��� �� �� �� ʱ  ��
	if (Year != NULL)
	{
		*Year = timenow->tm_year - 100;
	}
	if (Mon != NULL)
	{
		*Mon = timenow->tm_mon + 1; // 0 --- 11
	}
	if (Data != NULL)
	{
		*Data = timenow->tm_mday;
	}
	if (Hour != NULL)
	{
		*Hour = timenow->tm_hour;
	}
	if (Min != NULL)
	{
		*Min = timenow->tm_min;
	}
	if (Sec != NULL)
	{
		*Sec = timenow->tm_sec;
	}

	//localtime�����Ѵ�timeȡ�õ�ʱ��now�����������е�ʱ��(���������õĵ���)
	//asctime������ʱ��ת�����ַ���ͨ��printf()�������
    //PRINTF("Local   time   is   %s/n",asctime(timenow));
	//Tue Jun 10 14:25:14 2014
}

int GetSystemUTCTime(int *Year, int *Mon, int *Data, int *Hour, int *Min, int *Sec)
{
	time_t cur_time = time(NULL);
	if (cur_time < 0)
	{
		LOG_TRACE_PERROR("time error.");
		return -1;
	}

	struct tm utc_tm;
	if (NULL == gmtime_r(&cur_time, &utc_tm))
	{
		LOG_TRACE_PERROR("gmtime error.");
		return -1;
	}

	struct tm *timenow = &utc_tm;

	//�ֱ��� �� �� �� ʱ  ��
	if (Year != NULL)
	{
		*Year = timenow->tm_year - 100;
	}
	if (Mon != NULL)
	{
		*Mon = timenow->tm_mon + 1; // 0 --- 11
	}
	if (Data != NULL)
	{
		*Data = timenow->tm_mday;
	}
	if (Hour != NULL)
	{
		*Hour = timenow->tm_hour;
	}
	if (Min != NULL)
	{
		*Min = timenow->tm_min;
	}
	if (Sec != NULL)
	{
		*Sec = timenow->tm_sec;
	}

	return 0;
}

void gmt_time(char *str, char *gmt, int bufsize)
{
	int y = 0, m = 0, d = 0, h = 0, min = 0, s = 0;
	GetSystemUTCTime(&y, &m, &d, &h, &min, &s);
    snprintf(gmt, bufsize - 1, "%s: 20%d%d-%d%d-%d%d %d%d:%d%d:%d%d %d", 
    							str ? str : "*", 
    							y / 10, y % 10, 
    							m / 10, m % 10, 
    							d / 10, d % 10, 
    							h / 10, h % 10, 
    						  min / 10, min % 10, 
    							s / 10, s % 10, 
    							SYSTEM_SEC);
}

void system_time(char *str, char *sys_time, int bufsize)
{
	int y = 0, m = 0, d = 0, h = 0, min = 0, s = 0;
	GetSystemTime(&y, &m, &d, &h, &min, &s);
    snprintf(sys_time, bufsize - 1, "%s: 20%d%d-%d%d-%d%d %d%d:%d%d:%d%d %u", 
    							str ? str : "*", 
    							y / 10, y % 10, 
    							m / 10, m % 10, 
    							d / 10, d % 10, 
    							h / 10, h % 10, 
    						  min / 10, min % 10, 
    							s / 10, s % 10, 
    							SYSTEM_SEC);
}

void gmt_time_field(int *y, int *m, int *d, int *h, int *min, int *s, unsigned int *system_sec)
{
	int year = 0, mon = 0, day = 0, hour = 0, minute = 0, sec = 0;
	GetSystemUTCTime(&year, &mon, &day, &hour, &minute, &sec);

	if (y) *y = year;
	if (m) *m = mon;
	if (d) *d = day;
	if (h) *h = hour;
	if (min) *min = minute;
	if (s) *s = sec;
	if (system_sec) *system_sec = SYSTEM_SEC;
}

long gmt_sec_time()
{
	time_t ret_sec; 

	time_t cur_time = time(NULL);
	if (cur_time < 0)
	{
		LOG_TRACE_PERROR("time error.");
		return -1;
	}

	struct tm utc_tm;
	if (NULL == gmtime_r(&cur_time, &utc_tm))
	{
		LOG_TRACE_PERROR("gmtime error.");
		return -1;
	}

	ret_sec = timegm(&utc_tm);
	
	return (long)ret_sec;
}

//��ȡbase_time֮ǰ��ʱ��
int gmt_sync(char *str, char *gmt, int bufsize, long base_time)
{
	struct tm utm;

	int y = 0, m = 0, d = 0, h = 0, min = 0, s = 0;
	GetSystemUTCTime(&y, NULL, NULL, NULL, NULL, NULL);
	if (y < 17)
		return -1;

	long gmt_ts = gmt_sec_time();
	if (gmt_ts <= 0)
		return -1;

	long base_gmt_ts = gmt_ts - base_time;
	gmtime_r((time_t *)&base_gmt_ts, &utm);
	
	y = utm.tm_year - 100;
	m = utm.tm_mon + 1;
	d = utm.tm_mday;
	h = utm.tm_hour;
	min = utm.tm_min;
	s = utm.tm_sec;

	if (y < 17)
		return -1;

    snprintf(gmt, bufsize - 1, "%s: 20%d%d-%d%d-%d%d %d%d:%d%d:%d%d %d\n", 
    							str ? str : "*", 
    							y / 10, y % 10, 
    							m / 10, m % 10, 
    							d / 10, d % 10, 
    							h / 10, h % 10, 
    						  min / 10, min % 10, 
    							s / 10, s % 10, 
    							SYSTEM_SEC);

	return 0;
}

void check_gmt_time(char *str, char *gmt_time, int bufsize, int base_time)
{
	char str_y[10] = {0};
	
	char *p = strchr(gmt_time, ' ');
	char *q = strchr(gmt_time, '-');
	if (p && q)
	{
		p ++;
		if (q - p != 4)
			return ;
		
		memcpy(str_y, p, 4);
	}
	
	if (atoi(str_y) < 2017)
	{
		gmt_sync(str, gmt_time, bufsize, base_time);
	}
}


