#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define WWWT "/srv/www/kundaje/epigenomeviz_cache"

int main()
{
	srand(time(0));
	int randnum=rand();
	printf("Content-Type:application/text\n\n");

	char *line=malloc(1);
	size_t s=1;
	
	getline(&line,&s,stdin);
	char *extension=strdup(line);

	char *file;
	asprintf(&file, "%s/%d.json", WWWT, randnum);
	FILE *fout=fopen(file,"w");

	if(fout==NULL) {
		printf("ERROR: %s (%d)", strerror(errno), errno);
		return 1;
	}

	fprintf(fout,"%s",extension);
	fclose(fout);

	printf("%d.json", randnum);
	return 1;
}
