#include <stdio.h>
#include <unistd.h>

int main (void)
{
	return sysconf(_SC_NPROCESSORS_ONLN);
}
