
fn main() {
    println!("cargo:rustc-link-search=lib/");
    println!("cargo:rustc-link-lib=fog");
    println!("cargo:rustc-link-lib=SDL2");
    println!("cargo:rustc-link-lib=SDL2main");
    println!("cargo:rustc-link-lib=c");
    // println!("cargo:rustc-link-lib=c++");
    println!("cargo:rustc-link-lib=m");
}
