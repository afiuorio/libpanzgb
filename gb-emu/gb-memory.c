#include "gb.h"
#include <string.h>
#include <stdio.h>

/*This function assumes that the ROM is already loaded*/

void setGbBanking(gb *cpu){
    switch( cpu->cartridge [0x147]){
        case 1 :
        case 2 :
        case 3 :
            cpu->ROMType = 1;
            cpu->currentROMBank = 1;
            break;
            
            
        case 0x19 :
        case 0x1A :
        case 0x1B :
            cpu->ROMType = 5;
            cpu->currentROMBank = 0;
            break;
    }
    
    cpu->currentRAMBank = 0;
}

void changeBanks(gb *cpu, WORD addr, BYTE data)
{
    // RAM enabling
    if (addr < 0x2000)
    {
        if (cpu->ROMType == 1 || cpu->ROMType == 5)
        {
            BYTE testData = data & 0xF ;
            if (testData == 0xA)
                cpu->isRAMEnable = 1 ;
            else if (testData == 0x0)
                cpu->isRAMEnable = 0 ;
        }
    }
    
    /*Change ROM bank*/
    else if ((addr >= 0x2000) && (addr < 0x4000))
    {
        if (cpu->ROMType == 1)
        {
            BYTE lower5 = data & 31 ;
            cpu->currentROMBank &= 224; // turn off the lower 5
            cpu->currentROMBank |= lower5 ;
            if ( cpu->currentROMBank == 0    ||
                cpu->currentROMBank == 0x20 ||
                cpu->currentROMBank == 0x40 ||
                cpu->currentROMBank == 0x60)
                cpu->currentROMBank++ ;
        }
        else if (cpu->ROMType == 5){
            printf("change rom bank = %x %x\n", addr, data);
            if(addr < 0x3000){
                cpu->currentROMBank &= 0x0100;
                cpu->currentROMBank |= data;
            }
            else {
                cpu->currentROMBank &= 0x00FF;
                cpu->currentROMBank |= ( (data &0x1)<<9);
            }
            
            
            
            
            
        }
        
        
        
    }
    
    /*Depending from the MBC1 type, I have to change something*/
    else if ((addr >= 0x4000) && (addr < 0x6000))
    {
        if (cpu->ROMType == 1)
        {
            if(cpu->ROMBankType == 1){
                cpu->currentROMBank &= 31 ;
                data &= 0x3 ;
                data <<=5;
                cpu->currentROMBank |= data ;
                if ( cpu->currentROMBank == 0    ||
                     cpu->currentROMBank == 0x20 ||
                     cpu->currentROMBank == 0x40 ||
                     cpu->currentROMBank == 0x60)
                        cpu->currentROMBank++ ;
            }
            else{
                cpu->currentRAMBank = data & 0x3 ;
            }
        }
        if (cpu->ROMType == 5){
            cpu->currentRAMBank = data &0xF;
        }
        
        
    }
    
    /*Determine which type of MBC1 is*/
    else if ((addr >= 0x6000) && (addr < 0x8000))
    {
        if (cpu->ROMType == 1){
            BYTE newData = data & 0x1 ;
            cpu->ROMBankType = (newData == 0)?1:0;
            if (cpu->ROMBankType == 1)
                cpu->currentRAMBank = 0 ;
        }
    }
}



BYTE readMemory(gb *cpu, WORD addr){
    if((addr>=0x0000) && (addr <= 0x3FFF)){
        return cpu->cartridge[addr];
    }
    if ((addr>=0x4000) && (addr <= 0x7FFF))
    {
        WORD t = addr - 0x4000 ;
        return cpu->cartridge[t + (cpu->currentROMBank * 0x4000)] ;
    }
    
    else if ((addr >= 0xA000) && (addr <= 0xBFFF))
    {
        WORD t = addr - 0xA000 ;
        return cpu->RAMBank [t + (cpu->currentRAMBank * 0x2000)] ;
    }
    else if(addr == 0xFF00)
        return getKeypad(cpu);
    
    return cpu->memory[addr];
}

/*This function is necessary for replicate the ECHO (E000-FDFF) area*/

void writeMemory(gb *cpu, WORD addr, BYTE data){
    /*This part is mapped on the rom, so read-only*/
    if (addr < 0x8000)
    {
        changeBanks(cpu, addr,data) ;
    }
    
    else if ((addr >= 0xA000) && (addr < 0xC000))
    {
        if (cpu->isRAMEnable != 0)
        {
            WORD t = addr - 0xA000 ;
            cpu->RAMBank[t + (cpu->currentRAMBank * 0x2000)] = data ;
        }
    }
    
    else if ( ( addr >= 0xE000 ) && (addr < 0xFE00) )
    {
        cpu->memory[addr] = data ;
        writeMemory(cpu, addr-0x2000, data);
    }
    
    /*Not usable */
    else if ( ( addr >= 0xFEA0 ) && (addr < 0xFEFF) )
    {
    }
    
    else if (addr == TIMER_CONTROLLER)
    {
        BYTE freq = readMemory(cpu,TIMER_CONTROLLER) & 0x3;
        cpu->memory[TIMER_CONTROLLER] = data ;
        BYTE newfreq = readMemory(cpu,TIMER_CONTROLLER) & 0x3;
        
        if (freq != newfreq)
            setTimerFreq(cpu);
    }
    else if (addr == DIVIDER_TIMER)
        cpu->memory[DIVIDER_TIMER] = 0 ;
    else if (addr == LCD_SCANLINE_ADRR)
        cpu->memory[LCD_SCANLINE_ADRR] = 0 ;
    else if (addr == DMA_ADRR)
        DMATransfert(cpu,data) ;
    else
        cpu->memory[addr] = data;
}