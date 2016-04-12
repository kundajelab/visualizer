#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <limits.h>
#include <sys/stat.h>

//#define DIR_ROOT "/srv/www/kundaje/leepc12"
//#define URL_ROOT "http://mitra.stanford.edu/kundaje/leepc12"
//#define WWWT "/srv/www/kundaje/leepc12/epigenomeviz_cache"

char* DIR_ROOT;

// find recursively *.bigwig, *.tbi

/*
int is_regular_file(const char *path)
{
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISREG(path_stat.st_mode);
}
*/

int ends_with(const char *str, const char *suffix)
{
    if (!str || !suffix)
        return 0;
    size_t lenstr = strlen(str);
    size_t lensuffix = strlen(suffix);
    if (lensuffix >  lenstr)
        return 0;
    return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
}

char *str_replace(const char *orig, const char *rep, const char *with) {
    char *result; // the return string
    char *ins;    // the next insert point
    char *tmp;    // varies
    int len_rep;  // length of rep
    int len_with; // length of with
    int len_front; // distance between rep and end of last rep
    int count;    // number of replacements

    if (!orig)
        return NULL;
    if (!rep)
        rep = "";
    len_rep = strlen(rep);
    if (!with)
        with = "";
    len_with = strlen(with);

    ins = orig;
    for (count = 0; tmp = strstr(ins, rep); ++count) {
        ins = tmp + len_rep;
    }

    // first time through the loop, all the variable are set correctly
    // from here on,
    //    tmp points to the end of the result string
    //    ins points to the next occurrence of rep in orig
    //    orig points to the remainder of orig after "end of rep"
    tmp = result = malloc(strlen(orig) + (len_with - len_rep) * count + 1);

    if (!result)
        return NULL;

    while (count--) {
        ins = strstr(orig, rep);
        len_front = ins - orig;
        tmp = strncpy(tmp, orig, len_front) + len_front;
        tmp = strcpy(tmp, with) + len_with;
        orig += len_front + len_rep; // move to next "end of rep"
    }
    strcpy(tmp, orig);
    return result;
}


void list_dir (const char * dir_name )
{
    DIR * d;
    d = opendir (dir_name);

    if (! d) {
        fprintf (stderr, "Cannot open directory '%s': %s\n",
                 dir_name, strerror (errno));
	return;
        //exit (EXIT_FAILURE);
    }
    while (1) {
        struct dirent * entry;
        const char * d_name;

        /* "Readdir" gets subsequent entries from "d". */
        entry = readdir (d);
        if (! entry) {
            /* There are no more entries in this directory, so break
               out of the while loop. */
            break;
        }
        d_name = entry->d_name;

	char filepath[4096];
	sprintf(filepath,"%s/%s",dir_name,d_name);

	if ( !(entry->d_type & DT_DIR) && strcmp (d_name, "..") != 0 && strcmp (d_name, ".") != 0) {
        
                if ( ends_with( d_name, ".tbi" ) || 
			ends_with( d_name, ".bigwig" ) || 
			ends_with( d_name, ".bw" ) ||
			ends_with( d_name, ".bai" ) ) {

			char filename[300];
			sprintf(filename, "%s/%s", dir_name, d_name);
                        //printf ("%s/%s\n", dir_name, d_name);

			struct stat st;
			stat(filename, &st);
			if ( st.st_size > 0 ) printf( "%s\n", filename );
		}
                        
//                         printf ("%s/%s\n", str_replace(dir_name,DIR_ROOT,URL_ROOT), d_name);
        }

        if (entry->d_type & DT_DIR || entry->d_type & DT_LNK) {

            if (strcmp (d_name, "..") != 0 &&
                strcmp (d_name, ".") != 0) {
                int path_length;
                char path[PATH_MAX];
 
                path_length = snprintf (path, PATH_MAX,
                                        "%s/%s", dir_name, d_name);
                if (path_length >= PATH_MAX) {
                    fprintf (stderr, "Path length has got too long.\n");
                    exit (EXIT_FAILURE);
                }
                list_dir (path);
            }
	}
    }
    /* After going through all the entries, close the directory. */
    if (closedir (d)) {
        fprintf (stderr, "Could not close '%s': %s\n",
                 dir_name, strerror (errno));
        exit (EXIT_FAILURE);
    }
}


int main()
{
	printf("Content-Type:application/text\n\n");

	char *line=malloc(1);
        size_t s=1;

        getline(&line,&s,stdin);
        DIR_ROOT=strdup(line);

	list_dir(DIR_ROOT);
	//list_dir("/srv/www/kundaje/leepc12");
	return 1;
}
