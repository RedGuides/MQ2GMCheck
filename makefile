!include "../global.mak"

ALL : "$(OUTDIR)\MQ2GMCheck.dll"

CLEAN :
	-@erase "$(INTDIR)\MQ2GMCheck.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\MQ2GMCheck.dll"
	-@erase "$(OUTDIR)\MQ2GMCheck.exp"
	-@erase "$(OUTDIR)\MQ2GMCheck.lib"
	-@erase "$(OUTDIR)\MQ2GMCheck.pdb"


LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib $(DETLIB) ..\Release\MQ2Main.lib /nologo /dll /incremental:no /pdb:"$(OUTDIR)\MQ2GMCheck.pdb" /debug /machine:I386 /out:"$(OUTDIR)\MQ2GMCheck.dll" /implib:"$(OUTDIR)\MQ2GMCheck.lib" /OPT:NOICF /OPT:NOREF 
LINK32_OBJS= \
	"$(INTDIR)\MQ2GMCheck.obj" \
	"$(OUTDIR)\MQ2Main.lib"

"$(OUTDIR)\MQ2GMCheck.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) $(LINK32_FLAGS) $(LINK32_OBJS)


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("MQ2GMCheck.dep")
!INCLUDE "MQ2GMCheck.dep"
!ELSE 
!MESSAGE Warning: cannot find "MQ2GMCheck.dep"
!ENDIF 
!ENDIF 


SOURCE=.\MQ2GMCheck.cpp

"$(INTDIR)\MQ2GMCheck.obj" : $(SOURCE) "$(INTDIR)"

