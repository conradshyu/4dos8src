Unit History;

Interface

Uses
  Dos,
  TpHelp;

Procedure GetHistory(Help : HelpPtr);

Procedure SaveHistory(Help : HelpPtr);

Implementation

Procedure GetHistory(Help : HelpPtr);

Var
  Source : File;
  Path   : String;
  Amount : Word;

Begin
  Path := GetEnv('4HISTORY');
  If Path <> '' Then
  Begin
    Path := Path + '4HELP.HIS';
    Assign(Source,Path);
    {$I-}
    Reset(Source,1);
    {$I+}
    If IOResult = 0 Then
    Begin
      With Help^ Do
      Begin
        If FileSize(Source) = SizeOf(st) + SizeOf(sb) + SizeOf(stack) Then
        Begin
          BlockRead(Source,Help^.st,SizeOf(Help^.st));
          BlockRead(Source,Help^.sb,SizeOf(Help^.sb));
          BlockRead(Source,Help^.Stack,SizeOf(Help^.Stack));
        End;
        Close(Source);
      End;
    End;
  End;
End;

Procedure SaveHistory(Help : HelpPtr);

Var
  Target : File;
  Path   : String;

Begin
  Path := GetEnv('4HISTORY');
  If Path <> '' Then
  Begin
    Path := Path + '4HELP.HIS';
    Assign(Target,Path);
    {$I-}
    Rewrite(Target,1);
    {$I+}
    If IOResult <> 0 Then Exit;
    BlockWrite(Target,Help^.st,SizeOf(Help^.st));
    BlockWrite(Target,Help^.sb,SizeOf(Help^.sb));
    BlockWrite(Target,Help^.Stack,SizeOf(Help^.Stack));
    Close(Target);
  End;
End;

End.
