{$S-,R-,V-,I-,B-,F+}

{$IFNDEF Ver40}
  {$I OMINUS.INC}
{$ENDIF}

{*********************************************************}
{*                   TPMOUSE.PAS 5.07                    *}
{*        Copyright (c) TurboPower Software 1988.        *}
{* Portions copyright (c) Sunny Hill Software 1985, 1986 *}
{*     and used under license to TurboPower Software     *}
{*                 All rights reserved.                  *}
{*                                                       *}
{* With special modification to allow mouse hot boxes.   *}
{* See comments at MouseHotBox. To turn off the mod,     *}
{* simply comment the following define.                  *}
{*                                                       *}
{* Mod by Mike McIntyre, CompuServe 72251,43             *}
{*********************************************************}

{$DEFINE EnableMouseHotBoxes}

unit TpMouse;
  {-Mouse interface routines. Designed for use in text mode only.}

interface

uses TpCrt;

var
  MouseInstalled : Boolean;
  MouseCursorOn : Boolean;

type
  ButtonStatus = (
    NoButton, LeftButton, RightButton, BothButtons,
    {the following values are possible only on a 3-button mouse}
    CenterButton, LeftAndCenterButtons, RightAndCenterButtons, All3Buttons);

const
  DisableEventHandler = $00;
  MouseMoved = $01;
  LeftButtonPressed = $02;
  LeftButtonReleased = $04;
  RightButtonPressed = $08;
  RightButtonReleased = $10;
  CenterButtonPressed = $20;
  CenterButtonReleased = $40;
  AllMouseEvents = $7F;
type
  MouseEventType = DisableEventHandler..AllMouseEvents;

const
  DefaultScreenMask = $FFFF;
  DefaultCursorMask = $7700;

type
  MouseState =
    record
      BufSize : Word;
      Buffer : array[1..400] of Byte;
    end;
  MouseStatePtr = ^MouseState;

var
  {current window coordinates for mouse}
  MouseXLo : Byte;           {0-based}
  MouseYLo : Byte;           {0-based}
  MouseXHi : Byte;           {1-based}
  MouseYHi : Byte;           {1-based}

const
  {if True, MouseKeyWord waits for the button to be released before returning
   its key code}
  WaitForButtonRelease : Boolean = True;

  {pseudo-scan codes returned by MouseKeyWord--DO NOT CHANGE THESE}
  MouseLft = $EF00;          {left button}
  MouseRt = $EE00;           {right button}
  MouseBoth = $ED00;         {both buttons}
  MouseCtr = $EC00;          {center button}
  MouseLftCtr = $EB00;       {left and center buttons}
  MouseRtCtr = $EA00;        {right and center buttons}
  MouseThree = $E900;        {all three buttons}
var
  MouseKeyWordX : Byte;      {mouse coordinates at time of call to MouseKeyWord}
  MouseKeyWordY : Byte;

const
  MouseRoutine : Pointer = nil;
  MouseRoutineEvent : MouseEventType = DisableEventHandler;

  MouseEvent : MouseEventType = DisableEventHandler;
  MouseStatus : ButtonStatus = NoButton;
  MouseLastX : Byte = 1;
  MouseLastY : Byte = 1;

{$IFDEF EnableMouseHotBoxes}
  procedure MouseHotBox(XLow, YLow, XHigh, YHigh : byte; Button : ButtonStatus;
                        ScanCodeToReturn : word);
    {-Installs a mouse hot box record in a stack-like structure which is
      scanned in MouseKeyWord. If the button(s) pressed equals Button, and
      the mouse cursor is within the hot box window, then ScanCodeToReturn
      is stuffed into the keyboard buffer, and $FFFF is returned by
      MouseKeyWord, instead of the pseudo-scan code for the button(s)
      pressed. This facilitates the selection (with a mouse) of screen boxes
      that stand for keys like ESC and F1. Since all of TPro's high-level
      units ultimately call MouseKeyWord (if mouse support is enabled), the
      effect is global. Note: the screen coordinates are mouse window
      relative.}

  procedure PopMouseHotBox;
    {-Discards the most recently installed mouse hot box.}
{$ENDIF}

function MousePressed : Boolean;
  {-Return True if a mouse button is currently being pressed}

function MouseKeyWord : Word;
  {-Return a pseudo-scan code based on which mouse button is being pressed}

function ReadKeyOrButton : Word;
  {-Return next key or mouse button}

procedure EnableEventHandling;
  {-Enable the event handler needed for MousePressed and MouseKeyWord}

procedure DisableEventHandling;
  {-Disable the event handler installed by EnableEventHandling}

procedure InitializeMouse;
  {-Reinitializes mouse and sets MouseInstalled}

procedure ShowMouse;
  {-Show the mouse cursor.}

procedure HideMouse;
  {-Hide the mouse cursor}

procedure MouseWhereXY(var MouseX, MouseY : Byte; var Status : ButtonStatus);
  {-Return mouse position and button status}

function MouseWhereX : Byte;
  {-Return current X coordinate for mouse}

function MouseWhereY : Byte;
  {-Return current Y coordinate for mouse}

procedure MouseGotoXY(MouseX, MouseY : Byte);
  {-Set mouse position}

function MouseButtonPressed(Button : ButtonStatus; var Count : Word;
                            var LastX, LastY : Byte) : Boolean;
  {-Returns True if the Button to check has been pressed. If so, Count has the
    number of times it has been pressed, and LastX/LastY have its position the
    last time it was pressed.}

function MouseButtonReleased(Button : ButtonStatus; var Count : Word;
                             var LastX, LastY : Byte) : Boolean;
  {-Returns True if the Button to check has been released. If so, Count has the
    number of times it has been released, and LastX/LastY have its position the
    last time it was released.}

procedure MouseWindow(XLow, YLow, XHigh, YHigh : Byte);
  {-Sets window coordinates to be observed by the mouse}

procedure FullMouseWindow;
  {-Sets mouse window coordinates to full screen}

function MouseInWindow(XLo, YLo, XHi, YHi : Byte) : Boolean;
  {-Return True if mouse is within the specified window}

procedure SoftMouseCursor(ScreenMask, CursorMask : Word);
  {-Set mouse to use a software cursor}

procedure HardMouseCursor(StartLine, EndLine : Word);
  {-Set mouse to use the hardware cursor. StartLine and EndLine specify the
    shape of the cursor.}

procedure NormalMouseCursor;
  {-Set normal scan lines for mouse cursor based on current video mode}

procedure FatMouseCursor;
  {-Set larger scan lines for mouse cursor based on current video mode}

procedure BlockMouseCursor;
  {-Set scan lines for a block mouse cursor}

procedure HiddenMouseCursor;
  {-Hide the mouse cursor}

procedure GetMickeyCount(var Horizontal, Vertical : Integer);
  {-Returns the horizontal and vertical mickey count since the last call to
    this function. Negative numbers indicate movement up or to the left;
    positive numbers indicate movement down or to the right.}

procedure SetMickeyToPixelRatio(Horizontal, Vertical : Integer);
  {-Sets the mickey-to-pixel ratio. Default setting is 8,16. A setting of
    16,32 slows down the mouse considerably. A setting of 4,8 makes the
    mouse fly.}

procedure SetMouseEventHandler(EventMask : MouseEventType; UserRoutine : Pointer);
  {-Sets the address of a routine to be called when the specified mouse
    events occur. TPMOUSE handles the saving of the mouse driver's registers
    and sets up the DS register for the UserRoutine. Information about the
    Event is passed to UserRoutine using the global variables MouseEvent,
    MouseStatus, MouseLastX, and MouseLastY}

  {-- The remaining routines may not be implemented by all mouse drivers!! --}

function GetMousePage : Byte;
  {-Returns the video page where the mouse is being displayed}
  {-- May not be implemented in all mouse drivers!! --}

procedure SetMousePage(Page : Byte);
  {-Sets the video page where the mouse will be displayed}
  {-- May not be implemented in all mouse drivers!! --}

  {-- the following routines are intended primarily for use in TSR's --}

function MouseStateBufferSize : Word;
  {-Returns amount of memory needed to save the state of the mouse driver}

procedure SaveMouseState(var MSP : MouseStatePtr; Allocate : Boolean);
  {-Save the state of the mouse driver, allocating the buffer if requested.}

procedure RestoreMouseState(var MSP : MouseStatePtr; Deallocate : Boolean);
  {-Restore the state of the mouse driver and Deallocate the buffer if
    requested}

  {==========================================================================}

implementation

{$IFDEF EnableMouseHotBoxes}
  type
    HotBoxPtr             = ^HotBox;
    HotBox =
      record
        XLow,YLow         : byte;         {hot box screen coordinates}
        XHigh,YHigh       : byte;         {note: mouse window relative}
        Button            : ButtonStatus; {the trigger button(s)}
        ScanCodeToReturn  : word;         {the scan code/ascii value to return}
        Next              : HotBoxPtr;    {pointer to next hot box, nil if none}
      end;
  const
    TopHotBox : HotBoxPtr = nil;          {top of hot box stack}
{$ENDIF}

var
  SaveExitProc : Pointer;
const
  EventHandlerInstalled : Boolean = False;

  {$L TPMOUSE.OBJ}

  procedure InitializeMouse; external;
  procedure ShowMousePrim; external;
  procedure HideMousePrim; external;
  procedure MouseWhereXY(var MouseX, MouseY : Byte;
  var Status : ButtonStatus); external;
  function MouseWhereX : Byte; external;
  function MouseWhereY : Byte; external;
  procedure MouseGotoXY(MouseX, MouseY : Byte); external;
  function MouseButtonPressed(Button : ButtonStatus; var Count : Word;
  var LastX, LastY : Byte) : Boolean; external;
  function MouseButtonReleased(Button : ButtonStatus; var Count : Word;
  var LastX, LastY : Byte) : Boolean; external;
  procedure MouseWindow(XLow, YLow, XHigh, YHigh : Byte); external;
  procedure SoftMouseCursor(ScreenMask, CursorMask : Word); external;
  procedure HardMouseCursor(StartLine, EndLine : Word); external;
  function GetMousePage : Byte; external;
  procedure SetMousePage(Page : Byte); external;
  procedure GetMickeyCount(var Horizontal, Vertical : Integer); external;
  procedure SetMickeyToPixelRatio(Horizontal, Vertical : Integer); external;

  {these procedures, used internally, are all called FAR}
  procedure MouseEventPrim(EventMask : MouseEventType; UserRoutine : Pointer); external;
  procedure MouseEventHandler; external;
  function GetStorageSize : Word; external;
  {-Returns amount of memory needed to save state of mouse driver}
  procedure SaveMouseStatePrim(var Buffer); external;
  {-Save mouse state in Buffer}
  procedure RestoreMouseStatePrim(var Buffer); external;
  {-Restore mouse state from Buffer}

{$IFDEF EnableMouseHotBoxes}
  procedure MouseHotBox(XLow, YLow, XHigh, YHigh : byte; Button : ButtonStatus;
                        ScanCodeToReturn : word);
  var
    Temp                      : HotBoxPtr;
  begin
    if MouseInstalled and (SizeOf(HotBox) <= MaxAvail) then begin
      GetMem(Temp, SizeOf(HotBox));
      Temp^.XLow   := XLow;
      Temp^.YLow   := YLow;
      Temp^.XHigh  := XHigh;
      Temp^.YHigh  := YHigh;
      Temp^.Button := Button;
      Temp^.ScanCodeToReturn := ScanCodeToReturn;
      Temp^.Next   := TopHotBox;
      TopHotBox    := Temp;
    end;
  end;

  procedure PopMouseHotBox;
  var
    Temp                      : HotBoxPtr;
  begin
    if MouseInstalled and (TopHotBox <> nil) then begin
      Temp      := TopHotBox;
      TopHotBox := Temp^.Next;
      FreeMem(Temp, SizeOf(HotBox));
    end;
  end;
{$ENDIF}

  function MousePressed : Boolean;
    {-Return True if a mouse button is currently being pressed}
  begin
    if not(MouseInstalled and EventHandlerInstalled) then
      MousePressed := False
    else
      MousePressed := MouseStatus <> NoButton;
  end;

  function MouseKeyWord : Word;
    {-Return a pseudo scan code based on which key is being pressed}
  const
    ScanTable : array[LeftButton..All3Buttons] of Word = (MouseLft, MouseRt,
      MouseBoth, MouseCtr, MouseLftCtr, MouseRtCtr, MouseThree);
    BitsTable : array[ButtonStatus] of Byte = (0, 1, 1, 2, 1, 2, 2, 3);
  var
    Status, TempStatus : ButtonStatus;
    SaveBitsOn, BitsOn : Byte;
  {$IFDEF EnableMouseHotBoxes}
    Temp               : HotBoxPtr;
  {$ENDIF}
  begin
    {return bogus key code if no mouse or event handler not installed}
    if not(MouseInstalled and EventHandlerInstalled) then begin
      MouseKeyWord := $FFFF;
      Exit;
    end;

    {force interrupts on}
    inline($FB);               {sti}

    {wait for a button to be pressed}
    Status := MouseStatus;
    while Status = NoButton do begin
      {make sure TSR's can pop up}
      inline($cd/$28);
      Status := MouseStatus;
    end;

    if WaitForButtonRelease then begin
      {save the current number of buttons that are on}
      SaveBitsOn := BitsTable[Status];

      {wait for the button(s) now being pressed to be released}
      TempStatus := MouseStatus;
      while (Byte(TempStatus) and Byte(Status)) <> 0 do begin
        {see if an additional button has been pressed}
        BitsOn := BitsTable[TempStatus];
        if BitsOn > SaveBitsOn then begin
          {another button was pressed--we want it too}
          Status := TempStatus;
          SaveBitsOn := BitsOn;
        end;

        {make sure TSR's can pop up}
        inline($cd/$28);

        TempStatus := MouseStatus;
      end;
    end;

    {turn interrupts off}
    inline($FA);

    {return pseudo-scan code}
    MouseKeyWord := ScanTable[Status];

    {save current mouse coordinates}
    MouseKeyWordX := MouseLastX;
    MouseKeyWordY := MouseLastY;

    {turn interrupts on}
    inline($FB);

  {$IFDEF EnableMouseHotBoxes}
    Temp := TopHotBox;
    while Temp <> nil do   {scan Mouse hot boxes for a match}
      if (Status = Temp^.Button) and
         (MouseKeyWordX >= Temp^.XLow) and (MouseKeyWordX <= Temp^.XHigh) and
         (MouseKeyWordY >= Temp^.YLow) and (MouseKeyWordY <= Temp^.YHigh) then
          begin
            MouseKeyWord := $FFFF;
            StuffKey(Temp^.ScanCodeToReturn);
            Temp := nil;
          end
      else
            Temp := Temp^.Next;
  {$ENDIF}
  end;

  procedure ShowMouse;
    {-Show the mouse cursor.}
  begin
    if not MouseCursorOn then
      ShowMousePrim;
  end;

  procedure HideMouse;
    {-Hide the mouse cursor}
  begin
    if MouseCursorOn then
      HideMousePrim;
  end;

  procedure NormalMouseCursor;
    {-Set normal scan lines for mouse cursor based on current video mode}
  var
    ScanLines : Word;
  begin
    if Font8x8Selected then
      ScanLines := $0507
    else if CurrentMode = 7 then
      ScanLines := $0B0C
    else
      ScanLines := $0607;
    HardMouseCursor(Hi(ScanLines), Lo(ScanLines));
  end;

  procedure FatMouseCursor;
    {-Set larger scan lines for mouse cursor based on current video mode}
  var
    ScanLines : Word;
  begin
    if Font8x8Selected then
      ScanLines := $0307
    else if CurrentMode = 7 then
      ScanLines := $090C
    else
      ScanLines := $0507;
    HardMouseCursor(Hi(ScanLines), Lo(ScanLines));
  end;

  procedure BlockMouseCursor;
    {-Set scan lines for a block mouse cursor}
  var
    EndLine : Byte;
  begin
    if Font8x8Selected or (CurrentMode <> 7) then
      EndLine := $07
    else
      EndLine := $0C;
    HardMouseCursor(0, EndLine);
  end;

  procedure HiddenMouseCursor;
    {-Hide the mouse cursor}
  begin
    HardMouseCursor($20, 0);
  end;

  procedure FullMouseWindow;
    {-Sets mouse window coordinates to full screen}
  begin
    MouseWindow(1, 1, ScreenWidth, ScreenHeight);
  end;

  function MouseInWindow(XLo, YLo, XHi, YHi : Byte) : Boolean;
    {-Return True if mouse is within the specified window}
  var
    mX, mY : Byte;
    Status : ButtonStatus;
  begin
    {get current position of mouse and see if it's inside the window}
    MouseWhereXY(mX, mY, Status);
    MouseInWindow := (mX >= XLo) and (mX <= XHi) and (mY >= YLo) and (mY <= YHi);
  end;

  function MouseStateBufferSize : Word;
    {-Returns amount of memory needed to save the state of the mouse driver}
  var
    I : Word;
  begin
    if not MouseInstalled then
      MouseStateBufferSize := 0
    else begin
      I := GetStorageSize;
      if I <> 0 then
        Inc(I, SizeOf(Word));
      MouseStateBufferSize := I;
    end;
  end;

  procedure SaveMouseState(var MSP : MouseStatePtr; Allocate : Boolean);
    {-Save the state of the mouse driver, allocating the buffer if requested.}
  var
    I : Word;
  begin
    if Allocate then begin
      {assume failure}
      MSP := nil;

      {make sure a mouse is installed}
      if not MouseInstalled then
        Exit;

      {see how much memory we need}
      I := MouseStateBufferSize;

      {exit if 0 was returned or insufficient memory exists}
      if (I = 0) or (I > MaxAvail) then
        Exit;

      {allocate the MouseState record}
      GetMem(MSP, I);

      {fill in the MouseState record} {!!.10}
      MSP^.BufSize := I;              {!!.10}
    end;

    SaveMouseStatePrim(MSP^.Buffer);
  end;

  procedure RestoreMouseState(var MSP : MouseStatePtr; Deallocate : Boolean);
    {-Restore the state of the mouse driver and Deallocate the buffer if
      requested}
  begin
    {exit if MSP is nil}
    if (MSP = nil) or not MouseInstalled then
      Exit;

    {restore the mouse state}
    RestoreMouseStatePrim(MSP^.Buffer);

    if Deallocate then begin
      {deallocate the buffer}
      FreeMem(MSP, MSP^.BufSize);

      {set MSP to nil so we won't do the same thing twice}
      MSP := nil;
    end;
  end;

  procedure EnableEventHandling;
    {-Enable the event handler needed for MousePressed and MouseKeyWord}
  begin
    if MouseInstalled and not EventHandlerInstalled then begin
      MouseEventPrim(AllMouseEvents, @MouseEventHandler);
      EventHandlerInstalled := True;
    end;
  end;

  procedure SetMouseEventHandler(EventMask : MouseEventType; UserRoutine : Pointer);
    {-Sets the address of a routine to be called when the specified mouse
      events occur}
  begin
    {make sure a mouse is installed}
    if not MouseInstalled then
      Exit;

    if EventMask = DisableEventHandler then
      MouseRoutine := nil
    else
      MouseRoutine := UserRoutine;
    if MouseRoutine = nil then
      MouseRoutineEvent := DisableEventHandler
    else
      MouseRoutineEvent := EventMask;

    {enable the event handler if it isn't already}
    EnableEventHandling;
  end;

  procedure DisableEventHandling;
    {-Disable the event handler installed by EnableEventHandling}
  begin
    if EventHandlerInstalled then begin
      {disable the event handler}
      MouseEventPrim(DisableEventHandler, nil);

      {set flag to indicate that we're not installed}
      EventHandlerInstalled := False;

      {reset variables}
      MouseRoutine := nil;
      MouseRoutineEvent := DisableEventHandler;
      MouseEvent := DisableEventHandler;
      MouseStatus := NoButton;
    end;
  end;

  function ReadKeyOrButton : Word;
    {-Return next key or mouse button}
  var
    I : Word;
  begin
    I := $FFFF;
    repeat
      if KeyPressed then
        I := ReadKeyWord
      else if MousePressed then
        I := MouseKeyWord
      else
        {give TSR's a chance to pop up}
        inline($cd/$28);
    until I <> $FFFF;
    ReadKeyOrButton := I;
  end;

  procedure ExitHandler;
    {-Reinitialize and hide mouse on exit}
  begin
    {restore previous exit handler}
    ExitProc := SaveExitProc;

    {reinitialize and hide the mouse--disables all event handlers}
    InitializeMouse;
  end;

begin
  {initialize the mouse if one is installed (sets MouseInstalled)}
  InitializeMouse;

  {no need to install exit handler if not installed}
  if MouseInstalled then begin
    FullMouseWindow;
    SaveExitProc := ExitProc;
    ExitProc := @ExitHandler;
  end;
end.
   if Font8x8Sel
