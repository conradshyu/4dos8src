program X;
uses
  TpCRT;
var
  Row, I : byte;
begin
  CheckSnow := true;
  ClrScr;
  WriteLn('Current Mode is ', GetCrtMode);
  SelectFont8x8(true);
  WriteLn('Current Mode is ', GetCrtMode);
  Write('Press a key ...');
  if ReadKeyWord = 0 then {do nothing};
  while not KeyPressed do
    for Row := 1 to ScreenHeight do
      for I := 0 to 7 do begin
        Delay(1);
        FastWrite('FastWrite ', Row, Succ(I*10), Lo(Random(127)));
      end;
  if ReadKeyWord = 0 then {do nothing};
  SelectFont8x8(false);
end.

