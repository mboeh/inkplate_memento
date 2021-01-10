#include "Inkplate.h"
Inkplate display(INKPLATE_1BIT);

#include "pics/dinosaur.h"
#include "pics/conjuror.h"
#include "pics/skeleton.h"
#include "pics/birb.h"
#include "pics/anatomy.h"
#include "pics/cut_apple.h"
#include "pics/horse_bones.h"
#include "pics/macintosh.h"

struct {
  const uint8_t *bits;
  int w;
  int h;
} pics[] = {
  { skeleton, skeleton_w, skeleton_h },
  { conjuror, conjuror_w, conjuror_h },
  { dinosaur, dinosaur_w, dinosaur_h },
  { birb, birb_w, birb_h },
  { anatomy, anatomy_w, anatomy_h },
  { cut_apple, cut_apple_w, cut_apple_h },
  { horse_bones, horse_bones_w, horse_bones_h },
  { macintosh, macintosh_w, macintosh_h },
  { NULL, 0, 0 }
};

#define LINES 9
#define LINE_W 64
char msg[LINES][LINE_W] = {
  "Have a great day",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  ""
};

#define ELEMENTS 10
enum el_type {
  EL_NONE,
  EL_TEXT,
  EL_PIC,
  EL_PAD
};
struct {
  enum el_type type;
  int n;
} elements[ELEMENTS] = {
  { EL_PIC, 0 },
  { EL_TEXT, 0 },
  { EL_TEXT, 1 },
  { EL_TEXT, 2 },
  { EL_TEXT, 3 },
  { EL_NONE, 0 },
  { EL_NONE, 0 },
  { EL_NONE, 0 },
  { EL_NONE, 0 },
  { EL_NONE, 0 }
};

#define DFL_PIC_TOP 80
#define DFL_TEXT_MARGIN 45
#define DFL_LINE_PADDING 10

int pic_top = DFL_PIC_TOP;
int text_margin = DFL_TEXT_MARGIN;
int line_padding = DFL_LINE_PADDING;

#include "font.h"
#define FONT Century_Schoolbook_L_Italic_32

#define PIC(n) pics[n].bits
#define PIC_W(n) pics[n].w
#define PIC_H(n) pics[n].h

#define DELAY 5000

int needsDraw = 1;

int width = 0;
int height = 0;

int16_t tx0 = 0;
int16_t tx1 = 0;
int16_t ty0 = 0;
int16_t ty1 = 0;
uint16_t tw = 0;
uint16_t th = 0;

void setup()
{
  Serial.begin(115200);

  display.begin();
  display.setRotation(3);
  display.clearDisplay();
  display.display();
  display.setFont(&FONT);
  display.setTextColor(BLACK, WHITE);

  width = display.width();
  height = display.height();
}

void loop()
{
  while (Serial.available()) {
    int cmd = Serial.read();
    switch (cmd) {
      case '$':
        {
          int line = Serial.read();
          if (line >= LINES) {
            Serial.print("Bad line: ");
            Serial.println(line);
          }
          int len = Serial.read();
          int read = Serial.readBytesUntil('\0', msg[line], len);
          msg[line][read] = '\0';
          needsDraw = 1;
        }
        break;
      case '=':
        pic_top = DFL_PIC_TOP;
        text_margin = DFL_TEXT_MARGIN;
        line_padding = DFL_LINE_PADDING;
        elements[0] = { EL_PIC, 0 };
        for (int l = 0; l < LINES; l++) {
          msg[l][0] = '\0';
          elements[l+1] = { EL_TEXT, l };
        }
        needsDraw = 1;
        break;
      case '@':
        elements[0] = { EL_PIC, Serial.read() };
        needsDraw = 1;
        break;
      case '*':
        {
          enum el_type typ = EL_NONE;
          int el_n = Serial.read();
          int typ_c = Serial.read();
          int n = Serial.read();
          switch(typ_c) {
          case '@':
            typ = EL_PIC;
            break;
          case '$':
            typ = EL_TEXT;
            break;
          case '_':
            typ = EL_PAD;
            break;
          default:
            typ = EL_NONE;
            break;
          }
          elements[el_n] = { typ, n };
        }
        needsDraw = 1;
        break;
      case '<':
        pic_top = Serial.read();
        needsDraw = 1;
        break;
      case '>':
        text_margin = Serial.read();
        needsDraw = 1;
        break;
      case '_':
        line_padding = Serial.read();
        needsDraw = 1;
        break;
      default:
        Serial.print("Unknown cmd ");
        Serial.println(cmd);
    }
  }

  if (needsDraw) {
    display.clearDisplay();
    ty0 = pic_top;
    tx0 = 0;
    for (int el = 0; el < ELEMENTS; el++) {
              int el_n = elements[el].n;
      switch (elements[el].type) {
        case EL_PIC:
          display.drawImage(
            PIC(el_n),
            (width / 2) - (PIC_W(el_n) / 2),
            ty0,
            PIC_W(el_n),
            PIC_H(el_n),
            BLACK
          );
          ty0 += PIC_H(el_n) + text_margin;

          break;
        case EL_TEXT:
          display.getTextBounds(msg[el_n], tx0, ty0, &tx1, &ty1, &tw, &th);
          display.setCursor((width / 2) - (tw / 2), ty0);
          display.print(msg[el_n]);

          ty0 += th + line_padding;

          break;
        case EL_PAD:
          ty0 += el_n;
          break;
        case EL_NONE:
          // pass
          break;
      }
    }
    display.display();
    needsDraw = 0;
  }

  delay(DELAY);
}
