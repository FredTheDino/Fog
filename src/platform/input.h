// TODO(ed): Find better home.
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
    return (p == Player::P1 ||
            p == Player::P2 ||
            p == Player::P3 ||
            p == Player::P4);
}

bool is(Player p, Player filter) {
    return (bool) ((u8) p & (u8) filter);
}

u32 toID(Player p) {
    switch(p) {
        case(Player::P1):
            return 0;
        case(Player::P2):
            return 1;
        case(Player::P3):
            return 2;
        case(Player::P4):
            return 3;
        default:
            UNREACHABLE;
    }
    // Safe guard return.
    return 0;
}

namespace Input {

    enum class Name {
        NONE = 0,

        LEFT,
        RIGHT,
        UP,
        DOWN,
        QUIT,

        COUNT,
    };

    enum class ButtonState {
        RELEASED = 0b10,
        UP       = 0b00,
        PRESSED  = 0b11,
        DOWN     = 0b01,

        TRIGGERED = 0b10,
    };

    static ButtonState
    clear_frame_flag(ButtonState state) {
        return (ButtonState) ((u8) state & 0b01);
    }

    static ButtonState
    generate_from_down(bool down) {
        return down ? ButtonState::PRESSED : ButtonState::DOWN;
    }

    constexpr u32 NUM_ALTERNATIVE_BINDINGS = 4;
    constexpr u32 NUM_BINDINGS_PER_CONTROLLER = (u32) Name::COUNT * NUM_ALTERNATIVE_BINDINGS;
    constexpr u32 NUM_TOTAL_BINDINGS = (u32) Player::NUM * NUM_BINDINGS_PER_CONTROLLER;

    typedef u32 InputCode;
    struct Binding {
        InputCode code;
        Player player;
        Name name;
        u8 binding_id;

        bool operator== (InputCode &other) const {
            return name != Name::NONE && code == other;
        }

        bool operator< (InputCode &other) const {
            return name != Name::NONE && code < other;
        }

        bool operator> (InputCode &other) const {
            return name != Name::NONE && code > other;
        }

        u32 index() const {
            ASSERT(name != Name::NONE);
            return (u32) name * NUM_ALTERNATIVE_BINDINGS + binding_id;
        }

        u32 playerID() const {
            ASSERT(name != Name::NONE);
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
            bool is_down() { return (u32) state & (u32) ButtonState::DOWN; }
            bool is_used() { return name != Name::NONE; }
        };

        const VirtualButton
        get(Binding binding) const { return buttons[binding.playerID()]
                                                   [binding.index()];   }

        // All the states for each button.
        VirtualButton buttons[(u32) Player::NUM][NUM_BINDINGS_PER_CONTROLLER];
    };

    struct InputEvent {
        InputCode code;
        bool pressed;
        f32 value;
    };
};
