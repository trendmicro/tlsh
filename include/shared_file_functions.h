
#define	ERROR_READING_FILE	1
#define	WARNING_FILE_TOO_SMALL	2
#define	WARNING_CANNOT_HASH	3

bool is_dir(char *dirname);
int count_files_in_dir(char *dirname);
int read_file_eval_tlsh(char *fname, Tlsh *th, int show_details, int fc_cons_option, int showvers);
const char *convert_special_chars(char *filename, char *buf, size_t bufSize, int output_json);

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

int read_files_from_dir(char *dirname, struct FileName *fnames, int max_fnames, int *n_file);
void freeFileName(struct FileName *fnames, int count);
