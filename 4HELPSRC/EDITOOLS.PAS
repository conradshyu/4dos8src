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
  | Unpublished work, Copyright 1988, 1989, 1990, 1991, J.P. Software     |
  | Inc., All Rights Reserved.  Portions Copyright 1985, 1987, Borland    |
  | International Inc.                                                    |
  |_______________________________________________________________________|
*) 

{                          EDITOOLS.INC
                        Editor Toolbox 4.0
             Copyright (c) 1985, 87 by Borland International, Inc.            }

{$I-}
{$S-}
{$R-}

unit EDItools;

interface

uses
  tpCrt,                      {standard screen routines}
  Dos,                        {dos calls - standard unit}
  SInst;                      {fast screen routines}

const
  KeyLength = 6;              {maximum bytes in keystroke sequence}
  Escape = #27;

type
  VarString = string[DefNoCols];
  KeyString = string[KeyLength];

  KeyRec =
  record
    Modified,
    Conflict : Boolean;
    Keys : KeyString;
    MinCol, MaxCol : Byte;
  end;

var
  BlankLine : VarString;

  function FileExists(Name : VarString; var F : file) : Boolean;
    {-return true and an open file if file exists}

  Procedure FindFile(Var FileName : String);
    { -Search for the program file }

  function GetCursorCommand : Char;
    {-return cursor equivalent keys}

(*
  function Getkey(Prompt, Choices : VarString) : Char;
    {-return a legal menu choice}

  function YesNo(Prompt : VarString; Default : Char) : Boolean;
    {-return True for yes, False for no}

  procedure ClrEol(Col, Row, Attr : Integer);
    {-clear to end of line}
*)

  procedure HaltError(msg : varstring);
    {-display an error message and halt}

  function FindString(IdString : String; var Deft; Size : Word) : LongInt;
    {-return the location of IdString in ProgramFile and read Size bytes into
     Deft.}

  function ModifyDefaults(FileOfst : LongInt; var B; Size : Word) : Boolean;
    {-Write modified default settings back to disk, returning a success flag}

  procedure Initialize(Name, Title : String);
    {-Set up for installation}

  procedure CleanUp;
    {-Clean up at end of program}

  {==========================================================================}

implementation

const
  SBSize = 65518;
type
  SearchBuffer = array[0..SBSize] of Char; {Used to search for ID strings}

var
  ProgramFile : file;
  ProgramName : string;
  BufPtr : ^SearchBuffer;
  BytesRead : Word;
  FilePtr : LongInt;
  BufferFull : Boolean;


  function FileExists(Name : VarString; var F : file) : Boolean;
    {-return true and an open file if file exists}
  begin
    Assign(F, Name);
    Reset(F, 1);
    FileExists := (IOResult = 0);
  end;                        {fileexists}

  Procedure FindFile(Var FileName : String);

var
  SearchPath : String;
  Dir        : DirStr;
  Name       : NameStr;
  Ext        : ExtStr;

  begin
  SearchPath := ParamStr(0);
  FSplit(SearchPath,Dir,Name,Ext);
  SearchPath := Dir + ';' + GetEnv('PATH');
  FileName := FSearch(FileName,SearchPath);
  If FileName <> '' Then FileName := FExpand(FileName);
  end;

  function GetCursorCommand : Char;
    {-return cursor equivalent keys. Also allows Esc, 'C', and 'R'.}
  const
    CursorSet : set of Char =
   [^H, ^R, ^C, ^E, ^X, ^W, ^Z, ^A, ^S, ^D, ^F, ^M, 'C', 'R', ^T, ^B, ^U, #27, #254];
  var
    Ch : Char;
  begin
    repeat
      Ch := ReadKey;
      if (Ch = #0) then begin
        Ch := ReadKey;
        case Ch of
          #75 : Ch := ^S;
          #77 : Ch := ^D;
          #72 : Ch := ^E;
          #80 : Ch := ^X;
          #71 : Ch := ^T;
          #73 : Ch := ^R;
          #81 : Ch := ^C;
          #79 : Ch := ^B;
          #82 : Ch := ^U;
          #45 : Ch := #254;               {Alt-X}
        end;
      end
      else
        Ch := UpCase(Ch);
    until (Ch in CursorSet);
    GetCursorCommand := Ch;
  end;                        {GetCursorCommand}

(*
  function Getkey(Prompt, Choices : VarString) : Char;
    {-return a legal menu choice}
  var
    Ch : Char;
  begin
    Write(prompt);
    repeat
      Ch := UpCase(ReadKey);
    until Pos(Ch, choices) <> 0;
    Getkey := Ch;
  end;                        {GetKey}

  function YesNo(Prompt : VarString; Default : Char) : Boolean;
    {-return True for yes, False for no}
  const
{$IFDEF GERMAN}
    YesChar = 'J';
    NoChar = 'N';
    DispStr = '(J/N/<Enter> fÅr '
{$ELSE}
    YesChar = 'Y';
    NoChar = 'N';
    DispStr = '(Y/N/<Enter> for '
{$ENDIF}
  var
    Ch : Char;
  begin
    Write(Prompt, DispStr, Default, ') ');
    repeat
      Ch := ReadKey;
      if Ch = ^M then
        Ch := Default;
      Ch := UpCase(Ch);
    until (Ch = YesChar) or (Ch = NoChar);
    WriteLn(Ch);
    YesNo := (Ch = YesChar);
  end;                        {YesNo}

  procedure ClrEol(Col, Row, Attr : Integer);
    {-clear to end of line}
  begin
    BlankLine[0] := Chr(81-Col);
    FastWrite(BlankLine, Row, Col, Attr);
  end;
*)

  procedure HaltError(Msg : Varstring);
    {-Display an error message and halt}
  begin                       {HaltError}
    RestoreScreen;
    WriteLn;
    WriteLn(Msg);
    Halt(1);
  end;                        {HaltError}

  {$L SEARCH}
  {$F+}
  function Search(var Buffer; BufLength : Word; St : String) : Word; external;
    {-Search through Buffer for St. BufLength is length of range to search.
      Returns 0 if not found. Otherwise, the result is the index into an
      array whose lower bound is 1. Subtract 1 for 0-based arrays.}
  {$F-}

  function FindString(IdString : String; var Deft; Size : Word) : LongInt;
    {-return the location of IdString in ProgramFile and read Size bytes into
     Deft.}
  const
{$IFDEF GERMAN}
    SeekErrorMsg : string[30] = 'Positionierfehler beim Lesen von ';
    ReadErrorMsg : string[22] = 'Ein- Ausgabefehler beim Lesen von ';
{$ELSE}
    SeekErrorMsg : string[30] = 'Seek error while reading from ';
    ReadErrorMsg : string[22] = 'I/O error reading from ';
{$ENDIF}
  var
    I, BufPos,
    IdSize, BufSize : Word;
    FSTemp : LongInt;
  label
    FoundIdString;

  begin
    IdSize := Succ(Length(IdString));
    BufSize := SizeOf(SearchBuffer);

    {if we have a full buffer, see if it contains the ID string}
    if BufferFull then begin
      BufPos := Search(BufPtr^, BytesRead, IdString);
      if BufPos <> 0 then
        goto FoundIdString;
    end;

    {point at start of file}
    Seek(ProgramFile, 0);
    if (IOResult <> 0) then
      HaltError(SeekErrorMsg + ProgramName);

    {Read the first bufferful}
    BlockRead(ProgramFile, BufPtr^, BufSize, BytesRead);
    if (IOResult <> 0) then
      HaltError(ReadErrorMsg + ProgramName);

    {set flag to indicate the buffer is full}
    BufferFull := True;

    {keep track of file pointer}
    FilePtr := BytesRead;

    {scan the first buffer}
    BufPos := Search(BufPtr^, BytesRead, IdString);

    {loop until IdString found or end of file reached}
    while (BufPos = 0) and (BytesRead > IdSize) do begin

      {Move the tail end of the buffer to the front of the buffer}
      Move(BufPtr^[BytesRead-IdSize], BufPtr^, IdSize);

      {Read the next bufferful}
      BlockRead(ProgramFile, BufPtr^[IdSize], BufSize-IdSize, BytesRead);

      {keep track of where we are in the file}
      FilePtr := FilePtr + BytesRead;

      {adjust BytesRead to indicate the actual number of bytes in the buffer}
      BytesRead := BytesRead + IdSize;

      {search the buffer for the IdString}
      BufPos := Search(BufPtr^, BytesRead, IdString);
    end;

FoundIdString:
    if (BufPos = 0) then
      FSTemp := 0
    else begin
      {account for fact that BufPtr^ is a 0-based array}
      Dec(BufPos);

      {calculate the actual position in the file}
      FSTemp := (FilePtr - BytesRead) + BufPos + IdSize;

      {get the existing default parameter area into Deft}

      {Use contents of existing buffer if possible}
      if (BytesRead - BufPos) > Size then
        Move(BufPtr^[BufPos + IdSize], Deft, Size)
      else begin

        {seek to the right location}
        Seek(ProgramFile, FSTemp);
        if (IOResult <> 0) then
          HaltError(SeekErrorMsg + ProgramName);

        {read directly into Deft from ProgramFile}
        BlockRead(ProgramFile, Deft, Size, I);
        if I <> Size then
           HaltError(ReadErrorMsg + ProgramName);
      end;
    end;

    FindString := FSTemp;
  end;                      {findstring}

  function ModifyDefaults(FileOfst : LongInt; var B; Size : Word) : Boolean;
    {-Write modified default settings back to disk, returning a success flag}
  var
    BytesWritten : Word;
  begin                       {ModifyDefaults}
    {seek into file}
    Seek(ProgramFile, FileOfst);
    if (IOResult <> 0) then
{$IFDEF GERMAN}
      HaltError('Positionierfehler beim Schreiben von '+ ProgramName);
{$ELSE}
      HaltError('Seek error while writing to '+ ProgramName);
{$ENDIF}

    {write modified defaults}
    BlockWrite(ProgramFile, B, Size, BytesWritten);

    {return success/failure flag}
    ModifyDefaults := (BytesWritten = Size);
  end;                        {ModifyDefaults}

  procedure Initialize(Name, Title : String);
    {-Set up for installation}
  var
    ScreenOfs : longint;
  begin                       {Initialize}
    {setup screen}
    SetColor(TiColor);
    ClrScr;

    {save the name of the program for other routines}
    ProgramName := Name;

    {signon message}
    WriteLn(^M^J, Title, ^M^J);

    {Make sure executable file is found}
    FindFile(ProgramName);

{$IFDEF GERMAN}
    If ProgramName = '' Then
      HaltError('AusfÅhrbares Programm '+ProgramName+' nicht gefunden');
    if not(FileExists(ProgramName, ProgramFile)) then
      HaltError('AusfÅhrbares Programm '+ProgramName+' nicht gefunden');
{$ELSE}
    If ProgramName = '' Then
      HaltError('Executable file '+ProgramName+' not found');
    if not(FileExists(ProgramName, ProgramFile)) then
      HaltError('Executable file '+ProgramName+' not found');
{$ENDIF}

    {get a work area}
    New(BufPtr);
    BufferFull := False;

    {anything else}
    FillChar(BlankLine[1], 80, #32);
  end;                        {Initialize}

  procedure CleanUp;
    {-Clean up at end of program}
  begin                       {CleanUp}
    Close(ProgramFile);
    RestoreScreen;
  end;                        {CleanUp}

end.
