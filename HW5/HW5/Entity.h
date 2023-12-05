#pragma once

enum EntityType { PLAYER, CHAIN, DOOR, ENEMY };
enum ChainState { LAUNCH, SEARCHING, STICK, RETRACT };
enum ChainDirection { LEFT, RIGHT, UP, DOWN };
enum AIState { IDLE, PATROLING, CHASING };
enum AIType { PATROL };

#include "Map.h"

class Entity {
private:
    // position and tranformation variables
    glm::vec3 m_position;
    glm::mat4 m_model_matrix;

    // physics variables
    glm::vec3 m_velocity;
    glm::vec3 m_acceleration;
    glm::vec3 m_movement;

    // specific to different entity types
    float m_width = 1.0f;
    float m_height = 1.0f;
    float m_speed = 0.0f;
    float m_jumping_power = 0.0f;

    // ENEMY AI
    AIType     m_ai_type;
    AIState    m_ai_state;

    EntityType m_entity_type; // type of entity - treat as NAME

    bool m_is_active = true; // objects that are not active -- basically deleted
    bool m_is_rendered = true; // objects that are not rendered are still active

public:
    GLuint m_texture_id; // texture

    // physics - gravity
    bool m_has_gravity = false;

    // physics - collision for all directions
    bool m_collided_top = false;
    bool m_collided_bottom = false;
    bool m_collided_left = false;
    bool m_collided_right = false;

    // physics - wall collision
    bool m_wallcheck_left = false;
    bool m_wallcheck_right = false;
    float m_wallcheck_offset = 0.15f;

    bool is_facing_right = true;
    float ability_timer = 2.0f;

    // physics - jumping 
    bool  m_is_jumping = false;
    bool m_is_wall_jumping = false;

    // chain
    ChainDirection chain_direction = RIGHT;
    ChainState chain_state = LAUNCH;
    float chain_timer = 0.0f;

    // door
    bool level_finished = false;

    // enemy
    float guard_timer = 2.0f;
    bool touching_player = false;

    // default constructor
    Entity();

    void update(float delta_time, Entity* player, Entity* objects, int object_count, Map* map);
    void render(ShaderProgram* program);

    // collisions - both in the x and y axis
    bool const check_collision(Entity* other) const;
    void const check_collision_y(Entity* collidable_entities, int collidable_entity_count);
    void const check_collision_y(Map* map);
    void const check_collision_x(Entity* collidable_entities, int collidable_entity_count);
    void const check_collision_x(Map* map);

    void activate() { m_is_active = true; };
    void deactivate() { m_is_active = false; };

    void chain_activate(Entity* player, float delta_time);
    void move_to_target(const glm::vec3& target_position);

    // ai scripts
    void ai_activate(Entity* player, float delta_time);
    void ai_patrol(Entity* player, float delta_time);

    // movement
    void move_left() { m_movement.x = -1.0f; };
    void move_right() { m_movement.x = 1.0f; };
    void move_up() { m_movement.y = 1.0f; };
    void move_down() { m_movement.y = -1.0f; };

    // GETTERS
    EntityType const get_entity_type()    const { return m_entity_type; };
    glm::vec3  const get_position()       const { return m_position; };
    glm::vec3  const get_movement()       const { return m_movement; };
    glm::vec3  const get_velocity()       const { return m_velocity; };
    glm::vec3  const get_acceleration()   const { return m_acceleration; };
    float        const get_width()          const { return m_width; };
    float        const get_height()         const { return m_height; };
    float      const get_speed()          const { return m_speed; };
    float      const get_jumping_power()  const { return m_jumping_power; };
    bool       const get_active_state()   const { return m_is_active; };
    AIType     const get_ai_type()        const { return m_ai_type; };
    AIState    const get_ai_state()       const { return m_ai_state; };

    // SETTLERS
    void const set_entity_type(EntityType new_entity_type) { m_entity_type = new_entity_type; };
    void const set_position(glm::vec3 new_position) { m_position = new_position; };
    void const set_movement(glm::vec3 new_movement) { m_movement = new_movement; };
    void const set_velocity(glm::vec3 new_velocity) { m_velocity = new_velocity; };
    void const set_acceleration(glm::vec3 new_acceleration) { m_acceleration = new_acceleration; };
    void const set_width(float new_width) { m_width = new_width; };
    void const set_height(float new_height) { m_height = new_height; };
    void const set_speed(float new_speed) { m_speed = new_speed; };
    void const set_jumping_power(float new_jumping_power) { m_jumping_power = new_jumping_power; };
    void const set_ai_type(AIType new_ai_type) { m_ai_type = new_ai_type; };
    void const set_ai_state(AIState new_state) { m_ai_state = new_state; };

    void const disable() { m_is_active = false; };
    void const enable() { m_is_active = true; };
};