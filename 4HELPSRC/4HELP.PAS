(*** MASTER HELP EXECUTABLE FILE ******************************************
   _______________________________________________________________________
  |                                                                       |
  |                        CONFIDENTIAL MATERIALS                         |
  |                                                                       |
  | These materials include confidential and valuable trade secrets owned |
  | by JP Software Inc. or its suppliers, and are provided to you under   |
  | the terms of a non-disclosure agreement.  These materials may not be  |
  | transmitted or divulged to others or received, viewed, or used by     |
  | others in any way.  All source code, source code and technical        |
  | documentation, and related notes, are unpublished materials, except   |
  | to the extent that they are in part legally available as published    |
  | works from other suppliers, and use of a copyright notice below does  |
  | not imply publication of these materials.                             |
  |                                                                       |
  | This notice must remain as part of these materials and must not be    |
  | removed.                                                              |
  |                                                                       |
  | Unpublished work, Copyright 1988 - 1999, JP Software Inc., All        |
  | Rights Reserved.                                                      |
  |_______________________________________________________________________|


  Modifications September, October, November 1991: 

  Default input filename changed to 4DOS.HLP
  Reenable screen save on exit, return 1 to 4Dos
  Override screen save with /NX
  Changed version numbers to 4.00
  Added /M switch for Monochrome (/BW remains as well)
  Removed BREAKCNT from 4Make
  Added DoTTY procedure for TTY Output
  Added Cntl-P key for Topic Print option
  Added duplicate key assignments for Tab, Left movements
  Added test to take only the first non-switch commandline argument
  Added DV detection, and /NM switch to disable it
  Added Button Bar Attribute to attribute table
  Added /S0, /S2, /S4 mouse speed switches for mouse control
  Added /X switch to disable mouse initialization
  Added call to modified TpMouse unit procedure ManualMouseInit

  Scott McGrath, 11-01-91


  Added global break disable in DoHelp (was just in TPPICK)

  Tom Rawson, 11-02-91

  Modifications after those shown above are not documented individually.
*)

{ 4HELP Version 7.50

  Copyright 1990 - 2002, JP Software Inc., P.O. Box 328,
  Chestertown, MD  21620. All Rights Reserved.

  Originally written by Ross Neilson Wentworth, all rights sold to
  JP Software Inc. }


{$A-,B-,F-,N-,O-,R-,S-,V-}

{$M 12288,32768,655360}

Program FourDosHelp;

Uses
  TpCrt,       { high-speed video                              }
  TpWindow,    { windows                                       }
  Dos,         { DOS access                                    }
  TpDos,       { DOS access                                    }
  TpPick,      { pick lists                                    }
  TpString,    { string manipulation                           }
  TpMouse,     { integrated mouse support                      }
  TpHelp;      { the online help interface                     }

Const
{$IFDEF GERMAN}
  Version     = ' 4DOS 7.50 (D) HILFESYSTEM ';
{$ELSE}
  Version     = ' 4DOS 7.50 Help ';
{$ENDIF}
  Columns      = 5;    { number of columns for the topic index }
  Copyright   : String[80] = 'Copyright 1990 - 2002, JP Software Inc., P.O. Box 328, Chestertown, MD 21620, USA';
  SearchType  : SrchType = StringPickSrch;
  SaveSearch  : boolean = True;

{$IFDEF GERMAN}
  Error2 = 'Hilfedatei nicht vorhanden oder Lesefehler';
  Error100 = 'Unerwartetes Ende der Hilfedatei';
  Error106 = 'Hilfedatei hat ungÅltiges Format';
  Error203 = 'Nicht genÅgend Speicher fÅr das Hilfesystem';
  Error211 = 'Stichwort kann nicht geladen werden';
  Error212 = 'Stichwortliste kann nicht geladen werden';
  ErrorUnknown = 'Unbekannter Fehler im Hilfesytem, #';
{$ELSE}
  Error2 = 'Help file not found or not accessible';
  Error100 = 'Unexpected end of help file';
  Error106 = 'Incorrect version of help file';
  Error203 = 'Insufficient memory for help system';
  Error211 = 'Cannot load name index';
  Error212 = 'Cannot load topic list';
  ErrorUnknown = 'Unknown help system error #';
{$ENDIF}

{$IFDEF GERMAN}
  PrintKey = $04;              {Ctrl-D, print topic (D for Druck)}
  BackKey = $1A;               {Ctrl-Z for ZurÅck}
{$ELSE}
  PrintKey = $10;              {Ctrl-P, print topic}
  BackKey = $02;               {Ctrl-B for Back}
{$ENDIF}

{------------------- Patch area used by HELPCFG ------------------------}
  StartMarker : String[18] = '4DH701 PATCH HERE:';
  HelpColor : HelpColorArray =
{            Item           Background      Foreground
            -------------   -------------   ------------}
   ($17,  { frame           blue            white       }
    $17,  { normal text     blue            white       }
    $1F,  { header          blue            bright white}
    $70,  { selected xref   white           black       }
    $1B,  { unselected xref blue            bright cyan }
    $1F,  { headings (^A)   blue            bright white}
    $1E,  { options (^B)    blue            yellow      }
    $1C,  { warnings (^C)   blue            bright red  }
    $71,  { prompt window   white           blue        }
    $1F,  { prompt input    blue            bright white}
    $70); { Button Bar      white           black       }

  HelpMono  : HelpColorArray =
{            Item           Background      Foreground
            -------------   -------------   ------------}
   ($07,  { frame           black           white       }
    $07,  { normal text     black           white       }
    $0F,  { header          black           bright white}
    $70,  { selected xref   white           black       }
    $0F,  { unselected xref black           bright white}
    $0F,  { headings (^A)   black           bright white}
    $0F,  { options (^B)    black           bright white}
    $70,  { warnings (^C)   white           black       }
    $70,  { prompt window   white           black       }
    $07,  { prompt input    black           white       }
    $70); { Button Bar      white           black       }

  CheckForSnow : boolean = true;
{------------------------ End of patch area -----------------------------}

Type
  CLArr = Array[1..80] of Word;
  ClPtr = ^CLArr;

Var
  ForceBW  : boolean;                     {forces B&W on color monitor}
  ScrollLockMode : boolean;               {Up / down keys in scroll lock mode}
  EscIndex : boolean;                     {Esc returns to index}
  StartWithIndex : boolean;               {display index first}
  FullScreenIndex : boolean;              {force full-screen index}
  HelpTopic : string;                     {topic from command line}
  Path     : String;
  DOSPath  : String;
  HelpAttr : HelpColorArray;
  Help     : HelpPtr;
  OldAttr  : byte;                        {previous screen color}
  OldExit  : Pointer;
  Error    : Integer;
  CL       : ClPtr;
  PickCols : integer;
  Rows     : integer;
  PickRows : integer;
  I : integer;
  Param : string;
  SaveScreen: Boolean;                     {True if allow saving}
  NoMultiTasking : Boolean;                {detect DV if false}
  MouseSpeed : Byte;                       {don't adjust M2P ratio}
  MouseOff : Boolean;                      {don't load mouse driver}

{--------------------------------------------------------------------------}

{$F+}
Procedure MyExit;
Begin
  ExitProc := OldExit;
  CloseHelp(Help);
End;
{$F-}

{--------------------------------------------------------------------------}

Procedure Beep;
Begin
  Sound(500);
  Delay(50);
  NoSound;
  Delay(25);
End;


{--------------------------------------------------------------------------}
procedure HelpErrorMessage(ErrMsg : string);
label
  ExitPoint;
var
  PopWindow : WindowPtr;
  Dummy : word;
begin
  PopWindow := nil;
  Shadow := true;
  if ForceBW or (CurrentMode in [2,7]) then
    HelpAttr := HelpMono
  else
    HelpAttr := HelpColor;
  HiddenCursor;
  if not MakeWindow(PopWindow, 5, 10, 70, 16, True, True, True,
      HelpAttr[PWAttr], HelpAttr[FrAttr], HelpAttr[PWAttr], '')
      then begin
    Shadow := false;
    Beep; Beep;
    goto ExitPoint;
  end;
  Shadow := false;
  if not DisplayWindow(PopWindow) then begin
    Beep; Beep;
    goto ExitPoint;
  end;
{$IFDEF GERMAN}
{  FastWriteWindow('** FEHLER ** ', 2, 2, HelpAttr[PWAttr]);}
  FastWriteWindow('Fehler in der 4DOS-Hilfe:  ' + ErrMsg, 2, 2, HelpAttr[PWAttr]);
  FastWriteWindow('Abbruch mit beliebiger Taste ...', 4, 2, HelpAttr[PWAttr]);
{$ELSE}
  FastWriteWindow('4DOS help error:  ' + ErrMsg, 2, 2, HelpAttr[PWAttr]);
  FastWriteWindow('Press any key to exit ...', 4, 2, HelpAttr[PWAttr]);
{$ENDIF}
  Dummy := ReadKeyWord;
ExitPoint:
  if PopWindow <> nil then
    DisposeWindow(EraseTopWindow);
  NormalCursor;
end;

{--------------------------------------------------------------------------}

{$F+}   { this routine MUST be FAR }
Function TruncMatch(TopicName,ScreenName : String) : Boolean;
  {-trys to match truncated words }
Begin
  TruncMatch := (Pos(StUpCase(ScreenName),StUpCase(TopicName)) = 1);
End;
{$F-}  { end of FAR requirement }

{--------------------------------------------------------------------------}

procedure SetHelpKey(Key : HKType; ScanCode : word);
begin
  if not AddHelpCommand(Key, 1, ScanCode, 0) then
{$IFDEF GERMAN}
    HelpErrorMessage('Falsche Hilfetasten Zuweisung');
{$ELSE}
    HelpErrorMessage('Error in help key assignments');
{$ENDIF}
end;

{--------------------------------------------------------------------------}
(*  This routine doesn't work.  Draws before the Window is drawn.
procedure ShowKeys(Str1, Str2:string);
var
  DRow : integer;
  DString : string;
begin
  DRow := Help^.Height-1; {Rows + 1;}
  DString := Str1 + Str2;
  {FastWrite(CharStr(' ', ScreenWidth), DRow, 1, HelpAttr[HeAttr]);}
  FastWrite(Str1 + Str2, DRow, 2 {(ScreenWidth - Length(DString)) div 2},
            HelpAttr[XsAttr]);
end;
*)
{--------------------------------------------------------------------------}

Procedure DoHelp;

Var
  SaveBreak : Boolean;                    {Saved state of CheckBreak}
  Topic : Word;
  UserTopic : boolean;
  Done : boolean;
  PickWindow : WindowPtr;
  Dummy : boolean;
  OldWindow : boolean;
  PickChoice : Word;
  ShowIndex : boolean;

Begin

    {Disable Ctrl-Break}
  SaveBreak := CheckBreak;
  CheckBreak := False;

  if not AddPickCommand(PKSUser0, 1, $2200, 0) then
{$IFDEF GERMAN}
    HelpErrorMessage('Falsche Hilfetasten Zuweisung');
{$ELSE}
    HelpErrorMessage('Error in help key assignments');
{$ENDIF}

    { Define function keys for button bar }
  SetHelpKey(HKSPrevious, $4100);         {F7, previous}
  SetHelpKey(HKSNext, $4200);             {F8, next}

  if SaveScreen then
    SetHelpKey(HKSExitSave, $2D00);       {Alt-X, exit / save}

  {Make Esc back up to previous topic if not in index mode}
  if not EscIndex then
    SetHelpKey(HKSBack, $1B);             {Esc, back up}

  SetHelpKey(HKSQuickExit, $6B00);        {Alt-F4, quick exit}
  SetHelpKey(HKSQuickExit, $0003);        {Ctrl-C, quick exit}

  {Put Up / Down keys in xref mode if not in index mode}
  if not ScrollLockMode then begin
    SetHelpKey(HKSXUp, $4800);            {Up, scroll toward top via XRef}
    SetHelpKey(HKSXDown, $5000);          {Down, scroll toward bottom via XRef}
  end;

  {Add some duplicate assignments}
  SetHelpKey(HKSNext, $7400);             {Cntl-Right, next}
  SetHelpKey(HKSPrevious, $7300);         {Cntl-Left, previous}
  SetHelpKey(HKSLeft, $0F00);             {Shift-Tab}
  SetHelpKey(HKSRight, $0F09);            {Tab}
  SetHelpKey(HKSPgDn, $20);               {Space does a PgDn}
  SetHelpKey(HKSPrint, PrintKey);         {print topic}
  SetHelpKey(HKSPrint, $4300);            {F9 also prints topic}
  SetHelpKey(HKSTSearch, $06);            {Ctrl-F, topic search}
  SetHelpKey(HKSGSearch, $07);            {Ctrl-G, global search}
  SetHelpKey(HKSNSearch, $0E);            {Ctrl-N, next search}
  SetHelpKey(HKSBack, BackKey);           {back up}

    { Fix up mouse equivalents of above keys }
  if MouseInstalled and not MouseOff then begin
    SetHelpKey(HksNext,$EA00);            {Center-Right, next}
    SetHelpKey(HksPrevious,$EB00);        {Center-Left, previous}
    if SaveScreen then
      SetHelpKey(HksExitSave,$E900);      {All three, exit / save}
    if not EscIndex then
      SetHelpKey(HKSBack, $EE00);         {Right button, back up}
  end;

    { try getting a topic from the prompt line or command line }
  Topic := 0;                             {assume no topic}
  UserTopic := false;
  if HelpTopic <> '' then begin
    Topic := FindHelp(Help, HelpTopic, NIL); {try an exact match}
    if Topic = 0 then                 {no luck, try a partial match }
      Topic := FindHelp(Help, HelpTopic, @TruncMatch);
  end;
  if (Topic = 0) then begin
    if not StartWithIndex then begin      {pick up requested first topic}
      Topic := Help^.Hdr.FirstTopic
    end;
  end
  else
    UserTopic := true;

    {initialize loop control}
  HelpCmdNum := 0;                        {default in case it's never set}
  PickCmdNum := 0;                        {default in case it's never set}
  ShowIndex := (Topic = 0);               {show index if no topic specified}

  Done := false;                          {assume we're not done}
  OldWindow := false;                     {pick window isn't there yet}

  repeat
    with Help^, Hdr do begin
      if ShowIndex then begin             {show the index}
  
        Topic := PickHelp(Help, PickWindow, 1, 1, PickRows, PickCols,
                          false, false, PickChoice);
        if PickCmdNum <> PKSExit then
          OldWindow := true               {topic chosen, just remember
                                           window is there}
        else begin                        {Esc, we're done}
          Topic := 0;
          Done := true;
          DisposeWindow(EraseTopWindow);  {remove pick window}
        end;
      end;
  
      if Topic <> 0 then begin
        if not ShowHelp(Help, Topic, (PickCmdNum = PKSUser0)) then begin
          Beep;                           {can't find selected topic}
          Beep;
          ShowIndex := true;              {force index}
        end
        else if HelpCmdNum = HKSExit then begin
          if UserTopic then
            Done := true
          else if (FirstTopic = 0) or StartWithIndex then
            ShowIndex := true           {return to index if no topic
                                         specified, or we started there}
          else if Topic <> FirstTopic then
            Topic := FirstTopic
  				else
	  			  Done := true;               {exit from first topic}
        end
				else
				  Done := true;               {Alt-X, exit}
      end;
    end;
  until Done;

  CheckBreak := SaveBreak;

    { If exiting with save, put the cursor back at the bottom of the screen
      and clear the line there }
  if (HelpCmdNum = HksExitSave) and (SaveScreen = True) then begin
    Window(1,1,ScreenWidth,ScreenHeight);
    NormalCursor;
    GotoXY(1,ScreenHeight);
    TextBackground(OldAttr shr 4);
    TextColor(OldAttr and $F);
    ClrEol;
    GotoXY(1,ScreenHeight - 1);
    Halt(1);
  end;
end;

{--------------------------------------------------------------------------}
procedure DoTTY;

Var
  Topic : Word;
  Success : Boolean;

begin

  NormVideo;                          {write strings in starting video attr}

  Assign(Input, '');                  {allow commandline redirection}
  Reset(Input);
  Assign(Output, '');
  Rewrite(Output);

  Topic := 0;                             {assume no topic}
  if HelpTopic <> '' then begin
    Topic := FindHelp(Help, HelpTopic, NIL); {try an exact match}
    if Topic = 0 then                 {no luck, try a partial match }
      Topic := FindHelp(Help, HelpTopic, @TruncMatch);
  end;

  Success := TpHelp.ShowHelpTTY(Help, Topic);
  {do we need to report any failures here?}
(*
  if not Success then
{$IFDEF GERMAN}
    HelpErrorMessage('Unbekannter Fehler:  TTY Ausgabe mi·lungen.');}
{$ELSE}
    HelpErrorMessage('Unknown Error:  Failed to complete TTY Output.');
{$ENDIF}
*)
end;

{--------------------------------------------------------------------------}

procedure CheckSwitches(SwString : string);
begin
  if (Pos('/?', SwString) <> 0) then     {/? Select TTY help for Topic}
    IsTTY := TRUE;
  if (Pos('/E', SwString) <> 0) then     {/E to make Esc go back to index}
    EscIndex := TRUE;
{FIXME With /E  program exits when started with TOC, should return to TOC}
  if (Pos('/F', SwString) <> 0) then     {/F for full-screen index}
    FullScreenIndex := TRUE;
  if (Pos('/I', SwString) <> 0) then     {/I to start with index}
    StartWithIndex := TRUE;
  if (Pos('/L', SwString) <> 0) then     {/L to use "scroll-lock" up/down mode}
    ScrollLockMode := TRUE;
  if (Pos('/M', SwString) <> 0) then     {/M to force B&W on color}
    ForceBW := TRUE;
  if (Pos('/R', SwString) <> 0) then     {disable search saves}
    SaveSearch := False;
  if (Pos('/S0', SwString) <> 0) then    {/S0 Slow down the M2P ratio}
    MouseSpeed := 0;
  if (Pos('/S2', SwString) <> 0) then    {/S2 Double M2P ratio}
    MouseSpeed := 2;
  if (Pos('/S4', SwString) <> 0) then    {/S4 Quad M2P ratio}
    MouseSpeed := 4;
  if (Pos('/XM', SwString) <> 0) then    {/XM Do Not Detect Multitasking}
    NoMultiTasking := True
  else if (Pos('/X', SwString) <> 0) then  {/X Don't initialize the mouse}
    MouseOff := True;
  if (Pos('/BW', SwString) <> 0) then    {/BW to force B&W on color}
    ForceBW := TRUE;
  if (Pos('/NX', SwString) <> 0) then    {/NX Suppress SaveOnScreen}
    SaveScreen := False;
end;

{--------------------------------------------------------------------------}

var
  GotTopic : Boolean;                     {take only the first param}
  PopWindow : WindowPtr;                  {for error window}

Begin                                     {main program}

  DirectVideo := FALSE;                   {makes cursor behave on exit,
                                           doesn't slow down FastWrite
                                           video which is what we use}
  ForceBW := FALSE;
  ScrollLockMode := FALSE;
  EscIndex := FALSE;                      {override with /E}
  StartWithIndex := FALSE;                {override with /I}
  FullScreenIndex := FALSE;               {override with /F}
  SaveScreen := TRUE;                     {override with /NX}
  HelpTopic := '';
  NoMultiTasking := False;                {detect DV if false}
  MouseSpeed := 1;                        {don't adjust mouse speed}
  MouseOff := False;                      {load mouse driver}

  { Process user's command line and startup options }
  GotTopic := False;
  for I := 1 to ParamCount do begin
    Param := StUpCase(ParamStr(I));        {get word from command line}
    if Param[1] = '/' then
      CheckSwitches(Param)
    else
      if not GotTopic then begin
        HelpTopic := Param;
        GotTopic := True;
      end;
  end;

  CheckSwitches(StUpCase(GetEnv('4HELPC')));  {Also check for switches in
                                                control variable}

  if ForceBW then                         {force BW mode if requested}
    TpCrt.CurrentMode := 2;

    { Initialize local data }
  OldExit := ExitProc;
  ExitProc := @MyExit;
  CheckSnow := (CurrentDisplay = CGA) AND CheckForSnow;
  If CurrentMode In[2,7]
    Then HelpAttr := HelpMono      { if B&W or MONO }
  Else HelpAttr := HelpColor;      { color          }
  OldAttr := ReadAttrAtCursor;
  MapColors := False;              { don't change the given colors }

    { Initialize other units }
  TpHelp.HelpTitle := Version;
  TpPick.PickSrch := SearchType;   { set the search type }
  TpPick.PickSaveSrch := SaveSearch;  { set save search string on mismatch }
  TpHelp.UseHelpFrame := True;     { we want it framed   }
  TpHelp.HelpMore := True;         { show MORE arrow     }
  Rows := TpCRT.ScreenHeight;

  if MouseOff then                 {perform selective mouse init}
    MouseInstalled := False
  else
    TpMouse.ManualMouseInit;

  if MouseInstalled then begin
    TpHelp.EnableHelpMouse;
    TpHelp.HelpMouseScroll := True;
    if MouseSpeed <> 1 then         {adjust mouse speed if requested}
      case MouseSpeed of
        0 :
          SetMickeyToPixelRatio(16,32);
        2 :
          SetMickeyToPixelRatio(4,8);
        4 :
          SetMickeyToPixelRatio(2,4);
        end;
  end;
TpHelp.HelpMouseScroll := True;

  {Allows Detection of DV background for CRT Items}
  if not NoMultiTasking then begin
    DetectMultiTasking := True;
    ReInitCrt;
  end;

    { Search for 4DOS.HLP in the following order:
        1.) If 4HELPF is there, assume it is a full file name and use
            that file;
        2.) If 4HELP is there, assume it is a path name and use that path
            to look for 4DOS.HLP;
        3.) Otherwise look for 4DOS.HLP in the current directory and on
            the PATH;
    }
{FIXME - Search our own directory first!}
  Error := 2;                             {assume we can't find it}
  Path := GetEnv('4HELP');
  if Path <> '' then
    Error := OpenHelpFile(JustPathName(Path) + '\4DOS.HLP', 1, 1, Rows,
        Columns, FullScreenIndex, HelpAttr, Help);
  if Error <> 0 then
    Error := OpenHelpFile('4DOS.HLP', 1, 1, Rows, Columns, FullScreenIndex,
        HelpAttr, Help);

  if Error = 0 then begin
    if IsTTY then
      DoTTY
    else
      DoHelp;                         {opened, so show help}
  end else begin
    case Error of
        2 : HelpErrorMessage(Error2);
      100 : HelpErrorMessage(Error100);
      106 : HelpErrorMessage(Error106);
      203 : HelpErrorMessage(Error203);
      211 : HelpErrorMessage(Error211);
      212 : HelpErrorMessage(Error212);
      else  HelpErrorMessage(ErrorUnknown + Long2Str(Error));
    end;
  end;
end.

