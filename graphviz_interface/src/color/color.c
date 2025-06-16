#include "color.h"
#include "colors.h"

#define DEFAULT_COLOR 0x000000ff

/**
 * @brief Convert an hsv color to an rgb color
 *
 * @param h the hue of the color
 * @param s the saturation of the color
 * @param v the value of the color
 * @return int the color in rgba format
 */
int hsv_to_rgb(float h, float s, float v) {
    if (h < 0 || h > 1 || s < 0 || s > 1 || v < 0 || v > 1) {
        return DEFAULT_COLOR;
    }
    h *= 360;
    float C = v * s;
    float X = C * (1 - fabs(fmod((h / 60), 2.0f) - 1));
    float m = v - C;
    float r, g, b;
    if (h < 60) {
        r = C;
        g = X;
        b = 0;
    } else if (h < 120) {
        r = X;
        g = C;
        b = 0;
    } else if (h < 180) {
        r = 0;
        g = C;
        b = X;
    } else if (h < 240) {
        r = 0;
        g = X;
        b = C;
    } else if (h < 300) {
        r = X;
        g = 0;
        b = C;
    } else {
        r = C;
        g = 0;
        b = X;
    }
    int rp = (r + m) * 255;
    int gp = (g + m) * 255;
    int bp = (b + m) * 255;
    return rp << 24 | gp << 16 | bp << 8 | 0xff;
}

/**
 * @brief Convert a color name to an rgb color
 *
 * @param name the name of the color in x11 colors list
 * @return int the color in rgba format
 */
int name_to_rgb(const char *name) {
    size_t color_table_len = sizeof(colors) / sizeof(colors[0]);

    for (int i = 0; i < color_table_len; i++) {
        int result = strcmp(name, colors[i].name);
        if (result == 0) {
            return colors[i].rgb << 8 | 0xff;
        } else if (result < 0) {
            // colors are sorted in alphabetical order, so if we pass a color after the one we are looking
            // for, we can break the loop
            return DEFAULT_COLOR;
        }
    }
    return DEFAULT_COLOR;
}

/**
 * @brief Convert a hex string to an rgb color
 *
 * @param name a string in the format "#rrggbb"
 * @return int the color in rgba format
 */
int hex_to_rgb(const char *name) {
    int r, g, b;
    if (sscanf(name, "#%02x%02x%02x", &r, &g, &b) != 3) {
        return DEFAULT_COLOR;
    }
    return r << 24 | g << 16 | b << 8 | 0xff;
}

/**
 * @brief Convert a shortend hex string to an rgb color
 *
 * @param name a string in the format "#rgb"
 * @return int the color in rgba format
 */
int shortend_hex_to_rgb(const char *name) {
    int r, g, b;
    if (sscanf(name, "#%01x%01x%01x", &r, &g, &b) != 3) {
        return DEFAULT_COLOR;
    }
    return r << 24 | g << 16 | b << 8 | 0xff;
}

/**
 * @brief Convert a hex string to an rgba color
 *
 * @param name a string in the format "#rrggbbaa"
 * @return int the color in rgba format
 */
int hex_to_rgba(const char *name) {
    int r, g, b, a;
    if (sscanf(name, "#%02x%02x%02x%02x", &r, &g, &b, &a) != 4) {
        return DEFAULT_COLOR;
    }
    return r << 24 | g << 16 | b << 8 | a;
}

/**
 * @brief Convert hsv string to int color
 *
 * @param hsv a string in the format "h s v" or "h s v a"
 * @return int the color in rgba format
 */
int hsv_to_int(const char *hsv) {
    float h, s, v, a;
    int scanned = sscanf(hsv, "%f %f %f %f", &h, &s, &v, &a);
    int rgb = hsv_to_rgb(h, s, v);
    if (scanned == 3) {
        return rgb;
    } else if (scanned == 4) {
        return rgb & (int)(a * 255);
    }
    return DEFAULT_COLOR;
}

int color_to_int(const char *color) {
    if (color == NULL) {
        return DEFAULT_COLOR;
    }
    if (color[0] == '#') {
        if (strlen(color) == 7) {
            return hex_to_rgb(color);
        } else if (strlen(color) == 4) {
            return shortend_hex_to_rgb(color);
        } else if (strlen(color) == 9) {
            return hex_to_rgba(color);
        }
    } else if (color[0] == '0' || color[0] == '1') {
        return hsv_to_int(color);
    }
    return name_to_rgb(color);
}
