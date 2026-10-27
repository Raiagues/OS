/*
 * This file has been generated from klibc :
 * ftp://ftp.kernel.org/pub/linux/libs/klibc/
 *
 * By issuing the command :
 * cat memccpy.c memchr.c memrchr.c memcmp.c memcpy.c memmove.c memset.c strxspn.c memmem.c memswap.c strcat.c strchr.c strrchr.c strcmp.c strcpy.c strcspn.c strlen.c strnlen.c strncat.c strlcat.c strncmp.c strncpy.c strpbrk.c strsep.c strspn.c strstr.c strtok.c
 *
 * in the subdirectory :
 * usr/klibc/
 *
 * and then slightly edited.
 */
/* This example includes only the string routines it uses. */
#include "string.h"

void *memmove(void *dst, const void *src, size_t n)
{
	const char *p = src;
	char *q = dst;
	if (q < p) {
		while (n--) {
			*q++ = *p++;
		}
	} else {
		p += n;
		q += n;
		while (n--) {
			*--q = *--p;
		}
	}


	return dst;
}

void *memset(void *dst, int c, size_t n)
{
	char *q = dst;

	while (n--) {
		*q++ = c;
	}


	return dst;
}
