// Topology to LEDs numbers
const byte vertexLeds[] = {49, 47, 45, 43, 41, 25, 33, 31, 29, 27, 24, 22, 20, 18, 16, 9, 7, 5, 3, 1};
const byte edgeLeds[] = {48, 46, 44, 42, 40, 39, 38, 37, 36, 35, 26, 34, 32, 30, 28, 23, 21, 19, 17, 15, 14, 13, 12, 11, 10, 8, 6, 4, 2, 0};

typedef struct {
  uint8_t antipode;
  uint8_t vertices[5];
  uint8_t edges[5];
} Face;

const Face faces[12] PROGMEM = {
  {11, {0, 1, 2, 3, 4}, {0, 1, 2, 3, 4}},
  {10, {0, 6, 12, 7, 1}, {0, 5, 11, 17, 6}},
  {6, {1, 7, 13, 8, 2}, {1, 6, 12, 18, 7}},
  {7, {2, 8, 14, 9, 3}, {2, 7, 13, 19, 8}},
  {8, {3, 9, 10, 5, 4}, {3, 8, 14, 15, 9}},
  {9, {4, 5, 11, 6, 0}, {4, 9, 10, 16, 5}},
  {2, {5, 10, 16, 17, 11}, {10, 15, 20, 26, 21}},
  {3, {6, 11, 17, 18, 12}, {11, 16, 21, 27, 22}},
  {4, {7, 12, 18, 19, 13}, {12, 17, 22, 28, 23}},
  {5, {8, 13, 19, 15, 14}, {13, 18, 23, 29, 24}},
  {1, {9, 14, 15, 16, 10}, {14, 19, 24, 25, 20}},
  {0, {15, 19, 18, 17, 16}, {25, 29, 28, 27, 26}}
};

Face getFace(int f);
