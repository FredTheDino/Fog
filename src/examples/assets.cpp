///# Asset examples
// How to interface efficently with the asset system.

//// Loading a new asset
// <p>
// Place the desired asset in the "res" folder, make sure it's
// a compatible type for the engine, and rebuild the project.
// The "src/fog_assets.cpp" file has now been updated and the
// new constant can be used where "AssetID"s are asked for.
// The constants in "src/fog_assets.cpp" should be written
// as the name of the constants, since the numbers of the
// constants can change.
// </p>

//// Using loaded fonts
// <p>
// All asset constants start with the "ASSET_*" prefix, and then a name
// generated from the file loaded. One potential pitfall is passing of fonts,
// since there are multiple parts of the font, one for the texture and one for
// the actual font data, when passing in fonts you want the one ending with
// "*_FONT". So usage would look like:
// </p>
fog_renderer_draw_text("Blargh!", 0, 0, 1, ASSET_DROID_SANS_FONT, 0, fog_V4(1, 1, 1, 1), 0, 0);
