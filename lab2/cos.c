#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

//вычисление факториала
double factor(int n)
{
	double s = 1.0;
	int i;
	for(i=1; i<=n; i++)
		s*=i;
	return s;
}
//вычисление числа x^n
double in_pow(int n, double x)
{
	double s = 1.0;
	int i;
	for(i=1; i<=n; i++)
		s*=x;
	return s;
}

int main(int argc, char *argv[])
{
	printf("I'm in cos\n");
	double cos = 1.0;		// Значение косинуса
	double x; 			// аргумент косинуса
	double s; 			// переменная хранящая факториал
	int a = 1; 
	int pd[2];			//канал для записи дескриптора
	int n, i;

	//получаем информацию о канале
	sscanf(argv[0], "%d", &pd[0]);
	sscanf(argv[1], "%d", &pd[1]);	
	
	// считываем параметры
	read(pd[0], &n, sizeof(n));
	read(pd[0], &x, sizeof(x));
	printf("n in cos = %d\n", n);
	printf("x in cos = %lf\n", x);
	//собственно вычисления		
	for (i = 1; i < n; i++)
	{
		s = factor(2*i);
		a = a*(-1);
		cos += a*in_pow(2*i,x)/s;
		printf("%d %lf\n", i, cos);
	}

	write(pd[1], &cos, sizeof(cos));	// возвращаем результат в канал
	printf("cos(%lf) in cos = %lf\n", x,cos);//пишем результат на экран
	return 0;
}		
