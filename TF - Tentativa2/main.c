#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <time.h>
#include "nokia5110.h"

#define LCDWIDTH 84
#define LCDHEIGHT 48
#define OBJECT_WIDTH 5
#define OBJECT_HEIGHT 5
#define CENTRAL_OBJECT_WIDTH 8
#define CENTRAL_OBJECT_HEIGHT 5

// Defina a velocidade de movimento do objeto
#define MOVE_SPEED 1

#define BTN_UP PD5
#define BTN_DOWN PB0
#define BTN_LEFT PD6
#define BTN_RIGHT PD7
#define BTN_RESET PD4
int targetX, targetY; // Declare targetX and targetY here

uint8_t glyph[] = {
    0b00011000,
    0b00011000,
    0b01111110,
    0b10011001,
    0b00100100};
uint8_t anotherGlyph[] = {
    0b00011000,
    0b00111100,
    0b01100110,
    0b11100111,
    0b01100110,
    0b00111100,
    0b00011000};
uint8_t lifeGlyph[] = {
    0b101,
    0b111,
    0b010};

typedef struct
{
    int x;
    int y;
} Object;

uint8_t ShieldGlyphR[] = {
    0b0000001,
    0b0000001,
    0b0000001,
    0b0000001,
    0b0000001,
    0b0000001,
    0b0000001};
uint8_t ShieldGlyphU[] = {
    0b1111111,
    0b0000000,
    0b0000000,
    0b0000000,
    0b0000000,
    0b0000000,
    0b0000000};
uint8_t ShieldGlyphD[] = {
    0b0000000,
    0b0000000,
    0b0000000,
    0b0000000,
    0b0000000,
    0b0000000,
    0b1111111};
uint8_t ShieldGlyphL[] = {
    0b1000000,
    0b1000000,
    0b1000000,
    0b1000000,
    0b1000000,
    0b1000000,
    0b1000000};

// Atualização da estrutura Shield
typedef struct
{
    int x;
    int y;
    uint8_t *currentGlyph;
    uint8_t *glyphR;
    uint8_t *glyphU;
    uint8_t *glyphD;
    uint8_t *glyphL;
} Shield;

int shieldPosition = 0; // 0: direita, 1: cima, 2: esquerda, 3: baixo

#define SHIELD_WIDTH 7
#define SHIELD_HEIGHT 7
#define GAME_SCREEN 1
#define GAME_OVER_SCREEN 2
#define START_SCREEN 0

int gameState = START_SCREEN; // Adiciona uma variável para controlar o estado do jogo

void displayCenteredGlyph()
{
    // Calcula as coordenadas do centro da tela
    int centerX = (LCDWIDTH - CENTRAL_OBJECT_WIDTH) / 2;
    int centerY = (LCDHEIGHT - CENTRAL_OBJECT_HEIGHT) / 2;

    // Exibe o caractere no centro
    for (int i = 0; i < CENTRAL_OBJECT_HEIGHT; i++)
    {
        for (int j = 0; j < CENTRAL_OBJECT_WIDTH; j++)
        {
            nokia_lcd_set_pixel(centerX + j, centerY + i, (glyph[i] >> (CENTRAL_OBJECT_WIDTH - 1 - j)) & 1);
        }
    }
}

void displayAnotherGlyph(int x, int y)
{
    // Exibe o outro caractere nas coordenadas fornecidas
    for (int i = 0; i < 7; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            // Certifique-se de que a coordenada não ultrapasse o tamanho da tela
            if (x + j < LCDWIDTH && y + i < LCDHEIGHT)
            {
                nokia_lcd_set_pixel(x + j, y + i, (anotherGlyph[i] >> (7 - j)) & 1);
            }
        }
    }
}

void displayLifeGlyphs(int lives)
{
    // Exibe o caractere de vida no canto superior direito
    int startX = LCDWIDTH - 4;
    int startY = 0;

    for (int i = 0; i < lives; i++)
    {
        for (int j = 0; j < 3; j++)
        { // Três linhas para o caractere de vida
            for (int k = 0; k < 3; k++)
            { // Três colunas para o caractere de vida
                nokia_lcd_set_pixel(startX - i * 4 + k, startY + j, (lifeGlyph[j] >> (2 - k)) & 1);
            }
        }
    }
}

void clearLifeGlyph(int lives)
{
    // Limpa o caractere de vida no canto superior direito
    int startX = LCDWIDTH - 4 - (4 * (lives));
    int startY = 0;
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            nokia_lcd_set_pixel(startX + j, startY + i, 0);
        }
    }
    nokia_lcd_set_pixel(startX + 4, startY + 4, 0);
}

void clearAnotherGlyph(int x, int y)
{
    // Limpa o outro caractere nas coordenadas fornecidas
    for (int i = 0; i < 7; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            // Certifique-se de que a coordenada não ultrapasse o tamanho da tela
            if (x + j < LCDWIDTH && y + i < LCDHEIGHT)
            {
                nokia_lcd_set_pixel(x + j, y + i, 0);
            }
        }
    }
}

void moveObject(Object *obj, int targetX, int targetY, int *lives)
{
    // Limpa a exibição anterior do objeto
    clearAnotherGlyph(obj->x, obj->y);

    // Move o objeto em direção ao alvo
    if (obj->x < targetX)
    {
        obj->x += MOVE_SPEED;
    }
    else if (obj->x > targetX)
    {
        obj->x -= MOVE_SPEED;
    }

    if (obj->y < targetY)
    {
        obj->y += MOVE_SPEED;
    }
    else if (obj->y > targetY)
    {
        obj->y -= MOVE_SPEED;
    }

    // Verifica se o objeto atingiu o centro
    if (obj->x >= targetX && obj->x < targetX + OBJECT_WIDTH &&
        obj->y >= targetY && obj->y < targetY + OBJECT_HEIGHT)
    {
        // Reduz uma vida
        (*lives)--;
        // Reinicia a posição do objeto
        obj->x = rand() % (LCDWIDTH - OBJECT_WIDTH);
        obj->y = rand() % (LCDHEIGHT - OBJECT_HEIGHT);
        // Limpa o caractere de vida correspondente
        clearLifeGlyph(*lives);
    }
    _delay_ms(30);
}

void generateRandomObject(Object *objects, int *objectsInCenter)
{
    // Gera um objeto apenas se a quantidade de objetos no centro for menor que 4
    if (*objectsInCenter < 4)
    {
        int side = rand() % 4; // 0: top, 1: bottom, 2: left, 3: right

        switch (side)
        {
        case 0:
            // Topo
            objects[*objectsInCenter].x = rand() % (LCDWIDTH - OBJECT_WIDTH);
            objects[*objectsInCenter].y = 0;
            break;
        case 1:
            // Base
            objects[*objectsInCenter].x = rand() % (LCDWIDTH - OBJECT_WIDTH);
            objects[*objectsInCenter].y = LCDHEIGHT - OBJECT_HEIGHT;
            break;
        case 2:
            // Esquerda
            objects[*objectsInCenter].x = 0;
            objects[*objectsInCenter].y = rand() % (LCDHEIGHT - OBJECT_HEIGHT);
            break;
        case 3:
            // Direita
            objects[*objectsInCenter].x = LCDWIDTH - OBJECT_WIDTH;
            objects[*objectsInCenter].y = rand() % (LCDHEIGHT - OBJECT_HEIGHT);
            break;
        }

        (*objectsInCenter)++;
    }
}

char readButton()
{
    if (!(PIND & (1 << BTN_UP))){    // Verifique se o botão Up foi pressionado
        _delay_ms(50); // Debounce
        if (!(PIND & (1 << BTN_UP))){
            return 'W';} // Tecla Up        
    }
    // Verifique se o botão Down foi pressionado
    if (!(PINB & (1 << BTN_DOWN))){
        _delay_ms(50); // Debounce
        if (!(PINB & (1 << BTN_DOWN))){
            return 'S';} // Tecla Down   
    }
    // Verifique se o botão Left foi pressionado
    if (!(PIND & (1 << BTN_LEFT))){
        _delay_ms(50); // Debounce
        if (!(PIND & (1 << BTN_LEFT))){
            return 'A';} // Tecla Left
    }
    // Verifique se o botão Right foi pressionado
    if (!(PIND & (1 << BTN_RIGHT))){
        _delay_ms(50); // Debounce
        if (!(PIND & (1 << BTN_RIGHT))){
            return 'D';} // Tecla Right
    }
    return 0; // Nenhuma tecla pressionada
}
void displayShield(Shield *shield)
{
    for (int i = 0; i < SHIELD_HEIGHT; i++)
    {
        for (int j = 0; j < SHIELD_WIDTH; j++)
        {
            int pixelValue = (shield->currentGlyph[i] >> (SHIELD_WIDTH - 1 - j)) & 1;
            nokia_lcd_set_pixel(shield->x + j, shield->y + i, pixelValue);
        }
    }
}

void clearShield(Shield *shield)
{
    for (int i = 0; i < SHIELD_HEIGHT; i++)
    {
        for (int j = 0; j < SHIELD_WIDTH; j++)
        {
            nokia_lcd_set_pixel(shield->x + j, shield->y + i, 0);
        }
    }
}
void updateShieldPosition(Shield *shield, int position)
{
    // Limpa a exibição anterior do escudo
    clearShield(shield);

    // Atualiza a posição do escudo
    switch (position)
    {
    case 0: // Direita
        shield->currentGlyph = shield->glyphR;
        shield->x = (LCDWIDTH / 2) + 10; // Defina a posição desejada
        shield->y = LCDHEIGHT / 2;
        break;
    case 1: // Cima
        shield->currentGlyph = shield->glyphU;
        shield->x = (LCDWIDTH / 2); // Defina a posição desejada
        shield->y = (LCDHEIGHT / 2) + 10;
        break;
    case 2: // Esquerda
        shield->currentGlyph = shield->glyphL;
        shield->x = (LCDWIDTH / 2) - 10; // Defina a posição desejada
        shield->y = LCDHEIGHT / 2;
        break;
    case 3: // Baixo
        shield->currentGlyph = shield->glyphD;
        shield->x = (LCDWIDTH / 2); //// Defina a posição desejada
        shield->y = (LCDHEIGHT / 2) - 10;
        break;
    default:
        break;
    }

    // Exibe o escudo na nova posição
    displayShield(shield);
}
void displayStartScreen()
{
    nokia_lcd_clear();
    nokia_lcd_set_cursor(10, 10);
    nokia_lcd_write_string("Pressione uma tecla", 1);
    nokia_lcd_render();
    _delay_ms(30);
}

void displayGameOverScreen()
{
    nokia_lcd_clear();
    nokia_lcd_set_cursor(15, 20);
    nokia_lcd_write_string("GAME ", 1);
    nokia_lcd_write_string("OVER", 1);
    nokia_lcd_set_cursor(34, 30);
    nokia_lcd_write_string("BRO", 1);
    nokia_lcd_render();
    _delay_ms(2000);
}

void displayGameScreen(Shield *playerShield, Object *objects, int *objectsInCenter, int *lives)
{
    nokia_lcd_clear();

    updateShieldPosition(playerShield, shieldPosition);

    // Exibe o escudo
    displayShield(playerShield);

    // Atualiza a posição do escudo com base na tecla pressionada
    updateShieldPosition(playerShield, shieldPosition);

    // Gera um objeto a cada 5 segundos em uma direção aleatória
    if (rand() % 100 < 20)
    {
        generateRandomObject(objects, objectsInCenter);
        int targetX = (LCDWIDTH - OBJECT_WIDTH) / 2;
        int targetY = (LCDHEIGHT - OBJECT_HEIGHT) / 2;
    }

    // Mova todos os objetos em direção ao centro
    for (int i = 0; i < *objectsInCenter; i++)
    {
        moveObject(&objects[i], targetX, targetY, lives);
        displayAnotherGlyph(objects[i].x, objects[i].y);
    }

    // Exiba o outro Glyph em uma posição fixa (por exemplo, canto superior direito)
    displayAnotherGlyph(0, 0);

    // Exiba o Glyph centralizado
    displayCenteredGlyph();

    // Exiba o contador de vidas
    displayLifeGlyphs(*lives);

    // Atualize o display
    nokia_lcd_render();

    // Aguarde um curto período para dar a sensação de movimento suave
    _delay_ms(100);
}

int main(void)
{
    nokia_lcd_init();
    nokia_lcd_clear();
    DDRD &= ~(1 << BTN_UP | 1 << BTN_DOWN | 1 << BTN_LEFT | 1 << BTN_RIGHT);

    srand(time(NULL)); // Inicializa a semente para números aleatórios

    Object movingObject;
    Object objects[4]; // Array para rastrear objetos
    int objectsInCenter = 0;
    int lives = 3; // Inicializa o contador de vidas
    Shield playerShield = {40, 28, ShieldGlyphR, ShieldGlyphU, ShieldGlyphD, ShieldGlyphL};
    gameState = START_SCREEN;

    while (1)
    {
        targetX = (LCDWIDTH - OBJECT_WIDTH) / 2; // Initialize targetX
        targetY = (LCDHEIGHT - OBJECT_HEIGHT) / 2; // Initialize targetY

        if (gameState == START_SCREEN)
        {
            displayStartScreen();
        }

        char key = readButton();

        if (key)
        {
            switch (key)
            {
            case 'W':
                shieldPosition = 1; // Cima
                gameState = GAME_SCREEN;
                _delay_ms(30);
                break;
            case 'S':
                shieldPosition = 3; // Baixo
                gameState = GAME_SCREEN;
                _delay_ms(30);
                break;
            case 'A':
                shieldPosition = 2; // Esquerda
                gameState = GAME_SCREEN;
                _delay_ms(30);
                break;
            case 'D':
                shieldPosition = 0; // Direita
                gameState = GAME_SCREEN;
                _delay_ms(30);
                break;
            default:
                gameState = START_SCREEN;
                _delay_ms(30);
                break;
            }
        }

        if (gameState == GAME_SCREEN)
        {
            displayGameScreen(&playerShield, objects, &objectsInCenter, &lives);

            if (lives == 0)
            {
                gameState = GAME_OVER_SCREEN;
            }
        }

        if (gameState == GAME_OVER_SCREEN)
        {
            displayGameOverScreen();
            return 1;
        }
    }
}