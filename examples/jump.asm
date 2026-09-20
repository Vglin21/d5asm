; input constants
inpRight = $2
inpLeft  = $4
inpDown  = $8
inpUp    = $10

; variables
flags    = $0
pixel0   = $1
pixel1   = $2
pixel2   = $3
pixel3   = $4
pixel4   = $5
rand     = $6
jumpTime = $7

; code
    .org $8

reset:
    lda #$1
    sta pixel2

start:
    lda pixel2
    and #$1
    bne isPause

    lda pixel3
    dec
    sta pixel3
    lda pixel4
    dec
    sta pixel4

    lda #0
    beq incRand
isPause:
    dec
    sta pixel3
    lda #$8
    sta pixel4

incRand:
    lda rand
    add #$d6
    sta rand

decJump:
    lda jumpTime
    beq inputJump
    dec
    sta jumpTime

inputJump:
    lda flags
    and #inpUp
    beq notJumping

    lda rand
    add pixel3
    sta rand

    lda pixel2
    and #$2
    sta pixel2
    bne inputCrouch
    
    lda jumpTime
    bne inputCrouch

    lda #$4
    sta jumpTime
    lda #$2
    sta pixel2
    bne inputCrouch
notJumping:
    lda pixel2
    and #$1
    sta pixel2

inputCrouch:
    lda flags
    and #inpDown
    beq stand

    lda pixel2
    and #$2
    sta pixel2

    lda rand
    add pixel3
    sta rand

    lda jumpTime
    bne checkJumpTime

    lda #$d2
    sta pixel0
    sta pixel1
    bne checkJumpTime
stand:
    lda #$c2
    sta pixel0
    add #$10
    sta pixel1

checkJumpTime:
    lda jumpTime
    beq isCrouching

    lda #$b2
    sta pixel0
    add #$10
    sta pixel1
    bne movePixel3
isCrouching:
    lda $1
    sub #$d2
    beq movePixel3

    lda #$c2
    sta pixel0
    add #$10
    sta pixel1

movePixel3:
    lda pixel3
    and #$f
    bne movePixel4

    lda rand
    and #$1
    bne lowPixel3
    
    lda #$cf
    sta pixel3
    bne movePixel4
lowPixel3:
    lda #$df
    sta pixel3

movePixel4:
    lda pixel4
    and #$f
    bne checkCollisions

    lda rand
    and #$1
    bne lowPixel4

    lda #$cf
    sta pixel4
    bne checkCollisions
lowPixel4:
    lda #$df
    sta pixel4

checkCollisions:
    lda pixel3
    sub pixel0
    beq collisionTrue
    
    lda pixel3
    sub pixel1
    beq collisionTrue
    
    lda pixel4
    sub pixel0
    beq collisionTrue
    
    lda pixel4
    sub pixel1
    bne endVBlank
collisionTrue:
    lda pixel2
    and #$2
    inc
    sta pixel2
    
endVBlank:
    lda #0
    sta flags
wait:
    lda flags
    beq wait
    jmp start