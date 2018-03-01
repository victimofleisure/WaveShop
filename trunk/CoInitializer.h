// Copyleft 2012 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00		15nov12	initial version

		wrap COM library lifetime

*/

class CCoInitializer
{
public:
	explicit CCoInitializer(DWORD dwCoInit = COINIT_APARTMENTTHREADED)
	: _coinitialized(false)
	{
		// Initialize the COM library on the current thread.
		HRESULT hr = CoInitializeEx(NULL, dwCoInit);
		if (FAILED(hr))
			throw hr;
		_coinitialized = true;
	}
	~CCoInitializer()
	{
		// Free the COM library. 
		if (_coinitialized)
			CoUninitialize();
	}
private:
// Flags whether COM was properly initialized. 
	bool _coinitialized;

// Hide copy constructor and assignment operator.
	CCoInitializer(const CCoInitializer&);
	CCoInitializer& operator=(const CCoInitializer&);
};
