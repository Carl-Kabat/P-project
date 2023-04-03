#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

struct electron_data
{
    float x;
    float y;

    float Vx;
    float Vy;
    float V_length; // velocity vector length
    float V_angle;  // angle of travel
};

// Siatka 1111 atomow, start (0,0), odstep co 10 jednostek do (100y, 1000x)
void spawn_atoms (int atom_net[11][101])
{
    char i,j;
    for (i=0; i<11; i++)
    {
        for (j=0; j<101; j++)
        {
            atom_net[i][j] = 10*j;
        }
    }
}

// Tworzy elektron
struct electron_data spawn_electron (struct electron_data electron)
{
    srand(time(NULL));
    
    electron.x = 0.0;
    electron.y = rand()%21+40;

    electron.Vx = 1;
    electron.Vy = rand()%3+1 * (1.2);
    electron.V_length = sqrt(electron.Vx*electron.Vx + electron.Vy*electron.Vy);
    electron.V_angle = atan2(electron.Vy, electron.Vx);
    
    return electron;
}

struct electron_data collision_check (struct electron_data E, char row_section, char radius)
{
    return E;
}

struct electron_data Y_axis_overlap_check (struct electron_data E, char radius)
{
    char shift_y = 0;
    char row_section = 1;
    for (; shift_y <= 60; shift_y += 20, row_section++)
    {
        if (E.y <= 12+shift_y)
        {
            collision_check(E, row_section, radius);

            break;
        }
        if (shift_y == 60)
        {
            printf("\n\nChecking collisions for last row section\n\n");
            row_section = 5;
            E = collision_check(E, row_section, radius);
        }
    }
}

struct electron_data one_cycle (struct electron_data E, char radius, float acceleration)
{
    // Shift the position
    E.Vx += 0.001 * acceleration;
    E.x = E.x + E.Vx;
    E.y = E.y + E.Vy;

    // Check if in bounds
    if (E.y >= 100)
    {
        E.y = 99;
        E.Vy = -E.Vy;
    }
    if (E.y <= 0)
    {
        E.y = 1;
        E.Vy = -E.Vy;
    }

    int shift_x;
    for (shift_x = 0; shift_x <= 1000; shift_x += 10)
    {
        if (E.x <= (0+radius)+shift_x && E.x >= (0-radius)+shift_x)
        {
            E = Y_axis_overlap_check(E, radius);
        }
    }
}

int main ()
{
    char acceleration = 1;
    int atoms[11][101];
    const char atom_radius = 2;
    spawn_atoms(atoms);

    struct electron_data electron;
    electron = spawn_electron(electron);
    electron = one_cycle(electron, atom_radius, acceleration);
    
    
    return 0;
}