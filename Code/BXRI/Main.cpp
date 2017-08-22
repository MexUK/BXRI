#include "Main.h"
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <Windows.h>
#include "Static/CFile.h"
#include "Static/CStdVector.h"
#include "Static/CString2.h"
#include "Static/CPath.h"

using namespace std;
using namespace bxcf;

int main(void)
{
	detectRedundantIncludes();

	system("PAUSE");

	return 0;
}

/*

to find redundant #include files:
---------------------------------

for all .h and .cpp files:

for all #include lines in the file:

if "blah" from #include "some/path/blah.h" is not found in the .h file (or maybe .cpp file too)

then the #include line is not needed

*/

void detectRedundantIncludes(void)
{
	cout << "\nBXRI - BX Redundant Include Checker - by Mex\n\n";
	cout << "Enter a folder path:\n\n";

	char szFolderPathIn[MAX_PATH];
	for (;;)
	{
		cin.getline(szFolderPathIn, sizeof(szFolderPathIn));

		if (CPath::isValidPath(string(szFolderPathIn)))
		{
			break;
		}
		else
		{
			cout << "Invalid folder path, try again.\n\n";
			return;
		}
	}

	vector<string>
		vecFilePaths = CFile::getFilePaths(szFolderPathIn, true, false, "h,cpp", true);
	map<string, vector<string>>
		mapRedundantIncludes;

	for (string& strFilePath : vecFilePaths)
	{
		string strFileContent = CFile::getFileContent(strFilePath);
		vector<string> vecFileLines = CString2::split(strFileContent, "\n");
		for (string& strFileLine : vecFileLines)
		{
			string strFileLineClean = CString2::ltrim(strFileLine);
			if (strFileLineClean.substr(0, 8) == "#include")
			{
				int32 iQuoteStartIndex, iQuoteEndIndex;
				iQuoteStartIndex = strFileLineClean.find('"', 0);
				if (iQuoteStartIndex == -1)
				{
					iQuoteStartIndex = strFileLineClean.find('<', 0);
					iQuoteEndIndex = strFileLineClean.find('>', iQuoteStartIndex + 1);
				}
				else
				{
					iQuoteEndIndex = strFileLineClean.find('"', iQuoteStartIndex + 1);
				}

				string strIncludePath = strFileLineClean.substr(iQuoteStartIndex + 1, (iQuoteEndIndex - iQuoteStartIndex) - 1);
				if (strIncludePath == "")
				{
					continue;
				}

				string strFileName = CPath::getFileName(strIncludePath);
				if (strFileName == "")
				{
					continue;
				}

				string strFileNameNoExt = CPath::removeFileExtension(strFileName);
				if (strFileNameNoExt == "")
				{
					continue;
				}

				bool
					bFirstFileIsHeaderFile = CString2::toUpperCase(CPath::getFileExtension(strFilePath)) == "H",
					bSecondFileIsHeaderFile = !bFirstFileIsHeaderFile;
				uint32
					uiSearchStartIndex = 0;
				bool
					bFoundInAFilePair = false;

				// file 1
				if (bFirstFileIsHeaderFile)
				{
					uiSearchStartIndex = strFileContent.rfind("#include");
					if (uiSearchStartIndex != -1)
					{
						uiSearchStartIndex = strFileContent.find("\n", uiSearchStartIndex + 1);
						if (uiSearchStartIndex != -1)
						{
							uiSearchStartIndex++;
						}
					}
				}

				if (CString2::isIn(strFileContent, strFileNameNoExt, true, uiSearchStartIndex))
				{
					bFoundInAFilePair = true;
				}

				// file 2
				uiSearchStartIndex = 0;
				string
					strSecondFileExtension = bFirstFileIsHeaderFile ? "cpp" : "h",
					strSecondFilePath = CPath::replaceFileExtensionWithCase(strFilePath, strSecondFileExtension),
					strSecondFileContent = CFile::getFileContent(strSecondFilePath);

				if (bSecondFileIsHeaderFile)
				{
					uiSearchStartIndex = strSecondFileContent.rfind("#include");
					if (uiSearchStartIndex != -1)
					{
						uiSearchStartIndex = strSecondFileContent.find("\n", uiSearchStartIndex + 1);
						if (uiSearchStartIndex != -1)
						{
							uiSearchStartIndex++;
						}
					}
				}

				if (CString2::isIn(strSecondFileContent, strFileNameNoExt, true, uiSearchStartIndex))
				{
					bFoundInAFilePair = true;
				}

				if (!bFoundInAFilePair)
				{
					mapRedundantIncludes[strFilePath].push_back(strFileLineClean);
				}
			}
		}
	}

	cout << "Redundant Include Files:\n\n";
	for (auto& it : mapRedundantIncludes)
	{
		cout << it.first.c_str() << "\n\n";
		for (string& strIncludeLine : it.second)
		{
			cout << strIncludeLine.c_str() << "\n";
		}
		cout << "\n\n\n\n";
	}
	cout << "Done.\n\n";
}