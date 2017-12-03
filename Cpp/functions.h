#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <math.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <time.h>

typedef struct typeV3 {
  float x, y, z;
} v3;

typedef struct typeColor {
  float r, g, b;
} color;

enum shadeMode {
  SHADED,
  SHADELESS
};

typedef struct typeBlock {

  GLuint blockID; // VBO
  GLuint colorID; // CBO
  v3 *verts; // vertex array

  v3 size;
  v3 pos;
  v3 speed;

  glm::mat4 modelMat;

  color blockColor;
  shadeMode colorMode;

  double mass;

} block;

v3 getMin(block object);
v3 getMax(block object);
float magnitude(v3 a);
v3 crossProduct(v3 a, v3 b);

void createBlockVBO(block *object);
bool colliding(block a, block b);

double randRange(double a, double b);
