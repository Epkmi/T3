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
uint8_t ShieldGlyph[] = {
    0b001,
    0b001,
    0b001
};

typedef struct {
    int x;
    int y;
} Object;
typedef struct {
    int x;
    int y;
    char direction;  // 'U' para cima, 'D' para baixo, 'L' para esquerda, 'R' para direita
} Shield;

Shield playerShield;


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
int isCollisionWithShield(Object *obj) {
    int shieldXStart = playerShield.x - 2;
    int shieldXEnd = playerShield.x + CENTRAL_OBJECT_WIDTH + 2;
    int shieldYStart = playerShield.y - 2;
    int shieldYEnd = playerShield.y + CENTRAL_OBJECT_HEIGHT + 2;

    return (obj->x + OBJECT_WIDTH > shieldXStart && obj->x < shieldXEnd &&
            obj->y + OBJECT_HEIGHT > shieldYStart && obj->y < shieldYEnd);
}

void moveObject(Object *obj, int targetX, int targetY, int *lives) {
    // Limpa a exibição anterior do objeto
    clearAnotherGlyph(obj->x, obj->y);

    // Verifica se o objeto atingiu o escudo
    if (isCollisionWithShield(obj)) {
        // Remove o objeto
        obj->x = -1;  // Posição fora da tela
        obj->y = -1;
    } else {

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
    char key = 0; // Declare key antes de usá-la


    // Verifique se o botão Up foi pressionado
    if (!(PIND & (1 << PD5))) {
        _delay_ms(50); // Debounce
        if (!(PIND & (1 << PD5))) {
            key = 'w'; // Tecla Up
        }
    } else if (!(PIND & (1 << PB0))) {
        _delay_ms(50); // Debounce
        if (!(PIND & (1 << PB0))) {
            key = 's'; // Tecla down
        }
    } else if (!(PIND & (1 << PB6))) {
        _delay_ms(50); // Debounce
        if (!(PIND & (1 << PB6))) {
            key = 'a'; // Tecla Left
        }
    } else if (!(PIND & (1 << PD7))) {
        _delay_ms(50); // Debounce
        if (!(PIND & (1 << PD7))) {
            key = 'd'; // Tecla Right
        }
    }

    return key; // Certifique-se de retornar key mesmo se nenhum botão for pressionado
}
void displayShieldGlyph() {
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            nokia_lcd_set_pixel(playerShield.x + j, playerShield.y + i, (ShieldGlyph[i] >> (2 - j)) & 1);
        }
    }
}
void updateShieldPosition(char key) {
    // Atualiza a posição do escudo com base na tecla pressionada
    switch (key) {
        case 'w':
            playerShield.y--;
            playerShield.direction = 'U';
            break;
        case 's':
            playerShield.y++;
            playerShield.direction = 'D';
            break;
        case 'a':
            playerShield.x--;
            playerShield.direction = 'L';
            break;
        case 'd':
            playerShield.x++;
            playerShield.direction = 'R';
            break;
        // Adicione mais casos conforme necessário
    }
}


int main(void) {
    nokia_lcd_init();
    nokia_lcd_clear();
    DDRD &= ~(1 << BTN_UP | 1 << BTN_DOWN | 1 << BTN_LEFT | 1 << BTN_RIGHT);

    srand(time(NULL));

    Object movingObject;
    Object objects[4];
    int targetX, targetY;
    int objectsInCenter = 0;
    int lives = 3;

    // Configuração inicial para o GlyphShield
  playerShield.x = (LCDWIDTH - 3) / 2;  // 3 é a largura do ShieldGlyph
playerShield.y = (LCDHEIGHT - 3) / 2; // 3 é a altura do ShieldGlyph

    // Exibe o GlyphShield fora do loop principal
    displayShieldGlyph();
    nokia_lcd_render();

    while (1) {
        char key = readButton();

        if (key) {
            updateShieldPosition(key);

            switch (key) {
                case 'w':
                    break;
                case 's':
                    break;
                case 'a':
                    break;
                case 'd':
                    break;
                default:
                    break;
            }
        }

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

        // Atualiza a posição do GlyphShield
        displayShieldGlyph();

        // Atualize o display
        nokia_lcd_render();

        if (lives == 0) {
            nokia_lcd_write_string("ACABOU BROTHER DESISTE", 1);
            return 1;
        }

        // Aguarde um curto período para dar a sensação de movimento suave
        _delay_ms(20);
    }
}