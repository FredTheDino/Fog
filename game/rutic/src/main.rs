#![allow(non_camel_case_types)]
#![allow(non_upper_case_globals)]
#![allow(dead_code)]
mod fog;

fn main() {
    unsafe {
        let mut c: i8 = 0;
        let quit = fog::fog_input_request_name(1);
        fog::fog_init(0, & mut (& mut c as *mut i8) as *mut *mut i8);
        fog::fog_input_add(fog::fog_key_to_input_code(fog::SDLK_q as i32), quit, fog::Player_P4);
        loop {
            fog::fog_update();
            println!("I'm called from rust!");
            if fog::fog_input_down(quit, fog::Player_ANY) != 0 { break; }
            fog::fog_draw()
        }
    }
}
