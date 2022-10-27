#!/usr/bin/env python3
#	-*-	coding:	UTF-8	-*-
"""
v2.3 - 7/6/22	-	mouser@donationcoder.com
this tool	scans	infoWriter text	output and writes	all	events that	happen after a break or	on a hotkey	press
it uses	the	FIRST	hotkey it	finds	before a break to	set	the	starting offset	of the stream
OR you can manually	set	the	starting stream	offset time	by simply	adding a HOTKEY:HOTKEY GOLIVE	entry	with the real	Stream Time	Marker time
ALSO you can modify	whatever the calculated	timestamps are by	adding a line: "OFFSET 00:00:01" or	"OFFSET	-00:00:01" to	add	or subtract	a	second

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Usage: iwscan	<src>
"""
import sys
import os
import re
import datetime


# options
option_ignoreZero = True
optionStripLeadingHoursMinutes = True


def	main():
		#	Check	and	retrieve command-line	arguments
		if len(sys.argv) !=	3:
				print(__doc__)
				sys.exit(1)		#	Return a non-zero	value	to indicate	abnormal termination

		#	filename first param
		fileIn	=	sys.argv[1]

		#	form output	filename
		fileOut	=	fileIn.replace(".txt","_out.txt");
		if (fileOut	== fileIn):
			fileOut	=	fileIn + "_out.txt";
		
		#	next parameter is	offset in	form of	+-#
		offsetStr	=	sys.argv[2]
		offsetVal	=	int(offsetStr)

		#	Verify source	file
		if not os.path.isfile(fileIn):
				print("error:	{} does	not	exist".format(fileIn))
				sys.exit(1)
				
		linesWritten = doProcessFile(fileIn, fileOut,	offsetVal)

		print("File	proccessed,	timestamped	modified by:{}\n".format(offsetVal))



def	modifyTimestamp(timeStampString, offsetSeconds):
	#	convert	timestamp	match	to a string
	timeStampSecs	=	convertTimeStampToSeconds(timeStampString)
	if (option_ignoreZero and (timeStampSecs == 0)):
		# dont change it
		return timeStampString;
	timeStampSecs	+= offsetSeconds
	newTimeStampStr	=	converSecondsToTimeStamp(timeStampSecs)
	return newTimeStampStr


def	doProcessFile(fileIn,	fileOut, offsetSeconds):

		#	Verify destination file	overwrite? else	just overwrite without asking
		if False and os.path.isfile(fileOut):
				print("{}	exists.	Override (y/n)?".format(fileOut))
				reply	=	input().strip().lower()
				if reply[0]	!= 'y':
					 sys.exit(1)

		#	init
		regexTimestamp = re.compile("((?:([0-9]+):)?([0-9]+):([0-9]+))")
		outLineNumber	=	0

		#	Process	the	file line-by-line
		with open(fileIn,	'r') as	fpIn,	open(fileOut,	'w') as	fpOut:
				lineNumber = 0
				for	line in	fpIn:
						lineNumber +=	1
						line = line.rstrip()	 # Strip trailing	spaces and newline
						line = re.sub(regexTimestamp,	lambda m:	modifyTimestamp(m.group(0),	offsetSeconds),	line)
						writeRawLineToFile(fpOut,	line)
						outLineNumber	+= 1								
		#	return # of	lines	written
		return outLineNumber



def	adjustTimeStamp(timeStampText, timeOffsetSeconds):
		timeStampSecs	=	convertTimeStampToSeconds(timeStampText)
		timeStampSecs	-= timeOffsetSeconds	 
		return converSecondsToTimeStamp(timeStampSecs)


def	convertTimeStampToSeconds(timeStampText):
		#	see	https://stackoverflow.com/questions/6402812/how-to-convert-an-hmmss-time-string-to-seconds-in-python
		if (timeStampText==""):
			return 0
		sgn	=	1
		if (timeStampText[0]=='-'):
			sgn	=	-1
			timeStampText	=	timeStampText[1:]
		if (timeStampText.count(':')==1):
			timeStampText	=	"00:"	+	timeStampText
		elif (timeStampText.count(':')==0):
			timeStampText	=	"00:00:" + timeStampText
		h, m,	s	=	timeStampText.split(':')
		retv = sgn * (int(h) * 3600	+	int(m) * 60	+	int(s))
		return retv


def	converSecondsToTimeStamp(secs):
		#	see	https://stackoverflow.com/questions/1384406/convert-seconds-to-hhmmss-in-python
		dt = datetime.timedelta(seconds=secs)
		timeStampText = str(dt)
		# strip off 00: leading hours and minutes
		if (optionStripLeadingHoursMinutes):
			if (timeStampText.startswith("0:")):
				timeStampText = timeStampText[2:];
			elif (timeStampText.startswith("00:")):
				timeStampText = timeStampText[3:];
			if (timeStampText.startswith("00:")):
				timeStampText = timeStampText[3:];
		return timeStampText


def	writeRawLineToFile(fpOut,	str):
		fpOut.write("{}\n".format(str))










if __name__	== '__main__':
		main()