#define GL_SILENCE_DEPRECATION
#define GL_GLEXT_PROTOTYPES 1
#define FIXED_TIMESTEP 0.0166666f
#define LEVEL1_WIDTH 14
#define LEVEL1_HEIGHT 8
#define LEVEL1_LEFT_EDGE 5.0f

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <vector>
#include <SDL_mixer.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "cmath"
#include <ctime>
#include <vector>
#include "Entity.h"
#include "Map.h"
#include "Utility.h"
#include "Scene.h"
#include "MainMenu.h"
#include "Level1.h"
#include "Level2.h"
#include "Level3.h"
#include "Won.h"
#include "Lost.h"


// CONSTS
// window dimensions + viewport
const int WINDOW_WIDTH = 640 * 2,
WINDOW_HEIGHT = 480 * 2;

const int VIEWPORT_X = 0,
VIEWPORT_Y = 0,
VIEWPORT_WIDTH = WINDOW_WIDTH,
VIEWPORT_HEIGHT = WINDOW_HEIGHT;

// background color -- CHANGE LATER
const float BG_RED = 0.68f,
BG_BLUE = 0.88f,
BG_GREEN = 0.88f,
BG_OPACITY = 1.0f;

// shaders
const char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

const float MILLISECONDS_IN_SECOND = 1000.0;

// ————— GLOBAL VARIABLES ————— //
Scene* g_current_scene;
MainMenu* g_main_menu;
Level1* g_level_1;
Level2* g_level_2;
Level3* g_level_3;
Won* g_level_won;
Lost* g_level_lost;

Scene* g_levels[6];

SDL_Window* g_display_window;
bool g_game_is_running = true;

ShaderProgram g_shader_program;
glm::mat4 g_view_matrix, g_projection_matrix;

float g_previous_ticks = 0.0f;
float g_accumulator = 0.0f;

int next_level_index = 0;

bool is_paused = false;
bool is_won = false;
const char FONT_FILEPATH[] = "font.png";

int number_of_lives = 3;

void switch_to_scene(Scene* scene)
{
    next_level_index += 1;
    g_current_scene = scene;
    g_current_scene->initialise();
}

void initialise()
{
    // ————— VIDEO ————— //
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    g_display_window = SDL_CreateWindow("HW5!!!!!",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_OPENGL);

    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);

#ifdef _WINDOWS
    glewInit();
#endif

    // ————— GENERAL ————— //
    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);

    g_view_matrix = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);

    glUseProgram(g_shader_program.get_program_id());

    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);

    // levels setup
    g_main_menu = new MainMenu();
    g_level_1 = new Level1();
    g_level_2 = new Level2();
    g_level_3 = new Level3();
    g_level_won = new Won();
    g_level_lost = new Lost();

    g_levels[0] = g_main_menu;
    g_levels[1] = g_level_1;
    g_levels[2] = g_level_2;
    g_levels[3] = g_level_3;
    g_levels[4] = g_level_won;
    g_levels[5] = g_level_lost;

    // Start at level A
    switch_to_scene(g_levels[0]);

    // ————— BLENDING ————— //
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input()
{
    // reset player movement vector
    if (next_level_index != 0)
    {
        g_current_scene->m_state.player->set_movement(glm::vec3(0.0f));
    }

    SDL_Event event;
    // check if game is quit
    while (SDL_PollEvent(&event))
    {
        switch (event.type) {
            // End game
        case SDL_QUIT:
        case SDL_WINDOWEVENT_CLOSE:
            g_game_is_running = false;
            break;

        case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
            case SDLK_q:
                // Quit the game with a keystroke
                g_game_is_running = false;
                break;

            case SDLK_SPACE:
                // Jump
                if (!g_current_scene->m_state.chain->get_active_state())
                {
                    if (g_current_scene->m_state.player->m_collided_bottom && !g_current_scene->m_state.player->m_is_jumping)
                    {
                        g_current_scene->m_state.player->m_is_jumping = true;
                    }
                    else if ((g_current_scene->m_state.player->m_wallcheck_left || g_current_scene->m_state.player->m_wallcheck_right)
                        && !g_current_scene->m_state.player->m_is_wall_jumping)
                    {
                        g_current_scene->m_state.player->m_is_wall_jumping = true;
                    }
                    Mix_PlayChannel(-1, g_current_scene->m_state.jump_sfx, 0);
                }
                break;

            case SDLK_RETURN:
                if (g_current_scene == g_main_menu)
                {
                    switch_to_scene(g_levels[next_level_index]);
                }
                break;
            case SDLK_p:
                is_paused = !is_paused;
                break;
            }
        }
    }

    const Uint8* key_state = SDL_GetKeyboardState(NULL);

    if (!g_current_scene->m_state.chain->get_active_state() && !is_paused)
    {
        if (key_state[SDL_SCANCODE_A])
        {
            g_current_scene->m_state.player->move_left();
            if (key_state[SDL_SCANCODE_L])
            {
                g_current_scene->m_state.chain->enable();
                g_current_scene->m_state.chain->chain_direction = LEFT;
                g_current_scene->m_state.chain->chain_state = LAUNCH;
                Mix_PlayChannel(-1, g_current_scene->m_state.chain_sfx, 0);
            }
        }
        else if (key_state[SDL_SCANCODE_D])
        {
            g_current_scene->m_state.player->move_right();
            if (key_state[SDL_SCANCODE_L])
            {
                g_current_scene->m_state.chain->enable();
                g_current_scene->m_state.chain->chain_direction = RIGHT;
                g_current_scene->m_state.chain->chain_state = LAUNCH;
                Mix_PlayChannel(-1, g_current_scene->m_state.chain_sfx, 0);
            }
        }
        else if (key_state[SDL_SCANCODE_W])
        {
            if (key_state[SDL_SCANCODE_L])
            {
                g_current_scene->m_state.chain->enable();
                g_current_scene->m_state.chain->chain_direction = UP;
                g_current_scene->m_state.chain->chain_state = LAUNCH;
                Mix_PlayChannel(-1, g_current_scene->m_state.chain_sfx, 0);
            }
        }
        else if (key_state[SDL_SCANCODE_S])
        {
            if (key_state[SDL_SCANCODE_L])
            {
                g_current_scene->m_state.chain->enable();
                g_current_scene->m_state.chain->chain_direction = DOWN;
                g_current_scene->m_state.chain->chain_state = LAUNCH;
                Mix_PlayChannel(-1, g_current_scene->m_state.chain_sfx, 0);
            }
        }
    }
}

void update()
{
    // ————— DELTA TIME / FIXED TIME STEP CALCULATION ————— //
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    float delta_time = ticks - g_previous_ticks;
    g_previous_ticks = ticks;

    delta_time += g_accumulator;

    if (delta_time < FIXED_TIMESTEP)
    {
        g_accumulator = delta_time;
        return;
    }

    if (!is_paused)
    {
        if (g_current_scene->m_state.player->chain_timer > 0.0f)
        {
            g_current_scene->m_state.player->move_to_target(g_current_scene->m_state.chain->get_position());
            g_current_scene->m_state.player->m_has_gravity = false;
        }
        else g_current_scene->m_state.player->m_has_gravity = true;

        while (delta_time >= FIXED_TIMESTEP) {
            // ————— UPDATING THE SCENE (i.e. map, character, enemies...) ————— //
            g_current_scene->update(FIXED_TIMESTEP);

            delta_time -= FIXED_TIMESTEP;
        }

        g_accumulator = delta_time;

        // camera follow
        g_view_matrix = glm::mat4(1.0f);
        if (next_level_index != 0)
        {
            g_view_matrix = glm::translate(g_view_matrix, glm::vec3(-g_current_scene->m_state.player->get_position().x, -2.25f
                - g_current_scene->m_state.player->get_position().y, 0.0f));
        }

        // go to next scene if door flagged (or main menu flagged)
        if (g_current_scene->m_state.door->level_finished) switch_to_scene(g_levels[next_level_index]);

        if (g_current_scene->m_state.player->get_position().y <= -10.0f)
        {
            switch_to_scene(g_current_scene);
            next_level_index -= 1;
            number_of_lives -= 1;
        }

        if (g_current_scene->m_state.enemies->touching_player)
        {
            switch_to_scene(g_current_scene);
            next_level_index -= 1;
            number_of_lives -= 1;
        }
    }
    if (number_of_lives == 0) switch_to_scene(g_level_lost);
}

void render()
{
    g_shader_program.set_view_matrix(g_view_matrix);

    glClear(GL_COLOR_BUFFER_BIT);

    // ————— RENDERING THE SCENE (i.e. map, character, enemies...) ————— //
    g_current_scene->render(&g_shader_program);
    SDL_GL_SwapWindow(g_display_window);
}

void shutdown()
{
    SDL_Quit();

    // ————— DELETING LEVEL A DATA (i.e. map, character, enemies...) ————— //
    delete g_main_menu;
    delete g_level_1;
    delete g_level_2;
    delete g_level_3;
}

// ————— GAME LOOP ————— //
int main(int argc, char* argv[])
{
    initialise();

    while (g_game_is_running)
    {
        process_input();
        update();
        render();
    }

    shutdown();
    return 0;
}