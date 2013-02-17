; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CHexvisDlg
LastTemplate=CRichEditCtrl
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "hexvis.h"

ClassCount=4
Class1=CHexvisApp
Class2=CHexvisDlg
Class3=CAboutDlg

ResourceCount=3
Resource1=IDD_ABOUTBOX
Resource2=IDR_MAINFRAME
Class4=HexRichEdit
Resource3=IDD_HEXVIS_DIALOG

[CLS:CHexvisApp]
Type=0
HeaderFile=hexvis.h
ImplementationFile=hexvis.cpp
Filter=N

[CLS:CHexvisDlg]
Type=0
HeaderFile=hexvisDlg.h
ImplementationFile=hexvisDlg.cpp
Filter=D
BaseClass=CDialog
VirtualFilter=dWC
LastObject=CHexvisDlg

[CLS:CAboutDlg]
Type=0
HeaderFile=hexvisDlg.h
ImplementationFile=hexvisDlg.cpp
Filter=D

[DLG:IDD_ABOUTBOX]
Type=1
Class=CAboutDlg
ControlCount=4
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATIC,static,1342308480
Control3=IDC_STATIC,static,1342308352
Control4=IDOK,button,1342373889

[DLG:IDD_HEXVIS_DIALOG]
Type=1
Class=CHexvisDlg
ControlCount=31
Control1=IDC_PACKET_BODY,RICHEDIT,1353781700
Control2=IDC_PROCESS,button,1342242817
Control3=IDC_CLEAR,button,1342242816
Control4=IDC_LOAD_FILE,button,1342242816
Control5=IDC_SEARCH_NUMBER,edit,1350631552
Control6=IDC_FIND_NUMBER,button,1342242816
Control7=IDC_STATIC,button,1342177287
Control8=IDC_STATIC,static,1342308352
Control9=IDC_BYTE,edit,1350633600
Control10=IDC_STATIC,static,1342308352
Control11=IDC_SHORT_BE,edit,1350633600
Control12=IDC_SHORT_LE,edit,1350633600
Control13=IDC_STATIC,static,1342308352
Control14=IDC_LONG_BE,edit,1350633600
Control15=IDC_LONG_LE,edit,1350633600
Control16=IDC_STATIC,static,1342308352
Control17=IDC_UNIX_TIME,edit,1350633600
Control18=IDC_STATIC,static,1342308352
Control19=IDC_REAL,edit,1350633600
Control20=IDC_STATIC,static,1342308352
Control21=IDC_INT64_BE,edit,1350633600
Control22=IDC_INT64_LE,edit,1350633600
Control23=IDC_STATIC,static,1342308352
Control24=IDC_STRING,edit,1350633600
Control25=IDC_STATIC,static,1342308352
Control26=IDC_OFFSET,edit,1350633600
Control27=IDC_STATIC,static,1342308352
Control28=IDC_STATIC,static,1342308352
Control29=IDC_BIN_REP,edit,1350633600
Control30=IDC_STATE,static,1342308352
Control31=IDC_COPY_TO_CLIP,button,1342242816

[CLS:HexRichEdit]
Type=0
HeaderFile=HexRichEdit.h
ImplementationFile=HexRichEdit.cpp
BaseClass=CRichEditCtrl
Filter=W
VirtualFilter=WC
LastObject=HexRichEdit

