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
#include <assert.h>
#include <dirent.h>

#include "tlsh.h"
#include "input_desc.h"
#include "shared_file_functions.h"

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

const char *original_convert_special_chars(char *filename, char *buf, size_t bufSize)
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

const char *convert_special_chars(char *filename, char *buf, size_t bufSize, int output_json)
{
	int len = strlen(filename);
	if (output_json) {
		int bi = 0;
		for (int xi=0; xi<len; xi++) {
			if (bi == bufSize-2) {
				buf[bi] = '\0';
				return(buf);
			}
			unsigned char x = (unsigned char) filename[xi];
			switch (x) {
				case '"':	buf[bi]='\\'; bi++; buf[bi] = '\"'; break;
				case '\\':	buf[bi]='\\'; bi++; buf[bi] = '\\'; break;
				case '\b':	buf[bi]='\\'; bi++; buf[bi] = 'b'; break;
				case '\f':	buf[bi]='\\'; bi++; buf[bi] = 'f'; break;
				case '\n':	buf[bi]='\\'; bi++; buf[bi] = 'n'; break;
				case '\r':	buf[bi]='\\'; bi++; buf[bi] = 'r'; break;
				case '\t':	buf[bi]='\\'; bi++; buf[bi] = 't'; break;
#if defined WINDOWS || defined MINGW
				case '/':	buf[bi]='\\'; bi++; buf[bi] = '\\'; break;
#endif
				default:
						buf[bi] = x; break;
			}
			bi ++;
		}
		if (bi < bufSize) {
			buf[bi] = '\0';
		} else {
			buf[bufSize-1] = '\0';
		}
		return(buf);
	} else {
		strncpy(buf, filename, bufSize);
		if (len >= bufSize) {
			buf[bufSize-1] = '\0';
		}
		return(buf);
	}
}

////////////////////////////////////////////////////////////////////////////////

int read_file_eval_tlsh(char *fname, Tlsh *th, int show_details, int fc_cons_option, int showvers)
{
	///////////////////////////////////////
	// 1. How big is the file?
	///////////////////////////////////////
	FILE *fd = fopen(fname, "rb");
	if(fd==NULL)
		return(ERROR_READING_FILE);
	int ret = 1;
	int sizefile = 0;

	fseek(fd, 0L, SEEK_END);
	sizefile = ftell(fd);

	fclose(fd);

	if (fc_cons_option <= 1) {
		if (sizefile < MIN_DATA_LENGTH)
			return(WARNING_FILE_TOO_SMALL);
	} else {
		if (sizefile < MIN_CONSERVATIVE_DATA_LENGTH)
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
	fd = fopen(fname, "rb");
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
	th->final(data, sizefile, fc_cons_option);

	///////////////////////////////////////
	// 5. clean up and return
	///////////////////////////////////////
	free(data);
	if (th->getHash(showvers) == NULL || th->getHash(showvers)[0] == '\0') {
		return(WARNING_CANNOT_HASH);
	}
	if (show_details >= 1) {
		printf("eval	%s	%s\n", fname, th->getHash(showvers) );
	}
	return(0);
}

bool is_dir(char *dirname)
{
DIR	  *dip;
	if (dirname == NULL) {
		return(false);
	}
#ifndef WINDOWS
	dip = opendir(dirname);
#else
	WIN32_FIND_DATA data;
	HANDLE h = FindFirstFile(dirname, &data);
	if (h != nullptr)
	{
		dip = new DIR();
		dip->hFind = h;
	}
#endif
	if (dip == NULL) {
		return(false);
	}
#ifndef WINDOWS
	closedir(dip);
#else
	FindClose(dip->hFind);
	delete dip;
#endif
	return(true);
}

int count_files_in_dir(char *dirname)
{
DIR     *dip;
struct dirent   *dit;

#ifndef WINDOWS
	dip = opendir(dirname);
#else
	WIN32_FIND_DATA data;
	HANDLE h = FindFirstFile(dirname, &data);
	if (h != nullptr)
	{
		dip = new DIR();
		dip->hFind = h;
	}
#endif
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
#ifndef WINDOWS
	closedir(dip);
#else
	FindClose(dip->hFind);
	delete dip;
#endif
	return(n_file);
}

static int recursive_read_files_from_dir(char *dirname, char *thisdirname, struct FileName *fnames, int max_fnames, int *n_file)
{
DIR     *dip;
struct dirent   *dit;

#ifndef WINDOWS
	dip = opendir(dirname);
#else
	WIN32_FIND_DATA data;
	HANDLE h = FindFirstFile(dirname, &data);
	if (h != nullptr)
	{
		dip = new DIR();
		dip->hFind = h;
	}
#endif
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
#ifndef WINDOWS
	closedir(dip);
#else
	FindClose(dip->hFind);
	delete dip;
#endif
	return(0);
}

int read_files_from_dir(char *dirname, struct FileName *fnames, int max_fnames, int *n_file)
{
	*n_file = 0;
	int err = recursive_read_files_from_dir(dirname, dirname, fnames, max_fnames, n_file);
	return(err);
}

void freeFileName(struct FileName *fnames, int count)
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
