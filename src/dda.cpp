#include <glm/glm.hpp>
#include "map.hpp"

// structs
typedef struct {
        int *map_grid[MAP_GRID_LEN];
        glm::fvec2 *point_direction;
        glm::fvec2 *point_start;
        float dda_walk_max = 1000.0;
} DDA_init;

typedef struct {
        float ray_total = 0.0;
        bool ray_bound = false;
        glm::fvec2 delta;
        glm::fvec2 unitary_vector;
        glm::fvec2 unitary_step_size;
        glm::fvec2 ray_length_1D;
        glm::fvec2 step;
        glm::ivec2 tile_map_check;
} DDA_walk_internal;

typedef struct {
        glm::fvec2 collision_point; // ponto da collision
        int map_grid_index; // index do grid da collision
        enum { RIGTH, LEFT } side; // qual lado do grid foi a collision
} DDA_collision;

// declarations
static void init_walk_steps_on_x();
static void init_walk_steps_on_y();
static DDA_walk_internal DDA_init_walk_internal();
static void walk_on_x();
static void walk_on_y();
static bool check_map_collision();
bool check_map_bound_index(int index_x, int index_y, int index_max_x, int index_max_y);
DDA_collision DDA(DDA_init *);

// main
DDA_collision DDA(DDA_init)
{
        float ray_total = 0.0;
        bool ray_bound = false;

        glm::fvec2 delta(point_direction->x / App->map_height - point_start.x / App->map_height,
                         point_direction->y / App->map_height - point_start.y / App->map_height);
        glm::fvec2 unitary_vector(delta.x / glm::length(delta), delta.y / glm::length(delta));
        glm::fvec2 unitary_step_size(sqrtf(1 + powf(unitary_vector.y / unitary_vector.x, 2)),
                                     sqrtf(1 + powf(unitary_vector.x / unitary_vector.y, 2)));
        glm::fvec2 ray_length_1D, step;
        glm::ivec2 tile_map_check(point_start.x, point_start.y);
        glm::ivec2 absolute_map_tile;

        /* set x walk */
        if (unitary_vector.x < 0) {
                step.x = -1;
                ray_length_1D.x = (point_start.x - tile_map_check.x) * unitary_step_size.x;
        } else {
                step.x = 1;
                ray_length_1D.x = ((tile_map_check.x + 1) - point_start.x) * unitary_step_size.x;
        }

        /* set y walk */
        if (unitary_vector.y < 0) {
                step.y = -1;
                ray_length_1D.y = (point_start.y - tile_map_check.y) * unitary_step_size.y;
        } else {
                step.y = 1;
                ray_length_1D.y = ((tile_map_check.y + 1) - point_start.y) * unitary_step_size.y;
        }

        // init_walk_steps_on_x();
        // init_walk_steps_on_y()

        while (!ray_bound && ray_total < ray_total_max) {
                /* walk */
                if (ray_length_1D.x < ray_length_1D.y) {
                        // walk_on_x();

                        tile_map_check.x += step.x;
                        ray_total = ray_length_1D.x;
                        ray_length_1D.x += unitary_step_size.x;
                        *side = 0; /* vertical */
                } else {
                        // walk_on_y();
                        tile_map_check.y += step.y;
                        ray_total = ray_length_1D.y;
                        ray_length_1D.y += unitary_step_size.y;
                        *side = 1; /* horizontal */
                }

                /* check */
                absolute_map_tile.x = tile_map_check.x / App->map_height;
                absolute_map_tile.y = tile_map_check.y / App->map_height;
                if (check_map_bound_index(absolute_map_tile.x, absolute_map_tile.y, App->map_cols, App->map_rows) ==
                            true &&
                    App->map_tile[absolute_map_tile.y][absolute_map_tile.x] != 0) {
                        ray_bound = true;
                        *map_index_value = App->map_tile[absolute_map_tile.y][absolute_map_tile.x];
                }
        }

        if (ray_bound == true) {
                point_collision_map->x = point_start.x + unitary_vector.x * ray_total;
                point_collision_map->y = point_start.y + unitary_vector.y * ray_total;
        }
}

// implementation
DDA_walk_internal DDA_init_walk_internal()
{
        float ray_total = 0.0;
        bool ray_bound = false;

        glm::fvec2 delta(point_direction->x / App->map_height - point_start.x / App->map_height,
                         point_direction->y / App->map_height - point_start.y / App->map_height);
        glm::fvec2 unitary_vector(delta.x / glm::length(delta), delta.y / glm::length(delta));
        glm::fvec2 unitary_step_size(sqrtf(1 + powf(unitary_vector.y / unitary_vector.x, 2)),
                                     sqrtf(1 + powf(unitary_vector.x / unitary_vector.y, 2)));
        glm::fvec2 ray_length_1D, step;
        glm::ivec2 tile_map_check(point_start.x, point_start.y);
        glm::ivec2 absolute_map_tile;
}

void init_walk_steps_on_x()
{
        if (unitary_vector.x < 0) {
                step.x = -1;
                ray_length_1D.x = (point_start.x - tile_map_check.x) * unitary_step_size.x;
        } else {
                step.x = 1;
                ray_length_1D.x = ((tile_map_check.x + 1) - point_start.x) * unitary_step_size.x;
        }
}
void init_walk_steps_on_y()
{
        if (unitary_vector.y < 0) {
                step.y = -1;
                ray_length_1D.y = (point_start.y - tile_map_check.y) * unitary_step_size.y;
        } else {
                step.y = 1;
                ray_length_1D.y = ((tile_map_check.y + 1) - point_start.y) * unitary_step_size.y;
        }
}
bool check_map_bound_index(int index_x, int index_y, int index_max_x, int index_max_y)
{
        if (index_x >= 0 && index_x < index_max_x && index_y >= 0 && index_y < index_max_y)
                return true;
        else
                return false;
}