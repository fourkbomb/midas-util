#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static char **split(const char *str, char *delim) {
	char *token, *tofree, *string;
	char **ret;
	int nents = 2, i;
	int len = strlen(str);
	int dlen = strlen(delim);

	for (i = 0; i < len; i++) {
		if (strncmp(&str[i], delim, dlen) == 0)
			nents++;
	}

	ret = malloc(sizeof(*ret) * nents);

	ret[nents-1] = NULL; // sentinel

	tofree = string = strdup(str);
	if (string == NULL)
		return NULL;

	i = 0;
	while ((token = strsep(&string, ",")) != NULL) {
		ret[i] = strdup(token);
		i++;
	}

	return ret;
}


int main(int argc, char **argv) {
	if (argc < 2)
		return 1;

	char **res = split(argv[1], ",");
	int i = 0;
	while (res[i] != NULL)
		printf("%s\n", res[i++]);

	return 0;
}
