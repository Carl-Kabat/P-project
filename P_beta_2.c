#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

struct atom_data
{
    float x;
    float y;
};

struct electron_data
{
    float x;
    float y;

    float Vx;
    float Vy;
    float V_length; // velocity vector length
    float V_angle;  // angle of travel
};

struct vector_data
{
    float x;
    float y;
    float length;
    float slope;
};

// Siatka 1111 atomow, start (0,0), odstep co 10 jednostek, do (1000x, 100y)
void spawn_atoms(struct atom_data * atom)
{
    int column, row;
    for(row=0; row <= 10; row++)
    {
        for(column=0; column <= 100; column++)
        {
            (atom + row*101 + column) -> x = 10*column;
            (atom + row*101 + column) -> y = 10*row;
        }
    }
}

// Creates electron
struct electron_data spawn_electron (struct electron_data electron)
{
    srand(time(NULL));
    
    electron.x = 0;
    electron.y = rand()%21+40;

    electron.Vx = 1;
    electron.Vy = rand()%3+1 * (1.2);
    electron.V_length = sqrt(electron.Vx*electron.Vx + electron.Vy*electron.Vy);
    electron.V_angle = atan2(electron.Vy, electron.Vx);
    
    return electron;
}

struct electron_data collision_response (struct electron_data E,
                                         struct vector_data E_to_atom,
                                         struct atom_data * atom,
                                         char radius,
                                         int target_atom)
{
    struct vector_data atom_to_vel;
    // To correct electron's possition currently located inside of atom:
    // We want a closest point (P) from atom's center to velocity line
    // We'll achieve this by shifting the atom's center
    // by a vector perpendicular to the velocity line

    // We want their distance for pythagoras theorem
    // to calculate the distance by which the electron
    // needs to be shifted backwards

    // Velocity line
    // y = ax + b, where            // Ax + By + C = 0, where
    // a = E.Vy / E.Vx              // A = -a = -E.Vy / E.Vx
    // b = E.y - a * E.x            // B = 1    (just +y)
                                    // C = -b = a * Ex - E.y

    // Distance between atom & electron's velocity line
    atom_to_vel.length = (
                          fabsf((-E.Vy / E.Vx)*E.x + E.y + (E.Vy / E.Vx)*E.x - E.y)
                                / sqrt((E.Vy/E.Vx) * (E.Vy/E.Vx) + 1)
                         );
    // Slope (of the distance vector) is perpendicular to velocity's line
    atom_to_vel.slope = (-1.0) / E.Vy / E.Vx;
    float alfa = atan(atom_to_vel.slope);
    printf("\n\nalfa = %f\n\n", alfa);

    atom_to_vel.x = atom_to_vel.length * cos(alfa);
    atom_to_vel.y = atom_to_vel.length * sin(alfa);

    // Point P
    struct atom_data P;
    P.x = atom[target_atom].x + atom_to_vel.x;
    P.y = atom[target_atom].y + atom_to_vel.y;

    // Using Pythagoras theorem:
    // Distance from point P to Electron (k):
    float k = sqrt(E_to_atom.length*E_to_atom.length - atom_to_vel.length*atom_to_vel.length);
    // Distance from point P to desired Electron position (f) (may include 'k')
    float f = sqrt(radius*radius - atom_to_vel.length*atom_to_vel.length);

    // Next, we need to check whether the distance by which Electron needs to be moved back
    // is f+k or f-k
    // We will do this by checking the value of scalar product
    // of our original velocity vector and a vector pointing from point P to Electron.
    // If they're the same direction, we need to f+k, if opposite: f-e
    struct vector_data compatibility_vector;
            compatibility_vector.x = E.x - P.x;
            compatibility_vector.y = E.y - P.y;
            // Scalar product to check vectors' directions
            float scalar_product = compatibility_vector.x * E.x + compatibility_vector.y * E.y;

    struct vector_data REVERSE_VECTOR;
            if (scalar_product > 0)
            {
                REVERSE_VECTOR.length = f + k;
            }
            if (scalar_product < 0)
            {
                REVERSE_VECTOR.length = f - k;
            }
    // Our reverting vector is just the velocity vector, reversed & multiplied
    float multiplier = REVERSE_VECTOR.length / E.V_length;
            REVERSE_VECTOR.x = E.Vx * multiplier;
            REVERSE_VECTOR.y = E.Vy * multiplier;
            // Correcting the electron's possition
            E.x += REVERSE_VECTOR.x;
            E.x += REVERSE_VECTOR.y;

    // (This part is unverified)
            float theta = atan2(E_to_atom.y, E_to_atom.x);
            float phi = 2 * theta - E.V_angle;
            
            // Update angle of travel
            E.V_angle = phi;

    return E;
}

struct electron_data collision_check (struct electron_data E, struct atom_data * atom, char radius, int x_overlap_pos)
{
    struct vector_data E_to_atom;
    int row;

    for(row=0; row <= 10; row++)
    {
        // index 'row*101 + x_overlap_pos' tells us in which row the collision happend,
        // which gives us the exact index of the atom in the 1111 array
        E_to_atom.x = (atom + row*101 + x_overlap_pos)->x - E.x;
        E_to_atom.y = (atom + row*101 + x_overlap_pos)->y - E.y;

        if (sqrt(E_to_atom.x*E_to_atom.x + E_to_atom.y*E_to_atom.y) <= radius)
        {
            printf("\nJEB!\n\n");
            E = collision_response(E, E_to_atom, atom, radius, row*101 + x_overlap_pos);
        }
    }
    return E;
}

struct electron_data one_cycle (struct electron_data E, struct atom_data * atom, char radius, float acceleration)
{
    // Check if in bounds
    if(E.y >= 100)
    {
        E.y = 97;
        E.Vy = -E.Vy;
    }
    if(E.y <= 0)
    {
        E.y = 3;
        E.Vy = -E.Vy;
    }

    // Shift the position
    E.Vx += 0.001 * acceleration;
    E.x = E.x + E.Vx;
    E.y = E.y + E.Vy;

    // Check vertical overlap between electron and possible atoms
    int shift_x;
    for (shift_x = 0; shift_x <= 1000; shift_x += 10)
    {
        // Check if electron inside atoms (on X axis)
        if (E.x <= (0+radius)+shift_x 
         && E.x >= (0-radius)+shift_x)
        {
            E = collision_check(E, atom, radius, 0.1 * shift_x);
            break;
        }
    }
    return E;
}

int main()
{
    char acceleration = 1;
    struct atom_data atoms[1111];
    const char atom_radius = 2;
    spawn_atoms(atoms);

    struct electron_data electron;
    electron = spawn_electron(electron);
    electron = one_cycle(electron, atoms, atom_radius, acceleration);
    
    
    return 0;
}
