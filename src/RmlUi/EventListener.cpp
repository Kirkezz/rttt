#include "EventListener.hpp"
#include "../RTTT_Engine.hpp"

#include <SDL3/SDL_misc.h>

#include <iostream>

void ElementsEventListener::ProcessEvent(Rml::Event& event) {
    if (value == "submitNewGame") {
        unsigned width, height, win_condition, players_n;
        width = stoi(engine->new_game_doc->GetElementById("width")->GetAttribute("value", Rml::String("1")));
        height = stoi(engine->new_game_doc->GetElementById("height")->GetAttribute("value", Rml::String("1")));
        win_condition = stoi(engine->new_game_doc->GetElementById("win-condition")->GetAttribute("value", Rml::String("1")));
        players_n = stoi(engine->new_game_doc->GetElementById("number-of-players")->GetAttribute("value", Rml::String("1")));

        engine->createOfflineGame(width, height, win_condition, players_n);

        engine->new_game_doc->Hide();
        engine->active_game->Show();
    } else if (value == "saveSettings") {
        for (int i = 1; i <= 4; i++) {
            unsigned r = stoi(engine->settings_doc->GetElementById("player-" + std::to_string(i) + "-r")->GetAttribute("value", Rml::String("1"))),
                     g = stoi(engine->settings_doc->GetElementById("player-" + std::to_string(i) + "-g")->GetAttribute("value", Rml::String("1"))),
                     b = stoi(engine->settings_doc->GetElementById("player-" + std::to_string(i) + "-b")->GetAttribute("value", Rml::String("1")));

            SDL_SetTextureColorMod(engine->t_player[i - 1], r, g, b);
        }
        engine->settings_doc->Hide();
        engine->menu_doc->Show();
    } else if (value == "play") {
        engine->menu_doc->Hide();
        engine->new_game_doc->Show();
    } else if (value == "settings") {
        engine->menu_doc->Hide();
        engine->settings_doc->Show();
    } else if (value == "exit") {
        engine->running = false;
    } else if (value == "help") {
        SDL_OpenURL("https://en.wikipedia.org/wiki/Ultimate_tic-tac-toe");
    } else if (value == "github") {
        SDL_OpenURL("https://github.com/Kirkezz/rttt");
    } else if (value == "donate") {
        SDL_OpenURL("https://github.com/Kirkezz/rttt");
    } else if (value == "backToMenu") {
        engine->new_game_doc->Hide();
        engine->active_game->Hide();
        engine->settings_doc->Hide();
        engine->menu_doc->Show();
    } else if (element_self->IsClassSet("form_field")) {
        engine->new_game_doc->GetElementById(value + "-value")->SetInnerRML(element_self->GetAttribute("value", Rml::String("")));
    }
}

Rml::EventListener* EventListener::InstanceEventListener(const Rml::String& value, Rml::Element* element) {
    Rml::EventListener* listener = new ElementsEventListener(engine, value, element);
    return listener;
}
