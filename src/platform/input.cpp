#include <algorithm>

namespace Input {

    static Binding *
    find_binding(Mapping *mapping, InputCode code) {
        auto binding = std::lower_bound(
                mapping->bindings + 0,
                mapping->bindings + mapping->used_bindings,
                code,
                [](const Binding a, const InputCode b) {
                    return a.code < b;
                });

        if (binding == (mapping->bindings + mapping->used_bindings))
            return nullptr;
        return binding;
    }

    static void insert(Mapping *mapping, Binding binding) {
        ASSERT(mapping->used_bindings < NUM_TOTAL_BINDINGS);

        Binding *it = mapping->bindings + mapping->used_bindings;
        while (mapping->bindings <= it && binding.code < it->code) {
            *(it + 1) = *it;
            it--;
        }
        *(it) = binding;
        ++mapping->used_bindings;
    }

    static bool
    add(Mapping *mapping, InputCode code, Player player, Name name) {
        Binding binding = {code, player, name, 0};

        // Check if there is a free binding.
        ASSERT(is_valid_player(player));
        auto buttons = mapping->buttons[toID(player)];
        u32 index = binding.index();
        bool valid = false;
        for (u32 i = 0; i < NUM_ALTERNATIVE_BINDINGS; i++) {
            if (buttons[index + i].is_used())
                continue;
            binding.binding_id = i;
            buttons[index + i].reset(name);
            valid = true;
            break;
        }

        // We failed to find a free spot, fail.
        CHECK(valid);
        if (!valid) return false;

        insert(mapping, binding);
        return true;
    }
    
    static bool
    activate(Mapping *mapping, InputCode code, f32 value) {
        Binding *binding = find_binding(mapping, code);
        if (!binding) return false;
        u32 index = binding->index();
        ASSERT(0 <= index && index < NUM_BINDINGS_PER_CONTROLLER);
        u32 player = binding->playerID();
        ASSERT(0 <= (u32) player && player < (u32) Player::NUM);
        mapping->buttons[player][index].set(value);
        return true;
    }

    static f32
    value(Mapping *mapping, Player player, Name name) {
        Binding binding = {0, player, name, 0};
        u32 num_down = 0;
        f32 value = 0;
        for (u32 i = 0; i < NUM_ALTERNATIVE_BINDINGS; i++) {
            auto button = mapping->buttons[binding.playerID()][binding.index()];
            if (button.is_down() && button.is_used()) {
                ++num_down;
                value += button.value;
            }
        }
        if (num_down)
            return value / num_down;
        return 0;
    }
};
