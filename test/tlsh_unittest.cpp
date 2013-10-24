/*
 * Copyright 2013 Trend Micro Incorporated
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//////////////////////////////////////////////////////////////////////////
//
// (C) Trend Micro
// written Jon Oliver 2009 and 2010

//////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// for directory processing on Unix / Linux
#include <dirent.h>
#include <errno.h>

#include "tlsh.h"

////////////////////////////////////////////////////////////////////////////////

#define	ERROR_READING_FILE	1
#define	WARNING_FILE_TOO_SMALL	2
#define	WARNING_CANNOT_HASH	3

static int read_file_eval_tlsh(char *fname, Tlsh *th, int show_details)
{
	///////////////////////////////////////
	// 1. How big is the file?
	///////////////////////////////////////
	FILE *fd = fopen(fname, "r");
	if(fd==NULL)
		return(ERROR_READING_FILE);
	int ret = 1;
	int sizefile = 0;

	fseek(fd, 0L, SEEK_END);
	sizefile = ftell(fd);

	fclose(fd);

	if (sizefile < 128)
		return(WARNING_FILE_TOO_SMALL);

	///////////////////////////////////////
	// 2. allocate the memory
	///////////////////////////////////////
	unsigned char* data = (unsigned char*)malloc(sizefile);
	if (data == NULL) {
		fprintf(stderr, "out of memory...\n");
		exit(0);
	}

	///////////////////////////////////////
	// 3. read the file
	///////////////////////////////////////
	fd = fopen(fname, "r");
	if (fd==NULL)
		return(ERROR_READING_FILE);
	int offset = 0;
	ret = 0;
	while(offset<sizefile)
	{
		ret = fread(data+offset, sizeof(unsigned char), sizefile, fd);
		offset += ret;
	}
	fclose(fd);

	///////////////////////////////////////
	// 4. calculate the th
	///////////////////////////////////////
	th->final(data, sizefile);

	///////////////////////////////////////
	// 5. clean up and return
	///////////////////////////////////////
	free(data);
	if (th->getHash() == NULL) {
		return(WARNING_CANNOT_HASH);
	}
	if (show_details >= 1) {
		printf("eval	%s	%s\n", fname, th->getHash() );
	}
	return(0);
}

bool is_dir(char *dirname)
{
DIR	  *dip;
	if (dirname == NULL) {
		return(false);
	}
	dip = opendir(dirname);
	if (dip == NULL) {
		return(false);
	}
	closedir(dip);
	return(true);
}

struct FileName {
        char *tlsh;  // Only used with -l parameter
        char *name;
};

static int count_files_in_dir(char *dirname)
{
DIR     *dip;
struct dirent   *dit;

	dip = opendir(dirname);
	if (dip == NULL) {
		return(0);
	}
	dit = readdir(dip);
	int n_file = 0;
	while (dit != NULL) {
		char tmp_fname[2000];
		strncpy(tmp_fname,	dirname,	sizeof(tmp_fname));
		strncat(tmp_fname,	"/",		sizeof(tmp_fname));
		strncat(tmp_fname,	dit->d_name,	sizeof(tmp_fname));
		if (strlen(tmp_fname) < sizeof(tmp_fname) - 2) {
			if (is_dir(tmp_fname) ) {
				if ((strcmp(dit->d_name, ".") == 0) || (strcmp(dit->d_name, "..") == 0)) {
					;
				} else {
					int file_in_dir = count_files_in_dir(tmp_fname);
					n_file = n_file + file_in_dir;
				}
			} else {
				n_file ++;
			}
		}
		dit = readdir(dip);
	}
	closedir(dip);
	return(n_file);
}

static int recursive_read_files_from_dir(char *dirname, struct FileName *fnames, int max_fnames, int *n_file)
{
DIR     *dip;
struct dirent   *dit;

	dip = opendir(dirname);
	if (dip == NULL) {
		fprintf(stderr, "cannot open directory %s\n", dirname);
		return(1);
	}
	dit = readdir(dip);
	while (dit != NULL) {
		char tmp_fname[2000];
		strncpy(tmp_fname,	dirname,	sizeof(tmp_fname));
		strncat(tmp_fname,	"/",		sizeof(tmp_fname));
		strncat(tmp_fname,	dit->d_name,	sizeof(tmp_fname));
		// -2 for safety
		if (strlen(tmp_fname) < sizeof(tmp_fname) - 2) {
			if (is_dir(tmp_fname) ) {
				if ((strcmp(dit->d_name, ".") == 0) || (strcmp(dit->d_name, "..") == 0)) {
					;
				} else {
					int err = recursive_read_files_from_dir(tmp_fname, fnames, max_fnames, n_file);
					if (err) {
						closedir(dip);
						return(1);
					}
				}
			} else {
				if (*n_file >= max_fnames) {
					fprintf(stderr, "warning too many files max_fnames=%d *n_file=%d\n", max_fnames, *n_file);
					closedir(dip);
					return(1);
				}
				fnames[*n_file].name = strdup(tmp_fname);
				*n_file = *n_file + 1;
			}
		}
		dit = readdir(dip);
	}
	closedir(dip);
	return(0);
}

static int read_files_from_dir(char *dirname, struct FileName *fnames, int max_fnames, int *n_file)
{
	*n_file = 0;
	int err = recursive_read_files_from_dir(dirname, fnames, max_fnames, n_file);
	return(err);
}

static int compar_FileName(const void *x1, const void *x2)
{
struct FileName *r1;
struct FileName *r2;
        r1 = (struct FileName *) x1;
        r2 = (struct FileName *) x2;
	// printf("compare %s %s\n", r1->name, r2->name);
        return (strcmp(r1->name, r2->name));
}

static void trendLSH_ut(char *compare_fname, char *dirname, char *listname, char *fname, int xref, bool xlen, int show_details, int threshold)
{
int max_files;
	if (dirname) {
		if (! is_dir(dirname)) {
			fprintf(stderr, "error opening dir: %s\n", dirname);
			exit(1);
		}
		max_files = count_files_in_dir(dirname);
	}
	if (listname) {
		FILE *f;
		char *x;
		char buf[1000];
		f = fopen(listname, "r");
		if (f == NULL) {
			fprintf(stderr, "error: cannot read file %s\n", listname);
			exit(1);
		}
		max_files = 0;
		x = fgets(buf, 1000, f);
		while (x != NULL) {
			max_files ++;
			x = fgets(buf, 1000, f);
		}
		fclose(f);
	}
	if (fname) {
		max_files = 1;
	}
	if (max_files == 0)
		return;

	struct FileName *fnames;
	fnames = (struct FileName *) malloc ( sizeof(struct FileName) * (max_files+1) );
	int err;
	int n_file = 0;
	if (dirname) {
		err = read_files_from_dir(dirname, fnames, max_files+1, &n_file);
		if (err)
			return;

		qsort(fnames, n_file, sizeof(struct FileName), compar_FileName);

		// printf("after sort\n");
		// for (int fi=0; fi<n_file; fi++) {
		// 	printf("file = %s\n", fnames[fi].name );
		// }
	}
	if (listname) {
		FILE *f;
		char *x;
		char buf[1000];
		f = fopen(listname, "r");
		if (f == NULL) {
			fprintf(stderr, "error: cannot read file %s\n", listname);
			exit(1);
		}
		int count = 0;
		x = fgets(buf, 1000, f);
		while (x != NULL) {
		    // Make sure that buf is null terminated
			int len = strlen(buf);
			char lastc = buf[len-1];
			if ((lastc == '\n') || (lastc == '\r'))
				buf[len-1] = '\0';

		    // If buf contains tab character, then assume listname contains tlsh, filename pair 
			// (i.e. is output of runnint tlsh_unittest -r), so advance x to filename
			x = strchr(buf, '\t');
			if (x == NULL) {
			    x = buf; // No tab character, so set x to buf
			}
			else {
			    buf[x-buf] = '\0';  // separate tlsh from filename for strdup below
			    x++;     // advance past tab character to filename
		    }

			fnames[count].tlsh = strdup(buf);
			fnames[count].name = strdup(x);

			count ++;

			x = fgets(buf, 1000, f);
		}
		n_file = count;
		fclose(f);
	}
	if (fname) {
		fnames[0].name = strdup(fname);
		n_file = 1;
	}

	Tlsh **tptr;
	tptr = (Tlsh **) malloc ( sizeof(Tlsh *) * (max_files+1) );
	if (n_file > max_files) {
		fprintf(stderr, "error: too many files n_file=%d max_files=%d\n", n_file, max_files);
		return;
	}

	for (int ti=0; ti<n_file; ti++) {
		int err;
		tptr[ti] = NULL;
		if (listname) {
			Tlsh *th = new Tlsh();
			err = th->fromTlshStr(fnames[ti].tlsh);
			if (err) {
				fprintf(stderr, "warning: cannot read TLSH code %s\n", fnames[ti].name);
				tptr[ti] = NULL;
			} else {
				tptr[ti] = th;
			}
		} else {
			char *curr_fname = fnames[ti].name;
			Tlsh *th = new Tlsh();
			err = read_file_eval_tlsh(curr_fname, th, show_details);
			if (err == 0) {
				tptr[ti] = th;
			} else if (err == ERROR_READING_FILE) {
				fprintf(stderr, "error file %s: cannot read file\n", curr_fname);
				delete th;
			} else if (err == WARNING_FILE_TOO_SMALL) {
				fprintf(stderr, "file %s: file too small\n", curr_fname);
				delete th;
			} else if (err == WARNING_CANNOT_HASH) {
				fprintf(stderr, "file %s: cannot hash\n", curr_fname);
				delete th;
			} else {
				fprintf(stderr, "file %s: unknown error\n", curr_fname);
				delete th;
			}
		}
	}

	Tlsh *comp_th = NULL;
	if (compare_fname) {
		comp_th = new Tlsh();
		err = read_file_eval_tlsh(compare_fname, comp_th, show_details);
		if (err == 0) {
			;
		} else {
			if (err == ERROR_READING_FILE) {
				fprintf(stderr, "error file %s: cannot read file\n", compare_fname);
			} else if (err == WARNING_FILE_TOO_SMALL) {
				fprintf(stderr, "file %s: file too small\n", compare_fname);
			} else if (err == WARNING_CANNOT_HASH) {
				fprintf(stderr, "file %s: cannot hash\n", compare_fname);
			} else {
				fprintf(stderr, "file %s: unknown error\n", compare_fname);
			}
			delete comp_th;
			comp_th = NULL;
		}
	}

	if (xref) {
		for (int ti=0; ti<n_file; ti++) {
			Tlsh *th = tptr[ti];
			if (th != NULL) {
				for (int xi=ti+1; xi<n_file; xi++) {
					Tlsh *xh = tptr[xi];
					if (xh != NULL) {
						int tdiff = th->totalDiff(xh, xlen);
						if (tdiff <= threshold) {
							if (show_details)
								printf("%s	[%s]	%s	[%s]	%d\n", fnames[ti].name, th->getHash(), fnames[xi].name, xh->getHash(), tdiff);
							else
								printf("%s	%s	%d\n", fnames[ti].name, fnames[xi].name, tdiff);
						}
					}
				}
			}
		}
	} else {
		for (int ti=0; ti<n_file; ti++) {
			Tlsh *th = tptr[ti];
			if (th != NULL) {
				if (comp_th != NULL) {
					int tdiff = comp_th->totalDiff(th, xlen);
					if (tdiff <= threshold) {
						if (dirname || listname) {
							printf("%s	%s	%d\n", compare_fname, fnames[ti].name, tdiff);
						} else {
							printf("%4d	%s\n", tdiff, fnames[ti].name);
						}
					}
				} else {
					printf("%s	%s\n", th->getHash(), fnames[ti].name);
				}
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////

static void usage()
{
	printf("usage: trendLSH_ut [-c file] -r dir\n");
	printf("OR\n");
	printf("usage: trendLSH_ut [-c file] -f file\n");
	printf("OR\n");
	printf("usage: trendLSH_ut [-c file] -l listfile\n");
	printf("defaults to using -tlsh\n");
	exit(0);
}

int main(int argc, char *argv[])
{
	char *dirname			= NULL;
	char *listname			= NULL;
	char *compare_fname		= NULL;
	char *fname			= NULL;
	int xref			= 0;
        bool xlen                       = true;
	int show_details		= 0;
	int threshold			= 9999;

	int argIdx		= 1;
	while (argc > argIdx) {
		if (strcmp(argv[argIdx], "-c") == 0) {
			compare_fname = argv[argIdx+1];
			argIdx = argIdx+2;
		} else if (strcmp(argv[argIdx], "-l") == 0) {
			listname = argv[argIdx+1];
			argIdx = argIdx+2;
		} else if (strcmp(argv[argIdx], "-r") == 0) {
			dirname = argv[argIdx+1];
			argIdx = argIdx+2;
		} else if (strcmp(argv[argIdx], "-f") == 0) {
			fname = argv[argIdx+1];
			argIdx = argIdx+2;
		} else if (strcmp(argv[argIdx], "-T") == 0) {
			char *threshold_str = argv[argIdx+1];
			if ((threshold_str[0] >= '0') && (threshold_str[0] <= '9')) {
				threshold = atoi(argv[argIdx+1]);
			} else {
				printf("bad threshold '%s'\n", argv[argIdx+1]);
				usage();
			}
			argIdx = argIdx+2;
		} else if (strcmp(argv[argIdx], "-xref") == 0) {
			xref = 1;
			argIdx = argIdx+1;
		} else if (strcmp(argv[argIdx], "-details") == 0) {
			show_details ++;
			argIdx = argIdx+1;
                } else if (strcmp(argv[argIdx], "-xlen") == 0) {
                        xlen = false;
                        argIdx = argIdx+1;
		} else {
			printf("unknown option '%s'\n", argv[argIdx]);
			usage();
		}
	}

	// can only have one of fname / listname or dirname set
	int count = 0;
	if (fname)
		count ++;
	if (listname)
		count ++;
	if (dirname)
		count ++;
	if (count != 1) {
		usage();
	}
	trendLSH_ut(compare_fname, dirname, listname, fname, xref, xlen, show_details, threshold);
}
