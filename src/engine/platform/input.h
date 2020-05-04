void (*window_callback)(int, int) = nullptr;

// TODO(ed): Shift and key combinations? Should those be handled?

namespace Input {

///# Input
// The input system allows players and keys to be mapped to a name.
// The name is essentially an integer and should be kept in the enum
// Input::Name.

FOG_EXPORT_STRUCT
typedef enum {
    NO_ONE = 0b0000,

    P1 = 0b0001,
    P2 = 0b0010,
    P3 = 0b0100,
    P4 = 0b1000,

    // Max number of players.
    NUM = 4,

    ANY = 0b1111,
} Player;

b8 is_valid_player(Player p) {
    return (p == Player::P1 || p == Player::P2 || p == Player::P3 ||
            p == Player::P4);
}

b8 is(Player p, Player filter) { return (b8)((u8)p & (u8)filter); }

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

//// Player type
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

ButtonState generate_from_down(b8 down) {
    return down ? ButtonState::PRESSED : ButtonState::DOWN;
}

FOG_EXPORT
typedef u32 InputCode;

FOG_EXPORT
typedef u32 Name;

static Name NO_INPUT = 0;

// Change this to allow more bindings.
const u32 NUM_ALTERNATIVE_BINDINGS = 4;

struct Binding {
    InputCode code;
    Player player;
    Name name;
    u8 binding_id;
    s32 inverted;

    b8 operator==(InputCode &other) const {
        return name != NO_INPUT && code == other;
    }

    b8 operator<(InputCode &other) const {
        return name != NO_INPUT && code < other;
    }

    b8 operator>(InputCode &other) const {
        return name != NO_INPUT && code > other;
    }

    u32 index() const {
        ASSERT(name != NO_INPUT, "NONE is not a valid name");
        return (u32)name * NUM_ALTERNATIVE_BINDINGS + binding_id;
    }

    u32 playerID() const {
        ASSERT(name != NO_INPUT, "NONE is not a valid name");
        return toID(player);
    }
};

struct Mapping {
    // A list of all bindings
    Util::MemoryArena *arena;
    u32 num_bindings_per_controller;
    u32 num_total_bindings;

    u32 used_bindings;
    Binding *bindings;

    struct VirtualButton {
        Name name;
        f32 modifier;
        ButtonState state;
        f32 value;

        void set(f32 v) {
            value = v;
            state = v ? ButtonState::PRESSED : ButtonState::RELEASED;
        }

        void reset(Name name, f32 modifier) { *this = {name, modifier}; }
        b8 is_down() { return (u32)state & (u32)ButtonState::DOWN; }
        b8 is_used() { return name != NO_INPUT; }
    };

    const VirtualButton get(Binding binding) const {
        return buttons[binding.playerID()][binding.index()];
    }

    // All the states for each button.
    VirtualButton *buttons[(u32)Player::NUM];

    struct VirtualMouse {
        ButtonState state[3];
        s32 x, y;
        s32 move_x, move_y;
        s32 wheel_x, wheel_y;
        u32 depth;

        //TODO(er): Add moved
    };

    b8 text_input;

    static const u32 TEXT_LENGTH = 32;
    u32 text_length;
    char text[TEXT_LENGTH];

    VirtualMouse mouse;

    Name next_name = 1;

    b8 using_controller;
    b8 disallow_adding_of_mappings;
} global_mapping = {};

struct InputEvent {
    InputCode code;
    b8 pressed;
    f32 value;
};

///*
// Gives back a new unique name that can be used to
// react to inputs. If multiple names are requested,
// the first of the names are returned, and the subsequent
// numbers are registered names. The names allways start
// at 1 for the first name.
Name request_name(u32 num=1);

///*
// Returns the currently prefered input method of the
// user, which we assume is the IO-unit a action was
// done to latest. So if the last input was for a controller,
// this is true, otherwise it's false.
b8 using_controller();

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
b8 edit_string(char *text, u32 max_length);

///*
// Register a new mapping to the input system.
// The value returned from value(...) is multiplied with modifier.
// Player, the player that has this binding, can be P1, P2, P3, P4.
b8 add_mod(InputCode code, Name name, Player player, f32 modifier);

///*
// Register a new mapping to the input system.
// Player, the player that has this binding, can be P1, P2, P3, P4.
b8 add(InputCode code, Name name, Player player);

b8 add(InputCode code, Name name, Player player=P1) {
    return add_mod(code, name, player, 1.0f);
}

///*
// Returns true if the input button, stick or key was pressed or released this frame.
b8 triggered(Name name, Player player=ANY);

///*
// Returns true if the input button, stick or key was pressed this frame.
b8 pressed(Name name, Player player=ANY);

///*
// Returns true if the input button, stick or key was released this frame.
b8 released(Name name, Player player=ANY);

///*
// Returns true if the input button, stick or key is held down.
b8 down(Name name, Player player=ANY);

///*
// Returns the value of the input, useful for analog input.
f32 value(Name name, Player player=ANY);

///*
// Ignores if the input is enabled or not and returns if the button
// is pressed. Do not use this as a stand in for normal input.
b8 super_pressed(Name name, Player player=ANY);

///*
// Returns the screen coordinates in pixels for the mouse position.
Vec2 mouse_position();

///*
// Returns the movement of the mouse this frame, in pixels.
Vec2 mouse_move();

///*
// Converts screen coordinates to world coordinates.
Vec2 screen_to_world(Vec2 p, u32 camera_id=0);

///*
// The position of the mouse in world coordinated,
// taken into account the current camera transform.
Vec2 world_mouse_position(u32 camera_id=0);

///*
// The position of the mouse in the same scale as the world
// coordinates, but relative to the camera position.
Vec2 normalized_mouse_position();

///*
// The movement of the mouse in world coordinated,
// taken into account the current camera transform.
Vec2 world_mouse_move(u32 camera_id = 0);

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
// Returns the direction the mouse wheel was scrolled this
// frame.
Vec2 mouse_scroll();

///*
// Returns true if the mouse button was pressed or released this frame.
b8 mouse_triggered(u8 button);

///*
// Returns true if the mouse button was pressed this frame.
b8 mouse_pressed(u8 button);

///*
// Returns true if the mouse button was released this frame.
b8 mouse_released(u8 button);

///*
// Returns true if the mouse button is held down.
b8 mouse_down(u8 button);

};  // namespace Input
