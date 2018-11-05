{HELP.PAS}
  Version     = ' 4DOS 3.03 (D) HELP ';

  case Error of
      0 : DoHelp;                         {opened, so show help}
      2 : WriteLn('** ERROR ** Help file not found or not accessible');
    100 : WriteLn('** ERROR ** Unexpected end of help file');
    106 : WriteLn('** ERROR ** Help file has been corrupted');
    203 : WriteLn('** ERROR ** Insufficient memory for help system');
    else  WriteLn('** ERROR ** Unknown help system error #'+Long2Str(Error));
  end;

{HELPCFG.PAS}
Const
  Title1 = '4DOS 3.0 (D) HELP System';
  Title2 = 'Color Configuration';

{EDITOOLS.PAS}
      HaltError('Executable file '+ProgramName+' not found');

{EDISCRN.INC}
      Write('Press any key to begin snow test...');
      RetraceMode := YesNo('Did you see any "snow" on your display?', Ch);

      {prompts}
      MainPrompt { : string[57] } =
        'Use  to move highlight, Ù to select';
      MainPrompt2 = 
         '<Esc> to exit and save, Ctrl-Break to exit without save';
      MakeSelectionPrompt =
      'Use '^Z' to move highlight, Ù to select, <Esc> to cancel';
      MakeSelectionPrompt2 =
        'Press INS or ^U to toggle Blink/Underline on and off';
      AnyKey = 'Press any key to continue...';
      HereIs : string[37] = ' Here is a sample of this attribute. ';

          FrAttr : ColorName := 'Window frame';
          TeAttr : ColorName := 'Normal text & unselected menu';
          HeAttr : ColorName := 'Window header';
          XsAttr : ColorName := 'Selected menu & cross-reference';
          XrAttr : ColorName := 'Unselected cross-reference';
          SpAtt1 : ColorName := 'Headings';
          SpAtt2 : ColorName := 'Options and switches';
          SpAtt3 : ColorName := 'Cautions and warnings';

      FastWrite(' Choices ', TopRow, LeftCol+10, TiColor);

      FastWrite(' Sample ', FarTopRow, FarLeftCol+15, TiColor);
      FastWrite('ÃÄÄÄÄÄÄÄÄÄÄÄÄ Attributes ÄÄÄÄÄÄÄÄÄÄÄÄÄ´', FarTopRow+2,
                  FarLeftCol, TiColor);

