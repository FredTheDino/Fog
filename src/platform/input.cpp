// TODO(ed): You don't need this really...
#include <algorithm>

namespace Input {

    static Binding *
    find_binding(Mapping *mapping, InputCode code) {
        // TODO(ed): The place where <algorithm> is used.
        ASSERT(code != 0);
        auto binding = std::lower_bound(
                mapping->bindings + 0,
                mapping->bindings + mapping->used_bindings,
                code,
                [](const Binding a, const InputCode b) {
                    return a.code < b;
                });

        if (binding->code != code)
            return nullptr;
        return binding;
    }

    static void insert(Mapping *mapping, Binding binding) {
        ASSERT(mapping->used_bindings < NUM_TOTAL_BINDINGS);

        Binding *it = mapping->bindings + mapping->used_bindings;
        while (mapping->bindings != it && binding.code < (it - 1)->code) {
            *it = *(it - 1);
            it--;
        }
        *it = binding;
        mapping->used_bindings += 1;
    }

    static bool
    add(Mapping *mapping, InputCode code, Player player, Name name) {
        ASSERT(code != 0);
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

    // TODO(ed): This can be acchived with a time stamp and 2 bit
    // variables. But if you check more than each button every frame,
    // this saves performance.
    static void
    frame(Mapping *mapping) {
        for (u32 player = 0; player < (u32) Player::NUM; player++) {
            for (u32 button_id = 0; button_id < NUM_BINDINGS_PER_CONTROLLER; button_id++) {
                auto *button = mapping->buttons[player] + button_id;
                button->state = clear_frame_flag(button->state);
            }
        }
    }

    static bool
    activate(Mapping *mapping, InputCode code, f32 value) {
        Binding *binding = find_binding(mapping, code);
        if (!binding)
            return false;
        u32 index = binding->index();
        ASSERT(0 <= index && index < NUM_BINDINGS_PER_CONTROLLER);
        u32 player = binding->playerID();
        ASSERT(0 <= (u32) player && player < (u32) Player::NUM);
        mapping->buttons[player][index].set(value);
        return true;
    }

    // TODO(ed): down, released, pressed, triggered

#define BEGIN_BINDINGS_BLOCK \
        for (u32 p = 0; p < (u32) Player::NUM; p++) {\
            Player p_mask = Player(1 << p);\
            if (!is(player, p_mask)) continue;\
            Binding binding = {0, p_mask, name, 0};\
            for (u32 i = 0; i < NUM_ALTERNATIVE_BINDINGS; i++) {\
                auto button = mapping->get(binding);\
                if (!button.is_used()) continue;
#define END_BINDINGS_BLOCK \
            }\
        }

    static bool
    triggered(Mapping *mapping, Player player, Name name) {
        BEGIN_BINDINGS_BLOCK {
            if ((u32) button.state & (u32) ButtonState::TRIGGERED)
                return true;
        } END_BINDINGS_BLOCK
        return false;
    }

    static bool
    pressed(Mapping *mapping, Player player, Name name) {
        BEGIN_BINDINGS_BLOCK {
            if (button.state == ButtonState::PRESSED) return true;
        } END_BINDINGS_BLOCK
        return false;
    }

    static bool
    released(Mapping *mapping, Player player, Name name) {
        BEGIN_BINDINGS_BLOCK {
            if (button.state == ButtonState::RELEASED) return true;
        } END_BINDINGS_BLOCK
        return false;
    }

    static bool
    down(Mapping *mapping, Player player, Name name) {
        BEGIN_BINDINGS_BLOCK {
            if (button.is_down()) return true;
        } END_BINDINGS_BLOCK
        return false;
    }

    static f32
    value(Mapping *mapping, Player player, Name name) {
        u32 num_down = 0;
        f32 value = 0;
        BEGIN_BINDINGS_BLOCK {
            if (!button.is_down()) continue;
            num_down++;
            value += button.value;
        } END_BINDINGS_BLOCK
        if (num_down) return value / num_down;
        return 0;
    }

#undef BEGIN_BINDINGS_BLOCK
#undef END_BINDINGS_BLOCK
};
