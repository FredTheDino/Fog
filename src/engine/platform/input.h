void (*window_callback)(int, int) = nullptr;

// TODO(ed): Shift and key combinations? Should those be handled?

//*
// <p>
// Player is an enum bit-field, with enums for P1, P2, P3 and P4. There
// are also pseudo-players like "ANY" player, which is all players and "NONE"
// which doesn't match any player.
// More players can of course be added but requires some knowledge of the
// engine. These magic-constans are feed into the input functions and
// can trivially be used to identify player entities.
// </p>

enum class Player {
    NONE = 0b0000,

    P1 = 0b0001,
    P2 = 0b0010,
    P3 = 0b0100,
    P4 = 0b1000,

    // Max number of players.
    NUM = 4,

    ANY = 0b1111,
};

bool is_valid_player(Player p) {
    return (p == Player::P1 || p == Player::P2 || p == Player::P3 ||
            p == Player::P4);
}

bool is(Player p, Player filter) { return (bool)((u8)p & (u8)filter); }

u32 toID(Player p) {
    switch (p) {
        case (Player::P1):
            return 0;
        case (Player::P2):
            return 1;
        case (Player::P3):
            return 2;
        case (Player::P4):
            return 3;
        default:
            UNREACHABLE;
    }
    // Safe guard return.
    return 0;
}

namespace Input {

//*
// To add a new "key" or "name" to the input system, an enum in the engine has
// to be updated. This enum is called "Input::Name" an lives in
// "/src/engine/platform/input.h". Adding a new name should be done before
// the "COUNT" element.

enum class Name {
    NONE = 0,

    LEFT,
    RIGHT,
    UP,
    DOWN,
    QUIT,

    DEBUG_PERF,

    COUNT, // Don't write anything after this.
};

enum class ButtonState {
    RELEASED = 0b10,
    UP = 0b00,
    PRESSED = 0b11,
    DOWN = 0b01,

    TRIGGERED = 0b10,
};

static ButtonState clear_frame_flag(ButtonState state) {
    return (ButtonState)((u8)state & 0b01);
}

static ButtonState generate_from_down(bool down) {
    return down ? ButtonState::PRESSED : ButtonState::DOWN;
}

constexpr u32 NUM_ALTERNATIVE_BINDINGS = 4;
constexpr u32 NUM_BINDINGS_PER_CONTROLLER =
    (u32)Name::COUNT * NUM_ALTERNATIVE_BINDINGS;
constexpr u32 NUM_TOTAL_BINDINGS =
    (u32)Player::NUM * NUM_BINDINGS_PER_CONTROLLER;

typedef u32 InputCode;
struct Binding {
    InputCode code;
    Player player;
    Name name;
    u8 binding_id;

    bool operator==(InputCode &other) const {
        return name != Name::NONE && code == other;
    }

    bool operator<(InputCode &other) const {
        return name != Name::NONE && code < other;
    }

    bool operator>(InputCode &other) const {
        return name != Name::NONE && code > other;
    }

    u32 index() const {
        ASSERT(name != Name::NONE, "NONE is not a valid name");
        return (u32)name * NUM_ALTERNATIVE_BINDINGS + binding_id;
    }

    u32 playerID() const {
        ASSERT(name != Name::NONE, "NONE is not a valid name");
        return toID(player);
    }
};

struct Mapping {
    // A list of all bindings
    u32 used_bindings;
    Binding bindings[NUM_TOTAL_BINDINGS];

    struct VirtualButton {
        Name name;
        ButtonState state;
        f32 value;

        void set(f32 v) {
            value = v;
            state = v ? ButtonState::PRESSED : ButtonState::RELEASED;
        }

        void reset(Name name) { *this = {name}; }
        bool is_down() { return (u32)state & (u32)ButtonState::DOWN; }
        bool is_used() { return name != Name::NONE; }
    };

    const VirtualButton get(Binding binding) const {
        return buttons[binding.playerID()][binding.index()];
    }

    // All the states for each button.
    VirtualButton buttons[(u32)Player::NUM][NUM_BINDINGS_PER_CONTROLLER];

    struct VirtualMouse {
        ButtonState state[3];
        s32 x, y;
        s32 move_x, move_y;

        //TODO(er): Add moved
    };

    VirtualMouse mouse;
} global_mapping;

struct InputEvent {
    InputCode code;
    bool pressed;
    f32 value;
};

//*
// Register a new mapping to the input system. <br>
// <ul>
//    <li>mapping - The input map, a global one is provided by the engine as
//    "mapping".</li>
//    <li>code - The keycode, K(a) generates the keycode for pressing "a".</li>
//    <li>player - The player that has this binding, can be P1, P2, P3,
//    P4.</li>
// </ul>
static bool add(InputCode code, Player player, Name name);

//*
// Returns true if the input button, stick or key was pressed or released this frame.
static bool triggered(Player player, Name name);

//*
// Returns true if the input button, stick or key was pressed this frame.
static bool pressed(Player player, Name name);

//*
// Returns true if the input button, stick or key was released this frame.
static bool released(Player player, Name name);

//*
// Returns true if the input button, stick or key is held down.
static bool down(Player player, Name name);

//*
// Returns the value of the input, useful for analog input.
static f32 value(Player player, Name name);

//*
// Returns the screen coordinates in pixels for the mouse position.
static Vec2 mouse_position();

//*
// Returns the movement of the mouse this frame, in pixels.
static Vec2 mouse_move();

//*
// Returns true if the mouse button was pressed or released this frame.
static bool mouse_triggered(u8 button);

//*
// Returns true if the mouse button was pressed this frame.
static bool mouse_pressed(u8 button);

//*
// Returns true if the mouse button was released this frame.
static bool mouse_released(u8 button);

//*
// Returns true if the mouse button is held down.
static bool mouse_down(u8 button);

#ifdef _EXAMPLE_

////
// <h2>Adding and using input mappings</h2>
// To add an input mapping, first add the name for the input in the struct
// "Input::Name" in "/src/engine/platform/input.h". Then the rest is simple,
// just remember to call it in the initialization function for the game.
Input::add(K(a), Input::Player::P1, Input::Name::MY_INPUT);
Input::add(K(d), Input::Player::P2, Input::Name::MY_INPUT);
// This call adds a new mapping, mapping the key "a" on the keyboard
// to player 1's "MY_INPUT" and mapping "d" on the keyboard to
// player 2's "MY_INPUT". To see if the input was activated this
// frame, simply call:
if (Input::pressed(Input::Player::P1, Input::Name::MY_INPUT)) {
    do_stuff();
}
if (Input::pressed(Input::Player::ANY, Input::Name::MY_INPUT)) {
    do_more_stuff();
}
// "do_stuff" will be called if the "a" key was pressed THIS FRAME,
// we only check for the input for player 1. And "do_more_stuff" will
// be called if either "a" or "b" were pressed, since we removed the
// requirement of what player pressed it.  Assuming the previous section
// where the mapping was added was called off course.
//
// It might be a good idea to "using namespace Input;" if you are going
// to do a lot of input calls, since it quite quickly becomes very noise.



#endif

};  // namespace Input
