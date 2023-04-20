#pragma once

#include <string>
#include <vector>



//---------------------------------------------------------------------------
std::vector<std::string> splitString(const std::string& str, bool skipBlankLines);

std::string calcSecsAsNiceTimeString(unsigned long secs, bool flagPadLeadingZeros);
std::string getCurrentDateTimeAsNiceString();

bool doesStringMatchAnyItemsInPatternList(const std::string needle, std::vector<std::string>* haystackVectorp);
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
char* jrtrim(char* str);

// fast defined inlines
#define jrmax4(a,b,c,d) max(max(a,b),max(c,d))
#define jrPointDist(x1,y1,x2,y2) sqrt(pow(x1-x2,2)+pow(y1-y2,2))
#define jrRectDist(ax1,ay1,ax2,ay2, bx1,by1,bx2,by2) (fabs(ax1 - bx1) + fabs(ay1 - by1) + fabs(ax2 - bx2) + fabs(ay2 - by2))

#define mydebugbeep() Beep(750, 25)

bool jrLeftCharpSplit(char* str, char* leftPart, char separator);
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
bool jrParseCommaStringVector(std::string commaSeparatedFloats, std::vector<float>& vec);
bool JR_SplitStringLeft(std::string &mainstring,std::string &leftpart,char dividerchar);
void JR_trim_string(std::string& s);
//---------------------------------------------------------------------------
