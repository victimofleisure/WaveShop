; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CRulerBar
LastTemplate=CScrollBar
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "waveshop.h"
LastPage=0

ClassCount=7
Class1=CAboutDlg
Class2=CChildFrame
Class3=CMainFrame
Class4=CWaveShopApp
Class5=CWaveShopDoc
Class6=CWaveShopView

ResourceCount=3
Resource1=IDR_WAVVIETYPE
Resource2=IDR_MAINFRAME
Class7=CRulerBar
Resource3=IDD_ABOUTBOX

[CLS:CAboutDlg]
Type=0
BaseClass=CDialog
HeaderFile=AboutDlg.h
ImplementationFile=AboutDlg.cpp
LastObject=CAboutDlg

[CLS:CChildFrame]
Type=0
BaseClass=CMDIChildWnd
HeaderFile=ChildFrm.h
ImplementationFile=ChildFrm.cpp
LastObject=CChildFrame
Filter=M
VirtualFilter=mfWC

[CLS:CMainFrame]
Type=0
BaseClass=CMDIFrameWnd
HeaderFile=MainFrm.h
ImplementationFile=MainFrm.cpp
LastObject=CMainFrame
Filter=T
VirtualFilter=fWC

[CLS:CWaveShopApp]
Type=0
BaseClass=CWinApp
HeaderFile=WaveShop.h
ImplementationFile=WaveShop.cpp
LastObject=CWaveShopApp

[CLS:CWaveShopDoc]
Type=0
BaseClass=CDocument
HeaderFile=WaveShopDoc.h
ImplementationFile=WaveShopDoc.cpp
LastObject=CWaveShopDoc
Filter=N
VirtualFilter=DC

[CLS:CWaveShopView]
Type=0
BaseClass=CScrollView
HeaderFile=WaveShopView.h
ImplementationFile=WaveShopView.cpp
LastObject=CWaveShopView
Filter=C
VirtualFilter=VWC

[DLG:IDD_ABOUTBOX]
Type=1
Class=CAboutDlg
ControlCount=4
Control1=IDC_STATIC,static,1342177283
Control2=IDC_ABOUT_TEXT,static,1342308480
Control3=IDOK,button,1342373889
Control4=IDC_ABOUT_LICENSE,edit,1352730628

[TB:IDR_MAINFRAME]
Type=1
Class=?
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_SAVE
Command4=ID_APP_ABOUT
CommandCount=4

[MNU:IDR_MAINFRAME]
Type=1
Class=?
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_MRU_FILE1
Command4=ID_APP_EXIT
Command5=ID_VIEW_TOOLBAR
Command6=ID_VIEW_STATUS_BAR
Command7=ID_APP_ABOUT
CommandCount=7

[MNU:IDR_WAVVIETYPE]
Type=1
Class=?
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_CLOSE
Command4=ID_FILE_SAVE
Command5=ID_FILE_SAVE_AS
Command6=ID_FILE_MRU_FILE1
Command7=ID_APP_EXIT
Command8=ID_VIEW_TOOLBAR
Command9=ID_VIEW_STATUS_BAR
Command10=ID_VIEW_ZOOM_IN
Command11=ID_VIEW_ZOOM_OUT
Command12=ID_VIEW_FIT_WINDOW
Command13=ID_WINDOW_NEW
Command14=ID_WINDOW_CASCADE
Command15=ID_WINDOW_TILE_HORZ
Command16=ID_WINDOW_ARRANGE
Command17=ID_APP_ABOUT
CommandCount=17

[ACL:IDR_MAINFRAME]
Type=1
Class=?
Command1=ID_EDIT_COPY
Command2=ID_FILE_NEW
Command3=ID_FILE_OPEN
Command4=ID_FILE_PRINT
Command5=ID_FILE_SAVE
Command6=ID_EDIT_PASTE
Command7=ID_EDIT_UNDO
Command8=ID_EDIT_CUT
Command9=ID_VIEW_REFRESH
Command10=ID_NEXT_PANE
Command11=ID_PREV_PANE
Command12=ID_EDIT_COPY
Command13=ID_EDIT_PASTE
Command14=ID_EDIT_CUT
Command15=ID_EDIT_UNDO
CommandCount=15

[CLS:CRulerBar]
Type=0
HeaderFile=RulerBar.h
ImplementationFile=RulerBar.cpp
BaseClass=CControlBar
Filter=W
LastObject=CRulerBar
VirtualFilter=WC

