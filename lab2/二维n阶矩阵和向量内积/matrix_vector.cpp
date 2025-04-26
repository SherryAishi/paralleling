//n*n矩阵每列与给定向量做内积 
#include<bits/stdc++.h>
#include<windows.h>
#include<stdlib.h>
int result[5005];
int b[5005];
void f1(int**a,int n)
{
	for(int i=0;i<n;i++)
	{
		result[i]=0;
	}
    for(int i=0;i<n;i++)
    {
        for(int j=0;j<n;j++)
        {
            result[i]+=a[j][i]*b[i];
        }
	} 
	return;
}
void f2(int**a,int n)
{
	for(int i=0;i<n;i++)
    {
		result[i]=0;
	}
    for(int i=0;i<n;i++)
    {
        for(int j=0;j<n;j++)
        {
            result[j]+=a[i][j]*b[i];
        }
	}
	return;
}
void f3(int**a,int n)
{
	for(int i=0;i<n;i++)
    {
        result[i]=0;
    }
    for(int i=0; i<n; i++) 
	{
        int j;
        for(j=0;j<=n-4;j+=4) {
        result[j]   += a[i][j]*b[j];
        result[j+1] += a[i][j+1]*b[j+1];
        result[j+2] += a[i][j+2]*b[j+2];
        result[j+3] += a[i][j+3]*b[j+3];}
        for(;j< n;j++) 
		{
            result[j]+=a[i][j]*b[j];
        }
    } 
} 
using namespace std;
int main()
{
	ofstream way2_unroll_file("way2_unroll.txt"); 
	for(int n=10;n<=5000;n<400 ? n+=10:n+=100)
	{
		
		//利用堆空间储存二维数组 
		int** a=new int*[5005];
		for(int i=0;i<5005;i++)
		{
			a[i]=new int[5005];
		}
		
		for(int i=0;i<n;i++)
		{
			for(int j=0;j<n;j++)
			{
				a[i][j]=2*i+j+13;
			}
		}
		for(int i=0;i<n;i++)
		{
			b[i]=i*5+17;
		}
		//cout<<"111";
		LARGE_INTEGER freq, start, end;
        QueryPerformanceFrequency(&freq);
        
        QueryPerformanceCounter(&start);
        int cnt=0;
        while(cnt<100)
        {
            f3(a,n);
		    cnt++;
		}
        QueryPerformanceCounter(&end);
        double elapsed_time = static_cast<double>(end.QuadPart - start.QuadPart) / (freq.QuadPart * 100);
        way2_unroll_file<< "n = "<<n<<"; way2_Elapsed time: " << elapsed_time*1e6 << " us\n";
	}
}

