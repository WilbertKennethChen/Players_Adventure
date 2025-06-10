//
// Created by swift on 6/8/2025.
//

#ifndef KEYBINDING_H
#define KEYBINDING_H

#include <unordered_map>
#include <string>
#include <allegro5/allegro.h>

class KeyBindingManager {
public:
    static KeyBindingManager& GetInstance();

    int GetKeyForAction(const std::string& action) const;
    void SetKeyForAction(const std::string& action, int key);
    const std::string& GetActionForKey(int key) const;
    void ResetToDefaults();

    // Delete copy constructor and assignment operator
    KeyBindingManager(const KeyBindingManager&) = delete;
    KeyBindingManager& operator=(const KeyBindingManager&) = delete;

private:
    KeyBindingManager(); // Private constructor for singleton
    std::unordered_map<std::string, int> keyBindings;
    std::unordered_map<int, std::string> reverseKeyBindings;
};

#endif //KEYBINDING_H
