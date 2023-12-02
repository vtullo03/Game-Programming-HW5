#include "Level1.h"
#include "Utility.h"

#define LEVEL_WIDTH 14
#define LEVEL_HEIGHT 8

// texture filepaths
// MAPS
const char MAP_TILESET_FILEPATH[] = "tileset.png",
PLAYER_FILEPATH[] = "player_placeholder.png",
CHAIN_FILEPATH[] = "chain_placeholder.png";

Level1::~Level1()
{
    delete    m_state.enemies;
    delete    m_state.player;
    delete    m_state.map;
    delete m_state.chain;
    //Mix_FreeChunk(m_state.jump_sfx);
    //Mix_FreeMusic(m_state.bgm);
}

void Level1::initialise()
{
    unsigned int LEVEL_DATA[] =
    {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 0, 0, 1,
        1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
        1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
        1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
        1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1
    };

    GLuint map_texture_id = Utility::load_texture(MAP_TILESET_FILEPATH);
    m_state.map = new Map(LEVEL_WIDTH, LEVEL_HEIGHT, LEVEL_DATA, map_texture_id, 1.0f, 4, 1);

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
    m_state.chain->m_has_gravity = false;
    m_state.chain->m_texture_id = Utility::load_texture(CHAIN_FILEPATH);
    m_state.chain->disable();


    /**
     BGM and SFX
    
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);

    m_state.bgm = Mix_LoadMUS("assets/audio/dooblydoo.mp3");
    Mix_PlayMusic(m_state.bgm, -1);
    Mix_VolumeMusic(0.0f);

    m_state.jump_sfx = Mix_LoadWAV("assets/audio/bounce.wav");*/
}

void Level1::update(float delta_time)
{
    m_state.player->update(delta_time, m_state.player, NULL, 0, m_state.map);
    m_state.chain->update(delta_time, m_state.player, NULL, 0, m_state.map);
}


void Level1::render(ShaderProgram* program)
{
    m_state.map->render(program);
    m_state.player->render(program);
    m_state.chain->render(program);
}