// Copyleft 2010 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
		chris korda

		rev		date		comments
		00		29mar10		initial version
		01		04mar11		optimize PushOver, use unsigned ints
		02		19may11		in PushOver, if full, set tail to head
		03		19feb12		optimize construction

		ring buffer template

*/

#pragma once

template<class T>
class CRingBuf : public WObject {
public:
// Types
	class CIter : public WObject {
	public:
		CIter(CRingBuf<T>& Ring) : m_Ring(Ring)
		{
			m_Tail = m_Ring.m_Tail;
			m_Items = m_Ring.m_Items;
		}
		UINT	GetSize() const
		{
			return(m_Items);
		}
		bool	GetNext(T& Item)
		{
			if (!m_Items)
				return(FALSE);
			Item = *m_Tail++;
			if (m_Tail == m_Ring.m_End)
				m_Tail = m_Ring.m_Start;
			m_Items--;
			return(TRUE);
		}

	protected:
		CRingBuf<T>&	m_Ring;
		T		*m_Tail;	// the next element
		UINT	m_Items;	// number of elements
	};
	class CRevIter : public WObject {
	public:
		CRevIter(CRingBuf<T>& Ring) : m_Ring(Ring)
		{
			m_Head = m_Ring.m_Head;
			m_Items = m_Ring.m_Items;
		}
		UINT	GetSize() const
		{
			return(m_Items);
		}
		bool	GetNext(T& Item)
		{
			if (!m_Items)
				return(FALSE);
			if (m_Head == m_Ring.m_Start)
				m_Head = m_Ring.m_End;
			Item = *--m_Head;
			m_Items--;
			return(TRUE);
		}

	protected:
		CRingBuf<T>&	m_Ring;
		T		*m_Head;	// the next element
		UINT	m_Items;	// number of elements
	};

// Construction
	CRingBuf();
	CRingBuf(UINT Size);
	~CRingBuf();
	void	Create(UINT Size);
	void	Destroy();

// Attributes
	UINT	GetSize() const;
	UINT	GetCount() const;

// Operations
	bool	Push(const T& Item);
	bool	PushOver(const T& Item);
	bool	Pop(T& Item);
	void	Flush();
	UINT	Read(T *Buffer, UINT BufSize) const;
	UINT	ReadRev(T *Buffer, UINT BufSize) const;

protected:
// Member data
	T		*m_Start;	// points to first element
	T		*m_End;		// points to last element
	T		*m_Head;	// the next element to write to
	T		*m_Tail;	// the next element to read from
	UINT	m_Size;		// maximum number of elements
	UINT	m_Items;	// number of elements in use

// Helpers
	void	Init();
	void	Alloc(UINT Size);
	friend class CIter;
	friend class CRevIter;
};

template<class T>
inline CRingBuf<T>::CRingBuf()
{
	Init();
}

template<class T>
inline CRingBuf<T>::CRingBuf(UINT Size)
{
	Alloc(Size);
}

template<class T>
inline CRingBuf<T>::~CRingBuf()
{
	delete [] m_Start;
}

template<class T>
inline void CRingBuf<T>::Init()
{
	m_Start = NULL;
	m_End = NULL;
	m_Head = NULL;
	m_Tail = NULL;
	m_Size = 0;
	m_Items = 0;
}

template<class T>
inline void CRingBuf<T>::Alloc(UINT Size)
{
	m_Start = new T[Size];
	m_End = m_Start + Size;
	m_Head = m_Start;
	m_Tail = m_Start;
	m_Size = Size;
	m_Items = 0;
}

template<class T>
inline void CRingBuf<T>::Create(UINT Size)
{
	delete [] m_Start;	// in case undestroyed instance is reused
	Alloc(Size);
}

template<class T>
inline void CRingBuf<T>::Destroy()
{
	delete [] m_Start;
	Init();
}

template<class T>
inline UINT CRingBuf<T>::GetSize() const
{
	return(m_Size);
}

template<class T>
inline UINT CRingBuf<T>::GetCount() const
{
	return(m_Items);
}

template<class T>
inline bool CRingBuf<T>::Push(const T& Item)
{
	if (m_Items >= m_Size)
		return(FALSE);
	*m_Head++ = Item;
	if (m_Head == m_End)
		m_Head = m_Start;
	m_Items++;
	return(TRUE);
}

template<class T>
inline bool CRingBuf<T>::PushOver(const T& Item)
{
	if (!m_Size)
		return(FALSE);
	*m_Head++ = Item;
	if (m_Head == m_End)
		m_Head = m_Start;
	if (m_Items < m_Size)
		m_Items++;
	else
		m_Tail = m_Head;
	return(TRUE);
}

template<class T>
inline bool CRingBuf<T>::Pop(T& Item)
{
	if (!m_Items)
		return(FALSE);
	Item = *m_Tail++;
	if (m_Tail == m_End)
		m_Tail = m_Start;
	m_Items--;
	return(TRUE);
}

template<class T>
inline void CRingBuf<T>::Flush()
{
	m_Items = 0;
	m_Head = m_Tail = m_Start;
}

template<class T>
inline UINT CRingBuf<T>::Read(T *Buffer, UINT BufSize) const
{
	const T	*Tail = m_Tail;
	UINT	items = min(m_Items, BufSize);
	for (UINT i = 0; i < items; i++) {
		*Buffer++ = *Tail++;
		if (Tail == m_End)
			Tail = m_Start;
	}
	return(items);
}

template<class T>
inline UINT CRingBuf<T>::ReadRev(T *Buffer, UINT BufSize) const
{
	const T	*Head = m_Head;
	UINT	items = min(m_Items, BufSize);
	for (UINT i = 0; i < items; i++) {
		if (Head == m_Start)
			Head = m_End;
		*Buffer++ = *--Head;
	}
	return(items);
}
