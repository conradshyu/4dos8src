(***********************************************************************

  Slightly Modified TPMOUSE for 4HELP 4.0.  

  Added ManualMouseInit Procedure, which moves the mouse initialization
  code from this unit to the calling program.  You must call this 
  procedure in order to use the mouse in a program.

  Scott McGrath, 11-01-91 

************************************************************************)



{$S-,R-,V-,I-,B-,F+}

{$IFNDEF Ver40}
  {$I OMINUS.INC}
{$ENDIF}

{*********************************************************}
{*                   TPMOUSE.PAS 5.11                    *}
{*        Copyright (c) TurboPower Software 1988.        *}
{* Portions copyright (c) Sunny Hill Software 1985, 1986 *}
{*     and used under license to TurboPower Software     *}
{*                 All rights reserved.                  *}
{*********************************************************}

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
    CenterButton, LeftAndCenterButtons, RightAndCenterButtons, All3Buttons,
    {the following values are possible only on a wheel mouse}
    WheelDown, LeftButtonAndWheelDown, RightButtonAndWheelDown,
    BothButtonsAndWheelDown, CenterButtonAndWheelDown,
    LeftAndCenterButtonsAndWheelDown, RightAndCenterButtonsAndWheelDown,
    All3ButtonsAndWheelDown,
    WheelUp, LeftButtonAndWheelUp, RightButtonAndWheelUp,
    BothButtonsAndWheelUp, CenterButtonAndWheelUp,
    LeftAndCenterButtonsAndWheelUp, RightAndCenterButtonsAndWheelUp,
    All3ButtonsAndWheelUp
    );

const
  DisableEventHandler = $00;
  MouseMoved = $01;
  LeftButtonPressed = $02;
  LeftButtonReleased = $04;
  RightButtonPressed = $08;
  RightButtonReleased = $10;
  CenterButtonPressed = $20;
  CenterButtonReleased = $40;
  WheelMoved = $80;
  AllMouseEvents = $FF;
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
  MouseWhDn = $E800;         {wheel down}
  MouseLftWhDn = $E700;      {left button and wheel down}
  MouseRtWhDn = $E600;       {right button and wheel down}
  MouseBothWhDn = $E500;     {both buttons and wheel down}
  MouseCtrWhDn = $E400;      {center button and wheel down}
  MouseLftCtrWhDn = $E300;   {left and center buttons and wheel down}
  MouseRtCtrWhDn = $E200;    {right and center buttons and wheel down}
  MouseThreeWhDn = $E100;    {all three buttons and wheel down}
  MouseWhUp = $E000;         {wheel up}
  MouseLftWhUp = $DF00;      {left button and wheel up}
  MouseRtWhUp = $DE00;       {right button and wheel up}
  MouseBothWhUp = $DD00;     {both buttons and wheel up}
  MouseCtrWhUp = $DC00;      {center button and wheel up}
  MouseLftCtrWhUp = $DB00;   {left and center buttons and wheel up}
  MouseRtCtrWhUp = $DA00;    {right and center buttons and wheel up}
  MouseThreeWhUp = $D900;    {all three buttons and wheel up}
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

procedure ManualMouseInit;
  {Moved Mouse Initialization code out of this unit} {!!! 4.0}
  {==========================================================================}

implementation

var
  SaveExitProc : Pointer;
  EventHandlerInstalled : Boolean;

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
    ScanTable : array[LeftButton..All3ButtonsAndWheelUp] of Word = (MouseLft,
      MouseRt, MouseBoth, MouseCtr, MouseLftCtr, MouseRtCtr, MouseThree,
      MouseWhDn, MouseLftWhDn, MouseRtWhDn, MouseBothWhDn, MouseCtrWhDn,
      MouseLftCtrWhDn, MouseRtCtrWhDn, MouseThreeWhDn,
      MouseWhUp, MouseLftWhUp, MouseRtWhUp, MouseBothWhUp, MouseCtrWhUp,
      MouseLftCtrWhUp, MouseRtCtrWhUp, MouseThreeWhUp
    );
    BitsTable : array[ButtonStatus] of Byte = (0, 1, 1, 2, 1, 2, 2, 3,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
  var
    Status, TempStatus : ButtonStatus;
    SaveBitsOn, BitsOn : Byte;
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

    {reset wheel data}
    MouseStatus := ButtonStatus(ord(MouseStatus) and 7);

    {turn interrupts on}
    inline($FB);
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

  procedure ManualMouseInit;
  begin
    {initialize the mouse if one is installed (sets MouseInstalled)}
    InitializeMouse;

    {no need to install exit handler if not installed}
    if MouseInstalled then begin
      FullMouseWindow;
      SaveExitProc := ExitProc;
      ExitProc := @ExitHandler;
    end;
  end;

begin

  {INITIALIZATION CODE MOVED TO MANUALMOUSEINIT}

end.
