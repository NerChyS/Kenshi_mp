; Kenshi Online installer (multiplayer mod) - NSIS 3 - EN / ES / RU
Unicode true
SetCompressor /SOLID lzma
RequestExecutionLevel admin
ManifestDPIAware true

!define APPNAME "Kenshi Online"
!define APPVER  "0.3.3-alpha"
!define UNINSTKEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\KenshiOnline"
!define UNINSTEXE "Uninstall Kenshi Online.exe"

Name "${APPNAME}"
Caption "${APPNAME} ${APPVER}"
OutFile "KenshiOnline_Setup.exe"
InstallDir "$PROGRAMFILES32\Steam\steamapps\common\Kenshi"
BrandingText "Kenshi Online ${APPVER}"
VIProductVersion "0.3.1.0"
VIAddVersionKey /LANG=1033 "ProductName" "${APPNAME}"
VIAddVersionKey /LANG=1033 "FileDescription" "${APPNAME} installer"
VIAddVersionKey /LANG=1033 "FileVersion" "${APPVER}"
VIAddVersionKey /LANG=1033 "ProductVersion" "${APPVER}"
VIAddVersionKey /LANG=1033 "LegalCopyright" "MIT - Kenshi Online contributors / The404Studios"

!include "MUI2.nsh"
!include "LogicLib.nsh"
!include "FileFunc.nsh"

!define MUI_ABORTWARNING
!define MUI_LANGDLL_ALLLANGUAGES
!define MUI_WELCOMEPAGE_TITLE "$(T_WELCOME_TITLE)"
!define MUI_WELCOMEPAGE_TEXT "$(T_WELCOME_TEXT)"
!insertmacro MUI_PAGE_WELCOME

!define MUI_DIRECTORYPAGE_TEXT_TOP "$(T_DIR_TOP)"
!define MUI_DIRECTORYPAGE_TEXT_DESTINATION "$(T_DIR_DEST)"
!define MUI_PAGE_CUSTOMFUNCTION_LEAVE DirLeave
!insertmacro MUI_PAGE_DIRECTORY

!insertmacro MUI_PAGE_INSTFILES

!define MUI_FINISHPAGE_TITLE "$(T_FINISH_TITLE)"
!define MUI_FINISHPAGE_TEXT "$(T_FINISH_TEXT)"
!define MUI_FINISHPAGE_RUN
!define MUI_FINISHPAGE_RUN_TEXT "$(T_FINISH_RUN)"
!define MUI_FINISHPAGE_RUN_FUNCTION LaunchKenshi
!define MUI_FINISHPAGE_RUN_NOTCHECKED
!define MUI_FINISHPAGE_SHOWREADME "$INSTDIR\KenshiOnline\$(T_GUIDE)"
!define MUI_FINISHPAGE_SHOWREADME_TEXT "$(T_FINISH_GUIDE)"
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_LANGUAGE "English"
!insertmacro MUI_LANGUAGE "Spanish"
!insertmacro MUI_LANGUAGE "Russian"
!insertmacro MUI_RESERVEFILE_LANGDLL

LangString T_WELCOME_TITLE ${LANG_ENGLISH} "Install Kenshi Online"
LangString T_WELCOME_TITLE ${LANG_SPANISH} "Instalar Kenshi Online"
LangString T_WELCOME_TITLE ${LANG_RUSSIAN} "Установка Kenshi Online"
LangString T_WELCOME_TEXT ${LANG_ENGLISH} "This wizard installs the Kenshi Online multiplayer mod (up to 16 players) for Kenshi 1.0.68.$\r$\n$\r$\nClose Kenshi before continuing.$\r$\n$\r$\nClick Next to continue."
LangString T_WELCOME_TEXT ${LANG_SPANISH} "Este asistente instala el mod multijugador Kenshi Online (hasta 16 jugadores) para Kenshi 1.0.68.$\r$\n$\r$\nAntes de seguir, cierra Kenshi.$\r$\n$\r$\nPulsa Siguiente para continuar."
LangString T_WELCOME_TEXT ${LANG_RUSSIAN} "Мастер установит мод мультиплеера Kenshi Online (до 16 игроков) для Kenshi 1.0.68.$\r$\n$\r$\nПеред продолжением закройте Kenshi.$\r$\n$\r$\nНажмите «Далее»."
LangString T_DIR_TOP ${LANG_ENGLISH} "Choose the Kenshi folder: the one that contains kenshi_x64.exe.$\r$\n$\r$\nNot sure where it is? In Steam: right-click Kenshi > Manage > Browse local files.$\r$\n$\r$\nThe Install button is enabled only when the folder is correct."
LangString T_DIR_TOP ${LANG_SPANISH} "Elige la carpeta de Kenshi: la que contiene kenshi_x64.exe.$\r$\n$\r$\nSi no sabes dónde está: en Steam, clic derecho en Kenshi > Administrar > Ver archivos locales.$\r$\n$\r$\nEl botón Instalar solo se activa cuando la carpeta es la correcta."
LangString T_DIR_TOP ${LANG_RUSSIAN} "Выберите папку Kenshi — ту, где лежит kenshi_x64.exe.$\r$\n$\r$\nНе знаете где? В Steam: ПКМ по Kenshi > Управление > Просмотреть локальные файлы.$\r$\n$\r$\nКнопка «Установить» активна только для правильной папки."
LangString T_DIR_DEST ${LANG_ENGLISH} "Kenshi folder"
LangString T_DIR_DEST ${LANG_SPANISH} "Carpeta de Kenshi"
LangString T_DIR_DEST ${LANG_RUSSIAN} "Папка Kenshi"
LangString T_FINISH_TITLE ${LANG_ENGLISH} "Kenshi Online installed"
LangString T_FINISH_TITLE ${LANG_SPANISH} "Kenshi Online instalado"
LangString T_FINISH_TITLE ${LANG_RUSSIAN} "Kenshi Online установлен"
LangString T_FINISH_TEXT ${LANG_ENGLISH} "Done. Start a NEW GAME with the 'Multiplayer' start, wait ~90 s and press F1 > JOIN GAME.$\r$\n$\r$\nThe guide explains hosting and joining."
LangString T_FINISH_TEXT ${LANG_SPANISH} "Listo. Empieza un NUEVO JUEGO con el inicio 'Multiplayer', espera ~90 s y pulsa F1 > JOIN GAME.$\r$\n$\r$\nLa guía explica cómo alojar y unirse."
LangString T_FINISH_TEXT ${LANG_RUSSIAN} "Готово. Начните НОВУЮ ИГРУ со стартом 'Multiplayer', подождите ~90 с и нажмите F1 > JOIN GAME.$\r$\n$\r$\nВ руководстве описано, как создать игру и подключиться."
LangString T_FINISH_RUN ${LANG_ENGLISH} "Start Kenshi now"
LangString T_FINISH_RUN ${LANG_SPANISH} "Abrir Kenshi ahora"
LangString T_FINISH_RUN ${LANG_RUSSIAN} "Запустить Kenshi"
LangString T_FINISH_GUIDE ${LANG_ENGLISH} "Open the guide (PDF)"
LangString T_FINISH_GUIDE ${LANG_SPANISH} "Abrir la guía (PDF)"
LangString T_FINISH_GUIDE ${LANG_RUSSIAN} "Открыть руководство (PDF)"
LangString T_GUIDE ${LANG_ENGLISH} "Kenshi_Online_Guide_EN.pdf"
LangString T_GUIDE ${LANG_SPANISH} "Kenshi_Online_Guia_ES.pdf"
LangString T_GUIDE ${LANG_RUSSIAN} "Kenshi_Online_Guide_RU.pdf"
LangString T_KENSHI_OPEN ${LANG_ENGLISH} "Kenshi is running. Close it and click Install again."
LangString T_KENSHI_OPEN ${LANG_SPANISH} "Kenshi está abierto. Ciérralo y pulsa Instalar otra vez."
LangString T_KENSHI_OPEN ${LANG_RUSSIAN} "Kenshi запущен. Закройте его и нажмите «Установить» ещё раз."
LangString T_COPY_FAIL ${LANG_ENGLISH} "Could not copy KenshiMP.Core.dll. Is Kenshi running?"
LangString T_COPY_FAIL ${LANG_SPANISH} "No se pudo copiar KenshiMP.Core.dll. ¿Está Kenshi abierto?"
LangString T_COPY_FAIL ${LANG_RUSSIAN} "Не удалось скопировать KenshiMP.Core.dll. Kenshi запущен?"
LangString T_UN_KENSHI_OPEN ${LANG_ENGLISH} "Kenshi is running. Close it before uninstalling."
LangString T_UN_KENSHI_OPEN ${LANG_SPANISH} "Kenshi está abierto. Ciérralo antes de desinstalar."
LangString T_UN_KENSHI_OPEN ${LANG_RUSSIAN} "Kenshi запущен. Закройте его перед удалением."
LangString T_UN_WORLD ${LANG_ENGLISH} "Also delete the server world (world.kmpsave) and its settings (server.json)?"
LangString T_UN_WORLD ${LANG_SPANISH} "¿Borrar también el mundo guardado del servidor (world.kmpsave) y su configuración (server.json)?"
LangString T_UN_WORLD ${LANG_RUSSIAN} "Удалить также мир сервера (world.kmpsave) и его настройки (server.json)?"

; ---------------------------------------------------------------- helpers
; Quita CR/LF finales de $R9
!macro _TrimNL
  ${Do}
    StrCpy $R8 $R9 1 -1
    ${If} $R8 == "$\r"
    ${OrIf} $R8 == "$\n"
      StrCpy $R9 $R9 -1
    ${Else}
      ${ExitDo}
    ${EndIf}
  ${Loop}
!macroend

; AppendLineIfMissing <archivo> <linea>: añade la línea al final (con salto previo si hace falta)
!macro AppendLineIfMissingDef PFX
Function ${PFX}AppendLineIfMissing
  Exch $R1 ; linea
  Exch
  Exch $R0 ; archivo
  Push $R2
  Push $R3
  Push $R8
  Push $R9
  StrCpy $R2 0
  ClearErrors
  FileOpen $R3 $R0 r
  ${IfNot} ${Errors}
    ${Do}
      ClearErrors
      FileRead $R3 $R9
      ${If} ${Errors}
        ${ExitDo}
      ${EndIf}
      !insertmacro _TrimNL
      ${If} $R9 == $R1
        StrCpy $R2 1
        ${ExitDo}
      ${EndIf}
    ${Loop}
    FileClose $R3
  ${EndIf}
  ${If} $R2 == 0
    ClearErrors
    FileOpen $R3 $R0 a
    ${IfNot} ${Errors}
      FileSeek $R3 0 END $R9
      ${If} $R9 > 0
        FileSeek $R3 -1 END
        FileReadByte $R3 $R8
        FileSeek $R3 0 END
        ${If} $R8 != 10
          FileWrite $R3 "$\r$\n"
        ${EndIf}
      ${EndIf}
      FileWrite $R3 "$R1$\r$\n"
      FileClose $R3
    ${EndIf}
  ${EndIf}
  Pop $R9
  Pop $R8
  Pop $R3
  Pop $R2
  Pop $R0
  Pop $R1
FunctionEnd
!macroend
!insertmacro AppendLineIfMissingDef ""

; RemoveLine <archivo> <linea>: reescribe el archivo sin esa línea
Function un.RemoveLine
  Exch $R1
  Exch
  Exch $R0
  Push $R3
  Push $R4
  Push $R5
  Push $R8
  Push $R9
  IfFileExists $R0 0 rl_done
  GetTempFileName $R5
  FileOpen $R3 $R0 r
  FileOpen $R4 $R5 w
  ${Do}
    ClearErrors
    FileRead $R3 $R9
    ${If} ${Errors}
      ${ExitDo}
    ${EndIf}
    !insertmacro _TrimNL
    ${If} $R9 != $R1
      FileWrite $R4 "$R9$\r$\n"
    ${EndIf}
  ${Loop}
  FileClose $R3
  FileClose $R4
  CopyFiles /SILENT $R5 $R0
  Delete $R5
  rl_done:
  Pop $R9
  Pop $R8
  Pop $R5
  Pop $R4
  Pop $R3
  Pop $R0
  Pop $R1
FunctionEnd

!macro KenshiRunning OUT
  nsExec::ExecToStack 'cmd.exe /c tasklist /FI "IMAGENAME eq kenshi_x64.exe" /NH | find /I "kenshi_x64.exe"'
  Pop ${OUT}
  Pop $R7
!macroend

; ---------------------------------------------------------------- callbacks
Function .onInit
  !insertmacro MUI_LANGDLL_DISPLAY
  ; Intentar localizar Steam desde el registro
  ReadRegStr $0 HKLM "SOFTWARE\WOW6432Node\Valve\Steam" "InstallPath"
  ${If} $0 != ""
    ${If} ${FileExists} "$0\steamapps\common\Kenshi\kenshi_x64.exe"
      StrCpy $INSTDIR "$0\steamapps\common\Kenshi"
    ${EndIf}
  ${EndIf}
  ReadRegStr $0 HKCU "Software\Valve\Steam" "SteamPath"
  ${If} $0 != ""
  ${AndIfNot} ${FileExists} "$INSTDIR\kenshi_x64.exe"
    ${If} ${FileExists} "$0\steamapps\common\Kenshi\kenshi_x64.exe"
      StrCpy $INSTDIR "$0\steamapps\common\Kenshi"
    ${EndIf}
  ${EndIf}
  ${IfNot} ${FileExists} "$INSTDIR\kenshi_x64.exe"
  ${AndIf} ${FileExists} "C:\GOG Games\Kenshi\kenshi_x64.exe"
    StrCpy $INSTDIR "C:\GOG Games\Kenshi"
  ${EndIf}
FunctionEnd

Function .onVerifyInstDir
  IfFileExists "$INSTDIR\kenshi_x64.exe" +2
    Abort
FunctionEnd

Function DirLeave
  !insertmacro KenshiRunning $0
  ${If} $0 == 0
    MessageBox MB_ICONEXCLAMATION|MB_OK "$(T_KENSHI_OPEN)"
    Abort
  ${EndIf}
FunctionEnd

Function LaunchKenshi
  ExecShell "open" "steam://rungameid/233860"
FunctionEnd

; ---------------------------------------------------------------- instalación
Section "Kenshi Online" SecMain
  SectionIn RO
  SetOverwrite on

  DetailPrint "Backing up original files..."
  CreateDirectory "$INSTDIR\KenshiMP_backup"
  ${IfNot} ${FileExists} "$INSTDIR\KenshiMP_backup\Plugins_x64.cfg.bak"
    CopyFiles /SILENT "$INSTDIR\Plugins_x64.cfg" "$INSTDIR\KenshiMP_backup\Plugins_x64.cfg.bak"
  ${EndIf}
  ${IfNot} ${FileExists} "$INSTDIR\KenshiMP_backup\Kenshi_MainMenu.layout.bak"
    CopyFiles /SILENT "$INSTDIR\data\gui\layout\Kenshi_MainMenu.layout" "$INSTDIR\KenshiMP_backup\Kenshi_MainMenu.layout.bak"
  ${EndIf}
  ${IfNot} ${FileExists} "$INSTDIR\KenshiMP_backup\mods.cfg.bak"
    CopyFiles /SILENT "$INSTDIR\data\mods.cfg" "$INSTDIR\KenshiMP_backup\mods.cfg.bak"
  ${EndIf}

  DetailPrint "Copying plugin and server..."
  SetOutPath "$INSTDIR"
  ClearErrors
  File "files\KenshiMP.Core.dll"
  ${If} ${Errors}
    MessageBox MB_ICONSTOP "$(T_COPY_FAIL)"
    Abort
  ${EndIf}
  File "files\KenshiMP.Server.exe"
  SetOverwrite off
  File "files\server.json"
  SetOverwrite on

  DetailPrint "Installing multiplayer menus..."
  SetOutPath "$INSTDIR\data\gui\layout"
  File "files\Kenshi_MainMenu.layout"
  File "files\Kenshi_MultiplayerPanel.layout"
  File "files\Kenshi_MultiplayerHUD.layout"

  DetailPrint "Installing kenshi-online.mod..."
  ; Solo en mods\ (si también está en data\ se carga dos veces y da errores de "already defined")
  Delete "$INSTDIR\data\kenshi-online.mod"
  SetOutPath "$INSTDIR\mods\kenshi-online"
  File "files\kenshi-online.mod"

  DetailPrint "Configuring Plugins_x64.cfg and the mod list..."
  Push "$INSTDIR\Plugins_x64.cfg"
  Push "Plugin=KenshiMP.Core"
  Call AppendLineIfMissing
  Push "$INSTDIR\data\mods.cfg"
  Push "kenshi-online.mod"
  Call AppendLineIfMissing
  Push "$INSTDIR\data\__mods.list"
  Push "kenshi-online"
  Call AppendLineIfMissing

  DetailPrint "Guides..."
  SetOutPath "$INSTDIR\KenshiOnline"
  ; remove files from older versions
  Delete "$INSTDIR\KenshiOnline\KenshiMP.Injector.exe"
  Delete "$INSTDIR\KenshiOnline\JOINING.md"
  Delete "$INSTDIR\KenshiOnline\Guia_rapida.pdf"
  Delete "$INSTDIR\KenshiOnline\Desinstalar Kenshi Online.exe"
  File "files\KenshiMP.Core.dll"
  File "files\kenshi-online.mod"
  File "files\server.json"
  File "files\Kenshi_Online_Guide_EN.pdf"
  File "files\Kenshi_Online_Guia_ES.pdf"
  File "files\Kenshi_Online_Guide_RU.pdf"

  WriteUninstaller "$INSTDIR\KenshiOnline\${UNINSTEXE}"
  WriteRegStr HKLM "${UNINSTKEY}" "DisplayName" "${APPNAME} (Kenshi mod)"
  WriteRegStr HKLM "${UNINSTKEY}" "DisplayVersion" "${APPVER}"
  WriteRegStr HKLM "${UNINSTKEY}" "Publisher" "Kenshi Online contributors"
  WriteRegStr HKLM "${UNINSTKEY}" "InstallLocation" "$INSTDIR"
  WriteRegStr HKLM "${UNINSTKEY}" "UninstallString" '"$INSTDIR\KenshiOnline\${UNINSTEXE}"'
  WriteRegDWORD HKLM "${UNINSTKEY}" "NoModify" 1
  WriteRegDWORD HKLM "${UNINSTKEY}" "NoRepair" 1
  SetOutPath "$INSTDIR"
  DetailPrint "Done."
SectionEnd

; ---------------------------------------------------------------- desinstalación
Function un.onInit
  !insertmacro MUI_UNGETLANGUAGE
  ; El desinstalador vive en <Kenshi>\KenshiOnline
  ${GetParent} "$INSTDIR" $0
  ${If} ${FileExists} "$0\kenshi_x64.exe"
    StrCpy $INSTDIR $0
  ${EndIf}
  !insertmacro KenshiRunning $1
  ${If} $1 == 0
    MessageBox MB_ICONEXCLAMATION|MB_OK "$(T_UN_KENSHI_OPEN)"
    Abort
  ${EndIf}
FunctionEnd

Section "Uninstall"
  Delete "$INSTDIR\KenshiMP.Core.dll"
  Delete "$INSTDIR\KenshiMP.Server.exe"
  Delete "$INSTDIR\data\kenshi-online.mod"
  RMDir /r "$INSTDIR\mods\kenshi-online"
  Delete "$INSTDIR\data\gui\layout\Kenshi_MultiplayerPanel.layout"
  Delete "$INSTDIR\data\gui\layout\Kenshi_MultiplayerHUD.layout"

  ${If} ${FileExists} "$INSTDIR\KenshiMP_backup\Kenshi_MainMenu.layout.bak"
    CopyFiles /SILENT "$INSTDIR\KenshiMP_backup\Kenshi_MainMenu.layout.bak" "$INSTDIR\data\gui\layout\Kenshi_MainMenu.layout"
  ${EndIf}

  Push "$INSTDIR\Plugins_x64.cfg"
  Push "Plugin=KenshiMP.Core"
  Call un.RemoveLine
  Push "$INSTDIR\data\mods.cfg"
  Push "kenshi-online.mod"
  Call un.RemoveLine
  Delete "$INSTDIR\data\__mods.list"

  MessageBox MB_YESNO|MB_ICONQUESTION "$(T_UN_WORLD)" IDNO keep_world
    Delete "$INSTDIR\world.kmpsave"
    Delete "$INSTDIR\server.json"
    RMDir /r "$APPDATA\KenshiMP"
  keep_world:

  RMDir /r "$INSTDIR\KenshiMP_backup"
  DeleteRegKey HKLM "${UNINSTKEY}"
  Delete "$INSTDIR\KenshiOnline\*.*"
  RMDir "$INSTDIR\KenshiOnline"
SectionEnd
