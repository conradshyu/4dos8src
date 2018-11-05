

;  Permission is hereby granted, free of charge, to any person obtaining a copy
;  of this software and associated documentation files (the "Software"), to deal
;  in the Software without restriction, including without limitation the rights
;  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
;  copies of the Software, and to permit persons to whom the Software is
;  furnished to do so, subject to the following conditions:
;
;  (1) The above copyright notice and this permission notice shall be included in all
;  copies or substantial portions of the Software.
;
;  (2) The Software, or any portion of it, may not be compiled for use on any
;  operating system OTHER than FreeDOS without written permission from Rex Conn
;  <rconn@jpsoft.com>
;
;  (3) The Software, or any portion of it, may not be used in any commercial
;  product without written permission from Rex Conn <rconn@jpsoft.com>
;
;  (4) THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
;  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
;  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
;  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
;  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
;  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
;  SOFTWARE.


          title     DOSUTIL - Assembler utilities for 4DOS

          ;
          page      ,132                ;make wide listing
          ;
          comment   }

          (C) Copyright 1988 - 1999 Rex Conn & J.P. Software
          All Rights Reserved

          Original Author:    Rex Conn
          Rewritten by:       Tom Rawson, J.P. Software, 11/9/88
          Rewritten again:    Rex Conn 8/89 - 5/90, 3/91, 9/91-10/91, 11/92

          These 4DOS support routines provide fast screen I/O and other
          functions for 4DOS.

          } end description

          .model    medium

          ;
          ; Includes
          ;
          include   product.asm         ;product equates
          include   trmac.asm           ;general macros
          include   4dlparms.asm        ;loader parameters
          include   inistruc.asm        ;INI file structures and macros
          include   dvapi.inc           ;DESQview calls

EF_ID     equ       0200000h		; ID bit in EFLAGS register

MAXLINE   equ       256                 ;max command line length for dos_box

BIOS_RAM  segment at 40h
          org	87h
info	db	?
BIOS_RAM  ends

	.data

          extrn     __osmajor:byte      ; DOS major version number
          extrn     __osminor:byte	; DOS minor version number
          extrn     _gchMajor:byte	; "Real" version numbers
          extrn     _gchMinor:byte
          extrn     _gnOsVersion:word	; Decimal DOS version number
          extrn     _gaCountryInfo:CTRYINFO
          extrn     _gnCPU:word         ; CPU (86, 186, 286, 386, etc.)
          extrn     _gnOSFlags:word     ; O/S status flags
          extrn     _gpIniptr:word      ; INI file data pointer
          extrn     _gszCmdline:byte    ; parser's command line buffer

emm_name  db        'EMMXXXX0'          ; name of EMS driver
qemm_dev  db        'QEMM386$',0        ; name of QEMM driver

code37h   db        74h,7,3Ch,2,0B2h,0FFh,74h,1,0C3h; MS-DOS Fn 37h code:
code37len equ       $-code37h           ; jz $+9/cmp al,2/mov dl,-1/jz $+3/ret
          even
StatusNDP dw        ?                   ; address of NDP control word

SaveESP   dd        ?
int24seg  dw        ?			; saved INT 24h vector
int24off  dw        ?
rmargin   dw        ?                   ; display right margin
StatusVid dw        ?                   ; address of 6845 status register,
BIOSOutput dw       0			; send output through BIOS flag

BIOSRow   db        ?
BIOSCol   db        ?
BIOSPage  db        ?
BIOSColor db        ?

          ;
          ; Table to translate last 5 bits of return from INT 21
          ; function 4452h to DR-DOS version number -- 3 bytes per
          ; entry:  return value limit, minor version, major version.
          ;
DRVerTab  db        02h, 3, 40          ;1062 or below is DR-DOS 3.40
          db        04h, 3, 41          ;1064 or below is DR-DOS 3.41
          db        05h, 5, 00          ;1065 is DR-DOS 5.00
          db        11h, 6, 00          ;1071 or below is DR-DOS 6.00
          db        12h, 7, 01          ;1072 is Novell DOS 7 or OpenDOS 7.01
          db        13h, 7, 03          ;1073 is DR-DOS 7.02 or 7.03
          db        1Fh, 8, 00          ;1074 is DR-DOS 8.0

 IOCTL_INPUT	equ	 3
 IOCTL_OUTPUT	equ	12
  OPEN_COMMAND	equ	 0
 CLOSE_COMMAND	equ	 5
STATUS_COMMAND	equ	 6
  SIZE_COMMAND	equ	 8
          ;
          ; Device driver request header + data
          ;
ReqData	struc
          slength   db	?
          subunit   db	?
          command   db	?
          status    dw	?
          reserved  db	8 dup (?)
          media_id  db	?
          d_offset  dw	?
          d_segment dw	?
          bytes     db	?
          function  db	?
          cdstatus  dd	?
ReqData	ends

ReqHdr	ReqData	<18,0,IOCTL_OUTPUT,100h,'RESERVED',0,offset ReqHdr.function,_DATA,1,0,0>


	;
	; In DOS, references are in various segments; code is in MISC_TEXT
	;

SERV_TEXT segment   word public 'CODE'  ; define server segment
          extrn     ErrMsg:far          ; error message table scanner
          extrn     CritTAdr:dword      ; critical error text address
          extrn     ServErrs:byte       ; text address
SERV_TEXT ends                          ; just here for references, no data


_TEXT     segment   word public 'CODE'  ;define normal code seg for externals
          extrn     _HoldSignals:far	;break control
          extrn     _EnableSignals:far
          extrn     __write:far	;interface to C write routine
_TEXT     ends

	.code	MISC_TEXT  
          assume    cs:MISC_TEXT, ds:DGROUP, es:nothing, ss:nothing

calldosW	equ	int 21h		; call dos via INT 21

MSG_SEG	equ	SERV_TEXT		; use SERV_TEXT for messages


	;==================================================================
	;
	;  OPERATING SYSTEM INTERFACE ROUTINES (except files)
	;
	;==================================================================

          ;
          ; _ctoupper: map character to upper case (including NLS chars)
          ;
          ; On entry:
          ;         AX = character to convert
          ;
          ; On exit:
          ;         AX = converted character
          ;         All other registers unchanged, interrupts on
          ;

          public    _ctoupper_	; __WATCOMC__
_ctoupper_ label    far		; _fastcall
          entry     @_ctoupper,noframe,far
          or        ah,ah		; extended character?
          jnz       c2upret		; yes, return
          cmp       al,'a'		; no, upper case or other?
          jb        c2upret		; yes, return
          cmp       al,'z'		; no, lower case?
          ja        c2upnls		; no, see if NLS character
          and       al,11011111b	; yes, convert to upper case
          jmp       short c2upret	; return
c2upnls:  or        al,al		; ASCII?
          jns       c2upret		; yes, return
          cmp       _gnOsVersion,211	; no, DOS 2.11+?
          jb        c2upret		; no, return
          call      _gaCountryInfo.CY_CFUNC; yes, convert NLS character
c2upret:  exit

          ;
          ; QuerySwitchChar - get the current DOS switch character
          ;
          ; On exit:
          ;         AX, DX destroyed
          ;         All other registers unchanged, interrupts on
          ;

          entry     QuerySwitchChar,noframe,far

          mov       ax,3700h
          calldosW
          cmp       al,0FFh		; function supported?
          jne       got_switchar
          mov       dl,'/'		; default to forward slash
got_switchar:
          mov       al,dl

          exit


          ; SetSwitchChar - set the current DOS switch character
          ;
          ; On entry:
          ;         Arguments on stack:
          ;           char SwitchChar
          ;
          ; On exit:
          ;         AX, DX destroyed
          ;         All other registers unchanged, interrupts on
          ;

          entry     SetSwitchChar,argframe,far
          argB      SwitchChar
          push      cx
          push      si
          push      di
          push      es                  ;save all used registers but ax and dx
          mov       ax,3701h
          mov       dl,SwitchChar       ;try to set the switchar the normal way
          calldosW
          mov       ax,3700h            ;now verify that it was really accepted
          calldosW                      ;or just coincides with the current one
          cmp       dl,SwitchChar       ;do the requested and actual one match?
          je        SetSwChret          ;if so return, else do it the hard way!
          mov       dx,_gnOSFlags       ;Patch the Fn 37h code for MS-DOS 5.0+:
          shln      dh,4,cl             ; shift HMA bit (4) to CF
          sbb       ax,ax               ; -1 if DOS in HMA, 0 otherwise
          mov       es,ax               ; search segment
          xor       di,di               ; start from its beginning
          mov       cx,0FFFFh           ; search it all
          mov       al,code37h          ; for the start of the Fn 37h code
SearchCod:repne     scasb               ; do the search; first byte found?
          jne       SetSwChret          ; no, return
          push      di                  ; yes, save pointer
          push      cx                  ; save counter
          mov       cx,code37len-1      ; count from the second to last byte
          mov       si,offset code37h+1 ; point to the second byte
          repe      cmpsb               ; search for the rest of code 
          pop       cx                  ; restore counter
          pop       di                  ; restore pointer; found?
          jne       SearchCod           ; if not, keep searching
          cmp       word ptr es:[di-4],0B2C0h; C0=[or] al,al; B2=mov dl,['/']
          jne       SearchCod           ; previous 2 bytes don't match (hardly)
          mov       al,SwitchChar       ; take the new switchar
          dec       di                  ; point to the immediate operang of the
          dec       di                  ; previous instruction (mov dl,'/')
          stosb                         ; and store the new switchar there!
SetSwChret:pop      es                  ;restore the registers saved
          pop       di
          pop       si
          pop       cx
          exit


          ;
          ; QueryVerifyWrite - check disk write verify flag
          ;

          entry     QueryVerifyWrite,noframe,far

          mov       ah,054h
          calldosW
          xor       ah,ah

          exit


          ;
          ; SetVerifyWrite - set disk write verify flag
          ;
          ; On entry:
          ;         Arguments on stack:
          ;           unsigned int VerifyFlag
          ;

          entry     SetVerifyWrite,argframe,far

          argW      VerifyFlag

          mov       ax,VerifyFlag
          mov       ah,02Eh		; set VERIFY state
          xor       dx,dx
          calldosW

          exit


          ;
          ; GetFontPage - get Arabic/Hebrew font page
          ;
          ; On exit:
          ;         AX = current font page (0 if no Arabic or Hebrew installed)
          ;         All other registers unchanged, interrupts on
          ;

          entry     GetFontPage,noframe,far
          push      cx
          mov       ax,0AD41h		; get current font page in CX (DX = ???)
          xor       cx,cx		; so 0 is returned if no Arabic / Hebrew
          int       2Fh			; returned values:
          xchg      ax,cx		; Arabic CX=164/DX=161, Hebrew CX=DX=100
          pop       cx
          exit


          ;
          ; SetCodePage - set the current code page
          ;
          ; On entry:
          ;         int codepage = code page number to be set
          ;
          ; On exit:
          ;         AX, DX destroyed
          ;         All other registers unchanged, interrupts on
          ;

          entry     SetCodePage,argframe,far

          Argw      codepage

          push      bx
          mov       bx,codepage
          mov       ax,06602h
          calldosW
          jc        cp_error
          xor       ax,ax
cp_error:
          pop       bx
          exit


          ;
          ; SetCountry - set the current country code
          ;
          ; On entry:
          ;         int country = country code to be set
          ;
          ; On exit:
          ;         AX = 0 if successful, else error code
          ;         DX destroyed
          ;         All other registers unchanged, interrupts on
          ;

          entry     SetCountry,argframe,far
          ArgW      country
          push      bx
          mov       ax,country
          mov       bx,0FFh
          cmp       ax,bx
          jb        ctry_set
          xchg      ax,bx
ctry_set: mov       ah,38h
          mov       dx,-1
          calldosW
          jc        ctry_err
          xor       ax,ax
ctry_err: pop       bx
          exit


          ;
          ; GetIdlePtr - get DR DOS idle state data structure pointer
          ;
          ; On exit:
          ;         DX:AX = far pointer to the idle state data structure
          ;         All other registers unchanged, interrupts on
          ;

          entry     GetIdlePtr,noframe,far

          pushm     es,si,di
          mov       ax,4458h
          calldosW			; get pointer in ES:AX
          mov       dx,es
          popm      di,si,es

          exit


          ;
          ; CheckForBreak - check for a ^C / ^Break
          ;
          ; On exit:
          ;         AX destroyed
          ;         All other registers unchanged, interrupts on
          ;

          entry     CheckForBreak,noframe,far

          push      bx
          mov       ah, 0Bh
          int       21h
          pop       bx
          exit


          ;
          ; DUMMY24 - temporary replacement for INT 24h handler
          ;

          even
dummy24   proc      far

          mov       ax,3		; fail the call
          iret

dummy24   endp


          ;
          ; DosError - turn the DOS critical error handler on/off
          ;
          ; On entry:
          ;         Argument on stack:
          ;           int nErrorflag:   disable if 2, enable if 1
          ;
          ; On exit:
          ;         AX, DX destroyed
          ;         All other registers unchanged, interrupts on
          ;

          entry     DosError,argframe,far

          ArgW      nErrorFlag

          push      ds

          mov       ax,nErrorFlag
          cmp       ax,2
          jne       reset_popup

	  mov	    al,24h
	  calldos   GETINT		; save former INT 24h vector
	  mov	    int24off,bx
	  mov	    int24seg,es

          loadseg   ds,cs
          assume    ds:@curseg	;fix assumes
          mov       dx,offset dummy24
          mov       al,24h		; set dummy INT 24h vector
          calldos   SETINT
          jmp       short DEExit

reset_popup:
          assume    ds:DGROUP		;fix assumes
          mov       dx,int24off
          mov       ax,int24seg
          mov       ds,ax
          assume    ds:nothing          ; fix assumes
          mov       al,24h		; reset INT 24h vector
          calldos   SETINT
DEExit:
          pop       ds
          assume    ds:DGROUP		; fix assumes

          exit


          ;
          ; SetExtError - set extended error code, retrieved by DOS Function 59h
          ;
          ; On entry:
          ;         Argument on stack:
          ;           int nErrorCode
          ;
          ; On exit:
          ;         All registers unchanged, interrupts on
          ;

          entry     SetExtError,argframe,far
          ArgW      nErrorCode
          pushm     ax,bx,cx,dx,si,di,bp,ds,es
          calldos   GETPSP
          push      bx                  ; process ID in the DOS parameter list
          xor       ax,ax               ; (see RBIL Table 01686)
          pushm     ax,ax               ; reserved (0) and computer ID (current)
          calldos   GETERR              ; get extended error code (DOS Fn 59h)
          pushm     es,ds,di,si,dx,cx,bx
          push      nErrorCode          ; to be returned in AX by Function 59h
          mov       dx,sp               ; get a pointer to the parameter list
          loadseg   ds,ss               ; DS was destroyed by Function 59h!
          mov       ax,5D0Ah            ; set extended error information
          int       21h
          add       sp,22
          popm      es,ds,bp,di,si,dx,cx,bx,ax
          exit


          ;
          ; GETERROR - Get a DOS error message
          ;
          ; On entry:
          ;         Argument on stack using pascal calling convention:
          ;            int errnum:      error number
          ;            char *buf:       error output buffer
          ;
          ; On exit:
          ;         AX = address of error message (buf argument)
          ;         CX destroyed
          ;         All other registers and interrupt state unchanged
          ;

          entry     GetError,argframe,far   ;set up entry point

          argW      buf                 ;buffer address
          argW      errnum              ;error number

          pushm     bx,si,di,ds         ;save registers

          mov       di,buf              ;get buffer address
          push      ds                  ;save DS for buffer segment
          loadseg   es,MSG_SEG,ax	;get message segment address
          lds       si,es:CritTAdr      ;get critical error table address
          assume    ds:nothing          ;fix assumes
          pop       es                  ;get buffer segment in ES
          mov       ax,errnum           ;get error number
          call      ErrMsg              ;get critical error message
          xor       al,al               ;get null
          cld                           ;move goes forward
          stosb                         ;store null at end of message
          mov       ax,buf              ;set return value
          popm      ds,di,si,bx         ;restore registers
          assume    ds:DGROUP		;fix assumes

          exit                          ;all done


          ;
          ; GetDOSVersion - Get real DOS version information
          ;
          ; On entry:
          ;         No requirements
          ;
          ; On exit:
          ;         __osmajor, __osminor, and _gnOSFlags set for this DOS
          ;           version
          ;         AX, CX, DX, ES destroyed
          ;         All other registers and interrupt state unchanged
          ;

DOS_DR    equ       80h                 ;DR-DOS bit in _gnOSFlags high byte
DOS_OS2   equ       40h                 ;OS/2 bit in _gnOSFlags high byte
                                        ;  machine

          entry     GetDOSVersion,noframe,far

          pushm     bx,si,di,bp         ;save registers

          mov       ax,(D_VERSION shl 8);get DOS version in AX, OEM ID in BH
          calldos
          mov       bptr __osmajor,al   ;save major version
          mov       bptr __osminor,ah   ;save minor version
          mov       bptr _gchMajor,al   ;save true major version
          mov       bptr _gchMinor,ah   ;save true minor version
          mov       bl,bh               ;store the OEM ID
          mov       bh,0                ;isolate it
          shln      bx,3,cl             ;move it to bits 10-3
          mov       _gnOSFlags,bx       ;store these bits in OS flags
          ;
          mov       ax,30DBh            ;check for Datalight ROM-DOS
          xor       cx,cx               ;CX must be 0
          mov       si,0B2D2h           ;ROM-DOS specific function "key"
          mov       di,3                ;function #3, Get ROM-DOS revision
          calldos
          cmp       cx,0B2D2h           ;verify that ROM-DOS really set things
          jne       ChkDR               ;not ROM-DOS
          mov       _gnOSFlags,(0DBh shl 3) ;ROM-DOS - replace its 0FFh OEM ID
          jmp       short ChkDOS5
          ;
ChkDR:    mov       ax,DRCHECK          ;get DR-DOS inquiry flag
          calldos                       ;get DR-DOS version
          mov       bx,ax               ;copy result
          and       bx,0FBE0h           ;ignore low 5 bits
          cmp       bx,1060h            ;is it 106xh or 107xh?
           jne      ChkOS2              ;if not go look for OS/2 1.x
          bset      <bptr _gnOSFlags+1>,DOS_DR  ;set DR-DOS flag
          and       ax,01Fh             ;check real version
          mov       bx,offset DRVerTab  ;point to table
          ;
DRVTLoop: cmp       al,[bx]             ;check if we found the version
           jbe      DRGotVer            ;if so go on
          add       bx,3                ;skip to next table entry
          jmp       short DRVTLoop      ;and loop back
          ;
DRGotVer: mov       bx,1[bx]            ;get real version number
          jmp       short StoreVer      ;and go on
          ;
ChkOS2:   mov       bl,bptr _gchMajor   ;get current major version
          cmp       bl,10               ;is it OS/2 1.x?
          je        AdjOS2              ;if so skip DOS 5 check and do
                                        ;  OS/2 adjustment
ChkDOS5:  xor       bx,bx               ;clear BX for 3306 call
          mov       ax,3306h            ;check DOS 5, get real version
          calldos                       ;do the call
          or        bx,bx               ;was it DOS 5?
           jz       GVDone              ;if not go on
          cmp       __osmajor,al        ;now check for X-DOS 5.0 which
          jne       ChkD5BX             ;wrongly sets AX=1F03, BX=FF00
          cmp       __osminor,ah        ;(as if AH was 30h on input!)
          jne       ChkD5BX             ;did this mistake occur?
          and       _gnOSFlags,1111111111100111b   ;yes, set OEM ID=FC
          jmp       short GVDone
ChkD5BX:  cmp       bh,100              ;check also minor
          jae       GVDone
          cmp       bl,5                ;and major version
           jb       GVDone
          mov       bptr __osmajor,bl   ;save major version
          mov       bptr __osminor,bh   ;save minor version
          or        _gnOSFlags,dx       ;save flags
          ;
          cmp       bx,0A07h            ;version 7.10?
          jne       StoreVer            ;no, continue
          mov       ax,_gnOSFlags       ;yes
          and       ax,11111111000b     ;mask OEM ID
          cmp       ax,11111111000b     ;MS?
          jne       StoreVer            ;no, continue
          shln      dh,4,cl             ;shift HMA bit (4) to CF
          sbb       ax,ax               ;-1 if DOS in HMA, 0 otherwise
          mov       es,ax               ;search segment
          xor       di,di               ;start from its beginning
          mov       cx,8000h            ;search it all
          mov       ax,'ZL'             ;for the LZ-DOS ID
          repne     scasw               ;do the search; found?
          jne       StoreVer            ;no, continue
          and       _gnOSFlags,1111111111110111b   ;yes, set OEM ID=FE

StoreVer: mov       bptr _gchMajor,bl   ;save true major version
          mov       bptr _gchMinor,bh   ;save true minor version

AdjOS2:   mov       bh,10               ;get 10 for compare and divide
          cmp       bl,bh               ;is it OS/2?
           jb       GVDone              ;if not get out
          bset      <bptr _gnOSFlags+1>,DOS_OS2   ;set OS/2 flag
          mov       al,bl               ;copy version
          xor       ah,ah               ;clear high byte
          div       bh                  ;divide by 10
          mov       bptr _gchMajor,al   ;save as real version

;FIXME - switch 2.30 to 3.0, 2.40 to 4.0

GVDone:   popm      bp,di,si,bx         ;restore registers

          exit                          ;all done


          ;
          ; InstalledCheck - find out if a standard DOS TSR is installed
          ;
          ; On entry:
          ;         Arguments on stack:
          ;           int id
          ;
          ; On exit:
          ;         for SMARTDRV (id = 4A10h):
          ;           AX = BABEh if installed
          ;         else
          ;           AX = -1 if installed, 0 or 1 if not installed
          ;                 (0 if OK to install, 1 if not OK to install)
          ;         All other registers and interrupt state unchanged
          ;

          entry	InstalledCheck,argframe,far
          argW      id
          pushm     bx,cx,di,es
          mov       ax,0DADAh		; the MSCDEX check
          push      ax			; needs DADA in stack
          mov       ax,id
          xor       bx,bx		; for SMARTDRV check
          mov       cx,0EBABh		;	"	"
          xor       di,di		; for DOS Task Switcher check
          mov       es,di		;	"	"
          int       2Fh
          pop       bx
          cmp       ax,0BABEh		; generic TSR "installed" state?
          je        InstRet		; no, return AX without changes
          cbw				; yes, return 0, 1 or -1
InstRet:  popm      es,di,cx,bx
          exit


          ;
          ; PowerInstalled - find out if POWER.EXE is installed
          ;
          ; On entry:
          ;         No requirements
          ;
          ; On exit:
          ;         AX = 1 if POWER.EXE installed, 0 if not installed
          ;         All other registers and interrupt state unchanged
          ;

          entry	PowerInstalled,noframe,far
          push      bx
          mov       ax,5400h
          xor       bx,bx
          int       2Fh
          xor       ax,ax
          cmp       bx,'PM'	; POWER installed?
          jne       PwrRet	; no, return zero
          inc       ax		; yes, return flags
PwrRet:   pop       bx
          exit


          ;
          ; NetworkInstalled - find out if a network is installed
          ;
          ; On entry:
          ;         No requirements
          ;
          ; On exit:
          ;         AX destroyed
          ;         All other registers and interrupt state unchanged
          ;

          entry	NetworkInstalled,noframe,far
          push      bx
          mov       ax,0B800h
          xor       bx,bx
          int       2Fh		; BX = flags of installed network components
          or        al,al	; any network installed?
          jz        NetRet	; no, return zero
          xchg      ax,bx	; yes, return flags
NetRet:   pop       bx
          exit


          ;
          ; GetMachineName - get the machine name in the network
          ;
          ; On entry:
          ;         Arguments on stack:
          ;           char *machine
          ;
          ; On exit:
          ;         AX, DX destroyed
          ;         All other registers unchanged, interrupts on
          ;

          entry     GetMachineName,argframe,far
          argW      machine
          mov       dx,machine
          mov       ax,5E00h
          calldosW
          exit


          ;
          ; reset_disks - flush buffers & reset disk drives
          ;
          ; On entry:
          ;         No requirements
          ;
          ; On exit:
          ;         AX destroyed
          ;         All other registers and interrupt state unchanged
          ;
          ;

          entry	reset_disks,noframe,far

          mov       ah,0Dh
          calldosW
          exit


          ;
          ; SDFlush -- Flush SMARTDRV and compatible disk caches
          ;
          ; On entry:
          ;         No requirements
          ;
          ; On exit:
          ;         AX, CX, DX, ES destroyed
          ;         All other registers and interrupt state unchanged
          ;

          entry     SDFlush,noframe,far

        push    bx
	push	bp		;save register for SMARTDRV / NCACHE bug
	;
SDLoop:	mov	ax, 4A10h	;SMARTDRV code
	xor	bx, bx		;install check / status function
	int	2Fh		;is SMARTDRV there?
	cmp	ax, 0BABEh	;if so it returns BABE
	 jne	SDDone		;if not there get out
	or	cx, cx		;any dirty blocks?
	 jz	SDDone		;if not get out
	mov	ax, 4A10h	;SMARTDRV code
	mov	bx, 1		;commit function
	int	2Fh		;commit dirty blocks
	jmp	SDLoop		;loop (may not all commit on one call)
	;
SDDone:	pop	bp		;restore register for SMARTDRV / NCACHE bug
	pop	bx
          exit                  ;all done


	;==================================================================
	;
	;  OPERATING SYSTEM INTERFACE ROUTINES (files)
	;
	;==================================================================

          ;
          ; ForceDelete - Force a non-recoverable delete in OS/2 2.x
          ;
          ; On entry:
          ;         Arguments on stack:
          ;           char *Target
          ;
          ; On exit:
          ;         AX, BX, CX, DX destroyed
          ;         All other registers unchanged, interrupts on
          ;

          entry     ForceDelete,argframe,far

          argW      Target

          pushm     bx,di,si

          mov       ax,06400h
          mov       bx,0CBh		; ordinal
          mov       cx,0636Ch
          mov       dx,Target
          calldosW
          jc        FDError
          xor       ax,ax
FDError:
          popm      si,di,bx

          exit                          ; all done


          ;
          ; SetIOMode - get raw or cooked mode on file handle
          ;
          ; On entry:
          ;         Arguments on stack:
          ;           unsigned int fd:   file handle
          ;           int mode - 0 for binary, 1 for ASCII
          ;

          entry     SetIOMode,argframe,far

          argW      mode
          argW      fd

	  push	    bx
          mov       ax,04400h
          mov       bx,fd
          calldosW
          xor       dh,dh
          or        dl,020h		; binary (raw) mode
          mov       ax,04401h
          mov       bx,fd
          cmp       word ptr mode,0
          je        raw_mode
          and       dl,08Fh		; ASCII mode
raw_mode:
          calldosW
	  pop	    bx
          exit


          ;
          ; _dos_createEA - create a new file with Extended Attributes
          ;
          ; On entry:
          ;         int flag
          ;		if == 0, we're about to display the prompt
          ;		if == 1, we're about to accept keyboard input
          ;
          ; On exit:
          ;         AX, DX destroyed
          ;         All other registers unchanged, interrupts on
          ;

          entry     _dos_createEA,argframe,far

          Argw      eaop
          Argw      attribute
          Argw      outfile
          Argw      outname
          Argw      infile
          Argw      inname

          push      bx
          push      di
          push      si

          push      ds
          pop       es
          mov       di,eaop		; ES:DI = EAOPT structure
          mov       si,[inname]		; DS:SI = ASCIIZ filename
          mov       ax,05702h
          mov       bx,infile
          mov       cx,000Ch
          mov       dx,0004h		; get all EAs
          calldosW
          jc        ea_copy_error

          mov       si,[outname]	; DS:SI = ASCIIZ filename
          mov       ax,06C01h		; open file with EAs
          mov       bx,00100010b 	; read/write
          mov       cx,attribute
          mov       dx,00010010b
          calldosW
ea_copy_error:
          pop       si
          pop       di
          jc        ea_copy_bye

          mov       bx,outfile
          mov       [bx],ax		; save file handle
          xor       ax,ax
ea_copy_bye:
	  pop	    bx
          exit


          ;
          ; QueryIsConsole - test to see if Handle is the console (redir test)
          ;
          ; On exit:
          ;         AX = 0 if redirected, 1 if at the console
          ;         AX, DX destroyed
          ;         All other registers unchanged, interrupts on
          ;

          entry     QueryIsConsole,argframe,far

          argW      Handle

          push	    bx
          mov       ax,04400h		; IOCTL call for console status
          mov       bx,Handle
          calldosW
          mov       ax,0
          jc        not_console
          and       dx,083h
          cmp       dx,083h
          jne       not_console
          inc	    ax

not_console:
          pop	    bx
          exit
	

          ;
          ; CDIOctl - open or close CD tray or get drive status
          ;
          ; On entry:
          ;         Argument on stack using pascal calling convention:
          ;            int drivespec:   drive (0 - first, 1 - A, 2 - B, etc.)
          ;            int operation:   0: open, 5: close, 6: status, 8: size
          ;
          ; On exit:
          ;         DX:AX = -1 for invalid drive, status (operation = 6) or size
          ;         AX, DX destroyed
          ;         All other registers unchanged, interrupts on
          ;

          entry     CDIOctl,argframe,far

          ArgW      operation
          ArgW      drivespec

          push      bx
          mov       ax,1500h		; CD-ROM installation check, returns:
          xor       bx,bx		; BX = # of CD-ROM drive letters used
          int       2Fh			; CX = starting drive letter (0 = A:)
          dec       bx			; any CD-ROM drive letters?
          js        cdio_error		; no, error
          dec       word ptr drivespec	; yes, adjust (A=0, B=1, etc.)
          js        cdio_oper		; if it was 0, use current CX (1st drv)
          cmp       drivespec,cx	; drive argument too low?
          jb        cdio_error		; yes, error
          add       bx,cx		; no, get last drive letter
          cmp       drivespec,bx	; drive argument too high?
          ja        cdio_error		; yes, error
          mov       cx,drivespec	; no, use it
cdio_oper:
          mov       dx,operation
          mov       ReqHdr.function,dl
          mov       bl,IOCTL_OUTPUT
          mov       dh,1
          cmp       dl,STATUS_COMMAND	; status or size command?
          jb        cdio_doit		; no, do 1-byte output
          mov       bl,IOCTL_INPUT	; yes, do 5-byte input
          mov       dh,5
cdio_doit:
          mov       ReqHdr.command,bl
          mov       ReqHdr.bytes,dh
          mov       bx,offset ReqHdr
          loadseg   es,ds,dx
          mov       ReqHdr.d_segment,dx	; these 2 may get overwritten by driver
          mov       ReqHdr.d_offset,offset ReqHdr.function
          mov       al,10h
          int       2Fh			; send device driver request, CX: drive
          mov       ax,word ptr ReqHdr.cdstatus; return status, for operation 6
          mov       dx,word ptr ReqHdr.cdstatus+2    ; or size, for operation 8
          jmp       short cdio_exit
cdio_error:
          mov       ax,-1
          mov       dx,ax
cdio_exit:
          pop       bx
          exit


          ;
          ; GetDPB - get a far pointer to the DOS Drive Parameter Block
          ;
          ; On entry:
          ;         Argument on stack:
          ;           int drivespec:   logical drive (1 - A, 2 - B, etc.)
          ;
          ; On exit:
          ;         DX:AX = far pointer to the DPB for the specified drive
          ;         DX:AX = 0 on error (invalid or network drive)
          ;         All other registers unchanged, interrupts on
          ;
          entry     GetDPB,argframe,far
          ArgW      drivespec
          pushm     bx,ds
          mov       dx,drivespec
          mov       ah,32h		; get DPB for the specified drive
          int       21h
          xor       dx,dx		; prepare 0 to return in case of failure
          cmp       al,0		; successful?
          mov       ax,dx
          jne       DPBret		; no, invalid or network drive, return 0
          xchg      ax,bx		; yes
          mov       dx,ds		; move the DPB pointer to DX:AX
DPBret:   popm      ds,bx
          exit


          ;
          ; GetDrivePhysUnit - get physical (BIOS Int 13h) drive unit number
          ;
          ; On entry:
          ;         Argument on stack:
          ;           int drivespec:   drive to check (1 - A, 2 - B, etc.)
          ;
          ; On exit:
          ;         AX = physical unit # (0,1,...: FDD; 80h,81h,...: HDD)
          ;         AX = -1 on error (call not supported or no physical unit)
          ;         All other registers unchanged, interrupts on
          ;

          entry     GetDrivePhysUnit,argframe,far
          ArgW      drivespec
          pushm     bx,di,ds
          mov       bx,drivespec
          dec       bx			; logical drive: 0 = A:, 1 = B:. etc.
          xor       di,di
          mov       ds,di
          mov       ax,803h		; get Drive Data Table list (DOS 4.0+)
          int       2Fh
          mov       ax,ds
          or        ax,di		; valid address?
          jz        PUnitErr		; no, return -1
          xor       ax,ax		; yes, prepare unit number MSW
NextDDT:  mov       al,[di+4]		; get physical unit number
          cmp       bl,[di+5]		; same logical drive?
          je        PUnitRet		; yes, return unit number in AX
          lds       di,[di]		; no, load next DDT address
          cmp       di,-1		; last DDT?
          jne       NextDDT		; no, continue
PUnitErr: mov       ax,-1		; yes, return -1
PUnitRet: popm      ds,di,bx
          exit


	;==================================================================
	;
	;  HARDWARE / BIOS INTERFACE ROUTINES (except video)
	;
	;==================================================================

          ;
          ; GetHDDParams - get physical (BIOS Int 13h) HDD parameters
          ;
          ; On entry:
          ;         Argument on stack:
          ;           int drv:   physical unit # (80h,81h,...) for Int 13h
          ;           LBA *buf:  LBA buffer
          ;
          ; On exit:
          ;         All registers unchanged, interrupts on
          ;

          entry     GetHDDParams,argframe,far
          argW      buf
          argW      drv
          pushm     ax,dx,si
          mov       ah,48h		; Get drive parameters (LBA BIOS)
          mov       dx,drv
          mov       si,buf
          int       13h
          popm      si,dx,ax
          exit


          ;
          ; QueryMouseReady - test for mouse driver
          ;
          ; On exit:
          ;         AX = 0 if no mouse, 1 if present
          ;

          entry     _QueryMouseReady,noframe,far

          pushm     bx,es
          mov       al,33h              ; get mouse interrupt number
          calldos   GETINT              ; get interrupt vector in ES:BX
          mov       ax,es
          pop       es
          or        ax,bx		; Int 33h vector zero?
          jz        no_mouse		; if so, don't call it!
          mov       ax,015h
          xor       bx,bx
          int       033h		; if mouse loaded, BX != 0
          xor       ax,ax
          or        bx,bx
          jz        no_mouse
          mov       ax,1
no_mouse:
          pop       bx
          exit


          ;
          ; QueryDriveRemovable - test if specified drive is removable
          ;
          ; On entry:
          ;         Argument on stack:
          ;           int drivespec:   drive to check (1 - A, 2 - B, etc.)
          ;
          ; On exit:
          ;         AX, DX destroyed
          ;         All other registers unchanged, interrupts on
          ;

          entry     QueryDriveRemovable,argframe,far

          ArgW      drivespec

          push      bx
          mov       bx,drivespec

          mov       ax,04408h		; IOCTL check if block dev is removable
          calldosW
          jc        is_fixed		; error - assume fixed
          xor       al,1		; DOS returns AX=0: removable, 1: fixed
          jmp       short dr_bye
is_fixed:
          xor       ax,ax		; clear removable flag
dr_bye:
          pop       bx
          exit


          ;
          ; QueryDriveReady - test if specified drive is ready
          ;
          ; On entry:
          ;         Argument on stack:
          ;           int drivespec:   drive to check (1 - A, 2 - B, etc.)
          ;
          ; On exit:
          ;         AX, DX destroyed
          ;         All other registers unchanged, interrupts on
          ;

          entry     QueryDriveReady,argframe,far

          ArgW      drivespec

          push      drivespec
          call      QueryIsCDROM
          or        ax, ax		; CD-ROM?
          jz        ready_nocd		; no, continue
          push      drivespec		; yes, get inverted status
          mov       ax, STATUS_COMMAND	; bit 11, "no disc in drive"
          push      ax
          call      CDIOctl
          push      drivespec		; second call - the first may miss
          mov       ax, STATUS_COMMAND	; a CD if no other calls have been
          push      ax			; made since it has been inserted!
          call      CDIOctl
          not       ax
          push      cx
          mov       cl, 11
          shr       ax, cl
          pop       cx
          and       ax, 1
          jmp       short ready_ret

ready_nocd:
          push      bx
          mov       ax, 2
          push      ax
          call      DosError		; disable error popup

          mov       dx, drivespec
          xor       cx, cx
          calldos   DSKFREE		;get AX=sectors/cluster,CX=bytes/sector
          jcxz      ready_error		;PTS-DOS doesn't set AX=-1 if not ready
          cmp       ax, 0FFFFh
          jne       no_error

ready_error:
          xor       ax, ax
          jmp       short not_ready
no_error:
          mov       ax, 1
not_ready:
          push      ax			; save result (!= 0 if successful)

          mov       ax, 1
          push      ax
          call      DosError		; enable error popup

          pop       ax
          pop       bx
ready_ret:
          exit


          ;
          ; QueryDriveExists - test if specified drive exists
          ;
          ; On entry:
          ;         Argument on stack:
          ;           int drivespec:   drive to check (1 - A, 2 - B, etc.)
          ;
          ; On exit:
          ;         AX, DX destroyed
          ;         All other registers unchanged, interrupts on
          ;

          entry     QueryDriveExists,argframe,far

          argW      drivespec

          push      bx
          mov       bx,drivespec
          xor       dx,dx		; clear attribute word
          mov       ax,04409h		; IOCTL check if block dev is remote
          calldosW
          jnc       d_e_exists		; no error - assume drive exists
          mov       ah,0
          cmp       __osmajor,3
          jb        d_e_invchk		; if DOS 2.x, use the returned AL
          pushm     cx,dx,si,di,bp,ds,es; save registers destroyed by DOS call
          xor       bx,bx		; PTS-DOS may have returned invalid AL
          calldos   GETERR              ; so get extended error code
          popm      es,ds,bp,di,si,dx,cx
d_e_invchk:
          cmp       ax,15h
          je        d_e_exists		; if "drive not ready" assume it exists
          xor       ax,ax		; clear exists flag
          jmp       short d_e_bye
d_e_exists:
          mov       ax,1		; set exists flag
d_e_bye:
          pop       bx
          exit


          ;
          ; QueryDriveRemote - test if specified drive is remote
          ;
          ; On entry:
          ;         Argument on stack:
          ;           int drivespec:   drive to check (1 - A, 2 - B, etc.)
          ;
          ; On exit:
          ;         AX = -1 for invalid drive, 0 for local, 1 for remote
          ;         DX destroyed
          ;         All other registers unchanged, interrupts on
          ;

          entry     QueryDriveRemote,argframe,far

          ArgW      drivespec

          push      drivespec
          call      QueryDriveExists

          or        ax,ax		; if AX = 0, drive doesn't exist
          jz        is_remote

          and       dx,1000h		; test remote bit
          jnz       is_remote
          xor       ax,ax
is_remote:

          exit

          ;
          ; QueryIsCDROM - test if specified drive is a CD-ROM
          ;
          ; On entry:
          ;         Argument on stack:
          ;           int drivespec:   drive to check (1 - A, 2 - B, etc.)
          ;
          ; On exit:
          ;         AX = 1 if drive is CD-/DVD-ROM, 0 otherwise
          ;         BX, DX destroyed
          ;         All other registers unchanged, interrupts on
          ;

          entry     QueryIsCDROM,argframe,far

          ArgW      drive

          push      bx
	mov	cx,drive
	dec	cx			;0 = A:, 1 = B:, etc.
	mov	ax,0150Bh		;MSCDEX call
	xor	bx,bx
	int	2Fh			;returns AX=0 if drive not supported
	cmp	bx,0ADADh
	je	is_cdrom
	xor	ax,ax
is_cdrom:
	or	ax,ax
	jz	cd_bye
	mov	ax,1
cd_bye:
          pop       bx
          exit

          ;
          ; QueryPrinterReady  - test if the specified printer is ready
          ;
          ; On entry:
          ;         int lpt_num = printer number (1-n)
          ;
          ; On exit:
          ;         AX, DX destroyed
          ;         All other registers unchanged, interrupts on
          ;

          entry     QueryPrinterReady,argframe,far

          argW      lpt_num

          push      bx
          mov	ah,02h
          mov	dx,lpt_num
          int	17h			; get printer status
          cmp	ah,090h
          mov	bl,ah
          mov	ax,1
          cmp	bl,090h		; if printer ready ah == 0x90
          je	lpt_return
          xor	al,al
lpt_return:
          pop       bx
          exit

          ;
          ; QuerySerialReady  - test if the specified serial port is ready
          ;
          ; On entry:
          ;         int com_num = serial port number (1-n)
          ;
          ; On exit:
          ;         AX = 1: ready, 0: not ready
          ;         DX destroyed, all other registers unchanged, interrupts on
          ;

          entry     QuerySerialReady,argframe,far
          argW      com_num
          mov       ax,300h
          mov       dx,com_num
          int       14h			; get serial port status
          cmp       ax,-1		; disabled ports return -1 in W9x
          jne       com_dsr		; if not so, continue normally
          inc       ax			; clear AX if port disabled in W9x
com_dsr:  and       ax,100000b		; mask DSR bit
          jz        com_ret		; return 0 if inactive
          mov       al,1
com_ret:  exit

          ;
          ; GetSBDSPver - get Sound Blaster DSP version (see SBHWPG.PDF)
          ;
          ; On exit:
          ;    AX = DSP version, or 0 if no Sound Blaster present
          ;    All other registers unchanged, interrupts on
          ;
          extrn     @SysWait:far
          entry	GetSBDSPver,noframe,far
          pushm     bx,cx,dx
          mov       ax,1
          mov       dx,226h		; DSP reset port
          out       dx,al
          xor       cx,cx
          loop      $			; must wait at least 3 microseconds
          dec       ax
          out       dx,al
          push      dx
          mov       al,5
          cwd
          mov       bx,2		; 5 ms delay, much longer than 0.1 ms
          call      @SysWait
          pop       dx
          add       dl,8		; read buffer status port (22Eh)
          in        al,dx
          and       ax,10000000b	; any data to read?
          jz        SBDSPretn		; no, return 0 (no SB found)
          sub       dl,4		; yes, read DSP data port (22Ah)
          in        al,dx
          cmp       al,0AAh		; OK?
          mov       al,0		; (assume no)
          jne       SBDSPretn		; no SB found - return 0
          add       dl,2		; a SB was found
SBDSPbsy1:in        al,dx		; read DSP write buffer status (22Ch)
          or        al,al		; can we write to the DSP?
          js        SBDSPbsy1		; not yet, try again
          mov       al,0E1h		; yes, send the Get DSP Version command
          out       dx,al
          add       dl,2		; read buffer status port (22Eh)
SBDSPbsy2:in        al,dx
          or        al,al		; any data to read?
          jns       SBDSPbsy2		; not yet, try again
          sub       dl,4		; read data port (22Ah)
          in        al,dx		; read the version MSB
          mov       ah,al		; save it
          add       dl,4		; read buffer status port (22Eh)
SBDSPbsy3:in        al,dx		; repeat the procedure for the LSB
          or        al,al
          jns       SBDSPbsy3
          sub       dl,4
          in        al,dx		; read the version LSB
SBDSPretn:popm      dx,cx,bx
          exit				; return the DSP version in AX

          ;
          ; InV86mode - return 1 if CPU is in V86 mode, else 0 (386+ only)
          ;
          ; On exit:
          ;    AX = 0: CPU is in real mode
          ;         1: CPU is in V86 mode
          ;         All other registers unchanged, interrupts on
          ;
          entry     InV86mode,noframe,far
          .286
          smsw      ax			; get LSW of CR0
          .8086
          and       ax,1		; isolate bit 0 (PE)
          exit

          ;
          ; GET_CPU - get cpu type
          ;
          ; On exit:
          ;    AX = 86 if an 8088/8086
          ;        186 if an 80186/80188
          ;        200 for a NEC V20/V30
          ;        286 if an 80286
          ;        386 if an 80386
          ;        486 if an 80486
          ;        586 if a Pentium
          ;        686 if a Pentium Pro, II or III
          ;        786 if a Pentium 4 or later
          ;    CX destroyed, all other registers unchanged, interrupts on
          ;

          entry     _get_cpu,noframe,far

CPUID     MACRO
          db	0Fh		; hardcoded opcode for CPUID instruction
          db	0A2h
ENDM

          push      bx

          pushf				; put flags register onto the stack
          xor       ax,ax
          push      ax			; put AX on the stack
          popf				; bring it back in flags
          pushf				; try to set bits 12 thru 15 to a zero
          pop       ax			; get back Ur flags word in AX
          and       ax,0F000h		; if bits 12 thru 15 are set, then it's
          cmp       ax,0F000h		;   an 808x, 8018x or NEC V20/V30
          je        old_chip

          mov       ax,07000h		; set FLAG bits 12 thru 14 - NT, IOPL
          push      ax			; put it onto the stack
          popf				;   and try to put 07000H into flags
          pushf				; push flags again
          pop       ax			;   and bring back AX for a compare
          and       ax,07000h		; if bits 12 thru 14 are set
          mov       ax,286		;   then it's an 80386
          jz        cpu_bye

try_386:
          .386

          mov       ecx,esp		; save ESP
          mov       SaveESP,ecx
          and       esp,0FFFFFFFCh	; zero lower 2 bits to avoid
                                        ;   AC fault on 486
          pushfd			; save EFLAGS
          pushfd			; push EFLAGS
          pop       eax			; EAX = EFLAGS
          mov       edx,eax		; EDX = EFLAGS
          xor       eax,40000h		; toggle AC bit(bit 18) in EFLAGS
          push      eax			; push new value
          popfd				; put it in EFLAGS
          pushfd			; push EFLAGS
          pop       eax			; EAX = EFLAGS
          and       eax,40000h		; isolate bit 18 in EAX
          and       edx,40000h		; isolate bit 18 in EDX
          cmp       eax,edx		; are EAX and EDX equal?
          je        short got_386	;   yes - it's a 386

; it's a 486 or Pentium

          pushfd			; get current flags
          pop       eax
          mov       edx,eax
          xor       eax,EF_ID		; try to toggle ID bit
          push      eax
          popfd
          pushfd			; get new EFLAGS
          pop       eax

          and       eax,EF_ID		; if we can't toggle ID, it's a 486
          and       edx,EF_ID
          cmp       eax,edx
          je        short got_486

          mov       eax,1
          CPUID
          and       ax,0F00h		; mask everything except family field
          cmp       ax,0500h		; get Family field (5=Pentium)
          jb        got_486

          cmp       ax, 600h
          jb        got_586
          je        got_686
          mov       ax,786
          jmp       short restore_486
got_686:
          mov       ax,686
          jmp       short restore_486
got_586:
          mov       ax,586
          jmp       short restore_486

got_486:
          mov       ax,486
          jmp       short restore_486

got_386:
          mov       ax,386		; it's an 80386

restore_486:
          popfd                         ; restore EFLAGS
          mov       ecx,SaveESP
          mov       esp,ecx		; restore ESP
          jmp       short cpu_bye

          .8086

old_chip:
          mov       ax,0FFFFh		; load up AX
          mov       cl,33		; this will shift 33 times if it's an
					;   8088/8086, or once if 80188/80186
          shl       ax,cl		; Shifting 33 should zero all bits
          mov       ax,186
          jnz       short cpu_bye

not_186:
          xor       al,al		; set ZF
          mov       al,40h		; mul on NEC does NOT affect ZF
          mul       al			;   but on 8086/88, ZF gets thrown
          jz        got_nec
          mov       ax,86		; it's an 8088/8086
          jmp       short cpu_bye

got_nec:
          mov       ax,200		; it's a NEC V20/V30
cpu_bye:
          popf
          pop       bx

          exit

if 0
          ;
          ; GET_NDP - get ndp type
          ; Must assemble with the /R option set in MASM
          ;
          ; On exit:
          ;    AX = 0 if no ndp
          ;        87 if an 8087
          ;        287 if an 80287
          ;        387 if an 80387
          ;
          ;  All other registers unchanged, interrupts on
          ;

          entry     _get_ndp,noframe,far

          fninit			; try to initialize NDP
          mov       byte ptr StatusNDP+1,0  ; clear memory byte
          fnstcw    StatusNDP		; put control word in mem
          mov       ah,byte ptr StatusNDP+1
          cmp       ah,03h		; if AH is 3, NDP is present
          je        chk_87		;   found NDP
          xor       ax,ax		; clear AX to show no NDP
          jmp       short bye_ndp

; 'got an 8087 ?
chk_87:
          and       StatusNDP,NOT 0080h	; turn ON interrupts (IEM=0)
          fldcw     StatusNDP		; load control word
          fdisi				; turn OFF interrupts (IEM=1)
          fstcw     StatusNDP		; store control word
          test      StatusNDP,0080h	; if IEM=1, NDP is 8087
          jz        chk287
          mov       ax,087
          jmp       short bye_ndp
chk287:
          finit				; set default infinity mode
          fld1				; make infinity
          fldz				;     by dividing
          fdiv				;         1 by zero !!
          fld       st			; now make negative infinity
          fchs
          fcompp			; compare two infinities
          fstsw     StatusNDP		; if, for 8087 or 80287
          fwait				; 'til status word is put away
          mov       ax,StatusNDP	; get control word
          sahf				; put AH into flags
          mov       ax,0287
          jz        short bye_ndp
          mov       ax,0387		; must be a 80387
bye_ndp:
          exit
endif

          ;
          ; get_cpu_brand - get the CPU "brand string", if supported
          ;
          ; On entry:
          ;         Argument on stack:
          ;           char *brandstr:   48-byte brand string buffer
          ;
          ; On exit:
          ;         AX = 0 if this feature is supported, else a non-zero value
          ;         All other registers unchanged, interrupts on
          ;         brandstr buffer filled with the CPU brand string if AX = 0
          ;         Note: Brand string is zero-terminated and right-justified.
          ;               It can be left-padded with blank (space) characters.

          entry     get_cpu_brand,argframe,far
          argW      brandstr
          .386
          push      eax
          mov       eax,80000000h	; extended function information
          CPUID
          or        eax,eax		; extended functions supported?
          jns       nobrstr		; no, return
          sub       ax,4		; yes, Fn 4 (brand string) supported?
          jae       getbrst		; yes, continue
nobrstr:  pop       eax			; no, return -1
          mov       ax,-1
          jmp       short brstret
getbrst:  pushm     ebx,ecx,edx,si,di,es
          loadseg   es,ds
          mov       di,brandstr
          mov       si,2
          cld
next16b:  mov       eax,80000004h	; do CPUID with AX=0x8000000|2|3|4
          sub       ax,si
          CPUID
          stosd				; save returned string to brandstr
          xchg      eax,ebx		;   (4 x 4 x 3 = 48 bytes total)
          stosd
          xchg      eax,ecx
          stosd
          xchg      eax,edx
          stosd
          dec       si
          jns       next16b		; do it 3 times until SI becomes -1
          popm      es,di,si,edx,ecx,ebx,eax
          .8086
          xor       ax,ax		; return 0 (success)
brstret:  exit

          ;
          ; QuerySystemRAM - get the amount of installed [base] RAM
          ;
          ; On exit:
          ;         AX = kilobytes of contiguous memory starting at address 0
          ;         All other registers unchanged, interrupts on
          ;
          entry     QuerySystemRAM,noframe,far
          int       12h			; get RAM size from BIOS
          exit

          ;
          ; GET_EXPANDED - get amount of expanded memory
          ;
          ; On entry:
          ;         Argument on stack:
          ;           unsigned int *emsfree:   pointer to free ems page count
          ;
          ; On exit:
          ;         AX = pages (16K) of expanded memory
          ;         DX destroyed
          ;         All other registers unchanged, interrupts on
          ;

          entry     get_expanded,argframe,far

          argW      emsfree             ;one argument, point to free page cnt

          pushm     bx,si,di            ;save registers
          mov       al,67h              ;get EMS interrupt number
          calldos   GETINT              ;get interrupt vector in es:bx
          lea       si,emm_name         ;point to driver name for comparison
          mov       di,10               ;address of name field in dev header
          mov       cx,8                ;comparison length
          cld                           ;move forward
          repe      cmpsb               ;does driver name match?
          jne       noems               ;no, no EMS
          callems   UPCNT               ;get ems page counts
          or        ah,ah               ;any error?
          jnz       noems               ;if error, return no ems
          mov       si,emsfree          ;get address of free page count
          mov       [si],bx             ;store free page count
          mov       ax,dx               ;get total page count
          jmp       short gxret         ;and return
noems:
          xor       ax,ax               ;clear total page count
gxret:
          popm      di,si,bx            ;restore registers
          exit

          ;
          ; GET_EXTENDED - get amount of extended memory
          ;
          ; On exit:
          ;         DX:AX = # of 1K blocks of extended memory
          ;         All other registers unchanged, interrupts on
          ;

          entry     _get_extended,noframe,far
          pushm     bx,cx
          cmp       __osmajor,10
          cmc
          jb        got_88
          mov       ax,_gnCPU
          cmp       ax,286		; is it an 86/88/186/188/V20/V30?
          jb        got_88		;   yup, so no extended mem
          cmp       ax,386		; is it at least a 386?
          jb        got_286		; if not, use the CMOS
          mov       ax,0E801h		; try the "get memory size" request
          mov       cx,ax
          xor       bx,bx		; load registers with invalid values
          xor       dx,dx
          int       15h			; CF set even on success in Win9x, so
          push      ax			; it can't be used to check for error
          or        ax,bx
          pop       ax
          jnz       gotE801		; AX = BX = 0 in Win9x
          xchg      ax,cx		; if so, use CX, DX instead of AX, BX
          mov       bx,dx
gotE801:  or        bx,bx		; any memory above 16 MB?
          jz        got_286		; if not, use the CMOS
          cmp       ax,15*1024		; any memory "hole" below 16 MB?
          ja        got_286		; ERROR if > 15 MB! Try the CMOS
          xchg      ax,bx		; now BX = KBytes below 16 MB
          mov       dx,64		; convert the 64K units in AX
          mul       dx			;   into 1K units in DX:AX
          add       ax,bx		; add the KBytes below 16 MB
          adc       dx,0
          jnc       got_ext
got_286:  mov       al,31h		; get extended memory size (286+)
          out       70h,al		; (Int 15h function 88h is often
          jmp       $+2			; hooked by memory managers and
          in        al,71h		; returns 0, so don't use it)
          jmp       $+2
          mov       ah,al
          mov       al,30h		; 30:LSB, 31:MSB; 70:reg, 71:data
          out       70h,al		; get the size in KB from where
          jmp       $+2			; BIOS itself gets it - CMOS RAM
          in        al,71h
          clc
got_88:   mov       dx,0		; not a XOR to avoid clearing CF
          jnc       got_ext
          xor       ax,ax		; no extended memory
got_ext:  popm      cx,bx
          exit

          ;
          ; GET_XMS - get amount of XMS memory
          ;
          ; On entry:
          ;       int *hma_status
          ;
          ; On exit:
          ;         DX:AX = # of 1K blocks of extended memory
          ;         ES destroyed
          ;         All other registers unchanged, interrupts on
          ;

          entry     get_xms,varframe,far

          argW      hma_state
          varD      XMSControl		; address of XMS driver
          varend

          push      bx
          cmp       _gnCPU,286		; is it an 86/88/186/188/V20/V30?
          jb        not_286

          mov       ax,XMSTEST
          int       2Fh
          cmp       al,XMSFLAG
          je        got_xms
not_286:
          xor       ax,ax
	  xor	    dx,dx
          jmp       short no_xms
got_xms:
          mov       ax,XMSADDR
          int       2Fh
          mov       word ptr [XMSControl],bx
          mov       word ptr [XMSControl+2],es

          mov       ah,1		; get HMA availability
          mov       dx,0FFFFh
          call      dword ptr [XMSControl]

          mov       ah,al
          mov       al,bl		; save error code
          or        ah,ah
          jz        no_hma

          push      ax
          mov       ah,2		; release the HMA
          call      dword ptr [XMSControl]
          pop       ax
no_hma:
          mov       bx,hma_state
          mov       [bx],ax

	  cmp	    _gnCPU,386		; is it at least a 80386?
	  jb	    no_v3		; no, use the old method (up to 64 MB)

	  mov	    ah,0		; query XMS version
	  call	    dword ptr [XMSControl]
	  cmp	    ah,3		; 3.0+ ?
	  jb	    no_v3		; no, use the old method

	  mov	    ah,88h		; query free extended memory (XMS 3.0+)
	  call	    dword ptr [XMSControl]
	  .386
	  push	    eax
	  .8086
	  pop	    ax			; copy total free to DX:AX
	  pop	    dx
	  jmp	    short no_xms
no_v3:
          mov       ah,8		; query free extended memory
          call      dword ptr [XMSControl]
          mov       ax,dx               ; copy total free to AX
	  xor	    dx,dx
no_xms:
          pop       bx
          exit

          ;
          ; BIOS_SHIFTSTATE - check Shift/Alt/Ctrl/*Lock/Insert/SysReq status
          ;
          ; On exit:
          ;         AX destroyed
          ;         All other registers unchanged, interrupts on
          ;

          entry     _bios_shiftstate,noframe,far

          mov       ah,2
          int       16h
          mov       ah,12h
          int       16h ; PC/XT/AT BIOS will exit with AH = 10h and AL intact
          exit

          ;
          ; BIOS_KBHIT - check if a key is ready from the BIOS (for LIST /S)
          ;
          ; On exit:
          ;         AX, DX destroyed
          ;         All other registers unchanged, interrupts on
          ;

          entry     _bios_kbhit,noframe,far

          mov       ah,1
          int       16h
          mov       ax,1
          jnz       kbh_done
          dec       ax
kbh_done:
          exit                          ; all done

          ;
          ; BIOS_KEY - get key from the BIOS (for LIST /S)
          ;
          ; On exit:
          ;         AX, DX destroyed
          ;         All other registers unchanged, interrupts on
          ;

          entry     _bios_key,noframe,far

          push      bx
          xor       ax,ax
          int       16h
          mov       bh,ah
          xor       ah,ah
          or        al,al
          jnz       not_special
          mov       al,bh		; set extended scan code
          add       ax,0100h
not_special:
          pop       bx
          exit                          ; all done

          ;
          ; BIOS_KEYSTROKE - check for keystroke and return its codes if ready
          ;
          ; On exit:
          ;         AX = BIOS scan code (AH) and ASCII code (AL) if key pressed
          ;              -1 if no key pressed
          ;         All other registers unchanged, interrupts on
          ;

          entry     _bios_keystroke,noframe,far
          mov       ah,1
          int       16h			; is a keystroke available in buffer?
          jnz       key_ret		; yes, return its scan and ASCII code
          mov       ax,1680h		; no, give up Windows timeslice
          int       2Fh
          cmp       al,80h
          jne       key_none
          int       28h			; 2Fh 1680h not supported; call DOS idle
key_none: mov       ax,-1		; return FFFF which is an impossible
key_ret:  exit				; scan code / ASCII code combination

          ;
          ; DosBeep - beep the speaker with the specified tone & duration
          ;
          ; On entry:
          ;       int frequency
          ;       int duration (in 1/18th second increments)
          ;
          ; On exit:
          ;         AX, BX, CX, DX, ES destroyed
          ;         All other registers unchanged, interrupts on
          ;

          entry     DosBeep,argframe,far

          argW      duration            ; length of time (in 1/18th increments)
          argW      freq                ; frequency (in Hz)

          push      bx
          sti
          xor       bx,bx		; clear result flag
          mov       ax, 040h		; point ES to ROM BIOS data area
          mov       es, ax
          mov       cx,freq
          cmp       cx,20		; anything less than 20 Hz creates
          jb        beep_get_time	;   a divide overflow

          mov       dx,012h
          mov       ax,034DCh
          div       cx
          push      ax
          mov       al, 10110110b	; select 8253
          out       43h, al
          jmp       $+2
          pop       ax
          out       42h, al		; low byte of divisor
          jmp       $+2
          xchg      ah, al
          out       42h, al		; high byte of divisor
          jmp       $+2

; Wait for desired duration by monitoring time-of-day 18 Hz clock
          in        al, 61h		; get current value of control bits
          jmp       $+2
          or        al, 3
          out       61h, al		; turn speaker on

beep_get_time:
          mov       dx, es:[06Ch]       ; save original tick count
beep_loop:
          sti		   	; because programs keep doing CLIs!!
          cmp       word ptr duration,1 ; if only waiting 1 tick, don't check
          je        no_char		;   for a key

          mov       ah,0Fh		; this silly kludge is to work around
          int       10h			;   a bug in the OS/2 2.0 DOS box

          mov       ah,01h		; check for ^C or ^BREAK
          int       16h
          jz        no_char
          or        ax,ax
          jz        beep_ctrlc		; ^BREAK waiting
          cmp       al,3		; ^C waiting?
          jne       no_char
beep_ctrlc:
          mov       bx,3
          jmp       short beep_off

no_char:
	xor	bx, bx		; force 0 return if we are done
          mov       ax, es:[06Ch]
          sub       ax, dx              ; get time elapsed
          cmp       ax,duration         ; now check duration
          jae       beep_off            ; loop if not there yet

          cmp       word ptr freq,20	; check if DELAY'ing
          jae       beep_loop
          mov       ax,1680h
          int       2Fh			; give up Win / OS/2 timeslice
          cmp       al,80h
          jne       beep_loop
          int       28h			; 2Fh 1680h not supported; call DOS idle
          jmp       short beep_loop

beep_off:
          cmp       cx,20
          jb        beep_bye

          in        al,61h
          jmp       $+2
          and       al,11111100b	; turn speaker off
          out       61h, al

beep_bye:
          mov       ax,bx
          pop       bx
          exit
	
	;==================================================================
	;
	;  HARDWARE / BIOS INTERFACE ROUTINES (video)
	;
	;==================================================================

          ;
          ; GetDDCstring - get DDC (Display Data Channel)
          ;                   EDID (Enhanced Display Identification Data) string
          ;
          ; On entry:
          ;       int what (1 = manufacturer, 2 = model, 3 = serial number)
          ;       char *buf (128-byte buffer to store the EDID record)
          ;
          ; On exit:
          ;       AX = pointer to the desired string in buf, if any; else 0
          ;         All other registers unchanged, interrupts on
          ;

          entry     GetDDCstring,argframe,far
          argW      buf
          argW      what
          pushm     bx,cx,dx,si,di,es
          mov       di,buf
          loadseg   es,ds
          mov       cx,64
          xor       ax,ax
          rep       stosw		; clear buffer
          mov       di,buf
          xchg      ax,dx		; clear DX (CX already 0)
          mov       ax,4F15h
          mov       bl,1
          int       10h			; try to read the EDID from the BIOS
          cmp       ax,4Fh		; function supported and successful?
DDCzero:  mov       ax,0		; (return 0 if not)
          jne       DDCret		; no, exit
          mov       bx,what		; yes, check requested string
          dec       bx			; 3-letter manufacturer ID?
          jnz       DDCmodel		; no, continue
          mov       di,buf		; yes -
          mov       dx,[di+8]		; load its word
          xchg      dh,dl		; convert to big to little endian
          mov       cx,4		; 3 letters + 1 terminating null
DDCmanuf: mov       al,dh		; get next 5 MS bits
          shr       al,1		; move bits 14..10
          shr       al,1		;   to bits 12..08
          and       al,11111b		; mask them
          jz        DDCstr1		; leave a 0 the 3rd time
          add       al,'@'		; convert to ASCII
DDCstr1:  stosb				; store to the string
          push      cx			; shift bits 9..5 to 14..10
          shln      dx,5,cl		;	 and 4..0 to 09..05
          pop       cx			; loop until all letters plus
          loop      DDCmanuf		; a null terminator are stored
          mov       ax,buf		; return a pointer to the string
          jmp       short DDCret
DDCmodel: dec       bx			; model?
          mov       dx,0FC00h		; (load its text ID)
          jz        DDCtext		; yes, find it
          dec       bx			; no, serial number?
          mov       dx,0FF00h		; (load its text ID)
          jnz       DDCzero		; no, return 0
DDCtext:  mov       si,buf		; yes -
          add       si,26h		; 1st text ID address minus 10h
          mov       cx,4		; there are 4 of them
DDCfind:  add       si,10h		; next address
          lodsw
          or        ax,ax		; leading zeros?
          jne       DDCnext		; no, try next address
          cmp       [si],dx		; ID match?
          lea       ax,[si+3]		; (prepare pointer if so)
DDCnext:  loopne    DDCfind		; repeat until ID match
          jne       DDCzero		; if not found, return 0
DDCret:   popm      es,di,si,dx,cx,bx	; yes
          exit

          ;
          ; GetCurPos - get the cursor position
          ;
          ; On entry:
          ;       int *row
          ;       int *column
          ;
          ; On exit:
          ;         BX, DX, destroyed
          ;         All other registers unchanged, interrupts on
          ;

          entry     GetCurPos,argframe,far

          argW      gc_column
          argW      gc_row

          push      bx
          push      bp			; for PC1 bug
          mov       ah,0Fh
          int       10h			; get current page into BH
          mov       ah,03h
          int       10h
          pop       bp
          xor       ax,ax

          mov       al,dh
          mov       bx,gc_row
          mov       [bx],ax

          mov       al,dl
          mov       bx,gc_column
          mov       [bx],ax

          pop       bx
          exit

          ;
          ; SetCurPos - position the cursor
          ;
          ; On entry:
          ;         Arguments are:
          ;            int row
          ;            int column
          ;
          ; On exit:
          ;         AX, BX, DX destroyed
          ;         All other registers unchanged, interrupts on
          ;

          entry     SetCurPos,argframe,far

          argW      s_column
          argW      s_row

          push      bx
          mov       ah,0Fh		; get video mode (for page # in BH)
          push      bp			; save BP for PC1 BIOS bug
          int       10h
          pop       bp
          mov       dh,s_row
          mov       dl,s_column
          mov       ah,2
          int       10h
          pop       bx

          exit

          ;
          ; HideCursor - hide the cursor
          ;
          ; On exit:
          ;         All registers unchanged, interrupts on
          ;

          entry     HideCursor,noframe,far
          pushm     ax,cx,bp		; save BP for PC1 BIOS bug
          mov       ax,100h		; set cursor shape
          mov       cx,0FFFFh		; set start=end=FF to make it invisible
          int       10h
          popm      bp,cx,ax
          exit

          ;
          ; QuerySVGA - test to see if the video adapter is VESA SuperVGA
          ;
          ; On exit:
          ;         AX = 1: VESA SuperVGA BIOS present
          ;         AX = 0: no	"	"	"
          ;         BX destroyed, all other registers unchanged, interrupts on
          ;

          entry     QuerySVGA,noframe,far
          mov       ax,4F03h
          int       10h			; get current SuperVGA video mode in BX
          cmp       al,4Fh		; VESA SuperVGA BIOS present?
          mov       ax,0		; (assume no)
          jne       QSVGAret		; no, return 0
          inc       ax			; yes, return 1
QSVGAret: exit

          ;
          ; GETVIDEOMODE - get the video adapter and monitor type
          ;
          ; On exit:
          ;         Returns monitor type:
          ;            0 = monochrome
          ;            1 = cga
          ;            2 = ega w/mono monitor
          ;            3 = ega w/color monitor
          ;            4 = vga w/mono monitor
          ;            5 = vga w/color monitor
          ;
          ;         AX, BX, CX destroyed
          ;         All other registers unchanged, interrupts on
          ;

          entry     GetVideoMode,noframe,far

          push      bx
          push      bp			; save BP for old BIOS bug
          mov       ax,01A00h		; function 1A returns active adapter
          int       10h			; al will return as 1a if supported
          cmp       al,01ah
          jnz       no_dc
          cmp       bl,7		; monochrome VGA?
          jz        mono_vga
          cmp       bl,8		; color VGA?
          jz        color_vga
          mov       bl,4		; color EGA?
          jz        color_ega
          mov       bl,5		; monochrome EGA?
          jz        mono_ega

no_dc:
          mov       ah,12h		; Get information about the EGA
          mov       bl,10h
          int       10h
          cmp       bl,10h		; if it came back as 10h (no EGA),
          je        invalid		;   skip next test

          push      ds
          mov       ax,BIOS_RAM		; BIOS RAM area
          mov       ds,ax
          assume    ds:BIOS_RAM
          mov       bl,info		; get information byte
          pop       ds
          assume    ds:DGROUP

          test      bl,8		; is the EGA active (bit 3 == 0)?
          jnz       invalid
          cmp       bh,1		; monochrome monitor?
          jnz       color_ega
mono_ega:
          mov       ax,2
          jmp       short got_video
color_ega:
          mov       ax,3
          jmp       short got_video
mono_vga:
          mov       ax,4
          jmp       short got_video
color_vga:
          mov       ax,5
          jmp       short got_video

invalid:
          mov       ah,0Fh		; get video mode
          int       10h
          cmp       al,7
          mov       ax,1
          jnz       got_video
          xor       ax,ax
got_video:
          pop       bp
          pop       bx

          exit

          ;
          ; GetCellSize - get the cursor cell size
          ;
          ; On entry:
          ;       int cursor_type
          ;
          ; On exit:
          ;         AX, BX, DX, destroyed
          ;         All other registers unchanged, interrupts on
          ;

          entry     GetCellSize,argframe,far

          argW      cursor_type

          push      bx
          push      bp			; for PC1 bug
          mov       ah,0Fh
          int       10h			; get current page into BH
          push      ax
          mov       ah,03h
          int       10h
          pop       ax

          cmp       ch,020h		; if start line >= 32, reset it
          jae       is_invisible
          cmp       cl,5		; check for cursor in the middle of
          jb        is_invisible	;   the character cell
          cmp       ch,cl		; if start > end, something is wacky!
          jbe       not_invisible
is_invisible:
          mov       cx,0607h		; wake up!
not_invisible:
          cmp       al,7		; get video mode & test for BIOS bug on
          jne       not_mono		;   old PC - mono mode & cursor 6,7
          cmp       cx,0607h
          jne       not_mono
          mov       cx,0B0Ch		; set mono cursor to 11,12
not_mono:
          mov       ax,cursor_type	; set start & end rows by rounding
          mul       cl			;   off the value in CursO or CursI
          add       ax,49
          mov       dl,100
          div       dl
          mov       ch,cl
          sub       ch,al

          mov	    ax,cx		; return shape in AX
          pop       bp
          pop       bx

          exit

          ;
          ; GetAtt - get normal & inverse attributes at cursor position
          ;
          ; On entry:
          ;       int *normal
          ;       int *inverse
          ;
          ; On exit:
          ;         DX destroyed
          ;         All other registers unchanged, interrupts on
          ;

          entry     GetAtt,argframe,far

          argW      inverse
          argW      normal

          push      bx
          push      bp			; for PC1 bug
          mov       ah,0Fh
          int       10h			; get current page into BH
          mov       ah,8
          int       10h			; get character & attribute
          pop       bp

          mov       al,ah		; save normal
          xor       ah,ah
          or        al,al		; if black on black, change to white
          jnz       not_black
          mov       al,7
not_black:
          mov       bx,normal		; save normal attribute
          mov       [bx],ax

          and       ax,077h		; remove blink & intensity bits
          mov       cl,4		; ROR 4 to get inverse
          ror       al,cl

          mov       bx,inverse
          mov       [bx],ax		; save inverse attribute

          pop       bx
          exit

          ;
          ; Scroll - scroll or clear the window
          ;
          ; On entry:
          ;         Arguments are:
          ;            int upper row
          ;            int left column
          ;            int lower row
          ;            int right column
          ;            int scroll mode (-1 = down, 0 = clear window, 1 = up)
          ;            int attribute
          ;
          ; On exit:
          ;         AX, CX, DX destroyed
          ;         All other registers unchanged, interrupts on
          ;

          entry     Scroll,argframe,far

          argW      w_attrib
          argW      w_mode
          argW      w_rcol
          argW      w_lrow
          argW      w_lcol
          argW      w_urow

          push      bx
          mov       ch,w_urow
          mov       cl,w_lcol
          mov       dh,w_lrow
          mov       dl,w_rcol
          mov       bh,w_attrib

          cmp       word ptr w_mode,0
          jge       scroll_up
          mov       ax,0700h
          sub       ax,w_mode
          jmp       short scroll_it
scroll_up:
          mov       ah,06h
          mov       al,w_mode
scroll_it:
          pushm     bp, ds, es, di, si
          int       10h
          popm      si, di, es, ds, bp

          pop       bx
          exit

          ;
          ; SetBorderColor - set the border color
          ;
          ; On entry:
          ;         Arguments are:
          ;            int b_color
          ;
          ; On exit:
          ;         AX, DX destroyed
          ;         All other registers unchanged, interrupts on
          ;

          entry     SetBorderColor,argframe,far

          argW      b_color

          push      bx
          push      bp			; save BP for old PC BIOS bug
          mov       ah,0Bh		; first try it for CGA / MCGA
          mov       bx,b_color
          int       10h

          mov       ax,1001h		; EGA/VGA use different registers
          mov       bx,b_color
          mov       bh,bl
          int       10h
          pop       bp
          pop       bx

          exit

          ;
          ; SetLineColor - Rewrite the line attributes directly
          ;
          ; On entry:
          ;         Arguments are:
          ;            int row
          ;            int column
          ;            int length
          ;            int sa_attrib
          ;
          ; On exit:
          ;         AX, BX, CX, DX, ES destroyed
          ;         All other registers unchanged, interrupts on
          ;
          entry     SetLineColor,argframe,far

          argW      la_attrib
          argW      la_length
          argW      la_column
          argW      la_row

          pushm     bx,di

          mov       dh,la_row           ; get row
          mov       dl,la_column        ; get column
          call      initpchr            ; initialize the display routines

          cmp       BIOSOutput,0	; BIOS output requested?
          je        SLCNoBios
          call      GetCursor		; save current cursor location
          push      dx
SLCNoBios:
          mov       ax, la_attrib	; set attribute
          mov       cx, la_length
          call      chg_colors

          cmp       BIOSOutput,0	; BIOS output requested?
          je        SLCExit
          pop       dx
          call      SetCursor		; restore cursor location
SLCExit:
          popm      di,bx

          exit

          ;
          ; SetScrColor - Rewrite the screen attributes directly
          ;
          ; On entry:
          ;         Arguments are:
          ;            int sa_attrib
          ;
          ; On exit:
          ;         AX, BX, CX, DX, ES destroyed
          ;         All other registers unchanged, interrupts on
          ;
          entry     SetScrColor,argframe,far

          argW      sa_attrib
          argW      sa_Columns
          argW      sa_Rows

          pushm     bx, di

          xor       dx,dx		; start at 0,0
          call      initpchr		; screen buffer returned in ES:DI

          cmp       BIOSOutput,0	; BIOS output requested?
          je        SSCNoBios
          call      GetCursor		; save current cursor location
          push      dx

SSCBios:
          mov       ax,sa_attrib        ; set attribute
          mov       cx,sa_Columns
          call      chg_colors
          inc       BIOSRow
          mov       dx,sa_Rows
          cmp       dl,BIOSRow
          jae       SSCBIOS

          pop       dx
          call      SetCursor		; restore original cursor position

          jmp       short SSCExit

SSCNoBios:
          mov       cx,sa_Rows
          inc       cx
          mov       ax,sa_Columns
          mul       cx
          mov       cx,ax		; CX = total words in screen buffer

          mov       ax,sa_attrib        ; set attribute
          call      chg_colors
SSCExit:
          popm      di,bx

          exit

          ;
          ; ReadCellStr - read the characters and attributes at the 
          ;   specified location for the specified length
          ;
          ; On entry:
          ;         Arguments are:
          ;            char _far *rc_string
          ;            int rc_length
          ;            int rc_row
          ;            int rc_column
          ;
          ; On exit:
          ;         Returns nothing
          ;         AX, CX, DX destroyed
          ;         All other registers unchanged, interrupts on
          ;

          entry     ReadCellStr,argframe,far

          argW      rc_column
          argW      rc_row
          argW      rc_length
          argD      rc_string

          pushm     bx, ds, di, si

          mov       dh,rc_row
          mov       dl,rc_column
          call      initpchr

          mov       cx,rc_length
;          shr       cx,1                ; convert from byte to cell count

          cmp       BIOSOutput,0	; BIOS output requested?
          je        prloop

; BIOS code!

          les       di,rc_string

          push      bp			; save BP for old PC BIOS bug

          call      GetCursor		; save current cursor location
          push      dx
rbloop:
          mov       dh,BIOSRow
          mov       dl,BIOSCol
          call      SetCursor

          mov       ah,8
          mov       bh,BIOSPage
          int       10h			; get char & att at cursor

          mov       es:[di],ax
          add       di,2
          inc       BIOSCol
          loop      rbloop

          pop       dx
          call      SetCursor		; reset saved current cursor location

          pop       bp
          jmp       short rc_exit

prloop:
          mov       dx,StatusVid        ; get 6845 status register address
          lds       si,rc_string
rloop:
          or        dx,dx               ;  check for mono, EGA, or VGA
          jz        readit              ; if so, skip retrace wait

rtend:    in        al,dx               ; wait for horizontal retrace end
          shr       al,1                ; check retrace bit
          jc        rtend               ; wait for it to go off
          cli                           ; no interrupts while we wait for it 
                                        ;  to go back on
rtstart:  in        al,dx               ; wait for horizontal retrace start
          shr       al,1                ; check retrace bit
          jnc       rtstart             ; wait for it to go on

readit:   mov       ax,es:[di]          ; read from display
          sti                           ; enable interrupts again
          mov       ds:[si],ax
          add       si,2
          add       di,2
          loop      rloop

rc_exit:
          popm      si, di, ds, bx

          exit

          ;
          ; WriteCellStr - write the characters and attributes at the 
          ;   specified location for the specified length
          ;
          ; On entry:
          ;         Arguments are:
          ;            char _far *fptr
          ;            int length
          ;            int row
          ;            int column
          ;
          ; On exit:
          ;         Returns nothing
          ;         AX, CX, DX destroyed
          ;         All other registers unchanged, interrupts on
          ;

          entry     WriteCellStr,argframe,far

          argW      wc_column
          argW      wc_row
          argW      wc_length
          argD      wc_string

          pushm     bx, ds, di, si

          mov       dh,wc_row
          mov       dl,wc_column
          call      initpchr

          mov       cx,wc_length
;          shr       cx,1		; convert from byte to cell count

          cmp       BIOSOutput,0	; BIOS output requested?
          je        pwcloop

; BIOS output code!
          call      GetCursor		; save current cursor location
;          push      dx

          les       di,wc_string

          push      bp			; save BP for old PC BIOS bug
wcbloop:
          push      cx
          mov       dh,BIOSRow
          mov       dl,BIOSCol
          call      SetCursor

          mov       bx,es:[di]		; get character & attribute to write
          mov       al,bl
          mov       bl,bh		; attribute
          mov       bh,BIOSPage
          mov       ah,09h
          mov       cx,1
          int       10h			; write char & att at cursor

          add       di,2
          inc       BIOSCol
          pop       cx
          loop      wcbloop

          pop       bp

;          pop       dx
;          call      SetCursor		; restore cursor location
          jmp       short wcs_exit

pwcloop:
          mov       dx,StatusVid        ; get 6845 status register address
          lds       si,wc_string
wcloop:
          mov       bx,ds:[si]		; get character to write
          or        dx,dx               ;  check for mono, EGA, or VGA
          jz        wcwrite             ; if so, skip retrace wait

wcend:    in        al,dx               ; wait for horizontal retrace end
          shr       al,1                ; check retrace bit
          jc        wcend               ; wait for it to go off
          cli                           ; no interrupts while we wait for it 
                                        ;  to go back on
wcstart:  in        al,dx               ; wait for horizontal retrace start
          shr       al,1                ; check retrace bit
          jnc       wcstart             ; wait for it to go on

wcwrite:
          mov       es:[di],bx          ; write to display
          sti                           ; enable interrupts again
          add       di,2
          add       si,2
          loop      wcloop

wcs_exit:
          popm      si, di, ds, bx

          exit

          ;
          ; WriteChrAtt - write one character and attribute
          ;
          ; On entry:
          ;         Arguments are:
          ;            int row
          ;            int column
          ;            int attribute
          ;            int character
          ;
          ; On exit:
          ;         AX, CX, DX destroyed
          ;         All other registers unchanged, interrupts on
          ;

          entry     WriteChrAtt,argframe,far

          argW      wc_char
          argW      wc_attrib
          argW      wc_column
          argW      wc_row

          pushm     bx, di

          mov       dh,wc_row
          mov       dl,wc_column
          call      initpchr

          cmp       BIOSOutput,0	; BIOS output requested?
          je        wc1
          call      GetCursor		; save current cursor location
          push      dx
wc1:
          mov       ah,wc_attrib
          mov       al,wc_char
          call      putchar

          cmp       BIOSOutput,0	; BIOS output requested?
          je        wca_exit
          pop       dx
          call      SetCursor		; reset saved current cursor location

wca_exit:
          popm      di, bx

          exit


          ;
          ; WriteStrAtt - write a string & attribute directly to display memory
          ;
          ; On entry:
          ;         Arguments are:
          ;            int row:         row for output
          ;            int column:      column for output
          ;            int attribute:   attribute output
          ;            char *string:    string to display
          ;
          ; On exit:
          ;         AX, CX, DX, ES destroyed
          ;         All other registers unchanged, interrupts on
          ;

          entry     WriteStrAtt,argframe,far

          argW      string              ;string address
          argW      attrib              ;display attribute
          argW      ocolumn             ;column number
          argW      orow                ;row number

          pushm     bx,si,di		;save registers

          mov       dh,orow             ;get row
          mov       dl,ocolumn          ;get column
          call      initpchr            ;initialize the display routines
          mov       si,string           ;point to string

          cmp       BIOSOutput,0	; BIOS output requested?
          je        ploop
          call      GetCursor		; save current cursor location
          push      dx
ploop:
          lodsb                         ;get a character to display
          or        al,al               ;end of string?
          jz        pbye                ;if so we're done
          mov       ah,attrib           ;get attribute
          call      putchar             ;display the character
          jmp       short ploop
pbye:
          cmp       BIOSOutput,0	; BIOS output requested?
          je        ws_exit
          pop       dx
          call      SetCursor		; reset saved current cursor location
ws_exit:
          popm      di,si,bx            ;restore registers

          exit


          ;
          ; WriteCharStrAtt - write characters & attribute directly to display
          ;
          ; On entry:
          ;         Arguments are:
          ;            int row:         row for output
          ;            int column:      column for output
          ;            int length:	length of "string"
          ;            int attribute:   attribute output
          ;            char *string:    string to display
          ;
          ; On exit:
          ;         AX, BX, CX, DX, ES destroyed
          ;         All other registers unchanged, interrupts on
          ;

          entry     WriteCharStrAtt,argframe,far

          argW      string              ;string address
          argW      attrib              ;display attribute
          argW      olength		;string length
          argW      ocolumn             ;column number
          argW      orow                ;row number

          pushm     bx,si,di		;save registers

          mov       dh,orow             ;get row
          mov       dl,ocolumn          ;get column
          call      initpchr            ;initialize the display routines

          mov       si,string           ;point to string
          mov       cx,olength

          cmp       BIOSOutput,0	; BIOS output requested?
          je        qloop
          call      GetCursor		; save current cursor location
          push      dx
qloop:
          lodsb                         ;get a character to display
          mov       ah,attrib           ;get attribute
          push      cx
          call      putchar             ;display the character
          pop       cx
          loop      qloop

          cmp       BIOSOutput,0	; BIOS output requested?
          je        wcsa_exit
          pop       dx
          call      SetCursor		; reset saved current cursor location
wcsa_exit:
          popm      di,si,bx            ;restore registers

          exit

          ;
          ; WriteVStrAtt - quick-print a string and attribute vertically
          ;   directly to display memory
          ;
          ; On entry:
          ;         Arguments are:
          ;            int row:         row for output
          ;            int column:      column for output
          ;            int attribute:   attribute output
          ;            char *string:    string to display
          ;
          ; On exit:
          ;         AX, BX, CX, DX, ES destroyed
          ;         All other registers unchanged, interrupts on
          ;
          entry     WriteVStrAtt,argframe,far

          argW      v_string            ;string address
          argW      v_attrib            ;display attribute
          argW      v_column            ;column number
          argW      v_row               ;row number

          pushm     bx, si, di          ;save registers

          mov       dh, v_row           ;get row
          mov       dl, v_column        ;get column
          call      initpchr            ;initialize the display routines

          mov       si, v_string        ;point to string
          cmp       BIOSOutput,0	; BIOS output requested?
          je        vloop
          call      GetCursor		; save current cursor location
          push      dx
vloop:
          lodsb                         ;get a character to display
          or        al,al               ;end of string?
          jz        vbye                ;if so we're done
          mov       ah,v_attrib         ;get attribute
          call      putchar             ;display the character
          inc       BIOSRow
          dec       BIOSCol		;putchar INC'd BIOSCol

          cmp       BIOSOutput,0	; BIOS output requested?
          jne       vloop

          add       di,rmargin		; go to next line
          add       di,rmargin
          sub       di,2		; at the same column
          jmp       short vloop
vbye:
          cmp       BIOSOutput,0	; BIOS output requested?
          je        vs_exit
          pop       dx
          call      SetCursor		; reset saved current cursor location
vs_exit:
          popm      di, si, bx          ;restore registers

          exit

          ;
          ; WriteTTY - write a string to the display via BIOS output
          ;
          ; On entry:
          ;         Arguments are:
          ;            char *string:    string to display
          ;
          ; On exit:
          ;         AX, BX, CX, DX, ES destroyed
          ;         All other registers unchanged, interrupts on
          ;

          entry     WriteTTY,argframe,far

          argW      string              ; string address

          push      bp			; for PC1 bug
          pushm     bx, si
          mov       si,string

          mov       ah,0Fh
          int       10h			; get current page into BH
wloop:
          lodsb				; get next character
          or        al,al
          jz        write_bye
          mov       ah,0Eh
          mov       bl,7
          int       10h
          jmp       short wloop
write_bye:
          popm      si, bx
          pop       bp

          exit


          ;
          ; INITPCHR - Initialize direct video I/O
          ;
          ; On entry:
          ;         DH = screen row
          ;         DL = screen column
          ;
          ; On exit:
          ;         ES:DI = location in display buffer of specified row
          ;                 and column
          ;         AX, BX, DX, ES destroyed
          ;         All other registers unchanged, interrupts on
          ;

          entry     initpchr,noframe,,local   ;set entry point

          push      bp			; save BP for old PC BIOS bug

          mov       StatusVid,0         ;clear wait for retrace (CGA) address

          xor       ax,ax               ;get zero
          mov       es,ax               ;point es to ROM BIOS data area
          mov       ax,es:[044Ah]       ;get line length
          cmp       ax, 0
          jne       ValidColumns
          mov       ax, 80
          mov       bx,_gpIniptr        ;point to INI data
          cmp       [bx].I_Columns,0
          jz        ValidColumns
          mov       ax, [bx].I_Columns
ValidColumns:
          mov       rmargin,ax          ;set right margin to line length

          mov       BIOSRow,dh
          mov       BIOSCol,dl

          mov       bx,_gpIniptr
          cmp       [bx].I_BIOS,0	; BIOS output requested?
          je        NoInitBIOS

          mov       BIOSOutput, 1
          mov       ah,0Fh
          int       10h			; get current page into BH
          mov       BIOSPage,bh
          jmp       short init_exit

NoInitBIOS:
          mov       BIOSOutput, 0
          mul       dh                  ;line length * row = total row bytes
                                        ;  in ax
          xor       dh,dh               ;clear high byte so dx = column
          add       ax,dx               ;add the number of columns
          add       ax,ax               ;* 2 for attribute byte
          add       ax,es:[044Eh]       ;add offset into screen buffer
          push      ax			;save the cursor position offset

          call      GetVideoMode
          cmp       ax, 2
          jb        mda_cga
          mov       ah,12h              ;alternate function select
          mov       bl,10h              ;get EGA info
          int       10h
          cmp       bl,4
          ja        mda_cga             ;older than EGA (source: TECH Help!)
          cmp       bh,1
          je        mono                ;mono mode in effect
          cmp       bh,0
          je        vga                 ;colour mode in effect
mda_cga:
          cmp       byte ptr es:[0449h],7  ;check video mode
mono:     mov       ax,0B000h           ;assume mono screen
          je        ipdone              ;if mode 7, it's mono & we're done
          mov       ax,es:[0463h]       ;address of 6845 video controller
          add       ax,6                ;offset for status register
          mov       StatusVid,ax        ;save status register address
vga:
          mov       ax,0B800h           ;set for cga/ega/vga

ipdone:   mov       es,ax               ;copy video segment
          xor       di,di               ;set DI to hardware video offset

          ;
          ; Is DESQView loaded?  If so, get shadow video RAM buffer (if any)
          ;   make SHADOW call to DESQview to set window buffer address 
          ;   in ES:DI -- leaves ES:DI alone if no shadow buffer
          ;
          mov       bx,_gpIniptr
          cmp       [bx].I_DVMode,0	; Is DV loaded?
          je        not_dv
          cmp       __osmajor,10	; if OS/2 or NT, DV not loaded!
          jae       not_dv

          mov       ah,0FEh
          int       10h

not_dv:   pop       ax                  ;get back old cursor offset
          add       di,ax               ;add to video buffer offset

init_exit:
          pop       bp
          exit


          ;
          ; PCHAR - display a character directly to video memory
          ;
          ; On entry:
          ;         AH = attribute
          ;         AL = character to print
          ;         ES:DI = segment:offset for video memory
          ;
          ; On exit:
          ;         ES:DI = updated video memory location 
          ;         AX, CX, DX destroyed
          ;         All other registers unchanged, interrupts on
          ;

          EVEN
          entry     putchar,noframe,,local  ;set entry point

          xor       bx,bx
          mov       cx,1                ;print 1 character
          mov       BIOSColor,ah	; save color for BIOS output w/tabs
          cmp       al,9                ;check for tab
          jne       doprint             ;not tab, go display it
          mov       bl,BIOSCol          ;tab  - get current column number
          and       bx,07h              ;take lower three bits
          mov       cx,8                ;set tab width
          sub       cx,bx               ;get number of blanks
          mov       al,' '              ;display that many blanks
doprint:
          mov       bl,BIOSCol  	;get current column
          cmp       bx,rmargin          ;to right of right margin?
          jae       pchar_bye           ;  yup - exit

          mov       bx,_gpIniptr
          cmp       [bx].I_BIOS,0	; BIOS output requested?
          je        NoBIOS

          push      bp
          push      cx

          push      ax
          mov       dh,BIOSRow
          mov       dl,BIOSCol
          call      SetCursor
          pop       ax			; restore character & attribute

          mov       bh,BIOSPage		; page 0?
          mov       bl,BIOSColor	; attribute
          mov       cx,1
          mov       ah,9
          int       10h

          pop       cx
          pop       bp

          jmp       short noprint

NoBIOS:
          mov       dx,StatusVid        ;get 6845 status register address
          or        dx,dx               ; check for monochrome, EGA, or VGA
          jz        writeit             ;if so, skip retrace wait
          mov       bx,ax               ;save character and attribute

hrtend:   in        al,dx               ;wait for horizontal retrace end
          shr       al,1                ;check retrace bit
          jc        hrtend              ;wait for it to go off
          cli                           ;no interrupts while we wait for it 
                                        ;  to go back on
hrtstart: in        al,dx               ;wait for horizontal retrace start
          shr       al,1                ;check retrace bit
          jnc       hrtstart            ;wait for it to go on
          mov       ax,bx               ;get back character & attr
writeit:
          stosw                         ;write to display
          sti                           ;enable interrupts again

noprint:
          inc       BIOSCol             ;increment column counter
          loop      doprint             ;check for more chars to print

pchar_bye:
          exit


          ;
          ; CHG_COLORS - change attributes directly in video memory
          ;
          ; On entry:
          ;         AL = attribute
          ;         CX = Number of character cells to modify
          ;         ES:DI = segment:offset for video memory
          ;
          ; On exit:
          ;         ES:DI = updated video memory location 
          ;         AX, BX, CX, DX destroyed
          ;         All other registers unchanged, interrupts on
          ;

          entry     chg_colors,noframe,,local   ;set entry point

          mov       bx,_gpIniptr
          cmp       [bx].I_BIOS,0	; BIOS output requested?
          je        chg_direct

          push      bp			; save BP for old PC BIOS bug
          mov       BIOSColor,al
          mov       dh,BIOSRow
          mov       dl,BIOSCol
chgbloop:
          push      cx
          push      dx
          call      SetCursor

          mov       ah,8
          mov       bh,BIOSPage
          int       10h			; get char & att at cursor

          mov       bl,BIOSColor	; retrieve attribute to BL
          mov       cx,1
          mov       bh,BIOSPage
          mov       ah,9
          int       10h			; write char & att at cursor

          pop       dx
          inc       dx
          pop       cx
          loop      chgbloop

          pop       bp
          jmp       short chc_exit       

chg_direct:
          mov       dx,StatusVid        ; get 6845 status register address
c_attr:
          inc       di			; skip the character
          or        dx,dx               ; check for monochrome, EGA, or VGA
          jz        cw_attr             ; if so, skip retrace wait
          mov       bl,al
c_hrtend:
          in        al,dx               ; wait for horizontal retrace end
          shr       al,1                ; check retrace bit
          jc        c_hrtend            ; wait for it to go off
          cli                           ; no interrupts while we wait for it 
                                        ;  to go back on
c_hrtstart:
          in        al,dx               ; wait for horizontal retrace start
          shr       al,1                ; check retrace bit
          jnc       c_hrtstart          ; wait for it to go on
          mov       al,bl               ; get back attribute
cw_attr:
          stosb                         ; write attribute to display
          sti                           ; enable interrupts again
          loop      c_attr		; continue for rest of line / screen
chc_exit:
          exit


          ;
          ; GetCursor - save the current cursor position
          ;
          ; On exit:
          ;         DX = current cursor position
          ;         AX, BX, DX destroyed
          ;         All other registers unchanged, interrupts on
          ;

          entry     GetCursor,noframe,,local   ;set entry point

          push      cx
          mov       ah,3		; save current cursor location
          mov       bh,BIOSPage
          int       10h
          pop       cx

          exit


          ;
          ; SetCursor - set the cursor position
          ;
          ; On entry:
          ;         DX = cursor position
          ; On exit:
          ;         AX, BX, DX destroyed
          ;         All other registers unchanged, interrupts on
          ;

          entry     SetCursor,noframe,,local   ;set entry point

          mov       ah,2		; set cursor location
          mov       bh,BIOSPage
          int       10h

          exit


	;==================================================================
	;
	;  MISCELLANEOUS ROUTINES
	;
	;==================================================================

          ;
          ; NOAPPEND - dummy routine to keep APPEND from crashing 4DOS
          ;

          entry     noappend,noframe,far

          mov       ax,0FFFFh
          exit


          ;
          ; CriticalSection - Start or end critical section under
          ;                   multitaskers and task switchers
          ;
          ; On entry:
          ;         Arguments on stack:
          ;           unsigned int CritFlag       0 = end, 1 = start
          ;
          ; On exit:
          ;         AX, BX, DX, destroyed
          ;         All other registers unchanged, interrupts on
          ;

          entry     CriticalSection,argframe,far

          argW      CritFlag            ;1 to start, 0 to end

          push      bx
          mov       bx,_gpIniptr        ;point to INI data
          cmp       [bx].I_WMode,1      ;running under Windows?
           jbe      CFChkDV             ;if not check DV
          mov       ax,1681h            ;get begin critical section value
          cmp       wptr CritFlag,0     ;start?
           jne      CFWinGo             ;if so go on
          inc       ax                  ;bump to end value
          ;
CFWinGo:  int       2Fh                 ;call Windows, then drop through
          ;
CFChkDV:  cmp       [bx].I_DVMode,1     ;running under DESQview?
           jbe      CFDone              ;if not we're done
          mov       ax,DVC_BEGINC       ;get begin critical section value
          cmp       wptr CritFlag,0     ;start?
           jne      CFDVGo              ;if so go on
          mov       ax,DVC_ENDC         ;get end value
          ;
CFDVGo:   int       15h                 ;call DESQView
          ;
CFDone:
          pop       bx
          exit                          ; all done


          ;
          ; DecodeMsg - Decode and display a "secure" message
          ;
          ; On entry:
          ;   DecodeMsg(int MsgNum, (char *)MsgBuf)
          ;
          ; On exit:
          ;   AX = message buffer address
          ;   BX, CX, DX, ES destroyed
          ;   Other registers preserved, interrupt state unchanged
          ;

          entry     DecodeMsg,argframe,far
          ;
	argW	MsgBuf		;message buffer address
          argW      MsgNum              ;message number
          ;

          pushm     bx,si,di,ds         ;save registers

          loadseg   es,ds               ;set destination segment
          mov       ah,bptr MsgNum+1    ;make things confusing (this gets 0)
          mov       cl,ah               ;copy the 0
          loadseg   ds,MSG_SEG,bx     	;get server segment address
	assume	ds:MSG_SEG	;fix assumes
          mov       al,bptr MsgNum      ;get real message number + 36
          xchg      ah,al               ;put it in ah
          mov	di,MsgBuf           ;get output buffer address
          add       cl,4                ;get rotation count of 4
          mov       si,offset MSG_SEG:ServErrs  ;get error table address
          ror       ax,cl               ;slide things around
          sub       ax,(2400h shr 4)    ;adjust for offset of 36
          shr       ax,cl               ;slide result into place, now AX is
                                        ;  message number and hopefully we
                                        ;  confused the casual hacker a bit
          mov       es:[di],ah          ;confuse a little more
          add       al,193              ;add signon message base
          push      di                  ;save destination address
          call      ErrMsg              ;dump message into buffer
	xor	al,al		;get terminating NUL
	stosb			;terminate the string
	pop	ax		;get back buffer address
          popm      ds,di,si,bx         ;restore registers
	assume	ds:DGROUP		;fix assumes
          exit


          ;
          ; SAFE_APPEND - turn off DOS's default unsafe APPENDs (stripping
          ;    an explicit pathname)
          ;
          ; On exit:
          ;         AX, BX destroyed
          ;         All other registers unchanged, interrupts on
          ;

          entry     safe_append,noframe,far	;set up entry point

          push      bx
          mov       ax,0B706h		; get APPEND state
          xor       bx,bx
          int       02Fh
          test      bx,1		; test for APPEND active
          jz        no_append
          cmp       __osmajor,20
          jb        not_os2
          or        bh,00100000b	; OS/2 has the /PATH:OFF backwards!
          jmp       short set_append
not_os2:
          and       bh,11011111b	; set /PATH:OFF
set_append:
          mov       ax,0B707h
          int       02Fh
no_append:
          pop       bx
          exit

          ;
          ; QueryKSTACK - Return 1 if KSTACK is loaded
          ;
	  ; On entry:
	  ;	No requirements
	  ;
          ; On exit:
          ;         AX = 1 if KSTACK loaded, 0 if not
          ;         All other registers unchanged, interrupts on
          ;

          entry     QueryKSTACK,noframe,far	;set up entry point
          push      bx
	  mov	    ax,0D44Fh
	  xor	    bx,bx
	  int	    2Fh
	  cmp	    ax,44DDh		; AX = 44DD if KSTACK is loaded
	  jne	    no_kstack
	  mov	    bx,1
no_kstack:
	  mov	    ax,bx
          pop       bx
          exit


	;
	; QueryClipAvailable - Return 1 if clipboard is available
	;
	; On entry:
	;	No requirements
	;
	; On exit:
	;	AX = 1 if clipboard is available, 0 if not
	;	All other registers and interrupts unchanged
	;

	entry	QueryClipAvailable,noframe,far	;set up entry point
	mov	ax, 1700h 		;does the current session support
	int	2Fh			;  reading the clipboard?
	sub	ax, 1700h 		;if so it returns  AX != 1700h
	je	QCADone			;if not we're done (AX = 0)
	mov	ax, 1			;yes it does
QCADone:
        exit


	;
	; OpenClipboard - Open the clipboard
	;
	; On entry:
	;	No requirements
	;
	; On exit:
	;	Clipboard opened
	;	All registers and interrupts unchanged
	;

	entry	OpenClipboard,noframe,far	;set up entry point
	mov	ax, 1701h
	int	2Fh			;open the clipboard
	exit


	;
	; CloseClipboard - Close the clipboard
	;
	; On entry:
	;	No requirements
	;
	; On exit:
	;	Clipboard closed
	;	All registers and interrupts unchanged
	;

	entry	CloseClipboard,noframe,far	;set up entry point
	mov	ax, 1708h
	int	2Fh			;close the clipboard
	exit

	;
	; QueryClipSize - Return clipboard size
	;
	; On entry:
	;	Clipboard must be available per QueryClipAvailable
	;	  and opened by OpenClip
	;
	; On exit:
	;	DX:AX = size of clipboard
	;	All other registers and interrupts unchanged
	;

	entry	QueryClipSize,noframe,far	;set up entry point
	mov	ax, 1704h
	mov	dx, 1		;OEM text format
	int	2Fh		;get size of cliboard data in DX:AX
	exit

	;
	; ReadClipData - Read data from the clipboard
	;
	; On entry:
	;	Clipboard must be available per QueryClipAvailable
	;	  and opened by OpenClip
	;	Arguments on stack:
	;	  char _far *lpMemory	pointer to buffer
	;
	; On exit:
	;	All registers and interrupts unchanged
	;

	entry	ReadClipData,argframe,far	;set up entry point
	argD	lpMemory		;buffer address

	mov	ax, 1705h 	;read data
	les	bx, lpMemory	;data pointer
	mov	dx, 1		;OEM text format
	int	2Fh		;read the data
	exit

	;
	; SetClipData - Write data to the clipboard
	;
	; On entry:
	;	Clipboard must be available per QueryClipAvailable
	;	  and opened by OpenClip
	;	Arguments on stack:
	;	  char _far *lpMemory	pointer to buffer
	;	  long uClipSize	size of data to transfer
	;
	; On exit:
	;	All registers and interrupts unchanged
	;

	entry	SetClipData,argframe,far	;set up entry point
	argD	uClipSize	 	;bytes to transfer
	argD	lpMemory		;buffer address

	mov	ax, 1702h
	int	2Fh			; empty the clipboard
	les	bx, lpMemory		; get data address
	dload	cx, si, uClipSize	; get buffer size
	mov	dx, 7			; OEM text format
	mov	ax, 1703h
	int	2Fh			; set clipboard data
	mov	ax, 1708h
	int	2Fh			; close clipboard
	exit

	;
	; Divide64By32 - Divide a QWORD by a DWORD
	;
	; On entry:
	;	Arguments on stack:
	;	t_int64 *ullDividend	    pointer to the dividend
	;	unsigned long ulDivisor     divisor
	;
	; On exit:
	;	DX:AX = quotient
        ;	All other registers unchanged, interrupts on
	;
	entry	Divide64By32,argframe,far; set up entry point
	argD	ulDivisor
	argW	ullDividend
	push	bx
	mov	bx,ullDividend		; load dividend address
	cmp	_gnCPU,386
	jb	Div8086			; old CPU - divide by bit shifting
	.386
	push	eax
	pop	ax
	push	edx			; save MSDW of the used registers
	pop	dx
	mov	eax,[bx]
	mov	edx,[bx+4]		; and the dividend itself
	div	dword ptr ulDivisor
	push	eax			; move return value from EAX to DX:AX
	pop	ax
	pop	edx
	push	ax			; restore MSDW of the used registers
	pop	eax
	.8086
	jmp	short DivRet
Div8086:push	cx		; thanks to Peter Norton for this method!
	mov	cx,64
	xor	ax,ax
	xor	dx,dx
DivShft:shl	[bx+6],1	; shift 96 bits of DX:AX:ullDividend left by 1
	rcl	[bx+4],1
	rcl	[bx+2],1
	rcl	[bx],1
	rcl	ax,1
	rcl	dx,1
	cmp	dx,ulDivisor+2	; compare DX:AX and ulDivisor - first the MSW
	ja	DivOne
	jb	DivZero
	cmp	ax,ulDivisor	; DX = MSW of ulDivisor - check LSW
	jb	DivZero
DivOne:	sub	ax,ulDivisor
	sbb	dx,ulDivisor+2
	inc	word ptr [bx+6]	; put in a 1 as divisor went into dividend once
DivZero:loop	DivShft		; keep going all 64 times
	mov	ax,[bx]		; end up with quotient in ullDividend
	mov	dx,[bx+2]
	pop	cx
DivRet:	pop	bx
	exit

	;
	; ShiftLeft16 - Shift a 48-bit number left by 16 bits
	;
	; On entry:
	;	Arguments on stack:
	;	t_int64 *ullVal	    pointer to the number to shift
	;
	; On exit:
        ;	All registers unchanged, interrupts on
	;
	entry	ShiftLeft16,argframe,far; set up entry point
	argW	ullVal
	pushm	ax,bx
	mov	bx,ullVal		; load number address
	mov	ax,[bx+4]
	mov	[bx+6],ax		; shift bits 47-32 to bits 63-48
	mov	ax,[bx+2]
	mov	[bx+4],ax		; shift bits 31-16 to bits 47-32
	mov	ax,[bx]
	mov	[bx+2],ax		; shift bits 15-0 to bits 31-16
	popm	bx,ax
	exit

	;
	; SetTimerMode2 - Set timer to mode 2 (thanks to Kris Heidenstrom!)
	;
	; On entry:
	;	No requirements
	;
	; On exit:
        ;	All registers unchanged, interrupts on
	;
	entry	SetTimerMode2,noframe,far; set up entry point
	pushm	ax,es
	sti
	xor	ax,ax
	mov	es,ax
	mov	ax,es:[46Ch]	; get LSW of tick count
Waitick:cmp	ax,es:[46Ch]	; changed?
	je	Waitick		; if not, loop
	mov	al,00110100b	; channel 0, mode 2
	cli			; while disturbance to system time is minimal,
	out	43h,al		; set the mode
	jmp	$+2
	xor	ax,ax
	out	40h,al		; LSW of divisor
	jmp	$+2
	out	40h,al		; MSW of divisor
	jmp	$+2
	sti
	popm	es,ax
	exit

	;
	; GetTimer - Read current programmable interval timer 0 value
	;
	; On entry:
	;	No requirements
	;
	; On exit:
	;	AX = current timer 0 value
        ;	All other registers unchanged, interrupts on
	;
	entry	GetTimer,noframe,far	; set up entry point
	mov	al,0
	cli
	out	43h,al
	jmp	$+2
	in	al,40h	; LSB
	jmp	$+2
	mov	ah,al
	in	al,40h	; MSB
	jmp	$+2
	sti
	xchg	ah,al
	neg	ax	; timer counts down
	exit

	;
	; GetTimeStamp - Get 48-bit timestamp in 838.09534452 ns units since
	;		midnight (thanks to Kris Heidenstrom for this code!)
	;
	; On entry:
	;	Arguments on stack:
	;	t_int64 *ullClock    pointer for the timestamp to be written to
	;
	; On exit:
        ;	All registers unchanged, interrupts on
	;
	entry	GetTimeStamp,argframe,far; set up entry point
	argW	ullClock
	pushm	ax,bx,cx,dx,si,di,es
	xor	ax,ax
	mov	es,ax
	cli
	mov	si,es:[46Ch]	; tick count LSW
	mov	di,es:[46Eh]	;   "	"    MSW
	call	GetTimer	; timer 0 must operate in mode 2, divisor 65536
	cli			; GetTimer enables interrupts on exit
	mov	dx,es:[46Ch]	; read tick count again
	mov	bx,es:[46Eh]	; (timer may have generated interrupt since)
	sti
	cmp	dx,si		; tick counts same?
	je	GotTick		; yes, just return second tick count
	or	ax,ax		; no, is tick count low or high?
	jns	GotTick		; low, read was just past interrupt
	mov	dx,si		; high, previous tick count is right
	mov	bx,di		; get MSW of tick count too
GotTick:mov	di,ullClock
	mov	[di],ax		; write timestamp
	mov	[di+2],dx	; (possible range: [0...1800AFFFFFh])
	mov	[di+4],bx
	mov	[di+6],es
	popm	es,di,si,dx,cx,bx,ax
	exit

	;
	; GetTSC - Read current time stamp counter value (586+ only)
	;
	; On entry:
	;	Arguments on stack:
	;	t_int64 *ullTSC     pointer for the TSC value to be written to
	;
	; On exit:
        ;	All registers unchanged, interrupts on
	;
	entry	GetTSC,argframe,far	; set up entry point
	argW	ullTSC

RDTSC	MACRO
	db	0Fh,31h		  ; hardcoded opcode for RDTSC instruction
ENDM
	.386
	pushad
	mov	eax,1			; function 1 to get features in EDX
	CPUID				; serialise instruction execution
	xor	eax,eax			; prepare 0 in case there is no TSC
	and	edx,10000b		; does this CPU contain a TSC?
	jz	WrTSC			; no, write 0 (EDX is also 0 now)
	RDTSC				; yes, read the current TSC value
WrTSC:	mov	bx,ullTSC		; load the address to write it to
	mov	[bx],eax		; write the LSDW
	mov	[bx+4],edx		; write the MSDW
	popad
	.8086
	exit

	;
	; GetWinMs - Get Windows VTD time (thanks to Lattice Semiconductor!)
	;
	; On entry:
	;	No requirements
	;
	; On exit:
	;	DX:AX = current system time [ms since Windows started]
	;		(0 if the VTD does not support an API)
        ;	All other registers unchanged, interrupts on
	;
	entry	GetWinMs,noframe,far	; set up entry point
	pushm	bx,di,es
	mov	ax,1684h		; get Windows device API entry point
	mov	bx,5			; virtual timer device ID
	xor	di,di
	mov	es,di
	int	2Fh
	mov	ax,di			; get ready to return 0 on failure
	mov	dx,es
	or	ax,dx			; does the VTD support an API?
	jz	vtdRet			; no, return 0
	mov	word ptr SaveESP,di
	mov	word ptr SaveESP+2,es
	mov	ax,101h			; get current system time [ms] in EAX
	.386
	push	eax
	pop	ax
	push	edx			; save MSDW of the used registers
	pop	dx
	call	SaveESP
	push	eax			; move return value from EAX to DX:AX
	pop	ax
	pop	edx
	push	ax			; restore MSDW of the used registers
	pop	eax
	.8086
vtdRet:	popm	es,di,bx
	exit

	;
	; MonitorOnOff - Turn monitor on or off
	;		(if VESA BIOS present and function supported)
	;
	; On entry:
	;	Arguments on stack:
	;	char state     new power state:
	;			0	On
	;			1	standby
	;			2	suspend
	;			4	Off
	;			8	reduced On (for flat screens)
	;
	; On exit:
	;	Monitor turns off
	;
	entry	MonitorOnOff,argframe,far; set up entry point
	argW	state
	mov	ax,4F10h
	mov	bl,1			; set display power state
	mov	bh,state
	int	10h
	exit

	; convert 2 BCD nibbles in AL to binary (AH is zeroed)

	entry	bcd2bin,noframe,near
	mov	ah,al
	and	al,0Fh
	shrn	ah,4
	aad
	exit
	;
	; SetPowerState - Set new system power state (Standby, Suspend or Off)
	;		  (thanks to David Lindauer for this method!)
	;
	; On entry:
	;	Arguments on stack:
	;	  int state    new power state:
	;		1	standby
	;		2	suspend
	;		3	off (not supported for device ID 1 in APM 1.0)
	;
	; On exit:
	;	System switches off or enters standby or suspend state
	;
	entry	SetPowerState,argframe,far; set up entry point
	argW	state
	mov	ax,5300h		; See if APM available
	mov	bx,0			; device = BIOS
	int	15h
	jc	err
	cmp	ax,101h 		; See if version 1.1 or greater
	jc	err
	mov	cx,ax			; save version
	mov	ax,5304h		; disconnect any 32-bit prot.m.interface
	mov	bx,0			; device = BIOS
	int	15h
	mov	ax,5301h		; Do a real mode connection
	mov	bx,0			; device = BIOS
	int	15h
	jc	err
	mov	ax,530eh		; Enable latest version of APM
	mov	bx,0			; device = BIOS
	int	15h
	jc	err
	mov	ax,530dh		; Now engage and enable power management
	mov	bx,1			; device = all
	mov	cx,1			; enable
	int	15h
	jc	err
	mov	ax,530fh
	mov	bx,1			; device = ALL
	mov	cx,1			; enable
	int	15h
	jc	err
	mov	ax,5307h		; Set the power state
	mov	bx,1			; device = ALL
	mov	cx,state
	int	15h			; During Suspend, timer tick "freezes"
	cmp	_gnCPU,286		; so the time must be updated from RTC
	jb	err			; no Real-Time Clock (RTC) - exit
	mov	ah,2
	int	1Ah			; get Real-Time Clock time in BCD
	jc	err
	mov	al,ch			; Convert from BCD to binary
	call	bcd2bin			; hour
	mov	ch,al
	mov	al,cl
	call	bcd2bin			; minute
	mov	cl,al
	mov	al,dh
	call	bcd2bin			; second
	mov	dh,al
	calldos	SETTIME			; have DOS set the timer ticks
err:	exit

	;
	; RebootW9x - Reboot Windows 9x/ME (thanks to "sk" for this method!)
	;
	; On entry:
	;	No requirements
	;
	; On exit:
	;	Windows reboots
	;
	entry	RebootW9x,noframe,far	; set up entry point
	mov	bp,sp			; stack frame
	cli
	mov	ax,1687h		; get address of DPMI host's
	int	2Fh			; mode switch entry point
	or	ax,ax
	jnz	reset			; no DPMI host?!
	push	es			; save entry point
	push	di
	push	ds			; will zap the first 656 bytes of data
	pop	es			; segment of the data area
	push	cs			; all subsequent data - inline in code
	pop	ds			; 4DOS data section not needed anymore
	xor	ax,ax			; bit 0=0 indicates 16-bit application
	call	far ptr [bp-4]		; switch to protected mode
	jc	reset			; mode switch failed?!
	call	clcdlta
delta	db	'MS-DOS',0		; vendor-specific API name (MS Windows)
descr	dq	?			; descriptor copy buffer
clcdlta:pop	si			; "delta" offset
	mov	ax,168Ah
	int	2Fh			; get vendor-specific API entry point
	and	al,al
	jne	reset			; failed?!
	push	es			; save the entry point
	push	di
	push	ds			; descriptor buffer segment
	pop	es
	mov	cx,1			; number of descriptors to allocate
	xor	ax,ax
	int	31h			; allocate first LDT descriptor
	jc	reset
	push	ax			; save its base selector
	xor	ax,ax
	int	31h			; allocate second LDT descriptor
	jc	reset
	mov	[bp-2],ax		; save its base selector
	and	al,11111000b
	push	ax			; and a copy with TI=GDT:RPL=0
	mov	bx,cs			; LDT selector
	lea	di,[descr-delta+si]	; descriptor buffer offset
	mov	ax,0Bh
	int	31h			; get CS descriptor
	jc	reset
	mov	ax,100h
	call	far ptr [bp-8]		; get LDT alias selector
	jc	reset
	mov	es,ax			; save it
	lea	ax,[reset-delta+si]	; target segment entry point offset
	add	ax,[di+2]		; segment base, bits 15:00
	mov	bl,[di+4]		;	"	"    23:16
	mov	bh,[di+7]		;	"	"    31:24
	adc	bx,0			; now BX:AX = offset in segment
	pop	di			; the LDT base selector with RPL=0
	stosw				; Generate call gate: offset bits 15:00
	pop	ax			;		      segment selector
	stosw
	and	al,11111000b		; save a copy with TI=GDT:RPL=0
	push	ax
	mov	ax,11101100b shl 8	; P=1:DPL=3:S=0:Type=0Ch (call gate)
	stosw				;		      control bits
	xchg	ax,bx			;		      offset bits 31:16
	stosw
	pop	di			; Generate Ring 0 CS descriptor:
	mov	ax,-1			; segment limit will be 4 GB
	stosw				;	"	, bits 15:00
	inc	ax
	stosw				; zero segment base, bits 15:00
	stosb				;	"	"	" 23:16
	mov	ax,1100111110011010b	; G=1:32=1:AVL=0:Limit=4GB (bits 19:16)
	stosw				; P=1:DPL=0:S=1:Code:NoConf:Read:NoAces
	xor	al,al
	stosb				; zero segment base, bits 31:24
	call	far ptr [bp-4]		; switch to Ring 0
reset:	mov	al,0FEh			; pulse RESET low for 6 microseconds
	out	64h,al			; "cold" boot
RebootW9x endp

	;
	; RebootOS2 - Reboot OS/2 (thanks to Roman Stangl for this method!)
	;
	; On entry:
	;	Arguments on stack:
	;	  int nFH		file handle to the opened "DOS$" device
	;
	; On exit:
	;	OS/2 reboots
	;
	entry	RebootOS2,argframe,far	; set up entry point
	argW	nFH
	mov	ax,440Ch		; generic character device request
	mov	bx,nFH
	mov	cx,0D5ABh		; CH: category (IOCTL_DOS)
	xor	dx,dx			; CL: function (DOS_REBOOT)
	xor	si,si			; all other IOCTL parameters are zero
	xor	di,di
	int	21h			; do IOCTL device request and reboot
	mov	al,0FEh			; OS/2 didn't reboot for some reason -
	out	64h,al			;   let it terminate the process...
	exit

	;
	; Reboot
	;
	; On entry:
	;	Arguments on stack:
	;	  char fType		bit 0: 1 = "hard", 0 = "soft" reset
	;
	; On exit:
	;	PC reboots
	;
	entry	Reboot,argframe,far	; set up entry point
	argB	fType
	test	byte ptr fType,1	; "standard" reboot?
	jz	softRst			; yes
	cmp	_gnCPU,286		; no, get CPU type - XTs have different
	jb	softRst			;   keyboard controller
	cli
	in	al,64h			; read 804x status byte
	test	al,10b			; bit 1: "input buffer full"
	jnz	softRst
	xor	al,al			; clear DMA page port
	out	080h, al		; clear mfg port
	mov	al,0FEh			; cold boot (through keyboard reset)
	out	064h,al			;   (because QEMM & MAX intercept boot)
softRst:mov	ax,seg rsegm
	mov	es,ax
	cmp	es:[0FFF0h],19CDh	; Int 19h opcode (put by some EMM)?
	jne	jump			; no, jump to the ROM reset address
	int	19h			; yes, do it as intended by the EMM
jump:	mov	dx,offset qemm_dev	; check if QEMM installed (needed as
	mov	ax,3D00h		;   Stealth ROM doesn't support E05B)
	int	21h			; try to open "QEMM386$"
	jc	noqemm			; if CY, QEMM not present
	jmp	far ptr FFF0h		; reset (BIOS jumps to E05B)
noqemm:	jmp	far ptr E05Bh		; reboot
Reboot	endp

@curseg	ends

rsegm	segment at 0F000h
	org	0E05Bh
E05Bh	label	byte
	org	0FFF0h
FFF0h	label	byte
rsegm	ends

          end

