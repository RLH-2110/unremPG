
#ifndef FILTER_H
#define FILTER_H

#define unicodeFilterVersion 1.0f

#define true !0
#define false 0
#define bool int

enum FilterTag {
	FilterTag_Controll		= 0b0000000000000001, //		           1		// like 0x0
	FilterTag_NewLines		= 0b0000000000000010, //		          10		// like 0xa and 0xd aswell as 0xB
	FilterTag_Spaces		= 0b0000000000000100, //		         100		// like 0x9 and 0x20
	FilterTag_Deletion		= 0b0000000000001000, //		        1000		// like 0x8 and 0x7F
	FilterTag_PageBreak		= 0b0000000000010000, //		      1 0000		// like 0xC
	FilterTag_Escape		= 0b0000000000100000, //		     10 0000		// like 0x1B
	FilterTag_NonSpaceInvis = 0b0000000001000000, //		    100 0000		// Invisible (Non Space/Tab) Characters
	FilterTag_Unassigned	= 0b0000000010000000, //		   1000 0000		// unassinged in unicode
	FilterTag_PossibleNL	= 0b0000000100000000, //		 1 0000 0000		// 0xAD (its either invisible, or visible and a line break)
	FilterTag_Modify		= 0b0000000100000000, //		10 0000 0000		// Modifies chars or text in a way. example: 0x34F and 0x61C or even 0x83
	FilterTag_IDontKnow		= 0b0000000100000000, //	   100 0000 0000		// 0x2061 - WHAT DOES IT DO???????
	FilterTag_Surrogate		= 0b0000001000000000, //	  1000 0000 0000		// surrogates (0xd800-0xdFFFF)
};
typedef struct UNIfilter
{
	unsigned int start;
	unsigned int end;
	unsigned short tag;
} UNIfilter;


#define MaximumSupportedUnicodeValue 0xfffff

bool filterUnicode(unsigned int uchar, unsigned short filter); // filters out all characters with a tag where the flag is set

#endif /* FILTER_H */