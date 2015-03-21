# Microsoft Developer Studio Generated NMAKE File, Format Version 4.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

!IF "$(CFG)" == ""
CFG=BEOSWin - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to BEOSWin - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "BEOSWin - Win32 Release" && "$(CFG)" !=\
 "BEOSWin - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "BEOSWin.mak" CFG="BEOSWin - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "BEOSWin - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "BEOSWin - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 
################################################################################
# Begin Project
# PROP Target_Last_Scanned "BEOSWin - Win32 Debug"
RSC=rc.exe
MTL=mktyplib.exe
CPP=cl.exe

!IF  "$(CFG)" == "BEOSWin - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
OUTDIR=.\Release
INTDIR=.\Release

ALL : "$(OUTDIR)\BEOSWin.exe"

CLEAN : 
	-@erase ".\Release\BEOSWin.exe"
	-@erase ".\Release\shortcut.obj"
	-@erase ".\Release\revision.obj"
	-@erase ".\Release\install.obj"
	-@erase ".\Release\dialog.obj"
	-@erase ".\Release\utils.obj"
	-@erase ".\Release\beoswin.obj"
	-@erase ".\Release\mainwnd.obj"
	-@erase ".\Release\beoswin.res"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /W3 /GX /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /ML /W3 /GX /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/BEOSWin.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Release/
CPP_SBRS=
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
RSC_PROJ=/l 0x809 /fo"$(INTDIR)/beoswin.res" /d "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/BEOSWin.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:windows /incremental:no\
 /pdb:"$(OUTDIR)/BEOSWin.pdb" /machine:I386 /out:"$(OUTDIR)/BEOSWin.exe" 
LINK32_OBJS= \
	"$(INTDIR)/shortcut.obj" \
	"$(INTDIR)/revision.obj" \
	"$(INTDIR)/install.obj" \
	"$(INTDIR)/dialog.obj" \
	"$(INTDIR)/utils.obj" \
	"$(INTDIR)/beoswin.obj" \
	"$(INTDIR)/mainwnd.obj" \
	"$(INTDIR)/beoswin.res" \
	"..\Msdevstd\Lib\Comctl32.lib"

"$(OUTDIR)\BEOSWin.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "BEOSWin - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
OUTDIR=.\Debug
INTDIR=.\Debug

ALL : "$(OUTDIR)\BEOSWin.exe"

CLEAN : 
	-@erase ".\Debug\vc40.pdb"
	-@erase ".\Debug\vc40.idb"
	-@erase ".\Debug\BEOSWin.exe"
	-@erase ".\Debug\dialog.obj"
	-@erase ".\Debug\shortcut.obj"
	-@erase ".\Debug\revision.obj"
	-@erase ".\Debug\utils.obj"
	-@erase ".\Debug\install.obj"
	-@erase ".\Debug\mainwnd.obj"
	-@erase ".\Debug\beoswin.obj"
	-@erase ".\Debug\beoswin.res"
	-@erase ".\Debug\BEOSWin.ilk"
	-@erase ".\Debug\BEOSWin.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /MLd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/BEOSWin.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\Debug/
CPP_SBRS=
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
RSC_PROJ=/l 0x809 /fo"$(INTDIR)/beoswin.res" /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/BEOSWin.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:windows /incremental:yes\
 /pdb:"$(OUTDIR)/BEOSWin.pdb" /debug /machine:I386 /out:"$(OUTDIR)/BEOSWin.exe" 
LINK32_OBJS= \
	"$(INTDIR)/dialog.obj" \
	"$(INTDIR)/shortcut.obj" \
	"$(INTDIR)/revision.obj" \
	"$(INTDIR)/utils.obj" \
	"$(INTDIR)/install.obj" \
	"$(INTDIR)/mainwnd.obj" \
	"$(INTDIR)/beoswin.obj" \
	"$(INTDIR)/beoswin.res" \
	"..\Msdevstd\Lib\Comctl32.lib"

"$(OUTDIR)\BEOSWin.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

################################################################################
# Begin Target

# Name "BEOSWin - Win32 Release"
# Name "BEOSWin - Win32 Debug"

!IF  "$(CFG)" == "BEOSWin - Win32 Release"

!ELSEIF  "$(CFG)" == "BEOSWin - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\utils.c
DEP_CPP_UTILS=\
	".\beoswin.h"\
	

"$(INTDIR)\utils.obj" : $(SOURCE) $(DEP_CPP_UTILS) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\install.c
DEP_CPP_INSTA=\
	".\beoswin.h"\
	

"$(INTDIR)\install.obj" : $(SOURCE) $(DEP_CPP_INSTA) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\mainwnd.c
DEP_CPP_MAINW=\
	".\beoswin.h"\
	

"$(INTDIR)\mainwnd.obj" : $(SOURCE) $(DEP_CPP_MAINW) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\resource.h

!IF  "$(CFG)" == "BEOSWin - Win32 Release"

!ELSEIF  "$(CFG)" == "BEOSWin - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\revision.c

"$(INTDIR)\revision.obj" : $(SOURCE) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\shortcut.c
DEP_CPP_SHORT=\
	".\beoswin.h"\
	

"$(INTDIR)\shortcut.obj" : $(SOURCE) $(DEP_CPP_SHORT) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\dialog.c
DEP_CPP_DIALO=\
	".\beoswin.h"\
	

"$(INTDIR)\dialog.obj" : $(SOURCE) $(DEP_CPP_DIALO) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\beoswin.c
DEP_CPP_BEOSW=\
	".\beoswin.h"\
	

"$(INTDIR)\beoswin.obj" : $(SOURCE) $(DEP_CPP_BEOSW) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\beoswin.h

!IF  "$(CFG)" == "BEOSWin - Win32 Release"

!ELSEIF  "$(CFG)" == "BEOSWin - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\Msdevstd\Lib\Comctl32.lib

!IF  "$(CFG)" == "BEOSWin - Win32 Release"

!ELSEIF  "$(CFG)" == "BEOSWin - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\beoswin.rc
DEP_RSC_BEOSWI=\
	".\bitmap1.bmp"\
	".\ico00002.ico"\
	".\ico00003.ico"\
	".\icon1.ico"\
	".\ico00001.ico"\
	".\ico00004.ico"\
	".\def16.ico"\
	".\ico00005.ico"\
	

"$(INTDIR)\beoswin.res" : $(SOURCE) $(DEP_RSC_BEOSWI) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


# End Source File
# End Target
# End Project
################################################################################
