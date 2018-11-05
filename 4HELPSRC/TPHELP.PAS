(* _______________________________________________________________________
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
  | Unpublished work, Copyright 1988 - 1999, J.P. Software Inc.  All      |
  | Rights Reserved.  Portions Copyright 1987, TurboPower Software.       |
  |_______________________________________________________________________|


  Modifications September and October, 1991 by Scott McGrath:

  Base TPHELP file updated to TPro v.5.11, usable with TP 6
  Paged Help replaced with scrolling help system.
  Added Processing for TTY Output: TTYToggle, TTYOnlyMark
  Added new routines for TTY Output processing: ShowHelpTTY,
      OutputTTY, IsTTYAvailable
  Changed Scrollbar char to ASCII 196
  Added HKSPrint processing, which calls PrintTopic with the current topic
  Added ButtonBar procedure to display key commands
  Added test in HKSPgUp/Dn to stop needlessly redrawing the screen

*)

{$S-,R-,V-,I-,B-}

{ NOTE!  Major sections that have been customized are marked with
  "(* !!!! *)".  Major sections that have been eliminated to save
  space have been marked with (* ### ... ### *). }

{$IFDEF Ver40}
  {$F-}
  {$DEFINE FMinus}
{$ELSE}
  {$F+}
  {$I OPLUS.INC}
  {$I AMINUS.INC}
{$ENDIF}

{Conditional defines that may affect this unit}
{$I TPDEFINE.INC}

{*********************************************************}
{*                TPHELP.PAS 5.02, 5.11                  *}
{*        Copyright (c) TurboPower Software 1987.        *}
{* Portions copyright (c) Sunny Hill Software 1985, 1986 *}
{*     and used under license to TurboPower Software     *}
{*                 All rights reserved.                  *}
{*********************************************************}

unit TpHelp;
  {-General purpose help facility}

interface

uses
  Dos,
  TPDos,
  TPMemChk,
  TPString,
  TpCrt,
  TPWindow,
  {$IFDEF UseMouse}
  TpMouse,
  {$ENDIF}
  TpPick,
  TpCmd,
  TpEdit;

const
  NoHelpAvailable = $FFFFFFFF; {Flag that no help is available for topic}
  HelpEquated = $FFFFFFFE;     {Flag that this topic is equated to another}

{ *** 4.00 *** }
  MaxLinesPerSection = 2000; {Maximum number of lines per topic?}
  MaxXrefsPerSection = 512;  {Maximum number of topic xrefs per section}
  MaxTopics = 1024;          {Maximum number of topics in one help file}
  MaxHelpStack = 15;         {Highest stacked topic}
  MaxXrefSearchLen = 16;     {Max length of xref search string}
  PageDragFirst = 350;       {Delay for first page drag, ms}
  PageDragRep = 200;         {Delay for page drag repeat, ms}
  LineDragFirst = 250;       {Delay for first line drag, ms}
  LineDragRep = 50;          {Delay for line drag repeat, ms}

  Attr1Toggle = ^A;          {Character toggles special attribute 1}
  Attr2Toggle = ^B;          {Character toggles special attribute 2}
  Attr3Toggle = ^C;          {Character toggles special attribute 3}
  IndexMarker = ^D;          {Character marks topic number that follows}
  XrefToggle = ^E;           {Character toggles xref highlight}
  IndentMark = ^F;           {Character sets indent width}
  IgnoreMark = ^G;           {Ignore character for XRef search}
  LineBrkMark = ^M;          {Denotes end of line of help}
  PageBrkMark = ^L;          {Denotes end of page of help}
  SectEndMark = #0;          {Denotes end of help section}
  EscapeChar = ^X;           {Escape character for input text}
  TTYToggle  = ^N;           {Character toggles TTY output sections}
  TTYOnlyMark = #15;         {TTY only text at end of topic}

  {Command values for help system}
  HKSNone = 0;               {Not a command}
  HKSAlpha = 1;              {An alphanumeric character}
  HKSUp = 2;                 {Scroll toward top}
  HKSDown = 3;               {Scroll toward bottom}
  HKSPgUp = 4;               {Display previous help page}
  HKSPgDn = 5;               {Display next help page}
  HKSLeft = 6;               {Cursor left to previous cross-reference}
  HKSRight = 7;              {Cursor right to next cross-reference}
  HKSExit = 8;               {Exit the help system}
  HKSSelect = 9;             {Select the current cross-reference and display topic}
  HKSBack = 10;              {Display the most recent help topic}
  HKSHome = 11;              {Display first help page}
  HKSEnd = 12;               {Display last help page}
  HKSToC = 13;               {Display the first help topic}
  HKSIndex = 14;             {Display the help index}
  HKSKeys = 15;              {Display a special topic for keys help}
  HKSProbe = 16;             {Mouse selection}
  HksExitSave = 17;          {Customized-exit but keep help screen}
  HksQuickExit = 18;         {Quick-exit}
  HksNext = 19;              {Customized-show next help}
  HksPrevious = 20;          {Customized-show previous help}
  HKSPrint = 21;             {Print TTY Output for Current Topic}
  HKSTSearch = 22;           {Search within topic}
  HKSGSearch = 23;           {Global search across topics}
  HKSNSearch = 24;           {Search for next occurrence}
  HKSXUp = 25;               {Scroll toward top via XRefs}
  HKSXDown = 26;             {Scroll toward bottom via XRefs}
  HKSExtHelp = 27;           {Display external help}
  HKSSwapKeys = 28;          {Swap button bar keys}
  HKSUser0 = 29;             {User-defined exit commands}
  HKSUser1 = 30;
  HKSUser2 = 31;
  HKSUser3 = 32;

  {.F-}
  {Keystroke to command mapping}
  HelpKeyMax = 192;
  HelpKeySet : array[0..HelpKeyMax] of Byte =
  (                                               {Byte offset}
  3, $00, $48, HKSUp,       {Up}                  {0}
  3, $00, $50, HKSDown,     {Down}
  3, $00, $49, HKSPgUp,     {PgUp}
  3, $00, $51, HKSPgDn,     {PgDn}
  3, $00, $4B, HKSLeft,     {Left}
  3, $00, $4D, HKSRight,    {Right}
  3, $00, $47, HKSHome,     {Home}
  3, $00, $4F, HKSEnd,      {End}
  3, $00, $3B, HKSIndex,    {F1}
  3, $00, $3C, HKSToC,      {F2}
  3, $00, $3D, HKSKeys,     {F3}
  3, $00, $3E, HKSExtHelp,  {F4}
  3, $00, $3F, HKSTSearch,  {F5}
  3, $00, $40, HKSGSearch,  {F6}
  3, $00, $58, HKSNSearch,  {Shift-F5}
  3, $00, $59, HKSNSearch,  {Shift-F6}
  3, $00, $44, HKSSwapKeys, {F10}
  2, $1B,      HKSExit,     {Esc}
  2, $0D,      HKSSelect,   {Enter}

  2, $05,      HKSUp,       {^E}                  {74}
  2, $17,      HKSUp,       {^W}
  2, $18,      HKSDown,     {^X}
  2, $1A,      HKSDown,     {^Z}
  2, $12,      HKSPgUp,     {^R}
  2, $03,      HKSPgDn,     {^C}
  2, $13,      HKSLeft,     {^S}
  2, $04,      HKSRight,    {^D}
  3, $11, $12, HKSHome,     {^QR}
  3, $11, $03, HKSEnd,      {^QC}

  {$IFDEF UseMouse}
  3, $00, $EF, HKSProbe,    {Click left}          {106}
  3, $00, $EE, HKSExit,     {Click right}
  3, $00, $ED, HKSIndex,    {Click both}
  3, $00, $E8, HKSDown,     {Wheel down}
  3, $00, $E0, HKSUp,       {Wheel up}

  {$ELSE}
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,                   {106}
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  {$ENDIF}
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,                   {126}
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0
  );
  {.F+}

{$IFDEF GERMAN}
  HelpTitle : string[39] = ' Stichwîrter ';
                  
  NKeyLists = 1;
  NMaxKeys = 6;
  StdKeys : array[1..NKeyLists] of string[78] = (
    ' F1˘Index ≥ AltX˘ExitSave ≥ G˘Suche ≥ W˘Weiter ≥ D˘Druck ≥ Z˘ZurÅck ≥ '
  );
  StdKeyCommands : array [1..NKeyLists, 1..NMaxKeys] of byte = (
    (HKSIndex, HKSExitSave, HKSTSearch, HKSNSearch, HKSPrint, HKSBack)
  );
{  UpAndDown   =  'Bd/Bd';}
{$ELSE}
  HelpTitle : string[39] = ' Topics '; {Displayed at top of help pick window}
  NKeyLists = 2;
  NMaxKeys = 6;
  StdKeys : array[1..NKeyLists] of string[78] = (
    '≥ F1˘Index ≥ F2˘TOC ≥ F3˘Keys ≥ F4˘Ext Help ≥ F9˘Print ≥ F10˘Search Keys ≥',
    '≥ F5˘Topic Search ≥ F6˘Global Search ≥ Sh-F5 / Sh-F6˘Next ≥ F10˘Std Keys ≥'
  );
  StdKeyCommands : array [1..NKeyLists, 1..NMaxKeys] of byte = (
    (HKSIndex, HKSToC, HKSKeys, HKSExtHelp, HKSPrint, HKSSwapKeys),
    (HKSTSearch, HKSGSearch, HKSNSearch, HKSSwapKeys, HKSNone, HKSNone)
  );
{$ENDIF}
  UseHelpFrame : Boolean = True; {True to draw frame around help window}
  HelpMore : Boolean = True; {True to display PgUp/PgDn in help frame}
  HideCursor : Boolean = True; {False to leave hardware cursor on screen}
  IsTTY : Boolean = False; {True to use TTY Output methods}
  UseButtonBar : Boolean = True;  {True to display button bar}
  {$IFDEF UseMouse}
  HelpMouseScroll : Boolean = True; {True to support mouse scrolling}
  {$ENDIF}

  IndexXrefTopic : Word = 0;  {Topic number which brings up index when used as xref}

type
  HelpID = string[8];        {Version identifier at start of help file}

  HKtype = HKSNone..HKSUser3; {Valid help commands}
  SO = record
         O : Word;
         S : Word;
       end;

  HelpColorType = (
    FrAttr,                  {Frame}
    TeAttr,                  {Normal text}
    HeAttr,                  {Header and more prompt}
    XsAttr,                  {Selected cross-reference item}
    XrAttr,                  {Unselected cross-reference item}
    SpAtt1,                  {Special attribute #1}
    SpAtt2,                  {Special attribute #2}
    SpAtt3,                  {Special attribute #3}
    PWAttr,                  {Search window attribute}
    PIAttr,                  {Search input attribute}
    BBAttr);                 {Button Bar attribute}

  HelpColorArray = array[HelpColorType] of Byte;
  HelpToggleArray = array[Attr1Toggle..XrefToggle] of Byte;

{4.0 Mouse Code}
  MouseRegionType = (
    MouseLineUp,                      {up arrow at top of scroll bar}
    MouseLineDown,                    {down arrow at bottom}
    MousePgUp,                        {PgUp hot spot}
    MousePgDn,                        {PgDn hot spot}
    MouseScrollBar,                   {in scroll bar}
    MouseClose,                       {close window hot spot}
    MouseButtonBar,                   {button bar hot spots}
    MouseOther);                      {anywhere else}

  DragStateType = (
    DragNone,                         {dragging not active}
    DragScroll,                       {repeating line or page up/down}
    DragSlider,                       {dragging slider}
    DragHold);                        {drag hold due to mouse motion}

  MouseBarRegion = record
     Start  : Byte;                 {Starting Position}
     Last   : Byte;                 {Length of String}
    end;

  BarRegion = array[1..NMaxKeys] of MouseBarRegion;

  XlateArray = array[0..29] of Byte; {Most common characters in help text}

  SharewareDataRec =             {shareware timeout data stored in help file}
    record
      CheckSum : Word;                    {shareware data checksum}
      LastUsageDate : Word;               {last use of 4DOS}
      ReleaseNum : Word;                  {release number on first use}
      DaysUsed : Word;                    {number of days on which 4DOS has been used}
      MachineName : array[1..32] of byte; {machine ID bytes}
    	RestartCount : word;                {number of restarts}
    	ExpireDays : word;                  {days to expiration (encoded)}
	    RestrictDays : word;                {days to restriction (encoded)}
	    Signature : word;                   {shareware signature}
      Pad : array [1..72] of byte;        {pad to 128 bytes}
    end;

  HelpHeader =               {At start of help file}
    record
      ID : HelpID;           {Marks file as help file}
      HighestTopic : Word;   {Highest topic number}
      NumTopics : Word;      {Number of topics}
      BiggestTopic : Word;   {Size of largest topic in uncompressed bytes}
      NamedTopics : Word;    {Number of topics in help index}
      NameSize : Byte;       {Size of largest name, 0 for none}
      PickSize : Byte;       {Size of each entry in pick table, 0 for none}
      Width : Byte;          {Width of help window, with frame if any}
      FirstTopic : Word;     {Topic to show first (0 = index)}
      KeysTopic : Word;      {Topic to show when keys help needed}
      ExtHelpName : String[13];  {Name for external help file}
      ExtHelpEnv : String[16];   {Environment variable for alternate
                                  external help file name}
      XlateTable : XlateArray; {Table for decompression}
      SharewareData : SharewareDataRec;  {shareware info for 4DOS.COM}
    end;
  HelpHeaderPtr = ^HelpHeader;

  CharArray = array[0..65520] of Char; {List of names of help entries}
  CharArrayPtr = ^CharArray;

  HelpIndexRec =
    record
      Start : LongInt;       {File position of topic}
      CompLen : Word;        {Compressed length of topic}
    end;                     {Index of file positions}
  HelpIndex = array[1..MaxTopics] of HelpIndexRec;
  HelpIndexPtr = ^HelpIndex;

  TopicIndex = array[1..MaxTopics] of Word;
  TopicIndexPtr = ^TopicIndex;

  Xrefs = 0..MaxXrefsPerSection;
  Lines = 0..MaxLinesPerSection;

  HelpStackRec =
    record
      STopic : Word;         {Which topic}
      SLine : Word;          {Starting line count}
      SXnum : Xrefs;         {Which xref item selected}
    end;
  HelpStackIndex = 0..MaxHelpStack;
  HelpStackArray = array[HelpStackIndex] of HelpStackRec;

  HelpPtr = ^HelpDesc;       {The user hook to the help system}
  HelpDesc =                 {Holds parameters of help system}
    record
      BufP : CharArrayPtr;   {Points to a buffer that will hold largest section}
      Hdr : HelpHeader;      {Copy of header for fast reference}
      RowH : Byte;           {Upper left corner of help window - Row}
      ColH : Byte;           {Upper left corner of help window - Col}
      Height : Byte;         {Height of help window, with frame}
      A : HelpColorArray;    {Attributes used to draw help}
      Frame : FrameArray;    {Frame characters to use}
      ShowFrame : Boolean;   {True to display frame}
      ShowMore : Boolean;    {True to display More prompt in frame}
      MouseScroll : Boolean; {True to display mouse scroll bar}
      Stack : HelpStackArray; {Topics previously accessed}
      St : HelpStackIndex;   {Top of help stack}
      Sb : HelpStackIndex;   {Bottom of help stack}
      PickWindow : WindowPtr;  {window used for index}
      PickCols : integer;    {columns in index}
      PickRows : integer;    {rows in index}
      FillPickScreen : boolean;  {index fills screen}
      PickChoice : Word;     {previous index choice}
      case InRAM : Boolean of  {True if help file is bound into code}
        False :
          (Fil : file);      {Untyped file variable for help}
        True :
          (HdrP : HelpHeaderPtr; {Points to base of structure in RAM}
           NamP : CharArrayPtr;  {Points to pick name array in RAM}
           IndP : HelpIndexPtr); {Points to help section index in RAM}
    end;

const

  HelpSystemID : HelpID = '4DH701AA';  {Version identifier}

var
  PBuff : CharArrayPtr;      {Pointer to pick names buffer}
  PBufSize : Word;           {Pick names buffer size}
  TBuff : TopicIndexPtr;     {Pointer to topic index buffer}
  TBufSize : Word;           {Topic index buffer size}
  TBufPtr : Word;            {Topic list entry number for current topic}
  HelpKeyPtr : Pointer;      {Points to Kbd routine like ReadKeyWord}
  HelpCmdNum : HKtype;       {Last help command entered}
  HelpOnScreen : Boolean;    {True when help system is displayed}
  {$IFDEF UseMouse}
  HelpMouseEnabled : Boolean; {True if mouse is enabled}
  {$ENDIF}
  HelpResize : boolean;      {true if windows should be resized for topic}
  HelpSaveSrch : boolean;    {true if xref search string should be saved
                              on mismatch}
  BarDispCol : Word;         {column for BBar start}
  BarReg : BarRegion;
  Lst : Text;
  LstName : string[40];
  LstOpen : Boolean;



  {-----------------------------------------------------------------}

function OpenHelpFile(HelpFileName : string;
                      XLow, YLow, YHigh, PickCols : Byte;
                      FullScreenIndex : boolean;
                      Colors : HelpColorArray;
                      var Help : HelpPtr) : Word;
  {-Find and open help file, returning 0 or error code, and
    an initialized help descriptor if successful}

procedure CloseHelp(var Help : HelpPtr);
  {-Close help file and/or deallocate buffer space}

function ShowHelp(Help : HelpPtr; Var Topic : Word;
    StartAtMark : boolean) : Boolean;
  {-Display help screen for item Topic, returning true if successful}
  { I changed TOPIC to a VAR so that changes can be passed back }

function FindHelp(Help : HelpPtr; Name : string; MatchFunc : Pointer) : Word;
  {-Return Topic number of help with specified Name, 0 if not found}


  function PickHelp(Help : HelpPtr; var PickW : WindowPtr;
                    XLow, YLow, YHigh, PickCols : Byte;
                    OldWindow, SaveWindow : boolean;
                    var Choice : word) : Word;
  {-Display help pick list, returning Topic number, or 0 for none}

function AddHelpCommand(Cmd : HKtype; NumKeys : Byte; Key1, Key2 : Word) : Boolean;
  {-Add a new command key assignment or change an existing one}

procedure DisableHelpIndex;
  {-Disable the F1 help index inside of a help screen}

procedure EnableHelpIndex;
  {-Enable the F1 help index inside of a help screen}

  {$IFDEF UseMouse}
procedure EnableHelpMouse;
  {-Enable mouse control of help screens}

procedure DisableHelpMouse;
  {-Disable mouse control of help screens}
  {$ENDIF}

procedure Beep;
 {error beep}

procedure PrintTopic(Help : HelpPtr; Topic : word);
 {Print the current topic to the standard printer}

function SearchText(Help : HelpPtr; var Topic : word;
    GlobalSearch : boolean) : boolean;
 {Search the current topic for a string}

function ShowHelpTTY(Help : HelpPtr; Var Topic : Word) : Boolean;
  {display topic help to TTY Output}

  {=================================================================}

implementation

const
  FlexStackSize = 6;         {Max number of nested attributes}
  MouseUpMark = #24;         {Characters in scroll bar}
  MouseDnMark = #25;
  MouseQuitMark = #4;
  ScrollMark = #178;
  SBarChar = #176;           {scroll bar background char, 4.00}
                             {blank string used for indents (length may change)}
  Blanks : string[80] = '                                                                                ';

  FrameDelta : array[Boolean] of Byte = (1, 0);

  HelpIndexDisabled : Boolean = False; {True when F1 inside of help is disabled}

type
  StringPtr = ^string;

{ *** 4.00 *** }
  LineIndex = array[Lines] of Word; {offset into text for each line}
{  LineXRS = array[Lines] of Boolean; {start with xref flag for each line}
  FlexStack = array[0..FlexStackSize] of Byte; {Stacks current attributes}

  LineAttrRec =
    record
      FlexSP  : Byte;       {flex stack pointer}
      FlexSt  : FlexStack;  {active attribute indices}
    end;
  LineAttr = array[Lines] of LineAttrRec;
  LineIndent = array[Lines] of byte;

  XrefRec =
    record
      Line : Lines;          {which line the xref displays on}
      Col : Byte;            {Which col of window}
      Len : Byte;            {Length of highlight}
      Bofs : Word;           {Offset in uncompressed text buffer}
      Topic : Word;          {Which topic is cross-referenced}
    end;
  XrefIndex = array[Xrefs] of XrefRec;
{ *** 4.00 *** }

  HelpStateRec =
    record
      ColMin : Byte;         {Min window-relative col where text appears}
      ColMax : Byte;         {Max window-relative col where text appears}
      Xnum : Xrefs;          {Currently selected cross-reference}
      Xcnt : Xrefs;          {Number of cross-references in topic}
      X : XrefIndex;         {Index of cross-references}
      AC : HelpToggleArray;  {Attributes for each toggle character}
      ShowMoreNow : Boolean; {True to display More prompt in frame}
      MouseScrollNow : Boolean; {True to display mouse scroll bar}
      L               : LineIndex;  {buffer offset to start of each line}
      LineA           : LineAttr;   {starting attributes}
      LineI           : LineIndent; {indents}
{      LineX           : LineXRS;    {starting XRef flags}
      CurrentTopLine  : Lines;      {active top line}
      NewTopLine      : Lines;      {desired Top line}
      LastLine        : Lines;      {Last active line number}
      LineCnt         : Lines;      {Line Cnt of Current Topic}
      Lslid : Byte;          {Last slider position}
      W : WindowPtr;         {Pointer to window in which help appears}
      BBWin : WindowPtr;     {Pointer to Button Bar Window}
      KeyListNum : word;     {number of current button bar key list}
      NewHeight : byte;      {new window height}
      OldHeight : byte;      {previous window height}
      SearchStartPos : word;  {Starting position for compare}
      SearchGlobal : boolean;  {last search type}
      SearchString : string[40];  {String to search for}
      SearchLen : word;      {length of search string}
      FoundLine : Lines;     {line found in search}
      FoundMarkLine : Lines; {found line to be marked}
    end;

var
  HelpState : HelpStateRec;  {Overall help info}
  PopWindow : WindowPtr;     {global popup window pointer}
  SPtr : CharArrayPtr;       {Source pointer}
  Dptr : CharArrayPtr;       {Destination pointer}
  SI : Word;                 {Source index during decompression}
  DI : Word;                 {Destination index during decompression}
  BN : Byte;                 {Buffered nibble}
  Nibble : Boolean;          {True when partial byte entered during decompress}
  NSize : Byte;              {Size of array element in pick buffer}

  {$IFDEF FMinus}
  {$F+}
  {$ENDIF}

  function SendHelpName(Topic : Word) : string;
    {-Pass each help Topic to the pick unit}
  var
    NamePtr : ^string;
  begin
    NamePtr := Ptr(SO(PBuff).S, SO(PBuff).O+NSize*(TBuff^[Topic]-1));
    if NamePtr^[1] = Chr(24) then          {kill escape on topic name}
      SendHelpName := ' ' + Copy(NamePtr^, 2, Length(NamePtr^)-1)
    else
      SendHelpName := ' ' + NamePtr^;
    {SendHelpName := ' '+string(Ptr(SO(PBuff).S, SO(PBuff).O+NSize*(TBuff^[Topic]-1))^);}
  end;

  function Match(S1, S2 : string) : Boolean;
    {-Default match function}
  begin
    Match := (CompUCstring(S1, S2) = Equal);
  end;

  {$IFDEF FMinus}
  {$F-}
  {$ENDIF}

  function GetBuffer(Help : HelpPtr; var P; SeekOfs : LongInt;
                     SizeReq : Word; var SizeAlloc : Word) : Boolean;
    {-Return pointer to loaded array of help data}
  var
    Pt : Pointer absolute P;
    BytesRead : Word;
  begin
    GetBuffer := False;
    SizeAlloc := 0;

    with Help^, Hdr do
      if InRAM then
        {Already in memory, just compute the pointer}
        Pt := Ptr(SO(HdrP).S, SO(HdrP).O+Word(SeekOfs))

      else if FileRec(Fil).Mode = fmInOut then begin
        {On disk, first allocate space}
        if not GetMemCheck(P, SizeReq) then
          Exit;
        SizeAlloc := SizeReq;
        {Read names into buffer}
        Seek(Fil, SeekOfs);
        if IoResult <> 0 then
          Exit;
        BlockRead(Fil, Pt^, SizeReq, BytesRead);
        if (IoResult <> 0) or (BytesRead <> SizeReq) then
          Exit;

      end else
        {Help file not open}
        Exit;

    GetBuffer := True;
  end;

  function IDOK(Help : HelpPtr) : boolean;
  begin
    with Help^, Hdr do begin
      IDOK := (Length(ID) = Length(HelpSystemID)) and
        (CompString(Help^.Hdr.ID, HelpSystemID) = Equal)
    end;
  end;

  procedure DecryptTable(Help : HelpPtr);
    {-Decrypt the translation table}
  var
    I : Word;
  begin
    with Help^, Hdr do begin
      for I := 0 to 29 do
        XlateTable[I] := XlateTable[I] xor $C3;
    end;
  end;

  function OpenHelpFile(HelpFileName : string;
                        XLow, YLow, YHigh, PickCols : Byte;
                        FullScreenIndex : boolean;
                        Colors : HelpColorArray;
                        var Help : HelpPtr) : Word;
    {-Find and open help file, returning 0 or error code, and
      an initialized help descriptor if successful}
  label
    ErrorExit;
  var
    IO : Word;
    BytesRead : Word;
    IsOpen : Boolean;
    MaxCols : Byte;
    Rows : integer;
  begin
    {Initialize the result}
    Help := nil;
    PBuff := nil;
    TBuff := nil;
    TBufPtr := 0;
    IsOpen := False;

    {Find the help file}
    if not ExistOnPath(HelpFileName, HelpFileName) then begin
      OpenHelpFile := 2;
      goto ErrorExit;
    end;

    {Allocate space for help descriptor}
    if not GetMemCheck(Help, SizeOf(HelpDesc)) then begin
      OpenHelpFile := 203;
      goto ErrorExit;
    end;

    {Initialize the help descriptor}
    with Help^ do begin
      {Most help information is on disk}
      InRAM := False;

      {Open the help file}
      FileMode := 0;                      {open in read-only mode}
      Assign(Fil, HelpFileName);
      Reset(Fil, 1);
      IO := IoResult;
      if IO <> 0 then begin
        OpenHelpFile := IO;
        goto ErrorExit;
      end;
      IsOpen := True;

      {Get header from file}
      BlockRead(Fil, Hdr, SizeOf(HelpHeader), BytesRead);
      IO := IoResult;
      if IO <> 0 then begin
        OpenHelpFile := IO;
        goto ErrorExit;
      end;
      if BytesRead <> SizeOf(HelpHeader) then begin
        OpenHelpFile := 100;
        goto ErrorExit;
      end;

      with Hdr do begin
        {Check file ID}
        if not IDOK(Help) then begin
          {"Invalid numeric format" - used as error code for invalid ID}
          OpenHelpFile := 106;
          goto ErrorExit;
        end;

        {Decrypt the translation table}
        DecryptTable(Help);

        {Read pick name index}
        if not GetBuffer(Help, PBuff, SizeOf(HelpHeader),
            HighestTopic*NameSize, PBufSize) then begin
          OpenHelpFile := 211;
          goto ErrorExit;
        end;

        {Read topic list}
        if not GetBuffer(Help, TBuff, SizeOf(HelpHeader) +
            LongInt(HighestTopic)*(NameSize+SizeOf(HelpIndexRec)),
            HighestTopic*SizeOf(Word), TBufSize) then begin
          OpenHelpFile := 212;
          goto ErrorExit;
        end;

        {Get buffer space for reading help sections}
        if not GetMemCheck(BufP, BiggestTopic) then begin
          OpenHelpFile := 203;
          goto ErrorExit;
        end;

        {Initialize index window data}
        Rows := TpCRT.ScreenHeight;
        PickWindow := nil;
        PickCols := (NamedTopics + Rows - 4) div (Rows - 3);
        PickRows := ((NamedTopics + PickCols - 1) div PickCols) + 2;
        FillPickScreen := FullScreenIndex;
        PickChoice := 1;
        PickMouseScroll := false;         {disable mouse scroll in TPPICK}

      end;

      {Initialize remaining fields}
      RowH := YLow;
      ColH := XLow;
      Height := YHigh-YLow+1;
      A := Colors;
      Frame := FrameChars;
      ShowFrame := UseHelpFrame;
      ShowMore := UseHelpFrame and HelpMore;
      {$IFDEF UseMouse}
      MouseScroll := UseHelpFrame and HelpMouseScroll;
      {$ENDIF}
{      St := 0;
      Sb := 0;}

      {Successful initialization}
      OpenHelpFile := 0;
      Exit;
    end;

ErrorExit:
    if IsOpen then begin
      Close(Help^.Fil);
      IO := IoResult;
    end;
    FreeMemCheck(Help, SizeOf(HelpDesc));
    FreeMemCheck(PBuff, PBufSize);
    FreeMemCheck(TBuff, TBufSize);
  end;


  procedure CloseHelp(var Help : HelpPtr);
    {-Close help file and/or deallocate buffer space}
  var
    IO : Word;
  begin
    with Help^, Hdr do begin
      if not IDOK(Help) then
        {Not a valid help pointer}
        Exit;
      if not InRAM then
        if FileRec(Fil).Mode = fmInOut then begin
          {Close help file}
          Close(Fil);
          IO := IoResult;
        end;
      FreeMemCheck(BufP, BiggestTopic);
    end;
    FreeMemCheck(Help, SizeOf(HelpDesc));
    FreeMemCheck(PBuff, PBufSize);
    FreeMemCheck(TBuff, TBufSize);
    Help := nil;
    if LstOpen then begin
      Close(Lst);
      LstOpen := false;
    end;
  end;

  function GetNameString(Help : HelpPtr; Topic : Word) : string;
    {-Return name string for help item, if any}
  var
    S : string;
  begin
    GetNameString := '';
    with Help^, Hdr do
      if NameSize <> 0 then
        if InRAM then
          GetNameString := string(Ptr(SO(NamP).S, SO(NamP).O+NameSize*(Topic-1))^)
        else if FileRec(Fil).Mode = fmInOut then begin
          Seek(Fil, LongInt(SizeOf(HelpHeader))+NameSize*(Topic-1));
          if IoResult <> 0 then
            Exit;
          BlockRead(Fil, S, NameSize);
          if IoResult <> 0 then
            Exit;
          if S[1] = Chr(24) then          {kill escape on topic name}
            GetNameString := Copy(S, 2, Length(S)-1)
          else
            GetNameString := S;
        end;
  end;


  function MapPointer(Help : HelpPtr; Topic : Word) : Word;
    {-Find the topic map entry number for a topic}
  var
    MPtr : Word;
  begin
    MPtr := 1;
    with Help^.Hdr do begin
      while (MPtr < HighestTopic) and (TBuff^[MPtr] <> Topic) do
        Inc(MPtr);
      if MPtr >= HighestTopic then
        MapPointer := 0
      else
        MapPointer := integer(MPtr);
    end;
  end;


  procedure InitTopic(Help : HelpPtr);
    {-Examine topic and find xref points}
  var
    Bpos : Word;
    {Pofs : Word;}
    {Prow : Word;}
    Pcol : Word;
    {Mrow : Word;}
    WordP : ^Word;
    Done : Boolean;
    Ch : Char;
    HA : LineAttrRec;
    LA : LineAttrRec;
    LineOfs : word;
    IndentWidth : byte;
    SaveIndentWidth : byte;
    LineIndent : boolean;
{    InXRef : Boolean;}

    procedure NewLine;
    begin
      with HelpState do begin
        if LineCnt >= MaxLinesPerSection then
          Done := True
        else begin
          Inc(LineCnt);
          L[LineCnt] := LineOfs;
          LineA[LineCnt] := LA;
          LineI[LineCnt] := IndentWidth;
          if LineIndent then begin
            IndentWidth := SaveIndentWidth;  {reset to normal after first line}
            LineIndent := false;
          end;
{          LineX[LineCnt] := InXRef;}
          LineOfs := Bpos+1;
          L[LineCnt+1] := LineOfs;
          LA := HA;
        end;
      end;
    end;
{ *** 4.00 *** }

  begin
    with Help^, Hdr, HelpState do begin
      Xnum := 0;
      ColMin := 2;
      ColMax := Width-3;
      Bpos := 0;
      LineOfs := 0;
      Pcol := ColMin;
      {Mrow := Height-2;}
      Xcnt := 0;
      NewHeight := 0;
      LineCnt := 0;        {count of lines}
      LastLine := 0;
      IndentWidth := 0;
      LineIndent := false;

      {No special attributes initially active}
      FillChar(HA, SizeOf(LineAttrRec), 0);
      HA.FlexSt[0] := A[TeAttr];
      LA := HA;
{      InXRef := False;}

      Done := False;
      repeat
        Ch := BufP^[Bpos];
        case Ch of
          EscapeChar : {suppress normal meaning of next character}
            Inc(Bpos);

          Attr1Toggle..Attr3Toggle : {Modifying video attribute}
            with HA do
              if (FlexSp > 0) and (FlexSt[FlexSp] = AC[Ch]) then
                {Toggling current state off}
                Dec(FlexSp)
              else if FlexSp < FlexStackSize then begin
                {Changing to new attribute}
                Inc(FlexSp);
                FlexSt[FlexSp] := AC[Ch];
              end;

          XrefToggle :       {Marking a cross-reference}
            with HA do
              if (FlexSp > 0) and (FlexSt[FlexSp] = A[XrAttr]) then begin
                {Toggling current state off}
                Dec(FlexSp);
                {Store length of highlight}
                with X[Xcnt] do
                  Len := Bpos-Bofs-1;
{                InXRef := False;}
              end else if FlexSp < FlexStackSize then begin
                {Changing to new attribute}
                Inc(FlexSp);
                FlexSt[FlexSp] := A[XrAttr];
{                InXRef := True;}
              end;

          IndexMarker :      {Indicating cross-reference topic}
            begin
              if Xcnt < MaxXrefsPerSection then begin
                Inc(Xcnt);
                with X[Xcnt] do begin
                  Line := LineCnt+1;
                  Col := Pcol;
                  Bofs := Bpos+3;
                  WordP := @BufP^[Bpos+1];
                  Topic := WordP^;
                end;
              end;
              Inc(Bpos, 2);
            end;


          TTYToggle, IgnoreMark:
            ;

          IndentMark :
            begin
              SaveIndentWidth := IndentWidth;
              IndentWidth := Byte(Bufp^[Bpos+1]);  {get indent width}
              if IndentWidth >= 128 then begin
                LineIndent := true;       {indent changes for one line only}
                IndentWidth := IndentWidth - 128;
              end;
              Pcol := ColMin + IndentWidth;        {force column to left}
              Inc(Bpos);                           {skip it}
            end;

          PageBrkMark,
          LineBrkMark :      {End of line}
            begin
              NewLine;
              Pcol := ColMin + IndentWidth;
            end;

          TTYOnlyMark,
          SectEndMark :      {End of section}
           begin
              Done := True;
              NewLine;
              LastLine := LineCnt;
           end;

        else
          Inc(Pcol);
        end;
        Inc(Bpos);
      until Done;
        {use screen height if resize disabled}
      if not HelpResize then
        NewHeight := Height - 1;
    end;
  end;


  {$IFDEF UseMouse}
  procedure HiddenMouse;
  begin
    if HelpMouseEnabled then
      HideMouse;
  end;


  procedure VisibleMouse;
  begin
    if HelpMouseEnabled then
      ShowMouse;
  end;


  procedure WaitForMouseRelease;
  begin
    if HelpMouseEnabled then
      while MousePressed do
        inline($cd/$28);    {let TSRs pop up while we wait for release}
  end;


  function GetKeyStart(KeyString : String; AllKeys : String) : Byte;
    begin
      GetKeyStart := Pos(KeyString, AllKeys) + BarDispCol -1;
    end;

  procedure InitMouseBarKeys(Help : HelpPtr);
  const
    D : Charset = ['≥'];
  var
    KeyList : String;
    KeyStr : String;
    Keys : byte;

  begin
    if HelpMouseEnabled then
      with Help^, Hdr, HelpState do begin
        KeyList := StdKeys[KeyListNum];
        BarDispCol := (((ColH+Width-1) - Length(KeyList)) div 2) + 1;
        for Keys := 1 to NMaxKeys do begin
          KeyStr := ExtractWord(Keys, KeyList, D);
          BarReg[Keys].Start := GetKeyStart(KeyStr, KeyList);
          BarReg[Keys].Last := BarReg[Keys].Start + Length(KeyStr)-1;
        end;
      end;
  end;


  function GetBBCommand(Help : HelpPtr; XPos : Byte) : HKType;
    {return a command based on where on button the cursor is}
  var
    ButtonClicked : word;
  begin
    with Help^, HelpState do begin
      for ButtonClicked := 1 to NMaxKeys do begin
        if (XPos >= BarReg[ButtonClicked].Start) and
            (XPos <= BarReg[ButtonClicked].Last) then begin
          GetBBCommand := StdKeyCommands[KeyListNum, ButtonClicked];
          Exit;
        end;
      end;
    end;
    GetBBCommand := HKSNone;
  end;
  {$ENDIF}


  procedure ButtonBar(Help : HelpPtr);
   {display a mouse-hot bar with commands}
  var
    KeyList : String;
  begin
    HiddenMouse;
    with Help^, HelpState, Hdr, WindowP(BBWin)^, Draw do begin
      KeyList := StdKeys[KeyListNum];
      BarDispCol := (((ColH+Width-1) - Length(KeyList)) div 2) + 1;
      FastWrite(KeyList, Height, BarDispCol, A[BBAttr]);
    end;
    VisibleMouse;
  end;


  procedure DoIndent(R : byte; var C : byte; IndentWidth, Attr : byte);
    {do any indent}
  begin
    if IndentWidth > 0 then begin
      Blanks[0] := Char(IndentWidth);
      FastWriteWindow(Blanks, R, C, Attr);
      C := C + IndentWidth;
    end;
  end;


  procedure DrawLine(Help : HelpPtr; Start : Word; WriteRow : Byte);
    {-Draw one line of help}
  var
    Bpos : Word;
    Attr : Byte;
    R : Byte;
    C : Byte;
    Ch : Char;
    AtSt : LineAttrRec;
    Indenting : Boolean;
    Finished : Boolean;

  begin

    with Help^, HelpState, AtSt do begin

      Bpos := L[Start];              {starting position of top line}
      R := WriteRow;                 {window relative row}
      C := ColMin;
      Indenting := True;
      Finished := False;

      AtSt := LineA[Start];
      Attr := FlexSt[FlexSp];

      DoIndent(R, C, LineI[Start], A[TeAttr]);

      if (Attr = A[XrAttr]) and (R = 1) then  {don't display partial xref}
        Attr := A[TeAttr];

      repeat
        Ch := BufP^[Bpos];
        case Ch of

          TTYToggle, IgnoreMark:
            ;

          IndentMark :
            Inc(Bpos);                    {skip indent amount}

          LineBrkMark, PageBrkMark :
            Finished := True;

          Attr1Toggle..Attr3Toggle :
            if (FlexSp > 0) and (FlexSt[FlexSp] = AC[Ch]) then begin
              {Toggling current state off}
              Dec(FlexSp);
              Attr := FlexSt[FlexSp];
            end else if FlexSp < FlexStackSize then begin
              {Changing to new attribute}
              Inc(FlexSp);
              Attr := AC[Ch];
              FlexSt[FlexSp] := Attr;
            end;

          XrefToggle :
            if (FlexSp > 0) and (FlexSt[FlexSp] = A[XrAttr]) then begin
              {Toggling current state off}
              Dec(FlexSp);
              Attr := FlexSt[FlexSp];
            end else if FlexSp < FlexStackSize then begin
              {Changing to new attribute}
              Inc(FlexSp);
              if Bpos = X[Xnum].Bofs then
                {Selected cross-ref}
                Attr := A[XsAttr]
              else
                {Deselected cross-ref}
                Attr := A[XrAttr];
              FlexSt[FlexSp] := A[XrAttr];
            end;

          IndexMarker :
            {Skip over topic number}
            Inc(Bpos, 2);

          TTYOnlyMark,
          SectEndMark :
            Finished := True;

        else
          if Ch = EscapeChar then begin
            Inc(Bpos);                    {move to next character}
            Ch := BufP^[Bpos]             {get next character to write it}
          end;

          if C <= ColMax then begin
            if Indenting and (Ch = ' ') then
              FastWriteWindow(Ch, R, C, A[TeAttr])
            else begin
              FastWriteWindow(Ch, R, C, Attr);
              Indenting := False;
            end;
            Inc(C);
            end;
          end;
        Inc(Bpos);
      until Finished;
    end;
  end;


  procedure DrawPage(Help : HelpPtr; Start : Word);
    {-Draw one page of help}
  var
    Bpos : Word;
    Bend : Word;
    Attr : Byte;
    R : Byte;
    C : Byte;
    Ch : Char;
    AtSt : LineAttrRec;
    PrevMX, PrevMY : Byte;
    {LineNum : Word;}
    Indenting : Boolean;


  begin

    HiddenMouse;

    with Help^, HelpState, AtSt do begin

      Bpos := L[Start];              {starting position of top line}

      if LastLine <= Height-3 then
        Bend := L[LastLine+1]         {LastLine is end of Buffer}
      else
        Bend := L[Start+Height-3];  {buffer offset of last line on screen }

      R := 1;                       {window relative row}
      C := ColMin;

      AtSt := LineA[Start];
      Attr := FlexSt[FlexSp];
      ClrScr;

      DoIndent(R, C, LineI[Start], A[TeAttr]);

      repeat

        Ch := BufP^[Bpos];
        case Ch of

          TTYToggle, IgnoreMark:
            ;

          IndentMark :
            Inc(Bpos);                    {skip indent width byte}

          PageBrkMark,
          LineBrkMark :
            begin
              Inc(R);
              Inc(Start);
              C := ColMin;
              if (R < Height - 2) then
                DoIndent(R, C, LineI[Start], A[TeAttr]);
              Indenting := True;
            end;

          Attr1Toggle..Attr3Toggle :
            if (FlexSp > 0) and (FlexSt[FlexSp] = AC[Ch]) then begin
              {Toggling current state off}
              Dec(FlexSp);
              Attr := FlexSt[FlexSp];
            end else if FlexSp < FlexStackSize then begin
              {Changing to new attribute}
              Inc(FlexSp);
              Attr := AC[Ch];
              FlexSt[FlexSp] := Attr;
            end;

          XrefToggle :
            if (FlexSp > 0) and (FlexSt[FlexSp] = A[XrAttr]) then begin
              {Toggling current state off}
              Dec(FlexSp);
              Attr := FlexSt[FlexSp];
            end else if FlexSp < FlexStackSize then begin
              {Changing to new attribute}
              Inc(FlexSp);
              if Bpos = X[Xnum].Bofs then
                {Selected cross-ref}
                Attr := A[XsAttr]
              else
                {Deselected cross-ref}
                Attr := A[XrAttr];
              FlexSt[FlexSp] := A[XrAttr];
            end;

          IndexMarker :
            {Skip over topic number}
            Inc(Bpos, 2);

          TTYOnlyMark,
          SectEndMark :
            Bpos := Bend;                 {force exit}

        else
          if Ch = EscapeChar then begin
            Inc(Bpos);                    {move to next character}
            Ch := BufP^[Bpos]             {get next character to write it}
          end;
          if C <= ColMax then begin
            if Indenting and (Ch = ' ') then
              FastWriteWindow(Ch, R, C, A[TeAttr])
            else begin
              FastWriteWindow(Ch, R, C, Attr);
              Indenting := False;
            end;
            Inc(C);
          end;
        end;
        Inc(Bpos);
      until (R >= Height-2) or (Bpos >= Bend);  { make this screen relative }
    end;
    VisibleMouse;
  end;


  procedure DrawXref(Help : HelpPtr; Num : Xrefs);
    {-Draw Xref in appropriate attribute}
  var
    Bpos : Word;
    Bend : Word;
    Attr : Byte;
    R : Byte;
    C : Byte;
    Ch : Char;
    Indenting : Boolean;

  begin
    with Help^, HelpState, X[Num] do begin

      if (Num < 1) or (Num > XCnt) then
        exit;

      HiddenMouse;
      Indenting := False;

      Bpos := Bofs+1;
      if LastLine <= Height-3 then
        Bend := L[LastLine+1]-1         {LastLine is end of Buffer}
      else
      Bend := L[NewTopLine+Height-3];

      R := Line - NewTopLine+1;

      C := Col;
      if Num = Xnum then
        Attr := A[XsAttr]
      else
        Attr := A[XrAttr];

      repeat
        Ch := BufP^[Bpos];
        case Ch of
          PageBrkMark,
          LineBrkMark :
            begin
              Inc(R);
              C := ColMin + LineI[R + NewTopLine - 1];
              Indenting := True;
              if R >= Height-2 then
                Bpos := Bend;             {force exit}
            end;

          IndentMark:
            Inc(Bpos);

          IgnoreMark:							{skip xref ignore marks}
				;

          XrefToggle,
          TTYOnlyMark,
          SectEndMark :
            Bpos := Bend;                 {force exit}

        else
          if Ch = EscapeChar then begin
            Inc(Bpos);                    {move to next character}
            Ch := BufP^[Bpos]             {get next character to write it}
          end;
          if C <= ColMax then begin
            if Indenting and (Ch = ' ') then
              FastWriteWindow(Ch, R, C, A[TeAttr])
            else begin
              FastWriteWindow(Ch, R, C, Attr);
              Indenting := False;
            end;
            Inc(C);
          end;
        end;
        Inc(Bpos);
      until Bpos >= Bend;
    end;
    VisibleMouse;
  end;


  procedure DrawSearchString(Help : HelpPtr; SearchString : string; Attr : byte);
  begin
    with Help^ do
      FastWrite(SearchString, Height - 1, ColH + 2, Attr);
  end;


  function GetNibble : Byte;
    {-Return next nibble from source}
  begin
    if Nibble then begin
      {Buffered nibble available}
      GetNibble := BN shr 4;
      Nibble := False;
      Inc(SI);
    end else begin
      {First nibble of byte}
      BN := Ord(SPtr^[SI]);
      GetNibble := BN and $0F;
      Nibble := True;
    end;
  end;


  procedure Decompress(var X : XlateArray; Len : Word; S, D : CharArrayPtr);
    {-Decompress text of length Len at S to position D, using X for translation}
  var
    N : Byte;
    C : Char;
  begin
    Nibble := False;
    SI := 0;
    DI := 0;
    SPtr := S;
    Dptr := D;
    while SI < Len do begin
      N := GetNibble;
      case N of
        $0F:                              {three-nibble actual ASCII code}
          begin
            N := GetNibble;
            C := Char((GetNibble shl 4) or N);
          end;
        $0E:                              {two-nibble translation code}
           C := Char(X[GetNibble + 14]);
      else                                {one-nibble translation code}
        C := Char(X[N]);
      end;
      Dptr^[DI] := C;
      Inc(DI);
    end;
  end;


  function LoadHelp(Help : HelpPtr; Topic : Word) : Boolean;
    {-Load and decompress one help topic}
  var
    BytesRead : Word;
    Frec : HelpIndexRec;
    Comp : CharArrayPtr;
  begin
    LoadHelp := False;

    with Help^, Hdr do begin
      if InRAM then begin
        {Already in memory, just compute the pointer}
        Frec := IndP^[Topic];
        {Check for available help}
        if Frec.Start = NoHelpAvailable then
          Exit;
        Comp := Ptr(SO(HdrP).S, SO(HdrP).O+Frec.Start);

      end else if FileRec(Fil).Mode = fmInOut then begin
        {On disk, first read the index}
        Seek(Fil, (SizeOf(HelpHeader)+LongInt(NameSize)*HighestTopic+
          SizeOf(HelpIndexRec)*(Topic-1)));
        if IoResult <> 0 then
          Exit;
        BlockRead(Fil, Frec, SizeOf(HelpIndexRec), BytesRead);
        if (IoResult <> 0) or (BytesRead <> SizeOf(HelpIndexRec)) then
          Exit;
        {Check for available help}
        if Frec.Start = NoHelpAvailable then
          Exit;
        {Now read the help section}
        Seek(Fil, Frec.Start);
        if IoResult <> 0 then
          Exit;
        {Put compressed version at top of buffer}
        Comp := @BufP^[BiggestTopic-Frec.CompLen];
        BlockRead(Fil, Comp^, Frec.CompLen, BytesRead);
        if (IoResult <> 0) or (BytesRead <> Frec.CompLen) then
          Exit;
      end else
        {Help file not open}
        Exit;

      {Decompress text into BufP^[0]}
      Decompress(XlateTable, Frec.CompLen, Comp, BufP);

      {set the map pointer}
      if (TBufPtr = 0) or (TBuff^[TbufPtr] <> Topic) then
        TBufPtr := MapPointer(Help, Topic);

      LoadHelp := True;
    end;
  end;


  function FirstXref(Help : HelpPtr) : Xrefs;
    {-Return index of first xref on page, 0 if none}
  var
    Inum : Xrefs;
  begin
    with Help^, HelpState do begin
      for Inum := 1 to Xcnt do
        with X[Inum] do
          if (Line >= NewTopLine) and (Line <= (NewTopLine+Height-4))
              then begin
            FirstXref := Inum;
            Exit;
          end;
    end;
    FirstXref := 0;
  end;


  function LastXref(Help : HelpPtr) : Xrefs;
    {-Return index of last xref on page, 0 if none}
  var
    Inum : Xrefs;
  begin
    with Help^, HelpState do begin
      for Inum := Xcnt downto 1 do
        with X[Inum] do
          if (Line >= NewTopLine) and (Line <= (NewTopLine+Height-4))
              then begin
            LastXref := Inum;
            Exit;
          end;
    end;
    LastXref := 0;
  end;


  {$IFDEF UseMouse}
  function MatchXref(Help : HelpPtr; Num : Xrefs; MX, MY : Byte) : Boolean;
    {-Return true if any portion of xref intersects MX, MY}
  var
    Bpos : Word;
    Bend : Word;
    R : Byte;
    C : Byte;
  begin
    MatchXref := False;
    with Help^, HelpState, X[Num] do begin
      Bpos := Bofs+1;
      if LastLine <= Height-3 then
        Bend := L[LastLine+1]-1             {LastLine is end of Buffer}
      else
      Bend := L[CurrentTopLine+Height-3];   {buffer offset of last visible line}

      if CurrentTopLine = 1 then            {draw xref relative to currenttopline}
        R := Line
      else
        R := Line - CurrentTopLine+1;
      C := Col;
      repeat
        case BufP^[Bpos] of
          LineBrkMark :
            begin
              Inc(R);
              C := ColMin;
            end;
          XrefToggle, PageBrkMark, SectEndMark, TTYOnlyMark, IndentMark :
            Exit;

          TTYToggle :
            ;

        else
          {Check for a match}
          if (R = MY) and (C = MX) then begin
            MatchXref := True;
            Exit;
          end;
          if Bufp^[Bpos] = EscapeChar then  {skip next if escape}
            Inc(Bpos);
          Inc(C);
        end;
        Inc(Bpos);
      until Bpos >= Bend;
    end;
  end;

{ *** 4.00 *** }
  function IsXRef(Help : HelpPtr; MX, MY : Byte) : Xrefs;
    {-Select xref, if any, at position MX,MY}
  var
    Inum : Xrefs;
    ScrnRow : Lines;
  begin
    with Help^, HelpState do begin
     for ScrnRow := CurrentTopLine to (CurrentTopLine+Height-4) do begin
      for Inum := 1 to Xcnt do
        if X[Inum].Line = ScrnRow then
          if MatchXref(Help, Inum, MX, MY) then begin
            IsXRef := Inum;
            Exit;
          end;
    IsXRef := 0;
    end;
   end;
  end;
{ *** 4.00 *** }

  function SliderPos : Byte;
    {-Calculate the slider position in absolute coordinates}
    {Basic formula: First pos+(Topline-1)*SBHeight div HeightLastLine}
    {Note that the final -1 is necessary}
  begin
    with HelpState, WindowP(W)^ do
      if Lastline-(YH-YL)-1 <> 0 then
        SliderPos := YL + longint((NewTopLine-1)*(YH-YL)) div longint(Lastline-(YH-YL)-1)
      else
        SliderPos := YL;
  end;

  procedure UpdateMouseFrame(Help : HelpPtr);
    {-Set mouse window coordinates and scroll bar}
  var
    SBar : Byte;

  begin
    HiddenMouse;
    with Help^, HelpState, WindowP(W)^, Draw do begin
      MouseScrollNow := MouseScroll and (LastLine-Height+3 > 0);
      if MouseScrollNow then begin
        {Let mouse move into frame}
        MouseWindow(XL1, YL1, XH1, YH1+1);
        {Draw scroll marks}
        FastWrite(MouseUpMark, YL1, XH1, FAttr);
        FastWrite(MouseDnMark, YH1, XH1, FAttr);

        {draw scrollbar in bar character}
        for SBar := 2 to YH1-1 do
          FastWrite(SBarChar, SBar, XH1, FAttr);

        {ShowMoreNow := False;}
        Lslid := 0;
      end; (* else
        {Don't let mouse move into frame}
        MouseWindow(XL1, YL1, XH, YH1); *)
      {Draw previous help mark}
      FastWrite(MouseQuitMark, YL1, XL1, FAttr);
    end;
    VisibleMouse;
  end;
  {$ENDIF}


  procedure IncPrim(Help : HelpPtr; Delta : Integer);
    {-Increment or decrement to next valid Xref}
  var
    Inum : Xrefs;
  begin
    with Help^, HelpState do
      if Xnum <> 0 then begin
        Inum := Xnum;
        repeat
          Inc(Xnum, Delta);
          if Xnum < 1 then
            XNum := Xcnt
          else if Xnum > Xcnt then
            XNum := 1;
        until (Xnum = Inum) or ((X[Xnum].Line <= CurrentTopLine+Height-4)
            and (X[Xnum].Line >= CurrentTopLine));
      end;
  end;


  procedure IncXref(Help : HelpPtr; Delta : Integer);
    {-Increment or decrement to next valid Xref and update screen}
  var
    Inum : Xrefs;
  begin
    with HelpState do begin
      Inum := Xnum;
      IncPrim(Help, Delta);
      if Inum <> Xnum then begin
        {Update highlights}
        DrawXref(Help, INum);
        DrawXref(Help, XNum);
      end;
    end;
  end;


  function NextXref(Help : HelpPtr; Delta : Integer) : boolean;
    {-Increment or decrement to next valid Xref, non-circular}
    { returns false if no more XRefs on screen in this direction}
  var
    Inum : Xrefs;
  begin
    with Help^, HelpState do begin
      Inum := XNum;
      Inc(XNum, Delta);
      if (XNum >= 1) and (XNum <= Xcnt) and
          ((X[XNum].Line <= CurrentTopLine+Height-4) and
          (X[XNum].Line >= CurrentTopLine)) then begin
        DrawXref(Help, INum);             {clear old}
        DrawXref(Help, XNum);             {draw new}
        NextXRef := true;
      end else begin
        XNum := INum;                     {forget it, stay on old one}
        NextXRef := false;
      end;
    end;
  end;


{ *** 4.00 *** }
  procedure ScrollUp(Help : HelpPtr);

    begin
      HiddenMouse;
      with Help^, HelpState do begin
          ScrollWindow(True, 1);
            {remove any partial XRef at top}
          DrawLine(Help, CurrentTopLine+1, 1);
            {draw new bottom line}
          DrawLine(Help, CurrentTopLine+Height-3, Height-3);
          DrawXRef(Help, XNum);           {highlight any new XRef at bottom}
        end;
      VisibleMouse;
    end;

  procedure ScrollDown(Help : HelpPtr);

    begin
      HiddenMouse;
      with Help^, HelpState do begin
          ScrollWindow(False, 1);
            {Draw new top line}
          DrawLine(Help, CurrentTopLine-1, 1);
            {Draw second line in case 2-line xref scrolled on}
          DrawLine(Help, CurrentTopLine, 2);
            {Be sure selected XRef is drawn}
          DrawXRef(Help, XNum);
{          DrawLine(Help, CurrentTopLine+Height-5, Height-3);}
      end;
      VisibleMouse;
    end;
{ *** 4.00 *** }

  procedure GetHeaderString(Help : HelpPtr; Topic : Word;
                            var HeaderStr : string);
    {-Return string for header of window}
  begin
    HeaderStr := GetNameString(Help, Topic);
    if Length(HeaderStr) > 0 then
      HeaderStr := ' '+HeaderStr+' ';
  end;

  procedure FrameHelp(Help : HelpPtr; Title : string);
    {-Draw titled frame around help window}
  begin
    with Help^, Hdr, HelpState do
      if ShowFrame then
        FrameWindow(ColH, RowH, ColH+Width-1, Height-1,  {changed}
          A[FrAttr], A[HeAttr], Title);
  end;

  function LoadNewTopic(Help : HelpPtr; Topic : Word) : Boolean;
    {-Return true if specified topic successfully loaded}
  var
    HeaderStr : string[80];
    Dummy : boolean;
  begin
    with Help^, Hdr, HelpState do
      if LoadHelp(Help, Topic) then begin
        InitTopic(Help);
        {Dummy := ResizeWindow(0, (NewHeight - OldHeight), ' ');}
        {OldHeight := NewHeight;}
        GetHeaderString(Help, Topic, HeaderStr);
        FrameHelp(Help, HeaderStr);
        LoadNewTopic := True;
      end else
        LoadNewTopic := False;
  end;

  procedure IncSp(var Sp : HelpStackIndex);
    {-Increment and wrap}
  begin
    if Sp = MaxHelpStack then
      Sp := 0
    else
      Inc(Sp);
  end;

  procedure DecSp(var Sp : HelpStackIndex);
    {-Decrement and wrap}
  begin
    if Sp = 0 then
      Sp := MaxHelpStack
    else
      Dec(Sp);
  end;

  procedure PushStack(Help : HelpPtr; Topic : Word; Line : Lines;
                      Xnum : Xrefs);
    {-Push a help topic onto stack}
  begin
    with Help^ do begin
{      if Topic <> 0 then begin {!!.06}
        with Stack[St] do begin
          STopic := Topic;
          SLine := Line;
          SXnum := Xnum;
        end;
        IncSp(St);
        if St = Sb then
          IncSp(Sb);
{      end;}
    end;
  end;

  procedure PopStack(Help : HelpPtr; var Topic : Word; var Line : Lines;
                     var Xnum : Xrefs);
    {-Pop help topic from stack}
  begin
    with Help^ do begin
(*
      if St = Sb then {!!.07}
        Topic := 0    {!!.07}
      else begin      {!!.07}
*)
        DecSp(St);
        with Stack[St] do begin
          Topic := STopic;
          Line := SLine;
          Xnum := SXnum;
        end;
{      end;              {!!.07}
    end;
  end;


  function NextPrevious(Help : HelpPtr) : Word;
    {-move to the next or previous topic in the map}
  begin
    with Help^.Hdr do begin
      If HelpCmdNum = HksPrevious then
        Dec(TBufPtr)
      else
        Inc(TBufPtr);
      If TBufPtr > NamedTopics Then TBufPtr := 1;
      If TBufPtr < 1 Then TBufPtr := NamedTopics;
      NextPrevious := TBuff^[TBufPtr];
    end;
  end;


  procedure ShowExternalHelp(Hdr : HelpHeader);
  var
    ExtHelpPath : String;
    SaveMode : Byte;
    WC : WindowCoordinates;
    ScreenBufPtr : Pointer;
    MSP : MouseStatePtr;
    ExtHelpError : Integer;
  begin
    with Hdr do begin
      ExtHelpPath := '';
      if ExtHelpEnv <> '' then
        ExtHelpPath := GetEnv(ExtHelpEnv);
      if ExtHelpPath = '' then
        ExtHelpPath := ExtHelpName;
      SaveMode := FileMode;
      FileMode := 0;
      ExtHelpError := -99;
      if ExistOnPath(ExtHelpPath, ExtHelpPath) then begin
        if SaveWindow(1, 1, ScreenWidth, ScreenHeight, True, ScreenBufPtr) then begin
          SaveMouseState(MSP, True);
          StoreWindowCoordinates(WC);
          Window(1, 1, ScreenWidth, ScreenHeight);
          ClrScr;
          ExtHelpError := ExecDos(ExtHelpPath, false, Nil);
          RestoreWindowCoordinates(WC);
          RestoreWindow(1, 1, ScreenWidth, ScreenHeight, True, ScreenBufPtr);
          RestoreMouseState(MSP, True);
        end;
      end;
      if ExtHelpError <> 0 then begin
        Beep;
        Beep;
      end;
      FileMode := SaveMode;
    end;
  end;


  function ShowHelpIndex(Help : HelpPtr) : Word;
    {-Display help index, return topic number if one chosen, or 0}
  begin
    with Help^ do
      ShowHelpIndex := PickHelp(Help, PickWindow, 1, 1, PickRows, PickCols,
                                false, false, PickChoice);
  end;

  function ShowHelpPrim(Help : HelpPtr;
                        Var Topic : Word;
                        Line : Lines) : Boolean;
    {-Display help screen, returning true if successful}
  label
    ScrollViaXRefs,
    DoPopStack,
    ExitHelp,
    SelectATopic,
    SwitchTopics,
    ShowIndex,   {!!.10}
    ReShowIndex,
    DoSearch,
    ExitPoint;
  var
    Test : Word;
    Done : Boolean;
    Choice : Word;
    Row : Word;
    ChWord : Word;
    JumpXRefLine : word;
    SavePickMatrix : Word;
    Key1 : Word;
    Key2 : Word;
    SavePickHelp : Pointer;  {!! 5.09}
    NumKeys : Byte;
    ScrollCount : Integer;      {for difference of lines}
    ScrollDraw : Boolean;    {flag to indicate screen redraw}
    FoundRow : Word;
    OldFoundMarkLine : Word;
    XRDelta : integer;
    XRDummy : boolean;
    JumpToLine : boolean;
    JumpToXRef : boolean;
    XRefSearchString : string[MaxXrefSearchLen];  {XRef alpha search string}
    XRefCompString : string[MaxXrefSearchLen];    {XRef search comparison
                                                   string}
    XRefSearchLen : byte absolute XRefSearchString;
    XRefBufIndex : word;     {XRef alpha search position in buffer}
    XRefCompLen : word;      {length for XRef search comparison}
    XRefMatch : boolean;     {true if XRef search matches so far}
    NewChar : char;          {new character in xref search string}

    {$IFDEF UseMouse}
    MX : Byte;               {Mouse absolute X position}
    MY : Byte;               {Mouse absolute Y position}
    SaveMX : Byte;           {Saved mouse state}
    SaveMY : Byte;
    SaveMXL : Byte;
    SaveMXH : Byte;
    SaveMYL : Byte;
    SaveMYH : Byte;
    SaveWaitFor : Boolean;   {Saved WaitForButtonRelease variable}
    SaveMouseOn : Boolean;   {Was mouse cursor on at entry}
    SavePickMouseEnabled : Boolean; {Was pick enabled for mouse}
    Slid : Byte;             {Slider position}
    OldXnum : Xrefs;
    NewXnum : Xrefs;
    TmpXnum : Xrefs;

    MouseRegion, OldMouseRegion : MouseRegionType;  {init to MouseOther}
    DragState, DragSave : DragStateType;            {init to DragNone}
    PageScrollViaBar, CommandWasMouse : boolean;    {init to false}
    KillDrag : Boolean;
    DragDistance : Integer;
    DragDelay, DragNewDelay : Word;
    NewHelpCommand : HKType;                        {stuff commands into loop}

    {$ENDIF}

    PC : PickColorArray;
    SaveFrameChars : FrameArray;
    HeaderStr : string[80];
  begin
    ShowHelpPrim := False;
    with Help^, Hdr, HelpState do begin

      {Validate request}
      if not IDOK(Help) then
        Exit;
      if (Topic = 0) or (Topic > HighestTopic) then
        Exit;

      {Set colors and frame}
      AC[Attr1Toggle] := A[SpAtt1];
      AC[Attr2Toggle] := A[SpAtt2];
      AC[Attr3Toggle] := A[SpAtt3];
      AC[IndexMarker] := 0;
      AC[XrefToggle] := A[XsAttr];
      PC[WindowAttr] := A[TeAttr];
      PC[FrameAttr] := A[FrAttr];
      PC[HeaderAttr] := A[HeAttr];
      PC[SelectAttr] := A[XsAttr];
      PC[AltNormal] := A[TeAttr];
      PC[AltHigh] := A[XsAttr];
      SaveFrameChars := FrameChars;

      {Get help text into memory and initialize pointer to it}
      if not LoadHelp(Help, Topic) then
        Exit;
      {Scan help text to find page boundaries and xref markers}
      InitTopic(Help);

(*** 4.00 ***)
      if Line > LineCnt then
        CurrentTopLine := LineCnt - Height - 3;
(*** 4.00 ***)

      ShowMoreNow := ShowMore and (LastLine-Height+4 > 0);
      HelpOnScreen := True;

      {$IFDEF UseMouse}
      SaveMouseOn := MouseCursorOn;
      if SaveMouseOn then
        HideMouse;
      {$ENDIF}

      {Display window}
      FrameChars := Frame;
      GetHeaderString(Help, Topic, HeaderStr);

      {create Button Bar Window}
      if not MakeWindow(BBWin,
         ColH, Height,
         ColH+Width-1-2*FrameDelta[ShowFrame], Height,
         False, True, True,
         A[BBAttr], A[BBAttr], A[BBAttr], '') then
         goto ExitPoint;
      KeyListNum := 1;                    {Default to first set of button bar keys}
      if HelpMouseEnabled then
        InitMouseBarKeys(Help);            {added in 4.0}

      if not MakeWindow(WindowPtr(W),
        ColH, RowH,
        ColH+Width-1-2*FrameDelta[ShowFrame], Height-1,{was RowH+NewHeight}
        ShowFrame, True, true,
        A[TeAttr], A[FrAttr], A[HeAttr],
        HeaderStr) then
        goto ExitPoint;
      OldHeight := NewHeight;
      if not DisplayWindow(BBWin) then
        goto ExitPoint;

      if not DisplayWindow(W) then
        goto ExitPoint;
      if HideCursor then
        HiddenCursor;

      if not SetTopTiledWindow(W) then
        goto ExitPoint;

      {$IFDEF UseMouse}
      if HelpMouseEnabled then
        with WindowP(W)^, Draw do begin
          {Save current mouse parameters}
          SaveMX := MouseWhereX;
          SaveMY := MouseWhereY;
          SaveMXL := MouseXLo+1;
          SaveMXH := MouseXHi;
          SaveMYL := MouseYLo+1;
          SaveMYH := MouseYHi;
          SaveWaitFor := WaitForButtonRelease;
          {Set new mouse parameters}
          WaitForButtonRelease := True;
          UpdateMouseFrame(Help);
          {Position to top left corner of window}
          {MouseGoToXY(Width div 2, Height div 2); {was 1,1}
          VisibleMouse;
        end;
      {$ENDIF}

      UpdateMouseFrame(Help);

      ButtonBar(Help);                    {draw text in Button Bar Window}

      {Allow user to browse help}
{ *** 4.00 ***  Main Loop}
      ScrollDraw := True;
      ScrollCount := 0;
      NewTopLine := 1;
      CurrentTopLine := 0;
      OldFoundMarkLine := 0;
      XRDelta := 0;

      XRefSearchLen := 0;
      XRefMatch := false;

      MouseRegion := MouseOther;
      OldMouseRegion := MouseOther;
      DragState := DragNone;
      DragSave := DragNone;
      PageScrollViaBar := False;
      CommandWasMouse := False;
      NewHelpCommand := HKSNone;
      WaitForButtonRelease := False; {newmouse}

      with HelpState do begin
      Done := False;

      repeat

        if ((FoundMarkLine > 0) and ((CurrentTopLine = 0) or
            (FoundMarkLine < CurrentTopLine) or
            (FoundMarkLine > CurrentTopLine+Height-4))) then
          NewTopLine := FoundMarkLine;

        if (NewTopLine <> CurrentTopLine) then begin

          if NewTopLine > LastLine-Height+4 then
            NewTopLine := LastLine-Height+4;
          if NewTopLine < 1 then
             NewTopLine := 1;
          ScrollCount := NewTopLine - CurrentTopLine;
          OldXnum := Xnum;

          {adjust if selected XRef no longer on page}
          if (XNum = 0) or (X[XNum].Line < NewTopLine) then
            Xnum := FirstXref(Help)
          else if (X[XNum].Line > NewTopLine+Height-4) then
            XNum := LastXref(Help);

          {draw or scroll the page}
          if ScrollDraw or (Abs(ScrollCount) > 1) then begin
              DrawPage(Help, NewTopLine);
              OldFoundMarkLine := 0;      {old markers are gone}
          end else begin
            if ScrollCount = -1 then
              ScrollDown(Help)
            else if ScrollCount = 1 then
              ScrollUp(Help);
          end;

          CurrentTopLine := NewTopLine;
          ScrollDraw := False;

          {if we just scrolled 1 line and that changed the current
            xref, redraw it so the highlight shows}
          if (Abs(ScrollCount) = 1) and (Xnum <> 0) and (Xnum <> OldXNum) then
            DrawXref(Help, Xnum);

          {move to next xref if scrolling via XRefs}
          if (XNum = OldXNum) and (XRDelta <> 0) then begin
            XRDummy := NextXRef(Help, XRDelta);
          end;
          XRDelta := 0;

          {$IFDEF UseMouse}
{          if HelpMouseEnabled and MouseScrollNow then begin}
              {Draw slider}
              with WindowP(W)^.Draw do begin
                Slid := SliderPos;
                if Slid <> LSlid then begin
                  HiddenMouse;
                  if Lslid <> 0 then
                    FastWrite(SBarChar, Lslid, XH1, FAttr);
                  FastWrite(ScrollMark, Slid, XH1, FAttr);
                  Lslid := Slid;
                  VisibleMouse;
                end;
              end;
{          end;}
        end;

        {remove old "found" markers if any}
        FoundRow := OldFoundMarkLine - CurrentTopLine + 2;
        if ((OldFoundMarkLine > 0) and (FoundRow > 1) and
            (FoundRow < Height-1)) then begin
          FastWrite(' ', FoundRow, ColMin, A[HeAttr]);
          FastWrite(' ', FoundRow, ColMax+2, A[HeAttr]);
          OldFoundMarkLine := 0;
        end;

        {draw "found" markers if necessary}
        FoundRow := FoundMarkLine - CurrentTopLine + 2;
        if ((FoundMarkLine > 0) and (FoundRow > 1) and
            (FoundRow < Height-1)) then begin
          FastWrite('', FoundRow, ColMin, A[HeAttr]);
          FastWrite('', FoundRow, ColMax+2, A[HeAttr]);
          OldFoundMarkLine := FoundMarkLine;
          FoundMarkLine := 0;
        end;

        {Display the search string}
        if XRefSearchLen > 0 then
          DrawSearchString(Help, ' ' + XRefSearchString + ' ', A[HeAttr]);

        HelpCmdNum := HKSNone;            {default to no command}
        if CommandWasMouse then begin
          if DragState = DragNone then
              {non-repeatable mouse command, wait for mouse button to be released}
            WaitForMouseRelease
          else begin
              {repeatable command, see if mouse still pressed}
            if MousePressed then begin
              HelpCmdNum := HKSProbe;           {still pressed, fake a probe}
              MouseKeyWordX := MouseLastX;      {get current coordinates}
              MouseKeyWordY := MouseLastY;
            end else
              DragState := DragNone;            {button released, kill drag}
          end;
        end;
        {$ENDIF}

        if HelpCmdNum = HKSNone then
          if NewHelpCommand = HKSNone then
            HelpCmdNum := GetCommand(HelpKeySet, HelpKeyPtr, ChWord)
          else begin
            HelpCmdNum := NewHelpCommand;
            NewHelpCommand := HKSNone;            {reset NewHelpCommand}
          end;

        {Reset the search string and redraw the frame if appropriate}
        if (HelpCmdNum <> HKSAlpha) or
            ((not HelpSaveSrch) and (not XRefMatch)) then begin
          XRefSearchLen := 0;
          DrawSearchString(Help, CharStr(FrameChars[Horiz],
            SizeOf(XRefSearchString)), A[FrAttr]);
        end;

        JumpToLine := false;
        JumpToXRef := false;

        case HelpCmdNum of

          {Alpha -- search XRefs}
          HKSAlpha :
            begin
              NewChar := UpCase(Char(Lo(ChWord)));

              if (NewChar >= #32) and (NewChar <= #127) then begin

                {Accumulate search string}
                if XRefSearchLen < MaxXRefSearchLen then begin
                  Inc(XRefSearchLen);
                  XRefSearchString[XRefSearchLen] := NewChar;
                end;

                {Search each xref for a match}
                OldXNum := XNum;
                XRefMatch := false;

                repeat

                  {Check this xref}
                  XRefBufIndex := X[XNum].Bofs + 1;
                  XRefCompLen := X[XNum].Len;

                  {Skip ignore mark and first character if requested} 
                  if BufP^[XRefBufIndex] = IgnoreMark then begin
                    Inc(XRefBufIndex, 2);
                    Dec(XRefCompLen);
                  end;

                  {Copy the XRef text for comparison}
                  XRefCompString[0] := Chr(XRefSearchLen);
                  Move(BufP^[XRefBufIndex], XRefCompString[1], XRefSearchLen);

                  {Compare search string to xref if string is not too long}
                  XRefMatch := (XRefSearchLen <= XRefCompLen) and
                    (CompString(XRefSearchString, StUpCase(XRefCompString))
                    = Equal);

                  {Move to next xref}
                  Inc(XNum, 1);

                until XRefMatch or (XNum > XCnt);

                if XRefMatch then begin
                  Inc(XNum, -1);
                  if (X[XNum].Line < CurrentTopLine) or
                      (X[XNum].Line > CurrentTopLine+Height-4) then
                    NewTopLine := X[XNum].Line  {not on screen, redraw}
                  else begin
                    DrawXref(Help, OldXNum);  {on screen, clear old XRef}
                    DrawXref(Help, XNum);     {and draw new}
                  end;
                end else begin
                  XNum := OldXNum;        {restore previous XRef number}
                  Beep;
                  Dec(XRefSearchLen);     {discard erroneous character}
                end;
              end;
            end;

          HKSUp :
            NewTopLine := CurrentTopLine-1;

          HKSDown :
            NewTopLine := CurrentTopLine+1;

          {Commands to move xref highlight}
          HKSLeft :
            IncXRef(Help, -1);            {move xref up, circular}
          HKSRight :
            IncXRef(Help, +1);            {move xref down, circular}


          {Commands to scroll via XRefs}
          HKSXUp :
          begin
            XRDelta := -1;
            goto ScrollViaXRefs
          end;

          HKSXDown :
          begin
            XRDelta := 1;
ScrollViaXRefs:
            {if no XRef or we can't move the XRef, scroll the display}
            if (XNum = 0) or (not NextXRef(Help, XRDelta)) then
              NewTopLine := CurrentTopLine + XRDelta
          end;


          {Commands to select another page of help}
          HKSPgUp :
          begin
            if CurrentTopLine <> 1 then begin
              NewTopLine := CurrentTopLine-Height+4;
              ScrollDraw := True;
            end;
          end;

          HKSPgDn :
          begin
            if CurrentTopLine <> LastLine-Height+4 then begin
              NewTopLine := CurrentTopLine+Height-4;
              ScrollDraw := True;
            end;
          end;

          HKSHome :
          begin
            NewTopLine := 1;
            ScrollDraw := True;
          end;

          HKSEnd :
          begin
            NewTopLine := LastLine-Height+4;
            ScrollDraw := True;
          end;

          HKSPrint :
            {send the current topic's TTY Output to the std printer}
            PrintTopic(Help, Topic);

          HKSNSearch :
            {search again}
            if FoundLine > 0 then
              goto DoSearch
            else
              Beep;

          HKSTSearch :
          begin
            {search for text within this topic}
            SearchGlobal := false;
            SearchStartPos := 0;
            goto DoSearch;
          end;

          HKSGSearch :
          begin
            {search for text globally}
            SearchGlobal := true;
            SearchStartPos := 0;
DoSearch:
            if SearchText(Help, Topic, SearchGlobal) then begin
              {changed to a new topic}
              CurrentTopLine := 0;
              ScrollDraw := True;
              ScrollCount := 0;
              ShowMoreNow := ShowMore and (LastLine-Height+4 > 0);
            end;
          end;

          {$IFDEF UseMouse}
          {Mouse probe}
          HKSProbe :
          begin
          if HelpMouseEnabled then
            with WindowP(W)^, Draw do begin
              {Get absolute mouse coordinate}
              MX := MouseXLo+MouseKeyWordX;
              MY := MouseYLo+MouseKeyWordY;

              if (MX = XL1) and (MY = YL1) then
                  MouseRegion := MouseClose
              else if MouseScrollNow and ((MX = XH1) or (MX = XH1 - 1))
                  then begin
                    {In the scroll bar region}
                MouseRegion := MouseScrollBar;  {assume scroll bar}
                    {if we aren't in the middle of dragging the slider, see
                     if the mouse cursor is on a line up or line down arrow}
                if ((DragState <> DragSlider) and
                    ((DragState <> DragHold) or (DragSave <> DragSlider)))
                    then
                  if MY = YL1 then MouseRegion := MouseLineUp
                  else if MY = YH1 then MouseRegion := MouseLineDown
                  else if MY = YH1 + 1 then MouseRegion := MouseOther;
              end else if (MY = YH1+1) and (MX < XH1) then
                MouseRegion := MouseButtonBar
              else {if (MY >= YL) and (MY <= YH) then}
                MouseRegion := MouseOther;

                {see if we were already dragging}
              if DragState <> DragNone then begin
                {we were dragging -- is the mouse in the same region?}
                if MouseRegion = OldMouseRegion then begin
                  {yes it's the same region, repeat the action}
                  if DragState = DragHold then      {first remove any hold}
                    DragState := DragSave;

                    {for the slider, set a single line scroll with no delay}
                  if DragState = DragSlider then begin

                    if DragDistance = 0 then begin
                        {start dragging if mouse moved}
                      if MY > Lslid then
                        DragDistance := 1            {scroll up, no delay}
                      else if MY < Lslid then
                        DragDistance := -1;          {scroll down, no delay}

                        {stop dragging if slider moved down to meet us, but
                         be sure we don't get stuck at bottom of screen}
                    end else if ((DragDistance > 0) and (MY <= LSlid) and
                        ((LSlid < YH) or (CurrentTopLine = (LastLine - Height
                         + 4)))) then begin
                      DragDistance := 0;
                      MouseGotoXY(MX, LSlid);

                        {stop dragging if slider moved up to meet us, but
                         be sure we don't get stuck at top of screen}
                    end else if ((DragDistance < 0) and (MY >= LSlid) and
                        ((LSlid > YL) or (CurrentTopLine = 1))) then begin
                      DragDistance := 0;
                      MouseGotoXY(MX, LSlid);

                    end;

                    {it's not the slider, so we must be scrolling}
                  end else begin                    {not moving slider}
                    if PageScrollViaBar then begin
                      if (((DragDistance > 0) and (MY <= Lslid)) or
                          ((DragDistance < 0) and (MY >= Lslid)))
                          then
                        KillDrag := true;           {reached slider, kill drag}
                    end;
                    while ((DragDelay > 0) and (not KillDrag)) do begin  {do any delay}
                      Delay(50);                    {wait for a clock tick}
                      Dec(DragDelay,50);            {adjust remaining delay}
                      KillDrag := not MousePressed; {quit if button released}
                    end;
                    DragDelay := DragNewDelay;      {adjust repeat rate after 1st time}
                  end;

                  if KillDrag then begin
                    DragState := DragNone;
                    DragDistance := 0;
                    DragDelay := 0;
                    DragNewDelay := 0;
                  end;

                {we were dragging -- but the mouse moved out of the region}
                end else if DragState <> DragHold then begin
                  DragSave := DragState;  {save state}
                  DragState := DragHold;  {hold until something changes}
                end;

              {we weren't dragging, set up the appropriate event for the probe}
              end else begin
                DragDelay := 0;           {initialize first delay}
                DragNewDelay := 0;        {initialize repeat delay}
                DragDistance := 0;        {initialize distance}
                KillDrag := False;        {initialize kill}
                PageScrollViaBar := False;  {initialize scroll bar paging}

                case MouseRegion of

                  MousePgUp, MousePgDn:   {move a page at a time}
                    begin
                      DragState := DragScroll;      {start scrolling}
                      DragDelay := PageDragFirst;   {delay for first repeat}
                      DragNewDelay := PageDragRep;  {subsequent repeats}
                      DragDistance := Height - 4;   {scroll distance}
                      if MouseRegion = MousePgUp then   {switch direction for PgUp}
                        DragDistance := -DragDistance;
                    end;

                  MouseLineUp, MouseLineDown:       {move a line at a time}
                    begin
                      DragState := DragScroll;      {start scrolling}
                      DragDelay := LineDragFirst;   {delay for first repeat}
                      DragNewDelay := LineDragRep;  {subsequent repeats}
                      DragDistance := 1;            {scroll distance}
                      if MouseRegion = MouseLineUp then  {switch direction for line up}
                        DragDistance := -DragDistance;
                    end;

                  MouseScrollBar:                   {in the scroll bar}
                    begin
                    if MY = Lslid then
                      DragState := DragSlider      {just set slider state}
                    else begin
                      DragState := DragScroll;      {start scrolling}
                      PageScrollViaBar := true;     {set scroll bar paging}
                      DragDelay := PageDragFirst;   {delay for first repeat}
                      DragNewDelay := PageDragRep;  {subsequent repeats}
                      DragDistance := Height - 4;   {scroll distance}
                      if MY < Lslid then            {switch direction for PgUp}
                        DragDistance := -DragDistance;
                      end;
                    end;

                  MouseClose:                       {close the window}
                    NewHelpCommand := HKSExit;

                  MouseButtonBar :
                    NewHelpCommand := GetBBCommand(Help, MX);

                  MouseOther:
                    begin     {In active pick region, convert to window relative}
                      Dec(MX, XL-1);
                      Dec(MY, YL-1);
                        {Select another xref if possible}
                      NewXnum := IsXRef(Help, MX, MY);
                      if NewXnum <> 0 then
                        if NewXnum = Xnum then
                          {Second click on item, select it}
                          goto SelectATopic
                        else begin
                          {Move highlight to item}
                          TmpXnum := Xnum;
                          Xnum := NewXnum;
                          DrawXref(Help, TmpXnum);
                          DrawXref(Help, Xnum);
                        end;
                    end;

                end;                                {case MouseRegion}

                    {save region for next pass}
                if (DragState = DragScroll) or (DragState = DragSlider) then
                  OldMouseRegion := MouseRegion;
              end;                                  {we weren't dragging}

            if (DragState = DragScroll) or (DragState = DragSlider) then   {adjust top line}
              NewTopLine := CurrentTopLine + DragDistance;

            CommandWasMouse := True;              {show we had a mouse command}

            end;   {With ...}
          end;    {HKSProbe}


          {$ENDIF}

          {Commands to exit help or select another topic}

          HKSToC :                        {show Table of Contents if available}
            begin
              if FirstTopic = 0 then
                goto ShowIndex;
              PushStack(Help, Topic, CurrentTopLine, Xnum);
              Topic := FirstTopic;
              goto SwitchTopics;
            end;

          HKSIndex :
            begin
ShowIndex:
              PushStack(Help, Topic, CurrentTopLine, Xnum);
ReShowIndex:
              Topic := ShowHelpIndex(Help);
              case Topic of
                0: 
                  goto DoPopStack;
                $FFFF:
                  begin
                    ShowExternalHelp(Hdr);
                    goto ReShowIndex;
{                    goto DoPopStack;}
                  end;
                else begin
                  PushStack(Help, 0, 0, 0);   {put index on stack}
                  goto SwitchTopics;
                end;
              end;
            end;

          HKSBack :
DoPopStack:
            if St = Sb then begin
              HelpCmdNum := HKSExit;      {fake an exit}
              goto ExitHelp;
            end else begin
              WaitForMouseRelease;
              {Restore previous displayed topic and page}
              PopStack(Help, Topic, Line, OldXNum);
              if (Topic = 0) then
                goto ReShowIndex;
              JumpToLine := true;
              JumpToXRef := true;
              Goto SwitchTopics;
            end;

          HKSExit, HKSQuickExit, HKSExitSave, HKSUser0..HKSUser3 :
ExitHelp:
            begin
              WaitForMouseRelease;
              Done := True;
              ShowHelpPrim := True;
            end;

          HksPrevious, HksNext :
            Begin
              PushStack(Help, Topic, CurrentTopLine, Xnum);
              Topic := NextPrevious(Help);
              Goto SwitchTopics;
            End;

          HKSKeys :
            Begin
              PushStack(Help, Topic, CurrentTopLine, Xnum);
              Topic := KeysTopic;
              Goto SwitchTopics;
            End;

          HKSSelect :
            If Xnum <> 0 then begin
SelectATopic:
              {topic 0 is index, FFFF is external help}
              case X[Xnum].Topic of
                0:
                  NewHelpCommand := HKSIndex;
                $FFFF:
                  ShowExternalHelp(Hdr);

                else begin
  
                  {Save current help topic and page}
                  PushStack(Help, Topic, CurrentTopLine, Xnum);
                  Topic := X[Xnum].Topic;

SwitchTopics:
                  Done := not LoadNewTopic(Help, Topic);
                  if not Done then begin
                    CurrentTopLine := 0;
                    if JumpToLine then
                      NewTopLine := Line
                    else
                      NewTopLine := 1;
                    if JumpToXRef then begin
                      XNum := OldXNum;
                      OldXNum := 0;
                      XRDelta := 0;
                    end;
                    ScrollDraw := True;
                    ScrollCount := 0;
                    ShowMoreNow := ShowMore and (LastLine-Height+4 > 0);
                    XRefSearchLen := 0;
    
                    {$IFDEF UseMouse}
{                    if HelpMouseEnabled then}
                      UpdateMouseFrame(Help);
                    ButtonBar(Help);
                    {$ENDIF}
                  end;
                end;
              end;
            end;

          HKSExtHelp :
            ShowExternalHelp(Hdr);

          HKSSwapKeys :                   {swap button bar keys}
            begin
              Inc(KeyListNum);
              if KeyListNum > NKeyLists then
                KeyListNum := 1;
              if HelpMouseEnabled then
                InitMouseBarKeys(Help);
              ButtonBar(Help);
            end;

        end;
      until Done;

      {Restore the screen}

{========================================================================}
{ CAUTION!!  HksExitSave bypasses the normal screen cleanup routine when }
{ it exits.  It is provided so that the help screen can be retained when }
{ the program terminates.  Using it without terminating the program can  }
{ have unknown and undesired side-effects.                               }
{========================================================================}

      HiddenMouse;

      if HelpCmdNum = HksExitSave then
        DisposeWindow(W)
      else
        DisposeWindow(EraseTopWindow);

      if SetTopTiledWindow(BBWin) then
        DisposeWindow(EraseTopWindow);

(*
      If HelpCmdNum <> HksExitSave Then
        DisposeWindow(EraseTopWindow);
*)
      {$IFDEF UseMouse}
      if HelpMouseEnabled then begin
        {Restore mouse position and window}
        MouseWindow(SaveMXL, SaveMYL, SaveMXH, SaveMYH);
        MouseGoToXY(SaveMX, SaveMY);
        WaitForButtonRelease := SaveWaitFor;
        WaitForMouseRelease;
      end;
      if SaveMouseOn then
        ShowMouse
      else
        HideMouse;
      {$ENDIF}

ExitPoint:
      FrameChars := SaveFrameChars;
      HelpOnScreen := False;
    end;
  end;
end; { with Helpstate }


  function ShowHelp(Help : HelpPtr; Var Topic : Word;
    StartAtMark : boolean) : Boolean;
      {-Display help screen, returning true if successful}
  var
    StartLine : Lines;
  begin
    with Help^ do begin
      St := 0;
      Sb := 0;
    end;
    StartLine := 1;
    with HelpState do
      if StartAtMark and (FoundMarkLine > 0) then
        StartLine := FoundMarkLine
      else begin
        SearchStartPos := 0;
        FoundMarkLine := 0;
      end;
    ShowHelp := ShowHelpPrim(Help, Topic, StartLine)
  end;


  function FindHelp(Help : HelpPtr; Name : string; MatchFunc : Pointer) : Word;
    {-Return topic number of help with specified Name, 0 if not found}
  label
    ExitPoint;
  var
    NP : StringPtr;
    I : Word;

    function CallMatch(S1, S2 : string) : Boolean;
      {-Call routine pointed to by MatchFunc}
    inline($FF/$5E/<MatchFunc); {Call dword ptr [bp+<MatchFunc]}

  begin
    FindHelp := 0;
    if MatchFunc = nil then
      MatchFunc := @Match;
    with Help^, Hdr do begin
      {Validate help structure}
      if not IDOK(Help) then
        Exit;
      {Match the name}
      NP := StringPtr(PBuff);
      for I := 1 to HighestTopic do
        if CallMatch(NP^, Name) then begin
          FindHelp := I;
          Exit;
        end else
          Inc(SO(NP).O, NameSize);
      {Clear the topic stack}
    end;
  end;

  function WordChar(Ch : Char) : Boolean;
    {-Return true if Ch is a character in a word}
  begin
    case Upcase(Ch) of
      'A'..'Z', '_', '0'..'9' : WordChar := True;
    else
      WordChar := False;
    end;
  end;


  function PickHelp(Help : HelpPtr; var PickW : WindowPtr;
                    XLow, YLow, YHigh, PickCols : Byte;
                    OldWindow, SaveWindow : boolean;
                    var Choice : word) : Word;
    {-Display help pick list, returning Topic number, or 0 for none}
  var
    XHigh : Byte;
    SaveFrameChars : FrameArray;
    SavePickMatrix : Byte;
    SavePickMouseEnabled : Boolean;
    PC : PickColorArray;
    PickRow : word;
    Topic : word;
  begin
    PickHelp := 0;
    with Help^, Hdr, HelpState do begin
      {Validate help structure}
      if not IDOK(Help) then
        Exit;
      {Set colors and frame}
      PC[WindowAttr] := A[TeAttr];
      PC[FrameAttr] := A[FrAttr];
      PC[HeaderAttr] := A[HeAttr];
      PC[SelectAttr] := A[XsAttr];
      PC[AltNormal] := A[TeAttr];
      PC[AltHigh] := A[XsAttr];
      SaveFrameChars := FrameChars;
      SavePickMatrix := TpPick.PickMatrix;

      {Set up global with NameSize}
      NSize := NameSize;

      FrameChars := Frame;

          {Set the pick matrix to the number of columns that fit}
{      if (PickCols * (NameSize + 1)) <= (ScreenWidth - XLow - 2) then}
      if (PickCols * (PickSize + 1)) <= (ScreenWidth - XLow - 2) then
        TpPick.PickMatrix := PickCols
      else 
{        TpPick.PickMatrix := (ScreenWidth - XLow - 2) div (NameSize + 1);}
        TpPick.PickMatrix := (ScreenWidth - XLow - 2) div (PickSize + 1);

          {And set the end of the window based on the pick matrix}
{      XHigh := XLow + (TpPick.PickMatrix * (NameSize + 1)) + 1;}
      XHigh := XLow + (TpPick.PickMatrix * (PickSize + 1)) + 1;
      if (FillPickScreen) then begin
        XHigh := TpCRT.ScreenWidth;
        YHigh := TpCRT.ScreenHeight;
      end;

      {$IFDEF UseMouse}
      if HelpMouseEnabled then begin
        {Assure mouse is also on in TPPICK}
        SavePickMouseEnabled := PickMouseEnabled;
        if not PickMouseEnabled then
          EnablePickMouse;
      end;
      {$ENDIF}

      {Pick from list}
(*      if OldWindow then
        OldWindow := DisplayWindow(PickW);*)

      if (OldWindow or MakeWindow(PickW, XLow, YLow, XHigh, YHigh,
          UseHelpFrame, true, true, PC[WindowAttr], PC[FrameAttr],
          PC[HeaderAttr], HelpTitle)) then begin
        PickRow := 1;
        FillPickWindow(PickW, @SendHelpName, NamedTopics,
                      PC, Choice, PickRow);
        PickBar(PickW, @SendHelpName, NamedTopics, PC, false,
                Choice, PickRow);
        if PickCmdNum = PKSSelect then
          PickHelp := TBuff^[Choice]
        else if PickCmdNum = PKSUser0 then begin
          Topic := 0;                     {force search of all topics}
          if SearchText(Help, Topic, true) then
            PickHelp := Topic;
        end;
      end;                            {if OldWindow ...}

      {$IFDEF UseMouse}
      if HelpMouseEnabled then
        if not SavePickMouseEnabled then begin
          {Assure mouse is now off in TPPICK}
          DisablePickMouse;
          {But that it stays on for TPHELP}
          EnableHelpMouse;
        end;
      {$ENDIF}
      FrameChars := SaveFrameChars;
      TpPick.PickMatrix := SavePickMatrix;
    end;

(*    PickW := EraseTopWindow;
    if not SaveWindow then
      DisposeWindow(PickW);*)

    if not SaveWindow then
      DisposeWindow(EraseTopWindow);

  end;

  function AddHelpCommand(Cmd : HKtype; NumKeys : Byte; Key1, Key2 : Word) : Boolean;
    {-Add a new command key assignment or change an existing one}
  begin
    AddHelpCommand := AddCommandPrim(HelpKeySet, HelpKeyMax, Cmd, NumKeys, Key1, Key2);
  end;

  procedure DisableHelpIndex;
    {-Disable the F1 help index inside of a help screen}
  var
    Junk : Boolean;
  begin
    Junk := AddHelpCommand(HKSNone, 1, $3B00, 0);
    {$IFDEF UseMouse}
    Junk := AddHelpCommand(HKSNone, 1, $ED00, 0);
    {$ENDIF}
    HelpIndexDisabled := True;
  end;

  procedure EnableHelpIndex;
    {-Enable the F1 help index inside of a help screen}
  var
    Junk : Boolean;
  begin
    Junk := AddHelpCommand(HKSIndex, 1, $3B00, 0);
    {$IFDEF UseMouse}
    Junk := AddHelpCommand(HKSIndex, 1, $ED00, 0);
    {$ENDIF}
    HelpIndexDisabled := False;
  end;

  {$IFDEF UseMouse}
  procedure EnableHelpMouse;
    {-Enable mouse control of the help system}
  begin
    if MouseInstalled then begin
      HelpKeyPtr := @TpMouse.ReadKeyOrButton;
      EnableEventHandling;
      HelpMouseEnabled := True;
    end;
  end;

  procedure DisableHelpMouse;
    {-Disable mouse control of the help system}
  begin
    if HelpMouseEnabled then begin
      HelpKeyPtr := @ReadKeyWord;
      DisableEventHandling;
      HelpMouseEnabled := False;
    end;
  end;
  {$ENDIF}

  procedure Beep;
    {error beep}
  begin
    Sound(880);
    Delay(200);
    NoSound;
    Delay(100);
  end;


  procedure OneLineWindow(Help : HelpPtr; LeftIndent, WinWidth,
    TopOffset : byte; TitleString : string);
  begin
    with Help^, Hdr do begin
      Shadow := true;
      if not MakeWindow(PopWindow, ColH+LeftIndent, RowH+TopOffset,
          ColH+LeftIndent+WinWidth, RowH+TopOffset+2, True, True, True,
          A[PWAttr], A[PWAttr], A[PWAttr], TitleString) then begin
        Shadow := false;
        exit;
      end;
      if not DisplayWindow(PopWindow) then
        exit;
      Shadow := false;
    end;
  end;


  procedure OneLinePrompt(Help : HelpPtr; InputLength : byte;
    PromptString : string; var InputString : string);
  var
    PWindow : WindowPtr;
    Escaped : boolean;
  begin
    with Help^, Hdr do begin
      WindowRelative := true;
      ReadString(PromptString, 1, 1, InputLength, A[PIAttr], A[PIAttr],
        A[PIAttr], Escaped, InputString);
      if Escaped then
        InputString := '';
    end;
  end;


  procedure PrintTopic(Help : HelpPtr; Topic : word);
    {Print the current topic to the standard printer}
  const
    PrintColWidth : Byte = 80;
    PrintPageLength : Byte = 58;
  var
    DeviceName : string[40];
    Bpos : Word;
    R : Byte;
    C : Byte;
    Ch : Char;
    TTYState : Boolean;
    Finished : Boolean;
    IOTest : Word;
    LineOut : String;
    IndentWidth : byte;
    SaveIndentWidth : byte;
    LineIndent : boolean;

  begin

    if LstOpen then
      DeviceName := LstName
    else
      DeviceName := 'LPT1';
{$IFDEF GERMAN}
    OneLineWindow(Help, 10, 58, 5, '');
    OneLinePrompt(Help, 40, ' Drucke nach: ', DeviceName);
{$ELSE}
    OneLineWindow(Help, 10, 55, 5, '');
    OneLinePrompt(Help, 40, ' Print to: ', DeviceName);
{$ENDIF}
    DisposeWindow(EraseTopWindow);
    if DeviceName = '' then
      exit;

    if LstOpen and (DeviceName <> LstName) then begin
      Close(Lst);
      LstOpen := false;
    end;
    
    if not LstOpen then begin
      Assign(Lst, DeviceName);
      Rewrite(Lst);
      LstOpen := True;
      LstName := DeviceName;
    end;
    if IOResult <> 0 then begin
      Beep; Beep; Beep;
      Exit;
    end;

    with Help^ do begin
      Bpos := 0;              {starting position of top line}
      C := 1;
      IndentWidth := 0;
      LineIndent := false;
      LineOut := '';
      Finished := False;
{$IFDEF GERMAN}
      WriteLn(Lst, 'Stichwort der 4DOS-Hilfe:  ', GetNameString(Help, Topic));
{$ELSE}
      WriteLn(Lst, '4DOS Help Topic:  ', GetNameString(Help, Topic));
{$ENDIF}
      WriteLn(Lst, '');
      R := 3;

      repeat
        Ch := BufP^[Bpos];
        case Ch of
          Attr1Toggle..Attr3Toggle, XrefToggle, TTYToggle :
            ;

          IndexMarker :
            {Skip over topic number}
            Inc(Bpos, 2);

          IndentMark :
            begin
              SaveIndentWidth := IndentWidth;
              IndentWidth := Byte(Bufp^[Bpos+1]);  {get indent width}
              if IndentWidth >= 128 then begin
                LineIndent := true;       {indent changes for one line only}
                IndentWidth := IndentWidth - 128;
              end;
              Inc(Bpos);                           {skip width}
            end;

          TTYOnlyMark,
          SectEndMark :
          begin
            if LineOut <> '' then         {the last line may contain text}
                WriteLn(Lst, LineOut);
            Finished := True;
          end;

        else
          if Ch = EscapeChar then begin
            Inc(Bpos);                    {move to next character}
            Ch := BufP^[Bpos]             {get next character to write it}
          end;

          if C <= PrintColWidth then      {printable character}
            case Ch of
              LineBrkMark, PageBrkMark :
                begin
                  if R = PrintPageLength then begin
                    WriteLn(Lst, #12);            {print 58 lines per page}
                    IOTest := IOResult;
                    if IOTest <> 0 then begin
                      Beep; Beep;
                      Exit;
                    end;
                    R := 1;
                    end;
                  WriteLn(Lst, LineOut);
                  IOTest := IOResult;
                  if IOTest <> 0 then begin
                    Beep; Beep;
                    Exit;
                  end;
                  LineOut := '';
                  C := 1;
                  Inc(R);
                  if LineIndent then begin
                    IndentWidth := SaveIndentWidth;
                    LineIndent := false;
                  end;
                end
              else begin
                if (C = 1) and (IndentWidth > 0) then begin
                  FillChar(LineOut[1], IndentWidth, ' ');
                  LineOut[0] := Char(IndentWidth);
                  Inc(C, IndentWidth);
                end;
                LineOut := Lineout + Ch;
                Inc(C);
              end;
            end;
          end;
        Inc(Bpos);
      until Finished;

    end;
    WriteLn(Lst, #12);         {insert FF at end of TTY print}
  end;


  function SearchText(Help : HelpPtr; var Topic : word;
    GlobalSearch : boolean) : boolean;
  {Search for a string}

  var
    Ch : char;
    Bpos : word;
    SearchPos : word;
    Finished : boolean;
    Found : boolean;
    BogusSpace : boolean;
    SkipIncr : boolean;
    DoingGlobal : boolean;
    SaveTopic : word;
    HeaderStr : string[80];
    SearchTitle : string[40];
    WindowUp : boolean;
    QuitGlobal : boolean;
    SearchAll : boolean;
    IndexBuf : HelpIndexPtr;
    IndexBufSize : word;
    SearchStartLine : Lines;
  label
    SearchExit;

  begin

    WindowUp := false;
    SearchText := false;
    IndexBuf := nil;
    
    with Help^, Hdr, HelpState do begin

{$IFDEF GERMAN}
      if GlobalSearch then
        SearchTitle := ' Globale Suche (Esc fÅr Abbruch) '
      else
        SearchTitle := ' Suche im Stichwort ';
{$ELSE}
      if GlobalSearch then
        SearchTitle := ' Global Search  (Esc to stop) '
      else
        SearchTitle := ' Topic Search ';
{$ENDIF}

      SearchAll := (Topic = 0);
      if SearchAll then begin
        SearchStartPos := 0;
        FoundMarkLine := 0;
        Topic := HighestTopic;            {force wrap to first topic}
        BufP^[0] := SectEndMark;          {force topic increment}
      end;

      if SearchStartPos = 0 then begin
        SearchString := '';
{$IFDEF GERMAN}
        OneLineWindow(Help, 5, 66, 5, SearchTitle);
        OneLinePrompt(Help, 40, ' Suchstring: ', SearchString);
{$ELSE}
        OneLineWindow(Help, 5, 70, 5, SearchTitle);
        OneLinePrompt(Help, 40, ' Enter search string: ', SearchString);
{$ENDIF}
        SearchLen := Length(SearchString);
        if SearchLen = 0 then begin
          DisposeWindow(EraseTopWindow);
          exit;
        end;
        if not GlobalSearch then
          DisposeWindow(EraseTopWindow)
        else
          WindowUp := true;               {get rid of window later}
        Bpos := 0;
        FoundLine := 1;
      end else begin
        Bpos := SearchStartPos + 1;
        if BufP^[Bpos] = EscapeChar then
          Inc(Bpos);
        if GlobalSearch then begin        {window for search progress msg}
          OneLineWindow(Help, 5, 70, 5, SearchTitle);
          WindowUp := true;
        end;
      end;
        
      SearchPos := 1;
      Finished := false;
      Found := false;
      BogusSpace := false;
      SkipIncr := false;
      DoingGlobal := false;
      QuitGlobal := false;

      repeat

        if BogusSpace then begin
          Ch := ' ';
          BogusSpace := false;
          SkipIncr := true;
        end else
          Ch := BufP^[Bpos];

        case Ch of
          Attr1Toggle..Attr3Toggle, XrefToggle, TTYToggle :
            ;

          IndexMarker :
            {Skip over topic number}
            Inc(Bpos, 2);

          IndentMark :
            Inc(Bpos);                    {skip indent width}

          LineBrkMark, PageBrkMark :
          begin
            Inc(FoundLine);
            BogusSpace := ((Bpos > 1) and (BufP^[Bpos - 1] <> ' '));
          end;

          TTYOnlyMark,
          SectEndMark :
            if not GlobalSearch then begin
              FoundLine := 0;               {didn't find it}
              Finished := true;
            end else begin
              if DoingGlobal then begin   {global search in process}
                if KeyPressed then
                  QuitGlobal := (Lo(ReadKeyWord) = $1B)  {Esc}
                else if MousePressed then
                  QuitGlobal := (MouseKeyWord = $EE00);  {Rt Button}
              end else begin              {not found in current topic,
                                           start new global search}
                SaveTopic := Topic;
                ClrScr;

                {Read the sequential list so we can move through it quickly}
                if not GetBuffer(Help, IndexBuf,
                    SizeOf(HelpHeader) + LongInt(HighestTopic)*(NameSize),
                    HighestTopic*SizeOf(HelpIndexRec), IndexBufSize)
                    then begin
                  Beep; Beep; Beep;
                  goto SearchExit;
                end;
{$IFDEF GERMAN}
                FastWriteWindow('Suche ... ', 1, 1, A[HeAttr]);
{$ELSE}
                FastWriteWindow('Searching ... ', 1, 1, A[HeAttr]);
{$ENDIF}
              end;
              DoingGlobal := true;

              repeat                      {find next valid topic in index}
                Inc(Topic);
                if Topic > HighestTopic then Topic := 1;
              until (IndexBuf^[Topic].Start <> NoHelpAvailable) or
                (Topic = SaveTopic);

              if QuitGlobal or (Topic = SaveTopic) or
                  (not LoadHelp(Help, Topic)) then begin
                Topic := SaveTopic;
                if (not SearchAll) and (not LoadNewTopic(Help, Topic))
                    then begin
                  Beep; Beep;
                  Halt(11);
                end;
                FoundLine := 0;
                Finished := True;
              end else begin
                BPos := $FFFF;              {increments to 0!}
                SearchPos := 1;
                FoundLine := 1;
{$IFDEF GERMAN}
                FastFillWindow(55, ' ', 1, 11, A[TeAttr]);
                FastWriteWindow(GetNameString(Help, Topic), 1, 11, A[TeAttr]);
{$ELSE}
                FastFillWindow(55, ' ', 1, 15, A[TeAttr]);
                FastWriteWindow(GetNameString(Help, Topic), 1, 15, A[TeAttr]);
{$ENDIF}
              end;
            end;

          else begin
            if Ch = EscapeChar then begin
              Inc(Bpos);                  {move to next character}
              Ch := BufP^[Bpos];          {get next character}
            end;
            if UpCase(Ch) <> UpCase(SearchString[SearchPos]) then begin
              if SearchPos > 1 then begin
                Bpos := SearchStartPos;   {no match, back up}
                FoundLine := SearchStartLine;
                SkipIncr := false;
              end;
              SearchPos := 1;             {restart comparison}
              BogusSpace := false;
            end else begin                {character matched}
              if SearchPos = 1 then begin
                SearchStartPos := Bpos;   {save start of matching string}
                SearchStartLine := FoundLine;
              end;
              Inc(SearchPos);
              if SearchPos > SearchLen then begin  {all matched, found it!}
                FoundLine := SearchStartLine;
                Finished := True;
              end;
            end;
          end;
        end;

        if not SkipIncr then
          Inc(Bpos);

        SkipIncr := false;

      until Finished;

      if WindowUp then
        DisposeWindow(EraseTopWindow);

      if FoundLine = 0 then begin
        if not QuitGlobal then
          Beep
      end else begin
        if DoingGlobal and (not SearchAll) then begin
          InitTopic(Help);
          GetHeaderString(Help, Topic, HeaderStr);
          FrameHelp(Help, HeaderStr);
          {$IFDEF UseMouse}
{          if HelpMouseEnabled then}
            UpdateMouseFrame(Help);
          ButtonBar(Help);
          {$ENDIF}
        end;
      end;

      FoundMarkLine := FoundLine;

    end;

    if IndexBuf <> nil then
      FreeMemCheck(IndexBuf, IndexBufSize);

    SearchText := DoingGlobal and (not QuitGlobal);

SearchExit:
  end;


  function ShowHelpTTY(Help : HelpPtr;
                       Var Topic : Word) : Boolean;
    {-Display help screen, returning true if successful}
  var
    Done : Boolean;
    HeaderStr : string[80];

  function IsTTYAvailable(Help : HelpPtr) : Boolean;
  {does the topic contain TTY text?}
  var
    Bpos : Word;
    Ch : Char;
    Finished : Boolean;

  begin
    with Help^ do begin
      Bpos := 0;              {starting position of top line}
      Finished := False;

      IsTTYAvailable := False;
      repeat
        Ch := BufP^[Bpos];

        case Ch of
          SectEndMark :
            Finished := True;

          IndentMark :
            Inc(Bpos);

          TTYOnlyMark,
          TTYToggle :
          begin
            IsTTYAvailable := True;
            Exit;
          end;
        end;
        Inc(Bpos);
      until Finished;
    end;
  end;

  procedure OutputTTY(Help : HelpPtr);
  var
    Bpos : Word;
    R : Byte;
    C : Byte;
    Ch : Char;
    TTYState : Boolean;
    Finished : Boolean;
    LineOut : String;
    IndentWidth : byte;
    SaveIndentWidth : byte;
    LineIndent : boolean;

  begin
    with Help^ do begin
      Bpos := 0;              {starting position of top line}
      C := 1;
      LineOut := '';
      TTYState := False;
      Finished := False;
      IndentWidth := 0;
      LineIndent := false;

      repeat
        Ch := BufP^[Bpos];
        case Ch of
          Attr1Toggle..Attr3Toggle, XrefToggle :
            ;

          IndexMarker :
            {Skip over topic number}
            Inc(Bpos, 2);

          SectEndMark :
          begin
            if LineOut <> '' then         {the last line may contain text}
                WriteLn(LineOut);
            Finished := True;
          end;

          TTYOnlyMark:
            TTYState := True;

          TTYToggle :
            TTYState := not TTYState;

          IndentMark :
          begin
            SaveIndentWidth := IndentWidth;
            IndentWidth := Byte(Bufp^[Bpos+1]);  {get indent width}
            if IndentWidth >= 128 then begin
              LineIndent := true;       {indent changes for one line only}
              IndentWidth := IndentWidth - 128;
            end;
            Inc(Bpos);                           {skip width}
          end;

        else
          if Ch = EscapeChar then begin
            Inc(Bpos);                    {move to next character}
            Ch := BufP^[Bpos]             {get next character to write it}
          end;
         if TTYState then
          case Ch of
            LineBrkMark, PageBrkMark :
              begin
                WriteLn(LineOut);
                LineOut := '';
                C := 1;
                if LineIndent then begin
                  IndentWidth := SaveIndentWidth;
                  LineIndent := false;
                end;
              end
            else begin
              if C <= ScreenWidth then begin
                if (C = 1) and (IndentWidth > 0) then begin
                  FillChar(LineOut[1], IndentWidth, ' ');
                  LineOut[0] := Char(IndentWidth);
                  Inc(C, IndentWidth);
                end;
                LineOut := Lineout + Ch;
                Inc(C);
                end;
              end;
            end;
          end;
        Inc(Bpos);
      until Finished;
    end;
  end;

  begin
    ShowHelpTTY := False;
    with Help^, Hdr, HelpState do begin
      {Validate request}
      if not IDOK(Help) then
        Exit;
      if (Topic = 0) or (Topic > HighestTopic) then begin
{$IFDEF GERMAN}
        WriteLn('UnzulÑssiges Hilfe-Stichwort gewÑhlt');
{$ELSE}
        WriteLn('Invalid Help Topic Chosen.');
{$ENDIF}
        Exit;
      end;

      {Get help text into memory and initialize pointer to it}
      if not LoadHelp(Help, Topic) then
        Exit;

      if IsTTYAvailable(Help) then
        OutputTTY(Help)
      else
{$IFDEF GERMAN}
        WriteLn('Kein "/?" Hilfetext verfÅgbar.');
{$ELSE}
        WriteLn('No "/?" help text available.');
{$ENDIF}

    ShowHelpTTY := True;
  end;
end;

begin
  HelpKeyPtr := @ReadKeyWord;
  HelpOnScreen := False;
  LstOpen := false;
  {$IFDEF UseMouse}
  HelpMouseEnabled := False;
  {$ENDIF}
  HelpResize := false;
  with HelpState do begin
    FoundMarkLine := 0;
    FoundLine := 0;
  end;
end.

