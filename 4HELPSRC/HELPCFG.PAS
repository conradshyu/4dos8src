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
  | Unpublished work, Copyright 1990 - 1999, JP Software Inc., All Rights |
  | Reserved.                                                             |
  |_______________________________________________________________________|
*) 

{ HELPCFG Version 7.01

  Copyright 1990 - 1999, JP Software Inc., P.O. Box 1470, E. Arlington,
  MA 02174. All Rights Reserved.

  Originally written by Ross Neilson Wentworth, all rights sold to 
  JP Software Inc. }

Program HelpConfig;

Uses
  TpCrt,
  TpString,
  Sinst,
  EdiTools;

Const

{$IFDEF GERMAN}
  Title1 = '4DOS 7.01 (D)  HILFESYSTEM';
{$ELSE}
  Title1 = '4DOS 7.01 Help System';
{$ENDIF}

{$IFDEF GERMAN}
  Title2 = 'Farbauswahl';
{$ELSE}
  {$IFDEF UK}
    Title2 = 'Colour Configuration';
  {$ELSE}
    Title2 = 'Color Configuration';
  {$ENDIF}
{$ENDIF}

  Copyright = 'Copyright 1988 - 1999, JP Software Inc.';

  ProgName = '4HELP.EXE';

Type
  ColorType = (FrAttr,
               TeAttr,
               HeAttr,
               XsAttr,
               XrAttr,
               SpAtt1,
               SpAtt2,
               SpAtt3,
               PWAttr,
               PIAttr,
               BBAttr);
  AttributeArray = Array[ColorType] of Byte;
  SInstallRec = Record
                  ColorAttr : AttributeArray;
                  MonoAttr  : AttributeArray;
                  GoodColorCard : Boolean;
                End;

Const
  SidString : String[18] = '4DH701 PATCH HERE:';

Var
  SaveBreak : Boolean;                    {Saved state of CheckBreak}
  WP : WindowPtr;
  ScreenDefaults : SInstallRec;
  Param : string;
  ForceMono : boolean;
  ScreenOfs : longint;

{$I ediscrn.inc}

Begin
  SaveBreak := CheckBreak;
  CheckBreak := False;
  Param := StUpCase(ParamStr(1));        {check for /M or /BW}
  ForceMono := ((Pos('/M', Param) <> 0) or (Pos('/BW', Param) <> 0));
  SinstGetScreenMode(ForceMono);
  Initialize(ProgName, '');
  {Search for screen installation ID string}
  ScreenOfs := FindString(SIDstring, ScreenDefaults, SizeOf(ScreenDefaults));
  if ScreenOfs = 0 then
{$IFDEF GERMAN}
    HaltError('Incorrect version of '+ProgName);
{      HaltError('Bildschirmkennung nicht gefunden in '+ProgName);}
{$ELSE}
    HaltError('Incorrect version of '+ProgName);
{$ENDIF}

  ClrScr;
  Window(38, 1, 76, 7);
  FastCenter(Title1, 2, $0F);
  FastCenter(Title2, 3, $0F);
  FastCenter(Copyright, 5, $07);
  ScreenInstall(ForceMono, ScreenOfs);
  CleanUp;
  CheckBreak := SaveBreak;
End.

