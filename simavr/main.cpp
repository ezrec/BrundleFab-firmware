/*
 * Copyright (C) 2015, Jason S. McMullan
 * All right reserved.
 * Author: Jason S. McMullan <jason.mcmullan@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include "Arduino.h"

#include <HardwareSerial.h>
#include <SD.h>

#include <SDL.h>

#include "main.h"

HardwareSerial Serial;
SDClass SD;

static struct {
    SDL_Surface *gui;
} _update;

void simavr_update_gui(SDL_Surface *surf)
{
    _update.gui = surf;
}

int main(int argc, char **argv)
{
    bool dead = false;
    unsigned long update_timeout = millis();

    SDL_Init(SDL_INIT_VIDEO);

    setup();
    do {
        unsigned long now = millis();
        SDL_Event ev;

        loop();

        if (update_timeout < now) {
            if (_update.gui) {
                SDL_Flip(_update.gui);
                _update.gui = NULL;
            }
            update_timeout = now + 57;
        }

        if (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_KEYDOWN) {
                switch (ev.key.keysym.sym) {
                case SDLK_DOWN:  analogWrite(4, 20); break;
                case SDLK_RIGHT: analogWrite(4, 180); break;
                case SDLK_RETURN: analogWrite(4, 280); break;
                case SDLK_UP:    analogWrite(4, 380); break;
                case SDLK_LEFT:  analogWrite(4, 630); break;

                case SDLK_ESCAPE: dead = true; break;
                default: break;
                }
            } else if (ev.type == SDL_KEYUP) {
                analogWrite(4, 700);
            } else if (ev.type == SDL_QUIT) {
                dead = true;
            }
        }
    } while (!dead);

    SDL_Quit();
    Serial.end();
}

/* vim: set shiftwidth=4 expandtab:  */
