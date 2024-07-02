# E-PAPER demonstration
This example code demonstrates the use of the CH32V003 SPI peripheral with the WeAct Studio 1.54 inch e-paper module

## Implementation Details
As the CH32V003 does not have enough RAM to store a full bitmap image, a semigraphic characters approach is used.

A 25 line x 25 character screen is stored in the RAM, the graphic image is generated going through a 8x8 font stored in Flash.

## Use
Connect an SSD1306-based OLED in SPI interface mode as follows:
* PC2 - RST
* PC3 - CS
* PC4 - DC
* PC5 - SCK
* PC6 - MOSI
* PD2 - BUSY

