namespace Test {

Result logic_callback_add_pre_update_once() {
    int i = 0;
    auto func = [&i]() {
        i++;
    };
    Logic::LogicID id = Logic::add_callback(Logic::At::PRE_UPDATE, func, 0.0, Logic::ONCE);
    Logic::call(Logic::At::PRE_UPDATE);
    if (i != 1)
        return FAIL;
    Logic::call(Logic::At::PRE_UPDATE);
    return i == 1 ? PASS : FAIL;
}

Result logic_callback_add_post_update_once() {
    int i = 0;
    auto func = [&i]() {
        i++;
    };
    Logic::LogicID id = Logic::add_callback(Logic::At::POST_UPDATE, func, 0.0, Logic::ONCE);
    Logic::call(Logic::At::POST_UPDATE);
    if (i != 1)
        return FAIL;
    Logic::call(Logic::At::POST_UPDATE);
    return i == 1 ? PASS : FAIL;
}

Result logic_callback_add_pre_draw_once() {
    int i = 0;
    auto func = [&i]() {
        i++;
    };
    Logic::LogicID id = Logic::add_callback(Logic::At::PRE_DRAW, func, 0.0, Logic::ONCE);
    Logic::call(Logic::At::PRE_DRAW);
    if (i != 1)
        return FAIL;
    Logic::call(Logic::At::PRE_DRAW);
    return i == 1 ? PASS : FAIL;
}

Result logic_callback_add_post_draw_once() {
    int i = 0;
    auto func = [&i]() {
        i++;
    };
    Logic::LogicID id = Logic::add_callback(Logic::At::POST_DRAW, func, 0.0, Logic::ONCE);
    Logic::call(Logic::At::POST_DRAW);
    if (i != 1)
        return FAIL;
    Logic::call(Logic::At::POST_DRAW);
    return i == 1 ? PASS : FAIL;
}

Result logic_callback_single_frame() {
    int pre_u = 0;
    int post_u = 0;
    int pre_d = 0;
    int post_d = 0;
    auto f_pre_u = [&pre_u]() {
        pre_u++;
    };
    auto f_post_u = [&post_u]() {
        post_u++;
    };
    auto f_pre_d = [&pre_d]() {
        pre_d++;
    };
    auto f_post_d = [&post_d]() {
        post_d++;
    };
    Logic::LogicID id_pre_u = Logic::add_callback(Logic::At::PRE_UPDATE, f_pre_u, 0.0, Logic::ONCE);
    Logic::LogicID id_post_u = Logic::add_callback(Logic::At::POST_UPDATE, f_post_u, 0.0, Logic::ONCE);
    Logic::LogicID id_pre_d = Logic::add_callback(Logic::At::PRE_DRAW, f_pre_d, 0.0, Logic::ONCE);
    Logic::LogicID id_post_d = Logic::add_callback(Logic::At::POST_DRAW, f_post_d, 0.0, Logic::ONCE);
    Logic::call(Logic::At::PRE_UPDATE);
    Logic::call(Logic::At::POST_UPDATE);
    Logic::call(Logic::At::PRE_DRAW);
    Logic::call(Logic::At::POST_DRAW);
    if (pre_u != 1
        || post_u != 1
        || pre_d != 1
        || post_d != 1
       )
        return FAIL;
    Logic::call(Logic::At::PRE_UPDATE);
    Logic::call(Logic::At::POST_UPDATE);
    Logic::call(Logic::At::PRE_DRAW);
    Logic::call(Logic::At::POST_DRAW);
    return pre_u == 1
        && post_u == 1
        && pre_d == 1
        && post_d == 1
        ? PASS : FAIL;
}

Result logic_callback_forever() {
    int i = 0;
    auto func = [&i]() {
        i++;
    };
    Logic::LogicID id = Logic::add_callback(Logic::At::PRE_UPDATE, func, 0.0, Logic::FOREVER);
    for (int j = 0; j < 100; j++) {
        if (i != j)
            return FAIL;
        Logic::call(Logic::At::PRE_UPDATE);
    }
    return PASS;
}

Result logic_callback_remove() {
    int i = 0;
    auto func = [&i]() {
        i++;
    };
    Logic::LogicID id = Logic::add_callback(Logic::At::PRE_UPDATE, func, 0.0, Logic::FOREVER);
    Logic::call(Logic::At::PRE_UPDATE);
    if (i != 1)
        return FAIL_EXT;
    Logic::remove_callback(id);
    Logic::call(Logic::At::PRE_UPDATE);
    return i == 1 ? PASS : FAIL;
}

Result logic_callback_update() {
    int i = 0;
    auto func_inc = [&i]() {
        i++;
    };
    auto func_dec = [&i]() {
        i--;
    };
    Logic::LogicID id = Logic::add_callback(Logic::At::PRE_UPDATE, func_inc, 0.0, Logic::FOREVER);
    Logic::call(Logic::At::PRE_UPDATE);
    if (i != 1)
        return FAIL_EXT;
    Logic::update_callback(id, func_dec, 0.0, Logic::FOREVER);
    Logic::call(Logic::At::PRE_UPDATE);
    return i == 0 ? PASS : FAIL;
}

//TODO(gu)
// - step

}
