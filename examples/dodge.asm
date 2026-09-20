; constants
inpRight   = $2
inpLeft    = $4
inpDown    = $8
inpUp      = $10

; variables
flags    = $0
pixel0   = $1
pixel1   = $2
pixel2   = $3
pixel3   = $4
pixel4   = $5
rand     = $6
turnTime = $7

; code
    .org $8

reset:
    lda #$d7
    sta pixel0
    lda #$d
    sta pixel1
    lda #$58
    sta pixel2
    lda #$a3
    sta pixel3
    lda #1
    sta pixel4

start:
    lda flags
    and #inpRight
    beq moveLeft
    
    lda rand
    add pixel1
    sta rand
    
    lda #0
    sta pixel4

    lda pixel0
    and #$f
    sub #$f
    beq moveLeft
    
    lda pixel0
    inc
    sta pixel0

moveLeft:
    lda flags
    and #inpLeft
    beq checkPause
    
    lda rand
    add pixel2
    sta rand
    
    lda #0
    sta pixel4

    lda pixel0
    and #$f
    beq checkPause
    
    lda pixel0
    dec
    sta pixel0

checkPause:
    lda pixel4
    beq movePixel1
    jmp reset

movePixel1:
    lda pixel1
    and #$f0
    bne movePixel2

    lda rand
    and #$f
    sta pixel1

    lda rand
    add #$5e
    sta rand

movePixel2:
    lda pixel2
    and #$f0
    bne movePixel3

    lda rand
    and #$f
    sta pixel2

    lda rand
    add #$99
    sta rand

movePixel3:
    lda pixel3
    and #$f0
    bne checkDir

    lda rand
    and #$f
    sta pixel3

    lda rand
    add #$eb
    sta rand

checkDir:
    lda turnTime
    and #$f
    bne decCounter

    lda turnTime
    and #$80
    bne turnRight

    lda #$85
    sta turnTime
    bne decCounter
turnRight:
    lda #$5
    sta turnTime

decCounter:
    lda turnTime
    dec
    sta turnTime

movePixelsLeft:
    lda turnTime
    and #$80
    beq movePixelsRight

    lda pixel1
    add #$f
    sta pixel1
    lda pixel2
    add #$f
    sta pixel2
    lda pixel3
    add #$f
    sta pixel3

    lda #0
    beq checkCollisions

movePixelsRight:
    lda pixel1
    add #$11
    sta pixel1
    lda pixel2
    add #$11
    sta pixel2
    lda pixel3
    add #$11
    sta pixel3

checkCollisions:
    lda pixel1
    sub pixel0
    beq collisionTrue

    lda pixel2
    sub pixel0
    beq collisionTrue

    lda pixel3
    sub pixel0
    bne endVBlank
collisionTrue:
    lda #1
    sta pixel4

endVBlank:
    lda #0
    sta flags
wait:
    lda flags
    beq wait
    jmp start