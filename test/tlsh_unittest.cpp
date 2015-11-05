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

static const char *convert_special_chars(char *filename, char *buf, size_t bufSize, 
                                         const char *curTab, const char *replaceTab,
                                         const char *curNewline, const char *replaceNewline,
                                         const char *curLinefeed, const char *replaceLinefeed)
{
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

	if (sizefile < MIN_DATA_LENGTH)
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
	// 4. calculate the th
	///////////////////////////////////////
	th->final(data, sizefile);

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
		int len = snprintf(tmp_fname,	sizeof(tmp_fname)-1, "%s/%s", dirname, dit->d_name);
		// -2 for safety
		if (len < sizeof(tmp_fname) - 2) {
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

static void freeFileName(struct FileName *fnames, int count)
{
    for (int i=0; i<count; i++) {
		if (fnames[i].tlsh != NULL) {
			free(fnames[i].tlsh);
		}
		if (fnames[i].name != NULL) {
			free(fnames[i].name);
		}
	}
	free(fnames);
}

static void trendLSH_ut(char *compare_fname, char *dirname, char *listname, char *fname, char *digestname, int xref, bool xlen, int show_details, int threshold)
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
	if (fname || digestname) {
		max_files = 1;
	}
	if (max_files == 0)
		return;

	struct FileName *fnames;
	fnames = (struct FileName *) calloc ( max_files+1, sizeof(struct FileName) * (max_files+1) );
	int err;
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
			fnames[count].name = strdup(convert_special_chars(x, buf, sizeof(buf), CONVERT_TAB, "\t", CONVERT_NEWLINE, "\n", CONVERT_LINEFEED, "\r"));

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
	if (digestname) {
		fnames[0].name = strdup(digestname);  // set for error display
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
				fprintf(stderr, "warning: cannot read TLSH code %s\n", fnames[ti].name);
				delete th;
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
			freeFileName(fnames, max_files+1);
			exit(1);
		}
	}

	char buf1[2000];
	char buf2[2000];
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
								printf("%s	%s	%d\n", convert_special_chars(fnames[ti].name, buf1, sizeof(buf1), 
							 	                                             "\t", CONVERT_TAB, "\n", CONVERT_NEWLINE, "\r", CONVERT_LINEFEED), 
							   	                       convert_special_chars(fnames[xi].name, buf2, sizeof(buf2),
								                                             "\t", CONVERT_TAB, "\n", CONVERT_NEWLINE, "\r", CONVERT_LINEFEED), tdiff);
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
							printf("%s	%s	%d\n", convert_special_chars(compare_fname, buf1, sizeof(buf1), 
						 	                                             "\t", CONVERT_TAB, "\n", CONVERT_NEWLINE, "\r", CONVERT_LINEFEED), 
							                       convert_special_chars(fnames[ti].name, buf2, sizeof(buf2),
						 	                                             "\t", CONVERT_TAB, "\n", CONVERT_NEWLINE, "\r", CONVERT_LINEFEED), tdiff);
						} else {
							printf("%4d	%s\n", tdiff, convert_special_chars(fnames[ti].name, buf1, sizeof(buf1),
							                                                "\t", CONVERT_TAB, "\n", CONVERT_NEWLINE, "\r", CONVERT_LINEFEED)); 
						}
					}
				} else {
					printf("%s	%s\n", th->getHash(), convert_special_chars(fnames[ti].name, buf1, sizeof(buf1),
							                                                "\t", CONVERT_TAB, "\n", CONVERT_NEWLINE, "\r", CONVERT_LINEFEED)); 
				}
			}
		}
	}

    // free allocated memory
	for (int ti=0; ti<n_file; ti++) {
		if (tptr[ti] != NULL) {
			delete tptr[ti];
		}
	}
	if (comp_th != NULL) {
		delete comp_th;
	}
	free(tptr);
	freeFileName(fnames, max_files+1);
}

////////////////////////////////////////////////////////////////////////////////

#define DEFAULT_THRESHOLD 9999
static void usage(const char *fullPathName)
{
	const char *pgmName = strrchr(fullPathName, '/');
	if (pgmName != NULL) pgmName++;
	else pgmName = fullPathName;

	printf("usage: %s [-c <file|digest>]         -f <file>     [-T <threshold_value>] [-xlen] [-details]\n", pgmName);
	printf("     : %s  -c <file|digest>          -d <digest>   [-T <threshold_value>] [-xlen] [-details]\n", pgmName);
	printf("     : %s [-c <file|digest> | -xref] -r <dir>      [-T <threshold_value>] [-xlen] [-details]\n", pgmName);
	printf("     : %s [-c <file|digest> | -xref] -l <listfile> [-T <threshold_value>] [-xlen] [-details]\n", pgmName);
	printf("     : %s -version: prints version of tlsh library\n", pgmName);
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
	printf("  -details:           Results in extra detailed output.\n");
	printf("  -T threshold_value: Used only during comparisons (-c file|digset or -xref).  Specifies the maximum distance that a comparison must\n"); 
	printf("                      generate before it is reported. (defaults to %d)\n", DEFAULT_THRESHOLD);
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
		} else if (strcmp(argv[argIdx], "-details") == 0) {
			show_details ++;
			argIdx = argIdx+1;
                } else if (strcmp(argv[argIdx], "-xlen") == 0) {
                        xlen = false;
                        argIdx = argIdx+1;
                } else if (strcmp(argv[argIdx], "-version") == 0) {
		        printf("%s\n", Tlsh::version());
			return 0;
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

	trendLSH_ut(compare_fname, dirname, listname, fname, digestname, xref, xlen, show_details, threshold);
}
