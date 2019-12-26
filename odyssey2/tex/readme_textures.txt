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
	terrainShader->loadStbTextureRef("tex/snow_02_translucent.png", &snowTex, false);
	terrainShader->loadStbTextureRef("tex/burned_ground_01.png", &grassTex, false);
	terrainShader->loadStbTextureRef("tex/rock_06.png", &rockTex, false);
	
hw_morning: Mountains, rain forest
	Green, blue
	vec3(?, ?, ?)
	terrainShader->loadStbTextureRef("tex/rock_06.png", &snowTex, false);
	terrainShader->loadStbTextureRef("tex/mud_leaves.png", &grassTex, false);
	terrainShader->loadStbTextureRef("tex/rock_03.png", &rockTex, false); // Alt. rock_08
	
sb_frozen: Snowy mountains
	Blue
	vec3(?, ?, ?)
	terrainShader->loadStbTextureRef("tex/snow_02_translucent.png", &snowTex, false);
	terrainShader->loadStbTextureRef("tex/brown_mud_rocks_01.png", &grassTex, false);
	terrainShader->loadStbTextureRef("tex/rock_03.png", &rockTex, false);
	
ame_starfield: Star sky
	Dark
	vec3(0, 1, 0)?
	terrainShader->loadStbTextureRef("tex/rock_08.png", &snowTex, false); // Alt. rock_03
	terrainShader->loadStbTextureRef("tex/mud_leaves.png", &grassTex, false);
	terrainShader->loadStbTextureRef("tex/red_dirt_mud_01.png", &rockTex, false);
