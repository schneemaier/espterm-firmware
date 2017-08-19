#ifndef SCREEN_H
#define SCREEN_H

#include <stdint.h>
#include <stdbool.h>
#include <esp8266.h>
#include <httpd.h>

/**
 * This module handles the virtual screen and operations on it.
 *
 * It is interfaced by calls from the ANSI parser, and the screen
 * data can be rendered for the front-end.
 *
 * ---
 *
 * Colors are 0-15, 0-7 dim, 8-15 bright.
 *
 * NORMAL
 * 0 black, 1 red, 2 green, 3 yellow
 * 4 blue, 5 mag, 6 cyan, 7 white
 *
 * BRIGHT
 * 8 black, 9 red, 10 green, 11 yellow
 * 12 blue, 13 mag, 14 cyan, 15 white
 *
 * ints are 0-based, left-top is the origin.
 * X grows to the right, Y to the bottom.
 *
 * +---->
 * |    X
 * |
 * V Y
 *
 */

// Size designed for the terminal config structure
// Must be constant to avoid corrupting user config after upgrade
#define TERMCONF_SIZE 200

#define TERM_BTN_LEN 10
#define TERM_TITLE_LEN 64

typedef enum {
	CHANGE_CONTENT,
	CHANGE_LABELS,
} ScreenNotifyChangeTopic;

#define ATTR_BOLD (1<<0)
#define ATTR_FAINT (1<<1)
#define ATTR_ITALIC (1<<2)
#define ATTR_UNDERLINE (1<<3)
#define ATTR_BLINK (1<<4)
#define ATTR_FRAKTUR (1<<5)
#define ATTR_STRIKE (1<<6)

#define SCR_DEF_DISPLAY_TOUT_MS 20
#define SCR_DEF_PARSER_TOUT_MS 10
#define SCR_DEF_FN_ALT_MODE false
#define SCR_DEF_WIDTH 26
#define SCR_DEF_HEIGHT 10
#define SCR_DEF_TITLE "ESPTerm"

/** Maximum screen size (determines size of the static data array) */
#define MAX_SCREEN_SIZE (80*25)

typedef struct {
	u32 width;
	u32 height;
	u8 default_bg;
	u8 default_fg;
	char title[TERM_TITLE_LEN];
	char btn[5][TERM_BTN_LEN];
	u8 theme;
	u32 parser_tout_ms;
	u32 display_tout_ms;
	bool fn_alt_mode; // xterm compatibility mode (alternate codes for some FN keys)
} TerminalConfigBundle;

// Live config
extern TerminalConfigBundle * const termconf;

/**
 * Transient live config with no persist, can be modified via esc sequences.
 * terminal_apply_settings() copies termconf to this struct, erasing old scratch changes
 */
extern TerminalConfigBundle termconf_scratch;

void terminal_restore_defaults(void);
void terminal_apply_settings(void);
void terminal_apply_settings_noclear(void); // the same, but with no screen reset / init


typedef enum {
	CLEAR_TO_CURSOR=0, CLEAR_FROM_CURSOR=1, CLEAR_ALL=2
} ClearMode;

typedef uint8_t Color;

httpd_cgi_state screenSerializeToBuffer(char *buffer, size_t buf_len, void **data);

void screenSerializeLabelsToBuffer(char *buffer, size_t buf_len);

typedef struct {
	u8 lsb;
	u8 msb;
} WordB2;

typedef struct {
	u8 lsb;
	u8 msb;
	u8 xsb;
} WordB3;

/** Encode number to two nice ASCII bytes */
void encode2B(u16 number, WordB2 *stru);

/** Init the screen */
void screen_init(void);
/** Change the screen size */
void screen_resize(int rows, int cols);
/** Check if coord is valid */
bool screen_isCoordValid(int y, int x);

// --- Clearing ---

/** Screen reset to default state */
void screen_reset(void);
/** Clear entire screen */
void screen_clear(ClearMode mode);
/** Clear line */
void screen_clear_line(ClearMode mode);
/** Clear part of line */
void screen_clear_in_line(unsigned int count);
/** Shift screen upwards */
void screen_scroll_up(unsigned int lines);
/** Shift screen downwards */
void screen_scroll_down(unsigned int lines);
/** esc # 8 - fill entire screen with E of default colors (DEC alignment display) */
void screen_fill_with_E(void);

// --- insert / delete ---
void screen_insert_lines(unsigned int lines);
void screen_delete_lines(unsigned int lines);
void screen_insert_characters(unsigned int count);
void screen_delete_characters(unsigned int count);

// --- Cursor control ---

/** Set cursor position */
void screen_cursor_set(int y, int x);
/** Read cursor pos to given vars */
void screen_cursor_get(int *y, int *x);
/** Set cursor X position */
void screen_cursor_set_x(int x);
/** Set cursor Y position */
void screen_cursor_set_y(int y);
/** Reset cursor attribs */
void screen_reset_sgr(void);
/** Relative cursor move */
void screen_cursor_move(int dy, int dx, bool scroll);
/** Save the cursor pos */
void screen_cursor_save(bool withAttrs);
/** Restore the cursor pos */
void screen_cursor_restore(bool withAttrs);
/** Enable cursor display */
void screen_cursor_visible(bool visible);
/** Enable auto wrap */
void screen_wrap_enable(bool enable);
/** Enable CR auto */
void screen_set_newline_mode(bool nlm);

// --- Colors ---

/** Set cursor foreground color */
void screen_set_fg(Color color);
/** Set cursor background coloor */
void screen_set_bg(Color color);

/** enable attrs by bitmask */
void screen_attr_enable(u8 attrs);
/** disable attrs by bitmask */
void screen_attr_disable(u8 attrs);
/** Set the inverse cursor attribute */
void screen_inverse_enable(bool ena);
/** Toggle INSERT / REPLACE */
void screen_set_insert_mode(bool insert);
/** Toggle application keypad mode */
void screen_set_numpad_alt_mode(bool app_mode);
/** Toggle application cursor mode */
void screen_set_cursors_alt_mode(bool app_mode);

void screen_set_charset_n(int Gx);
void screen_set_charset(int Gx, char charset);

/**
 * Set a character in the cursor color, move to right with wrap.
 * The character may be ASCII (then only one char is used), or
 * unicode (then it can be 4 chars, or terminated by a zero)
 */
void screen_putchar(const char *ch);

#if 0
/** Debug dump */
void screen_dd(void);
#endif

extern void screen_notifyChange(ScreenNotifyChangeTopic topic);

#endif // SCREEN_H
