#include <iostream>
#include <string>
#include <cstring>
#include <immintrin.h>

using namespace std;

// 定义了Byte，便于使用
typedef unsigned char Byte;

// 定义了32比特
typedef unsigned int bit32;

// MD5的一系列参数。参数是固定的，其实你不需要看懂这些
#define s11 7
#define s12 12
#define s13 17
#define s14 22
#define s21 5
#define s22 9
#define s23 14
#define s24 20
#define s31 4
#define s32 11
#define s33 16
#define s34 23
#define s41 6
#define s42 10
#define s43 15
#define s44 21

/**
 * @Basic MD5 functions.
 *
 * @param there bit32.
 *
 * @return one bit32.
 */
// 定义了一系列MD5中的具体函数
// 这四个计算函数是需要你进行SIMD并行化的
// 可以看到，FGHI四个函数都涉及一系列位运算，在数据上是对齐的，非常容易实现SIMD的并行化

#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))

//用SSE方式改写函数如下：
//此时，xyz均为：4个无符号int构成的数组,对其进行向量操作，实现同时完成四个数据的相同运算
//_mm_and_si128：按位与;_mm_andnot_si128 按位非后与;_mm_or_si128 按位或

/* 
在SSE中并没有按位非的操作，但是由于~x=x ^ 0xFFFFFFFF
则有 {0xFFFFFFFF * 4}=_mm_set1_epi32(-1)=__m128i all_ones
~x=_mm_xor_si128(x, all_ones);
*/

inline __m128i F_sse(__m128i x, __m128i y, __m128i z) {
  __m128i and1 = _mm_and_si128(x, y);   
  __m128i all_ones=_mm_set1_epi32(0xFFFFFFFF);
  __m128i notx = _mm_xor_si128(x, all_ones); //~x
  __m128i and2 = _mm_and_si128(notx, z);        
  return _mm_or_si128(and1, and2);      
}

//仿照上面的F_sse函数，可以将其他函数进行修改
inline __m128i G_sse(__m128i x, __m128i y, __m128i z) {
  __m128i and1 = _mm_and_si128(x, z);//x&z  
  __m128i all_ones=_mm_set1_epi32(0xFFFFFFFF);
  __m128i notz = _mm_xor_si128(z, all_ones); //~z  
  __m128i and2 = _mm_and_si128(notz, y); //y&~z       
  return _mm_or_si128(and1, and2);      
}
//在NEON中，^运算为veorq_u32，则有
inline __m128i H_sse(__m128i x, __m128i y, __m128i z) {
  __m128i result1 = _mm_xor_si128(x,y); //x^y             
  return _mm_xor_si128(result1,z);      
}

inline __m128i I_sse(__m128i x, __m128i y, __m128i z) {      
  __m128i all_ones=_mm_set1_epi32(0xFFFFFFFF);
  __m128i notz = _mm_xor_si128(z, all_ones); //~z  
  __m128i result1 = _mm_or_si128(x,notz); //x|~z
  return _mm_xor_si128(y,result1);      
}


/**
 * @Rotate Left.
 *
 * @param {num} the raw number.
 *
 * @param {n} rotate left n.
 *
 * @return the number after rotated left.
 */
// 定义了一系列MD5中的具体函数
// 这五个计算函数（ROTATELEFT/FF/GG/HH/II）和之前的FGHI一样，都是需要你进行SIMD并行化的
// 但是你需要注意的是#define的功能及其效果，可以发现这里的FGHI是没有返回值的，为什么呢？你可以查询#define的含义和用法
#define ROTATELEFT(num, n) (((num) << (n)) | ((num) >> (32-(n))))

#define FF(a, b, c, d, x, s, ac) { \
  (a) += F ((b), (c), (d)) + (x) + ac; \
  (a) = ROTATELEFT ((a), (s)); \
  (a) += (b); \
}

#define GG(a, b, c, d, x, s, ac) { \
  (a) += G ((b), (c), (d)) + (x) + ac; \
  (a) = ROTATELEFT ((a), (s)); \
  (a) += (b); \
}
#define HH(a, b, c, d, x, s, ac) { \
  (a) += H ((b), (c), (d)) + (x) + ac; \
  (a) = ROTATELEFT ((a), (s)); \
  (a) += (b); \
}
#define II(a, b, c, d, x, s, ac) { \
  (a) += I ((b), (c), (d)) + (x) + ac; \
  (a) = ROTATELEFT ((a), (s)); \
  (a) += (b); \
}



inline __m128i ROTATELEFT1(__m128i num, int n) {
  __m128i result1=_mm_slli_epi32(num, n); 
  __m128i result2=_mm_srli_epi32(num, 32 - n); 
  return  _mm_or_si128(result1,result2);
}

//其中b,c,d参与F运算，则数据类型应该为__m128i,同理x和ac也同为该数据类型，而s代表数据的移位位数，仍为int

inline void FF1(__m128i &a,__m128i b,__m128i c,__m128i d,__m128i x,int s,bit32 ac){
  __m128i ac1=_mm_set1_epi32(ac);
  a=_mm_add_epi32(a,_mm_add_epi32( _mm_add_epi32(F_sse((b),(c),(d)),x),ac1 ));
  a=ROTATELEFT1((a),(s));
  a=_mm_add_epi32(a,b);
}
inline void GG1(__m128i &a,__m128i b,__m128i c,__m128i d,__m128i x,int s,bit32 ac){
  __m128i ac1=_mm_set1_epi32(ac);
  a=_mm_add_epi32(a,_mm_add_epi32( _mm_add_epi32(G_sse((b),(c),(d)),x),ac1 ));
  a=ROTATELEFT1((a),(s));
  a=_mm_add_epi32(a,b);
}
inline void HH1(__m128i &a,__m128i b,__m128i c,__m128i d,__m128i x,int s,bit32 ac){
  __m128i ac1=_mm_set1_epi32(ac);
  a=_mm_add_epi32(a,_mm_add_epi32( _mm_add_epi32(H_sse((b),(c),(d)),x),ac1 ));
  a=ROTATELEFT1((a),(s));
  a=_mm_add_epi32(a,b);
}
inline void II1(__m128i &a,__m128i b,__m128i c,__m128i d,__m128i x,int s,bit32 ac){
  __m128i ac1=_mm_set1_epi32(ac);
  a=_mm_add_epi32(a,_mm_add_epi32( _mm_add_epi32(I_sse((b),(c),(d)),x),ac1 ));
  a=ROTATELEFT1((a),(s));
  a=_mm_add_epi32(a,b);
}




inline __m256i F_avg(__m256i x, __m256i y, __m256i z) {
  __m256i and1 = _mm256_and_si256(x, y);   
  __m256i all_ones=_mm256_set1_epi32(0xFFFFFFFF);
  __m256i notx = _mm256_xor_si256(x, all_ones); //~x
  __m256i and2 = _mm256_and_si256(notx, z);        
  return _mm256_or_si256(and1, and2);      
}

//仿照上面的F_avg函数，可以将其他函数进行修改
inline __m256i G_avg(__m256i x, __m256i y, __m256i z) {
  __m256i and1 = _mm256_and_si256(x, z);//x&z  
  __m256i all_ones=_mm256_set1_epi32(0xFFFFFFFF);
  __m256i notz = _mm256_xor_si256(z, all_ones); //~z  
  __m256i and2 = _mm256_and_si256(notz, y); //y&~z       
  return _mm256_or_si256(and1, and2);      
}
//在NEON中，^运算为veorq_u32，则有
inline __m256i H_avg(__m256i x, __m256i y, __m256i z) {
  __m256i result1 = _mm256_xor_si256(x,y); //x^y             
  return _mm256_xor_si256(result1,z);      
}

inline __m256i I_avg(__m256i x, __m256i y, __m256i z) {      
  __m256i all_ones=_mm256_set1_epi32(0xFFFFFFFF);
  __m256i notz = _mm256_xor_si256(z, all_ones); //~z  
  __m256i result1 = _mm256_or_si256(x,notz); //x|~z
  return _mm256_xor_si256(y,result1);      
}

//8条口令的情况
inline __m256i ROTATELEFT2(__m256i num, int n) {
  __m256i result1=_mm256_slli_epi32(num, n); 
  __m256i result2=_mm256_srli_epi32(num, 32 - n); 
  return  _mm256_or_si256(result1,result2);
}
inline void FF2(__m256i &a,__m256i b,__m256i c,__m256i d,__m256i x,int s,bit32 ac){
  __m256i ac1=_mm256_set1_epi32(ac);
  a=_mm256_add_epi32(a,_mm256_add_epi32( _mm256_add_epi32(F_avg((b),(c),(d)),x),ac1 ));
  a=ROTATELEFT2((a),(s));
  a=_mm256_add_epi32(a,b);
}
inline void GG2(__m256i &a,__m256i b,__m256i c,__m256i d,__m256i x,int s,bit32 ac){
  __m256i ac1=_mm256_set1_epi32(ac);
  a=_mm256_add_epi32(a,_mm256_add_epi32( _mm256_add_epi32(G_avg((b),(c),(d)),x),ac1 ));
  a=ROTATELEFT2((a),(s));
  a=_mm256_add_epi32(a,b);
}
inline void HH2(__m256i &a,__m256i b,__m256i c,__m256i d,__m256i x,int s,bit32 ac){
  __m256i ac1=_mm256_set1_epi32(ac);
  a=_mm256_add_epi32(a,_mm256_add_epi32( _mm256_add_epi32(H_avg((b),(c),(d)),x),ac1 ));
  a=ROTATELEFT2((a),(s));
  a=_mm256_add_epi32(a,b);
}
inline void II2(__m256i &a,__m256i b,__m256i c,__m256i d,__m256i x,int s,bit32 ac){
  __m256i ac1=_mm256_set1_epi32(ac);
  a=_mm256_add_epi32(a,_mm256_add_epi32( _mm256_add_epi32(I_avg((b),(c),(d)),x),ac1 ));
  a=ROTATELEFT2((a),(s));
  a=_mm256_add_epi32(a,b);
}


void MD5Hash(string input, bit32 *state);
void MD5Hash1(string *input, __m128i *state);
void MD5Hash2(string *input, __m256i *state);