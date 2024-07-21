/*---------------------------------------------------------------------------------


     snes mouse demo
    -- alekmaul

    Mouse support by DigiDwrf


---------------------------------------------------------------------------------*/
#include "snes.h"

#ifndef MOUSE_SPEED
#define MOUSE_SPEED

#define slow 0
#define normal 1
#define fast 2

#endif

extern char snesfont, snespal, cursorsprite, cursorsprite_end, cursorpal, buttonsmap, buttonstiles, buttonstiles_end, buttonspal;
char hex_string[4];

// Init some variables
u16 p1_mouse_x = 0x80;
u16 p1_mouse_y = 0x70;
u16 p2_mouse_x = 0x80;
u16 p2_mouse_y = 0x70;

u8 odd = 0;

snesbool mc_mem[2] = {false};
snesbool printed[2] = {false};
snesbool mouseDown_L[2] = {false};
snesbool mouseDown_R[2] = {false};
snesbool mouseDown_LR[2] = {false};
snesbool speedset[2] = {true};

//---------------------------------------------------------------------------------
int main(void)
{
    // Init cursors sprite
    oamInitGfxSet(&cursorsprite, (&cursorsprite_end - &cursorsprite), &cursorpal, 48 * 2, 0, 0x0000, OBJ_SIZE16_L32);

    // Initialize text console with our font
    consoleSetTextVramBGAdr(0x6800);
    consoleSetTextVramAdr(0x3000);
    consoleSetTextOffset(0x0100);
    consoleInitText(0, 16 * 2, &snesfont, &snespal);

    // Draw a wonderful text :P
    consoleDrawText(11, 1, "MOUSE TEST");

    // we set mouse speed, or it will just output a random speed. We can change it later manually
    mouseSpeedSet[0] = slow;
    mouseSpeedSet[1] = slow;

    detectMouse(); // Let's check if a mouse is plugged in any port on boot, be sure nmi interrupt was called at least once (in this case, previous oamInitGfxSet() function was enough)
    WaitForVBlank(); // Let's make sure we read mouse for the first time after detectMouse()

    if (mouseConnect[0] == false)
        consoleDrawText(3, 5, "NO MOUSE PLUGGED ON PORT 0");
    else
    {
        dmaCopyVram(&buttonsmap + 0x60, 0x6188, 0x0A); // SLOW button pressed
        dmaCopyVram(&buttonsmap + 0xA0, 0x61A8, 0x0A); // SLOW button pressed
        dmaCopyVram(&buttonsmap + 0x4A, 0x618D, 0x16); // released buttons
        dmaCopyVram(&buttonsmap + 0x8A, 0x61AD, 0x16); // released buttons
        consoleDrawText(5, 5, "MOUSE PLUGGED ON PORT 0");
    }

    if (mouseConnect[1] == false)
        consoleDrawText(3, 17, "NO MOUSE PLUGGED ON PORT 1");
    else
    {
        dmaCopyVram(&buttonsmap + 0x60, 0x6308, 0x0A); // SLOW button pressed
        dmaCopyVram(&buttonsmap + 0xA0, 0x6328, 0x0A); // SLOW button pressed
        dmaCopyVram(&buttonsmap + 0x4A, 0x630D, 0x16); // released buttons
        dmaCopyVram(&buttonsmap + 0x8A, 0x632D, 0x16); // released buttons
        consoleDrawText(5, 17, "MOUSE PLUGGED ON PORT 1");
    }

    // Init background
    bgSetGfxPtr(0, 0x2000);
    bgSetGfxPtr(1, 0x4000);
    bgSetMapPtr(0, 0x6800, SC_32x32);
    bgSetMapPtr(1, 0x6000, SC_32x32);

    // Draw buttons for speed change
    bgInitTileSet(1, &buttonstiles, &buttonspal, 1, (&buttonstiles_end - &buttonstiles), 16 * 2, BG_16COLORS, 0x4000);

    // Now Put in 16 color mode
    setMode(BG_MODE1, 0);
    bgSetDisable(2);

    // Wait for nothing :P
    setScreenOn();

    while (1)
    {
        if (snes_mouse == false)
        detectMouse(); // Let's check if a mouse is plugged in any port

        odd++;
        // Optimize Draw text by printing new text just once
        if (mouseConnect[0] != mc_mem[0])
            printed[0] = true;
        mc_mem[0] = mouseConnect[0];

        // Update display with current mouse
        if (mouseConnect[0] == false)
        {
            if (printed[0])
            {
                WaitForVBlank();
                consoleDrawText(3, 5, "NO MOUSE PLUGGED ON PORT 0");
                consoleDrawText(11, 7, "          ");
                consoleDrawText(7, 10, "                    ");
                printed[0] = false;
            }
            oamSetVisible(0, OBJ_HIDE); // Hide 1p cursor
        }
        else
        {
            // We transform raw acceleration values into coordinates for OAM. Firt bit tell us mouse direction and the rest tell us how much fast it goes.
            if (mouse_x[0] & 0x80)
                p1_mouse_x -= mouse_x[0] & 0x7F;
            else
                p1_mouse_x += mouse_x[0] & 0x7F;
            if (mouse_y[0] & 0x80)
                p1_mouse_y -= mouse_y[0] & 0x7F;
            else
                p1_mouse_y += mouse_y[0] & 0x7F;

            // And set some boundaries
            if (p1_mouse_x > 0xFF00)
                p1_mouse_x = 0;
            if (p1_mouse_x > 0xFF)
                p1_mouse_x = 0xFF;
            if (p1_mouse_y > 0xFF00)
                p1_mouse_y = 0;
            if (p1_mouse_y > 0xEF)
                p1_mouse_y = 0xEF;

            oamSet(0, p1_mouse_x, p1_mouse_y, 3, 0, 0, 0, mouseConnect[1]);

            if (printed[0])
            {
                WaitForVBlank();
                consoleDrawText(3, 5, "  MOUSE PLUGGED ON PORT 0 ");
                consoleDrawText(11, 7, "X:    Y:");
                printed[0] = false;
            }

            if (mouseConnect[1] == false)
                odd = 1;

            if ((odd & 0x01) && (mouseButton[0] == false))
            {
                sprintf(hex_string, "%02X", p1_mouse_x);
                consoleDrawText(13, 7, hex_string);
                sprintf(hex_string, "%02X", p1_mouse_y);
                consoleDrawText(19, 7, hex_string);
            }

            // mousePressed works as turbo switch, it will be 1 until it gets released, and then it goes back to 0. Good for dragging or writing.
            switch (mousePressed[0])
            {
            case mouse_L:
                if (mouseDown_L[0] == false)
                {
                    consoleDrawText(7, 10, "LEFT BUTTON PRESSED ");
                    mouseDown_L[0] = true;
                    mouseDown_R[0] = false;
                    mouseDown_LR[0] = false;
                }
                break;
            case mouse_R:
                if (mouseDown_R[0] == false)
                {
                    consoleDrawText(7, 10, "RIGHT BUTTON PRESSED");
                    mouseDown_L[0] = false;
                    mouseDown_R[0] = true;
                    mouseDown_LR[0] = false;
                }
                break;
            case mouse_L + mouse_R:
                if (mouseDown_LR[0] == false)
                {
                    consoleDrawText(7, 10, "BOTH BUTTONS PRESSED");
                    mouseDown_L[0] = false;
                    mouseDown_R[0] = false;
                    mouseDown_LR[0] = true;
                }
                break;
            }
        }

        // Optimize Draw text by printing new text just once
        if (mouseConnect[1] != mc_mem[1])
            printed[1] = true;
        mc_mem[1] = mouseConnect[1];

        if (mouseConnect[1] == false)
        {
            if (printed[1])
            {
                WaitForVBlank();
                consoleDrawText(3, 17, "NO MOUSE PLUGGED ON PORT 1");
                consoleDrawText(11, 19, "          ");
                consoleDrawText(7, 22, "                    ");
                printed[1] = false;
            }
            oamSetVisible(4, OBJ_HIDE); // Hide 2p cursor
        }
        else
        {
            // We transform raw acceleration values into coordinates for OAM. Firt bit tell us mouse direction and the rest tell us how much fast it goes.
            if (mouse_x[1] & 0x80)
                p2_mouse_x -= mouse_x[1] & 0x7F;
            else
                p2_mouse_x += mouse_x[1] & 0x7F;
            if (mouse_y[1] & 0x80)
                p2_mouse_y -= mouse_y[1] & 0x7F;
            else
                p2_mouse_y += mouse_y[1] & 0x7F;

            // And set some boundaries
            if (p2_mouse_x > 0xFF00)
                p2_mouse_x = 0;
            if (p2_mouse_x > 0xFF)
                p2_mouse_x = 0xFF;
            if (p2_mouse_y > 0xFF00)
                p2_mouse_y = 0;
            if (p2_mouse_y > 0xEF)
                p2_mouse_y = 0xEF;

            oamSet(4, p2_mouse_x, p2_mouse_y, 3, 0, 0, 0, mouseConnect[0] << 1);

            if (printed[1])
            {
                WaitForVBlank();
                consoleDrawText(3, 17, "  MOUSE PLUGGED ON PORT 1 ");
                consoleDrawText(11, 19, "X:    Y:");
                printed[1] = false;
            }

            if (mouseConnect[0] == false)
                odd = 0;

            if (((odd & 0x01) == 0) && (mouseButton[1] == false))
            {
                sprintf(hex_string, "%02X", p2_mouse_x, p2_mouse_y);
                consoleDrawText(13, 19, hex_string);
                sprintf(hex_string, "%02X", p2_mouse_y);
                consoleDrawText(19, 19, hex_string);
            }

            // mousePressed works as turbo switch, it will be 1 until it gets released, and then it goes back to 0. Good for dragging or writing.
            switch (mousePressed[1])
            {
            case mouse_L:
                if (mouseDown_L[1] == false)
                {
                    consoleDrawText(7, 22, "LEFT BUTTON PRESSED ");
                    mouseDown_L[1] = true;
                    mouseDown_R[1] = false;
                    mouseDown_LR[1] = false;
                }
                break;
            case mouse_R:
                if (mouseDown_R[1] == false)
                {
                    consoleDrawText(7, 22, "RIGHT BUTTON PRESSED");
                    mouseDown_L[1] = false;
                    mouseDown_R[1] = true;
                    mouseDown_LR[1] = false;
                }
                break;
            case mouse_L + mouse_R:
                if (mouseDown_LR[1] == false)
                {
                    consoleDrawText(7, 22, "BOTH BUTTONS PRESSED");
                    mouseDown_L[1] = false;
                    mouseDown_R[1] = false;
                    mouseDown_LR[1] = true;
                }
                break;
            }
        }

        WaitForVBlank();

        // mouseButton works as a one frame value, so it gets released shortly after pressing the button. Good for clicking stuff that need to be called once, like buttons.
        if (mouseConnect[0])
        {
            if (mouseButton[0] & mouse_L)
            {
                // Let's choose speed setting
                if ((p1_mouse_y > 0x5E) && (p1_mouse_y < 0x6C))
                {
                    if ((p1_mouse_x > 0x44) && (p1_mouse_x < 0x64))
                    {
                        mouseSpeedSet[0] = slow;
                        speedset[0] = true;
                        mouseSpeedChange(0); // Let's tell the mouse we want to change speed. mouseSpeedSet[] has to be populated first.
                    }
                    if ((p1_mouse_x > 0x6C) && (p1_mouse_x < 0x94))
                    {
                        mouseSpeedSet[0] = normal;
                        speedset[0] = true;
                        mouseSpeedChange(0); // Let's tell the mouse we want to change speed. mouseSpeedSet[] has to be populated first.
                    }
                    if ((p1_mouse_x > 0x9C) && (p1_mouse_x < 0xBC))
                    {
                        mouseSpeedSet[0] = fast;
                        speedset[0] = true;
                        mouseSpeedChange(0); // Let's tell the mouse we want to change speed. mouseSpeedSet[] has to be populated first.
                    }
                }
            }

            if (speedset[0])
            {
                dmaCopyVram(&buttonsmap + 0x40, 0x6188, 0x20); // released buttons
                dmaCopyVram(&buttonsmap + 0x80, 0x61A8, 0x20); // released buttons

                switch (mouseSpeedSet[0])
                {
                case slow:
                    dmaCopyVram(&buttonsmap + 0x60, 0x6188, 0x0A); // SLOW button pressed
                    dmaCopyVram(&buttonsmap + 0xA0, 0x61A8, 0x0A); // SLOW button pressed
                    break;
                case normal:
                    dmaCopyVram(&buttonsmap + 0x6A, 0x618D, 0x0C); // NORMAL button pressed
                    dmaCopyVram(&buttonsmap + 0xAA, 0x61AD, 0x0C); // NORMAL button pressed
                    break;
                case fast:
                    dmaCopyVram(&buttonsmap + 0x76, 0x6193, 0x0A); // FAST button pressed
                    dmaCopyVram(&buttonsmap + 0xB6, 0x61B3, 0x0A); // FAST button pressed
                    break;
                }
                speedset[0] = false;
            }

            if (mousePressed[0] == false)
                dmaFillVram(&buttonsmap, 0x6940, 0x40); // wipe text
        }
        else if (speedset[0] == false)
        {
            dmaFillVram(&buttonsmap + 0x40, 0x6188, 0x20); // remove buttons
            dmaFillVram(&buttonsmap + 0x80, 0x61A8, 0x20); // remove buttons
            speedset[0] = true;
        }

        if (mouseConnect[1])
        {
            if (mouseButton[1] & mouse_L)
            {
                // Let's choose speed setting
                if ((p2_mouse_y > 0xBE) && (p2_mouse_y < 0xCC))
                {
                    if ((p2_mouse_x > 0x44) && (p2_mouse_x < 0x64))
                    {
                        mouseSpeedSet[1] = slow;
                        speedset[1] = true;
                        mouseSpeedChange(1); // Let's tell the mouse we want to change speed. mouseSpeedSet[] has to be populated first.
                    }
                    if ((p2_mouse_x > 0x6C) && (p2_mouse_x < 0x94))
                    {
                        mouseSpeedSet[1] = normal;
                        speedset[1] = true;
                        mouseSpeedChange(1); // Let's tell the mouse we want to change speed. mouseSpeedSet[] has to be populated first.
                    }
                    if ((p2_mouse_x > 0x9C) && (p2_mouse_x < 0xBC))
                    {
                        mouseSpeedSet[1] = fast;
                        speedset[1] = true;
                        mouseSpeedChange(1); // Let's tell the mouse we want to change speed. mouseSpeedSet[] has to be populated first.
                    }
                }
            }

            if (speedset[1])
            {
                dmaCopyVram(&buttonsmap + 0x40, 0x6308, 0x20); // released buttons
                dmaCopyVram(&buttonsmap + 0x80, 0x6328, 0x20); // released buttons

                switch (mouseSpeedSet[1])
                {
                case slow:
                    dmaCopyVram(&buttonsmap + 0x60, 0x6308, 0x0A); // SLOW button pressed
                    dmaCopyVram(&buttonsmap + 0xA0, 0x6328, 0x0A); // SLOW button pressed
                    break;
                case normal:
                    dmaCopyVram(&buttonsmap + 0x6A, 0x630D, 0x0C); // NORMAL button pressed
                    dmaCopyVram(&buttonsmap + 0xAA, 0x632D, 0x0C); // NORMAL button pressed
                    break;
                case fast:
                    dmaCopyVram(&buttonsmap + 0x76, 0x6313, 0x0A); // FAST button pressed
                    dmaCopyVram(&buttonsmap + 0xB6, 0x6333, 0x0A); // FAST button pressed
                    break;
                }
                speedset[1] = false;
            }

            if (mousePressed[1] == false)
                dmaFillVram(&buttonsmap, 0x6AC0, 0x40); // wipe text
        }
        else if (speedset[1] == false)
        {
            dmaFillVram(&buttonsmap + 0x40, 0x6308, 0x20); // remove buttons
            dmaFillVram(&buttonsmap + 0x80, 0x6328, 0x20); // remove buttons
            speedset[1] = true;
        }
    }
    return 0;
}