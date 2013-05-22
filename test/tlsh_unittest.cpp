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

static int read_file_eval_tlsh(char *fname, Tlsh *th)
{
	///////////////////////////////////////
	// 1. How big is the file?
	///////////////////////////////////////
	FILE *fd = fopen(fname, "r");
	if(fd==NULL)
		return(1);
	int ret = 1;
	int sizefile = 0;

	fseek(fd, 0L, SEEK_END);
	sizefile = ftell(fd);

	fclose(fd);

	///////////////////////////////////////
	// 2. allocate the memory
	///////////////////////////////////////
	unsigned char* data = (unsigned char*)malloc(sizefile);
	if (data == NULL) {
		printf("out of memory...\n");
		exit(0);
	}

	///////////////////////////////////////
	// 3. read the file
	///////////////////////////////////////
	fd = fopen(fname, "r");
	if (fd==NULL)
		return(1);
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
		printf("cannot open directory %s\n", dirname);
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
					printf("warning too many files max_fnames=%d *n_file=%d\n", max_fnames, *n_file);
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

static void trendLSH_ut(char *compare_fname, char *dirname, char *listname, char *fname, int xref, bool xlen)
{
int max_files;
	if (dirname) {
		if (! is_dir(dirname)) {
			printf("error opening dir: %s\n", dirname);
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
			printf("error: cannot read file %s\n", listname);
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

		qsort(fnames, n_file, sizeof(char *), compar_FileName);

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
			printf("error: cannot read file %s\n", listname);
			exit(1);
		}
		int count = 0;
		x = fgets(buf, 1000, f);
		while (x != NULL) {
			if (x != NULL) {
				int len = strlen(buf);
				char lastc = buf[len-1];
				if ((lastc == '\n') || (lastc == '\r'))
					buf[len-1] = '\0';
				fnames[count].name = strdup(buf);
				count ++;
			}
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
	int n_tptr = 0;

	for (int ti=0; ti<n_file; ti++) {
		int err;
		if (listname) {
			Tlsh *th = new Tlsh();
			err = th->fromTlshStr(fnames[ti].name);
			if (err) {
				printf("cannot read TLSH code %s\n", fnames[ti].name);
				exit(1);
			}
			tptr[n_tptr] = th;
			n_tptr ++;
		} else {
			char *curr_fname = fnames[ti].name;
			Tlsh *th = new Tlsh();
			err = read_file_eval_tlsh(curr_fname, th);
			if (err) {
				printf("error reading file %s\n", curr_fname);
				delete th;
			} else {
				tptr[n_tptr] = th;
				n_tptr ++;
			}
		}
	}

	Tlsh *comp_th = NULL;
	if (compare_fname) {
		comp_th = new Tlsh();
		err = read_file_eval_tlsh(compare_fname, comp_th);
		if (err) {
			printf("failed compare: error reading file %s\n", compare_fname);
			delete comp_th;
			comp_th = NULL;
		}
	}

	if (xref) {
		for (int ti=0; ti<n_tptr; ti++) {
			Tlsh *th = tptr[ti];
			for (int xi=ti+1; xi<n_tptr; xi++) {
				Tlsh *xh = tptr[xi];
				int common = th->totalDiff(xh, xlen);
				printf("%s	%s	%d\n", fnames[ti].name, fnames[xi].name, common);
			}
		}
	} else {
		for (int ti=0; ti<n_tptr; ti++) {
			Tlsh *th = tptr[ti];
			if (comp_th != NULL) {
				int common = comp_th->totalDiff(th, xlen);
				if (dirname || listname) {
					printf("%s	%s	%d\n", compare_fname, fnames[ti].name, common);
				} else {
					printf("%4d	%s\n", common, fnames[ti].name);
				}
			} else {
				printf("%s	%s\n", th->getHash(), fnames[ti].name);
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
		} else if (strcmp(argv[argIdx], "-xref") == 0) {
			xref = 1;
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
	trendLSH_ut(compare_fname, dirname, listname, fname, xref, xlen);
}
