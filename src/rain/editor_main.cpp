#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <cctype>
#include "editor_main.h"
#include "stdio.h"

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
            break;
    }
    // Safe guard return.
    return 0;
}

#define MAX(a, b) ((a) > (b)) ? (a): (b)
#define MIN(a, b) ((a) < (b)) ? (a): (b)
#define CLAMP(l, h, c) MAX(l, MIN(h, c))
#define ABS(a) ((a) > 0) ? (a) : -(a)
#define SIGN(a) ((a) > 0) ? 1 : -1


#define K(key) (fog_key_to_input_code((SDLK_##key)))
#define A(axis, player) (fog_axis_to_input_code((SDL_CONTROLLER_AXIS_##axis), toID(player)))
#define B(button, player) (fog_button_to_input_code((SDL_CONTROLLER_BUTTON_##button), toID(player)))

void read_with_default(const char *prompt, const char *def, char *val, u32 length) {
    s32 input_length;
    do {
        printf("%s", prompt);
        if (def)
            printf(" [%s]", def);
        printf(": ");

        fgets(val, length, stdin);
        input_length = strlen(val);
        printf("%d\n", input_length);
        if (val[MAX(input_length - 1, 0)] == '\n') {
            break;
        }

        printf("You passed in a too long string, max length is %d.\n", length);
        do {
            fgets(val, length, stdin);
            input_length = strlen(val);
        } while (val[MAX(input_length - 1, 0)] != '\n');
    } while (true);

    if (input_length == 2)
        memcpy((void *) def, (void *) val, strlen(def));
    else
        val[input_length - 2] = '\0';

}

void write_sprite_to_file(EditableSprite sprite, FILE *file) {
    fprintf(file, "%s:#%llu:%d:", sprite.name,
            fog_asset_hash(sprite.sheet), sprite.points.size());

    Vec2 center = fog_V2(0, 0);
    for (u32 i = 0; i < sprite.points.size(); i++) {
        center += sprite.points[i];
    }
    center /= sprite.points.size();
    for (u32 i = 0; i < sprite.points.size(); i++) {
        Vec2 p = sprite.points[i];
        fprintf(file, "(%f %f %f %f)",
            p.x - center.x, p.y - center.y,
            p.x, p.y);
    }
    fprintf(file, "\n");
}

AssetID find_next_sheet(AssetID start=0, int dir=1) {
    for (s32 i = 1; i < (s32) fog_asset_num(); i++) {
        AssetID curr = (start + i * dir + fog_asset_num()) % fog_asset_num();
        if (fog_asset_is(curr, Type::TEXTURE))
            return curr;
    }
    return -1;
}

// Commandline arguments
// rain [OPTION] .. [FILE]
//
// Options:
// -s, --sprite : Sprite editor <- Default
// -l, --level : Level editor
// -?, -h, --help : Show this message
//
// File:
// A file path which is then loaded

// NOTE(ed): The editors should be swapable between, so
// don't have unique setup for some and not for the other.
//
// It isn't currently though.. Not sure if we should have an
// editor at all... Maybe just load JSON data, or something?
const char *path = nullptr;

void create_new_sprite() {
    EditableSprite new_sprite = {
        std::vector<Vec2>(10),
        find_next_sheet(0, 1),
    };
    read_with_default("sprite name",
            nullptr,
            new_sprite.name,
            MAX_TEXT_LENGTH);

    global_editor.current_sprite = global_editor.sprites.size();
    global_editor.sprites.push_back(new_sprite);
}

void setup(int argc, char **argv) {
    // Commandline parsing, sets globalstate
    for (s32 i = 1; i < argc; i++) {
        char *val = argv[i];
        if (*val != '-') {
            if (path) {
                printf("Multiple files given replacing '%s' with '%s'.", path, argv[i]);
            }
            path = argv[i];
        } else {
            // Parse flags
            val++;
            if (*val == '-') val++;
            switch (*val) {
                default:
                    printf("Unkown argument '%s', ignoring.", argv[i]);
            };
        }
    }



    global_editor.sprites = std::vector<EditableSprite>(10);
    global_editor.cursor = fog_V2(0.5, 0.5);
    b8 no_path_passed = true;
    if (path != nullptr) {
        // NOTE(ed): Code bellow is copied from the asset loader. Might need to be a
        // unified function.
        FILE *sprite_file = fopen(path, "rb");
        while (sprite_file) {
            char *line = nullptr;
            size_t line_n = 0;
            getline(&line, &line_n, sprite_file);

            if (feof(sprite_file))
                break;

            char *ptr = line;
            u32 i = 0;
            char *name = ptr + i;
            for (; i < line_n; i++) if (ptr[i] == ':') { ptr[i] = '\0'; break; }
            char *res = ptr + i + 1;
            for (; i < line_n; i++) if (ptr[i] == ':') { ptr[i] = '\0'; break; }
            char *num = ptr + i + 1;
            for (; i < line_n; i++) if (ptr[i] == ':') { ptr[i] = '\0'; break; }
            char *points_str = ptr + i + 1;
            for (; i < line_n; i++) if (ptr[i] == ':') { ptr[i] = '\0'; break; }

            // TODO(ed): Parse points, and stuff
#define CHECK_FOR_ERROR(ptr)                                  \
            do {                                                      \
                if (ptr == endptr) {                                  \
                    fprintf(stderr, "Failed to parse number in sprite file '%s'", \
                            path);                      \
                    fclose(sprite_file);                              \
                    return;                                           \
                }                                                     \
            } while (false);

            char *endptr;
            u32 num_points = strtol(num, &endptr, 10);
            CHECK_FOR_ERROR(num);
            std::vector<Vec2> points = std::vector<Vec2>(num_points);
            for (u32 p = 0; p < num_points; p++) {
                char *endptr;
                Vec4 point;
                while (isspace(*points_str)) points_str++;
                points_str += 1; // (
                for (u32 coord = 0; coord < 4; coord++) {
                    point._[coord] = strtod(points_str, &endptr);
                    CHECK_FOR_ERROR(points_str);
                    points_str = endptr;
                }
                points_str += 1; // )
                points.push_back(fog_V2(point.z, point.w));
            }
#undef CHECK_FOR_ERROR

            u32 resource_hash = res[0] == '#' ? atoi(res + 1): fog_asset_asset_hash(res);
            // Reverse the hash
            for (u32 i = 0; i < fog_asset_num(); i++) {
                if (fog_asset_num() == resource_hash) {
                    resource_hash = i;
                }
            }

            EditableSprite sprite = {
                points,
                resource_hash,
            };
            memcpy(name, sprite.name, MIN(MAX_TEXT_LENGTH, strlen(name)));
            global_editor.sprites.push_back(sprite);
            no_path_passed = false;
            free(line);
        }
        if (sprite_file)
            fclose(sprite_file);
    } else {
        path = "default.spr";
    }

    if (no_path_passed) {
        create_new_sprite();

    }

    // Sprite editor
    fog_input_add(K(a), (u32) In::EDIT_PLACE, Player::P1);
    fog_input_add(K(s), (u32) In::EDIT_SELECT, Player::P1);
    fog_input_add(K(d), (u32) In::EDIT_REMOVE, Player::P1);
    fog_input_add(K(r), (u32) In::EDIT_RENAME, Player::P1);
    fog_input_add(K(u), (u32) In::EDIT_NEXT_SPRITE, Player::P1);
    fog_input_add(K(i), (u32) In::EDIT_PREV_SPRITE, Player::P1);
    fog_input_add(K(n), (u32) In::EDIT_NEXT_SPRITE_SHEET, Player::P1);
    fog_input_add(K(p), (u32) In::EDIT_PREV_SPRITE_SHEET, Player::P1);
    fog_input_add(K(j), (u32) In::EDIT_SNAP_SMALLER, Player::P1);
    fog_input_add(K(k), (u32) In::EDIT_SNAP_LARGER, Player::P1);
    fog_input_add(K(w), (u32) In::EDIT_SAVE, Player::P1);
    fog_input_add(K(c), (u32) In::EDIT_ADD_SPRITE, Player::P1);
    fog_input_add(K(r), (u32) In::EDIT_RENAME, Player::P1);

    fog_input_add(A(LEFTX, Player::P1), (u32) In::EDIT_MOVE_RIGHT_LEFT, Player::P1);
    fog_input_add(A(LEFTY, Player::P1), (u32) In::EDIT_MOVE_UP_DOWN, Player::P1);
    fog_input_add(A(RIGHTY, Player::P1), (u32) In::EDIT_ZOOM_IN_OUT, Player::P1);
    fog_input_add(B(B, Player::P1), (u32) In::EDIT_PLACE, Player::P1);
    fog_input_add(B(A, Player::P1), (u32) In::EDIT_SELECT, Player::P1);
    fog_input_add(B(Y, Player::P1), (u32) In::EDIT_REMOVE, Player::P1);
    // fog_input_add(B(LEFTSHOULDER, Player::P1), (u32) In::TWEAK_STEP, Player::P1);
    fog_input_add(B(DPAD_DOWN, Player::P1), (u32) In::EDIT_SNAP_SMALLER, Player::P1);
    fog_input_add(B(DPAD_UP, Player::P1), (u32) In::EDIT_SNAP_LARGER, Player::P1);
    fog_input_add(A(TRIGGERRIGHT, Player::P1), (u32) In::EDIT_NEXT_SPRITE, Player::P1);
    fog_input_add(A(TRIGGERLEFT, Player::P1), (u32) In::EDIT_PREV_SPRITE, Player::P1);
    fog_input_add(B(DPAD_RIGHT, Player::P1), (u32) In::EDIT_NEXT_SPRITE_SHEET, Player::P1);
    fog_input_add(B(DPAD_LEFT, Player::P1), (u32) In::EDIT_PREV_SPRITE_SHEET, Player::P1);
    fog_input_add(B(START, Player::P1), (u32) In::EDIT_SAVE, Player::P1);
    fog_input_add(B(BACK, Player::P1), (u32) In::EDIT_ADD_SPRITE, Player::P1);

    fog_renderer_fetch_camera(0)->position = fog_V2(0.5, 0.5);
}

void sprite_editor_update() {
    Vec2 point;
    // Input
    {
        // Rename?
        if (fog_input_pressed((u32) In::EDIT_RENAME, Player::ANY)) {
            read_with_default("sprite rename", nullptr,
                    global_editor.sprites[global_editor.current_sprite].name,
                    MAX_TEXT_LENGTH);
        }

        const u32 num_sprites = global_editor.sprites.size();
        if (fog_input_pressed((u32) In::EDIT_PREV_SPRITE, Player::ANY) && num_sprites) {
            global_editor.current_sprite -= 1;
            global_editor.current_sprite += num_sprites;
            global_editor.current_sprite %= num_sprites;
        }
        if (fog_input_pressed((u32) In::EDIT_NEXT_SPRITE, Player::ANY) && num_sprites) {
            global_editor.current_sprite += 1;
            global_editor.current_sprite += num_sprites;
            global_editor.current_sprite %= num_sprites;
        }

        f32 delta = MIN(fog_logic_delta(), 0.2);
        f32 move_x = fog_input_value((u32) In::EDIT_MOVE_RIGHT_LEFT, Player::ANY);
        f32 move_y = fog_input_value((u32) In::EDIT_MOVE_UP_DOWN, Player::ANY);

        f32 zoom;
        if (fog_input_using_controller())
            zoom = (1.0 + fog_input_value((u32) In::EDIT_ZOOM_IN_OUT, Player::ANY) * delta);
        else
            zoom = (1.0 + (fog_input_mouse_scroll().y * 5) * delta);
        fog_renderer_fetch_camera(0)->zoom *= zoom;
        f32 current_zoom = fog_renderer_fetch_camera(0)->zoom;
        const f32 speed = delta / current_zoom;
        Vec2 cursor;
        if (fog_input_using_controller())
            cursor = global_editor.cursor;
        else
            cursor = fog_input_world_mouse_position(0);
        global_editor.worst_best_distance = global_editor.orig_worst_best_distance / current_zoom;

        if (fog_input_pressed((u32) In::EDIT_SNAP_SMALLER, Player::ANY)) {
            global_editor.snapping_scale *= global_editor.snapping_scale_step;
        }
        if (fog_input_pressed((u32) In::EDIT_SNAP_LARGER, Player::ANY)) {
            global_editor.snapping_scale /= global_editor.snapping_scale_step;
        }
        global_editor.snapping_scale = CLAMP(1.0 / 512.0, 1.0, global_editor.snapping_scale);

        if (fog_input_down((u32) In::TWEAK_STEP, Player::ANY)) {
            const f32 snap = global_editor.snapping_scale;
            static float step_timer = 0.0;
            step_timer += delta;
            if (step_timer > 0.1) {
                if (ABS(move_x) > 0.3)
                    cursor.x += SIGN(move_x) * snap * 1.1;
                if (ABS(move_y) > 0.3)
                    cursor.y += SIGN(move_y) * snap * 1.1;
                step_timer = 0.0;
            }
            // fog_util_precise_snap(&cursor.x, snap, snap);
            // fog_util_precise_snap(&cursor.y, snap, snap);
        } else {
            cursor += fog_V2(move_x, move_y) * speed;
        }
        cursor = fog_V2(CLAMP(-0.1, 1.1, cursor.x), CLAMP(-0.1, 1.1, cursor.y));
        global_editor.cursor = cursor;
        // Like a return value... but not...
        point = cursor;
    }

    if (fog_input_pressed((u32) In::EDIT_ADD_SPRITE, Player::ANY)) {
        create_new_sprite();
    }

    // Drawing logic
    {
        f32 current_zoom = fog_renderer_fetch_camera(0)->zoom;
        const f32 snap = global_editor.snapping_scale;
        const f32 line_width = 0.03 / current_zoom;
        const Vec4 dim_color = fog_V4(1, 0, 1, 0.05);
        const Vec4 vis_color = fog_V4(1, 0, 1, 0.2);
        Vec4 color = fog_input_down((u32) In::TWEAK_STEP, Player::ANY) ? vis_color : dim_color;
        for (u32 i = 0; i <= 1.0 / snap; i++) {
            Vec2 start, end;
            start = fog_V2(i * snap, 0.0);
            end = fog_V2(i * snap, 1.0);
            fog_renderer_push_line(5, start, end, color, line_width);
            start = fog_V2(0.0, i * snap);
            end = fog_V2(1.0, i * snap);
            fog_renderer_push_line(5, start, end, color, line_width);
        }
        const Vec4 cursor_color = fog_V4(0.0, 1.0, 0.0, 1.0);
        fog_renderer_push_point(10, point, cursor_color, 0.05 / current_zoom);
    }

    // Camera Movement
    {
        Camera *camera = fog_renderer_fetch_camera(0);
        f32 dist = fog_distance_v2(camera->position, point);
        f32 threshold = 0.5 / camera->zoom;
        if (dist > threshold) {
            f32 speed = 10 * (dist - threshold) * camera->zoom * fog_logic_delta();
            camera->position += (point - camera->position) * speed;
        }
    }

    // TODO(ed): Select sprite
    // TODO(ed): Allow multiple sprites to be editable at once
    // TODO(ed): Tweak values for things like, hue of the overlay
    // and such, so any sprite can be rendered underneath

    // Sprite select
    EditableSprite *sprite = &global_editor.sprites[global_editor.current_sprite];
    {
        AssetID next = sprite->sheet;
        if (fog_input_pressed((u32) In::EDIT_NEXT_SPRITE_SHEET, Player::ANY)) {
            next = find_next_sheet(next, 1);
        }
        if (fog_input_pressed((u32) In::EDIT_PREV_SPRITE_SHEET, Player::ANY)) {
            next = find_next_sheet(next, -1);
        }
        if (next != -1)
            sprite->sheet = next;
    }

    // Logic
    if (fog_input_down((u32) In::EDIT_SELECT, Player::ANY) || fog_input_mouse_down(0)) {
        s32 best_index = -1;
        f32 best_dist = global_editor.worst_best_distance;
        std::vector<Vec2> *points = &sprite->points;
        u32 num_points = points->size();
        for (u32 i = 0; i < num_points; i++) {
            f32 dist = fog_length_v2(point - (*points)[i]);
            if (dist < best_dist) {
                best_index = i;
                best_dist = dist;
            }
        }
        if (best_index >= 0) {
            (*points)[best_index] = point;
        }
    }

    if (fog_input_pressed((u32) In::EDIT_REMOVE, Player::ANY) || fog_input_mouse_pressed(3)) {
        s32 best_index = -1;
        f32 best_dist = global_editor.worst_best_distance;
        std::vector<Vec2> *points = &sprite->points;
        u32 num_points = points->size();
        for (u32 i = 0; i < num_points; i++) {
            f32 dist = fog_length_v2(point - (*points)[i]);
            if (dist < best_dist) {
                best_index = i;
                best_dist = dist;
            }
        }
        if (best_index >= 0) {
            points->erase(points->begin() + best_index);
        }
    }

    if (fog_input_pressed((u32) In::EDIT_PLACE, Player::ANY) || fog_input_mouse_pressed(2)) {
        s32 best_index = 0;
        f32 best_dist = 1000;
        std::vector<Vec2> *points = &sprite->points;
        u32 num_points = points->size();
        for (u32 i = 0; i < num_points; i++) {
            Vec2 prev = (*points)[i];
            Vec2 next = (*points)[(i + 1) % num_points];
            f32 new_path = fog_length_v2(prev - point) + fog_length_v2(next - point);
            f32 change = new_path;
            if (change < best_dist) {
                best_index = (i + 1) % num_points;
                best_dist = change;
            }
        }
        points->insert(points->begin() + best_index, point);
    }

    if (fog_input_pressed((u32) In::EDIT_SAVE, Player::ANY)) {
        FILE *file = fopen(path, "w");
        for (u32 i = 0; i < global_editor.sprites.size(); i++) {
            write_sprite_to_file(global_editor.sprites[i], file);
        }
        fclose(file);
    }

}

// Draw functions
void sprite_editor_draw() {
    EditableSprite *sprite = &global_editor.sprites[global_editor.current_sprite];
    fog_renderer_push_sprite_rect(0, fog_V2(0.5, 0.5), fog_V2(1, 1), 0, sprite->sheet, fog_V2(0, 0), fog_V2(512, 512), fog_V4(1, 1, 1, 1));
    const Vec4 line_color = fog_V4(1.0, 0.5, 0.0, 1.0);
    const f32 line_width = 0.03 / fog_renderer_fetch_camera(0)->zoom;
    fog_renderer_push_line(1, fog_V2(0, 0), fog_V2(1, 0), line_color, line_width);
    fog_renderer_push_line(1, fog_V2(1, 0), fog_V2(1, 1), line_color, line_width);
    fog_renderer_push_line(1, fog_V2(1, 1), fog_V2(0, 1), line_color, line_width);
    fog_renderer_push_line(1, fog_V2(0, 1), fog_V2(0, 0), line_color, line_width);

    const Vec4 shape_color = fog_V4(0.0, 0.5, 1.0, 0.5);
    const Vec4 point_color = fog_V4(1.0, 0.5, 0.0, 0.75);
    const Vec4 closest_point_color = fog_V4(0.0, 0.5, 1.0, 0.75);

    s32 closest = -1;
    f32 dist = global_editor.worst_best_distance;
    Vec2 point = global_editor.cursor;

    std::vector<Vec2> *points = &sprite->points;
    s32 num_points = points->size();
    for (s32 i = 0; i < num_points; i++) {
        Vec2 curr = (*points)[i];
        Vec2 next = (*points)[(i + 1) % num_points];
        fog_renderer_push_line(2, next, curr, shape_color, line_width);

        if (fog_distance_v2(point, curr) < dist) {
            closest = i;
            dist = fog_distance_v2(point, curr);
        }
    }

    const f32 point_size = 0.04 / fog_renderer_fetch_camera(0)->zoom;
    const f32 point_size_closest = 0.045 / fog_renderer_fetch_camera(0)->zoom;
    for (s32 i = 0; i < num_points; i++) {
        Vec2 curr = (*points)[i];
        if (closest == i)
            fog_renderer_push_point(3, curr, closest_point_color, point_size_closest);
        else
            fog_renderer_push_point(3, curr, point_color, point_size);
    }

    // fog_util_debug_text(sprite->name, Renderer::fetch_camera()->aspect_ratio);
}

int main(int argc, char *argv[]) {
    fog_input_request_name((u32) In::EDIT_NUM_BINDINGS);
    fog_init(argc, argv);
    setup(argc, argv);

    fog_run(sprite_editor_update, sprite_editor_draw);
}
