#include <gba.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "asc126_new.h"

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 160
#define MODE4_STRIDE (SCREEN_WIDTH / 2)
#define PAGE1_OFFSET 0xA000

#define HEADER_HEIGHT 19
#define FOOTER_TOP 146
#define TEXT_X 4
#define TEXT_Y 21
#define TEXT_COLUMNS 38
#define TEXT_ROWS 10
#define TEXT_LINE_HEIGHT 12
#define GLYPH_ADVANCE 6
#define GLYPH_HEIGHT 12

#define LINE_POSITION_MASK 0x03FFFFFFu
#define LINE_BOLD_FLAG 0x04000000u
#define LINE_HEADING_SHIFT 27
#define LINE_HEADING_MASK (3u << LINE_HEADING_SHIFT)
#define LINE_RULE_FLAG 0x20000000u
#define GLYPH_BOLD_FLAG 0x8000u
#define GLYPH_VALUE_MASK 0x7FFFu

#define HEADER_PROGRESS_X 204
#define SCREEN_BYTE_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT)

#define MAX_VISUAL_LINES 49152
#define POGO_MAGIC 0xFAB0BABE
#define POGO_ARGS ((volatile uint32_t *)0x0203FC00)
#define POGO_FILE_SIZE (*(volatile uint32_t *)0x0203FBF8)
#define POGO_FILE_ADDRESS (*(volatile uint32_t *)0x0203FBFC)

#define EWRAM_BSS __attribute__((section(".sbss")))

enum PaletteIndex {
    PAL_BACKGROUND = 0,
    PAL_TEXT,
    PAL_ACCENT,
    PAL_ACCENT_TEXT,
    PAL_MUTED,
    PAL_FOOTER,
    PAL_PANEL,
    PAL_SHADOW,
    PAL_PROGRESS_TRACK
};

typedef struct {
    const uint8_t *data;
    uint32_t size;
    const char *name;
    bool markdown;
} Document;

static uint32_t line_offsets[MAX_VISUAL_LINES] EWRAM_BSS;
static uint32_t line_count;
static bool index_truncated;
static bool dark_theme;
static bool showing_controls;
static uint16_t *draw_page;
static bool page_one_visible;

static const char preview_text[] =
    "# DS Style Text Viewer\n"
    "\n"
    "A **clean, lightweight reader** for text and Markdown files. Long "
    "lines wrap neatly, and common line endings are handled automatically.\n"
    "\n"
    "---\n"
    "\n"
    "## Navigation\n"
    "Use Up and Down to move one line. Left, Right, L, R or A move one "
    "page. Press Select to switch between light and dark mode.\n"
    "\n"
    "Press Start at any time to view the controls.";

extern void return_to_ds_style(void) __attribute__((noreturn));

static inline uint16_t rgb15(uint8_t r, uint8_t g, uint8_t b)
{
    return (uint16_t)(r | (g << 5) | (b << 10));
}

static void set_palette(void)
{
    if (dark_theme) {
        BG_PALETTE[PAL_BACKGROUND] = rgb15(3, 3, 5);
        BG_PALETTE[PAL_TEXT] = rgb15(29, 29, 30);
        BG_PALETTE[PAL_ACCENT] = rgb15(14, 9, 25);
        BG_PALETTE[PAL_ACCENT_TEXT] = rgb15(31, 31, 31);
        BG_PALETTE[PAL_MUTED] = rgb15(19, 19, 21);
        BG_PALETTE[PAL_FOOTER] = rgb15(6, 6, 9);
        BG_PALETTE[PAL_PANEL] = rgb15(8, 8, 12);
        BG_PALETTE[PAL_SHADOW] = rgb15(0, 0, 0);
        BG_PALETTE[PAL_PROGRESS_TRACK] = rgb15(10, 10, 13);
    } else {
        BG_PALETTE[PAL_BACKGROUND] = rgb15(30, 30, 31);
        BG_PALETTE[PAL_TEXT] = rgb15(3, 3, 5);
        BG_PALETTE[PAL_ACCENT] = rgb15(14, 9, 25);
        BG_PALETTE[PAL_ACCENT_TEXT] = rgb15(31, 31, 31);
        BG_PALETTE[PAL_MUTED] = rgb15(14, 14, 17);
        BG_PALETTE[PAL_FOOTER] = rgb15(27, 27, 29);
        BG_PALETTE[PAL_PANEL] = rgb15(28, 28, 29);
        BG_PALETTE[PAL_SHADOW] = rgb15(19, 18, 22);
        BG_PALETTE[PAL_PROGRESS_TRACK] = rgb15(24, 24, 27);
    }
}

static inline void put_pixel(int x, int y, uint8_t colour)
{
    uint16_t *pixel_pair;
    uint16_t value;

    if ((unsigned)x >= SCREEN_WIDTH || (unsigned)y >= SCREEN_HEIGHT)
        return;

    pixel_pair = &draw_page[y * MODE4_STRIDE + (x >> 1)];
    value = *pixel_pair;
    if (x & 1)
        value = (uint16_t)((value & 0x00FF) | (colour << 8));
    else
        value = (uint16_t)((value & 0xFF00) | colour);
    *pixel_pair = value;
}

static void fill_screen(uint8_t colour)
{
    uint32_t packed = (uint32_t)(colour | (colour << 8));
    uint32_t *destination = (uint32_t *)draw_page;
    uint32_t index;

    packed |= packed << 16;
    for (index = 0; index < SCREEN_BYTE_SIZE / sizeof(*destination); ++index)
        destination[index] = packed;
}

static void fill_rect(int x, int y, int width, int height, uint8_t colour)
{
    int row;
    uint16_t packed = (uint16_t)(colour | (colour << 8));

    if (x < 0) {
        width += x;
        x = 0;
    }
    if (y < 0) {
        height += y;
        y = 0;
    }
    if (x + width > SCREEN_WIDTH)
        width = SCREEN_WIDTH - x;
    if (y + height > SCREEN_HEIGHT)
        height = SCREEN_HEIGHT - y;
    if (width <= 0 || height <= 0)
        return;

    if (x == 0 && width == SCREEN_WIDTH) {
        DMA3COPY(&packed, &draw_page[y * MODE4_STRIDE],
                 DMA_SRC_FIXED | DMA16 | (height * MODE4_STRIDE));
        return;
    }

    for (row = 0; row < height; ++row) {
        int left = x;
        int remaining = width;
        int pairs;
        int column;
        uint16_t *destination;

        if (left & 1) {
            put_pixel(left, y + row, colour);
            ++left;
            --remaining;
        }

        destination = &draw_page[(y + row) * MODE4_STRIDE + (left >> 1)];
        pairs = remaining >> 1;
        for (column = 0; column < pairs; ++column)
            destination[column] = packed;

        if (remaining & 1)
            put_pixel(left + pairs * 2, y + row, colour);
    }
}

static void fill_round_rect(int x, int y, int width, int height, uint8_t colour)
{
    fill_rect(x + 3, y, width - 6, height, colour);
    fill_rect(x, y + 3, width, height - 6, colour);
    fill_rect(x + 1, y + 1, width - 2, height - 2, colour);
}

enum TextAccent {
    TEXT_ACCENT_NONE,
    TEXT_ACCENT_ACUTE,
    TEXT_ACCENT_GRAVE,
    TEXT_ACCENT_CIRC,
    TEXT_ACCENT_TILDE,
    TEXT_ACCENT_DIAERESIS,
    TEXT_ACCENT_RING,
    TEXT_ACCENT_BREVE,
    TEXT_ACCENT_CEDILLA,
    TEXT_ACCENT_DOT
};

enum TextSpecial {
    TEXT_SPECIAL_NONE,
    TEXT_SPECIAL_DOTLESS_I,
    TEXT_SPECIAL_SHARP_S
};

static void draw_bitmap_glyph(int x, int y, const unsigned char *rows,
                              uint8_t colour, bool bold)
{
    int row;
    int column;

    for (row = 0; row < GLYPH_HEIGHT; ++row) {
        unsigned char bits = rows[row];
        if (bold)
            bits = (unsigned char)((bits | (bits >> 1)) & 0xFCu);
        for (column = 0; column < 8; ++column) {
            if (bits & (0x80u >> column))
                put_pixel(x + column, y + row, colour);
        }
    }
}

static void draw_ascii_glyph(int x, int y, uint8_t glyph, uint8_t colour,
                             bool bold)
{
    const unsigned char *rows = &ASC_DATA_NEW[(unsigned)glyph * GLYPH_HEIGHT];

    draw_bitmap_glyph(x, y, rows, colour, bold);
}

static void draw_custom_glyph(int x, int y, const unsigned char *rows,
                              uint8_t colour, bool bold)
{
    draw_bitmap_glyph(x, y, rows, colour, bold);
}

static void draw_accent_pixel(int x, int y, int px, int py, uint8_t colour,
                              bool bold)
{
    put_pixel(x + px, y + py, colour);
    if (bold && px < GLYPH_ADVANCE - 1)
        put_pixel(x + px + 1, y + py, colour);
}

static void draw_latin_accent(int x, int y, uint8_t colour, uint8_t accent,
                              uint8_t base, bool bold)
{
    int y_offset = accent == TEXT_ACCENT_DIAERESIS &&
                   (base == 'a' || base == 'e' || base == 'i' ||
                    base == 'o' || base == 'u' || base == 'y') ? 2 : 0;

    if (accent == TEXT_ACCENT_BREVE && base == 'g')
        y_offset = 1;

#define ACCENT_PIXEL(px, py) draw_accent_pixel(x, y, px, py, colour, bold)
    switch (accent) {
    case TEXT_ACCENT_ACUTE:
        ACCENT_PIXEL(4, y_offset); ACCENT_PIXEL(3, 1 + y_offset); break;
    case TEXT_ACCENT_GRAVE:
        ACCENT_PIXEL(2, y_offset); ACCENT_PIXEL(3, 1 + y_offset); break;
    case TEXT_ACCENT_CIRC:
        ACCENT_PIXEL(3, y_offset); ACCENT_PIXEL(2, 1 + y_offset);
        ACCENT_PIXEL(4, 1 + y_offset); break;
    case TEXT_ACCENT_TILDE:
        ACCENT_PIXEL(2, y_offset); ACCENT_PIXEL(4, y_offset);
        ACCENT_PIXEL(1, 1 + y_offset); ACCENT_PIXEL(3, 1 + y_offset); break;
    case TEXT_ACCENT_DIAERESIS:
        ACCENT_PIXEL(2, y_offset); ACCENT_PIXEL(4, y_offset); break;
    case TEXT_ACCENT_RING:
        ACCENT_PIXEL(3, y_offset); ACCENT_PIXEL(2, 1 + y_offset);
        ACCENT_PIXEL(4, 1 + y_offset); ACCENT_PIXEL(3, 2 + y_offset); break;
    case TEXT_ACCENT_BREVE:
        ACCENT_PIXEL(1, y_offset); ACCENT_PIXEL(5, y_offset);
        ACCENT_PIXEL(2, 1 + y_offset); ACCENT_PIXEL(3, 1 + y_offset);
        ACCENT_PIXEL(4, 1 + y_offset); break;
    case TEXT_ACCENT_CEDILLA:
        ACCENT_PIXEL(3, 10); ACCENT_PIXEL(2, 11); ACCENT_PIXEL(3, 11); break;
    case TEXT_ACCENT_DOT:
        ACCENT_PIXEL(3, 0); break;
    default:
        break;
    }
#undef ACCENT_PIXEL
}

static bool map_latin_glyph(uint16_t glyph, uint8_t *base, uint8_t *accent,
                            uint8_t *special)
{
    *accent = TEXT_ACCENT_NONE;
    *special = TEXT_SPECIAL_NONE;

#define MAP_LATIN(value, letter, mark) \
    case value: *base = letter; *accent = mark; return true
    switch (glyph) {
    MAP_LATIN(0x00C0, 'A', TEXT_ACCENT_GRAVE);
    MAP_LATIN(0x00C1, 'A', TEXT_ACCENT_ACUTE);
    MAP_LATIN(0x00C2, 'A', TEXT_ACCENT_CIRC);
    MAP_LATIN(0x00C3, 'A', TEXT_ACCENT_TILDE);
    MAP_LATIN(0x00C4, 'A', TEXT_ACCENT_DIAERESIS);
    MAP_LATIN(0x00C5, 'A', TEXT_ACCENT_RING);
    MAP_LATIN(0x00E0, 'a', TEXT_ACCENT_GRAVE);
    MAP_LATIN(0x00E1, 'a', TEXT_ACCENT_ACUTE);
    MAP_LATIN(0x00E2, 'a', TEXT_ACCENT_CIRC);
    MAP_LATIN(0x00E3, 'a', TEXT_ACCENT_TILDE);
    MAP_LATIN(0x00E4, 'a', TEXT_ACCENT_DIAERESIS);
    MAP_LATIN(0x00E5, 'a', TEXT_ACCENT_RING);
    MAP_LATIN(0x00C7, 'C', TEXT_ACCENT_CEDILLA);
    MAP_LATIN(0x00E7, 'c', TEXT_ACCENT_CEDILLA);
    MAP_LATIN(0x00C8, 'E', TEXT_ACCENT_GRAVE);
    MAP_LATIN(0x00C9, 'E', TEXT_ACCENT_ACUTE);
    MAP_LATIN(0x00CA, 'E', TEXT_ACCENT_CIRC);
    MAP_LATIN(0x00CB, 'E', TEXT_ACCENT_DIAERESIS);
    MAP_LATIN(0x00E8, 'e', TEXT_ACCENT_GRAVE);
    MAP_LATIN(0x00E9, 'e', TEXT_ACCENT_ACUTE);
    MAP_LATIN(0x00EA, 'e', TEXT_ACCENT_CIRC);
    MAP_LATIN(0x00EB, 'e', TEXT_ACCENT_DIAERESIS);
    MAP_LATIN(0x00CC, 'I', TEXT_ACCENT_GRAVE);
    MAP_LATIN(0x00CD, 'I', TEXT_ACCENT_ACUTE);
    MAP_LATIN(0x00CE, 'I', TEXT_ACCENT_CIRC);
    MAP_LATIN(0x00CF, 'I', TEXT_ACCENT_DIAERESIS);
    MAP_LATIN(0x00EC, 'i', TEXT_ACCENT_GRAVE);
    MAP_LATIN(0x00ED, 'i', TEXT_ACCENT_ACUTE);
    MAP_LATIN(0x00EE, 'i', TEXT_ACCENT_CIRC);
    MAP_LATIN(0x00EF, 'i', TEXT_ACCENT_DIAERESIS);
    MAP_LATIN(0x00D1, 'N', TEXT_ACCENT_TILDE);
    MAP_LATIN(0x00F1, 'n', TEXT_ACCENT_TILDE);
    MAP_LATIN(0x00D2, 'O', TEXT_ACCENT_GRAVE);
    MAP_LATIN(0x00D3, 'O', TEXT_ACCENT_ACUTE);
    MAP_LATIN(0x00D4, 'O', TEXT_ACCENT_CIRC);
    MAP_LATIN(0x00D5, 'O', TEXT_ACCENT_TILDE);
    MAP_LATIN(0x00D6, 'O', TEXT_ACCENT_DIAERESIS);
    MAP_LATIN(0x00F2, 'o', TEXT_ACCENT_GRAVE);
    MAP_LATIN(0x00F3, 'o', TEXT_ACCENT_ACUTE);
    MAP_LATIN(0x00F4, 'o', TEXT_ACCENT_CIRC);
    MAP_LATIN(0x00F5, 'o', TEXT_ACCENT_TILDE);
    MAP_LATIN(0x00F6, 'o', TEXT_ACCENT_DIAERESIS);
    MAP_LATIN(0x00D9, 'U', TEXT_ACCENT_GRAVE);
    MAP_LATIN(0x00DA, 'U', TEXT_ACCENT_ACUTE);
    MAP_LATIN(0x00DB, 'U', TEXT_ACCENT_CIRC);
    MAP_LATIN(0x00DC, 'U', TEXT_ACCENT_DIAERESIS);
    MAP_LATIN(0x00F9, 'u', TEXT_ACCENT_GRAVE);
    MAP_LATIN(0x00FA, 'u', TEXT_ACCENT_ACUTE);
    MAP_LATIN(0x00FB, 'u', TEXT_ACCENT_CIRC);
    MAP_LATIN(0x00FC, 'u', TEXT_ACCENT_DIAERESIS);
    MAP_LATIN(0x00DD, 'Y', TEXT_ACCENT_ACUTE);
    MAP_LATIN(0x00FD, 'y', TEXT_ACCENT_ACUTE);
    MAP_LATIN(0x00FF, 'y', TEXT_ACCENT_DIAERESIS);
    MAP_LATIN(0x011E, 'G', TEXT_ACCENT_BREVE);
    MAP_LATIN(0x011F, 'g', TEXT_ACCENT_BREVE);
    MAP_LATIN(0x0130, 'I', TEXT_ACCENT_DOT);
    MAP_LATIN(0x015E, 'S', TEXT_ACCENT_CEDILLA);
    MAP_LATIN(0x015F, 's', TEXT_ACCENT_CEDILLA);
    case 0x00DF: *special = TEXT_SPECIAL_SHARP_S; return true;
    case 0x0131: *special = TEXT_SPECIAL_DOTLESS_I; return true;
    default: return false;
    }
#undef MAP_LATIN
}

static void draw_glyph(int x, int y, uint16_t glyph, uint8_t colour, bool bold)
{
    static const unsigned char dotless_i[12] = {
        0x00, 0x00, 0x00, 0x00, 0x10, 0x10,
        0x10, 0x10, 0x10, 0x10, 0x00, 0x00
    };
    static const unsigned char sharp_s[12] = {
        0x00, 0x00, 0x70, 0x88, 0x88, 0x90,
        0xE0, 0x90, 0x88, 0xF0, 0x00, 0x00
    };
    uint8_t base;
    uint8_t accent;
    uint8_t special;

    if (glyph < 128) {
        draw_ascii_glyph(x, y, (uint8_t)glyph, colour, bold);
    } else if (map_latin_glyph(glyph, &base, &accent, &special)) {
        if (special == TEXT_SPECIAL_DOTLESS_I)
            draw_custom_glyph(x, y, dotless_i, colour, bold);
        else if (special == TEXT_SPECIAL_SHARP_S)
            draw_custom_glyph(x, y, sharp_s, colour, bold);
        else {
            draw_ascii_glyph(x, y, base, colour, bold);
            draw_latin_accent(x, y, colour, accent, base, bold);
        }
    } else {
        draw_ascii_glyph(x, y, '?', colour, bold);
    }
}

static void draw_ascii(int x, int y, const char *text, uint8_t colour)
{
    while (*text && x <= SCREEN_WIDTH - GLYPH_ADVANCE) {
        draw_glyph(x, y, (unsigned char)*text, colour, false);
        x += GLYPH_ADVANCE;
        ++text;
    }
}

static void draw_glyphs(int x, int y, const uint16_t *text, uint8_t colour)
{
    while (*text && x <= SCREEN_WIDTH - GLYPH_ADVANCE) {
        bool bold = (*text & GLYPH_BOLD_FLAG) != 0;
        draw_glyph(x, y, (uint16_t)(*text & GLYPH_VALUE_MASK), colour, bold);
        x += GLYPH_ADVANCE;
        ++text;
    }
}

static void present(void)
{
    VBlankIntrWait();
    if (page_one_visible) {
        REG_DISPCNT &= (uint16_t)~BACKBUFFER;
        draw_page = (uint16_t *)(0x06000000 + PAGE1_OFFSET);
    } else {
        REG_DISPCNT |= BACKBUFFER;
        draw_page = (uint16_t *)0x06000000;
    }
    page_one_visible = !page_one_visible;
}

static uint32_t decode_codepoint(const Document *document, uint32_t position,
                                 uint32_t *codepoint)
{
    const uint8_t *data = document->data;
    uint32_t size = document->size;
    uint8_t first = data[position];

    if (first < 0x80) {
        *codepoint = first;
        return position + 1;
    }

    if ((first & 0xE0) == 0xC0 && position + 1 < size &&
        (data[position + 1] & 0xC0) == 0x80) {
        uint32_t value = ((uint32_t)(first & 0x1F) << 6) |
                         (data[position + 1] & 0x3F);
        if (value >= 0x80) {
            *codepoint = value;
            return position + 2;
        }
    }

    if ((first & 0xF0) == 0xE0 && position + 2 < size &&
        (data[position + 1] & 0xC0) == 0x80 &&
        (data[position + 2] & 0xC0) == 0x80) {
        uint32_t value = ((uint32_t)(first & 0x0F) << 12) |
                         ((uint32_t)(data[position + 1] & 0x3F) << 6) |
                         (data[position + 2] & 0x3F);
        if (value >= 0x800) {
            *codepoint = value;
            return position + 3;
        }
    }

    *codepoint = first;
    return position + 1;
}

static uint16_t display_glyph(uint32_t codepoint)
{
    if (codepoint < 0x80 || (codepoint >= 0xA0 && codepoint <= 0xFF) ||
        codepoint == 0x011E || codepoint == 0x011F ||
        codepoint == 0x0130 || codepoint == 0x0131 ||
        codepoint == 0x015E || codepoint == 0x015F)
        return (uint16_t)codepoint;

    switch (codepoint) {
    case 0x80:
    case 0x20AC:
        return 'E';
    case 0x82:
    case 0x84:
    case 0x201A:
    case 0x201E:
        return ',';
    case 0x85:
    case 0x2026:
        return '.';
    case 0x91:
    case 0x92:
    case 0x2018:
    case 0x2019:
        return '\'';
    case 0x93:
    case 0x94:
    case 0x201C:
    case 0x201D:
        return '"';
    case 0x96:
    case 0x97:
    case 0x2013:
    case 0x2014:
        return '-';
    case 0xA0:
        return ' ';
    default:
        return '?';
    }
}

static uint32_t skip_inline_space(const Document *document, uint32_t position)
{
    while (position < document->size) {
        uint8_t value = document->data[position];
        if (value != ' ' && value != '\t')
            break;
        ++position;
    }
    return position;
}

static uint32_t line_position(uint32_t state)
{
    return state & LINE_POSITION_MASK;
}

static unsigned line_heading(uint32_t state)
{
    return (unsigned)((state & LINE_HEADING_MASK) >> LINE_HEADING_SHIFT);
}

static uint32_t make_line_state(uint32_t position, bool bold,
                                unsigned heading, bool rule)
{
    uint32_t state = position & LINE_POSITION_MASK;

    if (bold)
        state |= LINE_BOLD_FLAG;
    state |= ((uint32_t)(heading & 3u) << LINE_HEADING_SHIFT);
    if (rule)
        state |= LINE_RULE_FLAG;
    return state;
}

static uint32_t next_physical_line(const Document *document, uint32_t position)
{
    while (position < document->size && document->data[position] != '\r' &&
           document->data[position] != '\n')
        ++position;

    if (position < document->size) {
        uint8_t newline = document->data[position++];
        if (position < document->size &&
            ((newline == '\r' && document->data[position] == '\n') ||
             (newline == '\n' && document->data[position] == '\r')))
            ++position;
    }
    return position;
}

static bool markdown_rule_at(const Document *document, uint32_t position)
{
    uint32_t cursor = position;
    uint8_t marker;
    unsigned indentation = 0;
    unsigned count = 0;

    while (cursor < document->size && document->data[cursor] == ' ' &&
           indentation < 3) {
        ++cursor;
        ++indentation;
    }
    if (cursor >= document->size)
        return false;

    marker = document->data[cursor];
    if (marker != '-' && marker != '*' && marker != '_')
        return false;

    while (cursor < document->size && document->data[cursor] != '\r' &&
           document->data[cursor] != '\n') {
        uint8_t value = document->data[cursor++];
        if (value == marker)
            ++count;
        else if (value != ' ' && value != '\t')
            return false;
    }
    return count >= 3;
}

static uint32_t prepare_line_start(const Document *document, uint32_t position)
{
    uint32_t cursor;
    unsigned heading = 0;
    unsigned indentation = 0;

    if (!document->markdown || position >= document->size)
        return make_line_state(position, false, 0, false);

    if (markdown_rule_at(document, position))
        return make_line_state(position, false, 0, true);

    cursor = position;
    while (cursor < document->size && document->data[cursor] == ' ' &&
           indentation < 3) {
        ++cursor;
        ++indentation;
    }
    while (cursor + heading < document->size && heading < 3 &&
           document->data[cursor + heading] == '#')
        ++heading;

    if (heading > 0 && cursor + heading < document->size &&
        (document->data[cursor + heading] == ' ' ||
         document->data[cursor + heading] == '\t')) {
        cursor = skip_inline_space(document, cursor + heading);
        return make_line_state(cursor, false, heading, false);
    }

    return make_line_state(position, false, 0, false);
}

static bool has_bold_closer(const Document *document, uint32_t position,
                            uint8_t marker)
{
    while (position + 1 < document->size &&
           document->data[position] != '\r' &&
           document->data[position] != '\n') {
        if (document->data[position] == marker &&
            document->data[position + 1] == marker)
            return true;
        ++position;
    }
    return false;
}

static uint32_t layout_line(const Document *document, uint32_t state,
                            uint16_t *output)
{
    uint32_t position = line_position(state);
    uint32_t start = position;
    uint32_t last_break_position = UINT32_MAX;
    bool bold = (state & LINE_BOLD_FLAG) != 0;
    bool last_break_bold = bold;
    unsigned heading = line_heading(state);
    unsigned last_break_column = 0;
    unsigned column = 0;

    if (output)
        output[0] = 0;

    if (state & LINE_RULE_FLAG)
        return prepare_line_start(document,
                                  next_physical_line(document, position));

    while (position < document->size) {
        uint32_t token_position = position;
        uint32_t codepoint;
        uint32_t next_position;
        unsigned width = 1;
        bool whitespace;

        if (document->data[position] == '\r' ||
            document->data[position] == '\n') {
            uint8_t newline = document->data[position++];
            if (position < document->size &&
                ((newline == '\r' && document->data[position] == '\n') ||
                 (newline == '\n' && document->data[position] == '\r')))
                ++position;
            if (output)
                output[column] = 0;
            return prepare_line_start(document, position);
        }

        if (document->markdown && position + 1 < document->size &&
            ((document->data[position] == '*' &&
              document->data[position + 1] == '*') ||
             (document->data[position] == '_' &&
              document->data[position + 1] == '_'))) {
            uint8_t marker = document->data[position];
            if (bold || has_bold_closer(document, position + 2, marker)) {
                bold = !bold;
                position += 2;
                continue;
            }
        }

        next_position = decode_codepoint(document, position, &codepoint);
        if (codepoint == 0xFEFF) {
            position = next_position;
            continue;
        }

        whitespace = codepoint == ' ' || codepoint == '\t' || codepoint == 0xA0;
        if (codepoint == '\t')
            width = 4 - (column & 3);

        if (column + width > TEXT_COLUMNS) {
            if (last_break_position != UINT32_MAX) {
                if (output)
                    output[last_break_column] = 0;
                return make_line_state(
                    skip_inline_space(document, last_break_position),
                    last_break_bold, heading, false);
            }
            if (output)
                output[column] = 0;
            return make_line_state(token_position > start ? token_position
                                                            : next_position,
                                   bold, heading, false);
        }

        if (whitespace) {
            if (column == 0) {
                position = next_position;
                start = position;
                continue;
            }
            last_break_position = next_position;
            last_break_column = column;
            last_break_bold = bold;
            while (width-- > 0 && column < TEXT_COLUMNS) {
                if (output)
                    output[column] = ' ';
                ++column;
            }
        } else {
            if (output) {
                uint16_t glyph = display_glyph(codepoint);
                if (bold || heading > 0)
                    glyph |= GLYPH_BOLD_FLAG;
                output[column] = glyph;
            }
            ++column;
        }

        position = next_position;
    }

    if (output)
        output[column] = 0;
    return make_line_state(document->size, false, 0, false);
}

static void build_line_index(const Document *document)
{
    uint32_t state;
    uint32_t position = 0;

    line_count = 0;
    index_truncated = false;

    if (document->size >= 3 && document->data[0] == 0xEF &&
        document->data[1] == 0xBB && document->data[2] == 0xBF)
        position = 3;

    state = prepare_line_start(document, position);
    if (position >= document->size) {
        line_offsets[line_count++] = state;
        return;
    }

    while (line_position(state) < document->size &&
           line_count < MAX_VISUAL_LINES) {
        uint32_t next;
        uint32_t current_position = line_position(state);

        line_offsets[line_count++] = state;
        next = layout_line(document, state, NULL);
        if (line_position(next) <= current_position)
            next = prepare_line_start(document, current_position + 1);
        state = next;
    }

    index_truncated = line_position(state) < document->size;
}

static const char *basename_only(const char *name)
{
    const char *base = name;
    const char *cursor = name;

    while (*cursor) {
        if (*cursor == '/' || *cursor == '\\')
            base = cursor + 1;
        ++cursor;
    }
    return *base ? base : "Text Viewer";
}

static char ascii_lower(char value)
{
    return value >= 'A' && value <= 'Z' ? (char)(value + ('a' - 'A')) : value;
}

static bool has_extension(const char *name, const char *extension)
{
    size_t name_length = strlen(name);
    size_t extension_length = strlen(extension);
    size_t index;

    if (name_length <= extension_length ||
        name[name_length - extension_length - 1] != '.')
        return false;

    for (index = 0; index < extension_length; ++index) {
        if (ascii_lower(name[name_length - extension_length + index]) !=
            ascii_lower(extension[index]))
            return false;
    }
    return true;
}

static const char *bounded_string_end(const char *text, const char *limit)
{
    while (text < limit && *text)
        ++text;
    return text < limit ? text : NULL;
}

static Document load_document(void)
{
    Document document;
    bool valid = POGO_ARGS[0] == POGO_MAGIC && POGO_ARGS[1] >= 2;

    if (valid) {
        uint32_t address = POGO_FILE_ADDRESS;
        uint32_t size = POGO_FILE_SIZE;
        const char *argument_limit = (const char *)0x02040000;
        const char *command = (const char *)&POGO_ARGS[2];
        const char *command_end = bounded_string_end(command, argument_limit);
        const char *filename = command_end ? command_end + 1 : argument_limit;
        const char *filename_end = bounded_string_end(filename, argument_limit);

        valid = command_end && filename_end && filename < argument_limit &&
                address >= 0x08000000 && address < 0x0A000000 &&
                size <= 0x02000000 && address + size >= address &&
                address + size <= 0x0A000000;

        if (valid) {
            document.data = (const uint8_t *)address;
            document.size = size;
            document.name = basename_only(filename);
            document.markdown = has_extension(document.name, "md") ||
                                has_extension(document.name, "markdown");
            return document;
        }
    }

    document.data = (const uint8_t *)preview_text;
    document.size = sizeof(preview_text) - 1;
    document.name = "Text Viewer Preview.md";
    document.markdown = true;
    return document;
}

static void draw_header(const Document *document)
{
    char title[33];
    size_t length = strlen(document->name);
    size_t limit = sizeof(title) - 1;

    if (length <= limit) {
        memcpy(title, document->name, length + 1);
    } else {
        memcpy(title, document->name, limit - 3);
        memcpy(title + limit - 3, "...", 4);
    }

    fill_rect(0, 0, SCREEN_WIDTH, HEADER_HEIGHT, PAL_ACCENT);
    draw_ascii(5, 4, title, PAL_ACCENT_TEXT);
}

static void draw_header_progress(uint32_t current_line)
{
    char progress[6];
    uint32_t max_top = line_count > TEXT_ROWS ? line_count - TEXT_ROWS : 0;
    unsigned percent = max_top ? (unsigned)((current_line * 100u) / max_top) : 100;

    if (percent > 100)
        percent = 100;
    progress[0] = (percent == 100) ? '1' : (percent >= 10 ? (char)('0' + percent / 10) : ' ');
    progress[1] = (percent == 100) ? '0' : (char)('0' + percent % 10);
    progress[2] = (percent == 100) ? '0' : '%';
    progress[3] = (percent == 100) ? '%' : (index_truncated ? '+' : '\0');
    progress[4] = '\0';
    fill_rect(HEADER_PROGRESS_X, 0, SCREEN_WIDTH - HEADER_PROGRESS_X,
              HEADER_HEIGHT, PAL_ACCENT);
    draw_ascii(SCREEN_WIDTH - (int)strlen(progress) * GLYPH_ADVANCE - 5, 3,
               progress, PAL_ACCENT_TEXT);
}

static void draw_footer_progress(uint32_t current_line)
{
    uint32_t max_top = line_count > TEXT_ROWS ? line_count - TEXT_ROWS : 0;
    int progress_width = max_top ? (int)((current_line * SCREEN_WIDTH) / max_top)
                                 : SCREEN_WIDTH;

    fill_rect(0, 143, SCREEN_WIDTH, 2, PAL_PROGRESS_TRACK);
    fill_rect(0, 143, progress_width, 2, PAL_ACCENT);
}

static void draw_footer(uint32_t current_line)
{
    draw_footer_progress(current_line);
    fill_rect(0, FOOTER_TOP, SCREEN_WIDTH, SCREEN_HEIGHT - FOOTER_TOP,
              PAL_FOOTER);
    draw_ascii(4, 146, "START Help", PAL_MUTED);
    draw_ascii(84, 146, "SELECT Theme", PAL_MUTED);
    draw_ascii(200, 146, "B Exit", PAL_MUTED);
}

static void draw_visual_line(const Document *document, uint32_t line,
                             unsigned row)
{
    uint32_t state;
    int y;

    if (line >= line_count || row >= TEXT_ROWS)
        return;

    state = line_offsets[line];
    y = TEXT_Y + (int)row * TEXT_LINE_HEIGHT;
    if (state & LINE_RULE_FLAG) {
        fill_rect(10, y + 5, SCREEN_WIDTH - 20, 1, PAL_ACCENT);
    } else {
        uint16_t text[TEXT_COLUMNS + 1];
        uint8_t colour = line_heading(state) > 0 ? PAL_ACCENT : PAL_TEXT;

        layout_line(document, state, text);
        draw_glyphs(TEXT_X, y, text, colour);
    }
}

static void draw_document(const Document *document, uint32_t current_line)
{
    uint32_t row;

    fill_screen(PAL_BACKGROUND);
    draw_header(document);
    draw_header_progress(current_line);

    if (document->size == 0) {
        draw_ascii(75, 75, "Empty text file", PAL_MUTED);
    } else {
        for (row = 0; row < TEXT_ROWS && current_line + row < line_count; ++row)
            draw_visual_line(document, current_line + row, row);
    }

    draw_footer(current_line);
}

static void draw_controls(void)
{
    fill_round_rect(14, 23, 216, 118, PAL_SHADOW);
    fill_round_rect(12, 21, 216, 118, PAL_PANEL);
    draw_ascii(23, 29, "Controls", PAL_ACCENT);
    draw_ascii(23, 44, "Up / Down     One line", PAL_TEXT);
    draw_ascii(23, 57, "Left / Right  One page", PAL_TEXT);
    draw_ascii(23, 70, "L / R         One page", PAL_TEXT);
    draw_ascii(23, 83, "A             Next page", PAL_TEXT);
    draw_ascii(23, 96, "Select        Light / dark", PAL_TEXT);
    draw_ascii(23, 109, "B             Exit", PAL_TEXT);
    draw_ascii(23, 122, "Start         Close", PAL_TEXT);
}

static uint16_t *visible_page(void)
{
    return page_one_visible ? (uint16_t *)(0x06000000 + PAGE1_OFFSET)
                            : (uint16_t *)0x06000000;
}

static void render_full(const Document *document, uint32_t current_line)
{
    draw_document(document, current_line);
    if (showing_controls)
        draw_controls();
    present();
    dmaCopy(visible_page(), draw_page, SCREEN_BYTE_SIZE);
}

static bool render_line_scroll(const Document *document, uint32_t old_line,
                               uint32_t current_line)
{
    uint16_t *source = visible_page();
    const unsigned copied_rows = (TEXT_ROWS - 1) * TEXT_LINE_HEIGHT;

    if (current_line == old_line + 1) {
        dmaCopy(&source[(TEXT_Y + TEXT_LINE_HEIGHT) * MODE4_STRIDE],
                &draw_page[TEXT_Y * MODE4_STRIDE],
                copied_rows * SCREEN_WIDTH);
        fill_rect(0, TEXT_Y + (TEXT_ROWS - 1) * TEXT_LINE_HEIGHT,
                  SCREEN_WIDTH, TEXT_LINE_HEIGHT, PAL_BACKGROUND);
        draw_visual_line(document, current_line + TEXT_ROWS - 1,
                         TEXT_ROWS - 1);
    } else if (old_line == current_line + 1) {
        dmaCopy(&source[TEXT_Y * MODE4_STRIDE],
                &draw_page[(TEXT_Y + TEXT_LINE_HEIGHT) * MODE4_STRIDE],
                copied_rows * SCREEN_WIDTH);
        fill_rect(0, TEXT_Y, SCREEN_WIDTH, TEXT_LINE_HEIGHT, PAL_BACKGROUND);
        draw_visual_line(document, current_line, 0);
    } else {
        return false;
    }

    draw_header_progress(current_line);
    draw_footer_progress(current_line);
    present();
    return true;
}

int main(void)
{
    Document document;
    uint32_t current_line = 0;

    irqInit();
    irqEnable(IRQ_VBLANK);
    REG_DISPCNT = MODE_4 | BG2_ENABLE;
    draw_page = (uint16_t *)(0x06000000 + PAGE1_OFFSET);
    page_one_visible = false;
    dark_theme = false;
#ifdef VIEWER_PREVIEW_CONTROLS
    showing_controls = true;
#else
    showing_controls = false;
#endif
    setRepeat(15, 2);
    set_palette();

    document = load_document();
    fill_screen(PAL_BACKGROUND);
    fill_rect(0, 0, SCREEN_WIDTH, HEADER_HEIGHT, PAL_ACCENT);
    draw_ascii(5, 3, "Opening text...", PAL_ACCENT_TEXT);
    present();
    build_line_index(&document);
    render_full(&document, current_line);

    for (;;) {
        uint16_t pressed;
        uint16_t repeated;
        uint16_t held;
        uint32_t max_top;
        uint32_t old_line;
        bool full_redraw = false;

        VBlankIntrWait();
        scanKeys();
        pressed = keysDown();
        repeated = keysDownRepeat();
        held = keysHeld();
        max_top = line_count > TEXT_ROWS ? line_count - TEXT_ROWS : 0;
        old_line = current_line;

        if ((held & (KEY_L | KEY_R)) == (KEY_L | KEY_R))
            return_to_ds_style();

        if (showing_controls) {
            if (pressed & (KEY_START | KEY_B | KEY_A)) {
                showing_controls = false;
                full_redraw = true;
            }
        } else if (pressed & KEY_B) {
            return_to_ds_style();
        } else {
            if ((repeated & KEY_UP) && current_line > 0)
                --current_line;
            if ((repeated & KEY_DOWN) && current_line < max_top)
                ++current_line;
            if ((repeated & (KEY_LEFT | KEY_L)) && current_line > 0) {
                current_line = current_line > TEXT_ROWS
                                   ? current_line - TEXT_ROWS
                                   : 0;
            }
            if ((repeated & (KEY_RIGHT | KEY_R)) || (pressed & KEY_A)) {
                current_line = current_line + TEXT_ROWS < max_top
                                   ? current_line + TEXT_ROWS
                                   : max_top;
            }
            if (pressed & KEY_SELECT) {
                dark_theme = !dark_theme;
                set_palette();
                full_redraw = true;
            }
            if (pressed & KEY_START) {
                showing_controls = true;
                full_redraw = true;
            }
        }

        if (full_redraw) {
            render_full(&document, current_line);
        } else if (current_line != old_line &&
                   !render_line_scroll(&document, old_line, current_line)) {
            render_full(&document, current_line);
        }
    }
}
