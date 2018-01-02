const byte characters[][8] = {
  {0b00000000, // Empty (space)
   0b00000000,
   0b00000000,
   0b00000000,
   0b00000000,
   0b00000000,
   0b00000000,
   0b00000000},
  
  {0b00000000, // A...
   0b00110000,
   0b01111000,
   0b11001100,
   0b11111100,
   0b11001100,
   0b11001100,
   0b11001100},
  
  {0b00000000,
   0b11111000,
   0b11001100,
   0b11001100,
   0b11111000,
   0b11001100,
   0b11001100,
   0b11111000},
   
  {0b00000000,
   0b01111000,
   0b11001100,
   0b11000000,
   0b11000000,
   0b11000000,
   0b11001100,
   0b01111000},
   
  {0b00000000,
   0b11110000,
   0b11011000,
   0b11001100,
   0b11001100,
   0b11001100,
   0b11011000,
   0b11110000},
  
  {0b00000000,
   0b11111100,
   0b11000000,
   0b11000000,
   0b11110000,
   0b11000000,
   0b11000000,
   0b11111100},
  
  {0b00000000,
   0b11111100,
   0b11000000,
   0b11000000,
   0b11110000,
   0b11000000,
   0b11000000,
   0b11000000},

  {0b00000000,
   0b01111000,
   0b11001100,
   0b11000000,
   0b11011100,
   0b11001100,
   0b11001100,
   0b01111000},

  {0b00000000,
   0b11001100,
   0b11001100,
   0b11001100,
   0b11111100,
   0b11001100,
   0b11001100,
   0b11001100},

  {0b00000000,
   0b00111100,
   0b00011000,
   0b00011000,
   0b00011000,
   0b00011000,
   0b00011000,
   0b00111100},

  {0b00000000,
   0b00111100,
   0b00011000,
   0b00011000,
   0b00011000,
   0b00011000,
   0b11011000,
   0b01110000},

  {0b00000000,
   0b11001100,
   0b11011000,
   0b11110000,
   0b11100000,
   0b11110000,
   0b11011000,
   0b11001100},

  {0b00000000,
   0b11000000,
   0b11000000,
   0b11000000,
   0b11000000,
   0b11000000,
   0b11000000,
   0b11111100},

  {0b00000000,
   0b11000110,
   0b11101110,
   0b11111110,
   0b11010110,
   0b11000110,
   0b11000110,
   0b11000110},

  {0b00000000,
   0b11001100,
   0b11101100,
   0b11111100,
   0b11111100,
   0b11011100,
   0b11001100,
   0b11001100},

  {0b00000000,
   0b01111000,
   0b11001100,
   0b11001100,
   0b11001100,
   0b11001100,
   0b11001100,
   0b01111000},

  {0b00000000,
   0b11111000,
   0b11001100,
   0b11001100,
   0b11111000,
   0b11000000,
   0b11000000,
   0b11000000},

  {0b00000000,
   0b01111000,
   0b11001100,
   0b11001100,
   0b11001100,
   0b11001100,
   0b01111000,
   0b00011100},

  {0b00000000,
   0b11111000,
   0b11001100,
   0b11001100,
   0b11111000,
   0b11110000,
   0b11011000,
   0b11001100},

  {0b00000000,
   0b01111000,
   0b11001100,
   0b11000000,
   0b01111000,
   0b00001100,
   0b11001100,
   0b01111000},

  {0b00000000,
   0b11111100,
   0b00110000,
   0b00110000,
   0b00110000,
   0b00110000,
   0b00110000,
   0b00110000},

  {0b00000000,
   0b11001100,
   0b11001100,
   0b11001100,
   0b11001100,
   0b11001100,
   0b11001100,
   0b01111000},

  {0b00000000,
   0b11001100,
   0b11001100,
   0b11001100,
   0b11001100,
   0b11001100,
   0b01111000,
   0b00110000},

  {0b00000000,
   0b11000110,
   0b11000110,
   0b11000110,
   0b11010110,
   0b11111110,
   0b11101110,
   0b11000110},

  {0b00000000,
   0b11001100,
   0b11001100,
   0b01111000,
   0b00110000,
   0b01111000,
   0b11001100,
   0b11001100},

  {0b00000000,
   0b11001100,
   0b11001100,
   0b11001100,
   0b01111000,
   0b00110000,
   0b00110000,
   0b00110000},

  {0b00000000, // ...Z
   0b11111100,
   0b00001100,
   0b00011000,
   0b00110000,
   0b01100000,
   0b11000000,
   0b11111100},

  {0b11111111, // Top left corner
   0b11111111,
   0b11000000,
   0b11000000,
   0b11000000,
   0b11000000,
   0b11000000,
   0b11000000},

  {0b11111111, // Top right corner
   0b11111111,
   0b00000011,
   0b00000011,
   0b00000011,
   0b00000011,
   0b00000011,
   0b00000011},

  {0b11000000, // Bottom left corner
   0b11000000,
   0b11000000,
   0b11000000,
   0b11000000,
   0b11000000,
   0b11111111,
   0b11111111},

  {0b00000011, // Bottom right corner
   0b00000011,
   0b00000011,
   0b00000011,
   0b00000011,
   0b00000011,
   0b11111111,
   0b11111111},

  {0b11111111, // Top line
   0b11111111,
   0b00000000,
   0b00000000,
   0b00000000,
   0b00000000,
   0b00000000,
   0b00000000},

  {0b00000011, // Right line
   0b00000011,
   0b00000011,
   0b00000011,
   0b00000011,
   0b00000011,
   0b00000011,
   0b00000011},

  {0b00000000, // Bottom line
   0b00000000,
   0b00000000,
   0b00000000,
   0b00000000,
   0b00000000,
   0b11111111,
   0b11111111},

  {0b11000000, // Left line
   0b11000000,
   0b11000000,
   0b11000000,
   0b11000000,
   0b11000000,
   0b11000000,
   0b11000000},

  {0b00000000, // .
   0b00000000,
   0b00000000,
   0b00000000,
   0b00000000,
   0b00000000,
   0b00110000,
   0b00110000},

  {0b00000000, // ?
   0b01111000,
   0b11001100,
   0b00001100,
   0b00011000,
   0b00110000,
   0b00000000,
   0b00110000},

  {0b00000000, // !
   0b00110000,
   0b00110000,
   0b00110000,
   0b00110000,
   0b00110000,
   0b00000000,
   0b00110000},

  {0b11000011, // Cross
   0b11100111,
   0b01111110,
   0b00111100,
   0b00111100,
   0b01111110,
   0b11100111,
   0b11000011},

  {0b00000011, // Tick
   0b00000110,
   0b00000110,
   0b00001100,
   0b10001100,
   0b11011000,
   0b01111000,
   0b00110000},

  {0b00111000, // Clock 1
   0b01111100,
   0b10010010,
   0b10010010,
   0b10010010,
   0b10000010,
   0b10000010,
   0b01111100},

  {0b00111000, // Clock 2
   0b01111100,
   0b10000010,
   0b10000010,
   0b10011110,
   0b10000010,
   0b10000010,
   0b01111100},

  {0b00111000, // Clock 3
   0b01111100,
   0b10000010,
   0b10000010,
   0b10010010,
   0b10010010,
   0b10010010,
   0b01111100},

  {0b00111000, // Clock 4
   0b01111100,
   0b10000010,
   0b10000010,
   0b11110010,
   0b10000010,
   0b10000010,
   0b01111100},

  {0b00000000, // 0
   0b01111000,
   0b11001100,
   0b11011100,
   0b11101100,
   0b11001100,
   0b11001100,
   0b01111000},

  {0b00000000, // 1
   0b00110000,
   0b00110000,
   0b01110000,
   0b00110000,
   0b00110000,
   0b00110000,
   0b11111100},

  {0b00000000, // 2
   0b01111000,
   0b11001100,
   0b00001100,
   0b00011000,
   0b01100000,
   0b11000000,
   0b11111100},

  {0b00000000, // 3
   0b01111000,
   0b11001100,
   0b00001100,
   0b00111000,
   0b00001100,
   0b11001100,
   0b01111000},

  {0b00000000, // 4
   0b00001100,
   0b00011100,
   0b00111100,
   0b11001100,
   0b11111110,
   0b00001100,
   0b00001100},

  {0b00000000, // 5
   0b11111100,
   0b11000000,
   0b11111000,
   0b00001100,
   0b00001100,
   0b11001100,
   0b01111000},

  {0b00000000, // 6
   0b01111000,
   0b11001100,
   0b11000000,
   0b11111000,
   0b11001100,
   0b11001100,
   0b01111000},

  {0b00000000, // 7
   0b11111100,
   0b11001100,
   0b00011000,
   0b00110000,
   0b00110000,
   0b00110000,
   0b00110000},

  {0b00000000, // 8
   0b01111000,
   0b11001100,
   0b11001100,
   0b01111000,
   0b11001100,
   0b11001100,
   0b01111000},

  {0b00000000, // 9
   0b01111000,
   0b11001100,
   0b11001100,
   0b01111100,
   0b00001100,
   0b11001100,
   0b01111000},

  {0b00000000, // +
   0b00000000,
   0b00011000,
   0b00011000,
   0b01111110,
   0b00011000,
   0b00011000,
   0b00000000},

  {0b00000000, // -
   0b00000000,
   0b00000000,
   0b00000000,
   0b01111110,
   0b00000000,
   0b00000000,
   0b00000000},

  {0b00000000, // *
   0b00000000,
   0b01100110,
   0b00111100,
   0b01111110,
   0b00111100,
   0b01100110,
   0b00000000},

  {0b00000000, // /
   0b00000110,
   0b00001100,
   0b00011000,
   0b00110000,
   0b01100000,
   0b11000000,
   0b00000000},

  {0b00000000, // :
   0b00000000,
   0b00110000,
   0b00110000,
   0b00000000,
   0b00110000,
   0b00110000,
   0b00000000},

  {0b00000000, // %
   0b11000110,
   0b11001100,
   0b00011000,
   0b00110000,
   0b01100110,
   0b11000110,
   0b00000000},

  {0b00010000, // Power
   0b00010000,
   0b01111100,
   0b11010110,
   0b10010010,
   0b10000010,
   0b11000110,
   0b01111100}
};