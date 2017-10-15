/*
 * TLSH is provided for use under two licenses: Apache OR BSD.
 * Users may opt to use either license depending on the license
 * restictions of the systems with which they plan to integrate
 * the TLSH code.
 */ 

/* ==============
 * Apache License
 * ==============
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

/* ===========
 * BSD License
 * ===========
 * Copyright (c) 2013, Trend Micro Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.

 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

//////////////////////////////////////////////////////////////////////////
//
// (C) Trend Micro
// written Jon Oliver 2017
//
//////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#ifdef WINDOWS
#include <WinFunctions.h>
#else
// for directory processing on Unix / Linux
#include <dirent.h>
#endif

#include <errno.h>

#include "tlsh.h"

////////////////////////////////////////////////////////////////////////////////

#define	PATH_OPTION_FULL	1
#define	PATH_OPTION_FNAME	2
#define	PATH_OPTION_DIRNAME	3

#define	ERROR_READING_FILE	1
#define	WARNING_FILE_TOO_SMALL	2
#define	WARNING_CANNOT_HASH	3

#define CONVERT_TAB "_<tlsh_convert_tab>_"
#define CONVERT_NEWLINE "_<tlsh_convert_newline>_"
#define CONVERT_LINEFEED "_<tlsh_convert_linefeed>_"

typedef enum {
	TAB,
	NEWLINE,
	LINEFEED
} SpecialChar;

SpecialChar getSpecialChar(const char *tab, const char *newline, const char *linefeed)
{
    // To call this function, there has to be at least 1 special character
	assert (tab != NULL || newline != NULL || linefeed != NULL);  

	if (tab == NULL) return (newline == NULL) ? LINEFEED : (linefeed == NULL) ? NEWLINE : (newline < linefeed) ? NEWLINE : LINEFEED;
	if (newline == NULL) return (tab == NULL) ? LINEFEED : (linefeed == NULL) ? TAB : (tab < linefeed) ? TAB : LINEFEED;
	if (linefeed == NULL) return (tab == NULL) ? NEWLINE : (newline == NULL) ? TAB : (tab < newline) ? TAB : NEWLINE;

	assert (false);  // We should never get here
	return TAB;      // To remove compiler warning about reaching end of non-void function
}

static const char *convert_special_chars(char *filename, char *buf, size_t bufSize)
{
const char *curTab		= CONVERT_TAB;
const char *replaceTab		= "\t";
const char *curNewline		= CONVERT_NEWLINE;
const char *replaceNewline	= "\n";
const char *curLinefeed		= CONVERT_LINEFEED;
const char *replaceLinefeed	= "\r";

size_t fname_offset = 0;
size_t buf_offset = 0;

	while (true) {
		char *tab = strstr(filename+fname_offset, curTab);
		char *newline = strstr(filename+fname_offset, curNewline);
		char *linefeed = strstr(filename+fname_offset, curLinefeed);

		// when no more special characters to replace, copy the remaining part of filename and then return.
		if (tab == NULL && newline == NULL && linefeed == NULL) {
			snprintf(buf+buf_offset, bufSize-buf_offset, "%s", filename+fname_offset);
			return buf;
		}

		SpecialChar sp = getSpecialChar(tab, newline, linefeed);
		if (sp == TAB) {
			char save = *tab;
			*tab = '\0';  // terminate for snprintf
			buf_offset += snprintf(buf+buf_offset, bufSize-buf_offset, "%s%s", filename+fname_offset, replaceTab);
			fname_offset = tab - filename + strlen(curTab);
			*tab = save;  // replace tab
		}
		else if (sp == NEWLINE) {
			char save = *newline;
			*newline = '\0';  // terminate for snprintf
			buf_offset += snprintf(buf+buf_offset, bufSize-buf_offset, "%s%s", filename+fname_offset, replaceNewline);
			fname_offset = newline - filename + strlen(curNewline);
			*newline = save;  // replace newline
		}
		else {
			assert (sp == LINEFEED);
			char save = *linefeed;
			*linefeed = '\0';  // terminate for snprintf
			buf_offset += snprintf(buf+buf_offset, bufSize-buf_offset, "%s%s", filename+fname_offset, replaceLinefeed);
			fname_offset = linefeed - filename + strlen(curLinefeed);
			*linefeed = save;  // replace linefeed
		}
	}
}

static int read_file_eval_tlsh(char *fname, Tlsh *th, int show_details, int force_option)
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

	if (force_option == 0) {
		if (sizefile < MIN_DATA_LENGTH)
			return(WARNING_FILE_TOO_SMALL);
	} else {
		if (sizefile < MIN_FORCE_DATA_LENGTH)
			return(WARNING_FILE_TOO_SMALL);
	}

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
#ifdef WINDOWS
	// Handle differently for Windows because the fread function in msvcr80.dll has a bug
	// and it does not always read the entire file.
	if (!read_file_win(fname, sizefile, data)) {
		free(data);
		return(ERROR_READING_FILE);
	}
#else
	fd = fopen(fname, "r");
	if (fd==NULL) {
		free(data);
		return(ERROR_READING_FILE);
	}

	ret = fread(data, sizeof(unsigned char), sizefile, fd);
	fclose(fd);

	if (ret != sizefile) {
		fprintf(stderr, "fread %d bytes from %s failed: only %d bytes read\n", sizefile, fname, ret);
		return(ERROR_READING_FILE);
	}
#endif

	///////////////////////////////////////
	// 4. calculate the digest
	///////////////////////////////////////
	th->final(data, sizefile, force_option);

	///////////////////////////////////////
	// 5. clean up and return
	///////////////////////////////////////
	free(data);
	if (th->getHash() == NULL || th->getHash()[0] == '\0') {
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

//
// if	full_fname == "user/jon/abcdef.txt"
// then	only_fname ==          "abcdef.txt"
// 	dirname    ==          "jon"
//
struct FileName {
        char *tlsh;  // Only used with -l parameter
        char *full_fname;
        char *only_fname;
        char *dirname;
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
		int len = snprintf(tmp_fname,	sizeof(tmp_fname)-1, "%s/%s", dirname, dit->d_name);
		if (len < sizeof(tmp_fname) - 2) {
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

static int count_lines_in_file(char *fname)
{
char buf[1000];
	FILE *f = fopen(fname, "r");
	if (f == NULL) {
		fprintf(stderr, "error: cannot read file %s\n", fname);
		return(-1);
	}
	int count = 0;
	char *x = fgets(buf, sizeof(buf), f);
	while (x != NULL) {
		count ++;
		x = fgets(buf, sizeof(buf), f);
	}
	fclose(f);
	return(count);
}

static int recursive_read_files_from_dir(char *dirname, char *thisdirname, struct FileName *fnames, int max_fnames, int *n_file)
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
		int len = snprintf(tmp_fname,	sizeof(tmp_fname)-1, "%s/%s", dirname, dit->d_name);
		// -2 for safety
		if (len < sizeof(tmp_fname) - 2) {
			if (is_dir(tmp_fname) ) {
				if ((strcmp(dit->d_name, ".") == 0) || (strcmp(dit->d_name, "..") == 0)) {
					;
				} else {
					int err = recursive_read_files_from_dir(tmp_fname, dit->d_name, fnames, max_fnames, n_file);
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
				fnames[*n_file].full_fname = strdup(tmp_fname);
				fnames[*n_file].only_fname = strdup(dit->d_name);
				fnames[*n_file].dirname    = strdup(thisdirname);
// printf("this_dirname=%s\n", this_dirname);
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
	int err = recursive_read_files_from_dir(dirname, dirname, fnames, max_fnames, n_file);
	return(err);
}

static int compar_FileName(const void *x1, const void *x2)
{
struct FileName *r1;
struct FileName *r2;
        r1 = (struct FileName *) x1;
        r2 = (struct FileName *) x2;
	// printf("compare %s %s\n", r1->name, r2->name);
        return (strcmp(r1->full_fname, r2->full_fname));
}

static void freeFileName(struct FileName *fnames, int count)
{
    for (int i=0; i<count; i++) {
		if (fnames[i].tlsh != NULL) {
			free(fnames[i].tlsh);
		}
		if (fnames[i].full_fname != NULL) {
			free(fnames[i].full_fname);
		}
		if (fnames[i].only_fname != NULL) {
			free(fnames[i].only_fname);
		}
		if (fnames[i].dirname != NULL) {
			free(fnames[i].dirname);
		}
	}
	free(fnames);
}

class pattern_tlsh {
public:
	pattern_tlsh();
	~pattern_tlsh();
	int read_pattern_file(char *pattern_fname);
	int match_pattern(Tlsh *tlsh, bool xlen);
	
	Tlsh	**tlsh_array;
	char	**pattern_name;
	int	*tlsh_radius;
	int	npattern;

};

pattern_tlsh::pattern_tlsh()
{
	tlsh_array	= NULL;
	pattern_name	= NULL;
	tlsh_radius	= NULL;
	npattern	= 0;
}

pattern_tlsh::~pattern_tlsh()
{
	for (int ti=0; ti<npattern; ti++) {
		Tlsh *tp = tlsh_array[ti];
		if (tp != NULL) {
			delete tp;
		}
		if (pattern_name[ti] != NULL) {
			free(pattern_name[ti]);
		}
	}
	if (tlsh_array != NULL)
		free(tlsh_array);
	if (pattern_name != NULL)
		free(pattern_name);
	if (tlsh_radius != NULL)
		free(tlsh_radius);
}

int pattern_tlsh::match_pattern(Tlsh *tlsh, bool xlen)
{
	int nmatch = 0;
	for (int ti=0; ti<npattern; ti++) {
		Tlsh *tp = tlsh_array[ti];
		if (tp != NULL) {
			int dist = tp->totalDiff(tlsh, xlen);
			if (dist <= tlsh_radius[ti]) {
				printf("match	%s	%s	%d	%s\n", tlsh->getHash(), tp->getHash(), dist, pattern_name[ti]);
				nmatch ++;
			}
		}
	}
	return(nmatch);
}


int read_line(char *buf, char **col1, char **col2, char **col3, char **col4, char **col5)
{
	char *curr = &buf[0];
	*col1 = curr;

	*col2 = strchr(curr, '\t');
	if (*col2 == NULL)
		return(1);
	**col2 = '\0';
// printf(":*col1=%s\n", *col1);
	(*col2) ++;
	curr = *col2;

	*col3 = strchr(curr, '\t');
	if (*col3 == NULL)
		return(1);
	**col3 = '\0';
// printf(":*col2=%s\n", *col2);
	(*col3) ++;
	curr = *col3;

	*col4 = strchr(curr, '\t');
	if (*col4 == NULL)
		return(1);
	**col4 = '\0';
// printf(":*col3=%s\n", *col3);
	(*col4) ++;
	curr = *col4;

	*col5 = strchr(curr, '\t');
	if (*col5 == NULL)
		return(1);
	**col5 = '\0';
// printf(":*col4=%s\n", *col4);
	(*col5) ++;
// printf(":*col5=%s\n", *col5);

	return(0);
}

int pattern_tlsh::read_pattern_file(char *pattern_fname)
{
char buf[1000];
	npattern = count_lines_in_file(pattern_fname);
	if (npattern <= 0) {
		return(1);
	}

	tlsh_array	= (Tlsh **)	malloc ( sizeof(Tlsh *) * (npattern+1) );
	pattern_name	= (char **)	malloc ( sizeof(char *) * (npattern+1) );
	tlsh_radius	= (int *)	malloc ( sizeof(int)	* (npattern+1) );
	if ((tlsh_array == NULL) ||(tlsh_radius == NULL)) {
		fprintf(stderr, "error: unable to allocate memory for %d lines in pattern file\n", npattern);
		exit(1);
	}
	for (int ti=0; ti<npattern; ti++) {
		tlsh_array[ti]		= NULL;
		pattern_name[ti]	= NULL;
		tlsh_radius[ti]		= NULL;
	}

	FILE *f = fopen(pattern_fname, "r");
	if (f == NULL) {
		fprintf(stderr, "error: cannot read file %s\n", pattern_fname);
		return(1);
	}
	int count = 0;
	char *x;
	x = fgets(buf, sizeof(buf), f); // ignore line 1 - header line
	x = fgets(buf, sizeof(buf), f);
	while (x != NULL) {
		char *col1, *col2, *col3, *col4, *col5;
		int err = read_line(buf, &col1, &col2, &col3, &col4, &col5);
		if (err) {
			printf("error reading line %d of %s\n", count+2, pattern_fname);
		} else if (count >= npattern) {
			printf("memory error in pattern file: line %d of %s\n", count+2, pattern_fname);
		} else {
			Tlsh *th = new Tlsh();
			err = th->fromTlshStr(col3);
			if (err) {
				fprintf(stderr, "warning: line %d of %s: cannot read TLSH code %s\n", count+2, pattern_fname, col3);
				delete th;
			} else {
				tlsh_array[count]	= th;
				pattern_name[count]	= strdup(col5);
				if ((col4[0] >= '0') && (col4[0] <= '9')) {
					tlsh_radius[count] = atoi(col4);
				} else {
					fprintf(stderr, "warning: line %d of %s: cannot read distance %s\n", count+2, pattern_fname, col4);
					tlsh_radius[count] = -1;
				}
			}
			// printf("line %d	%s	%d\n", count+2, col3, tlsh_radius[count]);
		}
		count ++;
		x = fgets(buf, sizeof(buf), f);
	}
	fclose(f);
	return(0);
}

static void tlsh_pattern(char *dirname, char *listname, char *fname, char *digestname, bool xlen, int force_option, char *pattern_fname)
{
int show_details = 0;
int max_files;
	pattern_tlsh pat;
	int err = pat.read_pattern_file(pattern_fname);
	if (err) {
		fprintf(stderr, "error reading pattern file: %s\n", pattern_fname);
		exit(1);
	}

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
	if (fname || digestname) {
		max_files = 1;
	}
	if (max_files == 0)
		return;

	struct FileName *fnames;
	fnames = (struct FileName *) calloc ( max_files+1, sizeof(struct FileName));
	if (fnames == NULL) {
		fprintf(stderr, "error: unable to allocate memory for %d files\n", max_files);
		exit(1);
	}
	
	int n_file = 0;
	if (dirname) {
		err = read_files_from_dir(dirname, fnames, max_files+1, &n_file);
		if (err) {
			freeFileName(fnames, max_files+1);
			return;
		}

		qsort(fnames, n_file, sizeof(struct FileName), compar_FileName);

		// printf("after sort\n");
		// for (int fi=0; fi<n_file; fi++) {
		// 	printf("file = %s\n", fnames[fi].full_fname );
		// }
	}
	if (listname) {
		FILE *f;
		char *x;
		char buf[1000];
		f = fopen(listname, "r");
		if (f == NULL) {
			fprintf(stderr, "error: cannot read file %s\n", listname);
			freeFileName(fnames, max_files+1);
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
			const char *convert_buf = convert_special_chars(x, buf, sizeof(buf));
			fnames[count].full_fname = strdup(convert_buf);
			fnames[count].only_fname = strdup(convert_buf);
			fnames[count].dirname    = strdup(convert_buf);

			count ++;

			x = fgets(buf, 1000, f);
		}
		n_file = count;
		fclose(f);
	}
	if (fname) {
		fnames[0].full_fname = strdup(fname);
		fnames[0].only_fname = strdup(fname);
		fnames[0].dirname    = strdup(fname);
		n_file = 1;
	}
	if (digestname) {
		fnames[0].full_fname = strdup(digestname);  // set for error display
		fnames[0].only_fname = strdup(digestname);  // set for error display
		fnames[0].dirname    = strdup(digestname);  // set for error display
		fnames[0].tlsh = strdup(digestname);
		n_file = 1;
	}

	Tlsh **tptr;
	tptr = (Tlsh **) malloc ( sizeof(Tlsh *) * (max_files+1) );
	if (n_file > max_files) {
		fprintf(stderr, "error: too many files n_file=%d max_files=%d\n", n_file, max_files);
		free(tptr);
		freeFileName(fnames, max_files+1);
		return;
	}

	for (int ti=0; ti<n_file; ti++) {
		int err;
		tptr[ti] = NULL;
		if (listname || digestname) {
			Tlsh *th = new Tlsh();
			err = th->fromTlshStr(fnames[ti].tlsh);
			if (err) {
				fprintf(stderr, "warning: cannot read TLSH code %s\n", fnames[ti].full_fname);
				delete th;
			} else {
				tptr[ti] = th;
			}
		} else {
			char *curr_fname = fnames[ti].full_fname;
			Tlsh *th = new Tlsh();
			err = read_file_eval_tlsh(curr_fname, th, show_details, force_option);
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

	for (int ti=0; ti<n_file; ti++) {
		Tlsh *tlsh = tptr[ti];
		if (tlsh != NULL) {
			// printf("compare TLSH %s\n", tlsh->getHash());
			int nmatch = pat.match_pattern(tlsh, xlen);
			if (nmatch == 0) {
				printf("nomatch	%s\n", tlsh->getHash());
			}
		}
	}

    // free allocated memory
	for (int ti=0; ti<n_file; ti++) {
		if (tptr[ti] != NULL) {
			delete tptr[ti];
		}
	}
	free(tptr);
	freeFileName(fnames, max_files+1);
}

////////////////////////////////////////////////////////////////////////////////

#define DEFAULT_THRESHOLD 9999
static void usage()
{
	printf("usage: tlsh_pattern -f <file>      -pat <pattern_file> [-xlen] [-force]\n" );
	printf("     : tlsh_pattern -d <digest>    -pat <pattern_file> [-xlen] [-force]\n" );
	printf("     : tlsh_pattern -r <dir>       -pat <pattern_file> [-xlen] [-force]\n" );
	printf("     : tlsh_pattern -l <listfile>  -pat <pattern_file> [-xlen] [-force]\n" );
	printf("     : tlsh_pattern -version: prints version of tlsh library\n");
	printf("\n");
	printf("tlsh can be used to compute TLSH digest values or the distance between digest values in the following ways:\n");
	printf("  1) To compute the TLSH digest value of a single file (-f file), or a directory of files (-r dir).\n");
	printf("     This output can be used to create the listfile required by the -l option described below.\n");
	printf("  2) To compute the distance between a comparison file or TLSH digest (-c file|digest) and the\n");
	printf("     specified file (-f file), TLSH digest (-d digest), directory of files (-r dir), or list (-l listfile).\n");
	printf("  3) To compute the distance between each element in a set of files (-r dir) or files/digests in a\n");
	printf("     list (-l listfile) with every other element in that set, using the –xref flag\n");
	printf("\n");
	printf("parameters: \n");
	printf("  -f file:            Specifies a file whose TLSH values are to be compared to the pattern file\n");
	printf("  -d digest:          Specifies a TLSH digest value that is to be compared to the pattern file\n");
	printf("  -r dir:             Specifies a recursive directory search for files whose TLSH values are to be compared to the pattern file\n");
	printf("  -xlen:              Determines if the lengths of the compared files is to be included in determining the distance\n");
	printf("                      of the compared files is to be included in determining the distance.  See tlsh.h for details.\n");
	printf("  -force:             Force a digest to be created even when the input string is as short as %d characters.\n", MIN_FORCE_DATA_LENGTH);
	printf("\n");
	printf("Restrictions:\n");
	printf("  The input string to create a TLSH digest should be >= %d characters\n", MIN_DATA_LENGTH);
	printf("  If the -force option is used - then the string can be as short as %d characters\n", MIN_FORCE_DATA_LENGTH);
	printf("\n");
	exit(0);
}

int main(int argc, char *argv[])
{
	char *digestname		= NULL;
	char *dirname			= NULL;
	char *listname			= NULL;
	char *fname			= NULL;
	char *pattern_fname		= NULL;

	bool xlen                       = true;
	int force_option		= 0;

	int argIdx		= 1;
	while (argc > argIdx) {
		if (strcmp(argv[argIdx], "-l") == 0) {
			listname = argv[argIdx+1];
			argIdx = argIdx+2;
		} else if (strcmp(argv[argIdx], "-r") == 0) {
			dirname = argv[argIdx+1];
			argIdx = argIdx+2;
		} else if (strcmp(argv[argIdx], "-f") == 0) {
			fname = argv[argIdx+1];
			argIdx = argIdx+2;
		} else if (strcmp(argv[argIdx], "-d") == 0) {
			digestname = argv[argIdx+1];
			argIdx = argIdx+2;
		} else if (strcmp(argv[argIdx], "-pat") == 0) {
			pattern_fname = argv[argIdx+1];
			argIdx = argIdx+2;
		} else if (strcmp(argv[argIdx], "-force") == 0) {
			force_option = 1;
			argIdx = argIdx+1;
                } else if (strcmp(argv[argIdx], "-xlen") == 0) {
                        xlen = false;
                        argIdx = argIdx+1;
                } else if (strcmp(argv[argIdx], "-version") == 0) {
		        printf("%s\n", Tlsh::version());
			return 0;
		} else {
			printf("\nunknown option '%s'\n\n", argv[argIdx]);
			usage();
		}
	}

	// can only have one of fname / listname or dirname set
	int count = 0;
	if (fname)
		count ++;
	if (digestname)
		count ++;
	if (listname)
		count ++;
	if (dirname)
		count ++;
	if (count != 1) {
		if (count > 0) printf("\nSpecify EITHER option -f OR -d OR -r OR -l\n\n"); 
		usage();
	}
	if (pattern_fname == NULL) {
		printf("\nmust specify a pattern file using -pat <pattern_file>\n\n"); 
		usage();
	}

	tlsh_pattern(dirname, listname, fname, digestname, xlen, force_option, pattern_fname);
}
