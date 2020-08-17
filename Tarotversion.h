#ifndef TAROTVERSION_H
#define TAROTVERSION_H

	//Date Version Types
	static const char DATE[] = "17";
	static const char MONTH[] = "08";
	static const char YEAR[] = "2020";
	static const char UBUNTU_VERSION_STYLE[] =  "20.08";

	//Software Status
	static const char STATUS[] =  "Alpha";
	static const char STATUS_SHORT[] =  "a";

	//Standard Version Type
	static const long MAJOR  = 0;
	static const long MINOR  = 2;
	static const long BUILD  = 6;
	static const long REVISION  = 30;

	//Miscellaneous Version Types
	static const long BUILDS_COUNT  = 51;
	#define RC_FILEVERSION 0,2,6,30
	#define RC_FILEVERSION_STRING "0, 2, 6, 30\0"
	static const char FULLVERSION_STRING [] = "0.2.6.30";

	//These values are to keep track of your versioning state, don't modify them.
	static const long BUILD_HISTORY  = 5;


#endif //TAROTVERSION_H