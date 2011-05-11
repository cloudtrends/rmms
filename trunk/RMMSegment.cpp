//============================================================================
// Name        : RMMSegment.cpp
// Author      : xiaodong
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <fstream>
#include <map>
#include <cstring>
#include <string>
#include <cstdlib>
#include <vector>

using namespace std;
//最长句子长度
#define MAX_SENTENCE_LEN 256
//最大词组长度
#define MAX_SEGMENT_LEN 5

//字典结构
typedef map<string, bool>  DICT;

//判断一个utf-8是否是数字
inline bool  IsNumber(char c)
{
		if(c >= 0x30 && c <= 0x39)
			return true;
		else
            return false;
}

//判断一个uft-8是否是英文字母
inline bool IsAlpha(char c)
{
        if ((c >= 0x41 && c <= 0x5a) || ( c >= 0x61 && c <= 0x7a))
    		return true;
        else
        	return false;
}

//获取utf8字符串的长度，暂时没有用
int UtfLength(const char *src)
{
	const char *p = src;
	int lenght = 0;
	if(p)
	{
		int utf_len = 0;
		while(*p !='\0')
		{
			if      ((*p & 0xFC) == 0xFC) utf_len = 6; // 6 bytes length
		    else if ((*p & 0xF8) == 0xF8) utf_len = 5; // 5 bytes length
		    else if ((*p & 0xF0) == 0xF0) utf_len = 4; // 4 bytes length
		    else if ((*p & 0xE0) == 0xE0) utf_len = 3; // 3 bytes length
		    else if ((*p & 0xC0) == 0xC0) utf_len = 2; // 2 bytes length
		    else utf_len = 1;                          // 1 bytes length

			lenght++;
			p += utf_len;
		}
	}
	return lenght;
}

//获取目前src指向的utf8的字符长度
inline short UtfCharLength(const char *src)
{
	const char *p = src;
	short utf_len = 0;
	if(p)
	{
			if      ((*p & 0xFC) == 0xFC) utf_len = 6; // 6 bytes length
		    else if ((*p & 0xF8) == 0xF8) utf_len = 5; // 5 bytes length
		    else if ((*p & 0xF0) == 0xF0) utf_len = 4; // 4 bytes length
		    else if ((*p & 0xE0) == 0xE0) utf_len = 3; // 3 bytes length
		    else if ((*p & 0xC0) == 0xC0) utf_len = 2; // 2 bytes length
		    else utf_len = 1;                          // 1 bytes length
	}
	return utf_len;
}

//UTF转换成UNICODE，输入一个UTF8的字符串，输出一个short(16bit的数组)。
//从原理上看，UNICODE字符应要用32位整形表示，但是对于中日韩文字，UTF8
//长度最多4位，因此short表示UNICODE就够了
int Utf2Uni(const char *src,unsigned short *dst, unsigned int *dlen)
{
	const char *p = src;
	unsigned short *d = dst;
	int length = 0;
	unsigned short dst_code = 0;
	int utf_len = 0;
	unsigned char mask = 0xFF;

	if(p)
	{
		while(*p !='\0')
		{
			if      ((*p & 0xFC) == 0xFC) utf_len = 6; // 6 bytes length
		    else if ((*p & 0xF8) == 0xF8) utf_len = 5; // 5 bytes length
		    else if ((*p & 0xF0) == 0xF0) utf_len = 4; // 4 bytes length
		    else if ((*p & 0xE0) == 0xE0) utf_len = 3; // 3 bytes length
		    else if ((*p & 0xC0) == 0xC0) utf_len = 2; // 2 bytes length
		    else utf_len = 1;                          // 1 bytes length

			//decode
			mask = 0xFF;
			mask = mask >> utf_len;
			dst_code = *p++ & mask;
			while( --utf_len != 0)
			{
				dst_code = dst_code << 6;
				dst_code = dst_code | (*p++ & 0x3F) ;
			}

			*d++ = dst_code;
			length++;
		}

	}
	*dlen = length;
	return length;
}

//UNICODE转换成UTF8，采用顺序存放，输入一个short数组和数组的长度，返回一个UTF8的字符串
//以下是UTF8的6种类型，靠最前面一位来识别
//	U+00000000 - U+0000007F: 	0 xxxxxxx 	0x - 7x
//	U+00000080 - U+000007FF: 	110 xxxxx 10 xxxxxx 	Cx 8x - Dx Bx
//	U+00000800 - U+0000FFFF: 	1110 xxxx 10 xxxxxx 10 xxxxxx 	Ex 8x 8x - Ex Bx Bx
//	U+00010000 - U+001FFFFF: 	11110 xxx 10 xxxxxx 10 xxxxxx 10 xxxxxx 	F0 8x 8x 8x - F7 Bx Bx Bx 	很少用
//	U+00200000 - U+03FFFFFF: 	111110 xx 10 xxxxxx 10 xxxxxx 10 xxxxxx 10 xxxxxx 	F8 8x 8x 8x 8x - FB Bx Bx Bx Bx
//	U+04000000 - U+7FFFFFFF: 	1111110 x 10 xxxxxx 10 xxxxxx 10 xxxxxx 10 xxxxxx 10 xxxxxx 	FC 8x 8x 8x 8x 8x - FD Bx Bx Bx Bx Bx
char* Uni2Utf(const unsigned short *src, unsigned int slen, char *dst )
{
	const unsigned short *p = src;
	char *d = dst;
	unsigned short unicode = 0;
	int utf_len = 0;

	while(slen--)
	{
		unicode = *p++;
		if (unicode < 0x80) {
			*d++ = char(unicode);
			utf_len = 1;
		}else if (unicode < 0x800) {
            *d++ = ((unicode >> 6) & 0x1f)|0xc0;
			utf_len = 2;
		}else if (unicode < 0x10000) {
            *d++ = ((unicode >> 12) & 0x0f)|0xe0;
			utf_len = 3;
		}else if (unicode < 0x200000) {
            *d++ = ((unicode >> 18) & 0x07)|0xf0;
			utf_len = 4;
		}else if (unicode < 0x4000000) {
            *d++ = ((unicode >> 24) & 0x03)|0xf8 ;
			utf_len = 5;
		}else {
            *d++ = ((unicode >> 30) & 0x01)|0xfc;
			utf_len = 6;
		}

		while(--utf_len) *d++ = char(((unicode >>((utf_len-1)*6) ) & 0x3f)|0x80);
	}
	d = '\0';
	return dst;
}

class CRMMSegment
{
	public:
		CRMMSegment()
		{
			m_iCount = 0;
			cout << "Init RMMSegment..." << endl;

		}

		//加载字典
		void LoadDict(char *filename)
		{
			cout << "load dict, filename:" << filename  << endl;
			ifstream fin(filename);
		
			while(! fin.eof())
			{
				getline(fin, m_sBuffer);
				cout << "=====" << m_sBuffer << endl;

				m_mDict.insert(pair<string, bool>(m_sBuffer, true));
			}

			cout << "Dict Item Number: " << m_mDict.size() <<",  Memory Size: "<< sizeof(m_mDict) << endl;
			fin.close();
		}

		//查找字符串
		bool FindKey(string key)
		{
			DICT::iterator it = m_mDict.find(key);
			if (it == m_mDict.end() )
				return false;
			else
				return true;
		}

		//分词主程序
		void Segment(const char *src, vector<string> &dst){
			const char *p = src;
			int code = 0;
			int newCode = 0;
			const char *start = p;
			string tmpStr;
			vector<string> tmpVec;

			while(1){
				int len = UtfCharLength(p);

				//中文code:0, 英文code:1, 数字code:2
				if( len == 1){
					newCode = 0;
					if ( IsAlpha(*p)) 	newCode = 1;
					if( IsNumber(*p))   newCode = 2;
				}else{
					newCode = 0;
				}

				if(newCode != code || *p == '\0'){
					tmpStr = string(start, p);
					if(code == 0){
						SegmentCN(tmpStr.c_str(), tmpVec);
						for(vector<string>::reverse_iterator it = tmpVec.rbegin(); it != tmpVec.rend(); it++){
                                                    dst.push_back(*it);
						}
					}else{
						dst.push_back(tmpStr);
					}
					if( *p == '\0') break;

					code = newCode;
					start = p;
				}
				p = p + len;
			}
		}

		//对中文进行逆向最大匹配分词
		void SegmentCN(const char *src, vector<string> &dst)
		{
			unsigned int uLength = 0;
			int curPos = 0;
			int curLength = 0;
			unsigned short uBuffer[MAX_SENTENCE_LEN];
			char cBuffer[MAX_SEGMENT_LEN*4+1];
			unsigned short segmentBuffer[MAX_SEGMENT_LEN*2];

			memset(uBuffer, 0, MAX_SENTENCE_LEN*2);
			Utf2Uni(src, uBuffer, &uLength);
			curPos = uLength;
			while(uLength >0)
			{
				curLength = MAX_SEGMENT_LEN;
				//判断是否越界
				int start = curPos-curLength;
				if(start< 0)
				{
					start = 0;
					curLength = curPos;
				}

				while(curLength > 0)
				{
					//拷贝内存到segment buffer
					memcpy((char *)segmentBuffer, (char *)(uBuffer+ start), curLength*2);
					memset(cBuffer, 0, MAX_SEGMENT_LEN*4+1);
					Uni2Utf(segmentBuffer, curLength, cBuffer);

					if( FindKey(string(cBuffer)) )
						break;
					else{
						curLength--;
						start++;
					}
				}

				//没有查找到相关结果
				if(curLength == 0)
					curLength = 1;
				curPos = curPos - curLength;
				uLength = uLength - curLength;
				dst.push_back( string(cBuffer));
			}
		}

	private:
		int m_iCount;
		string m_sBuffer;
		DICT m_mDict;


};

int main()
{
	vector<string> dst;
	CRMMSegment segment;
	segment.LoadDict("D:/workspace/RMMSegment/src/dict.txt");
	segment.Segment(string("中国杭州的一只猪，哈哈，good123,你好").c_str(), dst);
	for(vector<string>:: iterator it = dst.begin(); it != dst.end(); it++)
		cout << *it << "/";
	cout << endl;
	cout << "!!!end!!!" << endl; // prints !!!Hello World!!!
	return 0;
}
