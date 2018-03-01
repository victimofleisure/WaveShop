# Microsoft Developer Studio Project File - Name="WaveShop" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=WaveShop - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "WaveShop.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "WaveShop.mak" CFG="WaveShop - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "WaveShop - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "WaveShop - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "WaveShop - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MT /W3 /GX /Zi /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /FR /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 version.lib shlwapi.lib imagehlp.lib dsound.lib dxguid.lib dxerr8.lib htmlhelp.lib /nologo /subsystem:windows /map /debug /machine:I386 /largeaddressaware
# SUBTRACT LINK32 /pdb:none /incremental:yes
# Begin Special Build Tool
OutDir=.\Release
WkspDir=.
SOURCE="$(InputPath)"
PostBuild_Desc=copying DLLs
PostBuild_Cmds=copy "$(WkspDir)\..\libsndfile\libsndfile-1.dll" $(OutDir) & copy "$(WkspDir)\..\libsamplerate\libsamplerate-0.dll" $(OutDir) & copy "$(WkspDir)\..\libmad\libmad.dll" $(OutDir) & copy "$(WkspDir)\..\libmp4ad\libmp4ad\Release\libmp4ad.dll" $(OutDir) & copy "$(WkspDir)\..\libid3tag\libid3tag.dll" $(OutDir)
# End Special Build Tool

!ELSEIF  "$(CFG)" == "WaveShop - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /FR /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 version.lib shlwapi.lib imagehlp.lib dsound.lib dxguid.lib dxerr8.lib htmlhelp.lib /nologo /subsystem:windows /map /debug /machine:I386 /pdbtype:sept /largeaddressaware
# SUBTRACT LINK32 /pdb:none
# Begin Special Build Tool
OutDir=.\Debug
WkspDir=.
SOURCE="$(InputPath)"
PostBuild_Desc=copying DLLs
PostBuild_Cmds=copy "$(WkspDir)\..\libsndfile\libsndfile-1.dll" $(OutDir) & copy "$(WkspDir)\..\libsamplerate\libsamplerate-0.dll" $(OutDir) & copy "$(WkspDir)\..\libmad\libmad.dll" $(OutDir) & copy "$(WkspDir)\..\libmp4ad\libmp4ad\Debug\libmp4ad.dll" $(OutDir) & copy "$(WkspDir)\..\libid3tag\libid3tag.dll" $(OutDir)
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "WaveShop - Win32 Release"
# Name "WaveShop - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\AboutDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\AboutDlg.h
# End Source File
# Begin Source File

SOURCE=.\AmplifyDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\AmplifyDlg.h
# End Source File
# Begin Source File

SOURCE=.\ArrayEx.h
# End Source File
# Begin Source File

SOURCE=.\AsyncJob.cpp
# End Source File
# Begin Source File

SOURCE=.\AsyncJob.h
# End Source File
# Begin Source File

SOURCE=.\Benchmark.cpp
# End Source File
# Begin Source File

SOURCE=.\Benchmark.h
# End Source File
# Begin Source File

SOURCE=.\ByteArrayEx.cpp
# End Source File
# Begin Source File

SOURCE=.\ByteArrayEx.h
# End Source File
# Begin Source File

SOURCE=.\ChangeFormatDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ChangeFormatDlg.h
# End Source File
# Begin Source File

SOURCE=.\ChannelBar.cpp
# End Source File
# Begin Source File

SOURCE=.\ChannelBar.h
# End Source File
# Begin Source File

SOURCE=.\ChannelRulerCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\ChannelRulerCtrl.h
# End Source File
# Begin Source File

SOURCE=.\ChildFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\ChildFrm.h
# End Source File
# Begin Source File

SOURCE=.\ClickSliderCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\ClickSliderCtrl.h
# End Source File
# Begin Source File

SOURCE=.\CoInitializer.h
# End Source File
# Begin Source File

SOURCE=.\CtrlResize.cpp
# End Source File
# Begin Source File

SOURCE=.\CtrlResize.h
# End Source File
# Begin Source File

SOURCE=.\DataTipCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\DataTipCtrl.h
# End Source File
# Begin Source File

SOURCE=.\DialogBarEx.cpp
# End Source File
# Begin Source File

SOURCE=.\DialogBarEx.h
# End Source File
# Begin Source File

SOURCE=.\DLLWrap.cpp
# End Source File
# Begin Source File

SOURCE=.\DLLWrap.h
# End Source File
# Begin Source File

SOURCE=.\DocIter.cpp
# End Source File
# Begin Source File

SOURCE=.\DocIter.h
# End Source File
# Begin Source File

SOURCE=.\DocManagerEx.cpp
# End Source File
# Begin Source File

SOURCE=.\DocManagerEx.h
# End Source File
# Begin Source File

SOURCE=.\DoubleBufDC.cpp
# End Source File
# Begin Source File

SOURCE=.\DoubleBufDC.h
# End Source File
# Begin Source File

SOURCE=.\DPoint.cpp
# End Source File
# Begin Source File

SOURCE=.\DPoint.h
# End Source File
# Begin Source File

SOURCE=.\DragRulerCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\DragRulerCtrl.h
# End Source File
# Begin Source File

SOURCE=.\DSCapture.cpp
# End Source File
# Begin Source File

SOURCE=.\DSCapture.h
# End Source File
# Begin Source File

SOURCE=.\DSPlayer.cpp
# End Source File
# Begin Source File

SOURCE=.\DSPlayer.h
# End Source File
# Begin Source File

SOURCE=.\EditSliderCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\EditSliderCtrl.h
# End Source File
# Begin Source File

SOURCE=.\EditSubitemListCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\EditSubitemListCtrl.h
# End Source File
# Begin Source File

SOURCE=.\Event.cpp
# End Source File
# Begin Source File

SOURCE=.\Event.h
# End Source File
# Begin Source File

SOURCE=.\ExportDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ExportDlg.h
# End Source File
# Begin Source File

SOURCE=.\FadeDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\FadeDlg.h
# End Source File
# Begin Source File

SOURCE=.\FileSearchDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\FileSearchDlg.h
# End Source File
# Begin Source File

SOURCE=.\FindClippingDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\FindClippingDlg.h
# End Source File
# Begin Source File

SOURCE=.\FindDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\FindDlg.h
# End Source File
# Begin Source File

SOURCE=.\FlatIconButton.cpp
# End Source File
# Begin Source File

SOURCE=.\FlatIconButton.h
# End Source File
# Begin Source File

SOURCE=.\FocusEdit.cpp
# End Source File
# Begin Source File

SOURCE=.\FocusEdit.h
# End Source File
# Begin Source File

SOURCE=.\FolderDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\FolderDialog.h
# End Source File
# Begin Source File

SOURCE=.\Globals.h
# End Source File
# Begin Source File

SOURCE=.\HelpIDs.h
# End Source File
# Begin Source File

SOURCE=.\HelpResMap.h
# End Source File
# Begin Source File

SOURCE=.\HistoryBar.cpp
# End Source File
# Begin Source File

SOURCE=.\HistoryBar.h
# End Source File
# Begin Source File

SOURCE=.\Hyperlink.cpp
# End Source File
# Begin Source File

SOURCE=.\Hyperlink.h
# End Source File
# Begin Source File

SOURCE=.\id3tag.h
# End Source File
# Begin Source File

SOURCE=.\id3tagDefs.h
# End Source File
# Begin Source File

SOURCE=.\id3tagFuncs.h
# End Source File
# Begin Source File

SOURCE=.\ID3TagWrap.cpp
# End Source File
# Begin Source File

SOURCE=.\ID3TagWrap.h
# End Source File
# Begin Source File

SOURCE=.\InsertSilenceDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\InsertSilenceDlg.h
# End Source File
# Begin Source File

SOURCE=.\kiss_fft\kiss_fft.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\kiss_fft\kiss_fft.h
# End Source File
# Begin Source File

SOURCE=.\kiss_fft\kiss_fftr.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\kiss_fft\kiss_fftr.h
# End Source File
# Begin Source File

SOURCE=.\KissFFT.h
# End Source File
# Begin Source File

SOURCE=.\KissFFTWrap.cpp
# End Source File
# Begin Source File

SOURCE=.\ladspa.h
# End Source File
# Begin Source File

SOURCE=.\lame.h
# End Source File
# Begin Source File

SOURCE=.\lameDefs.h
# End Source File
# Begin Source File

SOURCE=.\lameFuncs.h
# End Source File
# Begin Source File

SOURCE=.\LameWrap.cpp
# End Source File
# Begin Source File

SOURCE=.\LameWrap.h
# End Source File
# Begin Source File

SOURCE=.\libmp4ad.h
# End Source File
# Begin Source File

SOURCE=.\mad.h
# End Source File
# Begin Source File

SOURCE=.\madDefs.h
# End Source File
# Begin Source File

SOURCE=.\madFuncs.h
# End Source File
# Begin Source File

SOURCE=.\MadWrap.cpp
# End Source File
# Begin Source File

SOURCE=.\MadWrap.h
# End Source File
# Begin Source File

SOURCE=.\MainFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\MainFrm.h
# End Source File
# Begin Source File

SOURCE=.\MetadataDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\MetadataDlg.h
# End Source File
# Begin Source File

SOURCE=.\MetadataStr.h
# End Source File
# Begin Source File

SOURCE=.\MeterBar.cpp
# End Source File
# Begin Source File

SOURCE=.\MeterBar.h
# End Source File
# Begin Source File

SOURCE=.\MeterView.cpp
# End Source File
# Begin Source File

SOURCE=.\MeterView.h
# End Source File
# Begin Source File

SOURCE=.\MissingLibraryDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\MissingLibraryDlg.h
# End Source File
# Begin Source File

SOURCE=.\MP3EncoderDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\MP3EncoderDlg.h
# End Source File
# Begin Source File

SOURCE=.\MP4DecodeDefs.h
# End Source File
# Begin Source File

SOURCE=.\MP4DecodeFuncs.h
# End Source File
# Begin Source File

SOURCE=.\MP4DecodeWrap.cpp
# End Source File
# Begin Source File

SOURCE=.\MP4DecodeWrap.h
# End Source File
# Begin Source File

SOURCE=.\MySizingControlBar.cpp
# End Source File
# Begin Source File

SOURCE=.\MySizingControlBar.h
# End Source File
# Begin Source File

SOURCE=.\NavBar.cpp
# End Source File
# Begin Source File

SOURCE=.\NavBar.h
# End Source File
# Begin Source File

SOURCE=.\neaacdec.h
# End Source File
# Begin Source File

SOURCE=.\NormalizeDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\NormalizeDlg.h
# End Source File
# Begin Source File

SOURCE=.\NumEdit.cpp
# End Source File
# Begin Source File

SOURCE=.\NumEdit.h
# End Source File
# Begin Source File

SOURCE=.\NumFormat.cpp
# End Source File
# Begin Source File

SOURCE=.\NumFormat.h
# End Source File
# Begin Source File

SOURCE=.\OptionsDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\OptionsDlg.h
# End Source File
# Begin Source File

SOURCE=.\OptionsInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\OptionsInfo.h
# End Source File
# Begin Source File

SOURCE=.\OptionsPages.h
# End Source File
# Begin Source File

SOURCE=.\OptsAudioDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\OptsAudioDlg.h
# End Source File
# Begin Source File

SOURCE=.\OptsEditDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\OptsEditDlg.h
# End Source File
# Begin Source File

SOURCE=.\OptsRecordDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\OptsRecordDlg.h
# End Source File
# Begin Source File

SOURCE=.\OptsRTSADlg.cpp
# End Source File
# Begin Source File

SOURCE=.\OptsRTSADlg.h
# End Source File
# Begin Source File

SOURCE=.\OptsViewDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\OptsViewDlg.h
# End Source File
# Begin Source File

SOURCE=.\Oscillator.cpp
# End Source File
# Begin Source File

SOURCE=.\Oscillator.h
# End Source File
# Begin Source File

SOURCE=.\PathStr.cpp
# End Source File
# Begin Source File

SOURCE=.\PathStr.h
# End Source File
# Begin Source File

SOURCE=.\PeakStatsDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\PeakStatsDlg.h
# End Source File
# Begin Source File

SOURCE=.\Persist.cpp
# End Source File
# Begin Source File

SOURCE=.\Persist.h
# End Source File
# Begin Source File

SOURCE=.\PersistDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\PersistDlg.h
# End Source File
# Begin Source File

SOURCE=.\PitchBar.cpp
# End Source File
# Begin Source File

SOURCE=.\PitchBar.h
# End Source File
# Begin Source File

SOURCE=.\PlotCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\PlotCtrl.h
# End Source File
# Begin Source File

SOURCE=.\Plugin.cpp
# End Source File
# Begin Source File

SOURCE=.\Plugin.h
# End Source File
# Begin Source File

SOURCE=.\PluginManager.cpp
# End Source File
# Begin Source File

SOURCE=.\PluginManager.h
# End Source File
# Begin Source File

SOURCE=.\PluginParamDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\PluginParamDlg.h
# End Source File
# Begin Source File

SOURCE=.\PluginParamRow.cpp
# End Source File
# Begin Source File

SOURCE=.\PluginParamRow.h
# End Source File
# Begin Source File

SOURCE=.\PluginParamView.cpp
# End Source File
# Begin Source File

SOURCE=.\PluginParamView.h
# End Source File
# Begin Source File

SOURCE=.\ProgressDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ProgressDlg.h
# End Source File
# Begin Source File

SOURCE=.\RandList.cpp
# End Source File
# Begin Source File

SOURCE=.\RandList.h
# End Source File
# Begin Source File

SOURCE=.\Range.h
# End Source File
# Begin Source File

SOURCE=.\RecordDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\RecordDlg.h
# End Source File
# Begin Source File

SOURCE=.\RefPtr.h
# End Source File
# Begin Source File

SOURCE=.\ReportCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\ReportCtrl.h
# End Source File
# Begin Source File

SOURCE=.\ResampleDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ResampleDlg.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\ResultsBar.cpp
# End Source File
# Begin Source File

SOURCE=.\ResultsBar.h
# End Source File
# Begin Source File

SOURCE=.\ResultsReportCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\ResultsReportCtrl.h
# End Source File
# Begin Source File

SOURCE=.\RingBuf.h
# End Source File
# Begin Source File

SOURCE=.\RMSStatsDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\RMSStatsDlg.h
# End Source File
# Begin Source File

SOURCE=.\Round.h
# End Source File
# Begin Source File

SOURCE=.\RulerBar.cpp
# End Source File
# Begin Source File

SOURCE=.\RulerBar.h
# End Source File
# Begin Source File

SOURCE=.\RulerCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\RulerCtrl.h
# End Source File
# Begin Source File

SOURCE=.\samplerate.h
# End Source File
# Begin Source File

SOURCE=.\samplerateDefs.h
# End Source File
# Begin Source File

SOURCE=.\SampleRateEx.cpp
# End Source File
# Begin Source File

SOURCE=.\SampleRateEx.h
# End Source File
# Begin Source File

SOURCE=.\samplerateFuncs.h
# End Source File
# Begin Source File

SOURCE=.\scbarg.cpp
# End Source File
# Begin Source File

SOURCE=.\scbarg.h
# End Source File
# Begin Source File

SOURCE=.\sizecbar.cpp
# End Source File
# Begin Source File

SOURCE=.\sizecbar.h
# End Source File
# Begin Source File

SOURCE=.\sndfile.h
# End Source File
# Begin Source File

SOURCE=.\sndfileDefs.h
# End Source File
# Begin Source File

SOURCE=.\SndFileEx.cpp
# End Source File
# Begin Source File

SOURCE=.\SndFileEx.h
# End Source File
# Begin Source File

SOURCE=.\SndFileFormat.cpp
# End Source File
# Begin Source File

SOURCE=.\SndFileFormat.h
# End Source File
# Begin Source File

SOURCE=.\sndfileFuncs.h
# End Source File
# Begin Source File

SOURCE=.\SortArray.h
# End Source File
# Begin Source File

SOURCE=.\SortStringArray.cpp
# End Source File
# Begin Source File

SOURCE=.\SortStringArray.h
# End Source File
# Begin Source File

SOURCE=.\SpeakerDef.h
# End Source File
# Begin Source File

SOURCE=.\SpeakersDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\SpeakersDlg.h
# End Source File
# Begin Source File

SOURCE=.\SpectrumAnal.cpp
# End Source File
# Begin Source File

SOURCE=.\SpectrumAnal.h
# End Source File
# Begin Source File

SOURCE=.\SpectrumBar.cpp
# End Source File
# Begin Source File

SOURCE=.\SpectrumBar.h
# End Source File
# Begin Source File

SOURCE=.\SpectrumDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\SpectrumDlg.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\SubFileFind.cpp
# End Source File
# Begin Source File

SOURCE=.\SubFileFind.h
# End Source File
# Begin Source File

SOURCE=.\SwapChannelsDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\SwapChannelsDlg.h
# End Source File
# Begin Source File

SOURCE=.\SwatchButton.cpp
# End Source File
# Begin Source File

SOURCE=.\SwatchButton.h
# End Source File
# Begin Source File

SOURCE=.\SweepDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\SweepDlg.h
# End Source File
# Begin Source File

SOURCE=.\TimeRulerBar.cpp
# End Source File
# Begin Source File

SOURCE=.\TimeRulerBar.h
# End Source File
# Begin Source File

SOURCE=.\TimeRulerCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\TimeRulerCtrl.h
# End Source File
# Begin Source File

SOURCE=.\Undoable.cpp
# End Source File
# Begin Source File

SOURCE=.\Undoable.h
# End Source File
# Begin Source File

SOURCE=.\UndoCodeData.h
# End Source File
# Begin Source File

SOURCE=.\UndoCodes.h
# End Source File
# Begin Source File

SOURCE=.\UndoManager.cpp
# End Source File
# Begin Source File

SOURCE=.\UndoManager.h
# End Source File
# Begin Source File

SOURCE=.\UndoState.cpp
# End Source File
# Begin Source File

SOURCE=.\UndoState.h
# End Source File
# Begin Source File

SOURCE=.\UndoTest.cpp
# End Source File
# Begin Source File

SOURCE=.\UndoTest.h
# End Source File
# Begin Source File

SOURCE=.\Version.h
# End Source File
# Begin Source File

SOURCE=.\VersionInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\VersionInfo.h
# End Source File
# Begin Source File

SOURCE=.\ViewColors.h
# End Source File
# Begin Source File

SOURCE=.\ViewColorsDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ViewColorsDlg.h
# End Source File
# Begin Source File

SOURCE=.\VolumeBar.cpp
# End Source File
# Begin Source File

SOURCE=.\VolumeBar.h
# End Source File
# Begin Source File

SOURCE=.\Wave.cpp
# End Source File
# Begin Source File

SOURCE=.\Wave.h
# End Source File
# Begin Source File

SOURCE=.\WaveEdit.cpp
# End Source File
# Begin Source File

SOURCE=.\WaveEdit.h
# End Source File
# Begin Source File

SOURCE=.\WaveGenDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\WaveGenDlg.h
# End Source File
# Begin Source File

SOURCE=.\WaveGenOscDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\WaveGenOscDlg.h
# End Source File
# Begin Source File

SOURCE=.\WaveGenParms.h
# End Source File
# Begin Source File

SOURCE=.\WaveProcess.cpp
# End Source File
# Begin Source File

SOURCE=.\WaveProcess.h
# End Source File
# Begin Source File

SOURCE=.\WaveShop.cpp
# End Source File
# Begin Source File

SOURCE=.\WaveShop.h
# End Source File
# Begin Source File

SOURCE=.\WaveShop.rc
# End Source File
# Begin Source File

SOURCE=.\WaveShopDoc.cpp
# End Source File
# Begin Source File

SOURCE=.\WaveShopDoc.h
# End Source File
# Begin Source File

SOURCE=.\WaveShopView.cpp
# End Source File
# Begin Source File

SOURCE=.\WaveShopView.h
# End Source File
# Begin Source File

SOURCE=.\Win32Console.cpp
# End Source File
# Begin Source File

SOURCE=.\Win32Console.h
# End Source File
# Begin Source File

SOURCE=.\WinAppEx.cpp
# End Source File
# Begin Source File

SOURCE=.\WinAppEx.h
# End Source File
# Begin Source File

SOURCE=.\WindowFuncData.h
# End Source File
# Begin Source File

SOURCE=.\WObject.h
# End Source File
# Begin Source File

SOURCE=.\WorkerThread.cpp
# End Source File
# Begin Source File

SOURCE=.\WorkerThread.h
# End Source File
# Begin Source File

SOURCE=.\Wrapx64.h
# End Source File
# Begin Source File

SOURCE=.\ZoomRulerCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\ZoomRulerCtrl.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\header_sort.bmp
# End Source File
# Begin Source File

SOURCE=.\res\history_pos.bmp
# End Source File
# Begin Source File

SOURCE=.\res\mute_down.ico
# End Source File
# Begin Source File

SOURCE=.\res\mute_up.ico
# End Source File
# Begin Source File

SOURCE=.\res\nav_show_length.ico
# End Source File
# Begin Source File

SOURCE=.\res\tool_zoom.cur
# End Source File
# Begin Source File

SOURCE=.\res\tool_zoom_in.cur
# End Source File
# Begin Source File

SOURCE=.\res\tool_zoom_out.cur
# End Source File
# Begin Source File

SOURCE=.\res\Toolbar.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Transport.bmp
# End Source File
# Begin Source File

SOURCE=.\res\WaveShop.ico
# End Source File
# Begin Source File

SOURCE=.\res\WaveShop.rc2
# End Source File
# Begin Source File

SOURCE=.\res\WaveShopDoc.ico
# End Source File
# End Group
# End Target
# End Project
