
struct InputDescr {
	struct FileName *fnames;
	Tlsh **tptr;
	int max_files;
	int n_file;
	int *split_line_pos;
};

int set_input_desc(char *dirname, char *listname, int listname_col, int listname_csv,
	char *fname, char *digestname, int show_details, int fc_cons_option, char *splitlines, struct InputDescr *inputd, int showvers);
