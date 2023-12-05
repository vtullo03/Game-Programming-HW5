#include "Level1.h"
#include "Utility.h"

#define LEVEL_WIDTH 14
#define LEVEL_HEIGHT 8

// texture filepaths
// MAPS
const char MAP_TILESET_FILEPATH[] = "Tileset.png",
PLAYER_FILEPATH[] = "Player.png",
CHAIN_FILEPATH[] = "Chain.png",
DOOR_FILEPATH[] = "Door.png",
ENEMY_FILEPATH[] = "Enemy.png",
FONT_FILEPATH[] = "Font.png";

unsigned int LEVEL1_DATA[] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    2, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    2, 0, 0, 0, 0, 0, 0, 0, 3, 3, 3, 0, 0, 1,
    2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 0, 0, 1
};

Level1::~Level1()
{
    delete    m_state.enemies;
    delete    m_state.player;
    delete    m_state.map;
    delete m_state.chain;
    Mix_FreeChunk(m_state.jump_sfx);
    Mix_FreeMusic(m_state.bgm);
}

void Level1::initialise()
{

    GLuint map_texture_id = Utility::load_texture(MAP_TILESET_FILEPATH);
    m_state.map = new Map(LEVEL_WIDTH, LEVEL_HEIGHT, LEVEL1_DATA, map_texture_id, 1.0f, 3, 1);

    // PLAYER
    m_state.player = new Entity();
    m_state.player->set_entity_type(PLAYER);
    m_state.player->set_position(glm::vec3(1.0f, -6.0f, 0.0f));
    m_state.player->set_movement(glm::vec3(0.0f, 0.0f, 0.0f));
    m_state.player->set_speed(3.75f);
    m_state.player->set_jumping_power(6.0f);
    m_state.player->m_has_gravity = true;
    m_state.player->m_texture_id = Utility::load_texture(PLAYER_FILEPATH);

    // CHAIN
    m_state.chain = new Entity();
    m_state.chain->set_entity_type(CHAIN);
    m_state.chain->set_speed(3.75f);
    m_state.chain->set_movement(glm::vec3(0.0f, 0.0f, 0.0f));
    m_state.chain->m_has_gravity = false;
    m_state.chain->m_texture_id = Utility::load_texture(CHAIN_FILEPATH);
    m_state.chain->disable();

    // DOOR
    m_state.door = new Entity();
    m_state.door->set_entity_type(DOOR);
    m_state.door->set_position(glm::vec3(0.0f, -1.0f, 0.0f));
    m_state.door->set_speed(0.0f);
    m_state.door->m_has_gravity = false;
    m_state.door->m_texture_id = Utility::load_texture(DOOR_FILEPATH);

    // ENEMY
    m_state.enemies = new Entity();
    m_state.enemies->set_entity_type(ENEMY);
    m_state.enemies->set_ai_type(PATROL);
    m_state.enemies->set_ai_state(IDLE);
    m_state.enemies->set_position(glm::vec3(1.0f, -1.0f, 0.0f));
    m_state.enemies->set_speed(0.5f);
    m_state.enemies->m_has_gravity = true;
    m_state.enemies->m_texture_id = Utility::load_texture(ENEMY_FILEPATH);

    /*
     BGM and SFX*/
    
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);

    m_state.bgm = Mix_LoadMUS("crowd_hammer.mp3");
    Mix_PlayMusic(m_state.bgm, -1);
    Mix_VolumeMusic(10.0f);

    m_state.jump_sfx = Mix_LoadWAV("player_jump.wav");
    m_state.chain_sfx = Mix_LoadWAV("chain_throw.wav");
}

void Level1::update(float delta_time)
{
    m_state.player->update(delta_time, m_state.player, NULL, 0, m_state.map);
    m_state.chain->update(delta_time, m_state.player, m_state.enemies, 1, m_state.map);
    m_state.door->update(delta_time, m_state.player, m_state.player, 1, m_state.map);
    m_state.enemies->update(delta_time, m_state.player, m_state.player, 1, m_state.map);
}


void Level1::render(ShaderProgram* program)
{
    Utility::draw_text(program, Utility::load_texture(FONT_FILEPATH), "press L and movement key to grapple", 0.5f,
        -0.2f, glm::vec3(1.0f, -3.75f, 0.0f));
    Utility::draw_text(program, Utility::load_texture(FONT_FILEPATH), "grappling to enemies kills them", 0.5f,
        -0.2f, glm::vec3(1.0f, -4.50f, 0.0f));
    Utility::draw_text(program, Utility::load_texture(FONT_FILEPATH), "press space on walls to walljump", 0.5f,
        -0.2f, glm::vec3(1.0f, -5.75f, 0.0f));

    m_state.map->render(program);
    m_state.player->render(program);
    m_state.chain->render(program);
    m_state.door->render(program);
    m_state.enemies->render(program);
}