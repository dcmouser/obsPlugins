#include "jrhelpers.hpp"

#include <ctime>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <ctype.h>




//---------------------------------------------------------------------------
// see https://www.techiedelight.com/split-a-string-on-newlines-in-cpp/
std::vector<std::string> splitString(const std::string& str, bool skipBlankLines)
{
    std::vector<std::string> tokens;
    std::string astr;
 
    std::string::size_type pos = 0;
    std::string::size_type prev = 0;
    while ((pos = str.find('\n', prev)) != std::string::npos) {
	    astr = str.substr(prev, pos - prev);
	    if (astr != "" || !skipBlankLines) {
		    tokens.push_back(astr);
	    }
        prev = pos + 1;
    }
    astr = str.substr(prev);
    if (astr != "" || !skipBlankLines) {
	    tokens.push_back(astr);
    }
 
    return tokens;
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
std::string calcSecsAsNiceTimeString(unsigned long secs, bool flagPadLeadingZeros) {
	unsigned long mins = (unsigned long) (secs / 60L);
	secs = secs % 60;
	unsigned long hours = (unsigned long) (mins / 60L);
	mins = mins % 60;
	//
	char str[24];
	if (hours > 0) {
		if (flagPadLeadingZeros) {
			sprintf(str, "%02lu:%02lu:%02lu", hours, mins, secs);
		} else {
			sprintf(str, "%lu:%02lu:%02lu", hours, mins, secs);

		}
	} else {
		if (flagPadLeadingZeros) {
			sprintf(str, "%02lu:%02lu", mins, secs);
		} else {
			sprintf(str, "%lu:%02lu", mins, secs);
		}
	}
	//
	return std::string(str);
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
std::string getCurrentDateTimeAsNiceString() {
	char timestr[80];
	time_t temp;
	struct tm *timeptr;
	temp = time(NULL);
	timeptr = localtime(&temp);
	strftime(timestr, 80, "%A, %d %b %Y at %I:%M %p", timeptr);
	return std::string(timestr);
}
//---------------------------------------------------------------------------







//---------------------------------------------------------------------------
bool doesStringMatchAnyItemsInPatternList(const std::string needle, std::vector<std::string> *haystackVectorp) {
	const char* needleCharp = needle.c_str();
	for (std::vector<std::string>::iterator t = haystackVectorp->begin(); t != haystackVectorp->end(); ++t) {
		const char* patternCharp = t->c_str();
		if (strcmp(patternCharp, "") == 0) {
			continue;
		}
		if (strstr(needleCharp, patternCharp) != NULL) {
			return true;
		}
	}
	// not found
	return false;
}
//---------------------------------------------------------------------------




































//---------------------------------------------------------------------------
// from https://stackoverflow.com/questions/122616/how-do-i-trim-leading-trailing-whitespace-in-a-standard-way
char* jrtrim(char *str)
{
    size_t len = 0;
    char *frontp = str;
    char *endp = NULL;

    if( str == NULL ) { return NULL; }
    if( str[0] == '\0' ) { return str; }

    len = strlen(str);
    endp = str + len;

    /* Move the front and back pointers to address the first non-whitespace
     * characters from each end.
     */
    while( isspace((unsigned char) *frontp) ) { ++frontp; }
    if( endp != frontp )
    {
        while( isspace((unsigned char) *(--endp)) && endp != frontp ) {}
    }

    if( frontp != str && endp == frontp )
            *str = '\0';
    else if( str + len - 1 != endp )
            *(endp + 1) = '\0';

    /* Shift the string so that it starts at str so that if it's dynamically
     * allocated, we can still free it on the returned pointer.  Note the reuse
     * of endp to mean the front of the string buffer now.
     */
    endp = str;
    if( frontp != str )
    {
            while( *frontp ) { *endp++ = *frontp++; }
            *endp = '\0';
    }

    return str;
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
// ATTN: note no bounds checking be careful
bool jrLeftCharpSplit(char* str, char* leftPart, char separator) {
    if (str[0] == '\0') {
        strcpy(leftPart, "");
        return false;
    }
    char* charp = strchr(str, separator);
    if (!charp) {
        // last part
        strcpy(leftPart, str);
        strcpy(str, "");
    }
    else {
        strncpy(leftPart, str, charp - str);
        leftPart[charp - str] = '\0';
        strcpy(str, charp + 1);
    }
        return true;
}
//---------------------------------------------------------------------------










//---------------------------------------------------------------------------
bool jrParseCommaStringVector(std::string commaSeparatedFloats, std::vector<float> &vec) {
	bool success = true;
	vec.clear();
	std::string onevalstr;
	while (JR_SplitStringLeft(commaSeparatedFloats, onevalstr, ',')) {
		try {
			float val = std::stof(onevalstr);
			vec.push_back(val);
		}
		catch (...) {
			// just ignore
			success = false;
		}

	}
	return success;
}
//---------------------------------------------------------------------------






//---------------------------------------------------------------------------
bool JR_SplitStringLeft(std::string &mainstring,std::string &leftpart,char dividerchar)
{
	// split left part of a string off
	std::string tempstr;

	int len = (int)(mainstring.length());
	if (len==0)
		{
		// nothing to grab so return false
		leftpart="";
		return false;
		}

	const char *charp=mainstring.c_str();
	bool inquote=false;
	char c;

	// find 
	for (int count=0;count<len;++count)
		{
		c=charp[count];
		if (c==34 && inquote==false)
			inquote=true;
		else if (c==34 && inquote==true)
			inquote=false;
		else if (c==dividerchar && inquote==false)
			{
			// found it, grab left part
			if (count==0)
				leftpart="";
			else
				{
				leftpart=mainstring.substr(0,count);
				// ATTN: do we need to trim it?
				JR_trim_string(leftpart);
				}
			// now grab remainder
			if (count<len-1)
				{
				tempstr=mainstring.substr(count+1,len-(count+1));
				mainstring=tempstr;
				// ATTN: do we need to trim it?
				JR_trim_string(mainstring);
				}
			else
				mainstring="";
			return true;
			}
		}

	// not found, so its all left part
	leftpart=mainstring;
	JR_trim_string(leftpart);
	mainstring="";
	return true;
}




bool JR_SplitStringRight(std::string &mainstring,std::string &rightpart,char dividerchar)
{
	// split left part of a string off

	int len = (int)(mainstring.length());
	if (len==0)
		{
		// nothing to grab so return false
		rightpart="";
		return false;
		}

	const char *charp=mainstring.c_str();
	bool inquote=false;
	char c;

	// find 
	for (int count=len-1;count>=0;--count)
		{
		c=charp[count];
		if (c==34 && inquote==false)
			inquote=true;
		else if (c==34 && inquote==true)
			inquote=false;
		else if (c==dividerchar && inquote==false)
			{
			// found it, grab right part
			if (count==len-1)
				rightpart="";
			else
				{
				rightpart=mainstring.substr(count+1, (len-(count+1)) );
				// ATTN: do we need to trim it?
				JR_trim_string(rightpart);
				}
			// now grab remainder
			if (count>0)
				{
				mainstring=mainstring.substr(0,count);
				// ATTN: do we need to trim it?
				JR_trim_string(mainstring);
				}
			else
				mainstring="";
			return true;
			}
		}

	// not found, so its all left part
	rightpart=mainstring;
	JR_trim_string(rightpart);
	mainstring="";
	return true;
}




void JR_trim_string(std::string &s)
{ 
	s.erase(0,s.find_first_not_of(" \t\r\n"));
	s.erase(s.find_last_not_of(" \t\r\n")+1);
} 
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
std::string jrGetDirPathFromFilePath(std::string filepath) {
	std::string dummystr;
	JR_SplitStringRight(filepath, dummystr, '/');
	return filepath;
}
//---------------------------------------------------------------------------
