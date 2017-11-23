//
//  Particles.h
//  tutorial01_first_window
//
//  Created by Guilherme Paiva on 21/11/2017.
//

#ifndef Particles_h
#define Particles_h

#include <math.h>
#include <random>
#include <chrono>

void addVectors(double angle1, double lenght1, double angle2, double lenght2, double result[]);
void defineParticle(int x, int y, int size, int mass);
void defineGravity(double gravity[]);
double rand(double a, double b);
void move();

typedef struct {
    int x;
    int y;
    int size;
    int color[3];
    int thickness;
    double speed;
    int drag;
    int mass;
    double angle;
} particle;

double *grav;
particle p;

double rand(double a, double b) {
    
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator(seed);
    std::uniform_real_distribution<double> distribution(a, b);
    
    return distribution(generator);
}

void defineGravity(double gravity[]) {
    
    double g = rand(0.0, 1.0);
    gravity[0] = M_PI;
    gravity[1] = g;
    
    grav = gravity;

    printf("Gravidade: %f\n", g * 10);
}

void defineParticle(int x, int y, int size, int mass) {
    p.x = x;
    p.y = y;
    p.size = size;
    p.color[0] = 0;
    p.color[1] = 0;
    p.color[2] = 255;
    p.thickness = 0;
    p.speed = 0;
    p.drag = 1;
    p.mass = mass;
    p.angle = 0;
}

void move() {
    
    double result[2];
    addVectors(p.angle, p.speed, grav[0], grav[1], result);
    p.angle = result[0];
    p.speed = result[1];
    
    p.x += sin(p.angle) * p.speed;
    p.y -= cos(p.angle) * p.speed;
    double mass_of_air = rand(0.0, 0.5);
    p.speed *= pow((p.mass / (p.mass + mass_of_air)), p.size);
}

void addVectors(double angle1, double lenght1, double angle2, double lenght2, double result[]) {
    // retorna a soma de dois vetores
    
    double x = sin(angle1) * lenght1 + sin(angle2) * lenght2;
    double y = cos(angle1) * lenght1 + cos(angle2) * lenght2;
    
    double angle = 0.5 * M_PI - atan2(y, x);
    double lenght = hypot(x, y);
    
    result[0] = angle;
    result[1] = lenght;
}

#endif /* Particles_h */
