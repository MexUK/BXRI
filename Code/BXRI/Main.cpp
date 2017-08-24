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

if "blah" from #include "some/path/blah.h" is not found in the .h file or .cpp file

then the #include line is marked as not needed

*/

void detectRedundantIncludes(void)
{
	map<string, bool> mapSkipIncludes;
	bool bSkipAngleBracketIncludes;

	// -------------- settings start --------------

	bSkipAngleBracketIncludes = true; // e.g. Skip <Windows.h> instead of "Windows.h"
	mapSkipIncludes["Type/Types.h"] = true;

	// --------------- settings end ---------------

	cout << "\n";

	cout << "-------------------------- BXRI --------------------------\n";
	cout << "-------------------------- BX Redundant Include Checker --\n";
	cout << "-------------------------- by Mex ------------------------\n\n";

	cout << "This tool checks for:\n";
	cout << "- Redundant Includes\n";
	cout << "- Duplicate Includes\n\n";

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
	cout << "\n\n";

	vector<string>
		vecFilePaths = CFile::getFilePaths(szFolderPathIn, true, false, "h,hpp,c,cpp", true);
	map<string, vector<string>>
		mapRedundantIncludes,
		mapDuplicateIncludes;
	map<string, vector<string>>
		mapOppositeExtensions;
	string
		strFilePath2;

	mapOppositeExtensions["C"] = { "H", "HPP" };
	mapOppositeExtensions["CPP"] = { "HPP", "H" };
	mapOppositeExtensions["H"] = { "C", "CPP" };
	mapOppositeExtensions["HPP"] = { "CPP", "C" };

	for (string& strFilePath : vecFilePaths)
	{
		string strFileContent = CFile::getFileContent(strFilePath);
		vector<string> vecFileLines = CString2::split(strFileContent, "\n");
		for (string& strFileLine : vecFileLines)
		{
			string strFileLineClean = CString2::trim(strFileLine);
			if (strFileLineClean.substr(0, 8) == "#include")
			{
				int32 iQuoteStartIndex, iQuoteEndIndex;
				iQuoteStartIndex = strFileLineClean.find('"', 0);
				if (iQuoteStartIndex == -1)
				{
					iQuoteStartIndex = strFileLineClean.find('<', 0);
					if (iQuoteStartIndex != -1)
					{
						if (bSkipAngleBracketIncludes)
						{
							continue;
						}
						iQuoteEndIndex = strFileLineClean.find('>', iQuoteStartIndex + 1);
					}
				}
				else
				{
					iQuoteEndIndex = strFileLineClean.find('"', iQuoteStartIndex + 1);
				}

				string strIncludeLineClean = strFileLineClean.substr(0, iQuoteEndIndex);
				if (strIncludeLineClean == "")
				{
					continue;
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

				if (mapSkipIncludes.count(strIncludePath) == 1 && mapSkipIncludes[strIncludePath] == true)
				{
					continue;
				}


				string
					strExtUpper = CString2::toUpperCase(CPath::getFileExtension(strFilePath));
				bool
					bFileIsHeaderFile = strExtUpper == "H" || strExtUpper == "HPP",
					bIncludeIsUsed = false,
					bFile1IsHeaderFile = bFileIsHeaderFile;
				uint32
					uiFileHitCount = 0,
					uiHitCount = 0;
				map<uint32, bool>
					mapFilesHitFound;
				
				mapFilesHitFound[0] = false;
				mapFilesHitFound[1] = false;

				for (uint32 i = 0; i < 2; i++)
				{
					// header or source file
					uint32 uiSearchStartIndex = 0;

					if (bFileIsHeaderFile)
					{
						// header file
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
						bIncludeIsUsed = true;
					}

					uint32 uiHitCountInFile = CString2::getHitCount(strFileContent, strIncludeLineClean, false, 0);
					if (uiHitCountInFile > 0)
					{
						uiFileHitCount++;
						uiHitCount += uiHitCountInFile;
						mapFilesHitFound[i] = true;
					}

					strFilePath2 = CPath::replaceFileExtensionWithCase(strFilePath, mapOppositeExtensions[strExtUpper][0]);
					if (CFile::doesFileExist(strFilePath2))
					{
						strFilePath = strFilePath2;
					}
					else
					{
						strFilePath2 = CPath::replaceFileExtensionWithCase(strFilePath, mapOppositeExtensions[strExtUpper][1]);
						if (CFile::doesFileExist(strFilePath2))
						{
							strFilePath = strFilePath2;
						}
						else
						{
							break;
						}
					}

					strFileContent = CFile::getFileContent(strFilePath);
					strExtUpper = CString2::toUpperCase(CPath::getFileExtension(strFilePath));
					bFileIsHeaderFile = strExtUpper == "H" || strExtUpper == "HPP";
				}

				if (!bIncludeIsUsed)
				{
					mapRedundantIncludes[strFilePath].push_back(strFileLineClean);
				}

				if (uiHitCount > 1)
				{
					string strFoundInFilesText;
					if (mapFilesHitFound[0] && mapFilesHitFound[1])
						strFoundInFilesText = "header and source files";
					else if (mapFilesHitFound[0] && bFile1IsHeaderFile)
						strFoundInFilesText = "header file";
					else if (mapFilesHitFound[1] && !bFile1IsHeaderFile)
						strFoundInFilesText = "header file";
					else
						strFoundInFilesText = "source file";

					if (mapDuplicateIncludes.count(strFilePath) == 0)
					{
						mapDuplicateIncludes[strFilePath].push_back(strFileLineClean + " [" + CString2::toString(uiHitCount) + " hits, in " + strFoundInFilesText + "]");
					}
				}
			}
		}
	}


	cout << "------------------------\n";
	cout << "Redundant Include Files:\n";
	cout << "------------------------\n\n";
	if (mapRedundantIncludes.size() == 0)
	{
		cout << "None found!" << "\n\n";
	}
	for (auto& it : mapRedundantIncludes)
	{
		cout << it.first.c_str() << "\n\n";
		for (string& strIncludeLine : it.second)
		{
			cout << strIncludeLine.c_str() << "\n";
		}
		cout << "\n\n\n\n";
	}


	cout << "-----------------------\n";
	cout << "Duplicate Include Files:\n";
	cout << "-----------------------\n\n";
	if (mapDuplicateIncludes.size() == 0)
	{
		cout << "None found!" << "\n\n";
	}
	for (auto& it : mapDuplicateIncludes)
	{
		cout << it.first.c_str() << "\n\n";
		for (string& strIncludeLine : it.second)
		{
			cout << strIncludeLine.c_str() << "\n";
		}
		cout << "\n\n\n\n";
	}

	cout << "-----\n";
	cout << "Done.\n";
	cout << "-----\n\n";
}