; input constants
inpRight = $2
inpLeft  = $4
inpDown  = $8
inpUp    = $10

; variables
flags  = $0
pixel0 = $1
pixel1 = $2
pixel2 = $3
pixel3 = $4
pixel4 = $5
jump   = $6
rand   = $7

; code
    .org $8

reset:
    lda #$d7
    sta pixel0
    lda #$e6
    sta pixel1
    lda #$68
    sta pixel3
    lda #$80
    sta jump

start:
    lda rand
    add #$b6
    sta rand

    lda flags
    and #inpUp
    beq isPause

    lda rand
    add #$ae
    sta rand

    lda jump
    and #$80
    beq isPause
    lda #$0b
    sta jump

isPause:
    lda jump
    and #$40
    bne reset

moveLeft:
    lda flags
    and #inpLeft
    beq moveRight

    lda rand
    add #$79
    sta rand

    lda pixel0
    and #$f
    beq moveRight

    lda pixel0
    dec
    sta pixel0

moveRight:
    lda flags
    and #inpRight
    beq decJump

    lda rand
    add #$99
    sta rand

    lda pixel0
    and #$f
    sub #$f
    beq decJump

    lda pixel0
    inc
    sta pixel0

decJump:
    lda jump
    and #$f
    beq moveDown

    lda jump
    dec
    sta jump

    lda pixel0
    and #$f0
    sub #$70
    beq scroll

    lda pixel0
    sub #$10
    sta pixel0
    jmp updatePlatforms
scroll:
    lda pixel1
    add #$10
    sta pixel1
    and #$f0
    sub #$f0
    bne scroll0

    lda rand
    and #$f
    sta pixel1

scroll0:
    lda pixel3
    add #$10
    sta pixel3
    and #$f0
    sub #$f0
    bne updatePlatforms

    lda rand
    and #$f
    sta pixel3

    jmp updatePlatforms

moveDown:
    lda pixel0
    add #$10
    sta pixel0
    and #$f0
    sub #$f0
    bne updatePlatforms
    lda #$c0
    sta jump

updatePlatforms:
    lda pixel1
    inc
    sta pixel2
    lda pixel3
    inc
    sta pixel4

    lda jump
    and #$f
    bne collisionFalse

checkCollisions:
    lda pixel0
    sub pixel1
    beq collisionTrue

    lda pixel0
    sub pixel2
    beq collisionTrue

    lda pixel0
    sub pixel3
    beq collisionTrue

    lda pixel0
    sub pixel4
    bne collisionFalse
collisionTrue:
    lda #$80
    sta jump

    lda pixel0
    sub #$10
    sta pixel0

    jmp endVBlank
collisionFalse:
    lda jump
    and #$4f
    sta jump

endVBlank:
    lda #0
    sta flags
wait:
    lda flags
    beq wait
    jmp start
