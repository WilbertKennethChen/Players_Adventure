//
// Created by swift on 6/8/2025.
//
#include "keybinding.h"

KeyBindingManager& KeyBindingManager::GetInstance() {
    static KeyBindingManager instance;
    return instance;
}

KeyBindingManager::KeyBindingManager() {
    ResetToDefaults();
}

int KeyBindingManager::GetKeyForAction(const std::string& action) const {
    auto it = keyBindings.find(action);
    if (it != keyBindings.end()) {
        return it->second;
    }
    return -1; // or some default key
}

void KeyBindingManager::SetKeyForAction(const std::string& action, int key) {
    // Remove old mapping if it exists
    auto oldKey = keyBindings[action];
    reverseKeyBindings.erase(oldKey);

    // Set new mapping
    keyBindings[action] = key;
    reverseKeyBindings[key] = action;
}

const std::string& KeyBindingManager::GetActionForKey(int key) const {
    static std::string empty;
    auto it = reverseKeyBindings.find(key);
    if (it != reverseKeyBindings.end()) {
        return it->second;
    }
    return empty;
}

void KeyBindingManager::ResetToDefaults() {
    keyBindings = {
        {"up", ALLEGRO_KEY_W},
        {"down", ALLEGRO_KEY_S},
        {"left", ALLEGRO_KEY_A},
        {"right", ALLEGRO_KEY_D},
        {"note_lane_0", ALLEGRO_KEY_J},
        {"note_lane_1", ALLEGRO_KEY_K},
        {"note_lane_2", ALLEGRO_KEY_L}
    };

    reverseKeyBindings.clear();
    for (const auto& pair : keyBindings) {
        reverseKeyBindings[pair.second] = pair.first;
    }
}