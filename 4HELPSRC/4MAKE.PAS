(*
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
  | Unpublished work, Copyright 1988 - 1999, JP Software Inc., All Rights |
  | Reserved.  Portions Copyright 1987, TurboPower Software.              |
  |_______________________________________________________________________|


  Modifications for 4Make 4.00, October, 1991, by Scott McGrath
    Remove BreakCnt from PageBreak
    Updated version numbers
    Added code to support TTYToggle for TTY help output and printing
    Added TTYOnly Marker

*)

{$R-,S-,I-,V-,F-,B-}

{*********************************************************}
{*                  MAKEHELP.PAS 5.07                    *}
{*        Copyright (c) TurboPower Software 1987.        *}
{* Portions copyright (c) Sunny Hill Software 1985, 1986 *}
{*     and used under license to TurboPower Software     *}
{*                 All rights reserved.                  *}
{*********************************************************}

{ This version licensed by TurboPower Software to JP Software Inc.,
  (P.O. Box 1470, E. Arlington, MA 02174, USA, 617-646-3975),
  for use and redistribution.  Not to be distributed in any other
  manner without the express written permission of JP Software
  and TurboPower Software. }

{ NOTE!  Major sections that have been customized are marked with
  "(* !!!! *)".  Major sections that have been eliminated to save
  space have been marked with (* ### ... ### *). }

program MakeHelp;
  {-Build indexed binary help file from text file}
uses
  Dos,
  TPDos,
  TPMemChk,
  TPString,
  TPCrt,
  TPWindow,
  TPPick,
  TpHelp;

const
  FileBuffSize = 4096;       {Size of input and output file buffers}
  CommandMark = '!';         {Marks help metacommand in text file}
  CommentMark = ';';         {At start of line, marks comment in text file}
  MaxIncludeNest = 1;        {Maximum depth of include nesting}
(* !!! *)
  MaxCompIndex1 = 13;        {Primary xlate table is 0..13}      {TER}
  MaxCompIndex2 = 29;        {Secondary xlate table is 14..29}   {TER}
  CR = 13;                   {Carriage return}
  LF = 10;                   {Line feed}
  CodeChar = ^H;             {character for numeric ASCII codes}
(* !!! *)


type
  FileBuff = array[1..FileBuffSize] of Byte;
  String80 = string[80];
  CountArray = array[0..255] of longint;
  StringPtr = ^string;
  SO =
    record
      O : Word;
      S : Word;
    end;
  FileArray = array[0..MaxIncludeNest] of file;
  FilePosArray = array[0..MaxIncludeNest] of longint;
  InPosArray = array[0..MaxIncludeNest] of word;
  LineArray = array[0..MaxIncludeNest] of LongInt;

var
  InName : String80;         {Input file name}
  OutName : String80;        {Output file name}
  InF : FileArray;           {Input files (with include files)}
  OutF : file;               {Output file}
  InBuff : FileBuff;         {Buffer for input text}
  OutBuff : FileBuff;        {Buffer for binary output}
  InPos : Word;              {current position in input buffer}
  OutPos : Word;             {Bytes used in output buffer}
  InBufLen : Word;           {current input buffer length}
  LastFilePos : FilePosArray;  {saved file positions for include nesting}
  LastInPos : InPosArray;    {saved input positions for include nesting}
  IncLev : Word;             {Include nesting level}
  LineNum : LineArray;       {Current input line number}
  TotLines : LongInt;        {Total number of lines}
  FirstLine : boolean;       {First line of a paragraph}
  IndentWidth : byte;        {Width of current indent}
  NormIndentWidth : byte;    {Width of normal indent}
  FirstIndentWidth : byte;   {Width of first line indent}
  RightIndentWidth : byte;   {Width of right indent}
  NewIndentWidth : boolean;  {flag for new indent width}
  SkipWidth : byte;          {Space to skip at left side}

  Hdr : HelpHeader;          {Header of help file}
  CP : CharArrayPtr;         {Points to pick array}
  FI : HelpIndexPtr;         {Points to help index}
  TM : TopicIndexPtr;        {Points to sorted topic map}
  IK : TopicIndexPtr;        {Points to index key override}

  C : String80;              {Command or command parameter}
  Clen : Byte absolute C;    {Length of parameter}
  S : string;                {Raw input line}
  Slen : Byte absolute S;    {Length of input line}
  Spos : Byte;               {Position in input line}
  SectPos : LongInt;         {File offset of current section}
  TextWid : Byte;            {Max characters in a line}
  CurSect : Word;            {Current section number}
  BiggestTopicSect : Word;   {Section of biggest topic}
  LineLen : Byte;            {Current line width}
  SectLen : Word;            {Bytes in current section}
  CompLen : Word;            {Compressed bytes in current section}
  TotalSect : LongInt;       {Total uncompressed bytes}
  TotalComp : LongInt;       {Total compressed bytes}

  Count : CountArray;        {Count for each character}
  I : Word;                  {Index used for compression}
  B : Word;                  {Index used for count table position}
  MFB : Word;                {Most frequent byte}
  TranslateTable : XlateArray; {Easy to access xlate table}

  WriteWarnings : Boolean;   {True to write wrap warnings}
  Warnings : Word;           {Number of warnings reported}
  Nibble : Boolean;          {True when a nibble is pending}
  InOrder : Boolean;         {True when a sort section is in sort order}
  InXref : Boolean;          {True while in midst of xref item}
  Wrapping : Boolean;        {True when word wrapping}
(* !!! *)
  SkipEndMark : boolean;     {True to skip end of section mark for equated
                              topic}
(* !!! *)

  procedure WriteCopyRight;
    {-Write the copyright line}
  begin
WriteLn;
WriteLn('  ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿');
WriteLn('  ³                4DOS Version 7.01 Help Compiler                 ³');
WriteLn('  ³                                                                ³');
WriteLn('  ³ Copyright 1990 - 2000, JP Software Inc., All Rights Reserved.  ³');
WriteLn('  ³                                                                ³');
WriteLn('  ³ The 4DOS help compiler is licensed to you for your use on a    ³');
WriteLn('  ³ single computer only, and may not be distributed to others.    ³');
WriteLn('  ³ Please contact JP Software if you require additional copies    ³');
WriteLn('  ³ of this program.                                               ³');
WriteLn('  ³                                                                ³');
WriteLn('  ³ JP Software Inc., P.O. Box 1470, E. Arlington, MA 02474, USA   ³');
WriteLn('  ³ Phone 781-646-3975 /  Fax 781-646-0904 / www.jpsoft.com        ³');
WriteLn('  ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ');
WriteLn;
  end;

  procedure Error(Msg : string);
    {-Write error message and halt}
  begin
    WriteLn(^M'ERROR: ', Msg);
    Halt(1);
  end;

  procedure EraseOutFile;
    {-Close and erase output file}
  var
    IO : Word;
  begin
    Close(OutF);
    Erase(OutF);
    IO := IoResult;
  end;

  function FileName(var F : file) : String;
    {-Return name of file}
  var
    NLen : Byte;
  begin
    with FileRec(F) do begin
      NLen := 0;
      while Name[NLen] <> #0 do begin
        FileName[NLen+1] := Name[NLen];
        inc(NLen);
      end;
      FileName[0] := Char(NLen);
    end;
  end;

  procedure WarnLine(Msg : string);
    {-Report error message, but continue}
  var
    KW : Word;
  begin
    if WriteWarnings then begin
      WriteLn(^M'WARNING: ', Msg);
      WriteLn('File: ', FileName(Inf[IncLev]));
      WriteLn('Line number: ', LineNum[IncLev]);
      WriteLn(S);
      Inc(Warnings);
      if Warnings and 7 = 0 then begin
        Write('Press any key to continue, <Esc> to quit');
        KW := ReadKeyWord;
        if KW = $011B then begin
          WriteLn;
          EraseOutFile;
          Halt(1);
        end else begin
          Write(^M);
          ClrEol;
        end;
      end;
    end;
  end;


  procedure ErrorLine(Msg : string);
    {-Report error position and message}
  begin
    WriteLn(^M'ERROR: ', Msg);
    WriteLn('File: ', FileName(Inf[IncLev]));
    WriteLn('Line number: ', LineNum[IncLev]);
    WriteLn(S);
    EraseOutFile;
    Halt(1);
  end;

(* !!! *)
    {FlushTextIn procedure deleted}
(* !!! *)

(* !!! *)
  procedure ReadInBuf;
    {-Read the input buffer}
  begin
    BlockRead(Inf[IncLev], InBuff, FileBuffSize, InBufLen);
    if IoResult <> 0 then
      ErrorLine('Error reading from '+InName);
  end;
(* !!! *)

  procedure OpenInf(Name : String80);
    {-Open input file}
  begin
(* !!! *)
    if IncLev > 0 then
      LastInPos[IncLev - 1] := InPos;     {save old input position}
(* !!! *)
    Assign(InF[IncLev], Name);
    Reset(InF[IncLev], 1);
    if IoResult <> 0 then
      Error(Name+' not found');
    LineNum[IncLev] := 0;
    InPos := 1;                           {force read of first buffer}
    InBufLen := 0;
(* !!! *)
{    Write(^M, CharStr(' ', 64), ^M, Pad(StUpcase(Name), 13), 0:5);}
(* !!! *)
  end;

  procedure CloseInf;
    {-Close input file}
  begin
    WriteLn(^H^H^H^H^H, LineNum[IncLev]:5);
    Close(InF[IncLev]);
    inc(TotLines, LineNum[IncLev]);
    if IncLev > 0 then begin
      dec(IncLev);
(* !!! *)
      Seek(InF[IncLev], LastFilePos[IncLev]);
      if IoResult <> 0 then
        ErrorLine('Error seeking in '+InName);
      ReadInBuf;
      InPos := LastInPos[IncLev];
(* !!! *)
      Write('  ', Pad(StUpcase(FileName(InF[IncLev])), 13), LineNum[IncLev]:5);
    end;
  end;

  procedure Initialize;
    {-Prepare for analysis of help file}
  var
    I : Word;
    Arg : string[127];
  begin
    WriteWarnings := True;
    InName := '';
    OutName := '';

    InName := ParamStr(1);
    if Length(InName) = 0 then
      InName := '4DOS.TXT'
    else
      InName := DefaultExtension(InName,'TXT');

    OutName := ParamStr(2);
    if Length(OutName) = 0 then
      OutName := ForceExtension(InName, 'HLP');

    if InName = OutName then
      Error('Input and output filenames must differ');

    Assign(OutF, OutName);
    Rewrite(OutF, 1);
    if IoResult <> 0 then
      Error('Cannot create '+OutName);

(* !!! *)
    Write('  Pass 1............');
(* !!! *)
    IncLev := 0;
    OpenInf(InName);

    {Default help header}
    FillChar(Hdr, SizeOf(HelpHeader), 0);
    with Hdr do begin
      ID := HelpSystemID;
      SharewareData.Checksum := 0;    {checksum for all 0s}
      Width := 80;
    end;

    {No warnings yet}
    Warnings := 0;
    TotLines := 0;

    {Initialize character frequency count array}
    FillChar(Count, SizeOf(Count), 0);
  end;

(* !!! *)
  procedure CheckForCodes;

  var
    Index  : byte;
    NewIndex  : byte;
    Ch     : char;
    Number : byte;
    Result : integer;
    Source : string;
    Line   : string;
    Done   : boolean;

  begin
    Source := S;
    Index := Pos(CodeChar, Source);
    while Index > 0 do begin
      Line := '';
      Delete(Source,Index,1);
      Done := False;
      repeat
        if Index <= Length(Source) then begin
            case Source[Index] of
              '0'..'9' : begin
                          Line := Line + Source[Index];
                          Delete(Source,Index,1);
                        end;
              CodeChar : Done := True;
              else
                ErrorLine('Invalid character code');
            end;  { case }
        end else
          ErrorLine('Character code not properly terminated');
      until Done;
      Val(Line, Number, Result);
      Source[Index] := Chr(Number);       {overwrite terminating CodeChr}
      NewIndex := Pos(CodeChar,
                      Copy(Source, Index + 1, Length(Source) - Index));
      if NewIndex = 0 then
        Index := 0
      else
        NewIndex := NewIndex + Index;
    end;
    S := Source;
  end;
(* !!! *)

(* !!! *)
  function ReadTextChar : byte;
    {-Read next character from help text}
  begin
    if InPos > InBufLen then begin
      if EoF(Inf[IncLev]) then begin
        ReadTextChar := 0;
        Exit;
      end;
      LastFilePos[IncLev] := FilePos(Inf[IncLev]);
      ReadInBuf;
      InPos := 1;
    end;
    ReadTextChar := Inbuff[InPos];
    Inc(InPos);
  end;

  function ReadTextLine : boolean;
    {-Read next line from help text, return true if not EOF}
  var
    Ch : byte;
    Spos : byte;
  begin
    Inc(LineNum[IncLev]);
    SPos := 1;
    Slen := 0;
    repeat
      Ch := ReadTextChar;
      if Ch = CR then begin
        Ch := ReadTextChar;               {check for line feed}
        if Ch <> LF then begin
          S[Spos] := Chr(CR);             {no LF, output CR}
          Inc(Spos);
        end;
      end;
      if (Ch <> LF) and (Ch <> 0) then begin
        S[Spos] := Chr(Ch);
        Inc(Spos);
      end;
    until (Ch = LF) or (Ch = 0);
    Slen := Spos - 1;
(* !!! *)
    CheckForCodes;
(* !!! *)
    if Slen = 0 then
      S[1] := #0;
    if LineNum[IncLev] and $0F = 0 then
      Write(^H^H^H^H^H, LineNum[IncLev]:5);
    ReadTextLine := (Ch <> 0);
  end;
(* !!! *)

  procedure SkipWhite;
    {-Advance Spos past white space}
  begin
    while (Spos <= Slen) and ((S[Spos] = ' ') or (S[Spos] = Chr(9))) do
      Inc(Spos);
  end;

  procedure ParseWord(var C : string; MaxLen : Byte);
    {-Parse next word from S, returning it in C}
  var
    Clen : Byte absolute C;
  begin
    SkipWhite;
    Clen := 0;
    while (Spos <= Slen) and (S[Spos] > ' ') and (S[Spos] <> CommentMark) do begin
      if Clen < MaxLen then begin
        Inc(Clen);
        C[Clen] := S[Spos];
      end;
      Inc(Spos);
    end;
  end;

  function ParseNumber(Name : string) : Word;
    {-Parse a number from the line}
  var
    C : string[8];
    N : Word;
  begin
    ParseWord(C, 8);
    if Length(C) = 0 then
      ErrorLine(Name+' expected');
    if not Str2Word(C, N) then
      ErrorLine('Invalid '+Name+' specified');
    ParseNumber := N;
  end;


  procedure ParseIndents;
  begin
    SkipWhite;
    NormIndentWidth := Byte(ParseNumber('Indent'));
    SkipWhite;
    SkipWidth := 0;
    if Spos <= Slen then
      SkipWidth := Byte(ParseNumber('Indent skip'));
    SkipWhite;
    FirstIndentWidth := 0;
    if Spos <= Slen then
      FirstIndentWidth := Byte(ParseNumber('First line indent'));
    SkipWhite;
    RightIndentWidth := 0;
    if Spos <= Slen then
      RightIndentWidth := Byte(ParseNumber('Right indent'));
  end;


  function ClassifyCommand(C : string) : Word;
    {-Classify valid help metacommands}
  const
    NumCommands = 17;
    CommandNames : array[1..NumCommands] of string[5] =
    ('TOPIC', 'LINE', 'PAGE', 'WIDTH', 'INDEX', 'NOIND',
     'INCLU', 'WRAP', 'NOWRA', 'TTY', 'ENDTT', 'TTYON',
     'INDEN', 'SKIP', 'KEYS', 'FIRST', 'EXTER');
  var
    I : Integer;
  begin
    C := StUpcase(Copy(C, 1, 5));
    for I := 1 to NumCommands do
      if C = CommandNames[I] then begin
        ClassifyCommand := I;
        Exit;
      end;
    ClassifyCommand := 0;
  end;


  procedure BlockWriteChk(var B; Bytes : Word);
    {-Write a block to output and error check}
  var
    BytesWritten : Word;
  begin
    BlockWrite(OutF, B, Bytes, BytesWritten);
    if (IoResult <> 0) or (BytesWritten <> Bytes) then
      Error('Error writing to '+OutName);
  end;

  procedure EncryptTable;
    {-Encrypt (mildly) the translation table}
  begin
    with Hdr do begin
      for I := 0 to 29 do
        XlateTable[I] := XlateTable[I] xor $C3;
    end;
  end;

  procedure WriteHeaders;
    {-Write the binary header structures to the help file}
  begin
    with Hdr do begin
      BlockWriteChk(Hdr, SizeOf(HelpHeader));
      BlockWriteChk(CP^, HighestTopic*NameSize);
      BlockWriteChk(FI^, HighestTopic*SizeOf(HelpIndexRec));
      BlockWriteChk(TM^, HighestTopic*SizeOf(Word));
    end;
  end;

  procedure FindMostFrequent;
    {-Find the most frequently occurring characters}
  begin
    with Hdr do
      for I := 0 to 29 do begin
        MFB := 0;
        {Find most frequently occurring byte in Count table}
        for B := 0 to 255 do
          if Count[B] > Count[MFB] then
            MFB := B;
        {Store it in XlateTable}
        XlateTable[I] := MFB;
        {Mark it out so we can find next MFB}
        Count[MFB] := 0;
      end;
  end;

  procedure AllocWorkSpace;
    {-Allocate space for work arrays and initialize them}
  begin
    with Hdr do begin
      {Allocate space for names}
      if NameSize = 0 then
        CP := HeapPtr
      else if LongInt(HighestTopic)*NameSize > 65520 then
        Error('Topic name array exceeds 64K')
      else if not GetMemCheck(CP, HighestTopic*NameSize) then
        Error('Insufficient memory for name array');

      {Allocate space for file index}
      if LongInt(HighestTopic)*SizeOf(HelpIndexRec) > 65520 then
        Error('File index array exceeds 64K')
      else if not GetMemCheck(FI, HighestTopic*SizeOf(HelpIndexRec)) then
        Error('Insufficient memory for index array');

      {Allocate space for topic map}
      if not GetMemCheck(TM, HighestTopic*SizeOf(Word)) then
        Error('Insufficient memory for topic map');

      {Allocate space for index key}
      if not GetMemCheck(IK, HighestTopic*SizeOf(Word)) then
        Error('Insufficient memory for index key');

      {Initialize the arrays}
      FillChar(CP^, HighestTopic*NameSize, 0);
      FillChar(FI^, HighestTopic*SizeOf(HelpIndexRec), lo(NoHelpAvailable));
      FillChar(TM^, HighestTopic*SizeOf(Word), 0);
      FillChar(IK^, HighestTopic*SizeOf(Word), $77);
    end;
  end;

  procedure CountFile;
    {-Scan input file once to determine counts, sizes, frequencies}
  var
    Cnt : Word;
    Ch, PrevCh   : Char;
    EquateSect   : word;
    PrevIndex    : word;
    PrevNameSize : word;
    SkipError    : boolean;
    IncName : String80;
  begin
    PrevIndex := 0;
    PrevNameSize := 0;
    NormIndentWidth := 0;
    FirstIndentWidth := 0;
    RightIndentWidth := 0;
    SkipWidth := 0;
    with Hdr do begin
      while ReadTextLine do begin
        case S[1] of
          CommandMark :      {Line is a help metacommand}
            begin
              Spos := 2;
              ParseWord(C, 8);
              case ClassifyCommand(C) of
                1 :          {TOPIC}
                  begin
(* !!! *)
                    {Check number of topics}
                    Inc(NumTopics);
                    if (NumTopics > MaxTopics) then
                      ErrorLine('Too many topics in help file!');
                      
                    {Increase pick size if previous topic is indexed}
                    if (PrevIndex <> $FFFF) and (PrevNameSize > PickSize) then
                      PickSize := PrevNameSize;
                    PrevIndex := 0;
                    PrevNameSize := 0;
(* !!! *)
                    {New section, get section number}
                    CurSect := ParseNumber('Topic number');
                    if CurSect > HighestTopic then
                      HighestTopic := CurSect;
(* !!! *)
                    SkipWhite;
                    if ((Spos + 1) < Slen) and (S[Spos] = '=') then begin
                        {checking Spos+1 allows a topic with the name "="}
                      Inc(Spos);          {topic equate -- skip equal sign}
                      EquateSect := ParseNumber('Equated topic number');
                      SkipWhite;
                    end;
(* !!! *)
                    {Get optional pick name}
                    C := Copy(S, Spos, 80);
                    PrevNameSize := Length(C)+1;
                    if PrevNameSize > NameSize then
                      NameSize := PrevNameSize;
                  end;
(* !!! *)

                5 :          {INDEX}
                  begin
                    SkipWhite;
                    PrevIndex := ParseNumber('Index number');
                  end;


                6 :          {NOINDEX}
                  if CurSect = 0 then
                    ErrorLine('NOINDEX statement must follow TOPIC statement')
                  else
                    PrevIndex := $FFFF;
(* !!! *)

                7 :          {INCLUDE}
                  if IncLev = MaxIncludeNest then
                    Error('Too many nested files')
                  else begin
                    {Include file, get filename}
                    ParseWord(IncName, 79);
                    inc(IncLev);
                    OpenInf(IncName);
                    CountFile;
                  end;

                13 :          {INDENT}
                  begin
                    ParseIndents;
                      {count overall indent mark}
                    Inc(Count[Byte(IndentMark)]);
                    Inc(Count[NormIndentWidth]);
                      {count first line indent mark}
                    if FirstIndentWidth > 0 then begin
                      Inc(Count[Byte(IndentMark)]);
                      Inc(Count[128 + FirstIndentWidth]);
                    end;
                  end;

                {Ignore other metacommands this pass}
              end;
            end;

          CommentMark :
            {Ignore comment lines}
            ;

        else

          {Part of help text, keep count for compression}
          Spos := 1;
          PrevCh := Chr(0);

          SkipError := false;
          while (Spos <= Slen) and (Spos <= SkipWidth) do begin
            if S[Spos] <> ' ' then
              SkipError := true;
            Inc(Spos);                    {skip the skip area}
          end;
          if SkipError then
            WarnLine('Text in skip area');

          SkipWhite;
          if Spos > 3 then begin        {use indents for leading whitsespace}
            Inc(Count[Byte(IndentMark)]);
            Inc(Count[128 + Spos - 1]);
          end else
            Spos := 1;                    {reset, don't use indent}

          while Spos <= Slen do begin
            Ch := S[Spos];
            Inc(Count[Byte(Ch)]);
            if (Ch = IndexMarker) and (PrevCh <> EscapeChar) then
              {Skip over the cross-reference topic number}
              repeat
                Inc(Spos);
              until (Spos > Slen) or (S[Spos] = XrefToggle)
            else
              Inc(Spos);
            PrevCh := Ch;
          end;
          {Approximate the number of linebreak markers}
          Inc(Count[Byte(LineBrkMark)]);
        end;
      end;

      CloseInf;
      {Increase pick size if necessary for last topic}
      if (PrevIndex <> $FFFF) and (PrevNameSize > PickSize) then
        PickSize := PrevNameSize;
    end;
  end;

  function TranslateIndex(Ch : Char) : Byte;
    {-If Ch is in translate table, return index, else return $0F}
  inline
  ($8C/$D8/                  {mov ax,ds}
    $8E/$C0/                 {mov es,ax}
    $BF/>TranslateTable/     {mov di,>TranslateTable  ;es:di -> TranslateTable}
    $89/$FE/                 {mov si,di               ;save start offset}
    $58/                     {pop ax                  ;al = Ch}
(* !!! *)
    $B9/>MaxCompIndex2+1/    {mov cx,>MaxCompIndex2+1  ;cx = bytes to scan}
(* !!! *)
    $FC/                     {cld                     ;forward}
    $F2/                     {repne                   ;search}
    $AE/                     {scasb                   ;  until match}
    $75/$01/                 {jne fail                ;jump if not found}
    $4F/                     {dec di                  ;point to match if found}
    {fail:}
    $29/$F7/                 {sub di,si               ;compute index of match}
    $89/$F8);                {mov ax,di               ;return index in al}

  procedure FlushBuffer;
    {-Write the output buffer to file}
  begin
    if OutPos > 0 then begin
      BlockWriteChk(OutBuff, OutPos);
      OutPos := 0;
      Nibble := False;
    end;
  end;

  procedure NibbleOut(N : Byte);
    {-Send next nibble to output}
  begin
    if Nibble then begin
      OutBuff[OutPos] := OutBuff[OutPos] or (N shl 4);
      Nibble := False;
      if OutPos = FileBuffSize then
        FlushBuffer;
    end else begin
      Inc(OutPos);
      Inc(CompLen);
      OutBuff[OutPos] := N;
      Nibble := True;
    end;
  end;

(* !!! *)
  procedure CharOut(Ch : Char);
    {-Compress a single character and write it}
  var
    T : Byte;
  begin
    T := TranslateIndex(Ch);
    if T > MaxCompIndex2 then begin
        {Not compressible - output 3 nibbles}
      NibbleOut($0F);
      NibbleOut(Ord(Ch) and $0F);
      NibbleOut(Ord(Ch) shr 4);
    end else if T > MaxCompIndex1 then begin
        {Output 2-nibble compression code}
      NibbleOut($0E);
      NibbleOut(T - MaxCompIndex1 - 1);
    end else
        {Store compression code in 1 nibble}
      NibbleOut(T);
    Inc(SectLen);
  end;
(* !!! *)

  procedure NewSection;
    {-End the current section and prepare for the new}
  begin
(* !!! *)
    if not SkipEndMark then begin
      CharOut(SectEndMark);
      FI^[CurSect].CompLen := CompLen;
    end;
    SkipEndMark := false;
(* !!! *)
    with Hdr do
      if SectLen > BiggestTopic then begin
        BiggestTopic := SectLen;
        BiggestTopicSect := CurSect;
      end;

    Inc(TotalComp, CompLen);
    Inc(TotalSect, SectLen);

    Inc(SectPos, CompLen);
    SectLen := 0;
    CompLen := 0;
    LineLen := 0;
    Nibble := False;
    if OutPos = FileBuffSize then
      FlushBuffer;
  end;

(* !!! *) {leave out of 4.00}
  procedure NewPage(BreakCnt : word);
    {-End the current page}
  begin
    CharOut(PageBrkMark);
    {CharOut(Chr(BreakCnt));}
    LineLen := 0;
  end;
(* !!! *)

  procedure NewLine;
    {-End the current line}
  begin
(*    if InXref then
      {Line break in xref}
      WarnLine('Cross-reference straddles line break');*)
    CharOut(LineBrkMark);
    LineLen := 0;
  end;

  function LenCount(Ch : Char) : Byte;
    {-Return length to count for character}
  begin
    case Ch of
(* !!!! *)
      Attr1Toggle..XrefToggle, EscapeChar :   {don't count TTYToggle here?}
(* !!!! *)
        LenCount := 0;
    else
      LenCount := 1;
    end;
  end;

  procedure WordOut(var Spos : Byte; Tpos : Byte);
    {-Write line starting at Spos and continuing to Tpos}
  var
    Topic : Word;
    Code : Word;
    Ch : Char;
(* !!!! *)
    PrevEscape : Boolean;
(* !!!! *)
  begin
    PrevEscape := false;
    while Spos < Tpos do begin
      Ch := S[Spos];
      CharOut(Ch);
(* !!!! *)
      if PrevEscape then begin
        Inc(Spos);
        Inc(LineLen);
        PrevEscape := false;
      end else begin
        if (Ch = IndexMarker) and (not PrevEscape) then begin
          {Convert cross-reference topic number to binary}
          Clen := 0;
          repeat
            Inc(Spos);
            Inc(Clen);
            C[Clen] := S[Spos];
          until (Spos >= Tpos) or (S[Spos] = XrefToggle);
          Dec(Clen);
          Val(C, Topic, Code);
          if Code <> 0 then
            ErrorLine('Invalid cross-reference topic number');
          CharOut(Char(lo(Topic)));
          CharOut(Char(Hi(Topic)));
        end else begin
          if Ch = XrefToggle then
            InXref := not InXref;
          if Ch = EscapeChar then
            PrevEscape := true;
          Inc(LineLen, LenCount(Ch));
          Inc(Spos);
        end;
      end;
(* !!!! *)
    end;
  end;

  procedure LineOut;
    {-Wrap and write text lines}
  var
    Tpos : Byte;
    Tlen : Byte;
    LineWid : Byte;            {Max characters in a line with indent}
    PrevEscape : boolean;
    LineIndent : boolean;
    SaveSpos : byte;

  begin

    LineIndent := false;

    if not Wrapping then begin
      {Write entire line without wrapping}
      SPos := 1;
      WordOut(SPos, Slen+1);
      NewLine;
      Exit;
    end;

    {handle empty line}
    if Slen = 0 then begin
      {finish previous line}
      if LineLen > 0 then
        NewLine;
      {Insert blank line}
      NewLine;
      FirstLine := true;
      Exit;
    end;

    {change indent width if necessary}
    if NewIndentWidth then begin
      {finish previous line}
      if LineLen > 0 then
        NewLine;
      CharOut(IndentMark);
      CharOut(Char(NormIndentWidth));
      IndentWidth := NormIndentWidth;
      NewIndentWidth := False;
    end;

    {handle line indent if first line of paragraph}
    if FirstLine and (FirstIndentWidth <> NormIndentWidth) then begin
      CharOut(IndentMark);
      CharOut(Char(128 + FirstIndentWidth));
      IndentWidth := FirstIndentWidth;
      FirstLine := false;
      LineIndent := true;
    end;

    LineWid := TextWid - IndentWidth - RightIndentWidth;
    IndentWidth := NormIndentWidth;  {use normal indent for subsequent lines}

    Spos := 1;

    {skip the skip area}
    while (Spos <= Slen) and (Spos <= SkipWidth) do
      Inc(Spos);

    {Non-empty line}
    if (S[Spos] = ' ') then
      {Finish previous line}
      if LineLen > 0 then
        NewLine;

    if not LineIndent then begin
      {use indents for whitespace, unless we already have a line indent}
      SaveSpos := Spos;
      SkipWhite;
      if (Spos > (SaveSpos + 2)) then begin
        CharOut(IndentMark);
        CharOut(Char(128 + NormIndentWidth + Spos - SaveSpos));
        Inc(LineLen, (Spos - SaveSpos));
      end else
        Spos := SaveSpos;                 {< 3 spaces, don't use indent}
    end;

    repeat

      {Write white space}
      while (Spos <= Slen) and (S[Spos] = ' ') do begin
        if LineLen < LineWid then begin
          CharOut(' ');
          Inc(LineLen);
        end;
        Inc(Spos);
      end;

      if Spos > Slen then
        Exit;

      {See if next word fits on line}
      Tpos := Spos;
      Tlen := 0;

      PrevEscape := false;
      repeat
        if S[Tpos] = EscapeChar then begin
          PrevEscape := true;
          Inc(Tpos);
        end else begin
          if (S[Tpos] = IndexMarker) and (not PrevEscape) then
            {Skip over the cross-reference topic number}
            repeat
              Inc(Tpos);
            until (Tpos > Slen) or (S[Tpos] = XrefToggle)
          else begin
            Inc(Tlen, LenCount(S[Tpos]));
            Inc(Tpos);
          end;
          PrevEscape := false;
        end;

      until (Tpos > Slen) or (S[Tpos] = ' ');

      if LineLen+Tlen > LineWid then
        {Word won't fit on line, start a new one}
        NewLine;

      {Write the word}
      WordOut(Spos, TPos);

    until Spos > Slen;

    {End line with blank}
    if LineLen < LineWid then begin
      CharOut(' ');
      Inc(LineLen);
    end;
  end;

  function TopicPtr(Item : Word) : StringPtr;
    {-Return pointer to topic name}
  begin
    with Hdr do
      TopicPtr := Ptr(SO(CP).S, SO(CP).O+NameSize*(Item-1));
  end;

  procedure StorePickName(C : string);
    {-Store pick name for CurSect}
  begin
    Move(C, TopicPtr(CurSect)^, Length(C)+1);
  end;

  procedure Swap(I, J : Word);
    {-Swap topic map for I and J}
  var
    Tmp : Word;
  begin
    Tmp := TM^[J];
    TM^[J] := TM^[I];
    TM^[I] := Tmp;
    InOrder := False;
  end;

  procedure SortTopicMap;
    {-Sort the topic map into alphabetical order}
  var
    I : Word;
    J : Word;
    K : Word;
    Offset : Word;
    IM : Word;
    JM : Word;
    IPtr : StringPtr;
    JPtr : StringPtr;
  begin
    with Hdr do begin
      {Initialize topic map}
      for I := 1 to HighestTopic do
        TM^[I] := I;

      {Sort topic names via topic map}
      Offset := HighestTopic;
      while Offset > 1 do begin
        Offset := Offset shr 1;
        K := HighestTopic-Offset;
        repeat
          InOrder := True;
          for J := 1 to K do begin
            I := J+Offset;
            IM := TM^[I];
            JM := TM^[J];
            IPtr := TopicPtr(IM);
            JPtr := TopicPtr(JM);
            if Length(IPtr^) = 0 then
              {"I" is greater - sort empty names to end of list}
            else if Length(JPtr^) = 0 then
              {"J" is greater - sort empty names to end of list}
              Swap(I, J)
            else if IK^[IM] > IK^[JM] then
              {"I" is greater - no swap needed}
            else if IK^[IM] < IK^[JM] then
              {"J" is greater by index override}
              Swap(I, J)
            else if CompUCString(IPtr^, JPtr^) = Less then
              {"J" is greater alphabetically}
              Swap(I, J)
          end;
        until InOrder;
      end;

      {Find last selected, non-blank sorted topic name}
      for I := 1 to HighestTopic do begin
        IM := TM^[I];
        if (IK^[IM] = $FFFF) or (Length(TopicPtr(IM)^) = 0) then begin
          NamedTopics := I-1;
          Exit;
        end;
      end;
      NamedTopics := HighestTopic;
    end;
  end;

  function ValidTopic(Topic : word) : boolean;
    {-See if there is help available for a topic}
  begin
    ValidTopic := (Topic <= Hdr.HighestTopic) and
      (FI^[Topic].Start <> NoHelpAvailable)
  end;

  procedure ScanForEquates;
    {-Scan for equated topics and set up the pointers correctly}
  var
    I, EquateTopic : word;
  begin
    with Hdr do
      for I := 1 to Hdr.HighestTopic do
        if FI^[I].Start = HelpEquated then begin
          EquateTopic := FI^[I].CompLen;
          if not ValidTopic(EquateTopic) then begin
            WriteLn('Error equating topic ', I, ' to topic ', EquateTopic);
            Error('Invalid equate, target topic not defined');
          end;
          FI^[I].Start := FI^[EquateTopic].Start;
          FI^[I].CompLen := FI^[EquateTopic].CompLen;
        end;
  end;

  procedure CheckTopicLinks;
    {-Check global topic links to make sure the topics exist}
  begin
    with Hdr do begin
	   if (FirstTopic <> 0) and not ValidTopic(FirstTopic) then
        Error('Invalid first topic number');
	   if (KeysTopic <> 0) and not ValidTopic(KeysTopic) then
        Error('Invalid keys topic number');
    end;
  end;

  procedure InitForScan;
    {-Initialize for the scan file pass}
  begin
    with Hdr do begin
      {Correct default text dimensions for frames and spacing}
      TextWid := Width-4;

      {Store position for first help section}
      SectPos := (SizeOf(HelpHeader)+
        LongInt(HighestTopic)*(NameSize+SizeOf(HelpIndexRec)+SizeOf(Word)));
       {^^^^^^^^            ^}
       {!!.07}

      {Initialize counters}
      TotalSect := 0;
      TotalComp := 0;
      CurSect := 0;
      BiggestTopicSect := 0;
      LineLen := 0;
      SectLen := 0;
      CompLen := 0;
      OutPos := 0;
      Nibble := False;
      InXref := False;
      Wrapping := True;
      TranslateTable := XlateTable;

      Write('  Pass 2............');
      SkipEndMark := false;
      TotLines := 0;
      IncLev := 0;
      OpenInf(InName);
    end;
  end;


  procedure ScanFile;
    {-Scan input file to create help text}
  var
    IncName : String80;
    PageBrkCnt   : word;                  {value from !PAGE}
    EquateSect   : word;                  {section number for topic equate}

  begin

    NormIndentWidth := 0;
    FirstIndentWidth := 0;
    RightIndentWidth := 0;
    NewIndentWidth := false;
    SkipWidth := 0;
    FirstLine := true;

    with Hdr do begin
      LineNum[IncLev] := 0;
      while ReadTextLine do begin
        case S[1] of
          CommandMark :      {A help metacommand}
            begin
              Spos := 2;
              ParseWord(C, 8);
              case ClassifyCommand(C) of
                1 :          {TOPIC}
                  begin
                    if CurSect <> 0 then
                      {Complete previous section}
                      NewSection;
                    {Get section number}
                    CurSect := ParseNumber('Topic number');
                    {Error check}
                    if FI^[CurSect].Start <> NoHelpAvailable then
                      ErrorLine('Duplicate help topic number');
                    {Store file index}
(* !!! *)
                    SkipWhite;
                    if (Spos < Slen) and (S[Spos] = '=') then begin
                      Inc(Spos);          {topic equate -- skip equal sign}
                      EquateSect := ParseNumber('Equated topic number');
                      FI^[CurSect].Start := HelpEquated;
                      FI^[CurSect].CompLen := EquateSect;
                      SkipEndMark := true;  {no need for another end mark}
                      SkipWhite;
                    end else
                      FI^[CurSect].Start := SectPos;
(* !!! *)
                    {Get optional pick name}
                    C := Copy(S, Spos, 80);
                    if Length(C) > 0 then
                      {Store pick name}
                      StorePickName(C);
                  end;

                2 :          {LINE}
                  NewLine;

                3 :          {PAGE}
(* !!! *)
                  begin
                    ParseWord(C, 8);
                    if Length(C) = 0 then
                      PageBrkCnt := 0
                    else if not Str2Word(C, PageBrkCnt) then
                      ErrorLine('Invalid page break count specified');
                    NewPage(PageBrkCnt);
                  end;
(* !!! *)

                4 :          {WIDTH}
                  if CurSect <> 0 then
                    ErrorLine('WIDTH statement must precede first help topic')
                  else begin
                    {Parse width}
                    Width := ParseNumber('Width');
                    {Correct dimension for frame and spacing}
                    TextWid := Width-4;
                  end;

                5 :          {INDEX}
                  if CurSect = 0 then
                    ErrorLine('INDEX statement must follow TOPIC statement')
                  else
                    IK^[CurSect] := ParseNumber('Index number');

                6 :          {NOINDEX}
                  IK^[CurSect] := $FFFF;

                7 :          {INCLUDE}
                  if IncLev = MaxIncludeNest then
                    Error('Too many nested files')
                  else begin
                    {Include file, get filename}
                    ParseWord(IncName, 79);
                    inc(IncLev);
                    OpenInf(IncName);
                    ScanFile;
                  end;

                 8 :         {WRAP}
                   Wrapping := True;

                 9 :         {NOWRAP}
                   Wrapping := False;
{ *** 4.00 *** }
                10,
                11 :        {TTY, ENDTTY}
                begin
                  if LineLen > 0 then
                    NewLine;
                  CharOut(TTYToggle);
                  LineLen := 0;
                end;

                12 :        {TTYOnly Mark}
                begin
                  CharOut(TTYOnlyMark);
                  LineLen := 0;
                end;
{ *** 4.00 *** }

                13 :          {INDENT}
                  begin
                    ParseIndents;
                    NewIndentWidth := True;
                  end;

                15 :          {KEYS}
                  if CurSect <> 0 then
                    ErrorLine('KEYS statement must precede first help topic')
                  else
                    {Parse keys topic number}
                    KeysTopic := ParseNumber('Keys topic number');

                16 :          {FIRST}
                  if CurSect <> 0 then
                    ErrorLine('FIRST statement must precede first help topic')
                  else
                    {Parse first topic number}
                    FirstTopic := ParseNumber('First topic number');

                17 :          {EXTERNAL}
                  if CurSect <> 0 then
                    ErrorLine('EXTERNAL statement must precede first help topic')
                  else begin
                    {Parse external help names}
                    ParseWord(ExtHelpName, SizeOf(ExtHelpName) - 1);
                    ParseWord(ExtHelpEnv, SizeOf(ExtHelpEnv) - 1);
                  end;

              else
                ErrorLine('Unrecognized directive');
              end;
            end;
          CommentMark :
            {Ignore line} ;
        else
          {A text line - wrap and output}
          LineOut;
        end;
      end;

      CloseInf;
    end;
  end;

  procedure ScanDone;
    {-Clean up when scan pass is done}
  begin
    with Hdr do begin
      {Finalize status}
      if Warnings <> 0 then begin
        Write(^M);
        ClrEol;
        WriteLn;
      end;
(* !!! *)
{      WriteLn('  ', TotLines:6, ' total lines in help file');}
(* !!! *)

      {Store last section}
      if SectLen > 0 then
        NewSection;
      {Assure output goes to disk}
      FlushBuffer;

      {Sort the topic map}
(* !!! *)
      SortTopicMap;
      ScanForEquates;
		CheckTopicLinks;
(* !!! *)

      {Leave extra room for biggest section's decompression}
      Inc(BiggestTopic, 128);

      {Write the updated header and indexes}
      Reset(OutF, 1);
      EncryptTable;
      WriteHeaders;
      Close(OutF);
(* !!! *)
      WriteLn('  Compilation finished');
      WriteLn;
(* !!! *)
    end;
  end;


  procedure ShowStats;
    {-Show some interesting statistics}
  begin
    with Hdr do begin
      WriteLn(NumTopics:6, ' topics in help file');
      WriteLn(HighestTopic:6, ' highest topic number used');
      WriteLn(NamedTopics:6, ' indexed topics');
      WriteLn(BiggestTopic:6, ' bytes in largest topic (topic ', BiggestTopicSect,')');
{      WriteLn((HighestTopic*SizeOf(Word)):6, ' bytes in sorted topic map');
      WriteLn((HighestTopic*SizeOf(HelpIndexRec)):6, ' bytes in help index');
      WriteLn((HighestTopic*NameSize):6, ' bytes in topic name table');
      WriteLn(TotalSect:6, ' uncompressed help bytes');
      WriteLn(TotalComp:6, ' compressed help bytes');
      WriteLn((TotalComp/TotalSect):6:2, ' compression factor');}
    end;
  end;


begin
  WriteCopyRight;

  Initialize;

  {Perform counting pass}
  CountFile;
  if Hdr.HighestTopic = 0 then
    Error('No help topics specified');

  {Allocate heap space for help indexes}
  AllocWorkSpace;

  {Build the xlate table for compression}
  FindMostFrequent;

  {Reserve disk space for help indexes}
  WriteHeaders;

  {Prepare to reread the input file}
  InitForScan;

  {Perform the word wrap pass}
  ScanFile;

  {Clean up when scan done}
  ScanDone;

  {Show a summary}
  ShowStats;
end.

