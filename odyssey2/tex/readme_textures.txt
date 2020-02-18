Change skyboxes in main.h
Skyboxes from http://www.custommapmakers.org/skyboxes.php (licenses in corresponding folders)

Change textures in initTerrain() in main.cpp
Textures from https://texturehaven.com/
	brown_mud_rocks_01
	burned_ground_01 (fits snow_03, stormydays)
	mud_leaves
	red_dirt_mud_01 (fits mud_leaves)
	rock_03 (moss), rock_06 (rough), rock_08 (smooth)
	snow_02_translucent

Suggested sun colors (not implemented), sun directions (terrain.frag) and 
textures for different skyboxes
stormydays: Warm climate
	Yellow
	vec3(1, 0.75, 1)
	terrain_shader->load_stb_texture_ref("tex/snow_02_translucent.png", &terrain_tex_ids.snow_tex, false);
	terrain_shader->load_stb_texture_ref("tex/burned_ground_01.png", &terrain_tex_ids.grass_tex, false);
	terrain_shader->load_stb_texture_ref("tex/rock_06.png", &terrain_tex_ids.rock_tex, false);
	
hw_morning: Mountains, rain forest
	Green, blue
	vec3(?, ?, ?)
	terrain_shader->load_stb_texture_ref("tex/rock_06.png", &terrain_tex_ids.snow_tex, false);
	terrain_shader->load_stb_texture_ref("tex/mud_leaves.png", &terrain_tex_ids.grass_tex, false);
	terrain_shader->load_stb_texture_ref("tex/rock_03.png", &terrain_tex_ids.rock_tex, false); // Alt. rock_08
	
sb_frozen: Snowy mountains
	Blue
	vec3(?, ?, ?)
	terrain_shader->load_stb_texture_ref("tex/snow_02_translucent.png", &terrain_tex_ids.snow_tex, false);
	terrain_shader->load_stb_texture_ref("tex/brown_mud_rocks_01.png", &terrain_tex_ids.grass_tex, false);
	terrain_shader->load_stb_texture_ref("tex/rock_03.png", &terrain_tex_ids.rock_tex, false);
	
ame_starfield: Star sky
	Dark
	vec3(0, 1, 0)?
	terrain_shader->load_stb_texture_ref("tex/rock_08.png", &terrain_tex_ids.snow_tex, false); // Alt. rock_03
	terrain_shader->load_stb_texture_ref("tex/mud_leaves.png", &terrain_tex_ids.grass_tex, false);
	terrain_shader->load_stb_texture_ref("tex/red_dirt_mud_01.png", &terrain_tex_ids.rock_tex, false);
