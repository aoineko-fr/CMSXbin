//_____________________________________________________________________________							 
//   ▄▄   ▄ ▄  ▄▄▄ ▄▄ ▄ ▄▄   ▄ 
//  ██ ▀ ██▀█ ▀█▄  ▀█▄▀ ██▄  ▄  ██▀▄
//  ▀█▄▀ ██ █ ▄▄█▀ ██ █ ██▄▀ ██ ██ █
//_____________________________________________________________________________									 
//
// by Guillaume "Aoineko" Blanchard (aoineko@free.fr)
// available on GitHub (https://github.com/aoineko-fr/CMSXbin)
// under CC-BY-AS free license (https://creativecommons.org/licenses/by-sa/2.0/)

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <ctime>
#include <algorithm>
#include <vector>

//-----------------------------------------------------------------------------
// DEFINES

const char* VERSION = "1.1.0";

#define BUFFER_SIZE 1024

/// Data size enum
enum DATA_SIZE
{
	DATA_SIZE_8B,		///< Bytes data
	DATA_SIZE_16B,		///< Words data
	DATA_SIZE_32B,		///< DWords data
};

/// Data format enum
enum DATA_FORMAT
{
	DATA_FORMAT_DEC,	///< Decimal number
	DATA_FORMAT_HEXA,	///< Hexadecimal number (0xFF)
	DATA_FORMAT_BIN,	///< Binary number (0b10101010)
};

// Data language enum (C or Assembler)
enum DATA_LANGUAGE
{
	DATA_LANG_C,		///< C language
	DATA_LANG_ASM,		///< Assembler language (asxxx style)
};

/// Address offset
enum ADDR_OFFSET
{
	ADDR_NONE,			///< Don't display address offset
	ADDR_DEC,			///< Display address offset in decimal format
	ADDR_HEXA,			///< Display address offset in hexadecimal format
};

// integer types
typedef char               i8;  ///<  8 bits integer
typedef short              i16; ///< 16 bits integer
typedef long               i32; ///< 32 bits integer
typedef _int64             i64; ///< 64 bits integer

//-----------------------------------------------------------------------------
// unsigned integer types
typedef unsigned char      u8;  ///<  8 bits unsigned integer
typedef unsigned short     u16; ///< 16 bits unsigned integer
typedef unsigned long      u32; ///< 32 bits unsigned integer
typedef unsigned _int64    u64; ///< 64 bits unsigned integer

//-----------------------------------------------------------------------------
// signed integer types
typedef signed char        s8;	///<  8 bits signed integer
typedef signed short       s16;	///< 16 bits signed integer
typedef signed long        s32;	///< 32 bits signed integer
typedef signed _int64      s64;	///< 64 bits signed integer

//-----------------------------------------------------------------------------
// float types
typedef float              f32; ///< 32 bits floating-point
typedef double             f64; ///< 64 bits floating-point

//-----------------------------------------------------------------------------
// character types
typedef char               c8;	///<  8 bits character
typedef wchar_t            c16;	///< 16 bits character

#define numberof(tab) sizeof(tab) / sizeof(tab[0])

struct SkipData
{
	u32 From;
	u32 Size;
};

//-----------------------------------------------------------------------------
// VARIABLES

std::string				g_InputFile;
std::string				g_OutputFile;
std::string				g_TableName;
DATA_SIZE				g_Size = DATA_SIZE_8B;
DATA_FORMAT				g_Format = DATA_FORMAT_HEXA;
DATA_LANGUAGE			g_Lang = DATA_LANG_C;
i32						g_ValuePerLine = 16; // 0: not limit (only 1 line)
std::vector<SkipData>	g_Skip;
bool					g_ASCII = false;
ADDR_OFFSET				g_Address = ADDR_NONE;

//-----------------------------------------------------------------------------


// Remove the filename extension (if any)
std::string RemoveExt(const std::string& str)
{
	size_t lastdot = str.find_last_of(".");
	if (lastdot == std::string::npos)
		return str;
	return str.substr(0, lastdot);
}

// Remove the filename extension (if any)
std::string RemovePath(const std::string& str)
{
	size_t lastpos;
	size_t last1 = str.find_last_of("\\");
	size_t last2 = str.find_last_of("/");
	if ((last1 == std::string::npos) && (last2 == std::string::npos))
		return str;
	else if (last1 == std::string::npos)
		lastpos = last2;
	else if (last2 == std::string::npos)
		lastpos = last1;
	else
		lastpos = std::max(last1, last2);

	return str.substr(lastpos + 1);
}

// Remode all non-alphanumeric character 
std::string ConvertToAlphaNum(const std::string& str)
{
	std::string ret;
	for (u32 i = 0; i < str.length(); ++i)
	{
		if ((str[i] >= '0' && str[i] <= '9') || (str[i] >= 'a' && str[i] <= 'z') || (str[i] >= 'A' && str[i] <= 'Z'))
			ret += str[i];
		else
			ret += '_';
	}
	return ret;
}

//
const c8* GetNumberFormat()
{
	switch (g_Format)
	{
	case DATA_FORMAT_DEC:
		switch (g_Size)
		{
		default:
		case DATA_SIZE_8B: return "%3u";
		case DATA_SIZE_16B: return "%d";
		case DATA_SIZE_32B:	return "%d";
		};
	case DATA_FORMAT_BIN:
		switch (g_Size)
		{
		default:
		case DATA_SIZE_8B: return "0b%08X";
		case DATA_SIZE_16B: return "0b%016X";
		case DATA_SIZE_32B:	return "0b%032X";
		};
	default:
	case DATA_FORMAT_HEXA:
		switch (g_Size)
		{
		default:
		case DATA_SIZE_8B: return "0x%02X";
		case DATA_SIZE_16B: return "0x%04X";
		case DATA_SIZE_32B:	return "0x%08X";
		};
	}
}

//
void AddComment(std::string& data, const std::string& comment, bool bNewLine = true) 
{
	switch (g_Lang)
	{
	default:
	case DATA_LANG_C: data += "// "; break;
	case DATA_LANG_ASM: data += "; "; break;
	};
	data += comment;
	if (bNewLine)
		data += "\n";
}

//
void StartTable(std::string& data, const std::string& name)
{
	switch (g_Lang)
	{
	default:
	case DATA_LANG_C:
		switch (g_Lang)
		{
		default:
		case DATA_SIZE_8B:	data += "const unsigned char " + name + "[] = {"; break;
		case DATA_SIZE_16B:	data += "const unsigned short " + name + "[] = {"; break;
		case DATA_SIZE_32B: data += "const unsigned long " + name + "[] = {"; break;
		};
		break;
	case DATA_LANG_ASM:		data += name + ":"; break;
	};
}

//
void EndTable(std::string& data)
{
	switch (g_Lang)
	{
	default:
	case DATA_LANG_C: data += "\n};\n"; break;
	case DATA_LANG_ASM: data += "\n"; break;
	};
}

//
void AddNewLine(std::string& data)
{
	data += "\n";
	switch (g_Lang)
	{
	default:
	case DATA_LANG_C:		data += "\t"; break;
	case DATA_LANG_ASM:
		switch (g_Size)
		{
		default:
		case DATA_SIZE_8B:	data += ".db "; break;
		case DATA_SIZE_16B:	data += ".dw "; break;
		case DATA_SIZE_32B:	data += ".dd "; break;
		};
		break;
	};
}

//
void AddByte(std::string& data, u8 value)
{
	char strBuf[BUFFER_SIZE];
	sprintf_s(strBuf, BUFFER_SIZE, GetNumberFormat(), value);
	data += strBuf;
	switch (g_Lang)
	{
	default:
	case DATA_LANG_C:	data += ", "; break;
	case DATA_LANG_ASM:	data += " "; break;
	}
}

//
void AddWord(std::string& data, u16 value)
{
	char strBuf[BUFFER_SIZE];
	sprintf_s(strBuf, BUFFER_SIZE, GetNumberFormat(), value);
	data += strBuf;
	switch (g_Lang)
	{
	default:
	case DATA_LANG_C:	data += ", "; break;
	case DATA_LANG_ASM:	data += " "; break;
	}
}

//
void AddDWord(std::string& data, u32 value)
{
	char strBuf[BUFFER_SIZE];
	sprintf_s(strBuf, BUFFER_SIZE, GetNumberFormat(), value);
	data += strBuf;
	switch (g_Lang)
	{
	default:
	case DATA_LANG_C:	data += ", "; break;
	case DATA_LANG_ASM:	data += " "; break;
	}
}

//
bool Export()
{
	FILE* file;
	std::string strData;
	u8* binData;
	char strBuf[BUFFER_SIZE];
	u8 byte;
	//u16 word;
	//u32 dword;

	// Read binary file
	if (fopen_s(&file, g_InputFile.c_str(), "rb") != 0)
	{
		printf("Error: Fail to open file %s\n", g_InputFile.c_str());
		return false;
	}
	fseek(file, 0, SEEK_END);
	u32 fileSize = ftell(file);
	binData = (u8*)malloc(fileSize);
	fseek(file, 0, SEEK_SET);
	if (fread_s(binData, fileSize, sizeof(u8), fileSize, file) != fileSize)
	{
		free(binData);
		printf("Error: Fail to read file %s\n", g_InputFile.c_str());
		return false;
	}
	fclose(file);
	// Deco
	AddComment(strData, u8"_____________________________________________________________________________");
	AddComment(strData, u8"   ▄▄   ▄ ▄  ▄▄▄ ▄▄ ▄ ▄▄   ▄");
	AddComment(strData, u8"  ██ ▀ ██▀█ ▀█▄  ▀█▄▀ ██▄  ▄  ██▀▄");
	AddComment(strData, u8"  ▀█▄▀ ██ █ ▄▄█▀ ██ █ ██▄▀ ██ ██ █");
	AddComment(strData, u8"_____________________________________________________________________________");
	// License
	sprintf_s(strBuf, BUFFER_SIZE, "CMSXbin %s by Guillaume \"Aoineko\" Blanchard (2021) under CC BY-SA free license", VERSION);
	AddComment(strData, strBuf);
	// Date
	std::time_t result = std::time(nullptr);
	char* ltime = std::asctime(std::localtime(&result));
	ltime[strlen(ltime)-1] = 0; // remove final '\n'
	sprintf_s(strBuf, BUFFER_SIZE, "File generated on %s", ltime);
	AddComment(strData, strBuf);
	// Source
	sprintf_s(strBuf, BUFFER_SIZE, "Soure file: %s\n", g_InputFile.c_str());
	AddComment(strData, strBuf);

	StartTable(strData, g_TableName);
	AddNewLine(strData);

	char strLine[BUFFER_SIZE];
	char* sl = strLine;

	// Parse binary data
	u32 bytes = 0;
	u32 count = 0;
	u32 total = 0;
	u32 offset = 0;
	while(bytes < fileSize)
	{
		bool bSkip = false;
		for (u32 i = 0; i < g_Skip.size(); i++)
		{
			if (bytes == g_Skip[i].From)
			{
				bytes += g_Skip[i].Size;
				bSkip = true;
			}
		}
		if (bSkip)
			continue;

		switch (g_Size)
		{
		default:
		case DATA_SIZE_8B:
			byte = *(u8*)(binData + bytes);
			AddByte(strData, byte);
			if (g_ASCII)
				*sl++ = byte >= 32 ? byte : ' ';
			bytes++;
			total++;
			break;
		case DATA_SIZE_16B:
			AddWord(strData, *(u16*)(binData + bytes));
			bytes += 2;
			total += 2;
			break;
		case DATA_SIZE_32B:
			AddDWord(strData, *(u32*)(binData + bytes));
			bytes += 4;
			total += 4;
			break;
		};

		if ((g_ValuePerLine > 0) && ((((count + 1) % g_ValuePerLine) == 0) || (bytes >= fileSize)))
		{
			if (g_ASCII || (g_Address != ADDR_NONE))
			{
				if(g_ASCII && (g_Size == DATA_SIZE_8B))
				{
					*sl++ = ' ';
					*sl++ = 0;
					sl = strLine;
				}
				if (g_Address != ADDR_NONE)
				{
					if (g_ASCII && (g_Size == DATA_SIZE_8B))
						sprintf_s(strBuf, BUFFER_SIZE, (g_Address == ADDR_HEXA) ? "%08X | %s" : "%6d | %s", offset, strLine);
					else
						sprintf_s(strBuf, BUFFER_SIZE, (g_Address == ADDR_HEXA) ? "%08X" : "%6d", offset);

					strcpy(strLine, strBuf);
					offset = total;
				}
				AddComment(strData, strLine, false);
			}
			AddNewLine(strData);
		}
			
		count++;
	}

	EndTable(strData);
	sprintf_s(strBuf, BUFFER_SIZE, "Total bytes: %d", total);
	AddComment(strData, strBuf);

	// Write header file
	if (fopen_s(&file, g_OutputFile.c_str(), "wb") != 0)
	{
		printf("Error: Fail to create file %s\n", g_OutputFile.c_str());
		return false;
	}
	fwrite(strData.c_str(), 1, strData.size(), file);
	fclose(file);
	return true;
}

//-----------------------------------------------------------------------------
// Display help information
void PrintHelp()
{
	printf("CMSXbin %s - Convert binary to text file\n", VERSION);
	printf("Usage: bin2h <inputfile> [options]\n");
	printf("Options:\n");
	printf("  -o output     Filename of the output file (default: use input filename with .h/.asm extension)\n");
	printf("  -t mane       Data table name (default: use input filename)\n");
	printf("  -8b           8-bits data size (default)\n");
	printf("  -16b          16-bits data size\n");
	printf("  -32b          32-bits data size\n");
	printf("  -dec          Decimal data format\n");
	printf("  -hex          Hexadecimal data format (default)\n");
	printf("  -bin          Binary data format\n");
	printf("  -c            C language data (default)\n");
	printf("  -asm          Assembler language data\n");
	printf("  -n X          Number of data per line (default: 16)\n");
	printf("                (0: means only one line)\n");
	printf("  -skip F S     Skip binary data from 'F' address to a 'S' size\n");
	printf("                Value can be hexadecimal (format: 0xFFFF) or decimal\n");
	printf("                Can be invoked multiple times to skip several sections\n");
	printf("  -ascii        Display ASCII code of each line (only for 8-bits data)\n");
	printf("  -ad           Display decimal address offset of the first element of each line\n");
	printf("  -ax           Display hexadecimal address offset of the first element of each line\n");
	printf("  -h            Display this help\n");
}

//const char* ARGV[] = { "", "testcases/pt3/Beg!nsum.pt3", "-n", "16", "-skip", "0", "100", "-ascii", "-ax" };
//#define DEBUG_ARGS

//-----------------------------------------------------------------------------
// MAIN LOOP 
int main(int argc, const char* argv[])
{
#ifdef DEBUG_ARGS
	argc = sizeof(ARGV) / sizeof(ARGV[0]); argv = ARGV;
#endif

	// Error check
	if (argc < 2)
	{
		printf("Error: No enough parameters!\n");
		PrintHelp();
		return 1;
	}

	g_InputFile = argv[1];

	// Parse command line parameters
	for (i32 i = 2; i < argc; ++i)
	{
		// Display help
		if (_stricmp(argv[i], "-h") == 0)
		{
			PrintHelp();
			return 0;
		}
		// Output filename
		else if (_stricmp(argv[i], "-o") == 0)
			g_OutputFile = argv[++i];
		// Data table name
		else if (_stricmp(argv[i], "-t") == 0)
		{
			g_TableName = ConvertToAlphaNum(argv[++i]);
			if ((g_TableName[0] >= '0') && (g_TableName[0] <= '9'))
				g_TableName = "g_" + g_TableName;
		}
		// Data size
		else if (_stricmp(argv[i], "-8b") == 0)
			g_Size = DATA_SIZE_8B;
		else if (_stricmp(argv[i], "-16b") == 0)
			g_Size = DATA_SIZE_16B;
		else if (_stricmp(argv[i], "-32b") == 0)
			g_Size = DATA_SIZE_32B;
		// Data format
		else if (_stricmp(argv[i], "-dec") == 0)
			g_Format = DATA_FORMAT_DEC;
		else if (_stricmp(argv[i], "-hex") == 0)
			g_Format = DATA_FORMAT_HEXA;
		else if (_stricmp(argv[i], "-bin") == 0)
			g_Format = DATA_FORMAT_BIN;
		// Data language
		else if (_stricmp(argv[i], "-c") == 0)
			g_Lang = DATA_LANG_C;
		else if (_stricmp(argv[i], "-asm") == 0)
			g_Lang = DATA_LANG_ASM;
		// Value per line
		else if (_stricmp(argv[i], "-n") == 0)
			g_ValuePerLine = atoi(argv[++i]);
		// Display ASCII
		else if (_stricmp(argv[i], "-ascii") == 0)
			g_ASCII = true;
		// Display address
		else if (_stricmp(argv[i], "-ad") == 0)
			g_Address = ADDR_DEC;
		else if (_stricmp(argv[i], "-ax") == 0)
			g_Address = ADDR_HEXA;
			// Skip area
		else if (_stricmp(argv[i], "-skip") == 0)
		{
			SkipData s;
			i++;
			if ((argv[i][0] == '0') && (argv[i][1] == 'x'))	// hexadecimal
				s.From = strtol(argv[i], NULL, 16);
			else											// decimal
				s.From = atoi(argv[i]);
			i++;
			if ((argv[i][0] == '0') && (argv[i][1] == 'x'))	// hexadecimal
				s.Size = strtol(argv[i], NULL, 16);
			else											// decimal
				s.Size = atoi(argv[i]);
			g_Skip.push_back(s);
		}
	}

	// Validate parameters
	if (g_OutputFile.empty())
	{
		g_OutputFile = RemoveExt(g_InputFile);
		switch (g_Lang)
		{
		default:
		case DATA_LANG_C: g_OutputFile += ".h"; break;
		case DATA_LANG_ASM: g_OutputFile += ".asm"; break;
		}
	}
	if (g_TableName.empty())
	{
		g_TableName = RemovePath(g_InputFile);
		g_TableName = RemoveExt(g_TableName);
		g_TableName = "g_" + ConvertToAlphaNum(g_TableName);
	}

	Export();
}