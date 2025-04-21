#ifndef FONTS__H
#define FONTS__H

typedef struct {
    int width;
    int height;
    const unsigned char *data[];
} Font;

// extern const Font font5x7;
extern const Font font8x12;
extern const Font font8x14;
extern const Font font8x16;


#endif
