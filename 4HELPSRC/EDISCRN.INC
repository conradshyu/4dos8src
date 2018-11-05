{                          EDISCRN.INC
                           EDINST 4.0
             Copyright (c) 1985, 87 by Borland International, Inc.            }

  procedure ScreenInstall(ForceMono : Boolean; ScreenOfs : Longint);
    {-Customize FirstEd's screen.}
  var
    Ch : Char;

    procedure InstallColors(var AA : AttributeArray);
      {-Install default colors}
      {Note: This routine is designed to be easily incorporated into
       other installation programs for editors based on the Toolbox,
       including MSINST. Only two changes need to be made if more than four
       video attributes are used: (1) FarTopRow and FarBotRow need to be
       adjusted, and (2) the AttrsUsed set needs to be modified accordingly.
       For example, if the block cursor attribute is also used, FarBotRow
       can be increased by 1, and CursorColor would be added to the set.
       If all defined attributes are used, as they are in MicroStar, the
       box on the right (Far) side of the screen will be the same height
       as that on the left side.}
    const
      {if the asterisk is changed to another character, the BoxCharArray
       in DrawAttributeBox must also be changed}
      Choice : string[3] = ' * ';

      {Attributes used by FirstEd. For Microstar, use [TxtColor..Alt2Color]}
      AttrsUsed : set of ColorType = [FrAttr..BBAttr];

      {box on left}          {box on right (Far side)}
      TopRow = 2; FarTopRow = 7; {for MicroStar, these would be equal}
      BotRow = 21; FarBotRow = 14; {so would these}
      LeftCol = 5; FarLeftCol = 38;
      RtCol = 33; FarRtCol = 77;

      {prompt box on the bottom -- also defined by LeftCol and FarRtCol}
      TopPrompt = 22;
      BotPrompt = 25;

      {prompts}
      MainPrompt : string[38] =
      'Use  to move highlight, Ù to select';
      MainPrompt2: string[57] =
      'Alt-X to exit and save, Esc to exit without save';
      MakeSelectionPrompt =
      'Use '^Z' to move highlight, Ù to select, <Esc> to cancel';
      MakeSelectionPrompt2=
      'Press INS or ^U to toggle Blink/Underline on and off';
      AnyKey = 'Press any key to continue...';
      HereIs : string[38] = '  Here is a sample of this attribute. ';

    var
      CT, OldCT : ColorType;
      WAA : AttributeArray;
      Row, Col, Attr : Byte;
      Done : Boolean;
      BoxColorArray : array[3..20 {row} , 6..32 {col} ] of Byte;

      procedure Prompt(Message, Message2 : string);
        {-Show message centered in prompt box}
      var
        Width, Col : Byte;
      begin
        Width := Pred(FarRtCol-LeftCol);
        BlankLine[0] := Chr(Width);
        FastWrite(BlankLine, Succ(TopPrompt), Succ(LeftCol), LoColor);
        Col := Succ(LeftCol)+((Width-Length(Message)) shr 1);
        FastWrite(Message, Succ(TopPrompt), Col, LoColor);
        FastWrite(BlankLine, TopPrompt+2, Succ(LeftCol), LoColor);
        Col := Succ(LeftCol)+((Width-Length(Message2)) shr 1);
        FastWrite(Message2, TopPrompt+2, Col, LoColor);
      end;

      procedure CalcRowCol(Attr : Byte; var Row, Col : Byte);
        {-Calculate the row and column for an attribute}
      begin
        {calculate row}
        Row := (TopRow+2)+(Attr and $F);
        {calculate column}
        Col := (LeftCol+2)+(3*(Attr div 16));
      end;

      procedure DrawChart;
        {-Draw the color chart and initialize BoxColorArray}
      var
        I : Integer;
        Row, Col, Attr : Byte;
      begin
        FillChar(BoxColorArray, SizeOf(BoxColorArray), TiColor);
        for Attr := 0 to 127 do begin
          CalcRowCol(Attr, Row, Col);
          FastWrite(Choice, Row, Col, Attr);
          for I := Col to (Col+2) do
            BoxColorArray[Row, I] := (Attr and $F0)+(TiColor and $F);
        end;
      end;

      function ColorName(CType : ColorType) : VarString;
        {-Return a string describing the specified ColorType}
      begin                  {ColorName}
        case CType of
          {the following are used by both FirstEd and MicroStar}
          FrAttr : ColorName := 'Window frame';
          TeAttr : ColorName := 'Normal text & unselected menu';
          HeAttr : ColorName := 'Window header';
          XsAttr : ColorName := 'Selected menu & cross-reference';
          XrAttr : ColorName := 'Unselected cross-reference';
          SpAtt1 : ColorName := 'Headings';
          SpAtt2 : ColorName := 'Options and switches';
          SpAtt3 : ColorName := 'Cautions and warnings';
          PWAttr : ColorName := 'Prompt window';
          PIAttr : ColorName := 'Prompt input box';
          BBAttr : ColorName := 'Mouse button bar (bottom of screen)';
        end;
      end;                   {ColorName}

      function WhichRow(CType : ColorType) : Byte;
        {-Given a color type, return the row on which the color name should
         be displayed.}
      var
        CT : ColorType;
        LoopCount : Byte;
      begin                  {WhichRow}
        LoopCount := 0;
        for CT := FrAttr to BBAttr do
          if CT in AttrsUsed then
          begin
            Inc(LoopCount);
            if CT = CType then CT := BBAttr;
          end;
        WhichRow := FarTopRow+3+Pred(LoopCount);
      end;                   {WhichRow}

      procedure ColorSample(Attr : Byte);
        {-Change the attribute of the color sample}
      begin
        ChangeAttribute(Length(HereIs), Succ(FarTopRow), Succ(FarLeftCol), Attr);
      end;

      procedure DrawAttributeBox(Attr, Row, Col : Byte);
        {-Draw special box around current selection}
      const
        BoxCharArray : array[ -1..1, -2..2] of string[1] =
        (('Ú', 'Ä', 'Ä', 'Ä', '¿'),
         ('³', ' ', '*', ' ', '³'),
         ('À', 'Ä', 'Ä', 'Ä', 'Ù'));
      var
        A : Byte;
        I, J, RowDelta, ColDelta : Integer;
      begin
        for RowDelta := -1 to 1 do
          for ColDelta := -2 to 2 do begin
            I := Row+RowDelta;
            J := Col+ColDelta;
            A := BoxColorArray[I, J];

            {leave the attribute of ' * ' alone}
            case ColDelta of
              -1..1 : if RowDelta = 0 then
                        A := Attr;
            end;
            FastWrite(BoxCharArray[RowDelta, ColDelta], I, J, A);
          end;
      end;

      procedure ShowChoice(Attr : Byte; FirstCall : Boolean);
        {-Show the currently selected attribute}
      var
        Row, Col : Byte;
      begin
        {remove the previous box, if any}
        if not FirstCall then
          RestoreWindow(WP, False);

        {calculate the row and column for the new one}
        CalcRowCol(Attr and $7F, Row, Col);

        {save the portion of the window that will be overwritten}
        SaveWindow(WP, Pred(Col), Pred(Row), Col+3, Succ(Row), FirstCall);

        {draw the box that marks the current attribute}
        DrawAttributeBox(Attr, Row, Succ(Col));

        {change the attribute of the sample string too}
        ColorSample(Attr);
      end;

      procedure MakeSelection(var Attr : Byte);
        {-Allow user to select an attribute}
      var
        Done : Boolean;
        A : Byte;
      begin                  {MakeSelection}
        WP := nil;
        Done := False;
        A := Attr;
        ShowChoice(A, True);
        Prompt(MakeSelectionPrompt, MakeSelectionPrompt2);
        repeat
          case GetCursorCommand of
            ^M :             {select}
              begin
                Attr := A;
                Done := True;
              end;
            ^E, ^W :         {up}
              if (A and $F) = 0 then
                A := A+15
              else
                A := Pred(A);
            ^X, ^Z :         {down}
              if (A and $F) = $F then
                A := A-15
              else
                A := Succ(A);
            ^S :             {left}
              if A <= 15 then
                A := A+112
              else
                A := A-16;
            ^D :             {right}
              if A >= 112 then
                A := A-112
              else
                A := A+16;
            ^U :             {toggle blink/underline}
              A := A xor Blink;
            Escape : Done := True; {cancel}
          end;
          ShowChoice(A, False);
        until Done;
        RestoreWindow(WP, True);
      end;                   {MakeSelection}

      procedure HighlightName(OldCT, CT : ColorType);
        {-Highlight name of current selection}
      var
        N : Integer;
      begin
        {number of attribute bytes to change}
        N := Pred(FarRtCol-FarLeftCol);

        {remove highlight from OldCT}
        ChangeAttribute(N, WhichRow(OldCT), Succ(FarLeftCol), LoColor);

        {highlight CT}
        ChangeAttribute(N, WhichRow(CT), Succ(FarLeftCol), EdColor);
      end;

    begin                    {InstallColors}
      {initialize}
      WAA := AA;
      SetColor(TiColor);

      {hide the cursor}
      HiddenCursor;

      {draw the choices box}
      MakeBox(LeftCol, TopRow, RtCol, BotRow, TiColor);
      FastWrite(' Choices ', TopRow, LeftCol+10, TiColor);

      {draw the choices}
      for CT := FrAttr to BBAttr do
        if CT in AttrsUsed then
          FastWrite(ColorName(CT), WhichRow(CT), FarLeftCol+3, LoColor);

      {draw the menu box on the right}
      MakeBox(FarLeftCol, FarTopRow, FarRtCol, FarBotRow+7, TiColor);
      FastWrite(' Sample ', FarTopRow, FarLeftCol+16, TiColor);
      FastWrite('ÃÄÄÄÄÄÄÄÄÄÄÄÄÄ Attributes ÄÄÄÄÄÄÄÄÄÄÄÄÄ´', FarTopRow+2,
                  FarLeftCol, TiColor);
      FastWrite(HereIs, Succ(FarTopRow), Succ(FarLeftCol), TiColor);

      {draw the prompt box}
      MakeBox(LeftCol, TopPrompt, FarRtCol, BotPrompt, TiColor);
      Prompt(MainPrompt, MainPrompt2);

      {get choices}
      CT := FrAttr;
      Done := False;
      DrawChart;
      OldCT := CT;
      repeat
        ColorSample(WAA[CT]);
        HighlightName(OldCT, CT);
        OldCT := CT;
        Ch := GetCursorCommand;
        repeat
          case Ch of
            ^M : begin       {select}
                   MakeSelection(WAA[CT]);
                   Prompt(MainPrompt, MainPrompt2);
                 end;
            ^E, ^W :         {scroll up}
              if CT = FrAttr then
                CT := BBAttr
              else
                CT := Pred(CT);
            ^X, ^Z :         {scroll down}
              if CT = BBAttr then
                CT := FrAttr
              else
                CT := Succ(CT);
            Escape, #254 : Done := True; {done}
          end;
        until (CT in AttrsUsed);
      until Done;

      {copy the working array to the actual array}
      AA := WAA;

      NormalCursor;
    end;                     {InstallColors}

  begin                      {ScreenInstall}

    {install colors}
    with ScreenDefaults do
      if (VideoSegment = $B800) then begin
        {check for snow first}
        GoodColorCard := True; {assume no snow - argh, global warming!}
        InstallColors(ColorAttr);
      end
      else
        InstallColors(MonoAttr);

    {reset screen color}
    SetColor(LoColor);

    {write modified defaults}
    if Ch = #254 then
    begin
      if not ModifyDefaults(ScreenOfs, ScreenDefaults, SizeOf(ScreenDefaults)) then
        HaltError('Error writing screen defaults to '+ProgName);
    end;

  end;                       {ScreenInstall}
