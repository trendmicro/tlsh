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
#include "input_desc.h"
#include "shared_file_functions.h"

////////////////////////////////////////////////////////////////////////////////

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
	int best_ti = -1;
	int best_dist = -1;
	for (int ti=0; ti<npattern; ti++) {
		Tlsh *tp = tlsh_array[ti];
		if (tp != NULL) {
			int dist = tp->totalDiff(tlsh, xlen);
			if (dist <= tlsh_radius[ti]) {
				if ((best_dist == -1) || (dist < best_dist)) {
					best_dist = dist;
					best_ti	  = ti;
				}
				nmatch ++;
			}
		}
	}
	if (best_ti != -1) {
		Tlsh *tp = tlsh_array[best_ti];
		printf("match	pat_%d	%s	%s	%d	%s\n", best_ti, tlsh->getHash(), tp->getHash(), best_dist, pattern_name[best_ti]);
	}
	return(nmatch);
}

void chomp(char *s)
{
int len;
unsigned char x;
	if (s == NULL)
		return;
	len = strlen(s);
	if ((len >= 2) && (s[len-2] == '\r') && (s[len-1] == '\n')) {
		s[len-2] = '\0';
		return;
	} else {
		x = s[len-1];
		if ((x == '\n') || (x == '\r'))
			s[len-1] = '\0';
	}
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
		chomp(buf);
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

	pattern_tlsh pat;
	int err = pat.read_pattern_file(pattern_fname);
	if (err) {
		fprintf(stderr, "error reading pattern file: %s\n", pattern_fname);
		exit(1);
	}

	struct InputDescr inputd;
	inputd.fnames		= NULL;
	inputd.tptr		= NULL;
	inputd.max_files	= 0;
	inputd.n_file		= 0;
	char *splitlines	= NULL;
	int listname_col	= 1;
	int listname_csv	= 0;
	err = set_input_desc(dirname, listname, listname_col, listname_csv, fname, digestname, show_details, force_option, splitlines, &inputd);
	if (err) {
		return;
	}

	for (int ti=0; ti<inputd.n_file; ti++) {
		Tlsh *tlsh = inputd.tptr[ti];
		// char *ti_fname  = inputd.fnames[ti].only_fname;
		if (tlsh != NULL) {
			// printf("compare TLSH %s\n", tlsh->getHash());
			int nmatch = pat.match_pattern(tlsh, xlen);
			if (nmatch == 0) {
				printf("nomatch	%s\n", tlsh->getHash());
			}
		}
	}

    // free allocated memory
	for (int ti=0; ti<inputd.n_file; ti++) {
		if (inputd.tptr[ti] != NULL) {
			delete inputd.tptr[ti];
		}
	}
	free(inputd.tptr);
	freeFileName(inputd.fnames, inputd.max_files+1);
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
