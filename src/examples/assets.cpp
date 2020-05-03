///# Asset examples
// How to interface efficently with the asset system.

//// Loading a new asset
// <p>
// Place the desired asset in the "res" folder, make sure it's
// a compatible type for the engine, and rebuild the project.
// The AssetID can be found with the
// <code>fog_asset_fetch_id(char *name)</code>-function
// </p>
AssetID monaco_font = fog_asset_fetch_id("MONACO_FONT");
fog_renderer_draw_text("Blargh!", 0, 0, 1, monaco_font, 0, fog_V4(1, 1, 1, 1), 0, 1);
