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

{                            Screen Routines
                          Copyright (c) 1985,87
                                   by
                       Borland International, Inc.
                 ( from the Editor Toolbox Version 4 )

                           Modified for 4HELP
                                   by
                         Ross Neilson Wentworth
}
{$I-}
{$R-}
{$V-}
{$S-}

unit SInst;
{-Fast screen writing routines for installation programs}

interface

uses
  tpCrt,                    {screen routines - standard unit}
  Dos;                      {dos calls - standard unit}

const
  Defnorows = 25;             {Default number of rows/physical screen}
  Defnocols = 80;             {Default number of cols/physical screen}

type
  ScreenBuffer = array[0..1999] of Word; {Structure of video memory}

  WindowRec =
  record
    TopRow, LeftCol, Width, Height : Byte;
    Contents : array[0..1999] of Integer;
  end;

  WindowPtr = ^WindowRec;

var
  RetraceMode : Boolean;      {Check for snow on color cards?}
  LoColor,                    {LOw video}
  TiColor,                    {TItle / high video}
  ChColor,                    {CHanged keystrokes}
  EdColor,                    {keystroke being EDited / reverse video}
  CfColor : Byte;             {ConFlicting keystrokes}


  procedure RestoreScreen;
    {-Clean up screen at end of program}

  procedure SetColor(TheColor : Byte);
    {-set both textcolor and textbackground}

  procedure MakeBox(Left, Top, Right, Bottom, Attr : Byte);
    {-Draw a box on the screen}

  procedure SaveWindow(var WP : WindowPtr; Left, Top, Right, Bottom : Byte; Alloc : Boolean);
    {-Save a window}

  procedure RestoreWindow(var WP : WindowPtr; DeAlloc : Boolean);
    {-Restore a previously saved window and dispose of memory allocated to it}

  procedure SinstGetScreenMode(ForceMono : boolean);
    {-determine screen address and colors}

  {==========================================================================}

implementation

  procedure RestoreScreen;
    {-Clean up screen at end of program}
  begin                       {RestoreScreen}
    NormVideo;
(* !!! *)
    Window(1, 1, ScreenWidth, ScreenHeight);
    ClrScr;
  end;                        {RestoreScreen}

  procedure SetColor(TheColor : Byte);
    {-Set both textcolor and textbackground}
  begin
    TextColor(TheColor and $F);
    TextBackground(TheColor shr 4);
  end;                        {SetColor}

  procedure MakeBox(Left, Top, Right, Bottom, Attr : Byte);
    {-Draw a box on the screen}
  var
    Row : Byte;
    Span : String[80];
    SLen : Byte absolute Span;
  const
    Upright : string[1] = #179;
  begin                       {MakeBox}
    SLen := Pred(Right-Left);
    FillChar(Span[1], SLen, #196);
    {Top}
    FastWrite( #218+Span+#191, Top, Left, Attr);
    {Bottom}
    FastWrite(#192+Span+#217, Bottom, Left, Attr);
    {Middle}
    for Row := Succ(Top) to Pred(Bottom) do begin
      FastWrite(Upright, Row, Left, Attr);
      FastWrite(UpRight, Row, Right, Attr);
    end;
  end;                        {MakeBox}

  procedure SaveWindow(var WP : WindowPtr; Left, Top, Right, Bottom : Byte; Alloc : Boolean);
    {-Save a window}
  var
    Ofst : Word;
    W, H, I : Integer;
  begin                       {SaveWindow}
    W := Succ(Right-Left);
    H := Succ(Bottom-Top);
    if Alloc then
      GetMem(WP, (W*H) shl 1 + 4);
    with WP^ do begin
      Width := W;
      Height := H;
      TopRow := Top;
      LeftCol := Left;
      for I := 0 to Pred(H) do begin
        Ofst := ( ((Pred(Top)+I) * ScreenWidth) + Pred(Left) ) shl 1;
        MoveScreen(Mem[VideoSegment:Ofst], Contents[I*W], W);
      end;
    end;
  end;                        {SaveWindow}

   procedure RestoreWindow(var WP : WindowPtr; DeAlloc : Boolean);
     {-Restore a previously saved window and dispose of memory allocated to it}
   var
     I : Integer;
     Ofst : Word;
   begin                      {RestoreWindow}
     with WP^ do begin
       for I := 0 to Pred(Height) do begin
         Ofst := ( ((Pred(TopRow)+I) * ScreenWidth) + Pred(LeftCol) ) shl 1;
         MoveScreen(Contents[I*Width], Mem[VideoSegment:Ofst], Width);
       end;
       if DeAlloc then begin
         Freemem(WP, (Width*Height) shl 1 + 4);
         WP := Nil;
       end;
     end;
   end;                       {RestoreWindow}

  procedure SinstGetScreenMode(ForceMono : boolean);
    {-determine screen address and colors}

    procedure ChooseColorSet(Mono : Boolean);
    begin
      if Mono then begin
        LoColor := $07;
        TiColor := $0F;
        ChColor := $0F;
        EdColor := $70;
        CfColor := $70;
      end
      else begin
        LoColor := $07;
        TiColor := $0E;
        ChColor := $0F;
        EdColor := $1E;
        CfColor := $4F;
      end;
    end;

  begin                       {EdGetScreenMode}
    If CurrentMode < 2 Then TextMode (CurrentMode + 2);
    Retracemode := (CurrentMode <> 7);
    if Retracemode
      then begin
      {Color card}
        Retracemode := Not (CurrentDisplay In[EGA,VGA]);
        ChooseColorSet((CurrentMode <> 3) or ForceMono)
      end
      else begin
      {Monochrome}
        ChooseColorSet(True);
    end;
  end;                        {EdGetScreenMode}

begin
end.
