// Copyleft 2012 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      04oct12	initial version
		01		28jan13	add GetNowXClampEnd
		02		30jan13	add OnNextPane and OnPrevPane
		03		12feb13	move palette to options
        04      25feb13	add Resample
        05      28feb13	add audio insert
        06      12mar13	add channel captions
        07      19mar13	add spectrum dialog
		08		20may13	fix format change handling
        09		04jun13	add channel selection
		10		28jun13	add plugins
		11		28jul13	add metadata

		wave editor view
 
*/

// WaveShopView.h : interface of the CWaveShopView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_WAVVIEWVIEW_H__72AB0B56_FEA8_4A0A_9168_5EDD39C9AB3D__INCLUDED_)
#define AFX_WAVVIEWVIEW_H__72AB0B56_FEA8_4A0A_9168_5EDD39C9AB3D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ArrayEx.h"
#include "Undoable.h"
#include "UndoCodes.h"

class CWaveProcess;
class CTimeRulerCtrl;
class CChannelBar;

class CWaveShopView : public CScrollView, public CUndoable
{
protected: // create from serialization only
	CWaveShopView();
	DECLARE_DYNCREATE(CWaveShopView)

// Constants
	enum {	// update hints
		HINT_NONE			= 0,		// invalidate
		HINT_WAVE_UPDATE	= 0x01,		// audio changed
		HINT_TIME_UNIT		= 0x02,		// time unit changed
		HINT_WAVE_FORMAT	= 0x04,		// audio format changed
		HINT_SHOW_CAPTIONS	= 0x08,		// caption visibility changed
		HINT_INITIAL_UPDATE	= HINT_WAVE_UPDATE | HINT_TIME_UNIT | HINT_SHOW_CAPTIONS,
	};

// Attributes
public:
	CWaveShopDoc* GetDocument();
	const CWaveShopDoc* GetDocument() const;
	CWaveProcess&	GetWave();
	CSize	GetWndSize() const;
	CSize	GetPageSize() const;
	double	GetZoom() const;
	bool	SetZoom(int x, double Zoom);
	double	GetAmplitude(int ChannelIdx) const;
	bool	SetAmplitude(int ChannelIdx, double Amplitude);
	double	GetVerticalOrigin(int ChannelIdx) const;
	void	SetVerticalOrigin(int ChannelIdx, double Origin);
	bool	SetVerticalZoom(int ChannelIdx, int y, double Amplitude);
	LONGLONG	GetScrollPosition() const;
	int		GetTimeRulerOffset() const;
	void	SetTimeRuler(CTimeRulerCtrl *Ruler);
	void	SetChannelBar(CChannelBar *Bar);
	int		GetChannelPos(int ChannelIdx, int& Height) const;
	int		GetChannelCount() const;
	double	GetNow() const;
	W64INT	GetIntNow() const;
	int		GetNowX() const;
	int		GetNowXClampEnd() const;
	void	SetNow(double Now);
	CDblRange	GetSelection() const;
	CW64IntRange	GetIntSelection() const;
	CIntRange	GetSelectionX() const;
	void	SetSelection(const CDblRange& Sel);
	bool	HaveSelection() const;
	bool	SetChannelMask(UINT ChannelMask);
	void	InvalidateChannel(int ChannelIdx);
	void	SelectChannel(int ChannelIdx, bool Enable);
	bool	IsChannelSelected(int ChannelIdx) const;
	int		GetChannelSelection(CByteArray *ChanSel = NULL) const;
	void	SetChannelSelection(const BYTE *ChanSel);
	bool	AllChannelsSelected() const;
	void	SelectAllChannels();
	static	int		GetUndoTitleID(int UndoCode);

// Operations
public:
	double	XToFrame(double x) const;
	double	FrameToX(double FrameIdx) const;
	bool	StepZoom(int x, bool In);
	bool	StepAmplitude(int ChannelIdx, bool In);
	bool	StepVerticalZoom(int ChannelIdx, int y, bool In);
	void	MaximizeChannel(int ChannelIdx);
	void	ScrollToPosition(LONGLONG ScrollPos);
	void	FitInWindow();
	void	FitVertically();
	void	OnRecalcLayout();
	void	EnsureVisible(double FrameIdx, bool Center = TRUE);
	bool	Copy();
	bool	Delete(bool Cut);
	bool	Paste();
	bool	Insert();
	bool	Insert(const CWaveEdit& Wave);
	bool	InsertSilence(W64INT FrameCount);
	double	FindZeroCrossing(double Frame) const;
	bool	InsertChannel(int ChannelIdx);
	bool	InsertChannel(int ChannelIdx, const CWaveEdit& Wave);
	bool	DeleteChannel(int ChannelIdx);
	bool	Find(bool First);
	void	ApplyChannelSelection();
	static	bool	RespectsChannelSelection(WORD UndoCode);

// Overrides
	virtual	CString	GetUndoTitle(const CUndoState& State);
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWaveShopView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnInitialUpdate();
	virtual void OnPrepareDC(CDC* pDC, CPrintInfo* pInfo = NULL);
	virtual BOOL OnScroll(UINT nScrollCode, UINT nPos, BOOL bDoScroll = TRUE);
	protected:
	virtual BOOL OnScrollBy(CSize sizeScroll, BOOL bDoScroll = TRUE);
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CWaveShopView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CWaveShopView)
	afx_msg void OnAudioAmplify();
	afx_msg void OnAudioChangeFormat();
	afx_msg void OnAudioExtractChannels();
	afx_msg void OnAudioFade();
	afx_msg void OnAudioFindClipping();
	afx_msg void OnAudioInsertChannel();
	afx_msg void OnAudioInvert();
	afx_msg void OnAudioNormalize();
	afx_msg void OnAudioPeakStats();
	afx_msg void OnAudioRMSStats();
	afx_msg void OnAudioResample();
	afx_msg void OnAudioReverse();
	afx_msg void OnAudioSpeakers();
	afx_msg void OnAudioSpectrum();
	afx_msg void OnAudioSwapChannels();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnEditBeginSelection();
	afx_msg void OnEditCopy();
	afx_msg void OnEditCut();
	afx_msg void OnEditDelete();
	afx_msg void OnEditDeselect();
	afx_msg void OnEditEndSelection();
	afx_msg void OnEditFind();
	afx_msg void OnEditFindNext();
	afx_msg void OnEditFindZeroCrossing();
	afx_msg void OnEditGotoFirstFrame();
	afx_msg void OnEditGotoLastFrame();
	afx_msg void OnEditGotoNow();
	afx_msg void OnEditGotoSelEnd();
	afx_msg void OnEditGotoSelStart();
	afx_msg void OnEditInsert();
	afx_msg void OnEditInsertSilence();
	afx_msg void OnEditMetadata();
	afx_msg void OnEditPaste();
	afx_msg void OnEditRedo();
	afx_msg void OnEditSelectAll();
	afx_msg void OnEditUndo();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnFileExport();
	afx_msg void OnFileInfo();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnNextPane();
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTimer(W64UINT nIDEvent);
	afx_msg void OnUpdateAudioSwapChannels(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditCopy(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditCut(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditDelete(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditDeselect(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditPaste(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditRedo(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditUndo(CCmdUI* pCmdUI);
	afx_msg void OnUpdateNextPane(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewCtxDeleteChannel(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewCtxMaximizeChannel(CCmdUI* pCmdUI);
	afx_msg void OnUpdateWaveNotEmpty(CCmdUI* pCmdUI);
	afx_msg void OnUpdateWaveValid(CCmdUI* pCmdUI);
	afx_msg void OnViewCtxDeleteChannel();
	afx_msg void OnViewCtxEndSelection();
	afx_msg void OnViewCtxInsertChannel();
	afx_msg void OnViewCtxMaximizeChannel();
	afx_msg void OnViewCtxPaste();
	afx_msg void OnViewCtxStartSelection();
	afx_msg void OnViewCtxZoomIn();
	afx_msg void OnViewCtxZoomOut();
	afx_msg void OnViewFitVertically();
	afx_msg void OnViewFitWindow();
	afx_msg void OnViewRefresh();
	afx_msg void OnViewZoomIn();
	afx_msg void OnViewZoomOut();
	//}}AFX_MSG
	afx_msg LRESULT	OnMouseLeave(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT	OnFitInWindow(WPARAM wParam, LPARAM lParam);
	afx_msg void OnPlugin(UINT nID);
	afx_msg void OnPluginRepeat();
	afx_msg void OnUpdatePluginRepeat(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

// Types
	typedef CWave::SAMPLE SAMPLE;
	struct BUCKET {	// bucket of samples
		SAMPLE	Min;			// lowest sample
		SAMPLE	Max;			// highest sample
	};
	typedef CArrayEx<BUCKET, BUCKET&> CBucketArray;
	class CChannelInfo : public WObject {	// channel information
	public:
		class CCaption : public CStatic, public CRefObj {};	// caption control
		typedef CRefPtr<CCaption> CCaptionPtr;	// reference-counted pointer
		int		m_y;				// y-coordinate
		int		m_Height;			// height in pixels
		double	m_HeightFrac;		// height as a fraction of total height
		double	m_Origin;			// y-origin; 0 == center, +/- 0.5 == rails
		double	m_Amplitude;		// amplitude; 1 == fit to channel height
		CCaptionPtr	m_Caption;		// reference-counted pointer to caption
		bool	m_Selected;			// true if channel is selected
	};
	typedef	CArrayEx<CChannelInfo, CChannelInfo&> CChanInfoArray;
	class CClipboardUndoInfo : public CRefObj {
	public:
		CClipboardUndoInfo();
		~CClipboardUndoInfo();
		CWaveEdit	m_Wave;			// wave 
		double		m_Now;			// previous now
		CDblRange	m_Selection;	// previous selection
		CW64IntRange	m_EditSel;	// portion of wave being edited
		BYTE		*m_ChanSel;		// channel selection if applicable
	};
	class CMetadataUndoInfo : public CRefObj {
	public:
		CStringArray	m_Metadata;	// metadata string array
	};

// Constants
	enum {
		GUTTER_HEIGHT = 4,		// height of horizontal gutter between channels
		MAX_SCROLL_SIZE = INT_MAX,	// maximum horizontal scroll size, in pixels
		MIN_CHAN_HEIGHT = 8,	// minimum channel height during drag, in pixels
		SCROLL_TIMER_ID = 1789,	// scroll timer ID
		SCROLL_TIMER_PERIOD = 50,	// scroll timer period, in milliseconds
		CAPTION_HORZ_MARGIN = 2,	// caption text horizontal margin, in pixels
	};
	enum {	// drag states
		DRAG_NONE,				// default state
		DRAG_TRACK,				// tracking potential drag
		DRAG_SELECTION,			// dragging selection
		DRAG_GUTTER,			// dragging gutter
		DRAG_TRACK_CHAN_SELECT,	// tracking potential channel drag selection
		DRAG_CHANNEL_SELECTION,	// dragging channel selection
	};
	enum {	// hit test codes
		HT_NOWHERE,				// nowhere special
		HT_SEL_START,			// on selection start boundary
		HT_SEL_END,				// on selection end boundary
		HT_GUTTER,				// on gutter between channels
	};
	static const double MIN_ZOOM;
	static const double MAX_ZOOM;
	static const double DEFAULT_ZOOM;
	static const double MIN_AMP;
	static const double	MAX_AMP;
	static const double DEFAULT_AMP;
	static const int m_UndoTitleID[UNDO_CODES];
	static const WORD m_ChanSelUndoCode[];	// commands that respect channel selection

// Member data
	CMainFrame	*m_Main;		// pointer to main frame
	CWaveProcess	*m_Wave;	// pointer to wave
	CTimeRulerCtrl	*m_TimeRuler;	// pointer to time ruler
	CChannelBar	*m_ChannelBar;	// pointer to channel bar
	CBucketArray	m_Bucket;	// array of sample buckets
	CChanInfoArray	m_ChanInfo;	// array of channel info
	CSize	m_WndSize;			// size of window, in pixels
	double	m_Zoom;				// number of sample frames per pixel
	LONGLONG	m_WaveWidth;	// width of entire wave, in pixels
	LONGLONG	m_ScrollPos;	// horizontal scroll position, in pixels
	double	m_ScrollScale;		// scroll scaling to 32-bit
	int		m_PagePercent;		// scroll page percentage
	int		m_LinePercent;		// scroll line percentage
	int		m_PageSize;			// scroll page size, in pixels
	int		m_LineSize;			// scroll line size, in pixels
	int		m_BucketMargin;		// bucket array margin, in sample frames
	int		m_TimeRulerOffset;	// offset of time ruler from view, in pixels
	int		m_DragState;		// drag state, see enum above
	CPoint	m_DragOrigin;		// drag initial point, in client coords
	int		m_DragChannel;		// index of channel being dragged
	int		m_DragChanOrigin;	// initial position of channel being dragged
	double	m_DragSelOrigin;	// origin of selection being dragged, in frames
	int		m_DragScrollDelta;	// amount to scroll during drag, in pixels
	bool	m_IsZooming;		// true during zoom
	bool	m_IsTrackingMouse;	// true if tracking mouse
	bool	m_DeferBuckets;		// true if deferring update buckets
	bool	m_ShowCaptions;		// true if showing channel captions
	double	m_Now;				// current position, in sample frames
	CDblRange	m_Selection;	// current selection, in sample frames
	CDblRange	m_PrevSelection;	// previous selection, for undo paste
	CPoint	m_ContextTarget;	// context menu target, in client coords
	UINT	m_LastPluginCmdID;	// command ID of most recently used plugin

// Overrides
	virtual	void	SaveUndoState(CUndoState& State);
	virtual	void	RestoreUndoState(const CUndoState& State);

// Helpers
	void	Update(LPARAM Hint = HINT_WAVE_UPDATE);
	bool	IsValidBucket(const BUCKET *pBucket) const;
	void	UpdateBuckets(int x1, int x2);
	void	OnWaveUpdate();
	void	OnWaveFormatChange();
	void	DeferFitInWindow();
	LONGLONG	GetMaxScrollPos() const;
	void	CalcWaveWidth(double Zoom);
	int		CalcPageWidth(int WndWidth) const;
	int		CalcNumBuckets(int PageWidth) const;
	double	CalcFitZoom() const;
	void	VertSyncChannels(int ChannelIdx);
	void	ApplyVertZoomStep(bool In, double& Amplitude) const;
	void	UpdateScrollSize();
	void	ZoomRuler(double Zoom);
	void	OnTimeUnitChange();
	int		FindChannel(int y) const;
	int		FindChannelFuzzy(int y) const;
	int		FindGutter(int y) const;
	int		FindInsertPosition(int y) const;
	void	UpdateChannelCount();
	void	UpdateChannelHeights(int WndHeight);
	void	DragGutter(int y);
	void	EndDrag();
	void	ResetChannelInfo();
	void	GetClipboardUndoInfo(CUndoState& State, const CDblRange& Selection) const;
	void	ForceSelection();
	void	OnAudioProcessResult(bool Result);
	void	OnAudioProcessResult(bool Result, LPARAM Hint);
	int		HitTest(CPoint Point, int& ItemIdx) const;
	bool	Paste(const CWaveEdit& Wave, WORD UndoCode);
	static	int		Min64To32(LONGLONG x, int y);
	static	CDblRange	IntToDblRange(const CW64IntRange& Range);
	void	UpdateCaptions();
	void	RepositionAllCaptions(bool Show = TRUE) const;
	void	RepositionCaption(int ChannelIdx) const;
	void	ShowCaptions(bool Enable);
	void	SetUIState(const CClipboardUndoInfo& Info, bool Center = FALSE);
	void	SetUIState(double Now, const CDblRange& Sel, const BYTE *ChanSel, bool Center = FALSE);
	int		ToggleChannelSelection(UINT nFlags, CPoint point);

// Aliases for undo value members
	CUNDOSTATE_VAL(	UVInsert,		WORD,	p.x.w.lo)
	CUNDOSTATE_VAL(	UVSpeakers,		UINT,	p.x.u)
};

inline CWaveProcess& CWaveShopView::GetWave()
{
	ASSERT(m_pDocument);
	return(GetDocument()->m_Wave);
}

inline CSize CWaveShopView::GetWndSize() const
{
	return(m_WndSize);
}

inline double CWaveShopView::GetZoom() const
{
	return(m_Zoom);
}

inline double CWaveShopView::GetAmplitude(int ChannelIdx) const
{
	return(m_ChanInfo[ChannelIdx].m_Amplitude);
}

inline double CWaveShopView::GetVerticalOrigin(int ChannelIdx) const
{
	return(m_ChanInfo[ChannelIdx].m_Origin);
}

inline LONGLONG CWaveShopView::GetScrollPosition() const
{
	return(m_ScrollPos);
}

inline int CWaveShopView::GetTimeRulerOffset() const
{
	return(m_TimeRulerOffset);
}

inline void CWaveShopView::SetTimeRuler(CTimeRulerCtrl *Ruler)
{
	m_TimeRuler = Ruler;
}

inline void CWaveShopView::SetChannelBar(CChannelBar *Bar)
{
	m_ChannelBar = Bar;
}

inline int CWaveShopView::GetChannelPos(int ChannelIdx, int& Height) const
{
	const CChannelInfo&	info = m_ChanInfo[ChannelIdx];
	Height = info.m_Height;
	return(info.m_y);
}

inline int  CWaveShopView::GetChannelCount() const
{
	return(m_ChanInfo.GetSize());
}

inline double CWaveShopView::GetNow() const
{
	return(m_Now);
}

inline CDblRange CWaveShopView::GetSelection() const
{
	return(m_Selection);
}

inline bool CWaveShopView::HaveSelection() const
{
	return(!m_Selection.IsEmpty());
}

inline bool CWaveShopView::IsChannelSelected(int ChannelIdx) const
{
	return(m_ChanInfo[ChannelIdx].m_Selected);
}

inline int CWaveShopView::GetUndoTitleID(int UndoCode)
{
	ASSERT(UndoCode >= 0 && UndoCode < UNDO_CODES);
	return(m_UndoTitleID[UndoCode]);
}

#ifndef _DEBUG  // debug version in WaveShopView.cpp
inline CWaveShopDoc* CWaveShopView::GetDocument()
{
	return (CWaveShopDoc*)m_pDocument;
}
inline const CWaveShopDoc* CWaveShopView::GetDocument() const
{
	return (CWaveShopDoc*)m_pDocument;
}
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WAVVIEWVIEW_H__72AB0B56_FEA8_4A0A_9168_5EDD39C9AB3D__INCLUDED_)
