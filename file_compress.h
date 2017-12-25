#pragma once  

#include "huffmantree.h"  
#include<algorithm>  
#include<windows.h>  

typedef long long LongType;

struct FileInfo
{
	unsigned char _ch;
	LongType   _count;
	string      _code;

	FileInfo(unsigned char ch = 0)
		:_ch(ch)
		, _count(0)
	{}

	FileInfo operator+(FileInfo& fi)
	{
		FileInfo tmp;
		tmp._count = this->_count + fi._count;
		return tmp;
	}

	bool operator < (FileInfo& fi)
	{
		return this->_count < fi._count;
	}

	bool operator != (const FileInfo& fi)const
	{
		return this->_count != fi._count;
	}

};

template<class T>
class FileCompress
{
public:
	FileCompress()
	{
		for (int i = 0; i < 256; ++i)
		{
			_infos[i]._ch = i;
		}
	}

public:

	bool Compress(const char* filename)
	{
		//1.打开文件，统计文件字符出现的次数  
		long long Charcount = 0;
		assert(filename);
		FILE* fOut = fopen(filename, "rb");
		assert(fOut);

		char ch = fgetc(fOut);

		while (!feof(fOut))
		{
			_infos[(unsigned char)ch]._count++;
			ch = fgetc(fOut);
			Charcount++;
		}

		//2.生成对应的huffman编码  
		GenerateHuffmanCode();

		//3.压缩文件  
		string compressFile = filename;
		compressFile += ".compress";
		FILE* fwCompress = fopen(compressFile.c_str(), "wb");
		assert(fwCompress);

		fseek(fOut, 0, SEEK_SET);
		ch = fgetc(fOut);
		char inch = 0;
		int index = 0;
		while (!feof(fOut))
		{
			string& code = _infos[(unsigned char)ch]._code;
			for (size_t i = 0; i < code.size(); ++i)
			{
				inch = inch << 1;
				if (code[i] == '1')
				{
					inch |= 1;
				}
				if (++index == 8)
				{
					fputc(inch, fwCompress);
					inch = 0;
					index = 0;
				}
			}
			ch = fgetc(fOut);
		}

		if (index)
		{
			inch = inch << (8 - index);
			fputc(inch, fwCompress);
		}

		//4.配置文件，方便后续的解压缩  
		string configFile = filename;
		configFile += ".config";
		FILE *fconfig = fopen(configFile.c_str(), "wb");
		assert(fconfig);

		char CountStr[128];
		_itoa(Charcount >> 32, CountStr, 10);
		fputs(CountStr, fconfig);
		fputc('\n', fconfig);
		_itoa(Charcount & 0xffffffff, CountStr, 10);
		fputs(CountStr, fconfig);
		fputc('\n', fconfig);

		FileInfo invalid;
		for (int i = 0; i < 256; i++)
		{
			if (_infos[i] != invalid)
			{
				fputc(_infos[i]._ch, fconfig);
				fputc(',', fconfig);
				fputc(_infos[i]._count + '0', fconfig);
				fputc('\n', fconfig);
			}
		}

		fclose(fOut);
		fclose(fwCompress);
		fclose(fconfig);

		return true;
	}

	bool UnCompresss(const char* filename)
	{
		string configfile = filename;
		configfile += ".config";
		FILE* outConfig = fopen(configfile.c_str(), "rb");
		assert(outConfig);
		char ch;
		long long Charcount = 0;
		string line = ReadLine(outConfig);
		Charcount = atoi(line.c_str());
		Charcount <<= 32;
		line.clear();
		line = ReadLine(outConfig);
		Charcount += atoi(line.c_str());
		line.clear();

		while (feof(outConfig))
		{
			line = ReadLine(outConfig);
			if (!line.empty())
			{
				ch = line[0];
				_infos[(unsigned char)ch]._count = atoi(line.substr(2).c_str());
				line.clear();
			}
			else
			{
				line = '\n';
			}
		}

		HuffmanTree<FileInfo> ht;
		FileInfo invalid;
		ht.CreatTree(_infos, 256, invalid);

		HuffmanTreeNode<FileInfo>* root = ht.GetRootNode();

		string  UnCompressFile = filename;
		UnCompressFile += ".uncompress";
		FILE* fOut = fopen(UnCompressFile.c_str(), "wb");

		string CompressFile = filename;
		CompressFile += ".compress";
		FILE* fIn = fopen(CompressFile.c_str(), "rb");

		int pos = 8;
		HuffmanTreeNode<FileInfo>* cur = root;
		ch = fgetc(fIn);

		while ((unsigned char)ch != EOF)
		{
			--pos;
			if ((unsigned char)ch &(1 << pos))
			{
				cur = cur->_right;
			}
			else
			{
				cur = cur->_left;
			}
			if (cur->_left == NULL && cur->_right == NULL)
			{
				fputc(cur->_weight._ch, fOut);
				cur = root;
				Charcount--;
			}
			if (pos == 0)
			{
				ch = fgetc(fIn);
				pos = 8;
			}
			if (Charcount == 0)
			{
				break;
			}
		}

		fclose(outConfig);
		fclose(fIn);
		fclose(fOut);
		return true;
	}

protected:

	string ReadLine(FILE* fConfig)
	{
		char ch = fgetc(fConfig);
		if (ch == EOF)
		{
			return "";
		}
		string line;
		while (ch != '\n' && ch != EOF)
		{
			line += ch;
			ch = fgetc(fConfig);
		}
		return line;
	}

	void GenerateHuffmanCode()
	{
		HuffmanTree<FileInfo> hft;
		FileInfo invalid;
		hft.CreatTree(_infos, 256, invalid);
		_GenerateHuffmanCode(hft.GetRootNode());
	}

	void _GenerateHuffmanCode(HuffmanTreeNode<FileInfo>* root)
	{
		if (root == NULL)
		{
			return;
		}

		_GenerateHuffmanCode(root->_left);
		_GenerateHuffmanCode(root->_right);

		if (root->_left == NULL && root->_right == NULL)
		{
			HuffmanTreeNode<FileInfo>* cur = root;
			HuffmanTreeNode<FileInfo>* parent = cur->_parent;
			string& code = _infos[cur->_weight._ch]._code;

			while (parent)
			{
				if (parent->_left == cur)
				{
					code += '0';
				}
				else if (parent->_right == cur)
				{
					code += '1';
				}
				cur = parent;
				parent = cur->_parent;
			}

			reverse(code.begin(), code.end());
		}
	}

private:
	FileInfo _infos[256];
};

void TestFileCompress()
{

	FileCompress<FileInfo> fc;

	int begin1 = GetTickCount();
	fc.Compress("QQ20171029-144442-HD.mp4");
	int end1 = GetTickCount();
	cout << end1 - begin1 << endl;

	int begin2 = GetTickCount();
	fc.UnCompresss("QQ20171029-144442-HD.mp4");
	int end2 = GetTickCount();
	cout << end2 - begin2 << endl;

}