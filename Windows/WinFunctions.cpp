#ifdef WINDOWS

// implementation of Linux functions for Windows

#include <WinFunctions.h>

#include <WinSock.h> // struct timeval
#include <stdint.h>

#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
#define DELTA_EPOCH_IN_MICROSECS 11644473600000000Ui64
#else
#define DELTA_EPOCH_IN_MICROSECS 11644473600000000ULL
#endif

int gettimeofday(struct timeval *tv, struct timezone *tzp) {
	FILETIME file_time;
	GetSystemTimeAsFileTime(&file_time);

	uint64_t time = ((uint64_t)file_time.dwLowDateTime);
	time += ((uint64_t)file_time.dwHighDateTime) << 32;

	time /= 10; // convert into microseconds
	time -= DELTA_EPOCH_IN_MICROSECS;
	tv->tv_sec = (long)(time / 1000000UL);
	tv->tv_usec = (long)(time % 1000000UL);
	return 0;
}

DIR *opendir(const char *dirname)
{
	if (strlen(dirname) >= NAME_LENGTH) {
		printf("ERROR: directory name candidate, %s, is too long (%d >= %d)\n", 
			dirname, strlen(dirname), NAME_LENGTH);
		return NULL;
	}

	DWORD dw = GetFileAttributes(dirname);
	if ((dw & FILE_ATTRIBUTE_DIRECTORY) == 0)
		return NULL;
		
	DIR *dir = new DIR;
	dir->hFind = INVALID_HANDLE_VALUE;
	_snprintf(dir->dirname, sizeof(dir->dirname), "%s\\*", dirname);
	return dir;
}

struct dirent *readdir(DIR *dir)
{
	if (dir == NULL) {
		return NULL;
	}

	if (dir->hFind == INVALID_HANDLE_VALUE) {
		dir->hFind = FindFirstFile(dir->dirname, &dir->findFileData);
		if (dir->hFind == INVALID_HANDLE_VALUE) {
			return NULL;
		}
	}
	else {
		if (!FindNextFile(dir->hFind, &dir->findFileData)) {
			return NULL;
		}
	}

	_snprintf(dir->findDirEnt.d_name, sizeof(dir->findDirEnt.d_name), "%s", dir->findFileData.cFileName);

	return &dir->findDirEnt;
}


int closedir(DIR *dir)
{
	if (dir != NULL) {
		if (dir->hFind != INVALID_HANDLE_VALUE) {
			FindClose(dir->hFind);
		}
		delete dir;
	}
	return 0;
}

struct tm *localtime_r(const time_t *timep, struct tm *results)
{
	struct tm *tm_s = localtime(timep);
	memcpy(results, tm_s, sizeof(struct tm));
	return results;
}

bool read_file_win(const char *fname, int sizefile, unsigned char* data)
{
	HANDLE hdl = CreateFile(fname, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 
		                    FILE_ATTRIBUTE_NORMAL, NULL);
	if (hdl == INVALID_HANDLE_VALUE) {
		printf("ERROR: CreateFile failed on %s; error:%d\n", fname, GetLastError());
		return false;
	}

	bool retVal = true;
	DWORD bytesRead;
	if (!ReadFile(hdl, data, sizefile, &bytesRead, NULL)) {
		printf("ERROR: ReadFile failed on %s; error:%d\n", fname, GetLastError());
		retVal = false;
	}
	else if (bytesRead != sizefile) {
		printf("ERROR: ReadFile read %d bytes, but %d were requested\n", bytesRead, sizefile);
        retVal = false;
	}

	CloseHandle(hdl);
	return retVal;
}

#endif