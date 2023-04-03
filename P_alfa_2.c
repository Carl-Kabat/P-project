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
    float V_length; // vector length
    
    float V_angle;
};

struct atom_data
{
    float x;
    float y;
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
            (atom + 101*row+column) -> x = 10*column;
            (atom + 101*row+column) -> y = 10*row;
            (atom + 101*row+column) -> radius = 4;
        }
    }
}


struct electron_data spawn_electron()
{
    struct electron_data electron;
    srand(time(NULL));
    
    // Electron's position
    electron.x = 0.0;
    electron.y = rand()%21+40;

    // Electron's velocity
    electron.Vx = 1;
    electron.Vy = rand()%3+1 * (1.2);

    // Angle of travel
    electron.V_angle = atan2(electron.Vy, electron.Vx);
    
    return electron;
}


struct electron_data one_cycle(struct electron_data E, struct atom_data * atom, float acceleration)
{
    // Shift the position
    E.Vx += 0.001 * acceleration;
    E.x = E.x + E.Vx;
    E.y = E.y + E.Vy;

    // Check if in bounds
    if(E.y >= 100)
    {
        E.y = 99;
        E.Vy = -E.Vy;
    }
    if(E.y <= 0)
    {
        E.y = 1;
        E.Vy = -E.Vy;
    }
    
    E.V_length = sqrt(E.Vx*E.Vx + E.Vy*E.Vy);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    //                                    COLLISION HANDLER                                       //
    ////////////////////////////////////////////////////////////////////////////////////////////////
    int i;
    for(i=0; i<1110; i++)
    {
        // Vector from E to atom
        struct vector_data E_to_atom_vector;
        E_to_atom_vector.x = (atom+i)->x - E.x;
        E_to_atom_vector.y = (atom+i)->y - E.y;
        E_to_atom_vector.length = sqrt(E_to_atom_vector.x*E_to_atom_vector.x + E_to_atom_vector.y*E_to_atom_vector.y);

        if (E_to_atom_vector.length <= (atom+i)->radius) // We have a collision bois
        {
            printf("\n\nAŁA KURWA\n\n");

            // Line equation dictated by the velocity vector
            float x1 = E.x;
            float y1 = E.y;
            float x2 = x1 + E.Vx;
            float y2 = y1 + E.Vy;
            // Ay + Bx + C = 0
            float A = y2 - y1;
            float B = x1 - x2;
            float C = y1*x2 - y2+x1;

            // Distance from the atom to calculated velocity line
            struct vector_data atom_to_vel;
            float slope = (-1.0) / E.Vy / E.Vx; // Perpendicular slope
            float alfa = atan(slope);
            atom_to_vel.length = fabs(A*(atom+i)->x + B*(atom+i)->y + C) / sqrt(A*A + B*B);
            atom_to_vel.x = atom_to_vel.length * cos(alfa);
            atom_to_vel.y = atom_to_vel.length * sin(alfa);

            // Closest point on the velocity line to the atom's center
            struct atom_data point_v;
            point_v.x = (atom+i)->x + atom_to_vel.x;
            point_v.y = (atom+i)->y + atom_to_vel.y;

            // Distances between many points in intersecting electron & atom (you won't understand without a drawing i made)
            float e = sqrt(E_to_atom_vector.length*E_to_atom_vector.length - atom_to_vel.length*atom_to_vel.length); // from v to E
            float f = sqrt((atom+i)->radius*(atom+i)->radius - atom_to_vel.length*atom_to_vel.length); // from v to collision point

            // Vector from point of tangency (between atom and velocity line) to Electron.
            // If direction same as velocity vector: e+f. If not: e-f.
            struct vector_data compatibility_vector;
            compatibility_vector.x = E.x - point_v.x;
            compatibility_vector.y = E.y - point_v.y;
            // Scalar product to check vectors' directions
            float scalar_product = compatibility_vector.x * E.Vx + compatibility_vector.y * E.Vy;

            //                                                      //
            //                  POSITION CORRECTION                 //      about time
            //                                                      //
            struct vector_data REVERSE_VECTOR;
            if (scalar_product > 0)
            {
                REVERSE_VECTOR.length = f + e;
            }
            if (scalar_product < 0)
            {
                REVERSE_VECTOR.length = f - e;
            }
            // Vector correcting electron's position (It's just velocity vector reversed and scaled)
            float multiplier = REVERSE_VECTOR.length / E.V_length;
            REVERSE_VECTOR.x = E.Vx * multiplier;
            REVERSE_VECTOR.y = E.Vy * multiplier;
            // Correct the position
            E.x += REVERSE_VECTOR.x;
            E.y += REVERSE_VECTOR.y;

            // Angle of reflection  [is this part bad]
            float theta = atan2(E_to_atom_vector.y, E_to_atom_vector.x);
            float phi = 2 * theta - E.V_angle;
            
            // Update angle of travel
            E.V_angle = phi;
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
    while(electron.x < 1000)
    {
        electron = one_cycle(electron, atom, acceleration);
        fprintf(file, "%f %f\n", electron.x, electron.y);
    }

    ////////////////////////////////////////////////////////////////

    fclose(file);
    return 0;
}
