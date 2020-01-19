void (*window_callback)(int, int) = nullptr;

// TODO(ed): Shift and key combinations? Should those be handled?

///# Input
// The input system allows players and keys to be mapped to a name.
// The name is essentially an integer and should be kept in the enum
// Input::Name.

///* Player type
// <p>
// Player is an enum bit-field, with enums for P1, P2, P3 and P4. There
// are also pseudo-players like "ANY" player, which is all players and "NONE"
// which doesn't match any player.
// More players can of course be added but requires some knowledge of the
// engine. These magic-constans are feed into the input functions and
// can trivially be used to identify player entities.
// </p>
// <p>
// Possible values are:
// <ul>
//    <li>P1 - The first player</li>
//    <li>P2 - The second player</li>
//    <li>P3 - The third player</li>
//    <li>P4 - The fourth player</li>
//    <li>ANY - Any of the players (Does not currently work when assigning input)</li>
// </ul>
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

enum class ButtonState {
    RELEASED = 0b10,
    UP = 0b00,
    PRESSED = 0b11,
    DOWN = 0b01,

    TRIGGERED = 0b10,
};

ButtonState clear_frame_flag(ButtonState state) {
    return (ButtonState)((u8)state & 0b01);
}

ButtonState generate_from_down(bool down) {
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
        u32 depth;

        //TODO(er): Add moved
    };

    bool text_input;

    static const u32 TEXT_LENGTH = 32;
    u32 text_length;
    char text[TEXT_LENGTH];

    VirtualMouse mouse;
} global_mapping;

struct InputEvent {
    InputCode code;
    bool pressed;
    f32 value;
};

///*
// Switches the input to use text input, this disables
// the state of the input and makes starts listening to
// text input.
void start_text_input();

///*
// Stop listening for the text input re-enable the input
// to work like it normally does.
void stop_text_input();

// Types the text to the buffer of text held to the end of the frame.
void type_text(const char *string);

// TODO(ed): Make this take in a cursor aswell... It will be great.
///*
// Applies the edits made to the string that
// is supplied. "max_length" is the maximum allocated
// length of the string. Returns true if the length
// is changed.
bool edit_string(char *text, u32 max_length);

///* add
// Register a new mapping to the input system.<br>
// code, the keycode, should be recived from calling K(DESIRED_KEY), DESIRED_KEY
// should be lowercase letters for normal keys and UPPERCASE for special keys.
// Player, the player that has this binding, can be P1, P2, P3, P4.
bool add(InputCode code, Name name, Player player=Player::P1);

///*
// Returns true if the input button, stick or key was pressed or released this frame.
bool triggered(Name name, Player player=Player::ANY);

///*
// Returns true if the input button, stick or key was pressed this frame.
bool pressed(Name name, Player player=Player::ANY);

///*
// Returns true if the input button, stick or key was released this frame.
bool released(Name name, Player player=Player::ANY);

///*
// Returns true if the input button, stick or key is held down.
bool down(Name name, Player player=Player::ANY);

///*
// Returns the value of the input, useful for analog input.
f32 value(Name name, Player player=Player::ANY);

///*
// Ignores if the input is enabled or not and returns if the button
// is pressed. Do not use this as a stand in for normal input.
bool super_pressed(Name name, Player player=Player::ANY);

///*
// Returns the screen coordinates in pixels for the mouse position.
Vec2 mouse_position();

///*
// Returns the movement of the mouse this frame, in pixels.
Vec2 mouse_move();

///*
// Converts screen coordinates to world coordinates.
Vec2 screen_to_world(Vec2 p);

///*
// The position of the mouse in world coordinated,
// taken into account the current camera transform.
Vec2 world_mouse_position();

///*
// The position of the mouse in the same scale as the world
// coordinates, but relative to the camera position.
Vec2 normalised_mouse_position();

///*
// The movement of the mouse in world coordinated,
// taken into account the current camera transform.
Vec2 world_mouse_move();

///*
// Returns the depth of the mouse press, this far. Should increase
// when interacting with gui elements.
//
// Usefull when things can be placed "behind" other things.
u32 mouse_depth();

///*
// Increases the mouse depth.
void eat_mouse();

///*
// Returns true if the mouse button was pressed or released this frame.
bool mouse_triggered(u8 button);

///*
// Returns true if the mouse button was pressed this frame.
bool mouse_pressed(u8 button);

///*
// Returns true if the mouse button was released this frame.
bool mouse_released(u8 button);

///*
// Returns true if the mouse button is held down.
bool mouse_down(u8 button);

};  // namespace Input
