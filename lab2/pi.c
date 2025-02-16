#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

int main(int argc, char *argv[])
{
	printf("I'm in pi\n");
	double pi = 0;
	int a = 1;
	int n, i;
	int pd[2];					//канал для записи дескриптора
	sscanf(argv[0], "%d", &pd[0]);
	sscanf(argv[1], "%d", &pd[1]);
		
	read(pd[0], &n, sizeof(n));	
	printf("n in pi = %d\n", n);
	for (i = 0; i < n; i++)
	{
		pi += 4.0*a/(2.0*i + 1.0);
		a = a*(-1);
		printf("%d %lf\n", i, pi);
	}
	
	write (pd[1], &pi, sizeof(pi));	//запись в канал Pipe	

	printf("Pi in pi = %lf\n", pi);
	return 0;
}
