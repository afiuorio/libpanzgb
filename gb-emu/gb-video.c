#include "gb.h"

BYTE getColour(BYTE colourNum, BYTE palette)
{
    int hi = 0 ;
    int lo = 0 ;
    
    // which bits of the colour palette does the colour id map to?
    switch (colourNum)
    {
        case 0: hi = 1 ; lo = 0 ;break ;
        case 1: hi = 3 ; lo = 2 ;break ;
        case 2: hi = 5 ; lo = 4 ;break ;
        case 3: hi = 7 ; lo = 6 ;break ;
    }
    
    // use the palette to get the colour
    int colour = 0;
    colour = ((palette >> (hi)) &0x1) << 1;
    colour |= ((palette >> (lo)) &0x1) ;
    
    return colour ;
}

void renderTiles(gb *cpu)
{
    WORD tileData = 0 ;
    WORD backgroundMemory =0 ;
    BYTE unsig = 1 ;
    
    BYTE lcd_control = readMemory(cpu, LCD_REG_CONTROL);
    
    BYTE scrollY = readMemory(cpu, SCROLLY);
    BYTE scrollX = readMemory(cpu, SCROLLX);
    BYTE windowY = readMemory(cpu, WINDOWY);
    BYTE windowX = readMemory(cpu, WINDOWX) - 7;
    
    
    BYTE currentLine = readMemory(cpu, LCD_SCANLINE_ADRR);
    BYTE usingWindow = 0 ;
    
    /*The window is enabled and visible?*/
    if( (lcd_control & 0x20) != 0 && windowY <= currentLine )
        usingWindow = 1 ;
    
    //tile area ?
    if ((lcd_control &0x10) != 0)
        tileData = 0x8000 ;
    else
    {
        // IMPORTANT: This memory region uses signed
        // bytes as tile identifiers
        tileData = 0x8800 ;
        unsig= 0 ;
    }
    
    // which background mem?
    if (usingWindow == 0)
    {
        if( (lcd_control &0x8) != 0)
            backgroundMemory = 0x9C00 ;
        else
            backgroundMemory = 0x9800 ;
    }
    else
    {
        // which window memory?
        if( (lcd_control &0x40) != 0)
            backgroundMemory = 0x9C00 ;
        else
            backgroundMemory = 0x9800 ;
    }
    
    BYTE yPos = 0 ;
    
    // yPos is used to calculate which of 32 vertical tiles the
    // current scanline is drawing
    if (usingWindow == 0)
        yPos = scrollY + currentLine ;
    else
        yPos = currentLine - windowY;
    
    // which line of the tile is being rendered ?
    WORD tileRow = (yPos/8)*32 ;
    
    //Now I have to renderd the line, pixel by pixel
    for (int pixel = 0 ; pixel < 160; pixel++)
    {
        BYTE xPos = pixel+scrollX ;
        
        // translate the current x pos to window space if necessary
        if (usingWindow==1)
        {
            if (pixel >= windowX)
                xPos = pixel - windowX ;
        }
        
        // which of the 32 horizontal tiles does this xPos fall within?
        WORD tileCol = (xPos/8) ;
        SIGNED_WORD tileNum ;
        
        // get the tile identity number. Remember it can be signed
        // or unsigned
        WORD tileAddrss = backgroundMemory+tileRow+tileCol;
        if(unsig==1)
            tileNum =(BYTE)readMemory(cpu,tileAddrss);
        else
            tileNum =(SIGNED_BYTE)readMemory(cpu,tileAddrss );
        
        
        // deduce where this tile identifier is in memory.
        WORD tileLocation = tileData ;
        
        if (unsig==1)
            tileLocation += (tileNum * 16) ;
        else
            tileLocation += ((tileNum+128) *16) ;
        
        // find the correct vertical line we're on of the
        // tile to get the tile data
        //from in memory
        BYTE line = yPos % 8 ;
        line *= 2; // each vertical line takes up two bytes of memory
        BYTE data1 = readMemory(cpu,tileLocation + line) ;
        BYTE data2 = readMemory(cpu,tileLocation + line + 1) ;
        
        // pixel 0 in the tile is it 7 of data 1 and data2.
        // Pixel 1 is bit 6 etc..
        int colourBit = xPos % 8 ;
        colourBit -= 7 ;
        colourBit *= -1 ;
        
        // combine data 2 and data 1 to get the colour id for this pixel
        // in the tile
        int colourNum = (data2 >> (colourBit)) &0x1;
        colourNum <<= 1;
        colourNum |= ((data1 >> (colourBit)) &0x1) ;
        
        
        // now we have the colour id, needed the palette 0xFF47
        BYTE palette = readMemory(cpu, 0xFF47);
        
        BYTE col = getColour(colourNum, palette) ;
        
        cpu->screenData[currentLine][pixel] = col;
    }
}

void renderSprites(gb *cpu)
{
    BYTE ysize = 8;
    BYTE currentLine = readMemory(cpu,0xFF44);
    BYTE lcd_control = readMemory(cpu, LCD_REG_CONTROL);
    if ((lcd_control &0x4) != 0)
        ysize = 16 ;
   
    /*Explore all the sprite table*/
    /*Each sprite is formed by 4 bytes:
     * 0 : y pos - 16
     * 1 : x pos - 8
     * 2 : data location in memory
     * 3 : several flags
     */
    for (int index = 0 ; index < 40*4; index+=4)
    {
        BYTE yPos = readMemory(cpu,0xFE00+index) - 16;
        
        if ((currentLine < yPos) || (currentLine >= (yPos+ysize)))
            continue;
        
        BYTE xPos = readMemory(cpu, 0xFE00+index+1)-8;
        BYTE tileLocation = readMemory(cpu,0xFE00+index+2) ;
        BYTE attributes = readMemory(cpu,0xFE00+index+3) ;
        
        BYTE yFlip = attributes & 0x40;
        BYTE xFlip = attributes & 0x20;
        
        // does this sprite intercept with the scanline?
            int line = currentLine - yPos ;
            
            // read the sprite in backwards in the y axis
            if (yFlip != 0)
            {
                line -= ysize ;
                line *= -1 ;
            }
            
            line *= 2; // same as for tiles
            WORD dataAddress = (0x8000 + (tileLocation * 16)) + line ;
            BYTE data1 = readMemory(cpu, dataAddress ) ;
            BYTE data2 = readMemory(cpu, dataAddress +1 ) ;
            
            // its easier to read in from right to left as pixel 0 is
            // bit 7 in the colour data, pixel 1 is bit 6 etc...
            for (int tilePixel = 7; tilePixel >= 0; tilePixel--)
            {
                int colourbit = tilePixel ;
                // read the sprite in backwards for the x axis
                if (xFlip != 0)
                {
                    colourbit -= 7 ;
                    colourbit *= -1 ;
                }
                
                int colourNum = (data2 >> (colourbit)) &0x1;
                colourNum <<= 1;
                colourNum |= ((data1 >> (colourbit)) &0x1) ;
                
                WORD colourAddress;
                if( (attributes & 0x10) != 0)
                    colourAddress = 0xFF49;
                else
                    colourAddress = 0xFF48 ;
                BYTE palette = readMemory(cpu, colourAddress);
                BYTE col = getColour(colourNum, palette ) ;
                

                /*Must ignore the alpha color*/
                if(col == (palette &0x3) )
                    continue;
                
                int xPix = 0 - tilePixel ;
                xPix += 7 ;
                int pixel = xPos+xPix ;
                
                // check if pixel is hidden behind background
                if ((attributes &0x80) != 0 && cpu->screenData[currentLine][pixel] != 0 )
                    continue ;
                /*I can draw the pixel*/
                cpu->screenData[currentLine][pixel] = col;
            }
    }
}


void drawScanline(gb *cpu){
    BYTE control = readMemory(cpu, LCD_REG_CONTROL) ;
   if ( (control & 0x1) !=0 )
        renderTiles(cpu) ;
    
    if ( (control & 0x2) != 0)
        renderSprites(cpu);
}


void updateLCD(gb* cpu)
{
    BYTE status = readMemory(cpu, LCD_REG_STATUS) ;
    BYTE lcd_control = readMemory(cpu, LCD_REG_CONTROL);
    
    /*LCD is disabled*/
    if ( (lcd_control & 0x80) == 0 )
    {
        // set the mode to 1 during lcd disabled and reset scanline
        cpu->clockScanline = 456 ;
        cpu->memory[0xFF44] = 0 ;
        status &= 252 ;
        status |= 0x1;
        writeMemory(cpu, LCD_REG_STATUS, status);
        return ;
    }
    
    BYTE currentline = readMemory(cpu,LCD_SCANLINE_ADRR) ;
    BYTE mode = status & 0x3 ;
    
    BYTE nextMode = 0 ;
    BYTE reqInt = 0 ;
    
    // in vblank so set mode to 1
    if (currentline >= 144)
    {
        nextMode = 1;
        status &= ~(0x3);
        status |= 0x1;
        reqInt = status & 0x10;
    }
    else
    {
        // mode 2
        if (cpu->clockScanline >= 376)
        {
            nextMode = 2 ;
            status &= ~(0x3);
            status |= 0x2;
            reqInt = status & 0x20;
        }
        // mode 3
        else if(cpu->clockScanline >= 204)
        {
            nextMode = 3 ;
            status &= ~(0x3);
            status |= 0x3;
        }
        // mode 0
        else
        {
            nextMode = 0;
            status &= ~(0x3);
            reqInt = status & 0x8;
        }
    }
    
    if ((reqInt !=0) && (mode != nextMode))
        raiseInterrupt(cpu, 1) ;
    
    // check the conincidence flag
    if (readMemory(cpu, 0xFF44) == readMemory(cpu,0xFF45))
    {
        status |= 0x4;
        if ( (status & 0x40) != 0)
            raiseInterrupt(cpu, 1) ;
    }
    else
    {
        status &= ~(0x4);
    }
    writeMemory(cpu, LCD_REG_STATUS ,status) ;
}



void handleGraphic(gb *cpu, unsigned int cycles)
{
    updateLCD(cpu);
    BYTE lcd_control = readMemory(cpu, LCD_REG_CONTROL);
    
    if ( (lcd_control & 0x80) != 0 ){
        cpu->clockScanline -= cycles ;
    }
    else
        return ;
    
    if (cpu->clockScanline <= 0)
    {
        cpu->memory[LCD_SCANLINE_ADRR]++;
        BYTE currentline = readMemory(cpu, LCD_SCANLINE_ADRR) ;
        cpu->clockScanline = 456 ;
        
        /*V-blank interrupt*/
        if (currentline == 144)
            raiseInterrupt(cpu, 0);
        
        else if (currentline > 153)
            cpu->memory[LCD_SCANLINE_ADRR]=0;
        
        else if (currentline < 144)
            drawScanline(cpu) ;
    }
}