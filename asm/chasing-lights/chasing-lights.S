/* chasing-lights.S -- Chasing lights in AVR assembler
 *
 * Copyright (C) 2016 Artyom V. Poptsov <poptsov.artyom@gmail.com>
 *
 * This program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see
 * <https://www.gnu.org/licenses/>.
 */

#include "avr/io.h"
#include "avr/iom2560.h"
#include "avr/iomxx0_1.h"

        ;; NOTE:
        ;;
        ;; Digital port numbers on Arduino Mega 2560 aren't aligned
        ;; with port registers (PORTB etc.)

#define yl r28
#define yh r29
#define xl r26
#define xh r27

#define delay 10 // ms

        .data
next_switch_after_millis:
        .long   0
control_byte:
        .byte   1               ; control byte
port_number:
        .byte   13              ; digital port number

        .text

        .global setup
setup:
        ;; Configure output ports.
	;;
	;; 'out' fails to work with addresses greater than 63 [1];
        ;; given that 'DDRH' is in the upper addresses, we should
        ;; use 'sts' (Store direct To data Space) [2] to store the
        ;; register value right into memory address.
        ;;
        ;; [1] <https://stackoverflow.com/q/27470814>
        ;; [2] <https://people.ece.cornell.edu/land/courses/ece4760/AtmelStuff/AVRinstr2002.PDF>
        ;;     p. 13
        ;;
        ;; NOTE:
        ;; Don't use '_SFR_IO_ADDR' with 'sts'.
        ldi     r16, 0xF0
        out     _SFR_IO_ADDR(DDRB), r16 ; D10..D13
        sts     DDRH, r16
        ldi     r16, 0x38
        sts     DDRE, r16
        ldi     r16, 0x20
        sts     DDRG, r16
        ret

        ;; Load port address to X register.
load_port_addr:
        lds     r18, port_number
        cpi     r18, 0x04       ; 2..3
        brlt    load_porte_addr
        cpi     r18, 0x05       ; 4
        brlt    load_portg_addr
        cpi     r18, 0x06       ; 5
        brlt    load_porte_addr
        cpi     r18, 0x0A       ; 6..9
        brlt    load_porth_addr
	cpi     r18, 0x0E       ; 10..13
	brlt    load_portb_addr
load_porte_addr:
        ldi     xl, lo8(PORTE)
        ldi     xh, hi8(PORTE)
        ret
load_portg_addr:
        ldi     xl, lo8(PORTG)
        ldi     xh, hi8(PORTG)
        ret
load_porth_addr:
        ldi     xl, lo8(PORTH)
        ldi     xh, hi8(PORTH)
        ret
load_portb_addr:
        ldi     xl, lo8(PORTB)
        ldi     xh, hi8(PORTB)
        ret

        ;; Set port bit for right-ro-left chasing lights.
set_port_bit_rl:
        lds     r18, port_number
        cpi     r18, 0x02       ; 2..3
        breq    set_porte_bit_1
        cpi     r18, 0x04       ; 4
        breq    set_portg_bit
        cpi     r18, 0x05       ; 5
        breq    set_porte_bit_2
        cpi     r18, 0x06       ; 6..9
        breq    set_porth_bit
	cpi     r18, 0x0A       ; 10..13
	breq    set_portb_bit
        ret
set_porte_bit_1:
        ldi     r16, 0x10
        sts     control_byte, r16
        ret
set_portg_bit:
	ldi     r16, 0x20
	sts     control_byte, r16
        ret
set_porte_bit_2:
	ldi     r16, 0x08
	sts     control_byte, r16
        ret
set_porth_bit:
	ldi     r16, 0x08
	sts     control_byte, r16
        ret
set_portb_bit:
        ldi     r16, 0x10
	sts     control_byte, r16
        ret

	;; Set port bit for left-to-right chasing lights.
set_port_bit_lr:
        lds     r18, port_number
        cpi     r18, 0x03       ; 2..3
        breq    set_porte_bit_1_rl
        cpi     r18, 0x04       ; 4
        breq    set_portg_bit_rl
        cpi     r18, 0x05       ; 5
        breq    set_porte_bit_2_rl
        cpi     r18, 0x09       ; 6..9
        breq    set_porth_bit_rl
	cpi     r18, 0x0C       ; 10..13
	breq    set_portb_bit_rl
        ret
set_porte_bit_1_rl:
        ldi     r16, 0x20
        sts     control_byte, r16
        ret
set_portg_bit_rl:
	ldi     r16, 0x20
	sts     control_byte, r16
        ret
set_porte_bit_2_rl:
	ldi     r16, 0x08
	sts     control_byte, r16
        ret
set_porth_bit_rl:
	ldi     r16, 0x40
	sts     control_byte, r16
        ret
set_portb_bit_rl:
        ldi     r16, 0x80
	sts     control_byte, r16
        ret

led_on:
	lds     r16, control_byte
        ld      r15, x
        or      r15, r16
        st      x, r15
        ret

led_off:
        lds     r16, control_byte
        ld      r15, x
        eor     r15, r16
        st      x, r15
        ret

delay_ms:
	;; call 'millis': 4-byte return value
        ;; in r25..r22
        call    millis

        ldi     yl, lo8(next_switch_after_millis)
        ldi     yh, hi8(next_switch_after_millis)

        ld      r18, y+
        ld      r19, y+
        ld      r20, y+
        ld      r21, y

        sub     r18, r22
        sbc     r19, r23
        sbc     r20, r24
        sbc     r21, r25
	brcc    delay_ms

	ldi     r17, lo8(delay)
        add     r22, r17
        ldi     r17, hi8(delay)
        adc     r23, r17
        ldi     r17, hlo8(delay)
        adc     r24, r17
        ldi     r17, hhi8(delay)
        adc     r25, r17

	ldi     yl, lo8(next_switch_after_millis)
        ldi     yh, hi8(next_switch_after_millis)

        st      y+, r22
        st      y+, r23
        st      y+, r24
        st      y+, r25

        ret

blink:
        rcall   led_on
        rcall   delay_ms
        rcall   led_off
        rcall   delay_ms
        ret

        .global loop
loop:
	push    r15
	push    r16
	push    r17
        push    yl
        push    yh
        push    xl
        push    xh

        ldi     r18, 0x0D
        sts     port_number, r18

led_loop_right:
        rcall   load_port_addr
	rcall   set_port_bit_lr
        rcall   blink

        lds     r18, port_number
        dec     r18
        sts     port_number, r18
        lds     r16, control_byte
        lsr     r16
        sts     control_byte, r16

        cpi     r18, 0x01
        brne    led_loop_right

        ldi     r18, 0x02
        sts     port_number, r18

led_loop_left:
        rcall   load_port_addr
	rcall   set_port_bit_rl
        rcall   blink

        lds     r18, port_number
        inc     r18
        sts     port_number, r18
        lds     r16, control_byte
        lsl     r16
        sts     control_byte, r16

        cpi     r18, 0x0E
        brne    led_loop_left

end:
        pop     xh
        pop     xl
        pop     yh
        pop     yl
        pop     r17
	pop     r16
        pop     r15

        ret
