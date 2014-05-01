// Copyleft 2013 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      19feb13	initial version
		
		enhanced byte array that can exceed 4GB
 
*/

class CByteArrayEx : public CByteArray
{
public:
	void	SetSize(INT_PTR nNewSize, INT_PTR nGrowBy = -1);
};