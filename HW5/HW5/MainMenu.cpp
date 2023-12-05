#include "MainMenu.h"
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
FONT_FILEPATH[] = "font.png";

unsigned int MAINMENU_DATA[] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1
};

MainMenu::~MainMenu()
{
    delete    m_state.enemies;
    delete    m_state.player;
    delete    m_state.map;
    delete m_state.chain;
    Mix_FreeChunk(m_state.jump_sfx);
    Mix_FreeMusic(m_state.bgm);
}

void MainMenu::initialise()
{

    GLuint map_texture_id = Utility::load_texture(MAP_TILESET_FILEPATH);
    m_state.map = new Map(LEVEL_WIDTH, LEVEL_HEIGHT, MAINMENU_DATA, map_texture_id, 1.0f, 4, 1);

    // PLAYER
    m_state.player = new Entity();
    m_state.player->m_texture_id = Utility::load_texture(PLAYER_FILEPATH);
    m_state.player->disable();

    // CHAIN
    m_state.chain = new Entity();
    m_state.chain->m_texture_id = Utility::load_texture(CHAIN_FILEPATH);
    m_state.chain->disable();

    // DOOR
    m_state.door = new Entity();
    m_state.door->m_texture_id = Utility::load_texture(DOOR_FILEPATH);
    m_state.door->disable();

    // ENEMY
    m_state.enemies = new Entity();
    m_state.player->disable();

    /*
     BGM and SFX*/

    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);

    m_state.bgm = Mix_LoadMUS("crowd_hammer.mp3");
    Mix_PlayMusic(m_state.bgm, -1);
    Mix_VolumeMusic(10.0f);

    //m_state.jump_sfx = Mix_LoadWAV("assets/audio/bounce.wav");
}

void MainMenu::update(float delta_time)
{
    m_state.player->update(delta_time, m_state.player, NULL, 0, m_state.map);
    m_state.chain->update(delta_time, m_state.player, NULL, 0, m_state.map);
    m_state.door->update(delta_time, m_state.player, m_state.player, 1, m_state.map);
}


void MainMenu::render(ShaderProgram* program)
{
    Utility::draw_text(program, Utility::load_texture(FONT_FILEPATH), "CHAINED COWBOY", 0.5f,
        -0.2f, glm::vec3(-3.0f, 2.0f, 0.0f));
    Utility::draw_text(program, Utility::load_texture(FONT_FILEPATH), "Press enter to start", 0.5f,
        -0.2f, glm::vec3(-3.0f, 0.0f, 0.0f));
    m_state.map->render(program);
    m_state.player->render(program);
    m_state.chain->render(program);
    m_state.door->render(program);
}