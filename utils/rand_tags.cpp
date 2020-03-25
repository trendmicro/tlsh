
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include <string>
#include <string.h>
#include "tlsh.h"

static void html_contents(std::string &htmls);
static void html_table(std::string &htmls, int *ntags);

struct tagdef {
	int relative_count;
	const char *s;
};

struct tagdef anchor_def[] = {
	{ 165,	"<a href= >" },
	{ 2,	"<ahref= >" },
	{ 323,	"<A href= >" },
	{ 1,	"<Ahref= >" },
	{ 8,	"<A HREF= >" },
	{ 2,	"<a href= code= target= >" },
	{ 6,	"<a href= id= >" },
	{ 8,	"<a href= Id= >" },
	{ 7,	"<A href= id= >" },
	{ 11,	"<A href= Id= >" },
	{ 1,	"<Ahref= Id= >" },
	{ 4,	"<A href= id= id= >" },
	{ 14,	"<a href= id= id= target= >" },
	{ 6,	"<a href= ID= id= target= >" },
	{ 2,	"<A href= id= id= target= >" },
	{ 19,	"<a href= id= target= >" },
	{ 5,	"<A href= Id= target= >" },
	{ 67,	"<a href= target= >" },
	{ 2,	"<ahref= target= >" },
	{ 7,	"<A href= target= >" },
	{ 7,	"<A href= target= rel= >" },
	{ 1,	"<a href= Type= Id= >" },
	{ 6,	"<a href= Type= Id= target= >" },
	{ 2,	"<a target= href= >" },
	{ 2,	"<A title= href= target= >" },
	{ 0, NULL }
};

static int random_tags(struct tagdef *tag_def)
{
int count = 0;
int total = 0;
	while (tag_def[count].relative_count > 0) {
		total = total + tag_def[count].relative_count;
		count ++;
	}
// printf("total=%d count=%d\n", total, count);
	int x = abs((int) random()) % total;
	int idx = 0;
	for (int ti=0; ti<count; ti++) {
// printf("x=%d ti=%d\n", x, ti);
		x = x - tag_def[ti].relative_count;
		if (x <= 0) {
			idx = ti;
			break;
		}
	}
	return(idx);
}

static void endtag(std::string &htmls, char *tag)
{
	htmls += "</";
	int ti = 1;
	while (tag[ti] != '\0') {
		if ((tag[ti] == ' ') || (tag[ti] == '\t') || (tag[ti] == '>')) {
			htmls += '>';
			return;
		}
		htmls += tag[ti];
		ti ++;
	}
	htmls += '>';
}

static void anchor(std::string &htmls)
{
	int anchor_tag = random_tags(anchor_def);
	char *anchor_tag_str = (char *) anchor_def[anchor_tag].s;
	htmls += anchor_tag_str;
	htmls += '\n';
	endtag(htmls, anchor_tag_str);
	htmls += '\n';
}

struct tagdef body_def[] = {
	{ 83,	"<body>" },
	{ 38,	"<BODY>" },
	{ 17,	"<BODY bgcolor= >" },
	{ 46,	"<BODY bgColor= >" },
	{ 24,	"<body class= >" },
	{ 1,	"<BODY lang= style= vLink= link= bgColor= >" },
	{ 1,	"<BODY style= >" },
	{ 5,	"<BODY style= bgColor= background= >" },
	{ 1,	"<BODY style= text= bgColor= background= COLOR= >" },
	{ 1,	"<BODY style= text= vLink= aLink= link= bgColor= background= COLOR= >" },
	{ 0, NULL }
};

struct tagdef meta_def[] = {
	{ 9,	"<meta content= charset= http-equiv= >" },
	{ 1,	"<META content= charset= http-equiv= >" },
	{ 90,	"<META content= name= >" },
	{ 9,	"<meta http-equiv= content= >" },
	{ 22,	"<META http-equiv= content= >" },
	{ 23,	"<meta http-equiv= content= charset= >" },
	{ 88,	"<META http-equiv= content= charset= >" },
	{ 2,	"<META HTTP-EQUIV= CONTENT= charset= >" },
	{ 27,	"<meta name= content= >" },
	{ 2,	"<META NAME= CONTENT= >" },
	{ 0, NULL }
};

struct tagdef head_def[] = {
	{ 64,	"<head>" },
	{ 10,	"<Head>" },
	{ 110,	"<HEAD>" },
	{ 0, NULL }
};

static void head_meta(std::string &htmls)
{
	// <HEAD><META http-equiv= content= charset= ><META content= name= ><STYLE><TEXT></STYLE><style><TEXT></style><!-->--></HEAD>
	int head_tag = random_tags(head_def);
	char *head_tag_str = (char *) head_def[head_tag].s;
	htmls += head_tag_str;
	htmls += '\n';

	// meta #1
	int meta_tag = random_tags(meta_def);
	char *meta_tag_str = (char *) meta_def[meta_tag].s;
	htmls += meta_tag_str;
	htmls += '\n';

	// meta #2
	meta_tag = random_tags(meta_def);
	meta_tag_str = (char *) meta_def[meta_tag].s;
	htmls += meta_tag_str;
	htmls += '\n';

	endtag(htmls, head_tag_str);
	htmls += '\n';
}


struct tagdef html_def[] = {
	{ 111,	"<html>" },
	{ 111,	"<HTML>" },
	{ 1,	"<HTML >" },
	{ 0, NULL }
};

static void html_tags(std::string &htmls, bool verbose)
{
	bool bodyFlag = true;
	if (random() % 6 == 1)
		bodyFlag = false;

	if (random() % 20 == 1) {
		if (random() % 10 == 1)
			htmls += "<!doctype >";
		else
			htmls += "<!DOCTYPE >";
	}

	int html_tag = random_tags(html_def);
	char *html_tag_str = (char *) html_def[html_tag].s;
	htmls += html_tag_str;
	htmls += '\n';

	if (random() % 10 == 1) {
		head_meta(htmls);
	}

	char *body_tag_str;
	if (bodyFlag) {
		int body_tag = random_tags(body_def);
		body_tag_str = (char *) body_def[body_tag].s;
		htmls += body_tag_str;
		htmls += '\n';
	}
	if (verbose)
		printf("BEFORE html_contents:	%s\n", htmls.c_str() );
	html_contents(htmls);
	if (verbose)
		printf("AFTER html_contents:	%s\n", htmls.c_str() );
	if (bodyFlag) {
		endtag(htmls, body_tag_str);
		htmls += '\n';
	}
	endtag(htmls, html_tag_str);
	htmls += '\n';
}

#define	MIN_TLSH_LEN	512

static void html(unsigned int seed, bool show_lsh, char *dir)
{
std::string htmls;
Tlsh n;
bool verbose = false;
	int showvers = 0;
	srandom(seed);
	// if (seed == 4628)
	// verbose = true;
	html_tags(htmls, verbose);
// printf("seed=%d len=%d\n", seed, htmls.length());
	if (htmls.length() <= MIN_TLSH_LEN)
		return;
	n.final((unsigned char *)htmls.c_str(), htmls.length());
	const char *tlsh_str = n.getHash(showvers);
	if (tlsh_str == NULL)
		return;
	if (show_lsh) {
		printf("%s	%d\n", tlsh_str, seed);
	} else {
		if (dir == NULL) {
			printf("=== seed=%d ===\n", seed);
			printf("%s", htmls.c_str() );
		} else {
			char fname[1000];
			snprintf(fname, 1000, "%s/tags.%d", dir, seed);
			FILE *f;
			f = fopen(fname, "w");
			if (f == NULL) {
				printf("error: cannot open to write %s\n", fname);
				exit(1);
			}
			fprintf(f, "%s", htmls.c_str() );
			fclose(f);
		}
	}
}

struct tagdef random_def[] = {
	{ 13,	"<big>" },
	{ 10,	"<big style= >" },
	{ 1,	"<BLOCKQUOTE>" },
	{ 12,	"<blockquote class= style= >" },
	{ 2,	"<BLOCKQUOTE class= style= >" },
	{ 9,	"<blockquote style= >" },
	{ 4,	"<BLOCKQUOTE style= >" },
	{ 1,	"<blockquote type= class= cite= >" },
	{ 1,	"<center>" },
	{ 2,	"<cite>" },
	{ 1,	"<colgroup>" },
	{ 41,	"<div>" },
	{ 1863,	"<DIV>" },	// orig 18632
	{ 23,	"<div align= >" },
	{ 7,	"<DIV align= >" },
	{ 21,	"<div class= >" },
	{ 2,	"<DIV class= >" },
	{ 1,	"<DIV class= lang= dir= align= >" },
	{ 11,	"<DIV dir= align= >" },
	{ 28,	"<DIV dir= style= >" },
	{ 2,	"<div id= >" },
	{ 13,	"<DIV id= >" },
	{ 4,	"<div id= class= >" },
	{ 7,	"<DIV id= dir= >" },
	{ 2,	"<div id= style= >" },
	{ 25,	"<div style= >" },
	{ 11,	"<DIV style= >" },
	{ 7,	"<DL>" },
	{ 35,	"<DT>" },
	{ 4,	"<EM>" },
	{ 122,	"<font color= >" },
	{ 46,	"<FONT color= >" },
	{ 1,	"<FONT COLOR= >" },
	{ 2,	"<font color= face= >" },
	{ 36,	"<FONT color= size= >" },
	{ 40,	"<font color= size= face= >" },
	{ 2,	"<font color= target= >" },
	{ 11,	"<font face= >" },
	{ 1,	"<FONT face= >" },
	{ 12,	"<FONT FACE= >" },
	{ 2,	"<FONT face= color= >" },
	{ 11,	"<FONT face= color= size= >" },
	{ 1,	"<font face= size= >" },
	{ 52,	"<FONT face= size= >" },
	{ 47,	"<font size= >" },
	{ 93,	"<FONT size= >" },
	{ 5,	"<FONT SIZE= >" },
	{ 4,	"<font size= color= >" },
	{ 17,	"<font size= color= face= >" },
	{ 17,	"<FONT size= color= face= >" },
	{ 109,	"<font size= face= >" },
	{ 2,	"<FONT size= face= >" },
	{ 1,	"<FONT SIZE= SIZE= FACE= LANG= >" },
	{ 4,	"<FONT style= >" },
	{ 3,	"<h1>" },
	{ 8,	"<i>" },
	{ 40,	"<I>" },
	{ 205,	"<p>" },
	{ 1796,	"<P>" },
	{ 3,	"<p align= >" },
	{ 14,	"<P align= >" },
	{ 1,	"<P ALIGN= >" },
	{ 19,	"<pre>" },
	{ 1,	"<PRE>" },
	{ 2,	"<pre style= >" },
	{ 2,	"<p style= >" },
	{ 2,	"<P style= >" },
	{ 1,	"<small>" },
	{ 1,	"<span class= >" },
	{ 14,	"<SPAN class= >" },
	{ 10,	"<span dir= >" },
	{ 4,	"<SPAN id= >" },
	{ 4,	"<SPAN name= border= >" },
	{ 40,	"<span style= >" },
	{ 32,	"<SPAN style= >" },
	{ 11,	"<SPAN STYLE= >" },
	{ 1,	"<strong>" },
	{ 5,	"<STRONG>" },
	{ 32,	"<style>" },
	{ 55,	"<STYLE>" },
	{ 8,	"<style type= >" },
	{ 1,	"<STYLE type= >" },
	{ 25,	"<tt>" },
	{ 3,	"<u>" },
	{ 2,	"<U>" },
	{ 4,	"<ul>" },
	{ 0, NULL }
};

struct tagdef oneoff_def[] = {
	{ 2225,	"<br>" },	// originally 22256
	{ 17,	"<br >" },
	{ 494,	"<BR>" },	// originally 4941
	{ 6,	"<br clear= >" },
	{ 10,	"<br style= >" },
	{ 2,	"<hr>" },
	{ 17,	"<hr >" },
	{ 2,	"<HR>" },
	{ 1,	"<HR ALIGN= SIZE= WIDTH= >" },
	{ 1,	"<hr align= width= SIZE= >" },
	{ 2,	"<hr id= >" },
	{ 2,	"<HR id= >" },
	{ 2,	"<hr noshade>" },
	{ 10,	"<hr size= >" },
	{ 3,	"<HR SIZE= >" },
	{ 4,	"<HR style= >" },
	{ 1,	"<HR tabIndex= >" },
	{ 19,	"<IMG alt= hspace= src= align= border= >" },
	{ 1,	"<IMG alt= src= border= >" },
	{ 2,	"<IMG alt= src=\"cid:border= >" },
	{ 2,	"<IMG alt= src= id= border= >" },
	{ 2,	"<IMG alt= src= src=\"cid:border= >" },
	{ 2,	"<IMG src= >" },
	{ 1,	"<img src= alt= height= width= >" },
	{ 2,	"<img src= border= width= height= >" },
	{ 1,	"<IMG src=\"cid:border= >" },
	{ 1,	"<img src=\"cid:width= height= >" },
	{ 2,	"<IMG src= src=\"cid:border= >" },
	{ 8,	"<img width= height= src=\"cid:border= alt= >" },
	{ 1,	"<img width= height= src= src=\"cid:border= alt= >" },
	{ 4186,	"<TEXT>" },	// originally 41862
	{ 0, NULL }
};

static void rhtml_contents(std::string &htmls, int *ntags, int *ndistinct_tags)
{
	// original code
	// ((*ntags <= 0) && ( ndistinct_tags <= 0))
	// bad - should not do comparison on pointer value
	//
	// intention
	// ((*ntags <= 0) && (*ndistinct_tags <= 0))
	//	have the == NULL test to be consistent - pass regression tests
	if ((*ntags <= 0) && (ndistinct_tags == NULL))
		return; 
	if (random() % 10 == 1) {
		anchor(htmls);
		*ntags		= *ntags - 2;
		*ndistinct_tags = *ndistinct_tags - 1;
	} else if (random() % 20 == 1) {
		html_table(htmls, ntags);
		*ndistinct_tags = *ndistinct_tags - 1;
	} else if (random() % 3 == 1) {
		int oneoff_tag = random_tags(oneoff_def);
		char *oneoff_tag_str = (char *) oneoff_def[oneoff_tag].s;
		htmls += oneoff_tag_str;
		htmls += '\n';
		*ntags		= *ntags - 1;
		*ndistinct_tags = *ndistinct_tags - 1;
	} else if (random() % 3 == 1) {
		return;
	} else {
		int rtag = random_tags(random_def);
		char *rtag_str = (char *) random_def[rtag].s;
		htmls += rtag_str;
		htmls += '\n';
		*ntags		= *ntags - 2;
		*ndistinct_tags = *ndistinct_tags - 1;
		rhtml_contents(htmls, ntags, ndistinct_tags);
		endtag(htmls, rtag_str);
		htmls += '\n';
	}
}

static void html_contents(std::string &htmls)
{
	int ntags = random() % 32;
	int loop;
	for (loop=0; loop<8; loop++) {
		if (random() % 2 == 1)
			break;
		ntags = ntags * 2;
	}
	ntags = ntags + 5;
// printf("ntags=%d loop=%d\n", ntags, loop);

	int ndistinct_tags = 3;
	while ((ntags > 0) || (ndistinct_tags > 0))
		rhtml_contents(htmls, &ntags, &ndistinct_tags);
}

struct tagdef table_def[] = {
	{ 7,	"<table>" },
	{ 4,	"<table >" },
	{ 1,	"<table align= border= cellspacing= width= >" },
	{ 1,	"<TABLE bgColor= border= Color= cellPadding= cellSpacing= height= width= >" },
	{ 1,	"<table border= cellspacing= cellpadding= >" },
	{ 2,	"<table cellpadding= cellspacing= border= style= >" },
	{ 11,	"<table cellspacing= cellpadding= border= >" },
	{ 3,	"<TABLE cellSpacing= cellPadding= border= >" },
	{ 7,	"<TABLE cellSpacing= cellPadding= width= >" },
	{ 2,	"<TABLE cellSpacing= width= align= border= >" },
	{ 1,	"<TABLE class= width= >" },
	{ 7,	"<TABLE Color= height= width= bgColor= border= >" },
	{ 7,	"<TABLE Color= height= width= border= >" },
	{ 7,	"<TABLE id= cellSpacing= cellPadding= width= border= >" },
	{ 1,	"<table style= cellspacing= cellpadding= width= border= >" },
	{ 14,	"<table width= >" },
	{ 1,	"<table width= border= >" },
	{ 4,	"<table width= border= cellspacing= cellpadding= >" },
	{ 0, NULL }
};

struct tagdef td_def[] = {
	{ 59,	"<td>" },
	{ 2,	"<TD align= bgColor= height= width= >" },
	{ 8,	"<td bgcolor= >" },
	{ 1,	"<td bgcolor= height= >" },
	{ 2,	"<TD bgColor= height= >" },
	{ 1,	"<td class= style= width= height= >" },
	{ 7,	"<TD Color= width= bgColor= height= >" },
	{ 1,	"<td height= >" },
	{ 2,	"<TD height= >" },
	{ 1,	"<TD id= dir= style= width= >" },
	{ 1,	"<TD id= style= vAlign= width= >" },
	{ 5,	"<TD id= style= width= >" },
	{ 9,	"<TD id= vAlign= align= >" },
	{ 7,	"<TD id= width= >" },
	{ 1,	"<td style= width= >" },
	{ 4,	"<td valign= >" },
	{ 6,	"<TD vAlign= >" },
	{ 1,	"<TD vAlign= align= >" },
	{ 1,	"<TD vAlign= colSpan= >" },
	{ 10,	"<td valign= style= >" },
	{ 25,	"<td width= >" },
	{ 7,	"<TD width= >" },
	{ 7,	"<TD width= bgColor= height= >" },
	{ 1,	"<td width= bgcolor= valign= >" },
	{ 14,	"<TD width= height= >" },
	{ 0, NULL }
};

struct tagdef tr_def[] = {
	{ 20,	"<tr>" },
	{ 58,	"<TR>" },
	{ 3,	"<TR class= >" },
	{ 1,	"<tr style= height= >" },
	{ 47,	"<tr valign= >" },
	{ 0, NULL }
};

#define MAX_COL	10
#define MAX_ROW	10

static void html_table(std::string &htmls, int *ntags)
{
char * col_tag_str[MAX_COL];
char * row_tag_str;
	int table_tag = random_tags(table_def);
	char *table_tag_str = (char *) table_def[table_tag].s;
	htmls += table_tag_str;
	htmls += '\n';
	*ntags = *ntags - 1;

	int nrow = random() % MAX_ROW;
	int ncol = random() % MAX_COL;
	if (random() % 2 == 1)
		nrow = nrow / 2;
	if (random() % 2 == 1)
		nrow = nrow / 2;
	if (random() % 2 == 1)
		ncol = ncol / 2;
	if (random() % 2 == 1)
		ncol = ncol / 2;
	if (nrow <= 0)
		nrow = 1;
	if (ncol <= 0)
		ncol = 1;
// printf("nrow = %d\n", nrow);
// printf("ncol = %d\n", ncol);

	int row_tag = random_tags( tr_def );
	row_tag_str = (char *) tr_def[row_tag].s;
	for (int ci=0; ci<ncol; ci++) {
		int col_tag = random_tags( td_def );
		col_tag_str[ci] = (char *) td_def[col_tag].s;
	}

	for (int ri=0; ri<nrow; ri++) {
		htmls += row_tag_str;
		for (int ci=0; ci<ncol; ci++) {
			htmls += col_tag_str[ci];
			endtag(htmls, col_tag_str[ci]);
		}
		endtag(htmls, row_tag_str);
		htmls += '\n';

		*ntags = *ntags - (ncol + 2);
		if (*ntags <= 0)
			break;
	}

	endtag(htmls, table_tag_str);
	htmls += '\n';
	*ntags = *ntags - 1;
}

// struct tagdef meta_def[] = {
// 	{ 0, NULL }
// };

#ifdef UNUSED



	{ 2,	"<tbody>" },
	{ 35,	"<TBODY>" },

	{ 31,	"</title>" },
	{ 31,	"<title>" },
	{ 43,	"</TITLE>" },
	{ 43,	"<TITLE>" },


#endif

static void usage()
{
	printf("rand_tags [-start start] [-end end] [-tlsh] [-v1|v2]\n");
	printf("	generate random tag structure\n");
	printf("	start = starting seed\n");
	printf("	end   = ending seed\n");
	printf("	-tlsh output tlsh value instead of HTML\n");
	printf("	-v1 output nilsimsa\n");
	printf("	-v2 output TLSH		* default value *\n");
	exit(1);
}

int main(int argc, char *argv[])
{
	int start_seed			= 1;
	int end_seed			= -1;
	bool show_lsh			= false;
	char *dir			= NULL;
	
	int argIdx		= 1;
	if (argc == 1)
		usage();
	while (argc > argIdx) {
		if (strcmp(argv[argIdx], "-start") == 0) {
			if (argIdx+1 <= argc && isdigit((unsigned char)argv[argIdx+1][0])) {
				start_seed = atoi(argv[argIdx+1]);
			}
			argIdx = argIdx+2;
		} else if (strcmp(argv[argIdx], "-end") == 0) {
			if (argIdx+1 <= argc && isdigit((unsigned char)argv[argIdx+1][0])) {
				end_seed = atoi(argv[argIdx+1]);
			}
			argIdx = argIdx+2;
		} else if (strcmp(argv[argIdx], "-d") == 0) {
			dir = argv[argIdx+1];
			argIdx = argIdx+2;
		} else if (strcmp(argv[argIdx], "-tlsh") == 0) {
			show_lsh = true;
			argIdx = argIdx+1;
		} else {
			usage();
		}
	}
	if (end_seed == -1) {
		html(start_seed, show_lsh, dir);
	} else {
		for (int seed=start_seed; seed<=end_seed; seed++) {
			html(seed, show_lsh, dir);
		}
	}
}
