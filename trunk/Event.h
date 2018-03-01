// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
		chris korda

		rev		date		comments
		00		15aug04		initial version
		01		18oct04		ck: remove set debug name

		wrap Win32 event object

*/

#ifndef WEVENT_INCLUDED
#define WEVENT_INCLUDED

#ifndef WHANDLE_DEFINED
#define WHANDLE_DEFINED
typedef void *WHANDLE;
#endif

class WEvent : public WObject {
public:
//
// Constructs an event.
//
	WEvent();
//
// Destroys the event.
//
	~WEvent();
//
// Creates the event.
//
// Returns: True if successful.
//
	bool	Create(
		LPSECURITY_ATTRIBUTES lpEventAttributes,	// Pointer to security attributes.
		BOOL	bManualReset,		// Flag for manual-reset event.
		BOOL	bInitialState,		// Flag for initial state.
		LPCTSTR lpName,				// Pointer to event-object name.
		LPCSTR	DebugName = NULL	// Optional pointer to the debug name.
	);
//
// Closes the event handle.
//
	void	Close();
//
// Sets the event.
//
	void	Set();
//
// Resets the event.
//
	void	Reset();
//
// Retrieves the event's handle.
//
// Returns: The handle.
//
	operator WHANDLE() const;

protected:
	WHANDLE	m_hEvent;			// The event handle.
};

inline void WEvent::Set()
{
	SetEvent(m_hEvent);
}

inline void WEvent::Reset()
{
	ResetEvent(m_hEvent);
}

inline WEvent::operator WHANDLE() const
{
	return(m_hEvent);
}

#endif
