#include "STM32_CAN.h"

// STM32F103 CAN registers (base address: 0x40006400)
#define CAN1_BASE       0x40006400
#define CAN_MCR         (*(volatile uint32_t*)(CAN1_BASE + 0x000))
#define CAN_MSR         (*(volatile uint32_t*)(CAN1_BASE + 0x004))
#define CAN_TSR         (*(volatile uint32_t*)(CAN1_BASE + 0x008))
#define CAN_BTR         (*(volatile uint32_t*)(CAN1_BASE + 0x01C))
#define CAN_TI0R        (*(volatile uint32_t*)(CAN1_BASE + 0x180))
#define CAN_TDT0R       (*(volatile uint32_t*)(CAN1_BASE + 0x184))
#define CAN_TDL0R       (*(volatile uint32_t*)(CAN1_BASE + 0x188))
#define CAN_TDH0R       (*(volatile uint32_t*)(CAN1_BASE + 0x18C))

STM32_CAN::STM32_CAN() {}

void STM32_CAN::configurePins() {
    // Enable AFIO and GPIOA clocks
    RCC->APB2ENR |= RCC_APB2ENR_AFIOEN | RCC_APB2ENR_IOPAEN;
    
    // Remap CAN pins if necessary (No remap for PA11/PA12)
    // AFIO->MAPR &= ~AFIO_MAPR_CAN_REMAP; 

    // Configure PA11 (CAN_RX) as input floating
    GPIOA->CRH &= ~(GPIO_CRH_CNF11 | GPIO_CRH_MODE11);
    GPIOA->CRH |= GPIO_CRH_CNF11_1; 

    // Configure PA12 (CAN_TX) as alternate function push-pull
    GPIOA->CRH &= ~(GPIO_CRH_CNF12 | GPIO_CRH_MODE12);
    GPIOA->CRH |= GPIO_CRH_CNF12_1 | GPIO_CRH_MODE12_0 | GPIO_CRH_MODE12_1;
}

bool STM32_CAN::setBitTiming(uint32_t bitrate) {
    if (bitrate != 500000) {
        // Only 500kbps is supported
        return false;
    }

    // PCLK1 is 36MHz for 72MHz SYSCLK on Bluepill
    // Baudrate = PCLK1 / (Prescaler * (1 + TS1 + TS2))
    // 500kbps = 36MHz / (9 * (1 + 6 + 1))
    
    // BTR register configuration for 500kbps
    // LBKM = 0, SILM = 0
    // SJW = 1 quantum (0b00)
    // TS2 = 1 quantum (0b000)
    // TS1 = 6 quanta (0b0101)
    // BRP = 8 (Prescaler = 9)
    CAN_BTR = (0b00 << 24) | (0b000 << 20) | (0b0101 << 16) | 8;
    return true;
}

bool STM32_CAN::begin(uint32_t bitrate) {
    // Enable CAN1 clock
    RCC->APB1ENR |= RCC_APB1ENR_CAN1EN;

    configurePins();

    // Enter initialization mode
    CAN_MCR |= CAN_MCR_INRQ;
    while (!(CAN_MSR & CAN_MSR_INAK));

    // Disable sleep mode
    CAN_MCR &= ~CAN_MCR_SLEEP;

    // Set automatic bus-off management
    CAN_MCR |= CAN_MCR_ABOM;

    if (!setBitTiming(bitrate)) {
        return false;
    }

    // Leave initialization mode
    CAN_MCR &= ~CAN_MCR_INRQ;
    while (CAN_MSR & CAN_MSR_INAK);

    return true;
}

void STM32_CAN::write(CAN_Message &msg) {
    // Check if a mailbox is available
    if ((CAN_TSR & CAN_TSR_TME0) == CAN_TSR_TME0) {
        // Mailbox 0 is free
        CAN_TI0R = (msg.id << 21); // Standard ID
        CAN_TDT0R = msg.length;
        
        CAN_TDL0R = (msg.data[3] << 24) | (msg.data[2] << 16) | (msg.data[1] << 8) | msg.data[0];
        CAN_TDH0R = (msg.data[7] << 24) | (msg.data[6] << 16) | (msg.data[5] << 8) | msg.data[4];

        // Request transmission
        CAN_TI0R |= 1; 
    }
}
