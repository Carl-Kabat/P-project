#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>


struct electron_data
{
    float coordinate_x;
    float coordinate_y;
    float x_velocity;
    float y_velocity;
    float velocity_length; // vector length
    float angle; // angle of travel
};

struct atom_data
{
    float coordinate_x;
    float coordinate_y;
    float radius;
};

struct vector_data
{
    float x;
    float y;
    float length;
};


// Creates 11 rows and 101 columns of evenly spaced atoms (every 10 units) starting at (0,0). 1111 atoms total
void create_atom_net(struct atom_data * atom)
{
    int column, row;
    for(row=0; row <= 10; row++)
    {
        for(column=0; column <= 100; column++)
        {
            (atom + 101*row+column) -> coordinate_x = 10*column;
            (atom + 101*row+column) -> coordinate_y = 10*row;
            (atom + 101*row+column) -> radius = 4; // You can provide your own
        }
    }
}


struct electron_data spawn_electron()
{
    struct electron_data electron;
    srand(time(NULL));
    
    electron.coordinate_x = 0.0;
    electron.coordinate_y = rand()%21+40;

    electron.x_velocity = 0;
    electron.y_velocity = rand()%6-10;

    // Angle of travel
    electron.angle = atan2(electron.y_velocity, electron.x_velocity);
    
    return electron;
}


struct electron_data one_cycle(struct electron_data E, struct atom_data * atom, float acceleration)
{
    // Get shift value
    float dx = cos(E.angle);
    float dy = sin(E.angle);

    // Shift the position
    E.x_velocity += 0.001 * acceleration;
    E.coordinate_x = E.coordinate_x + dx + E.x_velocity;
    E.coordinate_y = E.coordinate_y + dy;

    // Check if in bounds
    if(E.coordinate_y >= 100)
    {
        E.coordinate_y = 99;
        E.y_velocity = -E.y_velocity;
    }
    if(E.coordinate_y <= 0)
    {
        E.coordinate_y = 1;
        E.y_velocity = -E.y_velocity;
    }

    E.velocity_length = sqrt(E.x_velocity*E.x_velocity + E.y_velocity*E.y_velocity);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // COLLISION HANDLER
    int i;
    for(i=0; i<1110; i++)
    {
        // Vector from E to atom
        struct vector_data E_to_atom_vector;
        E_to_atom_vector.x = (atom+i)->coordinate_x - E.coordinate_x;
        E_to_atom_vector.y = (atom+i)->coordinate_y - E.coordinate_y;
        E_to_atom_vector.length = sqrt(E_to_atom_vector.x*E_to_atom_vector.x + E_to_atom_vector.y*E_to_atom_vector.y);

        if (E_to_atom_vector.length <= (atom+i)->radius) // We have a collision bois
        {
            // Line equation dictated by the velocity vector
            float x1 = E.coordinate_x;
            float y1 = E.coordinate_y;
            float x2 = x1 + E.x_velocity;
            float y2 = y1 + E.y_velocity;
            // Ay + Bx + C = 0
            float A = y2 - y1;
            float B = x1 - x2;
            float C = y1*x2 - y2+x1;

            // Distance from the atom to calculated velocity line
            struct vector_data atom_to_velocity_vector;
            float slope = (-1.0) / E.y_velocity / E.x_velocity; // Perpendicular slope
            float alfa = atan(slope);
            // (Tangent) Distance from point to a line formula
            atom_to_velocity_vector.length = fabsf(A*atom->coordinate_x + B*atom->coordinate_y + C) / sqrt(A*A + B*B);
            atom_to_velocity_vector.x = atom_to_velocity_vector.length * cos(alfa);
            atom_to_velocity_vector.y = atom_to_velocity_vector.length * sin(alfa);

            // Closest point on the velocity line to the atom's center
            struct atom_data point_v;
            point_v.coordinate_x = atom->coordinate_x + atom_to_velocity_vector.x;
            point_v.coordinate_y = atom->coordinate_y + atom_to_velocity_vector.y;

            // Distances between many points in intersecting electron & atom (you won't understand without a drawing i made)
            float e = sqrt(E_to_atom_vector.length*E_to_atom_vector.length - atom_to_velocity_vector.length*atom_to_velocity_vector.length); // from point v to E
            float f = sqrt(atom->radius*atom->radius - atom_to_velocity_vector.length*atom_to_velocity_vector.length); // from v to collision point

            // Vector from point of tangency (between atom and velocity line) to Electron.
            // If direction same as velocity vector: e+f. If not: e-f.
            struct vector_data compatibility_vector;
            compatibility_vector.x = E.coordinate_x - point_v.coordinate_x;
            compatibility_vector.y = E.coordinate_y - point_v.coordinate_y;
            // Scalar product to check vectors' directions
            float scalar_product = compatibility_vector.x * E.x_velocity + compatibility_vector.y * E.y_velocity;

            //                              //
            //      POSITION CORRECTION     //      about time
            //                              //
            struct vector_data REVERSE_VECTOR;
            if (scalar_product > 0)
            {
                REVERSE_VECTOR.length = f + e;
            }
            if (scalar_product < 0)
            {
                REVERSE_VECTOR.length = f - e;
            }
            // A vector correcting electron's position (It's just velocity vector reversed & scaled)
            float multiplier = REVERSE_VECTOR.length / E.velocity_length;
            REVERSE_VECTOR.x = E.x_velocity * multiplier;
            REVERSE_VECTOR.y = E.y_velocity * multiplier;
            // Correct the position
            E.coordinate_x += REVERSE_VECTOR.x;
            E.coordinate_y += REVERSE_VECTOR.y;

            // Angle of reflection  [this part might be wrong]
            double theta = atan2(E_to_atom_vector.y, E_to_atom_vector.x);
            double phi = 2 * theta - E.angle;
            
            // Update angle of travel
            E.angle = phi;
        }
    }
    // Bye bye, electron
    return E;
}


/////////////////////////////// MAIN ///////////////////////////////
int main()
{
    float avg_velocity, acceleration;
    struct electron_data electron;
    struct atom_data atom[1200];
    create_atom_net(atom);

    ////////////////////////////////////////////////////////////////

    FILE *file = NULL;
    file = fopen("diagram.txt", "w+");
    if(file == NULL) {perror("\nERROR OCCURED >>"); return(13);}

    ////////////////////////////////////////////////////////////////

    printf("\nProgram simulates random movement of an electron in a piece of conductor (crystalic grid of atoms).");
    printf("\n\nInsert acceleration caused by Voltage.");
    printf("\n(Recommended 1 - 50)");
    printf("\na = "); scanf("%f", &acceleration);

    ////////////////////////////////////////////////////////////////

    electron = spawn_electron();
    while(electron.coordinate_x < 1000)
    {
        electron = one_cycle(electron, atom, acceleration);
        fprintf(file, "%f %f\n", electron.coordinate_x, electron.coordinate_y);
    }

    ////////////////////////////////////////////////////////////////

    fclose(file);
    return 0;
}
