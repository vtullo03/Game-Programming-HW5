
#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "Entity.h"


/*
    The ENTITY Class's Default constructor
*/
Entity::Entity()
{
    // position and tranformation variables
    m_position = glm::vec3(0.0f);
    m_model_matrix = glm::mat4(1.0f);

    // physics variables
    m_velocity = glm::vec3(0.0f);
    m_acceleration = glm::vec3(0.0f);
    m_movement = glm::vec3(0.0f);
}

/*
* Update function specifically for the ENTITY class
* Checks collision in all cardinal directions
* Then calculates physics
* Then updates transformations
*
* @param delta_time, float that's the value of real-life time in seconds
* @param player, the player ENTITY -- mainly used by enemies
* @param objects, an array of entities that this ENTITY can collide with
* @param object_count, size of the array mentioned above
* @param map, the level's MAP object that the entity can collide with
*/
void Entity::update(float delta_time, Entity* player, Entity* objects, int object_count, Map* map)
{
    // if not active -- then can't update, treat like deletion
    if (!m_is_active) return;
    if (m_entity_type == CHAIN) chain_activate(player, delta_time);

    // reset collision checks every frame
    // entity collision checks
    m_collided_top = false;
    m_collided_bottom = false;
    m_collided_left = false;
    m_collided_right = false;
    // wall checks
    m_wallcheck_left = false;
    m_wallcheck_right = false;

    m_velocity.x = m_movement.x * get_speed();
    if ((m_entity_type == CHAIN) || (m_entity_type == PLAYER && chain_timer > 0.0f)) m_velocity.y = m_movement.y * get_speed();

    if (m_has_gravity) set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f)); // gravity check
    m_velocity += get_acceleration() * delta_time; // velocity equation implemented in code

    // must be calculated seperatedly for seperate collisions
    m_position.x += m_velocity.x * delta_time;
    check_collision_x(objects, object_count);
    check_collision_x(map);

    m_position.y += m_velocity.y * delta_time;
    check_collision_y(objects, object_count);
    check_collision_y(map);

    // reset model before every change
    m_model_matrix = glm::mat4(1.0f);
    m_model_matrix = glm::translate(m_model_matrix, m_position);

    // jumping
    if (m_is_jumping)
    {
        m_is_jumping = false;
        m_velocity.y += m_jumping_power;
    }

    if (m_is_wall_jumping)
    {
        m_is_wall_jumping = false;
        m_velocity.y += m_jumping_power;
    }
}


/*
* Checks for collisions in the y-axis
* Iterates through all the entities that are collidable and checks if
* touching above or below.
*
* @param collidable_entities, an array of all entities that this ENTITY can collide with
* @param collidable_entity_count, size of the array above
*
* TREAT LIKE ON_COLLISION_ENTER
*/
void const Entity::check_collision_y(Entity* collidable_entities, int collidable_entity_count)
{
    for (int i = 0; i < collidable_entity_count; i++)
    {
        Entity* collidable_entity = &collidable_entities[i];

        if (check_collision(collidable_entity))
        {
            float y_distance = fabs(m_position.y - collidable_entity->get_position().y);
            float y_overlap = fabs(y_distance - (m_height / 2.0f) - (collidable_entity->get_height() / 2.0f));
            if (m_velocity.y > 0) {
                m_position.y -= y_overlap;
                m_velocity.y = 0;
                m_collided_top = true;
            }
            else if (m_velocity.y < 0) {
                m_position.y += y_overlap;
                m_velocity.y = 0;
                m_collided_bottom = true;
            }
        }
    }
}

/*
* Check for collisions with the map in the y-axis
*
* @param map, MAP object that the ENTITY object is colliding with
*/
void const Entity::check_collision_y(Map* map)
{
    // Check all tiles above, including left and right for corner interaction
    glm::vec3 top = glm::vec3(m_position.x, m_position.y + (m_height / 2), m_position.z);
    glm::vec3 top_left = glm::vec3(m_position.x - (m_width / 2), m_position.y + (m_height / 2), m_position.z);
    glm::vec3 top_right = glm::vec3(m_position.x + (m_width / 2), m_position.y + (m_height / 2), m_position.z);

    // Check all tiles belove, including left and right for corner interaction
    glm::vec3 bottom = glm::vec3(m_position.x, m_position.y - (m_height / 2), m_position.z);
    glm::vec3 bottom_left = glm::vec3(m_position.x - (m_width / 2), m_position.y - (m_height / 2), m_position.z);
    glm::vec3 bottom_right = glm::vec3(m_position.x + (m_width / 2), m_position.y - (m_height / 2), m_position.z);

    float penetration_x = 0;
    float penetration_y = 0;

    // REFACTOR LATER
    if (get_velocity().y > 0)
    {
        if (map->is_solid(top, &penetration_x, &penetration_y))
        {
            m_position.y -= penetration_y;
            m_velocity.y = 0;
            m_collided_top = true;
        }
        else if (map->is_solid(top_left, &penetration_x, &penetration_y))
        {
            if (!m_wallcheck_left && !m_wallcheck_right)
            {
                m_position.y -= penetration_y;
                m_velocity.y = 0;
                m_collided_top = true;
            }
        }
        else if (map->is_solid(top_right, &penetration_x, &penetration_y))
        {
            if (!m_wallcheck_left && !m_wallcheck_right)
            {
                m_position.y -= penetration_y;
                m_velocity.y = 0;
                m_collided_top = true;
            }
        }
    }
    else if (get_velocity().y < 0)
    {

        if (map->is_solid(bottom, &penetration_x, &penetration_y))
        {
            m_position.y += penetration_y;
            m_velocity.y = 0;
            m_collided_bottom = true;
        }
        else if (map->is_solid(bottom_left, &penetration_x, &penetration_y))
        {
            if (!m_wallcheck_left && !m_wallcheck_right)
            {
                m_position.y += penetration_y;
                m_velocity.y = 0;
                m_collided_bottom = true;
            }
        }
        else if (map->is_solid(bottom_right, &penetration_x, &penetration_y))
        {
            if (!m_wallcheck_left && !m_wallcheck_right)
            {
                m_position.y += penetration_y;
                m_velocity.y = 0;
                m_collided_bottom = true;
            }
        }
    }
}

/*
* Checks for collisions with other ENTITY objects in the x-axis
* Iterates through all the entities that are collidable and checks if
* touching to left or right.
*
* @param collidable_entities, an array of all entities that this ENTITY can collide with
* @param collidable_entity_count, size of the array above
*
* TREAT LIKE ON_COLLISION_ENTER
*/
void const Entity::check_collision_x(Entity* collidable_entities, int collidable_entity_count)
{
    for (int i = 0; i < collidable_entity_count; i++)
    {
        Entity* collidable_entity = &collidable_entities[i];

        if (check_collision(collidable_entity))
        {
            float x_distance = fabs(m_position.x - collidable_entity->get_position().x);
            float x_overlap = fabs(x_distance - (m_width / 2.0f) - (collidable_entity->get_width() / 2.0f));
            if (m_velocity.x > 0) {
                m_position.x -= x_overlap;
                m_velocity.x = 0;
                m_collided_right = true;
            }
            else if (m_velocity.x < 0) {
                m_position.x += x_overlap;
                m_velocity.x = 0;
                m_collided_left = true;
            }
        }
    }
}

/*
* Check for collisions with the map in the x-axis
*
* @param map, MAP object that the ENTITY object is colliding with
*/
void const Entity::check_collision_x(Map* map)
{
    // Check if touching tile
    glm::vec3 left = glm::vec3(m_position.x - (m_width / 2), m_position.y, m_position.z);
    glm::vec3 right = glm::vec3(m_position.x + (m_width / 2), m_position.y, m_position.z);

    // Check if touching wall
    glm::vec3 left_wall = left - glm::vec3(m_wallcheck_offset, 0.0f, 0.0f);
    glm::vec3 right_wall = right + glm::vec3(m_wallcheck_offset, 0.0f, 0.0f);

    float penetration_x = 0;
    float penetration_y = 0;

    // entity collision checks
    if (map->is_solid(left, &penetration_x, &penetration_y) && m_velocity.x < 0)
    {
        //std::cout << "hit: " << m_entity_type << std::endl;
        m_position.x += penetration_x;
        m_velocity.x = 0;
        m_collided_left = true;
    }
    if (map->is_solid(right, &penetration_x, &penetration_y) && m_velocity.x > 0)
    {
        m_position.x -= penetration_x;
        m_velocity.x = 0;
        m_collided_right = true;
    }

    if (map->is_solid(left_wall, &penetration_x, &penetration_y)) m_wallcheck_left = true;
    if (map->is_solid(right_wall, &penetration_x, &penetration_y)) m_wallcheck_right = true;
}

/*
* Render function specifically for the ENTITY class
*
* @param program, reference to the SHADERPROGRAM class -- to use it's functions
*/
void Entity::render(ShaderProgram* program)
{
    program->set_model_matrix(m_model_matrix);

    // if not active -- then can't render, treat like deletion
    if (!m_is_active || !m_is_rendered) { return; }

    float vertices[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
    float tex_coords[] = { 0.0,  1.0, 1.0,  1.0, 1.0, 0.0,  0.0,  1.0, 1.0, 0.0,  0.0, 0.0 };

    glBindTexture(GL_TEXTURE_2D, m_texture_id);

    glVertexAttribPointer(program->get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->get_position_attribute());
    glVertexAttribPointer(program->get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, tex_coords);
    glEnableVertexAttribArray(program->get_tex_coordinate_attribute());

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(program->get_position_attribute());
    glDisableVertexAttribArray(program->get_tex_coordinate_attribute());
}

/*
* General check collision function for static object
*
* @param other, the other ENTITY class that is being collided with
*
* TREAT LIKE NORMAL PHYSICS COLLISION -- STOPS OBJECTS
*/
bool const Entity::check_collision(Entity* other) const
{
    if (other == this) return false;
    // If either entity is inactive, there shouldn't be any collision
    if (!m_is_active || !other->m_is_active) return false;

    float x_distance = fabs(m_position.x - other->m_position.x) - ((m_width + other->m_width) / 2.0f);
    float y_distance = fabs(m_position.y - other->m_position.y) - ((m_height + other->m_height) / 2.0f);

    return x_distance < 0.0f && y_distance < 0.0f;
}

void Entity::chain_activate(Entity* player, float delta_time)
{
    float chain_offset = 1.0f;
    m_movement = glm::vec3(0.0f);
    switch (chain_state)
    {
    case(LAUNCH):

        switch (chain_direction)
        {
        case (LEFT):
            set_position(player->get_position() - glm::vec3(chain_offset, 0.0f, 0.0f));
            break;
        case (RIGHT):
            set_position(player->get_position() + glm::vec3(chain_offset, 0.0f, 0.0f));
            break;
        case (UP):
            set_position(player->get_position() + glm::vec3(0.0f, chain_offset, 0.0f));
            break;
        case (DOWN):
            set_position(player->get_position() - glm::vec3(0.0f, chain_offset, 0.0f));
            break;
        default:
            break;
        }

        chain_timer = 1.0f;
        chain_state = SEARCHING;
        break;

    case(SEARCHING):
        switch (chain_direction)
        {
        case (LEFT):
            move_left();
            break;
        case (RIGHT):
            move_right();
            break;
        case (UP):
            move_up();
            break;
        case (DOWN):
            move_down();
            break;
        default:
            break;
        }

        if (m_collided_right || m_collided_left || m_collided_top || m_collided_bottom)
        {
            player->chain_timer = 1.0f;
            chain_state = STICK;
        }

        chain_timer -= delta_time;
        if (chain_timer <= 0.0f) chain_state = RETRACT;

        break;

    case(STICK):
        player->chain_timer -= delta_time;
        if (player->chain_timer <= 0.0f)
        {
            chain_state = RETRACT;
        }
        break;

    case(RETRACT):
        chain_timer = 1.0f;
        m_is_active = false;
        break;

    default:
        m_is_active = false;
        break;
    }
}

void Entity::move_to_target(const glm::vec3& target_position)
{
    if (m_position == target_position) return;
    if (target_position.x < m_position.x) move_left();
    else if (target_position.x > m_position.x) move_right();
    if (target_position.y < m_position.y) move_down();
    else if (target_position.y > m_position.y) move_up();
}