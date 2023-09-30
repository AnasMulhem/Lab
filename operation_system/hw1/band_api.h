#ifndef BAND_API_H
#define BAND_API_H
#include <stdio.h>
#include <assert.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>
#include <errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/list.h>

#include <linux/module.h>

int band_create(int instrument)
{
	{

		int res;
		__asm__
		(
			"pushl %%eax;"
			"pushl %%ebx;"
			"movl $243, %%eax;"
			"movl %1, %%ebx;"
			"int $0x80;"
			"movl %%eax,%0;"
			"popl %%ebx;"
			"popl %%eax;"
			: "=m" (res)
			: "m" (instrument)
		);
		if (res < 0)
		{
			errno = -res;
			res = -1;
		}
		return res;
	}
}


int band_join(pid_t member, int instrument)
{
	int res;
	__asm__
	(
		"pushl %%eax;"
		"pushl %%ebx;"
		"pushl %%ecx;"
		"movl $244, %%eax;"
		"movl %1, %%ebx;"
		"movl %2, %%ecx;"
		"int $0x80;"
		"movl %%eax,%0;"
		"popl %%ecx;"
		"popl %%ebx;"
		"popl %%eax;"
		: "=m" (res)
		: "m" (member), "m" (instrument)
	);
	if (res < 0)
	{
		errno = -res;
		res = -1;
	}
	return res;
}



int band_play(int instrument, unsigned char note)
{
	int res;
	__asm__
	(
		"pushl %%eax;"
		"pushl %%ebx;"
		"pushl %%ecx;"
		"movl $245, %%eax;"
		"movl %1, %%ebx;"
		"movl %2, %%ecx;"
		"int $0x80;"
		"movl %%eax,%0;"
		"popl %%ecx;"
		"popl %%ebx;"
		"popl %%eax;"
		: "=m" (res)
		: "m" (instrument), "m" (note)
	);
	if (res < 0)
	{
		errno = -res;
		res = -1;
	}
	return res;
}




int band_listen(pid_t member, unsigned char* chord)
{
	int res;
	__asm__
	(
		"pushl %%eax;"
		"pushl %%ebx;"
		"pushl %%ecx;"
		"movl $246, %%eax;"
		"movl %1, %%ebx;"
		"movl %2, %%ecx;"
		"int $0x80;"
		"movl %%eax,%0;"
		"popl %%ecx;"
		"popl %%ebx;"
		"popl %%eax;"
		: "=m" (res)
		: "m" (member), "m" (chord)
	);
	if (res < 0)
	{
		errno = -res;
		res = -1;
	}
	return res;

}

#endif // BAND_API_H
