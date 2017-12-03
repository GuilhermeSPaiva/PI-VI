#include "functions.h"
#include <random>
#include <chrono>

v3 getMin(block object) {
  return (v3){object.pos.x - object.size.x/2,
              object.pos.y - object.size.y/2,
              object.pos.z - object.size.z/2};
}
v3 getMax(block object) {
  return (v3){object.pos.x + object.size.x/2,
              object.pos.y + object.size.y/2,
              object.pos.z + object.size.z/2};
}

float magnitude(v3 a) {
  return sqrt((a.x * a.x) + (a.y * a.y) + (a.z * a.z));
}

v3 crossProduct(v3 a, v3 b) {
  return (v3){(a.y*b.z) - (a.z*b.y),
              (a.z*b.x) - (a.x*b.z),
              (a.x*b.y) - (a.y*b.x)};
}

void createBlockVBO(block *object) {
  v3 front_top_left = {-object->size.x/2,  object->size.y/2, object->size.z/2};
  v3 front_top_right = {object->size.x/2,  object->size.y/2, object->size.z/2};
  v3 front_bot_left = {-object->size.x/2,  -object->size.y/2, object->size.z/2};
  v3 front_bot_right = {object->size.x/2,  -object->size.y/2, object->size.z/2};
  v3 back_top_left =  {-object->size.x/2,  object->size.y/2, -object->size.z/2};
  v3 back_top_right =  {object->size.x/2,  object->size.y/2, -object->size.z/2};
  v3 back_bot_left =  {-object->size.x/2,  -object->size.y/2, -object->size.z/2};
  v3 back_bot_right =  {object->size.x/2,  -object->size.y/2, -object->size.z/2};

  // Create Vertex Buffer Data
  GLfloat g_vertex_buffer_data[] = {
    // back face
    back_top_left.x, back_top_left.y, back_top_left.z,
    back_top_right.x, back_top_right.y, back_top_right.z,
    back_bot_left.x, back_bot_left.y, back_bot_left.z,

    // bottom face
    back_bot_right.x, back_bot_right.y, back_bot_right.z,
    front_bot_right.x, front_bot_right.y, front_bot_right.z,

    // right face
    back_top_right.x, back_top_right.y, back_top_right.z,
    front_top_right.x, front_top_right.y, front_top_right.z,

    // top face
    back_top_left.x, back_top_left.y, back_top_left.z,
    front_top_left.x, front_top_left.y, front_top_left.z,

    // left face
    back_bot_left.x, back_bot_left.y, back_bot_left.z,
    front_bot_left.x, front_bot_left.y, front_bot_left.z,

    // front face
    front_bot_right.x, front_bot_right.y, front_bot_right.z,
    front_top_left.x, front_top_left.y, front_top_left.z,
    front_top_right.x, front_top_right.y, front_top_right.z

  };

  if(object->colorMode == SHADED) {
    v3 uvs[] = {
      // front UVs (0)
      (v3) {front_bot_right.x - front_bot_left.x,
            front_bot_right.y - front_bot_left.y,
            front_bot_right.z - front_bot_left.z},
      (v3) {front_top_right.x - front_bot_left.x,
            front_top_right.y - front_bot_left.y,
            front_top_right.z - front_bot_left.z},

      // right UVs (1)
      (v3) {back_bot_right.x - front_bot_right.x,
            back_bot_right.y - front_bot_right.y,
            back_bot_right.z - front_bot_right.z},
      (v3) {back_top_right.x - front_bot_right.x,
            back_top_right.y - front_bot_right.y,
            back_top_right.z - front_bot_right.z},

      // back UVs (2)
      (v3) {back_bot_left.x - back_bot_right.x,
            back_bot_left.y - back_bot_right.y,
            back_bot_left.z - back_bot_right.z},
      (v3) {back_top_left.x - back_bot_right.x,
            back_top_left.y - back_bot_right.y,
            back_top_left.z - back_bot_right.z},

      // left UVs (3)
      (v3) {front_bot_left.x - back_bot_left.x,
            front_bot_left.y - back_bot_left.y,
            front_bot_left.z - back_bot_left.z},
      (v3) {front_top_left.x - back_bot_left.x,
            front_top_left.y - back_bot_left.y,
            front_top_left.z - back_bot_left.z},

      // top UVs (4)
      (v3) {front_top_right.x - front_top_left.x,
            front_top_right.y - front_top_left.y,
            front_top_right.z - front_top_left.z},
      (v3) {back_top_right.x - front_top_left.x,
            back_top_right.y - front_top_left.y,
            back_top_right.z - front_top_left.z},

      // bottom UVs (5)
      (v3) {back_bot_right.x - back_bot_left.x,
            back_bot_right.y - back_bot_left.y,
            back_bot_right.z - back_bot_left.z},
      (v3) {front_bot_right.x - back_bot_left.x,
            front_bot_right.y - back_bot_left.y,
            front_bot_right.z - back_bot_left.z}
    };

    // puts("NORMALS:");
    v3 normals[6];
    for(int i = 0; i < 12; i+=2) {
      normals[i/2] = crossProduct(uvs[i], uvs[i+1]);
      normals[i/2].x /= magnitude(normals[i/2]);
      normals[i/2].y /= magnitude(normals[i/2]);
      normals[i/2].z /= magnitude(normals[i/2]);
      // printf("%f, %f, %f\n", normals[i/2].x, normals[i/2].y, normals[i/2].z);
    }

    GLfloat g_color_buffer_data[] = {

      // back color
      object->blockColor.r * normals[2].y, object->blockColor.g * normals[2].y, object->blockColor.b * normals[2].y,
      object->blockColor.r * normals[2].y, object->blockColor.g * normals[2].y, object->blockColor.b * normals[2].y,
      object->blockColor.r * normals[2].y, object->blockColor.g * normals[2].y, object->blockColor.b * normals[2].y,

      // bottom color
      object->blockColor.r * normals[5].y, object->blockColor.g * normals[5].y, object->blockColor.b * normals[5].y,
      object->blockColor.r * normals[5].y, object->blockColor.g * normals[5].y, object->blockColor.b * normals[5].y,

      // right color
      object->blockColor.r * normals[1].y, object->blockColor.g * normals[1].y, object->blockColor.b * normals[1].y,
      object->blockColor.r * normals[1].y, object->blockColor.g * normals[1].y, object->blockColor.b * normals[1].y,

      // top color
      object->blockColor.r * normals[4].y, object->blockColor.g * normals[4].y, object->blockColor.b * normals[4].y,
      object->blockColor.r * normals[4].y, object->blockColor.g * normals[4].y, object->blockColor.b * normals[4].y,

      // left color
      object->blockColor.r * normals[3].y, object->blockColor.g * normals[3].y, object->blockColor.b * normals[3].y,
      object->blockColor.r * normals[3].y, object->blockColor.g * normals[3].y, object->blockColor.b * normals[3].y,

      // front color
      object->blockColor.r * normals[0].y, object->blockColor.g * normals[0].y, object->blockColor.b * normals[0].y,
      object->blockColor.r * normals[0].y, object->blockColor.g * normals[0].y, object->blockColor.b * normals[0].y,
      object->blockColor.r * normals[0].y, object->blockColor.g * normals[0].y, object->blockColor.b * normals[0].y

    };

    // Create Color Buffer
  	glGenBuffers(1, &object->colorID);
  	glBindBuffer(GL_ARRAY_BUFFER, object->colorID);
  	glBufferData(GL_ARRAY_BUFFER, sizeof(g_color_buffer_data),
                 g_color_buffer_data, GL_STATIC_DRAW);

  } else {
    GLfloat g_color_buffer_data[] = {
      object->blockColor.r, object->blockColor.g, object->blockColor.b,
      object->blockColor.r, object->blockColor.g, object->blockColor.b,
      object->blockColor.r, object->blockColor.g, object->blockColor.b,
      object->blockColor.r, object->blockColor.g, object->blockColor.b,
      object->blockColor.r, object->blockColor.g, object->blockColor.b,
      object->blockColor.r, object->blockColor.g, object->blockColor.b,
      object->blockColor.r, object->blockColor.g, object->blockColor.b,
      object->blockColor.r, object->blockColor.g, object->blockColor.b,
      object->blockColor.r, object->blockColor.g, object->blockColor.b,
      object->blockColor.r, object->blockColor.g, object->blockColor.b,
      object->blockColor.r, object->blockColor.g, object->blockColor.b,
      object->blockColor.r, object->blockColor.g, object->blockColor.b,
      object->blockColor.r, object->blockColor.g, object->blockColor.b,
      object->blockColor.r, object->blockColor.g, object->blockColor.b
    };

    // Create Color Buffer
  	glGenBuffers(1, &object->colorID);
  	glBindBuffer(GL_ARRAY_BUFFER, object->colorID);
  	glBufferData(GL_ARRAY_BUFFER, sizeof(g_color_buffer_data),
                 g_color_buffer_data, GL_STATIC_DRAW);

  }

  // Create Vertex Buffer
  glGenBuffers(1, &object->blockID);
  glBindBuffer(GL_ARRAY_BUFFER, object->blockID);
  glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data),
               g_vertex_buffer_data, GL_STATIC_DRAW);

}

bool colliding(block a, block b) {
  v3 minA = getMin(a);
  v3 maxA = getMax(a);
  v3 minB = getMin(b);
  v3 maxB = getMax(b);

  bool result = !(maxA.x <= minB.x ||
                  minA.x >= maxB.x ||
                  maxA.y <= minB.y ||
                  minA.y >= maxB.y ||
                  maxA.z <= minB.z ||
                  minA.z >= maxB.z);

  // if(result) {
  //   puts("cagueeeei!!");
  //   printf("A POS: %f, %f, %f\nB POS: %f, %f, %f\n", a.pos.x, minA.y, a.pos.z,
  //                                                    b.pos.x, maxB.y, b.pos.z);
  // }

  return result;
}

double randRange(double a, double b) {
    
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator(seed);
    std::uniform_real_distribution<double> distribution(a, b);
    
    return distribution(generator);
}

// double randRange(double a, double b) {
//   srand(  (unsigned) time(NULL) );
//   double result = a + fmod(rand(), b - a);

//   return result;
// }
