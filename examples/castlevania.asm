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
dir    = $6
timer  = $7

; code
    .org $8

start:
    lda pixel0
    bne checkTimer

    lda flags
    and #$1e
    beq start

    lda #$c7
    sta pixel0

checkTimer:
    lda timer
    beq moveRight

    dec
    sta timer
    
    jmp enemyExists
moveRight:
    lda flags
    and #inpRight
    beq moveLeft

    lda pixel0
    and #$f
    sub #$f
    beq moveLeft

    lda pixel0
    inc
    sta pixel0

    lda dir
    ora #$1
    sta dir

    jmp whip
moveLeft:
    lda flags
    and #inpLeft
    beq whip

    lda pixel0
    and #$f
    beq whip

    lda pixel0
    dec
    sta pixel0

    lda dir
    and #$fe
    sta dir

whip:
    lda flags
    and #inpDown
    beq noWhip

    lda dir
    ora #$80
    sta dir

    lda #$2
    sta timer

    lda dir
    and #$1
    beq whipLeft
whipRight:
    lda pixel0
    inc
    sta pixel2

    jmp enemyExists
whipLeft:
    lda pixel0
    dec
    sta pixel2

    jmp enemyExists
noWhip:
    lda pixel0
    sta pixel2

enemyExists:
    lda pixel3
    bne moveEnemy0

    lda dir
    and #$1
    bne spawnEnemyLeft
spawnEnemyRight:
    lda #$cf
    sta pixel3
    
    lda dir
    and #$fd
    sta dir

    jmp moveEnemy0
spawnEnemyLeft:
    lda #$c0
    sta pixel3

    lda dir
    ora #$2
    sta dir

moveEnemy0:
    lda dir
    and #$4
    bne moveEnemy1

    lda dir
    ora #$4
    sta dir

    jmp setPixel4
moveEnemy1:
    lda dir
    and #$2
    beq moveEnemyLeft
moveEnemyRight:
    lda pixel3
    inc
    sta pixel3

    lda dir
    and #$fb
    sta dir

    jmp setPixel4
moveEnemyLeft:
    lda pixel3
    dec
    sta pixel3

    lda dir
    and #$fb
    sta dir

setPixel4:
    lda pixel3
    add #$10
    sta pixel4

checkPlayerCollision:
    lda pixel4
    sub pixel1
    bne setPixel1

    lda #0
    sta pixel0
    sta pixel3

setPixel1:
    lda pixel0
    add #$10
    sta pixel1

    sub pixel4
    bne checkWhipCollision

    lda #0
    sta pixel0
    sta pixel3

checkWhipCollision:
    lda pixel3
    sub pixel2
    bne endVBlank

    lda #0
    sta pixel3
    sta pixel4

endVBlank:
    lda #0
    sta flags
wait:
    lda flags
    beq wait
    jmp start