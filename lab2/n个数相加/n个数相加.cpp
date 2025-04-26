//n个数相加 
#include<bits/stdc++.h>
#include<windows.h>
#include<stdlib.h>
using namespace std;

const int N (1<<25);
int f1(int* a,int n) //平凡算法 
{
int sum0=0,sum1=0,sum2=0,sum3=0,sum4=0;
int i;
for(i=0;i<n;i+=4)
{
    sum0+=a[i];
    sum1+=a[i+1];
    sum2+=a[i+2];
    sum3+=a[i+3];
}
for(int j=i;j<n;j++)
{
    sum4+=a[j];
}
int sum=sum0+sum1+sum2+sum3+sum4;
return sum;
}

int f2(int* a,int n) //两路链式 
{
	int sum1=0;
	int sum2=0;
	for(int i=0;i<n;i+=2)
	{
		sum1+=a[i];
		sum2+=a[i+1];
	}
	return sum1+sum2;
}
void f3(int* a,int n) //递归算法 
{
	if(n==1) return;
	for(int i=0;i<n;i+=2)
	{
		a[i/2]=a[i]+a[i+1];
	}
	n=n/2;
	f3(a,n);
}

int main()
{	
    ofstream f1_unrolling_plus_file("f1_unrolling_plus.txt"); 
	for(int t=0;t<25;t++)
	{
		int n=1;
		for(int i=0;i<t;i++)
		{
			n=n*2;
		}
		int* a=new int[N+5];
		for(int i=0;i<n;i++)
		{
			a[i]=3*i+71;
		}
		int sum=0;
		LARGE_INTEGER freq, start, end;		
		QueryPerformanceFrequency(&freq);
		
		//获得运行该算法所需要的时间(以周期为单位)		
		QueryPerformanceCounter(&start);
		for(int cnt = 0;cnt < 100;cnt++) sum=f1(a,n);
		QueryPerformanceCounter(&end);
		
		double elapsed_time = static_cast<double>(end.QuadPart - start.QuadPart) / (freq.QuadPart * 100);
        f1_unrolling_plus_file<< "f1_Elapsed time: " << elapsed_time*1e6 << " us\n";
	}
	return 0;
}



