// Copyleft 2013 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      28apr13	initial version
		01		17may13	add activation types
		02		18jun13	add open recording checkbox

		record dialog
 
*/

// RecordDlg.cpp : implementation file
//

#include "stdafx.h"
#include "WaveShop.h"
#include "RecordDlg.h"
#include "PathStr.h"
#include "DocIter.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRecordDlg dialog

#define DXChk(f) { HRESULT hr = f; if (FAILED(hr)) { OnDXError(hr); return(FALSE); }}

const CCtrlResize::CTRL_LIST CRecordDlg::m_CtrlList[] = {
	{IDC_REC_METER,		BIND_ALL},
	{IDOK,				BIND_BOTTOM},
	{IDCANCEL,			BIND_BOTTOM},
	{0}	// list terminator
};

const CRecordDlg::STATUS_INFO CRecordDlg::m_StatusInfo[STATUSES] = {
	// Name						PauseBtnText		BkColor
	{IDS_REC_STATUS_STOP,		IDC_REC_CANCEL,		0},
	{IDS_REC_STATUS_RECORD,		IDC_REC_PAUSE,		RGB(255, 0, 0)},
	{IDS_REC_STATUS_PAUSE,		IDC_REC_RESUME,		RGB(255, 255, 0)},
};

const UINT CRecordDlg::m_SampleRatePreset[] = {
	8000, 9600, 11025, 12000, 16000, 22050, 24000, 
	32000, 44100, 48000, 88200, 96000, 176400, 192000
};

#define RK_RECORD			REG_SETTINGS _T("\\Record")
#define RK_SYNC_PLAYBACK	_T("SyncPlayback")
#define RK_OPEN_RECORDING	_T("OpenRecording")

CRecordDlg::CRecordDlg(CString OutPath, CWnd* pParent /*=NULL*/)
	: CPersistDlg(IDD, IDR_MAINFRAME, _T("RecordDlg"), pParent), m_OutPath(OutPath)
{
	//{{AFX_DATA_INIT(CRecordDlg)
	m_SyncPlayback = FALSE;
	m_OpenRecording = TRUE;
	//}}AFX_DATA_INIT
	m_InitSize = CSize(0, 0);
	m_MeterView = NULL;
	m_Canceled = FALSE;
	m_TriggerLen = 0;
	LoadSettings();
}

CRecordDlg::~CRecordDlg()
{
	if (!m_Canceled)	// if dialog not canceled
		StoreSettings();
}

const RECORD_PARMS& CRecordDlg::GetRecParms()
{
	return(theApp.GetMain()->GetOptions().m_Record);
}

inline int CRecordDlg::GetActivationType()
{
	return(GetRecParms().ActivationType);
}

inline void CRecordDlg::LoadSettings()
{
	const RECORD_PARMS& parms = theApp.GetMain()->GetOptions().m_Record;
	m_Channels		= parms.Channels;
	m_SampleRate	= parms.SampleRate;
	m_SampleSize	= parms.SampleSize;
	m_SyncPlayback	= theApp.RdRegExInt(RK_RECORD, RK_SYNC_PLAYBACK, FALSE);
	m_OpenRecording	= theApp.RdRegExInt(RK_RECORD, RK_OPEN_RECORDING, TRUE);
}

inline void CRecordDlg::StoreSettings()
{
	RECORD_PARMS	parms = theApp.GetMain()->GetOptions().m_Record;
	parms.Channels		= m_Channels;
	parms.SampleRate	= m_SampleRate;
	parms.SampleSize	= m_SampleSize;
	theApp.GetMain()->SetRecordParms(parms);
	theApp.WrRegExInt(RK_RECORD, RK_SYNC_PLAYBACK, m_SyncPlayback);
	theApp.WrRegExInt(RK_RECORD, RK_OPEN_RECORDING, m_OpenRecording);
}

bool CRecordDlg::CloseDoc(LPCTSTR Path)
{
	LPCTSTR	FileName = PathFindFileName(Path);
	CAllDocIter	iter;
	CDocument	*pDoc;
	while ((pDoc = iter.GetNextDoc()) != NULL) {	// for each document
		if (pDoc->GetPathName() == Path) {	// if document is open
			if (pDoc->IsModified()) {	// if document has unsaved changes
				CString	msg;
				AfxFormatString1(msg, IDS_REC_UNSAVED_CHANGES, FileName);
				AfxMessageBox(msg);
				return(FALSE);	// user must save or abandon changes
			}
			pDoc->OnCloseDocument();	// close unmodified document
			break;	// no need for further iteration
		}
	}
	return(TRUE);
}

bool CRecordDlg::DoFileDlg(CString& Path)
{
	CString	Title(LPCTSTR(IDS_REC_SAVE_AS));
	CString	Filter(LPCTSTR(IDS_REC_FILTER));
	CString	DefName(LPCTSTR(IDS_REC_UNTITLED));
	CFileDialog	fd(FALSE, WAV_EXT, DefName, OFN_OVERWRITEPROMPT, Filter);
	fd.m_ofn.lpstrTitle = Title;
	if (fd.DoModal() != IDOK)
		return(FALSE);	// user canceled
	if (!CloseDoc(fd.GetPathName()))
		return(FALSE);	// user canceled
	Path = fd.GetPathName();
	return(TRUE);
}

CRecordDlg *CRecordDlg::Record()
{
	CString	path;
	if (GetActivationType() == ACT_PROMPT) {	// if prompting user
		if (!DoFileDlg(path))	// get output path
			return(FALSE);
	}
	CRecordDlg	*pDlg = new CRecordDlg(path);
	if (!pDlg->Create(IDD_RECORD))	// create modeless record dialog
		return(NULL);	// create failed
	return(pDlg);	// return pointer to record dialog instance
}

void CRecordDlg::UpdateUI()
{
	UpdateDialogControls(this, TRUE);
}

int CRecordDlg::GetStatus() const
{
	if (IsRecording()) {	// if recording
		if (IsPaused())	// if paused
			return(ST_PAUSE);
		else	// not paused
			return(ST_RECORD);
	} else	// not recording
		return(ST_STOP);
}

bool CRecordDlg::Start()
{
	if (IsRecording())	// if recording
		return(TRUE);	// nothing to do
	if (!UpdateData())	// save and validate dialog data
		return(FALSE);
	StoreSettings();	// store persistent settings
	if (!UpdateMeterCapture())
		return(FALSE);
	if (GetActivationType() != ACT_PROMPT) {	// if not prompting user
		// generate filename from current time
		CString	FileName = CTime::GetCurrentTime().Format("%Y-%m-%d_%H-%M-%S");
		CPathStr	path(theApp.GetMain()->GetOptions().m_RecordFolderPath);
		theApp.MakeAbsolutePath(path);
		path.Append(FileName + WAV_EXT);
		m_OutPath = path;
		OnOutputPathChange();
	}
	if (!m_RecordCapture.Open(m_OutPath, m_Channels, m_SampleRate, m_SampleSize))
		return(FALSE);
	if (!Pause(FALSE))	// resume to start recording
		return(FALSE);
	// replace cancel button with pause/resume button
	CWnd	*pCancelBtn = GetDlgItem(IDCANCEL);
	if (pCancelBtn != NULL)
		pCancelBtn->SetDlgCtrlID(IDC_REC_PAUSE);
	OnCaptureWrite(0, 0);	// show initial stats
	UpdateUI();
	CWnd	*pStartBtn = GetDlgItem(IDOK);
	GotoDlgCtrl(pStartBtn);	// focus stop button
	return(TRUE);
}

bool CRecordDlg::Stop(UINT Flags)
{
	if (IsRecording()) {	// if recording
		// if confirmation required, confirm stop request
		if ((Flags & SF_CONFIRM) && !StopCheck())
			return(FALSE);
		if (!Pause(TRUE))	// stop capturing
			return(FALSE);
		if (!m_RecordCapture.Close())	// close capture instance
			return(FALSE);
		UpdateUI();
		UpdateData();	// retrieve in case controls changed while recording
		// if recording should be opened, and isn't empty
		if ((Flags & SF_OPEN_RECORDING) && m_OpenRecording
		&& m_RecordCapture.GetOutputDataSize())
			theApp.OpenDocumentFile(m_OutPath);	// open audio file
	}
	if (Flags & SF_CLOSE_DIALOG) {	// if dialog should be closed
		DestroyWindow();	// destroy our instance; no further member access
	} else {	// not closing dialog
		// replace pause/resume button with cancel button
		CWnd	*pPauseBtn = GetDlgItem(IDC_REC_PAUSE);
		if (pPauseBtn != NULL)
			pPauseBtn->SetDlgCtrlID(IDCANCEL);
	}
	return(TRUE);
}

bool CRecordDlg::StopCheck()
{
	if (!IsRecording())	// if not recording
		return(TRUE);	// check unneeded
	if (GetActivationType() != ACT_PROMPT)	// if not prompting user
		return(TRUE);	// check undesired
	Show();
	UINT	style = MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2;
	int	retc = AfxMessageBox(IDS_REC_STOP_CHECK, style);
	return(retc == IDYES);
}

bool CRecordDlg::Pause(bool Enable)
{
	bool	retc;
	if (Enable)	// if pausing
		retc = m_RecordCapture.Stop();	// stop capturing
	else	// resuming
		retc = m_RecordCapture.Start();	// start capturing
	if (!retc)	// if capture error
		return(FALSE);
	if (IsDlgButtonChecked(IDC_REC_SYNC_PLAYBACK))	// if synchronized playback
		theApp.GetMain()->SetPlaying(!Enable);	// set player state
	UpdateUI();
	return(TRUE);
}

void CRecordDlg::Show()
{
	if (IsIconic())
		ShowWindow(SW_SHOWNORMAL);
	SetFocus();
}

bool CRecordDlg::CreateDevices()
{
	const COptionsInfo&	opts = theApp.GetMain()->GetOptions();
	LPGUID	RecDevGuid = const_cast<LPGUID>(&opts.m_RecordDeviceGuid);
	if (!m_MeterCapture.Create(NULL, RecDevGuid))	// create meter capture
		return(FALSE);
	if (!m_RecordCapture.Create(m_hWnd, RecDevGuid))	// create record capture
		return(FALSE);
	// set record capture buffer duration
	if (!m_RecordCapture.SetBufferDuration(opts.m_RecordBufferSize))
		return(FALSE);
	return(TRUE);
}

bool CRecordDlg::OnDeviceChange()
{
	if (IsRecording()) {	// if recording
		AfxMessageBox(IDS_REC_CANT_CHANGE_DEVICE);
		return(FALSE);	// can't change device while recording
	}
	if (!CreateDevices())
		return(FALSE);
	if (!UpdateMeterCapture())	// update meter capture
		return(FALSE);
	return(TRUE);
}

void CRecordDlg::ResizeMeterView()
{
	CWnd	*pWnd = GetDlgItem(IDC_REC_METER);
	if (pWnd != NULL) {
		CRect	r;
		pWnd->GetWindowRect(r);
		ScreenToClient(r);
		m_MeterView->MoveWindow(r);
	}
}

bool CRecordDlg::OnFormatChange()
{
	const WAVEFORMATEX&	wf = m_MeterCapture.GetWaveFormat();
	if (m_Channels != wf.nChannels
	|| m_SampleRate != wf.nSamplesPerSec
	|| m_SampleSize != wf.wBitsPerSample) {	// if audio format changed
		return(UpdateMeterCapture());
	}
	return(TRUE);
}

bool CRecordDlg::UpdateMeterCapture()
{
	if (!m_MeterCapture.Start(m_Channels, m_SampleRate, m_SampleSize))
		return(FALSE);
	m_MeterView->UpdateView();
	return(TRUE);
}

void CRecordDlg::UpdateFreeDiskSpace()
{
	ULARGE_INTEGER	UserFreeBytes;
	// get free bytes available to user associated with calling thread
	if (GetDiskFreeSpaceEx(m_OutDir, &UserFreeBytes, NULL, NULL)) {
		CString	s(m_NumFormat.FormatByteSize(UserFreeBytes.QuadPart));
		if (s != m_PrevFreeDiskStr) {	// if free disk space string changed
			m_FreeDiskSpace.SetWindowText(s);
			m_PrevFreeDiskStr = s;
		}
	}
}

void CRecordDlg::InitSampleRateCombo(CComboBox& Combo, UINT SampleRate)
{
	CWnd*	pComboEdit = Combo.GetWindow(GW_CHILD);
	if (pComboEdit != NULL)
		pComboEdit->ModifyStyle(0, ES_NUMBER);	// numeric only
	CString	s;
	int	iSelPreset = -1;
	int	presets = _countof(m_SampleRatePreset);
	for (int iPreset = 0; iPreset < presets; iPreset++) {	// for each rate preset
		s.Format(_T("%d"), m_SampleRatePreset[iPreset]);
		Combo.AddString(s);
		if (m_SampleRatePreset[iPreset] == SampleRate)	// if rate matches caller's
			iSelPreset = iPreset;	// save index of rate preset
	}
	if (iSelPreset >= 0)	// if rate is a preset
		Combo.SetCurSel(iSelPreset);	// select preset
	else {	// rate isn't a preset
		s.Format(_T("%d"), SampleRate);	// convert rate to string
		Combo.SetWindowText(s);	// set edit control
	}
}

UINT CRecordDlg::GetSampleRate(CComboBox& Combo)
{
	int	sel = Combo.GetCurSel();
	if (sel >= 0) {	// if valid selection
		ASSERT(sel < _countof(m_SampleRatePreset));
		return(m_SampleRatePreset[sel]);	// return selected rate preset
	}
	CString	s;
	Combo.GetWindowText(s);
	return(_ttoi(s));	// return rate typed in edit control
}

void CRecordDlg::ValidateSampleRate(CDataExchange *pDX, int nIDC, CComboBox& Combo, UINT& SampleRate)
{
	if (pDX->m_bSaveAndValidate) {
		pDX->PrepareCtrl(nIDC);
		UINT	rate = GetSampleRate(Combo);
		DDV_MinMaxUInt(pDX, rate, 1, UINT_MAX);
		SampleRate = rate;
	}
}

void CRecordDlg::InitSampleSizeCombo(CComboBox& Combo, UINT SampleSize)
{
	CString	s;
	int	iSelPreset = -1;
	for (int iPreset = 0; iPreset < SAMPLE_SIZE_PRESETS; iPreset++) {
		UINT	size = (iPreset + 1) * 8;
		s.Format(_T("%d"), size);
		Combo.AddString(s);
		if (size == SampleSize)
			iSelPreset = iPreset;
	}
	Combo.SetCurSel(iSelPreset);
}

UINT CRecordDlg::GetSampleSize(CComboBox& Combo)
{
	int	sel = Combo.GetCurSel();
	return((sel + 1) * 8);
}

void CRecordDlg::OnOutputPathChange()
{
	CString	s(m_InitCaption);
	if (!m_OutPath.IsEmpty())
		 s += CString(" - ") + PathFindFileName(m_OutPath);
	SetWindowText(s);
	// GetDiskFreeSpaceEx won't accept path of open file
	CPathStr	OutDir(m_OutPath);
	OutDir.RemoveFileSpec();	// pass it containing folder instead
	m_OutDir = OutDir;
}

bool CRecordDlg::CWaveDSCapture::Start(UINT Channels, UINT SampleRate, UINT SampleBits)
{
	m_Wave.SetFormat(Channels, SampleRate, SampleBits);
	m_BufDuration = 1000 / CRecordDlg::GetTimerFrequency();
	if (!CreateCaptureBuffer(m_Wave))
		return(FALSE);
	m_Wave.SetFrameCount(m_BufSize / m_Wave.GetFrameSize());
	DXChk(m_DSCBuf->Start(DSCBSTART_LOOPING));
	return(TRUE);
}

bool CRecordDlg::CWaveDSCapture::Read()
{
	DWORD	ReadCursor;
	DXChk(m_DSCBuf->GetCurrentPosition(NULL, &ReadCursor));
	// move read cursor backwards in time by our buffer size, wrapping as needed
	if (ReadCursor < m_BufSize)	// if cursor before middle
		ReadCursor += m_BufSize;	// wrap cursor by buffer size
	else	// cursor at or past middle
		ReadCursor -= m_BufSize;	// decrement cursor by buffer size
	// lock buffer and copy captured audio to wave container
	PVOID	pBuf1, pBuf2;
	DWORD	Len1, Len2;
	DXChk(m_DSCBuf->Lock(ReadCursor, m_BufSize, &pBuf1, &Len1, &pBuf2, &Len2, 0));
	memcpy(m_Wave.GetData(), pBuf1, Len1);
	if (pBuf2 != NULL)	// if second buffer needed
		memcpy(m_Wave.GetData() + Len1, pBuf2, Len2);
	DXChk(m_DSCBuf->Unlock(pBuf1, Len1, pBuf2, Len2));
	return(TRUE);
}

void CRecordDlg::DoDataExchange(CDataExchange* pDX)
{
	CPersistDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRecordDlg)
	DDX_Control(pDX, IDC_REC_SAMPLE_SIZE, m_SampleSizeCombo);
	DDX_Control(pDX, IDC_REC_SAMPLE_RATE, m_SampleRateCombo);
	DDX_Control(pDX, IDC_REC_STATUS, m_Status);
	DDX_Control(pDX, IDC_REC_FREE_DISK_SPACE, m_FreeDiskSpace);
	DDX_Control(pDX, IDC_REC_RECORDING_SIZE, m_RecordingSize);
	DDX_Control(pDX, IDC_REC_ELAPSED, m_Elapsed);
	DDX_Text(pDX, IDC_REC_CHANNELS, m_Channels);
	DDV_MinMaxUInt(pDX, m_Channels, 1, 100);
	DDX_Check(pDX, IDC_REC_SYNC_PLAYBACK, m_SyncPlayback);
	DDX_Check(pDX, IDC_REC_OPEN_RECORDING, m_OpenRecording);
	//}}AFX_DATA_MAP
	ValidateSampleRate(pDX, IDC_REC_SAMPLE_RATE, m_SampleRateCombo, m_SampleRate);
	if (pDX->m_bSaveAndValidate)
		m_SampleSize = GetSampleSize(m_SampleSizeCombo);
}

BEGIN_MESSAGE_MAP(CRecordDlg, CPersistDlg)
	//{{AFX_MSG_MAP(CRecordDlg)
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
	ON_WM_CREATE()
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_REC_PAUSE, OnPause)
	ON_EN_KILLFOCUS(IDC_REC_CHANNELS, OnKillfocusChannels)
	ON_CBN_KILLFOCUS(IDC_REC_SAMPLE_RATE, OnKillfocusChannels)
	ON_CBN_SELCHANGE(IDC_REC_SAMPLE_RATE, OnKillfocusChannels)
	ON_CBN_SELCHANGE(IDC_REC_SAMPLE_SIZE, OnKillfocusChannels)
	//}}AFX_MSG_MAP
	ON_UPDATE_COMMAND_UI(IDC_REC_CHANNELS, OnUpdateFormat)
	ON_UPDATE_COMMAND_UI(IDC_REC_SAMPLE_RATE, OnUpdateFormat)
	ON_UPDATE_COMMAND_UI(IDC_REC_SAMPLE_SIZE, OnUpdateFormat)
	ON_UPDATE_COMMAND_UI(IDC_REC_CHANNELS_CAP, OnUpdateFormat)
	ON_UPDATE_COMMAND_UI(IDC_REC_SAMPLE_RATE_CAP, OnUpdateFormat)
	ON_UPDATE_COMMAND_UI(IDC_REC_SAMPLE_SIZE_CAP, OnUpdateFormat)
	ON_UPDATE_COMMAND_UI(IDC_REC_STATUS, OnUpdateStatus)
	ON_UPDATE_COMMAND_UI(IDOK, OnUpdateStop)
	ON_UPDATE_COMMAND_UI(IDC_REC_PAUSE, OnUpdatePause)
	ON_MESSAGE(UWM_CAPTURE_WRITE, OnCaptureWrite)
	ON_MESSAGE(UWM_CAPTURE_ERROR, OnCaptureError)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRecordDlg message handlers

int CRecordDlg::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CPersistDlg::OnCreate(lpCreateStruct) == -1)
		return -1;

	// create capture devices
	if (!CreateDevices())
		return -1;
	// create meter view
	CRuntimeClass	*pFactory = RUNTIME_CLASS(CMeterView);
	m_MeterView = STATIC_DOWNCAST(CMeterView, pFactory->CreateObject());
	DWORD	dwStyle = AFX_WS_DEFAULT_VIEW;
	CRect	r(0, 0, 0, 0);
    if (!m_MeterView->Create(NULL, NULL, dwStyle, r, this, 0, NULL))
		return -1;
	m_MeterView->SetWave(&m_MeterCapture.m_Wave);
	m_MeterView->SetTimerFrequency(TIMER_FREQUENCY);
	
	return 0;
}

BOOL CRecordDlg::OnInitDialog() 
{
	CPersistDlg::OnInitDialog();
	
	// save initial size
	CRect	rc;
	GetWindowRect(rc);
	m_InitSize = rc.Size();
	// set app icon and add document title to caption
	GetWindowText(m_InitCaption);
	CWaveShopApp::InitDialogCaptionView(*this, NULL);
	OnOutputPathChange();
	// init resizing
	m_Resize.AddControlList(this, m_CtrlList);
	// init combo boxes
	InitSampleRateCombo(m_SampleRateCombo, m_SampleRate);
	InitSampleSizeCombo(m_SampleSizeCombo, m_SampleSize);
	// init meter view
	GetDlgItem(IDC_REC_METER)->ShowWindow(SW_HIDE);	// hide placeholder
	ResizeMeterView();
	// init timer
	SetTimer(TIMER_ID, 1000 / TIMER_FREQUENCY, NULL);
	// init fonts
	HGDIOBJ	hFont = GetStockObject(SYSTEM_FONT);
	m_Elapsed.SendMessage(WM_SETFONT, WPARAM(hFont));
	// show free disk immediately to avoid timer lag
	UpdateFreeDiskSpace();
	UpdateUI();
	// start meter capture
	UpdateMeterCapture();
	// if one-touch activation, start recording now
	if (GetActivationType() == ACT_ONE_TOUCH) {
		if (!Start())	// if start fails
			EndDialog(IDABORT);	// close dialog ASAP
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CRecordDlg::OnDestroy() 
{
	CPersistDlg::OnDestroy();
	KillTimer(TIMER_ID);
	m_RecordCapture.Destroy();
	m_MeterCapture.Destroy();
}

void CRecordDlg::PostNcDestroy() 
{
	CPersistDlg::PostNcDestroy();
	// notify main window that modeless dialog was destroyed
	theApp.GetMain()->SendMessage(UWM_MODELESS_DESTROY, WPARAM(this));
	delete this;	// destroy this instance
}

void CRecordDlg::OnTimer(W64UINT nIDEvent) 
{
	if (m_MeterCapture.IsOpen()) {	// if meter capture in progress
		if (m_MeterCapture.Read()) {	// capture audio to wave container
			m_MeterView->TimerHook();	// update meter view from wave
			if (GetActivationType() == ACT_SOUND) {	// if sound activation
				double	MaxLevelNorm = m_MeterView->GetMaxLevel();
				double	MaxLevel = CMeterView::LevelToDecibels(MaxLevelNorm);
				const RECORD_PARMS& RecParms = GetRecParms();
				bool	ThresholdMet;
				bool	IsRec = IsRecording();
				if (IsRec)	// if recording
					ThresholdMet = MaxLevel < RecParms.StopLevel;
				else	// not recording
					ThresholdMet = MaxLevel >= RecParms.StartLevel;
				if (ThresholdMet) {	// if level meets threshold
					m_TriggerLen += 1000 / TIMER_FREQUENCY;
					float	duration; 
					if (IsRec)	// if recording
						duration = RecParms.StopDuration;
					else	// not recording
						duration = RecParms.StartDuration;
					// if level has met threshold for long enough
					if (m_TriggerLen > duration * 1000) {
						m_TriggerLen = 0;	// reset duration counter
						if (IsRec)	// if recording
							Stop(0);	// stop without closing dialog
						else	// not recording
							Start();	// start recording
					}
				} else	// level doesn't meet threshold
					m_TriggerLen = 0;	// reset duration counter
			}
		} else	// audio capture failed
			m_MeterCapture.Close();	// close to avoid multiple message dialogs
	}
	// while recording, OnCaptureWrite updates free disk space
	if (!IsRecording())	// if not recording
		UpdateFreeDiskSpace();	// update free disk space
	CPersistDlg::OnTimer(nIDEvent);
}

void CRecordDlg::OnSize(UINT nType, int cx, int cy) 
{
	CPersistDlg::OnSize(nType, cx, cy);
	m_Resize.OnSize();	// resize controls
	ResizeMeterView();	// resize meter view
}

void CRecordDlg::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI)
{
	lpMMI->ptMinTrackSize = CPoint(m_InitSize);
	CPersistDlg::OnGetMinMaxInfo(lpMMI);
}

void CRecordDlg::OnOK() 
{
	if (IsRecording())	// if recording
		Stop();	// stop recording
	else	// not recording
		Start();	// start recording
}

void CRecordDlg::OnCancel() 
{
	m_Canceled = TRUE;
	Stop();
}

void CRecordDlg::OnKillfocusChannels() 
{
	if (!m_Canceled && UpdateData())
		OnFormatChange();
}

void CRecordDlg::OnUpdateFormat(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(!IsRecording());
}

void CRecordDlg::OnUpdateStatus(CCmdUI* pCmdUI)
{
	int	iStatus = GetStatus();
	pCmdUI->SetText(LDS(m_StatusInfo[iStatus].Name));
}

void CRecordDlg::OnUpdateStop(CCmdUI* pCmdUI)
{
	int	nID = IsRecording() ? IDC_REC_STOP : IDC_REC_START;
	pCmdUI->SetText(LDS(nID));
}

void CRecordDlg::OnUpdatePause(CCmdUI* pCmdUI)
{
	int	iStatus = GetStatus();
	pCmdUI->SetText(LDS(m_StatusInfo[iStatus].PauseBtnText));
}

LRESULT CRecordDlg::OnCaptureWrite(WPARAM wParam, LPARAM lParam)
{
	ASSERT(IsRecording());
	// update elapsed time
	UINT	FrameSize = m_MeterCapture.m_Wave.GetFrameSize();
	LONGLONG	RecordedFrames = m_RecordCapture.GetOutputDataSize() / FrameSize;
	m_Elapsed.SetWindowText(
		CRulerCtrl::FormatTime(double(RecordedFrames) / m_SampleRate));
	// update recording size
	m_RecordingSize.SetWindowText(
		m_NumFormat.FormatByteSize(m_RecordCapture.GetOutputDataSize()));
	UpdateFreeDiskSpace();	// update free disk space
	return 0;
}

LRESULT CRecordDlg::OnCaptureError(WPARAM wParam, LPARAM lParam)
{
	Stop(SF_DEFAULT & ~SF_CONFIRM);	// stop recording; suppress confirmation dialog
	return 0;
}

HBRUSH CRecordDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	if (pWnd == &m_Status) {
		int	iStatus = GetStatus();
		if (iStatus) {
			COLORREF	BkColor = m_StatusInfo[iStatus].BkColor;
			SetDCBrushColor(pDC->m_hDC, BkColor);
			pDC->SetBkColor(BkColor);
			return HBRUSH(GetStockObject(DC_BRUSH));
		}
	}
	return CPersistDlg::OnCtlColor(pDC, pWnd, nCtlColor);
}

void CRecordDlg::OnPause()
{
	bool	paused = IsPaused();
	if (!paused) {
		UINT	style = MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2;
		int	retc = AfxMessageBox(IDS_REC_PAUSE_CHECK, style);
		if (retc != IDYES)
			return;
	}
	Pause(!paused);
}
