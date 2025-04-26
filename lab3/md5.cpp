#include "md5.h"
#include <iomanip>
#include <assert.h>
#include <chrono>
#include <immintrin.h>

using namespace std;
using namespace chrono;

Byte *StringProcess(string input, int *n_byte)
{
	// 将输入的字符串转换为Byte为单位的数组
	Byte *blocks = (Byte *)input.c_str();
	int length = input.length();

	// 计算原始消息长度（以比特为单位）
	int bitLength = length * 8;

	// paddingBits: 原始消息需要的padding长度（以bit为单位）
	// 对于给定的消息，将其补齐至length%512==448为止
	// 需要注意的是，即便给定的消息满足length%512==448，也需要再pad 512bits
	int paddingBits = bitLength % 512;
	if (paddingBits > 448)
	{
		paddingBits += 512 - (paddingBits - 448);//之所以按448来分割，是由于需要留出64bit用于存放原本信息长度
	}
	else if (paddingBits < 448)
	{
		paddingBits = 448 - paddingBits;  
	}
	else if (paddingBits == 448)
	{
		paddingBits = 512;
	}
    //简单来说,paddingBits就是通过填补一些空余空间，来形成多个512bit的数据块


	// 原始消息需要的padding长度（以Byte为单位）
	int paddingBytes = paddingBits / 8;
	// 创建最终的字节数组
	// length + paddingBytes + 8:
	// 1. length为原始消息的长度（bits）
	// 2. paddingBytes为原始消息需要的padding长度（Bytes）
	// 3. 在pad到length%512==448之后，需要额外附加64bits的原始消息长度，即8个bytes
	int paddedLength = length + paddingBytes + 8;
	Byte *paddedMessage = new Byte[paddedLength];
    //paddedMessage指向大小为paddedlength的Byte类型数组


	// 复制原始消息
	memcpy(paddedMessage, blocks, length);

	// 添加填充字节。填充时，第一位为1，后面的所有位均为0。
	// 所以第一个byte是0x80
	paddedMessage[length] = 0x80;							 // 添加一个0x80字节
	memset(paddedMessage + length + 1, 0, paddingBytes - 1); // 填充0字节

	// 添加消息长度（64比特，小端格式）
	for (int i = 0; i < 8; ++i)
	{
		// 特别注意此处应当将bitLength转换为uint64_t
		// 这里的length是原始消息的长度
		paddedMessage[length + paddingBytes + i] = ((uint64_t)length * 8 >> (i * 8)) & 0xFF;
	}

	// 验证长度是否满足要求。此时长度应当是512bit的倍数
	int residual = 8 * paddedLength % 512;
	// assert(residual == 0);

	// 在填充+添加长度之后，消息被分为n_blocks个512bit的部分
	*n_byte = paddedLength;
	return paddedMessage;
}
//串行
void MD5Hash(string input, bit32 *state)
{

	Byte *paddedMessage;
	int *messageLength = new int[1];  //messageLengrh璁板綍鐨勬槸鏈€缁坧addermessage鐨勫ぇ灏? 鍘熷淇℃伅闀垮害+padding鐨勯暱搴?8(鐢ㄤ簬瀛樻斁鍘熷淇℃伅鐨勯暱搴? )
	for (int i = 0; i < 1; i += 1)
	{
		paddedMessage = StringProcess(input, &messageLength[i]);
		// cout<<messageLength[i]<<endl;
		assert(messageLength[i] == messageLength[0]);
	}
	int n_blocks = messageLength[0] / 64;  //鍒檔_blocks鐨勫€间负锛氬皢paddedmessage鍖哄垎涓?12bit鐨勫潡鍙互鍒嗕负澶氬皯鍧?

	// bit32* state= new bit32[4];
	state[0] = 0x67452301; 
	state[1] = 0xefcdab89;
	state[2] = 0x98badcfe;
	state[3] = 0x10325476;
    //state浣滀负鏁扮粍琚紶鍏ュ埌鍑芥暟涓紝瀹為檯涓婁紶鍏ョ殑鏄叾鎸囬拡锛屽湪鍑芥暟涓鍏惰繘琛屼慨鏀癸紝浼氱‘瀹炵殑鏀瑰彉鍏跺€?




	// 閫恇lock鍦版洿鏂皊tate
	for (int i = 0; i < n_blocks; i += 1)
	{
		bit32 x[16]; //灏嗘瘡涓猙lock鍖哄垎涓?6閮ㄥ垎

		// 涓嬮潰鐨勫鐞嗭紝鍦ㄧ悊瑙ｄ笂杈冧负澶嶆潅
		for (int i1 = 0; i1 < 16; ++i1)
		{
			x[i1] = (paddedMessage[4 * i1 + i * 64]) |
					(paddedMessage[4 * i1 + 1 + i * 64] << 8) |
					(paddedMessage[4 * i1 + 2 + i * 64] << 16) |
					(paddedMessage[4 * i1 + 3 + i * 64] << 24);
		}

		bit32 a = state[0], b = state[1], c = state[2], d = state[3];

		auto start = system_clock::now();
		/* Round 1 */
		FF(a, b, c, d, x[0], s11, 0xd76aa478);
		FF(d, a, b, c, x[1], s12, 0xe8c7b756);
		FF(c, d, a, b, x[2], s13, 0x242070db);
		FF(b, c, d, a, x[3], s14, 0xc1bdceee);
		FF(a, b, c, d, x[4], s11, 0xf57c0faf);
		FF(d, a, b, c, x[5], s12, 0x4787c62a);
		FF(c, d, a, b, x[6], s13, 0xa8304613);
		FF(b, c, d, a, x[7], s14, 0xfd469501);
		FF(a, b, c, d, x[8], s11, 0x698098d8);
		FF(d, a, b, c, x[9], s12, 0x8b44f7af);
		FF(c, d, a, b, x[10], s13, 0xffff5bb1);
		FF(b, c, d, a, x[11], s14, 0x895cd7be);
		FF(a, b, c, d, x[12], s11, 0x6b901122);
		FF(d, a, b, c, x[13], s12, 0xfd987193);
		FF(c, d, a, b, x[14], s13, 0xa679438e);
		FF(b, c, d, a, x[15], s14, 0x49b40821);

		/* Round 2 */
		GG(a, b, c, d, x[1], s21, 0xf61e2562);
		GG(d, a, b, c, x[6], s22, 0xc040b340);
		GG(c, d, a, b, x[11], s23, 0x265e5a51);
		GG(b, c, d, a, x[0], s24, 0xe9b6c7aa);
		GG(a, b, c, d, x[5], s21, 0xd62f105d);
		GG(d, a, b, c, x[10], s22, 0x2441453);
		GG(c, d, a, b, x[15], s23, 0xd8a1e681);
		GG(b, c, d, a, x[4], s24, 0xe7d3fbc8);
		GG(a, b, c, d, x[9], s21, 0x21e1cde6);
		GG(d, a, b, c, x[14], s22, 0xc33707d6);
		GG(c, d, a, b, x[3], s23, 0xf4d50d87);
		GG(b, c, d, a, x[8], s24, 0x455a14ed);
		GG(a, b, c, d, x[13], s21, 0xa9e3e905);
		GG(d, a, b, c, x[2], s22, 0xfcefa3f8);
		GG(c, d, a, b, x[7], s23, 0x676f02d9);
		GG(b, c, d, a, x[12], s24, 0x8d2a4c8a);

		/* Round 3 */
		HH(a, b, c, d, x[5], s31, 0xfffa3942);
		HH(d, a, b, c, x[8], s32, 0x8771f681);
		HH(c, d, a, b, x[11], s33, 0x6d9d6122);
		HH(b, c, d, a, x[14], s34, 0xfde5380c);
		HH(a, b, c, d, x[1], s31, 0xa4beea44);
		HH(d, a, b, c, x[4], s32, 0x4bdecfa9);
		HH(c, d, a, b, x[7], s33, 0xf6bb4b60);
		HH(b, c, d, a, x[10], s34, 0xbebfbc70);
		HH(a, b, c, d, x[13], s31, 0x289b7ec6);
		HH(d, a, b, c, x[0], s32, 0xeaa127fa);
		HH(c, d, a, b, x[3], s33, 0xd4ef3085);
		HH(b, c, d, a, x[6], s34, 0x4881d05);
		HH(a, b, c, d, x[9], s31, 0xd9d4d039);
		HH(d, a, b, c, x[12], s32, 0xe6db99e5);
		HH(c, d, a, b, x[15], s33, 0x1fa27cf8);
		HH(b, c, d, a, x[2], s34, 0xc4ac5665);

		/* Round 4 */
		II(a, b, c, d, x[0], s41, 0xf4292244);
		II(d, a, b, c, x[7], s42, 0x432aff97);
		II(c, d, a, b, x[14], s43, 0xab9423a7);
		II(b, c, d, a, x[5], s44, 0xfc93a039);
		II(a, b, c, d, x[12], s41, 0x655b59c3);
		II(d, a, b, c, x[3], s42, 0x8f0ccc92);
		II(c, d, a, b, x[10], s43, 0xffeff47d);
		II(b, c, d, a, x[1], s44, 0x85845dd1);
		II(a, b, c, d, x[8], s41, 0x6fa87e4f);
		II(d, a, b, c, x[15], s42, 0xfe2ce6e0);
		II(c, d, a, b, x[6], s43, 0xa3014314);
		II(b, c, d, a, x[13], s44, 0x4e0811a1);
		II(a, b, c, d, x[4], s41, 0xf7537e82);
		II(d, a, b, c, x[11], s42, 0xbd3af235);
		II(c, d, a, b, x[2], s43, 0x2ad7d2bb);
		II(b, c, d, a, x[9], s44, 0xeb86d391);

		state[0] += a;
		state[1] += b;
		state[2] += c;
		state[3] += d;
	}

	// 涓嬮潰鐨勫鐞嗭紝鍦ㄧ悊瑙ｄ笂杈冧负澶嶆潅
	for (int i = 0; i < 4; i++)
	{
		uint32_t value = state[i];
		state[i] = ((value & 0xff) << 24) |		 // 灏嗘渶浣庡瓧鑺傜Щ鍒版渶楂樹綅
				   ((value & 0xff00) << 8) |	 // 灏嗘浣庡瓧鑺傚乏绉?
				   ((value & 0xff0000) >> 8) |	 // 灏嗘楂樺瓧鑺傚彸绉?
				   ((value & 0xff000000) >> 24); // 灏嗘渶楂樺瓧鑺傜Щ鍒版渶浣庝綅
	}

	// 杈撳嚭鏈€缁堢殑hash缁撴灉
	// for (int i1 = 0; i1 < 4; i1 += 1)
	// {
	// 	cout << std::setw(8) << std::setfill('0') << hex << state[i1];
	// }
	// cout << endl;

	// 閲婃斁鍔ㄦ€佸垎閰嶇殑鍐呭瓨
	// 瀹炵幇SIMD骞惰绠楁硶鐨勬椂鍊欙紝涔熻璁板緱鍙婃椂鍥炴敹鍐呭瓨锛?
	delete[] paddedMessage;
	delete[] messageLength;
}

//4路
void MD5Hash1(string *input, __m128i *state)
{
	Byte *paddedMessage[4];
	int messageLength[4];  //messageLengrh记录的是最终paddermessage的大小( 原始信息长度+padding的长度+8(用于存放原始信息的长度) )
	for (int i = 0; i < 4; i += 1)
	{
		paddedMessage[i] = StringProcess(input[i], &messageLength[i]);
		// cout<<messageLength[i]<<endl;
		assert(messageLength[i] == messageLength[0]);
	}
	int n_blocks = messageLength[0] / 64;  //则n_blocks的值为：将paddedmessage区分为512bit的块可以分为多少块
    //由于n_blocks都是一样的，口令不太可能超过2个block


	// bit32* state= new bit32[4];
	state[0] = _mm_set1_epi32(0x67452301); 
	state[1] = _mm_set1_epi32(0xefcdab89);
	state[2] = _mm_set1_epi32(0x98badcfe);
	state[3] = _mm_set1_epi32(0x10325476);
    //state作为数组被传入到函数中，实际上传入的是其指针，在函数中对其进行修改，会确实的改变其值




	// 逐block地更新state
	for (int i = 0; i < n_blocks; i += 1)
	{
		__m128i x[16]; //将每个block区分为16部分

		bit32 y[4];

		// 下面的处理，在理解上较为复杂
		for (int i1 = 0; i1 < 16; ++i1)
		{
			for(int j=0;j<4;j++)
			{
				y[j]=(paddedMessage[j][4 * i1 + i * 64]) | (paddedMessage[j][4 * i1 + 1 + i * 64] << 8) | 
				(paddedMessage[j][4 * i1 + 2 + i * 64] << 16) | (paddedMessage[j][4 * i1 + 3 + i * 64] << 24);
			}
			x[i1] = _mm_loadu_si128((__m128i*)y);
		}

		__m128i a = state[0], b = state[1], c = state[2], d = state[3];

		auto start = system_clock::now();
		/* Round 1 */
		FF1(a, b, c, d, x[0], s11, 0xd76aa478);
		FF1(d, a, b, c, x[1], s12, 0xe8c7b756);
		FF1(c, d, a, b, x[2], s13, 0x242070db);
		FF1(b, c, d, a, x[3], s14, 0xc1bdceee);
		FF1(a, b, c, d, x[4], s11, 0xf57c0faf);
		FF1(d, a, b, c, x[5], s12, 0x4787c62a);
		FF1(c, d, a, b, x[6], s13, 0xa8304613);
		FF1(b, c, d, a, x[7], s14, 0xfd469501);
		FF1(a, b, c, d, x[8], s11, 0x698098d8);
		FF1(d, a, b, c, x[9], s12, 0x8b44f7af);
		FF1(c, d, a, b, x[10], s13, 0xffff5bb1);
		FF1(b, c, d, a, x[11], s14, 0x895cd7be);
		FF1(a, b, c, d, x[12], s11, 0x6b901122);
		FF1(d, a, b, c, x[13], s12, 0xfd987193);
		FF1(c, d, a, b, x[14], s13, 0xa679438e);
		FF1(b, c, d, a, x[15], s14, 0x49b40821);

		/* Round 2 */
		GG1(a, b, c, d, x[1], s21, 0xf61e2562);
		GG1(d, a, b, c, x[6], s22, 0xc040b340);
		GG1(c, d, a, b, x[11], s23, 0x265e5a51);
		GG1(b, c, d, a, x[0], s24, 0xe9b6c7aa);
		GG1(a, b, c, d, x[5], s21, 0xd62f105d);
		GG1(d, a, b, c, x[10], s22, 0x2441453);
		GG1(c, d, a, b, x[15], s23, 0xd8a1e681);
		GG1(b, c, d, a, x[4], s24, 0xe7d3fbc8);
		GG1(a, b, c, d, x[9], s21, 0x21e1cde6);
		GG1(d, a, b, c, x[14], s22, 0xc33707d6);
		GG1(c, d, a, b, x[3], s23, 0xf4d50d87);
		GG1(b, c, d, a, x[8], s24, 0x455a14ed);
		GG1(a, b, c, d, x[13], s21, 0xa9e3e905);
		GG1(d, a, b, c, x[2], s22, 0xfcefa3f8);
		GG1(c, d, a, b, x[7], s23, 0x676f02d9);
		GG1(b, c, d, a, x[12], s24, 0x8d2a4c8a);

		/* Round 3 */
		HH1(a, b, c, d, x[5], s31, 0xfffa3942);
		HH1(d, a, b, c, x[8], s32, 0x8771f681);
		HH1(c, d, a, b, x[11], s33, 0x6d9d6122);
		HH1(b, c, d, a, x[14], s34, 0xfde5380c);
		HH1(a, b, c, d, x[1], s31, 0xa4beea44);
		HH1(d, a, b, c, x[4], s32, 0x4bdecfa9);
		HH1(c, d, a, b, x[7], s33, 0xf6bb4b60);
		HH1(b, c, d, a, x[10], s34, 0xbebfbc70);
		HH1(a, b, c, d, x[13], s31, 0x289b7ec6);
		HH1(d, a, b, c, x[0], s32, 0xeaa127fa);
		HH1(c, d, a, b, x[3], s33, 0xd4ef3085);
		HH1(b, c, d, a, x[6], s34, 0x4881d05);
		HH1(a, b, c, d, x[9], s31, 0xd9d4d039);
		HH1(d, a, b, c, x[12], s32, 0xe6db99e5);
		HH1(c, d, a, b, x[15], s33, 0x1fa27cf8);
		HH1(b, c, d, a, x[2], s34, 0xc4ac5665);

		/* Round 4 */
		II1(a, b, c, d, x[0], s41, 0xf4292244);
		II1(d, a, b, c, x[7], s42, 0x432aff97);
		II1(c, d, a, b, x[14], s43, 0xab9423a7);
		II1(b, c, d, a, x[5], s44, 0xfc93a039);
		II1(a, b, c, d, x[12], s41, 0x655b59c3);
		II1(d, a, b, c, x[3], s42, 0x8f0ccc92);
		II1(c, d, a, b, x[10], s43, 0xffeff47d);
		II1(b, c, d, a, x[1], s44, 0x85845dd1);
		II1(a, b, c, d, x[8], s41, 0x6fa87e4f);
		II1(d, a, b, c, x[15], s42, 0xfe2ce6e0);
		II1(c, d, a, b, x[6], s43, 0xa3014314);
		II1(b, c, d, a, x[13], s44, 0x4e0811a1);
		II1(a, b, c, d, x[4], s41, 0xf7537e82);
		II1(d, a, b, c, x[11], s42, 0xbd3af235);
		II1(c, d, a, b, x[2], s43, 0x2ad7d2bb);
		II1(b, c, d, a, x[9], s44, 0xeb86d391);

		state[0] = _mm_add_epi32(state[0],a);
		state[1] = _mm_add_epi32(state[1],b);
		state[2] = _mm_add_epi32(state[2],c);
		state[3] = _mm_add_epi32(state[3],d);
	}

	// 下面的处理，在理解上较为复杂
	for (int i = 0; i < 4; i++)
	{
		__m128i value = state[i];
		__m128i re1=_mm_or_si128(
			_mm_slli_epi32(	_mm_and_si128(value,_mm_set1_epi32(0xff)),24),
			_mm_slli_epi32(	_mm_and_si128(value,_mm_set1_epi32(0xff00)),8));
		
		__m128i re2=_mm_or_si128(
			_mm_srli_epi32(	_mm_and_si128(value,_mm_set1_epi32(0xff0000)),8),
			_mm_srli_epi32(	_mm_and_si128(value,_mm_set1_epi32(0xff000000)),24));
		
		__m128i re=_mm_or_si128(re1,re2);
		state[i] = re;
	}

	// 输出最终的hash结果
	// for (int i1 = 0; i1 < 4; i1 += 1)
	// {
	// 	cout << std::setw(8) << std::setfill('0') << hex << state[i1];
	// }
	// cout << endl;

	// 释放动态分配的内存
	// 实现SIMD并行算法的时候，也请记得及时回收内存！
}
//8路
void MD5Hash2(string *input, __m256i *state)
{
	Byte *paddedMessage[8];
	int messageLength[8];  //messageLengrh记录的是最终paddermessage的大小( 原始信息长度+padding的长度+8(用于存放原始信息的长度) )
	for (int i = 0; i < 8; i += 1)
	{
		paddedMessage[i] = StringProcess(input[i], &messageLength[i]);
		// cout<<messageLength[i]<<endl;
		assert(messageLength[i] == messageLength[0]);
	}
	int n_blocks = messageLength[0] / 64;  //则n_blocks的值为：将paddedmessage区分为512bit的块可以分为多少块
    //由于n_blocks都是一样的，口令不太可能超过2个block


	// bit32* state= new bit32[4];
	state[0] = _mm256_set1_epi32(0x67452301); 
	state[1] = _mm256_set1_epi32(0xefcdab89);
	state[2] = _mm256_set1_epi32(0x98badcfe);
	state[3] = _mm256_set1_epi32(0x10325476);
    //state作为数组被传入到函数中，实际上传入的是其指针，在函数中对其进行修改，会确实的改变其值




	// 逐block地更新state
	for (int i = 0; i < n_blocks; i += 1)
	{
		__m256i x[16]; //将每个block区分为16部分

		bit32 y[8];

		// 下面的处理，在理解上较为复杂
		for (int i1 = 0; i1 < 16; ++i1)
		{
			for(int j=0;j<8;j++)
			{
				y[j]=(paddedMessage[j][4 * i1 + i * 64]) | (paddedMessage[j][4 * i1 + 1 + i * 64] << 8) | 
				(paddedMessage[j][4 * i1 + 2 + i * 64] << 16) | (paddedMessage[j][4 * i1 + 3 + i * 64] << 24);
			}
			x[i1] = _mm256_loadu_si256((__m256i*)y);
		}

		__m256i a = state[0], b = state[1], c = state[2], d = state[3];

		auto start = system_clock::now();
		/* Round 1 */
		FF2(a, b, c, d, x[0], s11, 0xd76aa478);
		FF2(d, a, b, c, x[1], s12, 0xe8c7b756);
		FF2(c, d, a, b, x[2], s13, 0x242070db);
		FF2(b, c, d, a, x[3], s14, 0xc1bdceee);
		FF2(a, b, c, d, x[4], s11, 0xf57c0faf);
		FF2(d, a, b, c, x[5], s12, 0x4787c62a);
		FF2(c, d, a, b, x[6], s13, 0xa8304613);
		FF2(b, c, d, a, x[7], s14, 0xfd469501);
		FF2(a, b, c, d, x[8], s11, 0x698098d8);
		FF2(d, a, b, c, x[9], s12, 0x8b44f7af);
		FF2(c, d, a, b, x[10], s13, 0xffff5bb1);
		FF2(b, c, d, a, x[11], s14, 0x895cd7be);
		FF2(a, b, c, d, x[12], s11, 0x6b901122);
		FF2(d, a, b, c, x[13], s12, 0xfd987193);
		FF2(c, d, a, b, x[14], s13, 0xa679438e);
		FF2(b, c, d, a, x[15], s14, 0x49b40821);

		/* Round 2 */
		GG2(a, b, c, d, x[1], s21, 0xf61e2562);
		GG2(d, a, b, c, x[6], s22, 0xc040b340);
		GG2(c, d, a, b, x[11], s23, 0x265e5a51);
		GG2(b, c, d, a, x[0], s24, 0xe9b6c7aa);
		GG2(a, b, c, d, x[5], s21, 0xd62f105d);
		GG2(d, a, b, c, x[10], s22, 0x2441453);
		GG2(c, d, a, b, x[15], s23, 0xd8a1e681);
		GG2(b, c, d, a, x[4], s24, 0xe7d3fbc8);
		GG2(a, b, c, d, x[9], s21, 0x21e1cde6);
		GG2(d, a, b, c, x[14], s22, 0xc33707d6);
		GG2(c, d, a, b, x[3], s23, 0xf4d50d87);
		GG2(b, c, d, a, x[8], s24, 0x455a14ed);
		GG2(a, b, c, d, x[13], s21, 0xa9e3e905);
		GG2(d, a, b, c, x[2], s22, 0xfcefa3f8);
		GG2(c, d, a, b, x[7], s23, 0x676f02d9);
		GG2(b, c, d, a, x[12], s24, 0x8d2a4c8a);

		/* Round 3 */
		HH2(a, b, c, d, x[5], s31, 0xfffa3942);
		HH2(d, a, b, c, x[8], s32, 0x8771f681);
		HH2(c, d, a, b, x[11], s33, 0x6d9d6122);
		HH2(b, c, d, a, x[14], s34, 0xfde5380c);
		HH2(a, b, c, d, x[1], s31, 0xa4beea44);
		HH2(d, a, b, c, x[4], s32, 0x4bdecfa9);
		HH2(c, d, a, b, x[7], s33, 0xf6bb4b60);
		HH2(b, c, d, a, x[10], s34, 0xbebfbc70);
		HH2(a, b, c, d, x[13], s31, 0x289b7ec6);
		HH2(d, a, b, c, x[0], s32, 0xeaa127fa);
		HH2(c, d, a, b, x[3], s33, 0xd4ef3085);
		HH2(b, c, d, a, x[6], s34, 0x4881d05);
		HH2(a, b, c, d, x[9], s31, 0xd9d4d039);
		HH2(d, a, b, c, x[12], s32, 0xe6db99e5);
		HH2(c, d, a, b, x[15], s33, 0x1fa27cf8);
		HH2(b, c, d, a, x[2], s34, 0xc4ac5665);

		/* Round 4 */
		II2(a, b, c, d, x[0], s41, 0xf4292244);
		II2(d, a, b, c, x[7], s42, 0x432aff97);
		II2(c, d, a, b, x[14], s43, 0xab9423a7);
		II2(b, c, d, a, x[5], s44, 0xfc93a039);
		II2(a, b, c, d, x[12], s41, 0x655b59c3);
		II2(d, a, b, c, x[3], s42, 0x8f0ccc92);
		II2(c, d, a, b, x[10], s43, 0xffeff47d);
		II2(b, c, d, a, x[1], s44, 0x85845dd1);
		II2(a, b, c, d, x[8], s41, 0x6fa87e4f);
		II2(d, a, b, c, x[15], s42, 0xfe2ce6e0);
		II2(c, d, a, b, x[6], s43, 0xa3014314);
		II2(b, c, d, a, x[13], s44, 0x4e0811a1);
		II2(a, b, c, d, x[4], s41, 0xf7537e82);
		II2(d, a, b, c, x[11], s42, 0xbd3af235);
		II2(c, d, a, b, x[2], s43, 0x2ad7d2bb);
		II2(b, c, d, a, x[9], s44, 0xeb86d391);

		state[0] = _mm256_add_epi32(state[0],a);
		state[1] = _mm256_add_epi32(state[1],b);
		state[2] = _mm256_add_epi32(state[2],c);
		state[3] = _mm256_add_epi32(state[3],d);
	}

	// 下面的处理，在理解上较为复杂
	for (int i = 0; i < 4; i++)
	{
		__m256i value = state[i];
		__m256i re1=_mm256_or_si256(
			_mm256_slli_epi32(	_mm256_and_si256(value,_mm256_set1_epi32(0xff)),24),
			_mm256_slli_epi32(	_mm256_and_si256(value,_mm256_set1_epi32(0xff00)),8));
		
		__m256i re2=_mm256_or_si256(
			_mm256_srli_epi32(	_mm256_and_si256(value,_mm256_set1_epi32(0xff0000)),8),
			_mm256_srli_epi32(	_mm256_and_si256(value,_mm256_set1_epi32(0xff000000)),24));
		
		__m256i re=_mm256_or_si256(re1,re2);
		state[i] = re;
	}
}


// int main()
// {
//      string pwd[4] = {"abc","def","hij","knb"};
//      __m128i state_simd[4];
//      MD5Hash1(pwd,state_simd);
//      bit32 state_show[4][4];
//      for (int i = 0; i < 4; i++) {
//      uint32_t tmp0[4], tmp1[4], tmp2[4], tmp3[4];
// 	 _mm_storeu_si128((__m128i*)tmp0, state_simd[0]);
// 	 _mm_storeu_si128((__m128i*)tmp1, state_simd[1]);
// 	 _mm_storeu_si128((__m128i*)tmp2, state_simd[2]);
// 	 _mm_storeu_si128((__m128i*)tmp3, state_simd[3]);

//      state_show[i][0] = tmp0[i];
//      state_show[i][1] = tmp1[i];
//      state_show[i][2] = tmp2[i];
//      state_show[i][3] = tmp3[i];
//     }

//      for(int i = 0;i < 4;i++)
//      {
//           cout<<"me:";
//           for(int j = 0;j < 4;j++)
//           {
//                cout << std::setw(8) << std::setfill('0') << hex << state_show[i][j];
//           }
//           puts("");
//      }
//      return 0;
// }