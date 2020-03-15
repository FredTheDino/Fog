#include "fog.h"
#include <vector>

#define MAX_TEXT_LENGTH 16
struct EditableSprite {
    std::vector<Vec2> points;
    AssetID sheet;
    char name[MAX_TEXT_LENGTH];
};

struct EditorState {
    u32 current_sprite;
    std::vector<EditableSprite> sprites;

    Vec2 cursor;
    f32 orig_worst_best_distance = 0.2;
    // Calculated after zoom.
    f32 worst_best_distance;
    f32 snapping_scale = 1.0;
    f32 snapping_scale_step = 1.0 / 4.0;

} global_editor;


enum class In {
    EDIT_NONE,
    EDIT_PLACE,
    EDIT_SELECT,
    EDIT_REMOVE,
    EDIT_RENAME,
    EDIT_NEXT_SPRITE,
    EDIT_PREV_SPRITE,
    EDIT_NEXT_SPRITE_SHEET,
    EDIT_PREV_SPRITE_SHEET,
    EDIT_SNAP_SMALLER,
    EDIT_SNAP_LARGER,
    EDIT_SAVE,
    EDIT_ADD_SPRITE,
    EDIT_MOVE_RIGHT_LEFT,
    EDIT_MOVE_UP_DOWN,
    EDIT_ZOOM_IN_OUT,

    TWEAK_STEP,
    TWEAK_SMOOTH,

    EDIT_NUM_BINDINGS,
};
