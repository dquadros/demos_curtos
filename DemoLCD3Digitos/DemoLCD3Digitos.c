// Demo de uso de mini LCD com 3 dígitos

#include "pico/stdlib.h"

// Taxa de atualização do LCD
#define LCD_FREQ 100
#define LCD_TIME_MS (1000/LCD_FREQ)

// Conexões ao LCD
// COM1 = 12, COM2 = 13, COM3 = 14, COM4 = 15
// SEG1 = 16, SEG2 = 17, SEG3 = 18, SEG4 = 19, SEG5 = 20, SEG6 = 21
#define COM_MASK 0x0000F000
#define SEG_MASK 0x003F0000
#define PIN_COM1 12     // Assume que os demais COM são sequenciais

// Programação dos segmentos conforme o valor a apresentar em cada dígito
static const uint32_t seg_digito[3][10][4] = {
    {   // Primeiro dígito
        { 0x00200000, 0x00300000, 0x00100000, 0x00300000 }, // 0
        { 0x00000000, 0x00100000, 0x00100000, 0x00000000 }, // 1
        { 0x00200000, 0x00200000, 0x00300000, 0x00100000 }, // 2
        { 0x00200000, 0x00100000, 0x00300000, 0x00100000 }, // 3
        { 0x00000000, 0x00100000, 0x00300000, 0x00200000 }, // 4
        { 0x00200000, 0x00100000, 0x00200000, 0x00300000 }, // 5
        { 0x00200000, 0x00300000, 0x00200000, 0x00300000 }, // 6
        { 0x00000000, 0x00100000, 0x00100000, 0x00100000 }, // 7
        { 0x00200000, 0x00300000, 0x00300000, 0x00300000 }, // 8
        { 0x00200000, 0x00100000, 0x00300000, 0x00300000 }  // 9
    },
    {   // Segundo dígito
        { 0x00080000, 0x000C0000, 0x00040000, 0x000C0000 }, // 0
        { 0x00000000, 0x00040000, 0x00040000, 0x00000000 }, // 1
        { 0x00080000, 0x00080000, 0x000C0000, 0x00040000 }, // 2
        { 0x00080000, 0x00040000, 0x000C0000, 0x00040000 }, // 3
        { 0x00000000, 0x00040000, 0x000C0000, 0x00080000 }, // 4
        { 0x00080000, 0x00040000, 0x00080000, 0x000C0000 }, // 5
        { 0x00080000, 0x000C0000, 0x00080000, 0x000C0000 }, // 6
        { 0x00000000, 0x00040000, 0x00040000, 0x00040000 }, // 7
        { 0x00080000, 0x000C0000, 0x000C0000, 0x000C0000 }, // 8
        { 0x00080000, 0x00040000, 0x000C0000, 0x000C0000 }  // 9
    },
    {   // Terceiro dígito
        { 0x00020000, 0x00030000, 0x00010000, 0x00030000 }, // 0
        { 0x00000000, 0x00010000, 0x00010000, 0x00000000 }, // 1
        { 0x00020000, 0x00020000, 0x00030000, 0x00010000 }, // 2
        { 0x00020000, 0x00010000, 0x00030000, 0x00010000 }, // 3
        { 0x00000000, 0x00010000, 0x00030000, 0x00020000 }, // 4
        { 0x00020000, 0x00010000, 0x00020000, 0x00030000 }, // 5
        { 0x00020000, 0x00030000, 0x00020000, 0x00030000 }, // 6
        { 0x00000000, 0x00010000, 0x00010000, 0x00010000 }, // 7
        { 0x00020000, 0x00030000, 0x00030000, 0x00030000 }, // 8
        { 0x00020000, 0x00010000, 0x00030000, 0x00030000 }  // 9
    }
};

// Timer to generate LCD signals
static struct repeating_timer timer;

// Guarda a programação dos segmentos
static uint32_t segtos [4] = { SEG_MASK, SEG_MASK, SEG_MASK, SEG_MASK };

// Rotinas locais
static void init(void);
static bool updateLCD(struct repeating_timer *t);
static void mostraLCD(char *valor);

int main()
{
    char valor[4] = "000";

    init();
    while (true) {
        mostraLCD(valor);
        for (int i = 2; i >= 0; i--) {
            if (valor[i] < '9') {
                valor[i]++;
                break;
            }
            valor[i] = '0';
        }
        sleep_ms(1000);
    }
}

// Iniciação
void init() {
    // GPIO
    gpio_init_mask (SEG_MASK | COM_MASK);
    gpio_set_dir_masked (SEG_MASK | COM_MASK, SEG_MASK);

    // Dispara rotina para atualizar o LCD
    add_repeating_timer_ms(LCD_TIME_MS, updateLCD, NULL, &timer);
}

// Atualiza o LCD
bool updateLCD(struct repeating_timer *t) {
    static int fase = 0;

    // Atualiza sinais COM
    // apenas um COM será saída, alternando entre 1 e 0
    int pin_com = (PIN_COM1 + (fase & 0x03));
    uint32_t mask = 1 << pin_com;
    gpio_set_dir_masked (COM_MASK, mask);
    gpio_put (pin_com, (fase & 4) == 0);

    // Atualiza sinais seg
    // Inverte nas 4 fases finais
    uint32_t seg = (fase & 4) != 0 ? segtos[fase & 3] : segtos[fase & 3] ^ SEG_MASK;
    gpio_put_masked (SEG_MASK, seg);

    // Passa para a próxima fase
    fase = (fase + 1) & 7;

    // Continuar chamando
    return true;    
}

static void mostraLCD(char *valor) {
    uint32_t new_segtos[4] = { 0, 0, 0, 0 };

    for (int pos = 0; pos < 3; pos++) {
        const uint32_t *val = seg_digito[pos][valor[pos]-'0'];
        for (int i = 0; i < 4; i++) {
            new_segtos[i] |= val[i];
        }
    }
    for (int i = 0; i < 4; i++) {
        segtos[i] = new_segtos[i];
    }
}
