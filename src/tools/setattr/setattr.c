/*
 * 	Diamond (Release 1.0)
 *      A system for interactive brute-force search
 *
 *      Copyright (c) 2002-2005, Intel Corporation
 *      All Rights Reserved
 *
 *  This software is distributed under the terms of the Eclipse Public
 *  License, Version 1.0 which can be found in the file named LICENSE.
 *  ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS SOFTWARE CONSTITUTES
 *  RECIPIENT'S ACCEPTANCE OF THIS AGREEMENT
 */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include "obj_attr.h"


static char const cvsid[] = "$Header$";

int
isstring(char *str, int len)
{
	int i;
	for (i = 0; i<(len -1); i++) {
		if (!(isalnum((int)str[i]))) {
			return(0);
		}
	}

	if (str[len-1] != '\0') {
		return(0);
	}
	return(1);

}

int
print_attr(attr_record_t *arec)
{
	char *data;

	if (arec->flags & ATTR_FLAG_FREE) {
		printf("%-20s ", " ");
		printf(" F ");
		printf("%8d \n", arec->rec_len);
		return(0);
	} else {
		printf("%-20s ", &arec->data[0]);
		printf("   ");
		printf("%8d ", arec->data_len);
	}



	data = &arec->data[arec->name_len];

	if (isstring(data, arec->data_len)) {
		printf("%s \n", data);
	} else {
		if (arec->data_len == 4) {
			int	foo = *(int *)data;
			printf("%16d \n", foo);

		} else {
			int i;

			/* XXX this messes up the endian stuff for little */
			printf("0X");
			for (i=0; i < arec->data_len; i++) {
				printf("%02x", (unsigned char)data[i]);
			}
			printf("\n");
		}
	}
	return(0);
}


int
add_attr(char *attr_name, char *aname, char *data, int datalen)
{
	int		err;
	obj_attr_t 	attr;

	err = obj_read_attr_file(attr_name, &attr);
	if (err != 0) {
		printf("XXX failed to init attr \n");
		exit(1);
	}


	err = obj_write_attr(&attr, aname, datalen, data);
	if (err) {
		printf("failed to write attributes \n");
		exit(1);
	}


	err = obj_write_attr_file(attr_name, &attr);
	if (err != 0) {
		printf("XXX failed to write attributes \n");

	}
	return (0);
}

void
usage()
{
	printf("setattr attribute [-v value][-s string] file1 <file2 ...>\n");

}

int
main(int argc , char **argv)
{
	int			i = 1;
	char			attr_name[NAME_MAX];
	char *			cur_file;
	char *			poss_ext;
	int			flen;
	int			extlen;
	int			is_attr = 0;
	char *			aname;
	char *			data;
	int			datalen;
	int			value;

	if (argc < 3) {
		usage();
		exit(1);
	}

	if (argv[2][0]!= '-') {
		usage();
		exit(1);
	}

	aname = argv[1];

	switch  (argv[2][1]) {
		case 's':
			data = argv[3];
			datalen = strlen(data) + 1;
			break;

		case 'v':
			value = strtol(argv[3], NULL, 0);
			datalen = sizeof(value);
			data = (char *)&value;
			break;

		default:
			usage();
			exit(1);
			break;
	}

	i = 4;
	while (argc != i) {
		cur_file = argv[i];
		i++;

		/*
		 * if the name ends in ".attr" then we were passed
		 * the attribute file and we need to determine the real
		 * file name.
		 */
		extlen = strlen(ATTR_EXT);
		flen = strlen(cur_file);

		/* XXX check maxlen !! */
		if (flen > extlen) {
			poss_ext = &cur_file[flen - extlen];
			if (strcmp(poss_ext, ATTR_EXT) == 0) {
				is_attr = 1;
			}
		}

		if (is_attr) {
			strcpy(attr_name, cur_file);

		} else {
			sprintf(attr_name, "%s%s", cur_file, ATTR_EXT);
		}

		add_attr(attr_name, aname, data, datalen);
	}

	return (0);
}

