Unit Guess;

Interface

Uses
  TpCrt,
  TpHelp,
  TpString;

Procedure InitialTopic(Help : HelpPtr;Var Topic : Word);

{-Tries to guess the initial topic number.  The guess is made in the }
{ following order:                                                   }
{          1. If ParamCount <> 0 use ParamStr(1)                     }
{          2. The word at the start of the line                      }
{                                                                    }
{ If a word is found a topic number for an exact match is attempted. }
{ If that is unsuccessful an attempt is made to find a match of the  }
{ following types:                                                   }
{          1. Truncated, e.g. "SELE" for "SELECT"                    }
{                                                                    }
{ Often more than one topic could match the given string, e.g. "DI"  }
{ would match "DIR", "DIRS", "DISKCOMP", and "DISKCOPY".  When this  }
{ occurs the first (as displayed on the topic menu) matching topic   }
{ will be selected.                                                  }
{                                                                    }
{ If, after all these attempts, no topic is found, zero (0) is       }
{ returned as the topic number, which signals the calling procedure  }
{ to display the topic menu.                                         }

Implementation

{$F+}   { these routines MUST be FAR }

Function TruncMatch(TopicName,ScreenName : String) : Boolean;

{-trys to match truncated words }

Begin
  TruncMatch := Pos(StUpCase(ScreenName),StUpCase(TopicName)) = 1;
End;

{$F-}  { end of FAR requirement }

Procedure InitialTopic(Help : HelpPtr;Var Topic : Word);

Var
  Name : String;
  Index : integer;
  WStart : integer;
  ScreenLine : String;

Begin
  If ParamCount > 0
    Then Name := ParamStr(1)      { get the word from the command line   }
  Else Begin                      { get the word from the screen         }
    FastRead((80 - WhereXAbs + 1),WhereYAbs,WhereXAbs,ScreenLine);
    ScreenLine := StUpCase(TrimTrail(ScreenLine));
    WriteLn('WhereY is ', WhereY, '   screen line is [', ScreenLine, ']');
    Name := '';
    Index := 1;
    while (not (ScreenLine[Index] in ['A'..'Z'])) and
      (Index <= Length(ScreenLine)) do
        Inc(Index);
    WStart := Index;
    if Index <= Length(ScreenLine) then begin
      while (ScreenLine[Index] in ['A'..'Z']) and
        (Index <= Length(ScreenLine)) do
          Inc(Index);
      Name := Copy(ScreenLine, WStart, (Index - WStart + 1));
    end;
    WriteLn('Name is [', Name, ']', ', press any key ...');
    ReadLn;
  End;
  If Name = ''                    { no word so no topic number           }
    Then Topic := 0
  Else Begin
    Topic := FindHelp(Help,Name,NIL);     { try an exact match }
       { if that didn't work try for an inexact match }
    If Topic = 0 Then Topic := FindHelp(Help,Name,@TruncMatch);
  End;
End;

End.
