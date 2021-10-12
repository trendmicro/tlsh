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

#define	PATH_OPTION_FULL	1
#define	PATH_OPTION_FNAME	2
#define	PATH_OPTION_DIRNAME	3

////////////////////////////////////////////////////////////////////////////////

static void trendLSH_ut(char *compare_fname, char *dirname, char *outfname, char *listname, int listname_col, int listname_csv, char *fname, char *digestname,
	int xref, bool xlen, int show_details, int threshold, int fc_cons_option, int path_option, int output_json, int output_null, char *splitlines, int showvers)
{
int err;

	struct InputDescr inputd;
	inputd.fnames		= NULL;
	inputd.tptr		= NULL;
	inputd.max_files	= 0;
	inputd.n_file		= 0;
	err = set_input_desc(dirname, listname, listname_col, listname_csv, fname, digestname, show_details, fc_cons_option, splitlines, &inputd, showvers);
	if (err) {
		return;
	}

	Tlsh *comp_th = NULL;
	if (compare_fname) {
		comp_th = new Tlsh();
		err = read_file_eval_tlsh(compare_fname, comp_th, show_details, fc_cons_option, showvers);
		if (err == 0) {
			;
		} else if (err == ERROR_READING_FILE) {
			// compare_fname is not a file.  Assume it is a TLSH digest
			int col_length = strlen(compare_fname);
			// col_length == TLSH_STRING_LEN_REQ	TLSH with version number T1
			// col_length == TLSH_STRING_LEN_REQ-2	original TLSH
			if ((col_length != TLSH_STRING_LEN_REQ) && (col_length != TLSH_STRING_LEN_REQ-2)) {
				fprintf(stderr, "error %s: not a valid file or TLSH digest\n", compare_fname);
				delete comp_th;
				comp_th = NULL;
 			} else if (comp_th->fromTlshStr(compare_fname)) {
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

	FILE *outf = stdout;
	if (outfname != NULL) {
		outf = fopen(outfname, "w");
		if (outf == NULL) {
			fprintf(stderr, "cannot write to file %s\n", outfname);
			exit(1);
		}
	}
	int	output_flag = 0;

	char buf1[2000];
	char buf2[2000];
	char nullstr[6];
	sprintf(nullstr, "TNULL");

	if (xref) {
		if (output_json) {
			fprintf(outf, "{ \"distances\":[\n");
		}
		for (int ti=0; ti<inputd.n_file; ti++) {
			Tlsh *th = inputd.tptr[ti];
			char *ti_fname;
			if (path_option == PATH_OPTION_FULL) {
				ti_fname = inputd.fnames[ti].full_fname;
			} else if (path_option == PATH_OPTION_FNAME) {
				ti_fname = inputd.fnames[ti].only_fname;
			} else if (path_option == PATH_OPTION_DIRNAME) {
				ti_fname = inputd.fnames[ti].dirname;
			}
			if (th != NULL) {
				for (int xi=ti+1; xi<inputd.n_file; xi++) {
					Tlsh *xh = inputd.tptr[xi];
					char *xi_fname;
					if (path_option == PATH_OPTION_FULL) {
						xi_fname = inputd.fnames[xi].full_fname;
					} else if (path_option == PATH_OPTION_FNAME) {
						xi_fname = inputd.fnames[xi].only_fname;
					} else if (path_option == PATH_OPTION_DIRNAME) {
						xi_fname = inputd.fnames[xi].dirname;
					}
					if (xh != NULL) {
						int tdiff = th->totalDiff(xh, xlen);
						if (tdiff <= threshold) {
							if (show_details) {
								fprintf(outf, "%s	[%s]	%s	[%s]	%d\n", ti_fname, th->getHash(showvers), xi_fname, xh->getHash(showvers), tdiff);
							} else {
								const char *con_buf1 = convert_special_chars(ti_fname, buf1, sizeof(buf1), output_json);
								const char *con_buf2 = convert_special_chars(xi_fname, buf2, sizeof(buf2), output_json);
								if (output_json) {
									if (output_flag) fprintf(outf, ",\n");
									fprintf(outf, "{ \"tlsh1\":\"%s\", \"tlsh2\":\"%s\", \"dist\":\"%d\" }", con_buf1, con_buf2, tdiff);
								} else {
									fprintf(outf, "%s	%s	%d\n", con_buf1, con_buf2, tdiff);
								}
							}
							output_flag ++;
						}
					}
				}
			}
		}
	} else {
		if (output_json) {
			if (comp_th != NULL) {
				fprintf(outf, "{ \"distances\":[\n");
			} else {
				fprintf(outf, "{ \"digests\":[\n");
			}
		}
		for (int ti=0; ti<inputd.n_file; ti++) {
			Tlsh *th = inputd.tptr[ti];
			char *ti_fname		= NULL;
			char *ti_dirname	= NULL;
			if (path_option == PATH_OPTION_FULL) {
				ti_fname = inputd.fnames[ti].full_fname;
			} else if (path_option == PATH_OPTION_FNAME) {
				ti_fname = inputd.fnames[ti].only_fname;
			} else if (path_option == PATH_OPTION_DIRNAME) {
				ti_fname	= inputd.fnames[ti].only_fname;
				ti_dirname	= inputd.fnames[ti].dirname;
			}
			if ((th != NULL) || (output_null)) {
				if (comp_th != NULL) {
					if (th != NULL) {
						const char *tlsh_str = th->getHash(showvers);
						int tdiff = comp_th->totalDiff(th, xlen);
						if (tdiff <= threshold) {
							const char *con_buf2 = convert_special_chars(ti_fname,      buf2, sizeof(buf2), output_json);
							if (dirname || listname) {
								const char *con_buf1 = convert_special_chars(compare_fname, buf1, sizeof(buf1), output_json);
								if (output_json) {
									if (output_flag) fprintf(outf, ",\n");
									fprintf(outf, "{ \"tlsh1\":\"%s\", \"tlsh2\":\"%s\", \"dist\":\"%d\" }", con_buf1, con_buf2, tdiff);
								} else {
									fprintf(outf, "%s	%s	%d\n", con_buf1, con_buf2, tdiff);
								}
							} else {
								if (output_json) {
									if (output_flag) fprintf(outf, ",\n");
									fprintf(outf, "{ \"tlsh1\":\"%s\", \"dist\":\"%d\" }", con_buf2, tdiff);
								} else {
									fprintf(outf, "%4d	%s\n", tdiff, con_buf2);
								}
							}
							output_flag ++;
						}
					}
				} else {
					const char *tlsh_str;
					if (th == NULL) {
						tlsh_str = nullstr;
					} else {
						tlsh_str = th->getHash(showvers);
					}
					if (path_option == PATH_OPTION_DIRNAME) {
						const char *con_buf1 = convert_special_chars(ti_dirname, buf1, sizeof(buf1), output_json);
						const char *con_buf2 = convert_special_chars(ti_fname,   buf2, sizeof(buf2), output_json);
						if (output_json) {
							if (output_flag) fprintf(outf, ",\n");
							fprintf(outf, "{ \"tlsh\":\"%s\", \"dir\":\"%s\", \"filename\":\"%s\" }", tlsh_str, con_buf1, con_buf2);
						} else {
							fprintf(outf, "%s	%s	%s\n", tlsh_str, con_buf1, con_buf2);
						}
						output_flag ++;
					} else {
						if (splitlines && (strlen(tlsh_str) == 0)) {
							;
						} else {
							const char *con_buf1 = convert_special_chars(ti_fname, buf1, sizeof(buf1), output_json);
							if (output_json) {
								if (output_flag) fprintf(outf, ",\n");
								fprintf(outf, "{ \"tlsh\":\"%s\", \"file\":\"%s\" }", tlsh_str, con_buf1);
							} else {
								fprintf(outf, "%s	%s\n", tlsh_str, con_buf1);
							}
							output_flag ++;
						}
					}
				}
			}
		}
	}
	if (output_json) fprintf(outf, "]\n}\n");
	if (outfname != NULL) {
		fclose(outf);
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
static void usage(const char *fullPathName, int fullUsage)
{
	const char *pgmName = strrchr(fullPathName, '/');
	if (pgmName != NULL) pgmName++;
	else pgmName = fullPathName;

	printf("usage: tlsh [-c <file|digest>]         -f <file>                     [-T <threshold_value>] OPTIONS\n" );
	printf("     : tlsh  -c <file|digest>          -d <digest>                   [-T <threshold_value>] OPTIONS\n" );
	printf("     : tlsh [-c <file|digest> | -xref] -r <dir>                      [-T <threshold_value>] OPTIONS [-out_fname|-out_dirname]\n" );
	printf("     : tlsh [-c <file|digest> | -xref] -l <listfile> [-l1|-l2|-lcsv] [-T <threshold_value>] OPTIONS\n" );
	printf("     : tlsh -split linenumbers         -f <file>                                            OPTIONS\n" );
	printf("     : OPTIONS is [-xlen] [-conservative] [-old] [-details] [-o outfile] [-ojson] [-onull]\n" );
	printf("     : tlsh -version: prints version of tlsh library\n");
	printf("     : tlsh -notice:  prints NOTICE.txt of tlsh library\n");
	printf("     : tlsh -help: prints full usage information\n");
	if (fullUsage == 0) {
		exit(0);
	}
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
	printf("  -c file|digest:     Specifies a filename or digest whose TLSH value will be compared to a specified TLSH digest (-d) OR the TLSH value of a specified\n");
	printf("                      file (-f) OR the TLSH values of files in a specified directory (-r) OR the TLSH values in a specified listfile (-l)\n");
	printf("  -xref:              Used only when a set of files (-r dir) or TLSH values (-l listfile) is specified.\n");
	printf("                      Results in the calculation of distance between each element in the set.\n");
	printf("  -f file:            Specifies a file whose TLSH values are to be computed, or used for comparison (-c file|digest)\n");
	printf("  -d digest:          Specifies a TLSH digest value that is to be compared to the specified comparison file or digest (-c file|digset)\n");
	printf("  -r dir:             Specifies a recursive directory search for files whose TLSH values are to be computed, or used for comparison (-c file|digset or -xref)\n");
	printf("  -o file:            Specifies the output file\n");
	printf("  -ojson:             output json format instead of TAB separated columns\n");
	printf("  -onull:             output TNULL when we cannot generate a digest (for empty files, files < 50 characters, ...)\n");
	printf("  -out_fname:         Specifies that only the filename is outputted when using the -r option (no path included in output)\n");
	printf("  -out_dirname        Specifies that the dirname and filename are outputted when using the -r option (no path included in output)\n");
	printf("  -l listfile:        Used for comparison purposes only (-c file|digset or -xref).  Each line in listfile can contain either:\n");
	printf("                      - a TLSH digest value (comparison output will display TLSH digests)\n");
	printf("                      - a tab separated TLSH digest value and its corresponding filename (comparison output will display filenames)\n");
	printf("                      The tab separated listfile can be generated by running %s with either the -f or -r flag\n", pgmName);
	printf("  -l1                 (default) listfile contains TLSH value in column 1\n");
	printf("  -l2                           listfile contains TLSH value in column 2\n");
	printf("  -lcsv               listfile is csv (comma separated) file (default is TAB separated file)\n");

	printf("  -xlen:              Passed as the len_diff parameter to Tlsh::totalDiff().  If not specified, len_diff will be true, else false.  Determines if the lengths\n");
	printf("                      of the compared files is to be included in determining the distance.  See tlsh.h for details.\n");
	printf("  -old:               Generate digests without version number. Changed in version 4.0.0 to put 'T1' at the start of TLSH digests\n");
	printf("                      done so that we can change/adapt algorithm and maintain backwards compatibility\n");
	printf("  -force:             Old option. Changed in version 3.17.0 to be default behaviour\n");
	printf("  -conservative:      Original behaviour. Changed in version 3.17.0. Only create a TLSH digest if the input string is >= %d characters.\n", MIN_CONSERVATIVE_DATA_LENGTH);
	printf("  -details:           Results in extra detailed output.\n");
	printf("  -T threshold_value: Used only during comparisons (-c file|digset or -xref).  Specifies the maximum distance that a comparison must\n"); 
	printf("                      generate before it is reported. (defaults to %d)\n", DEFAULT_THRESHOLD);
	printf("  -split linenumbers: linenumbers is a comma separated list of line numbers (example 50,100,200 )\n");
	printf("                      split the file into components and eval the TLSH for each component\n"); 
	printf("                      example. -split 50,100,200 evals 4 TLSH digests. lines 1-49, 50-99, 100-199, 200-end\n");
	printf("                      for the purpose of splitting the file, each line has a max length of 2048 bytes\n"); 
	printf("\n");
	printf("Restrictions:\n");
	printf("  The input string to create a TLSH digest should be >= %d characters\n", MIN_DATA_LENGTH);
	printf("  If the -conservative option is used - then the string must >= %d characters\n", MIN_CONSERVATIVE_DATA_LENGTH);
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

#include "tlsh_impl.h"

void long_version()
{
	printf("%d\tbuckets hash (128 compact, 256 full)\n", EFF_BUCKETS);
#ifdef CHECKSUM_0B
	printf("0\tbyte checksum\n");
#else
	printf("%d\tbyte checksum\n", TLSH_CHECKSUM_LEN);
#endif
	printf("%d\tsliding window\n", SLIDING_WND_SIZE);
}

int main(int argc, char *argv[])
{
	char *digestname		= NULL;
	char *dirname			= NULL;
	char *compare_fname		= NULL;
	char *fname			= NULL;
	char *outfname			= NULL;
	int   output_json		= 0;		// default is TAB separated
	int   output_null		= 0;
	char *listname			= NULL;
	int   listname_col		= 1;		// default is col 1
	int   listname_csv		= 0;		// default is TAB separated

	int xref			= 0;
	bool xlen                       = true;
	int show_details		= 0;
	int threshold			= DEFAULT_THRESHOLD;
	char *splitlines		= NULL;
	int fc_cons_option		= 0;
	int showvers			= 1;
	int path_option			= PATH_OPTION_FULL;

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
		} else if (strcmp(argv[argIdx], "-r") == 0) {
			dirname = argv[argIdx+1];
			argIdx = argIdx+2;
		} else if (strcmp(argv[argIdx], "-f") == 0) {
			fname = argv[argIdx+1];
			argIdx = argIdx+2;
		} else if (strcmp(argv[argIdx], "-d") == 0) {
			digestname = argv[argIdx+1];
			argIdx = argIdx+2;
		} else if (strcmp(argv[argIdx], "-o") == 0) {
			outfname = argv[argIdx+1];
			argIdx = argIdx+2;
		} else if (strcmp(argv[argIdx], "-ojson") == 0) {
			output_json = 1;
			argIdx = argIdx+1;
		} else if (strcmp(argv[argIdx], "-onull") == 0) {
			output_null = 1;
			argIdx = argIdx+1;

		} else if (strcmp(argv[argIdx], "-l") == 0) {
			listname = argv[argIdx+1];
			argIdx = argIdx+2;
		} else if (strcmp(argv[argIdx], "-l1") == 0) {
			listname_col = 1;
			argIdx = argIdx+1;
		} else if (strcmp(argv[argIdx], "-l2") == 0) {
			listname_col = 2;
			argIdx = argIdx+1;
		} else if (strcmp(argv[argIdx], "-lcsv") == 0) {
			listname_csv = 1;
			argIdx = argIdx+1;

		} else if (strcmp(argv[argIdx], "-T") == 0) {
			char *threshold_str = argv[argIdx+1];
			if ((threshold_str[0] >= '0') && (threshold_str[0] <= '9')) {
				threshold = atoi(argv[argIdx+1]);
			} else {
				printf("\nBad threshold '%s' - must be a numeric value\n", argv[argIdx+1]);
				usage(argv[0], 0);
			}
			argIdx = argIdx+2;
		} else if (strcmp(argv[argIdx], "-split") == 0) {
			splitlines = argv[argIdx+1];
			if (! ((splitlines[0] >= '0') && (splitlines[0] <= '9')) ) {
				printf("\nBad split argument '%s' - must be a list of numeric values (comma separated, no spaces)\n", argv[argIdx+1]);
				usage(argv[0], 0);
			}
			argIdx = argIdx+2;
		} else if (strcmp(argv[argIdx], "-xref") == 0) {
			xref = 1;
			argIdx = argIdx+1;
		} else if (strcmp(argv[argIdx], "-old") == 0) {
			showvers = 0;
			argIdx = argIdx+1;
		} else if (strcmp(argv[argIdx], "-force") == 0) {
			argIdx = argIdx+1;
		} else if (strcmp(argv[argIdx], "-conservative") == 0) {
			fc_cons_option += TLSH_OPTION_CONSERVATIVE;
			argIdx = argIdx+1;
		} else if (strcmp(argv[argIdx], "-thread") == 0) {
			fc_cons_option += TLSH_OPTION_THREADED;
			argIdx = argIdx+1;
		} else if (strcmp(argv[argIdx], "-private") == 0) {
			fc_cons_option += TLSH_OPTION_PRIVATE;
			argIdx = argIdx+1;
		} else if (strcmp(argv[argIdx], "-details") == 0) {
			show_details ++;
			argIdx = argIdx+1;
                } else if (strcmp(argv[argIdx], "-xlen") == 0) {
                        xlen = false;
                        argIdx = argIdx+1;
                } else if (strcmp(argv[argIdx], "-out_fname") == 0) {
			path_option	= PATH_OPTION_FNAME;
                        argIdx = argIdx+1;
                } else if (strcmp(argv[argIdx], "-out_dirname") == 0) {
			path_option	= PATH_OPTION_DIRNAME;
                        argIdx = argIdx+1;
                } else if (strcmp(argv[argIdx], "-notice") == 0) {
		        Tlsh::display_notice();
			return 0;
                } else if (strcmp(argv[argIdx], "-version") == 0) {
		        printf("%s\n", Tlsh::version());
			return 0;
                } else if (strcmp(argv[argIdx], "-longversion") == 0) {
		        long_version();
			return 0;
                } else if (strcmp(argv[argIdx], "-help") == 0) {
			usage(argv[0], 1);
#ifdef TLSH_DISTANCE_PARAMETERS
		} else if (strcmp(argv[argIdx], "-SPlen") == 0) {
			char *len_mult_str = argv[argIdx+1];
			if ((len_mult_str[0] >= '0') && (len_mult_str[0] <= '9')) {
				length_mult_value	= atoi(argv[argIdx+1]);
			} else {
				printf("\nBad SPlen '%s' - must be a numeric value\n", argv[argIdx+1]);
				usage(argv[0], 0);
			}
			argIdx = argIdx+2;
		} else if (strcmp(argv[argIdx], "-SPqrat") == 0) {
			char *qratio_mult_str = argv[argIdx+1];
			if ((qratio_mult_str[0] >= '0') && (qratio_mult_str[0] <= '9')) {
				qratio_mult_value	= atoi(argv[argIdx+1]);
			} else {
				printf("\nBad SPqrat '%s' - must be a numeric value\n", argv[argIdx+1]);
				usage(argv[0], 0);
			}
			argIdx = argIdx+2;
		} else if (strcmp(argv[argIdx], "-SPdiff1") == 0) {
			char *hist_diff1_add_str = argv[argIdx+1];
			if ((hist_diff1_add_str[0] >= '0') && (hist_diff1_add_str[0] <= '9')) {
				hist_diff1_add_value	= atoi(argv[argIdx+1]);
			} else {
				printf("\nBad SPdiff1 '%s' - must be a numeric value\n", argv[argIdx+1]);
				usage(argv[0], 0);
			}
			argIdx = argIdx+2;
		} else if (strcmp(argv[argIdx], "-SPdiff2") == 0) {
			char *hist_diff2_add_str = argv[argIdx+1];
			if ((hist_diff2_add_str[0] >= '0') && (hist_diff2_add_str[0] <= '9')) {
				hist_diff2_add_value	= atoi(argv[argIdx+1]);
			} else {
				printf("\nBad SPdiff2 '%s' - must be a numeric value\n", argv[argIdx+1]);
				usage(argv[0], 0);
			}
			argIdx = argIdx+2;
		} else if (strcmp(argv[argIdx], "-SPdiff3") == 0) {
			char *hist_diff3_add_str = argv[argIdx+1];
			if ((hist_diff3_add_str[0] >= '0') && (hist_diff3_add_str[0] <= '9')) {
				hist_diff3_add_value	= atoi(argv[argIdx+1]);
			} else {
				printf("\nBad SPdiff3 '%s' - must be a numeric value\n", argv[argIdx+1]);
				usage(argv[0], 0);
			}
			argIdx = argIdx+2;
#endif
		} else {
			printf("\nunknown option '%s'\n\n", argv[argIdx]);
			usage(argv[0], 0);
		}
	}

	// can only have one of fname / listname or dirname set
	int count = 0;
	if (fname) {
		if (xref) {
			printf("\n-xref option does not work with -f option\n\n");
			usage(argv[0], 0);
		}
		count ++;
	}
	if (digestname) {
		if (!compare_fname) {
			printf("\nA file or digest comparison (-c option) must be specified with the digest (-d) option\n\n");
			usage(argv[0], 0);
		}
		count ++;
	}
	if (listname)
		count ++;
	if (dirname)
		count ++;
	if (count != 1) {
		if (count > 0) printf("\nSpecify EITHER option -f OR -d OR -r OR -l\n\n"); 
		usage(argv[0], 0);
	}
	if (compare_fname && xref) {
		printf("\nSpecify either the -c or -xref option, but not both.\n\n");
		usage(argv[0], 0);
	}

#ifdef TLSH_DISTANCE_PARAMETERS
	set_tlsh_distance_parameters(length_mult_value, qratio_mult_value, hist_diff1_add_value, hist_diff2_add_value, hist_diff3_add_value);
#endif
	trendLSH_ut(compare_fname, dirname, outfname, listname, listname_col, listname_csv, fname, digestname, xref,
		xlen, show_details, threshold, fc_cons_option, path_option, output_json, output_null, splitlines, showvers);
}
