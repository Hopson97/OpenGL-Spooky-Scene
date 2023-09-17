/*
 * Nuklear - 1.32.0 - public domain
 * no warrenty implied; use at your own risk.
 * authored from 2015-2016 by Micha Mettke
 */
#pragma once

#include "nuklear_def.h"
#include <SFML/Window.hpp>

NK_API struct nk_context* nk_sfml_init(sf::Window* window);
NK_API void nk_sfml_font_stash_begin(struct nk_font_atlas** atlas);
NK_API void nk_sfml_font_stash_end(void);
NK_API int nk_sfml_handle_event(sf::Event* event);
NK_API void nk_sfml_render(enum nk_anti_aliasing, int max_vertex_buffer,
                           int max_element_buffer);
NK_API void nk_sfml_shutdown(void);

NK_API void nk_sfml_device_create(void);
NK_API void nk_sfml_device_destroy(void);

// demo stuff
enum nk_theme
{
    THEME_BLACK,
    THEME_WHITE,
    THEME_RED,
    THEME_BLUE,
    THEME_DARK
};
int nk_overview(struct nk_context* ctx);
int nk_node_editor(struct nk_context* ctx);
void nk_calculator(struct nk_context* ctx);
void nk_set_style(struct nk_context* ctx, enum nk_theme theme);