// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      09jul05	initial version
		01      01mar13	convert to static class
 
		sortable string array
 
*/

#ifndef CSORTSTRINGARRAY_INCLUDED
#define CSORTSTRINGARRAY_INCLUDED

class CSortStringArray : public WObject {
public:
	static	void	Sort(CStringArray& StrArray);

private:
	static	int		compare(const void *arg1, const void *arg2);
};

#endif
