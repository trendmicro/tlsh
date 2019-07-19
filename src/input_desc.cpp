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

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include "tlsh.h"
#include "input_desc.h"
#include "shared_file_functions.h"

static int compar_FileName(const void *x1, const void *x2)
{
struct FileName *r1;
struct FileName *r2;
        r1 = (struct FileName *) x1;
        r2 = (struct FileName *) x2;
	// printf("compare %s %s\n", r1->name, r2->name);
        return (strcmp(r1->full_fname, r2->full_fname));
}

int set_input_desc(char *dirname, char *listname, int listname_col, int listname_csv,
	char *fname, char *digestname, int show_details, int force_option, char *splitlines, struct InputDescr *inputd)
{
	////////////////////////////
	// Step 1. set inputd->max_files
	////////////////////////////
	if (dirname) {
		if (! is_dir(dirname)) {
			fprintf(stderr, "error opening dir: %s\n", dirname);
			return(1);
		}
		inputd->max_files = count_files_in_dir(dirname);
	}
	if (listname) {
		FILE *f;
		char *x;
		char buf[1000];
		f = fopen(listname, "r");
		if (f == NULL) {
			fprintf(stderr, "error: cannot read file %s\n", listname);
			return(1);
		}
		inputd->max_files = 0;
		x = fgets(buf, 1000, f);
		while (x != NULL) {
			inputd->max_files ++;
			x = fgets(buf, 1000, f);
		}
		fclose(f);
	}
	if (fname || digestname) {
		inputd->max_files = 1;
	}
	if (inputd->max_files == 0)
		return(1);

	////////////////////////////
	// Step 2. alloc memory
	////////////////////////////

	inputd->fnames = (struct FileName *) calloc ( inputd->max_files+1, sizeof(struct FileName));
	if (inputd->fnames == NULL) {
		fprintf(stderr, "error: unable to allocate memory for %d files\n", inputd->max_files);
		exit(1);
	}

	////////////////////////////
	// Step 3. put file names in inputd->fnames
	////////////////////////////
	inputd->n_file = 0;
	if (dirname) {
		int err = read_files_from_dir(dirname, inputd->fnames, inputd->max_files+1, &(inputd->n_file));
		if (err) {
			freeFileName(inputd->fnames, inputd->max_files+1);
			return(1);
		}

		qsort(inputd->fnames, inputd->n_file, sizeof(struct FileName), compar_FileName);

		// printf("after sort\n");
		// for (int fi=0; fi<n_file; fi++) {
		// 	printf("file = %s\n", inputd->fnames[fi].full_fname );
		// }
	}
	if (listname) {
		FILE *f;
		char *x;
		char buf[1000];
		f = fopen(listname, "r");
		if (f == NULL) {
			fprintf(stderr, "error: cannot read file %s\n", listname);
			freeFileName(inputd->fnames, inputd->max_files+1);
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
			if (listname_csv) {
				// CSV file - comma seperated
				x = strchr(buf, ',');
			} else {
				// TAB seperated
				x = strchr(buf, '\t');
			}
			if (x == NULL) {
				x = buf; // No tab character, so set x to buf
			} else {
				buf[x-buf] = '\0';  // separate tlsh from filename for strdup below
		 		x++;     // advance past tab character to filename
			}
			char *col_tlsh  = NULL;
			char *col_fname = NULL;
			if (listname_col == 1) {
				col_tlsh	= buf;
				col_fname	= x;
			} else if (listname_col == 2) {
				col_tlsh	= x;
				col_fname	= buf;
			} else {
				fprintf(stderr, "error: bad listname_col=%d\n", listname_col);
				return(1);
			}
			inputd->fnames[count].tlsh = strdup(col_tlsh);
			inputd->fnames[count].full_fname = strdup(col_fname);
			inputd->fnames[count].only_fname = strdup(col_fname);
			inputd->fnames[count].dirname    = strdup(col_fname);

			count ++;

			x = fgets(buf, 1000, f);
		}
		inputd->n_file = count;
		fclose(f);
	}
////////////////////////////
	if (fname) {
		inputd->fnames[0].full_fname = strdup(fname);
		inputd->fnames[0].only_fname = strdup(fname);
		inputd->fnames[0].dirname    = strdup(fname);
		inputd->n_file = 1;
	}
	if (digestname) {
		inputd->fnames[0].full_fname = strdup(digestname);  // set for error display
		inputd->fnames[0].only_fname = strdup(digestname);  // set for error display
		inputd->fnames[0].dirname    = strdup(digestname);  // set for error display
		inputd->fnames[0].tlsh = strdup(digestname);
		inputd->n_file = 1;
	}

	////////////////////////////
	// Step 4. eval TLSH
	////////////////////////////
	inputd->tptr = (Tlsh **) malloc ( sizeof(Tlsh *) * (inputd->max_files+1) );
	if (inputd->n_file > inputd->max_files) {
		fprintf(stderr, "error: too many files n_file=%d max_files=%d\n", inputd->n_file, inputd->max_files);
		free(inputd->tptr);
		freeFileName(inputd->fnames, inputd->max_files+1);
		return(1);
	}

	for (int ti=0; ti<inputd->n_file; ti++) {
		int err;
		inputd->tptr[ti] = NULL;
		if (listname || digestname) {
			Tlsh *th = new Tlsh();
			err = th->fromTlshStr(inputd->fnames[ti].tlsh);
			if (err) {
				fprintf(stderr, "warning: cannot read TLSH code %s\n", inputd->fnames[ti].full_fname);
				delete th;
			} else {
				inputd->tptr[ti] = th;
			}
		} else {
			char *curr_fname = inputd->fnames[ti].full_fname;
			Tlsh *th = new Tlsh();
			err = read_file_eval_tlsh(curr_fname, th, show_details, force_option);
			if (err == 0) {
				inputd->tptr[ti] = th;
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
	return(0);
}
