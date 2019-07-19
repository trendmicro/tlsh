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
// written Jon Oliver 2009 and 2010
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

#define	ERROR_READING_FILE	1
#define	WARNING_FILE_TOO_SMALL	2
#define	WARNING_CANNOT_HASH	3

static void trendLSH_ut(char *compare_fname, char *dirname, char *listname, char *fname, char *digestname, int xref, bool xlen, int show_details, int threshold, int force_option)
{
int err;

	struct InputDescr inputd;
	inputd.fnames		= NULL;
	inputd.tptr		= NULL;
	inputd.max_files	= 0;
	inputd.n_file		= 0;

	int listname_col	= 1;
	int listname_csv	= 0;
	char *splitlines	= NULL;
	
	err = set_input_desc(dirname, listname, listname_col, listname_csv, fname, digestname, show_details, force_option, splitlines, &inputd);
	if (err) {
		return;
	}

	Tlsh *comp_th = NULL;
	if (compare_fname) {
		comp_th = new Tlsh();
		err = read_file_eval_tlsh(compare_fname, comp_th, show_details, force_option);
		if (err == 0) {
			;
		} else if (err == ERROR_READING_FILE) {
			// compare_fname is not a file.  Assume it is a TLSH digest
 			if (comp_th->fromTlshStr(compare_fname)) {
				fprintf(stderr, "error %s: not a valid file or TLSH digest\n", compare_fname);
				delete comp_th;
				comp_th = NULL;
			}
		} else {
			if (err == WARNING_FILE_TOO_SMALL) {
				fprintf(stderr, "file %s: file too small\n", compare_fname);
			} else if (err == WARNING_CANNOT_HASH) {
				fprintf(stderr, "file %s: cannot hash\n", compare_fname);
			} else {
				fprintf(stderr, "file %s: unknown error\n", compare_fname);
			}
			delete comp_th;
			comp_th = NULL;
		}
		if (comp_th == NULL) {
			freeFileName(inputd.fnames, inputd.max_files+1);
			exit(1);
		}
	}

	char buf1[2000];
	char buf2[2000];
	if (xref) {
		for (int ti=0; ti<inputd.n_file; ti++) {
			Tlsh *th = inputd.tptr[ti];
			if (th != NULL) {
				for (int xi=ti+1; xi<inputd.n_file; xi++) {
					Tlsh *xh = inputd.tptr[xi];
					if (xh != NULL) {
						int tdiff = th->totalDiff(xh, xlen);
						if (tdiff <= threshold) {
							if (show_details)
								printf("%s	[%s]	%s	[%s]	%d\n", inputd.fnames[ti].full_fname, th->getHash(), inputd.fnames[xi].full_fname, xh->getHash(), tdiff);
							else
								printf("%s	%s	%d\n", convert_special_chars(inputd.fnames[ti].full_fname, buf1, sizeof(buf1)),
							   	                       convert_special_chars(inputd.fnames[xi].full_fname, buf2, sizeof(buf2)), tdiff);
						}
					}
				}
			}
		}
	} else {
		for (int ti=0; ti<inputd.n_file; ti++) {
			Tlsh *th = inputd.tptr[ti];
			if (th != NULL) {
				if (comp_th != NULL) {
					int tdiff = comp_th->totalDiff(th, xlen);
					if (tdiff <= threshold) {
						if (dirname || listname) {
							printf("%s	%s	%d\n", convert_special_chars(compare_fname, buf1, sizeof(buf1)),
							                       convert_special_chars(inputd.fnames[ti].full_fname, buf2, sizeof(buf2)), tdiff);
						} else {
							printf("%4d	%s\n", tdiff, convert_special_chars(inputd.fnames[ti].full_fname, buf1, sizeof(buf1)));
						}
					}
				} else {
					printf("%s	%s\n", th->getHash(), convert_special_chars(inputd.fnames[ti].full_fname, buf1, sizeof(buf1)));
				}
			}
		}
	}

    // free allocated memory
	for (int ti=0; ti<inputd.n_file; ti++) {
		if (inputd.tptr[ti] != NULL) {
			delete inputd.tptr[ti];
		}
	}
	if (comp_th != NULL) {
		delete comp_th;
	}
	free(inputd.tptr);
	freeFileName(inputd.fnames, inputd.max_files+1);
}

////////////////////////////////////////////////////////////////////////////////

#define DEFAULT_THRESHOLD 9999
static void usage(const char *fullPathName)
{
	const char *pgmName = strrchr(fullPathName, '/');
	if (pgmName != NULL) pgmName++;
	else pgmName = fullPathName;

	printf("usage: %s [-c <file|digest>]         -f <file>     [-T <threshold_value>] [-xlen] [-force] [-details]\n", pgmName);
	printf("     : %s  -c <file|digest>          -d <digest>   [-T <threshold_value>] [-xlen] [-force] [-details]\n", pgmName);
	printf("     : %s [-c <file|digest> | -xref] -r <dir>      [-T <threshold_value>] [-xlen] [-force] [-details]\n", pgmName);
	printf("     : %s [-c <file|digest> | -xref] -l <listfile> [-T <threshold_value>] [-xlen] [-force] [-details]\n", pgmName);
	printf("     : %s -version: prints version of tlsh library\n", pgmName);
	printf("     : tlsh -notice:  prints NOTICE.txt of tlsh library\n");
	printf("\n");
	printf("%s can be used to compute TLSH digest values or the distance between digest values in the following ways:\n", pgmName);
	printf("  1) To compute the TLSH digest value of a single file (-f file), or a directory of files (-r dir).\n");
	printf("     This output can be used to create the listfile required by the -l option described below.\n");
	printf("  2) To compute the distance between a comparison file or TLSH digest (-c file|digest) and the\n");
	printf("     specified file (-f file), TLSH digest (-d digest), directory of files (-r dir), or list (-l listfile).\n");
	printf("  3) To compute the distance between each element in a set of files (-r dir) or files/digests in a\n");
	printf("     list (-l listfile) with every other element in that set, using the –xref flag\n");
	printf("\n");
	printf("parameters: \n");
	printf("  -c file|digest:     Specifies a filename or digest whose TLSH value will be compared to a specified TLSH digest (-d) OR the TLSH value of a sepcified\n");
	printf("                      file (-f) OR the TLSH values of files in a specified directory (-r) OR the TLSH values in a specified listfile (-l)\n");
	printf("  -xref:              Used only when a set of files (-r dir) or TLSH values (-l listfile) is specified.\n");
	printf("                      Results in the calculation of distance between each element in the set.\n");
	printf("  -f file:            Specifies a file whose TLSH values are to be computed, or used for comparison (-c file|digest)\n");
	printf("  -d digest:          Specifies a TLSH digest value that is to be compared to the specified comparison file or digest (-c file|digset)\n");
	printf("  -r dir:             Specifies a recursive directory search for files whose TLSH values are to be computed, or used for comparison (-c file|digset or -xref)\n");
	printf("  -l listfile:        Used for comparison purposes only (-c file|digset or -xref).  Each line in listfile can contain either:\n");
	printf("                      - a TLSH digest value (comparison output will display TLSH digests)\n");
	printf("                      - a tab separated TLSH digest value and its corresponding filename (comparison output will display filenames)\n");
	printf("                      The tab separated listfile can be generated by running %s with either the -f or -r flag\n", pgmName);
	printf("  -xlen:              Passed as the len_diff parameter to Tlsh::totalDiff().  If not specified, len_diff will be true, else false.  Determines if the lengths\n");
	printf("                      of the compared files is to be included in determining the distance.  See tlsh.h for details.\n");
	printf("  -force:             Force a digest to be created even when the input string is as short as %d characters.\n", MIN_FORCE_DATA_LENGTH);
	printf("  -details:           Results in extra detailed output.\n");
	printf("  -T threshold_value: Used only during comparisons (-c file|digset or -xref).  Specifies the maximum distance that a comparison must\n"); 
	printf("                      generate before it is reported. (defaults to %d)\n", DEFAULT_THRESHOLD);
	printf("\n");
	printf("Restrictions:\n");
	printf("  The input string to create a TLSH digest should be >= %d characters\n", MIN_DATA_LENGTH);
	printf("  If the -force option is used - then the string can be as short as %d characters\n", MIN_FORCE_DATA_LENGTH);
	printf("\n");
	printf("Example usages:\n");
	printf("  To calculate the distance between two files, run the command:\n");
	printf("     %s -c <file 1> -f <file 2>\n", pgmName);
	printf("\n");
	printf("  To calculate the distance between two TLSH digest values, run the command:\n");
	printf("     %s -c <TLSH digest 1> -d <TLSH digest 2>\n", pgmName);
	printf("\n");
	printf("  To calculate the TLSH digest values for every file in a directory - this can create the input for the -l option\n");
	printf("     %s -r <dir>\n", pgmName);
	printf("\n");
	printf("  To get the distance between a reference TLSH digest value, and a list of TLSH digest values in a file:\n");
	printf("     %s -c <TLSH digest> -l <file>\n", pgmName);
	printf("\n");
	printf("  To compare the TLSH value for every file in a directory, to every other file in that directory, run the command:\n");
	printf("     %s -xref -r <dir>\n", pgmName);
	printf("\n");
	exit(0);
}

int main(int argc, char *argv[])
{
	char *digestname		= NULL;
	char *dirname			= NULL;
	char *listname			= NULL;
	char *compare_fname		= NULL;
	char *fname			= NULL;
	int xref			= 0;
	bool xlen                       = true;
	int show_details		= 0;
	int threshold			= DEFAULT_THRESHOLD;
	int force_option		= 0;

#ifdef TLSH_DISTANCE_PARAMETERS
	int length_mult_value		= -1;
	int qratio_mult_value		= -1;
	int hist_diff1_add_value	= -1;
	int hist_diff2_add_value	= -1;
	int hist_diff3_add_value	= -1;
#endif

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
		} else if (strcmp(argv[argIdx], "-d") == 0) {
			digestname = argv[argIdx+1];
			argIdx = argIdx+2;
		} else if (strcmp(argv[argIdx], "-T") == 0) {
			char *threshold_str = argv[argIdx+1];
			if ((threshold_str[0] >= '0') && (threshold_str[0] <= '9')) {
				threshold = atoi(argv[argIdx+1]);
			} else {
				printf("\nBad threshold '%s' - must be a numeric value\n", argv[argIdx+1]);
				usage(argv[0]);
			}
			argIdx = argIdx+2;
		} else if (strcmp(argv[argIdx], "-xref") == 0) {
			xref = 1;
			argIdx = argIdx+1;
		} else if (strcmp(argv[argIdx], "-force") == 0) {
			force_option = 1;
			argIdx = argIdx+1;
		} else if (strcmp(argv[argIdx], "-details") == 0) {
			show_details ++;
			argIdx = argIdx+1;
                } else if (strcmp(argv[argIdx], "-xlen") == 0) {
                        xlen = false;
                        argIdx = argIdx+1;
                } else if (strcmp(argv[argIdx], "-notice") == 0) {
		        Tlsh::display_notice();
			return 0;
                } else if (strcmp(argv[argIdx], "-version") == 0) {
		        printf("%s\n", Tlsh::version());
			return 0;
#ifdef TLSH_DISTANCE_PARAMETERS
		} else if (strcmp(argv[argIdx], "-SPlen") == 0) {
			char *len_mult_str = argv[argIdx+1];
			if ((len_mult_str[0] >= '0') && (len_mult_str[0] <= '9')) {
				length_mult_value	= atoi(argv[argIdx+1]);
			} else {
				printf("\nBad SPlen '%s' - must be a numeric value\n", argv[argIdx+1]);
				usage(argv[0]);
			}
			argIdx = argIdx+2;
		} else if (strcmp(argv[argIdx], "-SPqrat") == 0) {
			char *qratio_mult_str = argv[argIdx+1];
			if ((qratio_mult_str[0] >= '0') && (qratio_mult_str[0] <= '9')) {
				qratio_mult_value	= atoi(argv[argIdx+1]);
			} else {
				printf("\nBad SPqrat '%s' - must be a numeric value\n", argv[argIdx+1]);
				usage(argv[0]);
			}
			argIdx = argIdx+2;
		} else if (strcmp(argv[argIdx], "-SPdiff1") == 0) {
			char *hist_diff1_add_str = argv[argIdx+1];
			if ((hist_diff1_add_str[0] >= '0') && (hist_diff1_add_str[0] <= '9')) {
				hist_diff1_add_value	= atoi(argv[argIdx+1]);
			} else {
				printf("\nBad SPdiff1 '%s' - must be a numeric value\n", argv[argIdx+1]);
				usage(argv[0]);
			}
			argIdx = argIdx+2;
		} else if (strcmp(argv[argIdx], "-SPdiff2") == 0) {
			char *hist_diff2_add_str = argv[argIdx+1];
			if ((hist_diff2_add_str[0] >= '0') && (hist_diff2_add_str[0] <= '9')) {
				hist_diff2_add_value	= atoi(argv[argIdx+1]);
			} else {
				printf("\nBad SPdiff2 '%s' - must be a numeric value\n", argv[argIdx+1]);
				usage(argv[0]);
			}
			argIdx = argIdx+2;
		} else if (strcmp(argv[argIdx], "-SPdiff3") == 0) {
			char *hist_diff3_add_str = argv[argIdx+1];
			if ((hist_diff3_add_str[0] >= '0') && (hist_diff3_add_str[0] <= '9')) {
				hist_diff3_add_value	= atoi(argv[argIdx+1]);
			} else {
				printf("\nBad SPdiff3 '%s' - must be a numeric value\n", argv[argIdx+1]);
				usage(argv[0]);
			}
			argIdx = argIdx+2;
#endif
		} else {
			printf("\nunknown option '%s'\n\n", argv[argIdx]);
			usage(argv[0]);
		}
	}

	// can only have one of fname / listname or dirname set
	int count = 0;
	if (fname) {
		if (xref) {
			printf("\n-xref option does not work with -f option\n\n");
			usage(argv[0]);
		}
		count ++;
	}
	if (digestname) {
		if (!compare_fname) {
			printf("\nA file or digest comparison (-c option) must be specified with the digest (-d) option\n\n");
			usage(argv[0]);
		}
		count ++;
	}
	if (listname)
		count ++;
	if (dirname)
		count ++;
	if (count != 1) {
		if (count > 0) printf("\nSpecify EITHER option -f OR -d OR -r OR -l\n\n"); 
		usage(argv[0]);
	}
	if (compare_fname && xref) {
		printf("\nSpecify either the -c or -xref option, but not both.\n\n");
		usage(argv[0]);
	}

#ifdef TLSH_DISTANCE_PARAMETERS
	set_tlsh_distance_parameters(length_mult_value, qratio_mult_value, hist_diff1_add_value, hist_diff2_add_value, hist_diff3_add_value);
#endif
	trendLSH_ut(compare_fname, dirname, listname, fname, digestname, xref, xlen, show_details, threshold, force_option);
}
