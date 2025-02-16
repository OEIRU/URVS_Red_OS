#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

double factor(int n)
{
	double s = 1.0;
	int i;
	for(i=1; i<=n; i++)
		s*=i;
	return s;
}
double in_pow(int n, double x)
{
	double s = 1.0;
	int i;
	for(i=1; i<=n; i++)
		s*=x;
	return s;
}

int main()
{
	struct storage
	{
		int pid;	//идентификатор потомка
		double data;	//значение, полученное от потомка
	} st;
	int i, n, fd, pid_pi, pid_cos;	//fd - файловый дескриптор .tmp	
	int a = 1;
	double f, x, cos, Pi, s = 1.0;
	char *file_name = "file.tmp";
	struct stat tmp_stat;
	
	fd = open(file_name, O_RDWR);		//открываем временый файл
	printf("\nВведите x = ");
	scanf("%lf", &x);
	printf("\nВведите n = ");
	scanf("%d", &n);
	printf("\n");
	
	pid_pi = fork();
	if (pid_pi == 0)			//первый потомок процесса (для Pi)
	{
		st.pid = getpid();
		st.data = 0.0;
		for (i = 0; i < n; i++)
		{
			st.data += 4.0*a/(2.0*i + 1.0);
			a = a*(-1);
		}
		write (fd, &st, sizeof(st));
		exit(0);
	}
	
	pid_cos = fork();
	if (pid_cos == 0)			//второй потомок процесса (для cos)
	{
		st.pid = getpid();
		st.data = 1.0;		
		for (i = 1; i < n; i++)
		{
			s = factor(2*i);
			a = a*(-1);
			st.data += a*in_pow(2*i,x)/s;
		}
		write (fd, &st, sizeof(st));
		exit(0);
	}
	
	do {
		fstat(fd, &tmp_stat);
		} while (tmp_stat.st_size != 2*sizeof(st));
		
	lseek(fd,0,SEEK_SET);		//переходим в начало файла
	read(fd,&st,sizeof(st));
	if (st.pid == pid_pi) 
		Pi = st.data;
	if (st.pid == pid_cos) 
		cos = st.data;
	read(fd,&st,sizeof(st));
	if (st.pid == pid_pi) 
		Pi = st.data;
	if (st.pid == pid_cos) 
		cos = st.data;
		
	f = Pi * cos;	//вычисляем функцию
	
	printf("Pi = %lf\n", Pi);
	printf("cos(%lf) = %lf\n", x,cos);
	printf("f(%lf) = %lf\n", x,f);
	close(fd);				//закрываем
	remove(file_name);		//удаляем
	
	waitpid(pid_pi,NULL,0);
	waitpid(pid_cos,NULL,0);
	
	sleep(2);
	return 0;
}
