; Force references to various bits and pieces from sasjp.lib. We need to
; ensure that these are referenced *before* 4dostart.

include product.asm
include trmac.asm
.cseg   LOAD
include model.inc

.extrnx MEM_CPY:near
.extrnx MEM_MOV:near
.extrnx MEM_SET:near
.extrnx MEM_CMPI:near
.extrnx MEM_CHRN:near
.extrnx MEM_CSPN:near
.extrnx GET_CHR:near
.extrnx PUT_CHR:near
.extrnx GET_STR:near
.extrnx PUT_NEWLINE:near
.extrnx PUT_STR:near
.extrnx STR_CSPN:near
.extrnx STR_LEN:near
.extrnx STR_NCMPI:near
.extrnx STR_PBRKN:near
.extrnx E_CODE:near
.extrnx OPENEX_H:near
.extrnx CLOSE_H:near
.extrnx READ_H:near
.extrnx DEC_TO_BYTE:near
.extrnx DEC_TO_WORD:near
.extrnx DEC_TO_WORDS:near
.extrnx ASC_TO_BYTE:near
.extrnx ASC_TO_WORD:near
.extrnx ASC_TO_WORDS:near
.extrnx WORD_TO_DEC:near
.extrnx IS_ALPHA:near
.extrnx IS_DIGIT:near
.extrnx TO_UPPER:near
.extrnx MEM_CHRNDI:near
.extrnx STR_LENDI:near
.extrnx $CHR_CMPI:near
.extrnx $SET_ECODE:near
.extrnx _OSMAJOR:near
.extrnx WORD_TO_ASCE:near
.extrnx DIV_WB:near

end
