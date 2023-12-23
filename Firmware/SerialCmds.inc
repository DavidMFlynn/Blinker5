;=========================================================================================
;
;    Filename:      SerialServoCmds.inc
;    Date:          6/1/2018
;    File Version:  1.1
;
;    Author:        David M. Flynn
;    Company:       Oxford V.U.E., Inc.
;    E-Mail:        dflynn@oxfordvue.com
;    Web Site:      http://www.oxfordvue.com/
;
;=========================================================================================
; Notes:
;  Command interpreter for SerialServo
;
; History
; 1.1    6/1/2018	All commands are here now.
; 1.0    5/25/2018	Moved out of SerialServo.asm
;=========================================================================================
;
kCmd_SetMode	EQU	0x81	;+1 data (SysMode), return ACK
kCmd_GetMode	EQU	0x01
;
kCmd_SaveParams	EQU	0x94	;Save all eeprom params, return ACK
kCmd_RestoreParams	EQU	0x95	;Copy to ram, return ACK
kCmd_ReBoot	EQU	0x99	;ReBoot the controller
kCmd_RunBootloader	EQU	0x9A	;Set EEPROM (0xFF) = 0xFF and reboot
;
; these commands save params and return a 0,0,0,0 packet with the new address
kCmd_SetMasterAddr	EQU	0xA1	;+1 data, return a zero packet
kCmd_SetSlaveAddr	EQU	0xA2	;+1 data, return a zero packet
;
;Special packet (0xDD, 0xDD, 0x01, 0x02, 0xAA, 0x00, 0x00, 0x00, checksum)
kCmd_SendIDString	EQU	0xAA	; returns "DMFE Serial Blinker\n"
;
;=========================================================================================
;
RS232_Send_S0	mLongCall	RS232_Send
	return
;
;=========================================================================================
; Entry: RXDataIsNew=1, data in RX_Data
;
HandleRXData	movlb	RX_Flags	;bank 1
	bcf	RXDataIsNew
	btfsc	RXDataValidFlag	;from master to me?
	bra	HandleRXData_1
HandleRXData_Rtn	movlb	0	;bank 0
	return		; no, ignore this packet
;
HandleRXData_1	movlb	0	;bank 0
                       mMOVLF	RX_Data,FSR1
	mMOVLF	TX_Data,FSR0
;Dispatcher
;
	moviw	FSR1++
	movwf	Param78	;aka RX_Data
	andlw                  0xF0
	SKPZ
	bra                    Dispatcher_1
	movf                   Param78,W
	andlw                  0x0F
;Jump table for 0x00 to 0x0F
                       brw
                       return                                        ;0x00 not used
                       bra                    Cmd_GetMode
                       return
                       return
                       return
                       return
                       return
                       return
                       return
                       return
                       return
                       return
	return
                       return
                       return
                       return
;
Dispatcher_1           sublw                  0x80
                       SKPZ
                       bra                    Dispatcher_2
                       movf                   Param78,W
	andlw                  0x0F
;Jump table for 0x80 to 0x8F
                       brw
                       return                                        ;0x80 not used
                       bra                    Cmd_SetMode
                       return
                       return
                       return
                       return
                       return
                       return
                       return
                       return
                       return
                       return
                       return
                       return
                       return
                       return
;
;Jump table for 0x20 to 0x27
Dispatcher_2           movf                   Param78,W
                       andlw                  0xF0
                       sublw                  0x20
                       SKPZ
                       bra                    Dispatcher_3
                       btfsc                  Param78,3              ;>7?
                       return                                        ; Yes, oops
                       movlw                  0x07
                       andwf                  Param78,W
                       brw
                       return
                       return
                       return
                       return
                       return
                       return
                       return
                       return
;
Dispatcher_3           movf                   Param78,W
                       andlw                  0xF0
                       sublw                  0xB0
                       SKPZ
                       bra                    Dispatcher_4
                       btfsc                  Param78,3              ;>7?
                       return                                        ; Yes, oops
                       movlw                  0x07
                       andwf                  Param78,W
                       brw
                       return
                       return
                       return
                       return
                       return
                       
                       return
                       return
                       return
;
;Jump table for 0x90 to 0x9F
Dispatcher_4           movf                   Param78,W
                       andlw                  0xF0
                       sublw                  0x90
                       SKPZ
                       bra                    Dispatcher_5
                       movlw                  0x0F
                       andwf                  Param78,W
                       brw
                       return                                        ;0x90 not used
                       return
                       return
                       return
                       goto                   Cmd_SaveParams
                       goto                   Cmd_RestoreParams
                       return
                       return                 
                       return
                       reset                                         ;Cmd_ReBoot
                       goto                   Cmd_RunBootloader
                       return
                       return
                       return
                       return
                       return
;
;Jump table for 0xA0 to 0xAF
Dispatcher_5           movf                   Param78,W
                       andlw                  0xF0
                       sublw                  0xA0
                       SKPZ
                       bra                    Dispatcher_6
                       movlw                  0x0F
                       andwf                  Param78,W
                       brw
Dispatcher_6           return                                        ;0xA0 not used
                       goto                   Cmd_SetMasterAddr
                       goto                   Cmd_SetSlaveAddr
                       return
                       return
                       return
                       return
                       return
                       return
                       return
                       goto                   Cmd_SendIDString       ;0xAA
                       return
                       return
                       return
                       return
                       return
;
;---kCmd_SetMode----------------
; Set Mode
Cmd_SetMode	movlw	kMaxMode+1
	subwf	INDF1,W
	SKPB		;kMaxMode+1>Data
	bra	HandleRXData_Rtn
;
	moviw	FSR1++
	movwf	SysMode
	goto	TX_ACK
;
;---kCmd_GetMode------------------
Cmd_GetMode	movf	SysMode,W
	movwi	FSR0++
	goto	RS232_Send_S0
;
;
;---kCmd_SaveParams-----------------
Cmd_SaveParams	mLongCall	SaveParams
	goto	TX_ACK
;
;---kCmd_RestoreParams--------------
Cmd_RestoreParams	mLongCall	CopyToRam
	goto	TX_ACK
;
;---kCmd_RunBootloader--------------
Cmd_RunBootloader	movlw	0xFF
	movwf	EEDataTemp
	mLongCall	EEWriteW	;Write EEPROM address in W
	reset
;
;---kCmd_SetMasterAddr------------
Cmd_SetMasterAddr	moviw	FSR1++
	movwf	RS232_MasterAddr
	mLongCall	SaveParams
	goto	RS232_Send_S0
;
;---kCmd_SetSlaveAddr-------------
Cmd_SetSlaveAddr	moviw	FSR1++
	movwf	RS232_SlaveAddr
	mLongCall	SaveParams
	goto	RS232_Send_S0
;
;---kCmd_SendIDString------------
Cmd_SendIDString	clrf	Param79
Cmd_SendIDString_L1	movf	Param79,W
	call	IDString
	iorlw	0x00
	SKPNZ
	return
	call	StoreSerOut_S0
	incf	Param79,F
	bra	Cmd_SendIDString_L1
;
IDString	brw
	retlw	'D'
	retlw	'M'
	retlw	'F'
	retlw	'E'
	retlw	' '
	retlw	'B'
	retlw	'l'
	retlw	'i'
	retlw	'n'
	retlw	'k'
	retlw	'e'
	retlw	'r'
	retlw	' '
	retlw	'5'
	retlw	'\n'
	retlw	0x00
;
;--------------------------------
	return
;
TX_ACK	movlw	0xFF
StoreSerOut_S0	mLongCall	StoreSerOut
	return
;
;=========================================================================================
