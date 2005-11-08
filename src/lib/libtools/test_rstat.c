/*
 *      Diamond (Release 1.0)
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
#include "rstat.h"


static char const cvsid[] =
    "$Header$";

int
main(int argc, char **argv)
{
	u_int64_t       freq,
	                mem;

	r_cpu_freq(&freq);
	r_freemem(&mem);
	printf("cpu frequency = %llu\n", freq);
	printf("memory = %llu KB\n", mem / 1000);

	return 0;
}
