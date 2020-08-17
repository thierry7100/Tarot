#ifndef VERSION_H
#define VERSION_H

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
	static const long BUILD  = 4;
	static const long REVISION  = 22;
	
	//Miscellaneous Version Types
	static const long BUILDS_COUNT  = 35;
	#define RC_FILEVERSION 0,2,4,22
	#define RC_FILEVERSION_STRING "0, 2, 4, 22\0"
	static const char FULLVERSION_STRING [] = "0.2.4.22";
	
	//These values are to keep track of your versioning state, don't modify them.
	static const long BUILD_HISTORY  = 3;
	

#endif //VERSION_H
