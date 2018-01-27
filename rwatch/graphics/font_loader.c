/* font_loader.c
 * routines for [...]
 * libRebbleOS
 *
 * Author: Barry Carter <barry.carter@gmail.com>
 */

#include "rebbleos.h"
#include "librebble.h"

// TODO This is still somewhat sketchy in that I'm not convinced some of these magic offsets
// are right

uint16_t _fonts_get_resource_id_for_key(const char *key);


GFont fonts_get_system_font_by_resource_id(uint32_t resource_id);

typedef struct GFontCache
{
    uint32_t resource_id;
    GFont font;
} GFontCache;

static GFontCache _cached_fonts[5];
static uint8_t _cached_count = 0;

// get a system font and then cache it. Ugh.
// TODO make this not suck (RAM)
GFont fonts_get_system_font(const char *font_key)
{
    uint16_t res_id = _fonts_get_resource_id_for_key(font_key);
    
    return fonts_get_system_font_by_resource_id(res_id);
}

/*
 * Load a system font from the resource table
 * Will save into a cheesey cache so it isn't loaded over and over.
 */
GFont fonts_get_system_font_by_resource_id(uint32_t resource_id)
{
    if (_cached_count == 0)
    {
        for (uint8_t i = 0; i < 5; i++)
            _cached_fonts[i].resource_id = 0;
    }
    else
    {
        for (uint8_t i = 0; i < 5; i++)
        {
            if (_cached_fonts[i].resource_id == resource_id)
                return _cached_fonts[i].font;
        }
    }

    uint8_t *buffer = resource_fully_load_id_system(resource_id);

    GFont font = (GFont)buffer;

    _cached_fonts[_cached_count].resource_id = resource_id;
    _cached_fonts[_cached_count].font = font;
    _cached_count++;

    return font;
}

/*
 * Load a custom font
 */
GFont *fonts_load_custom_font(ResHandle *handle, const struct file* file)
{
    // The font is offset. account for it.
    //handle->offset += APP_FONT_START;
    
    uint8_t *buffer = resource_fully_load_res_app(*handle, file);

    return (GFont *)buffer;
}

/*
 * Unload a custom font
 */
void fonts_unload_custom_font(GFont font)
{
    app_free(font);
}

#define EQ_FONT(font) (strncmp(key, font, strlen(key)) == 0) return font ## _ID;

/*
 * Load a font by a string key
 */
uint16_t _fonts_get_resource_id_for_key(const char *key)
{   
    // this seems kinda.... messy and bad. Why a char * Pebble? why pass strings around?
    // I got my answer from Heiko:
    /*
     @Heiko 
     That API has been around forever. Strings are an easy way to maintain an ABI contract between app and firmware compiled at different times. Was helpful as different SDK versions and models introduced various new fonts over time. It also allows for "secret fonts" that were not (yet) published. While one could accomplish the same with enums that have gaps and vary over time we already had those names in the firmware. And of course we had to maintain backwards compatibility when one of the fonts was renamed... again old API :wink:
      
     */
    // so still seems like a bad choice, but backward compat.
    if EQ_FONT(FONT_KEY_FONT_FALLBACK)
    else if EQ_FONT(FONT_KEY_GOTHIC_09)
    else if EQ_FONT(FONT_KEY_GOTHIC_14)
    else if EQ_FONT(FONT_KEY_GOTHIC_14_BOLD)
    else if EQ_FONT(FONT_KEY_GOTHIC_18)
    else if EQ_FONT(FONT_KEY_GOTHIC_18_BOLD)
    else if EQ_FONT(FONT_KEY_GOTHIC_24)
    else if EQ_FONT(FONT_KEY_GOTHIC_24_BOLD)
    else if EQ_FONT(FONT_KEY_GOTHIC_28)
    else if EQ_FONT(FONT_KEY_GOTHIC_28_BOLD)

    else if EQ_FONT(FONT_KEY_BITHAM_30_BLACK)
    else if EQ_FONT(FONT_KEY_BITHAM_42_BOLD)
    else if EQ_FONT(FONT_KEY_BITHAM_42_LIGHT)
    else if EQ_FONT(FONT_KEY_BITHAM_42_MEDIUM_NUMBERS)
    else if EQ_FONT(FONT_KEY_BITHAM_34_MEDIUM_NUMBERS)
    else if EQ_FONT(FONT_KEY_BITHAM_34_LIGHT_SUBSET)
    else if EQ_FONT(FONT_KEY_BITHAM_18_LIGHT_SUBSET)

    else if EQ_FONT(FONT_KEY_ROBOTO_CONDENSED_21)
    else if EQ_FONT(FONT_KEY_ROBOTO_BOLD_SUBSET_49)
    else if EQ_FONT(FONT_KEY_DROID_SERIF_28_BOLD)

    else if EQ_FONT(FONT_KEY_LECO_20_BOLD_NUMBERS)
    else if EQ_FONT(FONT_KEY_LECO_26_BOLD_NUMBERS_AM_PM)
    else if EQ_FONT(FONT_KEY_LECO_28_LIGHT_NUMBERS)
    else if EQ_FONT(FONT_KEY_LECO_32_BOLD_NUMBERS)
    else if EQ_FONT(FONT_KEY_LECO_36_BOLD_NUMBERS)
    else if EQ_FONT(FONT_KEY_LECO_38_BOLD_NUMBERS)
    else if EQ_FONT(FONT_KEY_LECO_42_NUMBERS)

    else if EQ_FONT(FONT_KEY_AGENCY_FB_60_THIN_NUMBERS_AM_PM)
    else if EQ_FONT(FONT_KEY_AGENCY_FB_60_NUMBERS_AM_PM)
    else if EQ_FONT(FONT_KEY_AGENCY_FB_36_NUMBERS_AM_PM)
        
    return FONT_KEY_FONT_FALLBACK_ID;
}
