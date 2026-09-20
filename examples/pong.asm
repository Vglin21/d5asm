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
dirX   = $6
dirY   = $7

; code
    .org $8

reset:
    lda #$7f
    sta pixel2

start:
    lda flags
    and #inpDown
    beq moveUp

    lda pixel0
    add #$10
    sta pixel0

moveUp:
    lda flags
    and #inpUp
    beq setPixel1

    lda pixel0
    sub #$10
    sta pixel0

setPixel1:
    lda pixel0
    add #$10
    sta pixel1

moveBallX:
    lda dirX
    beq moveBallLeft

    lda pixel4
    inc
    sta pixel4

    lda #0
    beq moveBallY
moveBallLeft:
    lda pixel4
    dec
    sta pixel4

moveBallY:
    lda dirY
    beq moveBallUp

    lda pixel4
    add #$10
    sta pixel4

    lda #0
    beq checkBoundsY
moveBallUp:
    lda pixel4
    sub #$10
    sta pixel4

checkBoundsY:
    lda pixel4
    and #$f0
    bne isBallBottom

    lda #$1
    sta dirY
    bne imDone

isBallBottom:
    sub #$f0
    bne imDone

    lda #0
    sta dirY

imDone:
    lda pixel4
    and #$f
    bne isBallLeft
    lda dirX
    beq yeah
    lda dirY
    beq yeah

    lda pixel4
    sub #$10
    sta pixel4

    lda #0
    beq yeah

isBallLeft:
    sub #$f
    bne yeah
    lda dirX
    bne yeah
    lda dirY
    bne yeah

    lda pixel4
    add #$10
    sta pixel4

yeah:
    lda pixel4
    and #$f0
    sta flags

checkCollisionPixel0:
    lda pixel4
    dec
    sub pixel0
    bne checkCollisionPixel1

    lda #1
    sta dirX
    dec
    sta dirY

checkCollisionPixel1:
    lda pixel4
    dec
    sub pixel1
    bne checkCollisionPixel2

    lda #1
    sta dirX
    sta dirY

checkCollisionPixel2:
    lda pixel4
    inc
    sub pixel2
    bne checkCollisionPixel3

    lda #0
    sta dirX
    sta dirY

checkCollisionPixel3:
    lda pixel4
    inc
    sub pixel3
    bne movePixel2

    lda #0
    sta dirX
    inc
    sta dirY

movePixel2:
    lda pixel2
    and #$f0
    sub flags
    bne movePixel3

    lda dirY
    bne movePixel3

    lda pixel2
    sub #$10
    sta pixel2

movePixel3:
    lda pixel3
    and #$f0
    sub flags
    bne setPixel3

    lda dirY
    beq setPixel3

    lda pixel2
    add #$10
    sta pixel2

setPixel3:
    lda pixel2
    add #$10
    sta pixel3
  
endVBlank:
    lda #0
    sta flags
wait:
    lda flags
    beq wait
    jmp start