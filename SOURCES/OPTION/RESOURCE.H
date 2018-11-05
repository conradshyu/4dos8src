// RESOURCE.H -- Resource IDs for OPTION.EXE
// 	Copyright 1995  Rex Conn and JP Software Inc.  All rights reserved

// Lines beginning with //NUM are designed for processing by RENUM.BTM.
// DO NOT remove these lines!  See RENUM.BTM for syntax documentation.

// ---------------------------------- General --------------------------
// Hexadecimal IDs won't work for controls (due to limitations in Win16
// Resource Compiler)
//NUM DEC ON
//NUM NEXT 100


// ---------------------- INI file dialogs ------------------
//NUM OFF 2
#define IDI_NULL 0
#define IDI_BASE 1500
//NUM NEXT 1501

// Startup
//NUM ROUND
#define IDI_STARTUP_START IDI_AutoExecPath
#define IDI_STARTUP_FOCUS IDI_AutoExecPath
#define IDI_AutoExecPath        1511
#define IDI_AutoExecParms       1512
#define IDI_UMBLoad             1513
#define IDI_UMBHistory          1514
#define IDI_UMBDirHistory       1515
#define IDI_UMBEnvironment      1516
#define IDI_UMBAlias            1517
#define IDI_UMBFunction		1518
#define IDI_LocalAliases        1519
#define IDI_LocalDirHistory     1520
#define IDI_LocalFunctions      1521
#define IDI_LocalHistory        1522
#define IDI_LoadAssoc           1523
#define IDI_Alias               1524
#define IDI_EnvFree             1525
#define IDI_Environment         1526
#define IDI_Swapping            1527
#define IDI_Function		1528

#define IDI_4StartPath          1530
#define IDI_PauseOnError        1531
#define IDI_INIQuery            1532
#define IDI_History             1533
#define IDI_DirHistory          1534
#define IDI_WindowX             1535
#define IDI_WindowY             1536
#define IDI_WindowWidth         1537
#define IDI_WindowHeight        1538
#define IDI_ConsoleRows         1539
#define IDI_ConsoleColumns      1540
#define IDI_StackSize           1541
#define IDI_STARTUP_END IDI_StackSize

// Display
//NUM ROUND
#define IDI_DISPLAY_START IDI_ANSI
#define IDI_DISPLAY_FOCUS IDI_ANSI
#define IDI_ANSI                1550
#define IDI_BrightBG            1551
#define IDI_ScreenColumns       1552
#define IDI_ScreenRows          1553
#define IDI_Tabs                1554
#define IDI_StdColorsFG         1555
#define IDI_StdColorsBG         1556
#define IDI_InputColorsFG       1557
#define IDI_InputColorsBG       1558
#define IDI_DISPLAY_END IDI_InputColorsBG

// Command line
//NUM ROUND
#define IDI_CMDLINE_START IDI_EditModeOver
#define IDI_CMDLINE_FOCUS IDI_EditModeOver
#define IDI_EditModeOver        1560
#define IDI_EditModeIns         1561
#define IDI_EditModeInitOver    1562
#define IDI_EditModeInitIns     1563
#define IDI_CursorIns           1564
#define IDI_CursorOver          1565
#define IDI_NormalScrollKeys    1566
#define IDI_SwapScrollKeys      1567
#define IDI_HistMin             1568
#define IDI_HistCopy            1569
#define IDI_FileCompletion      1570
#define IDI_HistMove            1571
#define IDI_HistWrap            1572
#define IDI_HistDups		1573
//#define IDI_HistDups1		1574
// #define IDI_HistDups2		1575
#define IDI_AppendToDir         1576
#define IDI_CompleteHidden      1577
#define IDI_FuzzyCD0            1578
#define IDI_FuzzyCD1            1579
#define IDI_FuzzyCD2            1580
#define IDI_FuzzyCD3            1581
#define IDI_TreePath            1582
#define IDI_CMDLINE_END IDI_TreePath

// Windows
//NUM ROUND
#define IDI_WINDOWS_START IDI_PopupWinLeft
#define IDI_WINDOWS_FOCUS IDI_PopupWinLeft
#define IDI_PopupWinLeft        1600
#define IDI_PopupWinTop         1601
#define IDI_PopupWinWidth       1602
#define IDI_PopupWinHeight      1603
#define IDI_PopupWinColorsFG    1604
#define IDI_PopupWinColorsBG    1605
#define IDI_CDDWinLeft          1606
#define IDI_CDDWinTop           1607
#define IDI_CDDWinWidth         1608
#define IDI_CDDWinHeight        1609
#define IDI_CDDWinColorsFG      1610
#define IDI_CDDWinColorsBG      1611
#define IDI_Mouse               1612
#define IDI_WINDOWS_END IDI_Mouse

// Options 1
//NUM ROUND
#define IDI_CONFIG1_START IDI_Descriptions
#define IDI_CONFIG1_FOCUS IDI_Descriptions
#define IDI_Descriptions        1620
#define IDI_DescriptionMax      1621
#define IDI_CommandSep          1622
#define IDI_EscapeChar          1623
#define IDI_ParameterChar       1624
#define IDI_DecCharAuto         1625
#define IDI_DecCharPeriod       1626
#define IDI_DecCharComma        1627
#define IDI_ThouCharAuto        1628
#define IDI_ThouCharPeriod      1629
#define IDI_ThouCharComma       1630
#define IDI_UpperCase           1631
#define IDI_BatchEcho           1632
#define IDI_NoClobber           1633
#define IDI_Win95SFNSearch      1634
#define IDI_UnixPaths           1635
#define IDI_AmPmCtry            1636
#define IDI_AmPm12              1637
#define IDI_AmPm24              1638
#define IDI_DescriptionName     1639
#define IDI_BeepLength          1640
#define IDI_BeepFreq            1641
#define IDI_CONFIG1_END IDI_BeepFreq


// Options 2
//NUM ROUND
#define IDI_CONFIG2_START IDI_LogOn
#define IDI_CONFIG2_FOCUS IDI_LogOn
#define IDI_LogOn               1650
#define IDI_LogName             1651
#define IDI_LogErrors           1652
#define IDI_HistLogOn           1653
#define IDI_HistLogName         1654
#define IDI_EvalMin             1655
#define IDI_EvalMax             1656
#define IDI_EXTPROGGROUP        1657
#define IDI_HelpPath            1658
#define IDI_HelpOptions         1659
#define IDI_HelpBook            1660
#define IDI_REXXPath            1661
#define IDI_InstallPath         1662
#define IDI_CONFIG2_END IDI_InstallPath


// Commands
//NUM ROUND
#define IDI_CMD_START IDI_CopyPrompt
#define IDI_CMD_FOCUS IDI_CopyPrompt
#define IDI_CopyPrompt		1670
#define IDI_ColorDir            1671
#define IDI_ListColorsFG        1672
#define IDI_ListColorsBG        1673
#define IDI_ListStatBarColorsFG 1674
#define IDI_ListStatBarColorsBG 1675
#define IDI_Printer             1676
#define IDI_SelectColorsFG      1677
#define IDI_SelectColorsBG      1678
#define IDI_SelectStatBarColorsFG 1679
#define IDI_SelectStatBarColorsBG 1680
#define IDI_CMD_END IDI_SelectStatBarColorsBG

#define IDI_PathExt             1773

#define IDI_ID_MAX IDI_CMD_END


// Symbols used in INIFILE.H for items with multiple controls for a single
// INI file directive (radio button or color items)
#define IDI_StdColors IDI_StdColorsFG
#define IDI_InputColors IDI_InputColorsFG
#define IDI_EditMode IDI_EditModeOver
#define IDI_PopupWinColors IDI_PopupWinColorsFG
#define IDI_CDDWinColors IDI_CDDWinColorsFG
#define IDI_AmPm IDI_AmPmCtry
#define IDI_ListColors IDI_ListColorsFG
#define IDI_SelectColors IDI_SelectColorsFG
#define IDI_FuzzyCD IDI_FuzzyCD0
// #define IDI_HistDups IDI_HistDups0
#define IDI_EvalDecimal IDI_EvalDecimalPeriod
#define IDI_ListStatBarColors IDI_ListStatBarColorsFG
#define IDI_SelectStatBarColors IDI_SelectStatBarColorsFG
#define IDI_DecimalChar IDI_DecCharAuto
#define IDI_ThousandsChar IDI_ThouCharAuto

