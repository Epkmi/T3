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
    0b00011000
};
uint8_t lifeGlyph[] = {
    0b101,
    0b111,
    0b010};


typedef struct {
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
    0b0000001
};
uint8_t ShieldGlyphU[] = {
    0b1111111,
    0b0000000,
    0b0000000,
    0b0000000,
    0b0000000,
    0b0000000,
    0b0000000
};
uint8_t ShieldGlyphD[] = {
    0b0000000,
    0b0000000,
    0b0000000,
    0b0000000,
    0b0000000,
    0b0000000,
    0b1111111
};
uint8_t ShieldGlyphL[] = {
    0b1000000,
    0b1000000,
    0b1000000,
    0b1000000,
    0b1000000,
    0b1000000,
    0b1000000
};

// Atualização da estrutura Shield
typedef struct {
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
void displayCenteredGlyph() {
    // Calcula as coordenadas do centro da tela
    int centerX = (LCDWIDTH - CENTRAL_OBJECT_WIDTH) / 2;
    int centerY = (LCDHEIGHT - CENTRAL_OBJECT_HEIGHT) / 2;

    // Exibe o caractere no centro
    for (int i = 0; i < CENTRAL_OBJECT_HEIGHT; i++) {
        for (int j = 0; j < CENTRAL_OBJECT_WIDTH; j++) {
            nokia_lcd_set_pixel(centerX + j, centerY + i, (glyph[i] >> (CENTRAL_OBJECT_WIDTH - 1 - j)) & 1);
        }
    }
}

void displayAnotherGlyph(int x, int y) {
    // Exibe o outro caractere nas coordenadas fornecidas
    for (int i = 0; i < 7; i++) {
        for (int j = 0; j < 8; j++) {
            // Certifique-se de que a coordenada não ultrapasse o tamanho da tela
            if (x + j < LCDWIDTH && y + i < LCDHEIGHT) {
                nokia_lcd_set_pixel(x + j, y + i, (anotherGlyph[i] >> (7 - j)) & 1);
            }
        }
    }
}

void displayLifeGlyphs(int lives) {
    // Exibe o caractere de vida no canto superior direito
    int startX = LCDWIDTH - 4;
    int startY = 0;

    for (int i = 0; i < lives; i++) {
        for (int j = 0; j < 3; j++) {  // Três linhas para o caractere de vida
            for (int k = 0; k < 3; k++) {  // Três colunas para o caractere de vida
                nokia_lcd_set_pixel(startX - i * 4 + k, startY + j, (lifeGlyph[j] >> (2 - k)) & 1);
            }
        }
    }
}


void clearLifeGlyph(int lives) {
    // Limpa o caractere de vida no canto superior direito
    int startX = LCDWIDTH -4- (4*(lives));
int startY=0;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
         nokia_lcd_set_pixel(startX + j, startY+i, 0);   
  }
    }        nokia_lcd_set_pixel(startX + 4, startY+4, 0);   

}

void clearAnotherGlyph(int x, int y) {
    // Limpa o outro caractere nas coordenadas fornecidas
    for (int i = 0; i < 7; i++) {
        for (int j = 0; j < 8; j++) {
            // Certifique-se de que a coordenada não ultrapasse o tamanho da tela
            if (x + j < LCDWIDTH && y + i < LCDHEIGHT) {
                nokia_lcd_set_pixel(x + j, y + i, 0);
            }
        }
    }
}

void moveObject(Object *obj, int targetX, int targetY, int *lives) {
    // Limpa a exibição anterior do objeto
    clearAnotherGlyph(obj->x, obj->y);

    // Move o objeto em direção ao alvo
    if (obj->x < targetX) {
        obj->x += MOVE_SPEED;
    } else if (obj->x > targetX) {
        obj->x -= MOVE_SPEED;
    }

    if (obj->y < targetY) {
        obj->y += MOVE_SPEED;
    } else if (obj->y > targetY) {
        obj->y -= MOVE_SPEED;
    }

    // Verifica se o objeto atingiu o centro
    if (obj->x >= targetX && obj->x < targetX + OBJECT_WIDTH &&
        obj->y >= targetY && obj->y < targetY + OBJECT_HEIGHT) {
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

void generateRandomObject(Object *objects, int *objectsInCenter) {
    // Gera um objeto apenas se a quantidade de objetos no centro for menor que 4
    if (*objectsInCenter < 4) {
        int side = rand() % 4; // 0: top, 1: bottom, 2: left, 3: right

        switch (side) {
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

char readButton() {
    // Verifique se o botão Up foi pressionado
    if (!(PIND & (1 << PD5))) {
        _delay_ms(50); // Debounce
        if (!(PIND & (1 << PD5))) {
            return 'W'; // Tecla Up
        }
    }
     if (!(PIND & (1 << PB0))) {
        _delay_ms(50); // Debounce
        if (!(PIND & (1 << PB0))) {
            return 'S'; // Tecla down
        }
    } if (!(PIND & (1 << PB6))) {
        _delay_ms(50); // Debounce
        if (!(PIND & (1 << PB6))) {
            return 'A'; // Tecla Left
        }
    } if (!(PIND & (1 << PD7))) {
        _delay_ms(50); // Debounce
        if (!(PIND & (1 << PD7))) {
            return 'D'; // Tecla Right
        }
    }

    // Faça o mesmo para os outros botões (Down, Left, Right, Reset)

    return 0; // Nenhuma tecla pressionada
}
int main(void) {
    nokia_lcd_init();
    nokia_lcd_clear();
    DDRD &= ~(1 << BTN_UP | 1 << BTN_DOWN | 1 << BTN_LEFT | 1 << BTN_RIGHT);

    srand(time(NULL)); // Inicializa a semente para números aleatórios

    Object movingObject;
    Object objects[4];  // Array para rastrear objetos
    int targetX, targetY;
    int objectsInCenter = 0;
    int lives = 3;  // Inicializa o contador de vidas
Shield playerShield = {40, 28, ShieldGlyphR, ShieldGlyphU, ShieldGlyphD, ShieldGlyphL};

       while (1) {
        // Leia os botões
        char key = readButton();

       if (key) {
    // Ações com base na tecla pressionada
    switch (key) {
        case 'W':
            shieldPosition = 1; // Cima
            break;
        case 'S':
            shieldPosition = 3; // Baixo
            break;
        case 'A':
            shieldPosition = 2; // Esquerda
            break;
        case 'D':
            shieldPosition = 0; // Direita
            break;
        // Adicione mais casos conforme necessário

        default:
            // Ação padrão (se necessário)
            break;
    }
}

// Atualize as coordenadas do escudo com base na posição
// Atualize as coordenadas do escudo com base na posição
// Atualize as coordenadas do escudo com base na posição
switch (shieldPosition) {
    case 0: // Direita
        playerShield.x = movingObject.x + OBJECT_WIDTH;  // Ajuste conforme necessário
        playerShield.y = movingObject.y;               // Ajuste conforme necessário
        playerShield.currentGlyph = ShieldGlyphR;
        break;
    case 1: // Cima
        playerShield.x = movingObject.x;               // Ajuste conforme necessário
        playerShield.y = movingObject.y - SHIELD_HEIGHT; // Ajuste conforme necessário
        playerShield.currentGlyph = ShieldGlyphU;
        break;
    case 2: // Esquerda
        playerShield.x = movingObject.x - SHIELD_WIDTH;  // Ajuste conforme necessário
        playerShield.y = movingObject.y;               // Ajuste conforme necessário
        playerShield.currentGlyph = ShieldGlyphL;
        break;
    case 3: // Baixo
        playerShield.x = movingObject.x;               // Ajuste conforme necessário
        playerShield.y = movingObject.y + OBJECT_HEIGHT; // Ajuste conforme necessário
        playerShield.currentGlyph = ShieldGlyphD;
        break;
}

// Exiba o escudo
for (int i = 0; i < SHIELD_HEIGHT; i++) {
    for (int j = 0; j < SHIELD_WIDTH; j++) {
        nokia_lcd_set_pixel(playerShield.x + j, playerShield.y + i, (playerShield.currentGlyph[i] >> (SHIELD_WIDTH - 1 - j)) & 1);
    }
}


// Certifique-se de que o escudo permaneça dentro dos limites da tela
if (playerShield.x < 0) playerShield.x = 0;
if (playerShield.x > LCDWIDTH - SHIELD_WIDTH) playerShield.x = LCDWIDTH - SHIELD_WIDTH;
if (playerShield.y > LCDHEIGHT - SHIELD_HEIGHT) playerShield.y = LCDHEIGHT - SHIELD_HEIGHT;


        // Gere um objeto a cada 5 segundos em uma direção aleatória
        if (rand() % 100 < 20) { // Ajuste o valor para controlar a frequência de geração
            generateRandomObject(objects, &objectsInCenter);
            targetX = (LCDWIDTH - OBJECT_WIDTH) / 2;
            targetY = (LCDHEIGHT - OBJECT_HEIGHT) / 2;
        }

        // Mova todos os objetos em direção ao centro
        for (int i = 0; i < objectsInCenter; i++) {
            moveObject(&objects[i], targetX, targetY, &lives);
            // Exiba cada objeto em sua nova posição
            displayAnotherGlyph(objects[i].x, objects[i].y);
        }

        // Exiba o outro Glyph em uma posição fixa (por exemplo, canto superior direito)
        displayAnotherGlyph(0, 0);

        // Exiba o Glyph centralizado
        displayCenteredGlyph();

        // Exiba o contador de vidas
        displayLifeGlyphs(lives);

        // Atualize o display
        nokia_lcd_render();

        if (lives == 0) {
            nokia_lcd_write_string("ACABOU BROTHER DESISTE", 1);
            return 1;
        }

        // Aguarde um curto período para dar a sensação de movimento suave
        _delay_ms(100);
    }
}
