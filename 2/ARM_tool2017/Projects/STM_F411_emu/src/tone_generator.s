.syntax unified
.cpu cortex-m4

.equ CPU_FREQ, 84000000

wait_us:

.global beep

.EXTERN beep

beep:
// m:n period
// r0 = m periods [us]
// r1 = n periods [us]
// r2 = period count
        push {r0-r9, lr}

        LDR R0, =0x40020000
        MOV R1, #32
        STR R1, [R0, #0x18]


        LDR R3, = 10
    wait:
        SUB R3, R3, #1
        CMP R3, #0
        BNE wait


        STR R1, [R0, #0x1A]

        ;LDR R3, =84000000
    ;wait1:
        ;SUB R3, R3, #1
        ;BNE wait1

        pop {r0-r9, pc}

