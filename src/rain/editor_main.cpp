#include "editor_main.h"
#include "stdio.h"

void read_with_default(const char *prompt, const char *def, char *val, u32 length) {
    s32 input_length;
    do {
        printf("%s", prompt);
        if (def)
            printf(" [%s]", def);
        printf(": ");

        fgets(val, length, stdin);
        input_length = Util::str_len(val);
        if (val[MAX(input_length - 2, 0)] == '\n') {
            break;
        }

        printf("You passed in a too long string, max length is %d.\n", length);
        do {
            fgets(val, length, stdin);
            input_length = Util::str_len(val);
        } while (val[MAX(input_length - 2, 0)] != '\n');
    } while (true);

    if (input_length == 2)
        Util::copy_bytes(def, val, Util::str_len(def));
    else
        val[input_length - 2] = '\0';

}

void write_sprite_to_file(EditableSprite sprite, FILE *file) {
    fprintf(file, "%s:#%llu:%d:", sprite.name,
            Res::HASH_LUT[sprite.sheet], sprite.points.length);

    Vec2 center = V2(0, 0);
    for (u32 i = 0; i < sprite.points.length; i++) {
        center += sprite.points[i];
    }
    center /= sprite.points.length;
    for (u32 i = 0; i < sprite.points.length; i++) {
        Vec2 p = sprite.points[i];
        fprintf(file, "(%f %f %f %f)",
            p.x - center.x, p.y - center.y,
            p.x, p.y);
    }
    fprintf(file, "\n");
}

AssetID find_next_sheet(AssetID start=0, int dir=1) {
    for (s32 i = 1; i < (s32) Res::NUM_ASSETS; i++) {
        AssetID curr = (start + i * dir + Res::NUM_ASSETS) % Res::NUM_ASSETS;
        if (Asset::is_of_type(curr, Asset::Type::TEXTURE))
            return curr;
    }
    return Asset::ASSET_ID_NO_ASSET;
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
        Util::create_list<Vec2>(10),
        find_next_sheet(0, 1),
    };
    read_with_default("sprite name",
            nullptr,
            new_sprite.name,
            MAX_TEXT_LENGTH);

    global_editor.current_sprite = global_editor.sprites.length;
    global_editor.sprites.append(new_sprite);
}

void setup(int argc, char **argv) {
    Util::allow_all_allocations();

    // Commandline parsing, sets globalstate
    for (s32 i = 1; i < argc; i++) {
        char *val = argv[i];
        if (*val != '-') {
            if (path) {
                LOG("Multiple files given replacing '%s' with '%s'.", path, argv[i]);
            }
            path = argv[i];
        } else {
            // Parse flags
            val++;
            if (*val == '-') val++;
            switch (*val) {
                default:
                    LOG("Unkown argument '%s', ignoring.", argv[i]);
            };
        }
    }



    global_editor.sprites = Util::create_list<EditableSprite>(10);
    global_editor.cursor = V2(0.5, 0.5);
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
                    ERR("Failed to parse number in sprite file '%s'", \
                            path);                      \
                    fclose(sprite_file);                              \
                    return;                                           \
                }                                                     \
            } while (false);

            char *endptr;
            u32 num_points = strtol(num, &endptr, 10);
            CHECK_FOR_ERROR(num);
            List<Vec2> points = Util::create_list<Vec2>(num_points);
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
                points.append(V2(point.z, point.w));
            }
#undef CHECK_FOR_ERROR

            u32 resource_hash = res[0] == '#' ? atoi(res + 1): Asset::asset_hash(res);
            // Reverse the hash
            for (u32 i = 0; i < LEN(Res::HASH_LUT); i++) {
                if (Res::HASH_LUT[i] == resource_hash) {
                    resource_hash = i;
                }
            }

            EditableSprite sprite = {
                points,
                resource_hash,
            };
            Util::copy_bytes(name, sprite.name, MIN(MAX_TEXT_LENGTH, Util::str_len(name)));
            global_editor.sprites.append(sprite);
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

    using namespace Input;
    // Sprite editor
    add(K(a), Name::EDIT_PLACE);
    add(K(s), Name::EDIT_SELECT);
    add(K(d), Name::EDIT_REMOVE);
    add(K(r), Name::EDIT_RENAME);
    add(K(u), Name::EDIT_NEXT_SPRITE);
    add(K(i), Name::EDIT_PREV_SPRITE);
    add(K(n), Name::EDIT_NEXT_SPRITE_SHEET);
    add(K(p), Name::EDIT_PREV_SPRITE_SHEET);
    add(K(j), Name::EDIT_SNAP_SMALLER);
    add(K(k), Name::EDIT_SNAP_LARGER);
    add(K(w), Name::EDIT_SAVE);
    add(K(c), Name::EDIT_ADD_SPRITE);
    add(K(r), Name::EDIT_RENAME);

    add(A(LEFTX, Player::P1), Name::EDIT_MOVE_RIGHT_LEFT);
    add(A(LEFTY, Player::P1), Name::EDIT_MOVE_UP_DOWN);
    add(A(RIGHTY, Player::P1), Name::EDIT_ZOOM_IN_OUT);
    add(B(B, Player::P1), Name::EDIT_PLACE);
    add(B(A, Player::P1), Name::EDIT_SELECT);
    add(B(Y, Player::P1), Name::EDIT_REMOVE);
    add(B(LEFTSHOULDER, Player::P1), Name::TWEAK_STEP);
    add(B(DPAD_DOWN, Player::P1), Name::EDIT_SNAP_SMALLER);
    add(B(DPAD_UP, Player::P1), Name::EDIT_SNAP_LARGER);
    add(A(TRIGGERRIGHT, Player::P1), Name::EDIT_NEXT_SPRITE);
    add(A(TRIGGERLEFT, Player::P1), Name::EDIT_PREV_SPRITE);
    add(B(DPAD_RIGHT, Player::P1), Name::EDIT_NEXT_SPRITE_SHEET);
    add(B(DPAD_LEFT, Player::P1), Name::EDIT_PREV_SPRITE_SHEET);
    add(B(START, Player::P1), Name::EDIT_SAVE);
    add(B(BACK, Player::P1), Name::EDIT_ADD_SPRITE);

    Renderer::fetch_camera()->position = V2(0.5, 0.5);
}

void sprite_editor_update() {
    using namespace Input;
    Vec2 point;
    // Input
    {
        // Rename?
        if (pressed(Name::EDIT_RENAME)) {
            read_with_default("sprite rename", nullptr,
                    global_editor.sprites[global_editor.current_sprite].name,
                    MAX_TEXT_LENGTH);
        }

        const u32 num_sprites = global_editor.sprites.length;
        if (pressed(Name::EDIT_PREV_SPRITE) && num_sprites) {
            global_editor.current_sprite -= 1;
            global_editor.current_sprite += num_sprites;
            global_editor.current_sprite %= num_sprites;
        }
        if (pressed(Name::EDIT_NEXT_SPRITE) && num_sprites) {
            global_editor.current_sprite += 1;
            global_editor.current_sprite += num_sprites;
            global_editor.current_sprite %= num_sprites;
        }

        f32 delta = MIN(Logic::delta(), 0.2);
        f32 move_x = value(Name::EDIT_MOVE_RIGHT_LEFT);
        move_x += value(Name::EDIT_MOVE_RIGHT);
        move_x -= value(Name::EDIT_MOVE_LEFT);

        f32 move_y = value(Name::EDIT_MOVE_UP_DOWN);
        move_y += value(Name::EDIT_MOVE_UP);
        move_y -= value(Name::EDIT_MOVE_DOWN);

        f32 zoom;
        if (Input::using_controller())
            zoom = (1.0 + value(Name::EDIT_ZOOM_IN_OUT) * delta);
        else
            zoom = (1.0 + (mouse_scroll().y * 5) * delta);
        Renderer::fetch_camera(0)->zoom *= zoom;
        f32 current_zoom = Renderer::fetch_camera(0)->zoom;
        const f32 speed = delta / current_zoom;
        Vec2 cursor;
        if (Input::using_controller())
            cursor = global_editor.cursor;
        else
            cursor = Input::world_mouse_position();
        global_editor.worst_best_distance = global_editor.orig_worst_best_distance / current_zoom;

        if (pressed(Name::EDIT_SNAP_SMALLER)) {
            global_editor.snapping_scale *= global_editor.snapping_scale_step;
        }
        if (pressed(Name::EDIT_SNAP_LARGER)) {
            global_editor.snapping_scale /= global_editor.snapping_scale_step;
        }
        global_editor.snapping_scale = CLAMP(1.0 / 512.0, 1.0, global_editor.snapping_scale);

        if (down(Name::TWEAK_STEP)) {
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
            Util::precise_snap(&cursor.x, snap, snap);
            Util::precise_snap(&cursor.y, snap, snap);
        } else {
            cursor += V2(move_x, move_y) * speed;
        }
        cursor = V2(CLAMP(-0.1, 1.1, cursor.x), CLAMP(-0.1, 1.1, cursor.y));
        global_editor.cursor = cursor;
        // Like a return value... but not...
        point = cursor;
    }

    if (pressed(Name::EDIT_ADD_SPRITE)) {
        create_new_sprite();
    }

    // Drawing logic
    {
        f32 current_zoom = Renderer::fetch_camera(0)->zoom;
        const f32 snap = global_editor.snapping_scale;
        const f32 line_width = 0.03 / current_zoom;
        const Vec4 dim_color = V4(1, 0, 1, 0.05);
        const Vec4 vis_color = V4(1, 0, 1, 0.2);
        Vec4 color = down(Name::TWEAK_STEP) ? vis_color : dim_color;
        for (u32 i = 0; i <= 1.0 / snap; i++) {
            Vec2 start, end;
            start = V2(i * snap, 0.0);
            end = V2(i * snap, 1.0);
            Renderer::push_line(5, start, end, color, line_width);
            start = V2(0.0, i * snap);
            end = V2(1.0, i * snap);
            Renderer::push_line(5, start, end, color, line_width);
        }
        const Vec4 cursor_color = V4(0.0, 1.0, 0.0, 1.0);
        Renderer::push_point(10, point, cursor_color, 0.05 / current_zoom);
    }

    // Camera Movement
    {
        Renderer::Camera *camera = Renderer::fetch_camera(0);
        f32 dist = distance(camera->position, point);
        f32 threshold = 0.5 / camera->zoom;
        if (dist > threshold) {
            f32 speed = 10 * (dist - threshold) * camera->zoom * Logic::delta();
            camera->position += (point - camera->position) * speed;
        }
    }

    // TODO(ed): Select sprite
    // TODO(ed): Allow multiple sprites to be editable at once
    // TODO(ed): Tweak values for things like, hue of the overlay
    // and such, so any sprite can be rendered underneath

    // Sprite select
    EditableSprite *sprite = global_editor.sprites + global_editor.current_sprite;
    {
        AssetID next = sprite->sheet;
        if (pressed(Name::EDIT_NEXT_SPRITE_SHEET)) {
            next = find_next_sheet(next, 1);
        }
        if (pressed(Name::EDIT_PREV_SPRITE_SHEET)) {
            next = find_next_sheet(next, -1);
        }
        if (next != Asset::ASSET_ID_NO_ASSET)
            sprite->sheet = next;
    }

    // Logic
    if (down(Name::EDIT_SELECT) || mouse_down(0)) {
        s32 best_index = -1;
        f32 best_dist = global_editor.worst_best_distance;
        List<Vec2> *points = &sprite->points;
        u32 num_points = points->size();
        for (u32 i = 0; i < num_points; i++) {
            f32 dist = length(point - (*points)[i]);
            if (dist < best_dist) {
                best_index = i;
                best_dist = dist;
            }
        }
        if (best_index >= 0) {
            points[best_index] = point;
        }
    }

    if (pressed(Name::EDIT_REMOVE) || mouse_pressed(3)) {
        s32 best_index = -1;
        f32 best_dist = global_editor.worst_best_distance;
        List<Vec2> *points = &sprite->points;
        u32 num_points = points->length;
        for (u32 i = 0; i < num_points; i++) {
            f32 dist = length(point - (*points)[i]);
            if (dist < best_dist) {
                best_index = i;
                best_dist = dist;
            }
        }
        if (best_index >= 0) {
            points->remove(best_index);
        }
    }

    if (pressed(Name::EDIT_PLACE) || mouse_pressed(2)) {
        s32 best_index = 0;
        f32 best_dist = 1000;
        Util::List<Vec2> *points = &sprite->points;
        u32 num_points = points->length;
        for (u32 i = 0; i < num_points; i++) {
            Vec2 prev = (*points)[i];
            Vec2 next = (*points)[(i + 1) % num_points];
            f32 new_path = length(prev - point) + length(next - point);
            f32 change = new_path;
            if (change < best_dist) {
                best_index = (i + 1) % num_points;
                best_dist = change;
            }
        }
        points->insert(best_index, point);
    }

    if (pressed(Name::EDIT_SAVE)) {
        FILE *file = fopen(path, "w");
        for (u32 i = 0; i < global_editor.sprites.length; i++) {
            write_sprite_to_file(global_editor.sprites[i], file);
        }
        fclose(file);
    }

}

// Draw functions
void sprite_editor_draw() {
    EditableSprite *sprite = global_editor.sprites + global_editor.current_sprite;
    Renderer::push_sprite_rect(0, V2(0.5, 0.5), V2(1, 1), 0, sprite->sheet, V2(0, 0), V2(512, 512));
    const Vec4 line_color = V4(1.0, 0.5, 0.0, 1.0);
    const f32 line_width = 0.03 / Renderer::fetch_camera(0)->zoom;
    Renderer::push_line(1, V2(0, 0), V2(1, 0), line_color, line_width);
    Renderer::push_line(1, V2(1, 0), V2(1, 1), line_color, line_width);
    Renderer::push_line(1, V2(1, 1), V2(0, 1), line_color, line_width);
    Renderer::push_line(1, V2(0, 1), V2(0, 0), line_color, line_width);

    const Vec4 shape_color = V4(0.0, 0.5, 1.0, 0.5);
    const Vec4 point_color = V4(1.0, 0.5, 0.0, 0.75);
    const Vec4 closest_point_color = V4(0.0, 0.5, 1.0, 0.75);

    s32 closest = -1;
    f32 dist = global_editor.worst_best_distance;
    Vec2 point = global_editor.cursor;

    List<Vec2> points = sprite->points;
    s32 num_points = points.length;
    for (s32 i = 0; i < num_points; i++) {
        Vec2 curr = points[i];
        Vec2 next = points[(i + 1) % num_points];
        Renderer::push_line(2, next, curr, shape_color, line_width);

        if (distance(point, curr) < dist) {
            closest = i;
            dist = distance(point, curr);
        }
    }

    const f32 point_size = 0.04 / Renderer::fetch_camera(0)->zoom;
    const f32 point_size_closest = 0.045 / Renderer::fetch_camera(0)->zoom;
    for (s32 i = 0; i < num_points; i++) {
        Vec2 curr = points[i];
        if (closest == i)
            Renderer::push_point(3, curr, closest_point_color, point_size_closest);
        else
            Renderer::push_point(3, curr, point_color, point_size);
    }

    Util::debug_text(sprite->name, Renderer::fetch_camera()->aspect_ratio);
}

int main(int argc, char *argv[]) {
    fog_input_request_name((u32) Name::EDIT_NUM_BINDINGS);
    fog_init();

    fog_run(sprite_editor_update, sprite_editor_draw);
}
