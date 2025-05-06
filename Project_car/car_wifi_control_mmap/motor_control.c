#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>

typedef struct { uint32_t status, ctrl; } GPIOregs;
typedef struct { uint32_t Out, OE, In, InSync; } rioregs;

volatile uint32_t *GPIOBase, *RIOBase;

#define GPIO   ((GPIOregs *) GPIOBase)
#define rio    ((rioregs *)  RIOBase)
#define rioSET ((rioregs *) (RIOBase + 0x2000 / 4))
#define rioCLR ((rioregs *) (RIOBase + 0x3000 / 4))

#define SET(pin) rioSET->Out = (1 << pin)
#define CLR(pin) rioCLR->Out = (1 << pin)
#define OUT(pin) rioSET->OE  = (1 << pin)

void init_gpio(uint32_t *pad, uint32_t pin, uint32_t fn) {
    GPIO[pin].ctrl = fn;     
    pad[pin] = 0x10;         
    OUT(pin);                
}

void stop_all(uint32_t a1, uint32_t a2, uint32_t b1, uint32_t b2) {
    CLR(a1); CLR(a2);
    CLR(b1); CLR(b2);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("command: %s [F|B|L|R|S]\n", argv[0]);
        return 1;
    }

    char cmd = argv[1][0];

    int memfd = open("/dev/mem", O_RDWR | O_SYNC);
    if (memfd < 0) {
        perror("open /dev/mem");
        return 1;
    }

    uint32_t *map = mmap(NULL, 64 * 1024 * 1024, PROT_READ | PROT_WRITE, MAP_SHARED, memfd, 0x1f00000000);
    if (map == MAP_FAILED) {
        perror("mmap failed");
        return 1;
    }
    close(memfd);

    uint32_t *PERIBase = map;
    GPIOBase = PERIBase + 0xD0000 / 4;
    RIOBase  = PERIBase + 0xE0000 / 4;
    uint32_t *PADBase = PERIBase + 0xF0000 / 4;
    uint32_t *pad = PADBase + 1;
    uint32_t fn = 5;

    uint32_t A_IN1 = 14, A_IN2 = 15, A_ENA = 25;

    uint32_t B_IN1 = 26, B_IN2 = 27, B_ENB = 22;

    init_gpio(pad, A_IN1, fn);
    init_gpio(pad, A_IN2, fn);
    init_gpio(pad, A_ENA, fn);
    init_gpio(pad, B_IN1, fn);
    init_gpio(pad, B_IN2, fn);
    init_gpio(pad, B_ENB, fn);

    SET(A_ENA);
    SET(B_ENB);

    stop_all(A_IN1, A_IN2, B_IN1, B_IN2);


    switch (cmd) {
        case 'F': // å‰é€²
            SET(A_IN1); CLR(A_IN2);
            SET(B_IN1); CLR(B_IN2);
            break;
        case 'B':€€
            CLR(A_IN1); SET(A_IN2);
            CLR(B_IN1); SET(B_IN2);
            break;
        case 'L':
            CLR(A_IN1); SET(A_IN2);
            SET(B_IN1); CLR(B_IN2);
            break;
        case 'R': 
            SET(A_IN1); CLR(A_IN2);
            CLR(B_IN1); SET(B_IN2);
            break;
        case 'S': 
        default:
            stop_all(A_IN1, A_IN2, B_IN1, B_IN2);
            break;
    }

    munmap(map, 64 * 1024 * 1024);
    return 0;
}

