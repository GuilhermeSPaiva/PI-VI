#include "functions.h"
#include "shader.hpp"
#include "objloader.hpp"
#include "texture.hpp"

#define WIDTH 1280
#define HEIGHT 720
#define GRAVITY_MAX 9.81f * 1.5f
#define AIRMASS_MAX 0.5f

GLFWwindow *window;

int main() {

  bool spacePressed = false;
  bool resetPressed = false;

  float pos = 0;

  double deltaTime = 0;
  double lastTime = 0;
  double currentTime = 0;

  double gravity = randRange(0.1f, GRAVITY_MAX);//9.81f;
  printf("GRAVITY: %lf m/s^2\n", gravity);
  double airMass = randRange(0.f, AIRMASS_MAX);
  // printf("AIR MASS is %lf scrubles\n", airMass);
  float zoomOut = 500;
  float leftRight = 0;
  float horizontalAngle = M_PI;
  float verticalAngle = 0.f;

  v3 initialPos = {leftRight+300, 98, zoomOut};
  v3 currentPos = {
    cos(verticalAngle) * sin(horizontalAngle),
    sin(verticalAngle),
    cos(verticalAngle) * cos(horizontalAngle)
  };

  v3 right = {
    sin(horizontalAngle + M_PI/2.f),
    0.f,
    cos(horizontalAngle + M_PI/2.f)
  };

  v3 upRight = crossProduct(currentPos, right);

  srand(time(NULL));

  int tilesX = 32;
  int tilesZ = 32;
  v3 tileSize = {2*WIDTH/(float)tilesX,
                 2*WIDTH/(float)tilesX,
                 2*WIDTH/(float)tilesX};

  gravity *= tileSize.x;

  // create tileMap block array
  block tileMap[tilesX*tilesZ];

  int width, height;
  float ratio;

  if(!glfwInit()) {
    printf("Error loading GLFW.\n");
    getchar();
    return -1;
  }

  glfwWindowHint(GLFW_SAMPLES, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  window = glfwCreateWindow(WIDTH, HEIGHT, "PI Final", NULL, NULL);
  if(!window) {
    printf("Error opening GLFW window.\n");
    getchar();
    glfwTerminate();
    return -1;
  }

  glfwMakeContextCurrent(window);

  if(glewInit() != GLEW_OK) {
    printf("Error loading GLEW.\n");
    getchar();
    glfwTerminate();
    return -1;
  }

  glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
  glfwPollEvents();

  glClearColor(0.f, 0.f, 0.f, 0.f);

  // Enable depth test
  glEnable(GL_DEPTH_TEST);
  // Accept fragment if it closer to the camera than the former one
  glDepthFunc(GL_LESS);

  for(int i = 0; i < tilesZ; i++) {
    for(int j = 0; j < tilesX; j++) {
      tileMap[j+(i*tilesX)].size = (v3){tileSize.x, tileSize.y, tileSize.z};
      tileMap[j+(i*tilesX)].pos = (v3){j*tileSize.x - (tilesX*tileSize.x/2),
                                       (-tileSize.y/2 + ((int)(rand()) % (int)tileSize.y)),
                                       i*tileSize.z - (tilesZ*tileSize.z/2)};
      tileMap[j+(i*tilesX)].speed = (v3){0,0,0};
      tileMap[j+(i*tilesX)].blockColor = (color){1.f, 1.f, 1.f};
      tileMap[j+(i*tilesX)].colorMode = SHADED;
      createBlockVBO(&tileMap[j+(i*tilesX)]);
    }
  }

  // create astronaut collision box
  block astronaut;
  astronaut.size = (v3) {0.9*tileSize.x, tileSize.y * 2, 0.9*tileSize.z};
  astronaut.pos = (v3) {-tilesX * tileSize.x/2, astronaut.size.y, 0};
  astronaut.speed = (v3) {0,0,0};
  astronaut.blockColor = (color){1.f, 0.f, 1.f};
  astronaut.colorMode = SHADELESS;
  astronaut.mass = 80.0;
  createBlockVBO(&astronaut);

  // Create a Vertex Array
  GLuint VertexArrayID;
  glGenVertexArrays(1, &VertexArrayID);
  glBindVertexArray(VertexArrayID);

  GLuint VertexArrayID2;
  glGenVertexArrays(1, &VertexArrayID2);
  glBindVertexArray(VertexArrayID2);

  // Create and compile our GLSL program from the shaders
  GLuint programID = LoadShaders("TransformVertexShader.vertexshader", "ColorFragmentShader.fragmentshader");
  GLuint programID2 = LoadShaders( "TransformVertexShader2.vertexshader", "TextureFragmentShader2.fragmentshader");

  // Get a handle for our "MVP" uniform
  GLuint MatrixID = glGetUniformLocation(programID, "MVP");
  GLuint MatrixID2 = glGetUniformLocation(programID2, "MVP");

  // Projection Matrix : 45˚ Field of View, 4:3 ratio, display range: 0.1 to 100 units
  glm::mat4 Projection = glm::perspective(glm::radians(70.f), 16.f/9.f, 0.1f, 10000.f);

  // Camera Matrix
  glm::mat4 View = glm::lookAt(
                    glm::vec3(initialPos.x, initialPos.y, initialPos.z),
                    glm::vec3(initialPos.x, initialPos.y, initialPos.z) +
                    glm::vec3(currentPos.x, currentPos.y, currentPos.z),
                    glm::vec3(upRight.x, upRight.y, upRight.z)
  );

  // Model matrix : an identity matrix (model will be at the origin)
  glm::mat4 Model = glm::mat4(1);
  // Our ModelViewProjection : multiplication of our 3 matrices
  glm::mat4 MVP = Projection * View * Model; // Remember, matrix multiplication is the other way around

  GLuint Texture2 = loadBMP_custom("Astronaut_BaseColor.bmp");
  GLuint Texture2ID = glGetUniformLocation(programID2, "myTextureSampler");

  std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals; // Won't be used at the moment.
  loadOBJ("Astronaut.obj", vertices, uvs, normals);

  GLuint vertexbuffer2;
	glGenBuffers(1, &vertexbuffer2);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer2);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

	GLuint uvbuffer2;
	glGenBuffers(1, &uvbuffer2);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer2);
	glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);

  ////////////////////////////////////////////////



  // GLuint programID2 = LoadShaders( "TransformVertexShader2.vertexshader", "TextureFragmentShader.fragmentshader" );

  // // Get a handle for our "MVP" uniform
  // GLuint MatrixID2 = glGetUniformLocation(programID2, "MVP");

  // // Load the texture
  // GLuint Texture = loadBMP_custom("Astronaut_BaseColor.bmp");

  // // Get a handle for our "myTextureSampler" uniform
  // GLuint TextureID  = glGetUniformLocation(programID2, "myTextureSampler");

  // // Read our .obj file
  // std::vector<glm::vec3> vertices;
  // std::vector<glm::vec2> uvs;
  // std::vector<glm::vec3> normals; // Won't be used at the moment.
  // loadOBJ("Astronaut.obj", vertices, uvs, normals);

  // // Load it into a VBO
  // GLuint vertexbuffer2;
  // glGenBuffers(1, &vertexbuffer2);
  // glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer2);
  // glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

  // GLuint uvbuffer;
  // glGenBuffers(1, &uvbuffer);
  // glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
  // glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);

  ////////////////////////////////////////////////
  //////////////// MAIN LOOP /////////////////////
  ////////////////////////////////////////////////

  while(glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
        glfwWindowShouldClose(window) == 0) {

    currentTime = glfwGetTime();
    deltaTime = (currentTime - lastTime);
    lastTime = currentTime;

    float navSpeed = 500.f;
    if(glfwGetKey(window, GLFW_KEY_S)) {
      initialPos.x -= currentPos.x * deltaTime * navSpeed;
      initialPos.y -= currentPos.y * deltaTime * navSpeed;
      initialPos.z -= currentPos.z * deltaTime * navSpeed;
    } else if(glfwGetKey(window, GLFW_KEY_W)) {
      initialPos.x += currentPos.x * deltaTime * navSpeed;
      initialPos.y += currentPos.y * deltaTime * navSpeed;
      initialPos.z += currentPos.z * deltaTime * navSpeed;
    } else if(glfwGetKey(window, GLFW_KEY_A)) {
      initialPos.x += right.x * deltaTime * navSpeed;
      initialPos.y += right.y * deltaTime * navSpeed;
      initialPos.z += right.z * deltaTime * navSpeed;
    } else if(glfwGetKey(window, GLFW_KEY_D)) {
      initialPos.x -= right.x * deltaTime * navSpeed;
      initialPos.y -= right.y * deltaTime * navSpeed;
      initialPos.z -= right.z * deltaTime * navSpeed;
    } else if(glfwGetKey(window, GLFW_KEY_R) && !resetPressed) {
      srand(time(NULL));
      resetPressed = true;
      astronaut.pos = (v3) {-tilesX * tileSize.x/2, astronaut.size.y, 0};
      astronaut.speed = (v3){0,0,0};
      pos = astronaut.pos.x;
      gravity = randRange(0.1f, GRAVITY_MAX);
      printf("GRAVITY: %lf m/s^2\n", gravity);
      // airMass = randRange(0.f, AIRMASS_MAX);
  	  // printf("AIR MASS is %lf scrubles\n", airMass);
      gravity *= tileSize.x;
    }

    if(glfwGetKey(window, GLFW_KEY_R) == GLFW_RELEASE) {
      resetPressed = false;
    }

    if(glfwGetKey(window,GLFW_KEY_SPACE) &&
       !spacePressed) {
      spacePressed = true;
      astronaut.speed.y = 5 * tileSize.x;
      astronaut.speed.x = 5 * tileSize.x;
      pos = astronaut.pos.x;
      // puts("space");
    }

    if(zoomOut < 0) zoomOut = 0;

    View = glm::lookAt(
                      glm::vec3(initialPos.x, initialPos.y, initialPos.z),
                      glm::vec3(initialPos.x, initialPos.y, initialPos.z) +
                      glm::vec3(currentPos.x, currentPos.y, currentPos.z),
                      glm::vec3(upRight.x, upRight.y, upRight.z)
    );

    MVP = Projection * View * Model;

    // Clear screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Use our shader
		glUseProgram(programID);

    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // First attribute of the buffer : vertices
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    // Draw Tile Map
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    for(int i = 0; i < tilesX*tilesZ; i++) {
      glm::mat4 tilePos = Model;
      tilePos = glm::translate(tilePos, glm::vec3(tileMap[i].pos.x, tileMap[i].pos.y, tileMap[i].pos.z));
      MVP = Projection * View * tilePos;
      glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

      glBindBuffer(GL_ARRAY_BUFFER, tileMap[i].blockID);
      glVertexAttribPointer(
        0,              // attribute 0.
        3,              // size (in vertices)
        GL_FLOAT,       // type
        GL_FALSE,       // normalized(yes or no?)
        0,              // offset
        (void *)0       // offset of array buffer
      );

      glBindBuffer(GL_ARRAY_BUFFER, tileMap[i].colorID);
      glVertexAttribPointer(
        1,              // attribute 1.
        3,              // size (in vertices)
        GL_FLOAT,       // type
        GL_FALSE,       // normalized(yes or no?)
        0,              // offset
        (void *)0       // offset of array buffer
      );

      glDrawArrays(GL_TRIANGLE_STRIP, 0, 14); // 3 indices starting at 0 -> 1 triangle
    }

    // Draw Astronaut

    glm::mat4 trans = Model;
    trans = glm::translate(trans, glm::vec3(astronaut.pos.x, astronaut.pos.y, astronaut.pos.z));

    MVP = Projection * View * trans;

    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

    glEnable(GL_LINE_SMOOTH);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    glBindBuffer(GL_ARRAY_BUFFER, astronaut.blockID);
    glVertexAttribPointer(
      0,              // attribute 0.
      3,              // size (in vertices)
      GL_FLOAT,       // type
      GL_FALSE,       // normalized(yes or no?)
      0,              // offset
      (void *)0       // offset of array buffer
    );

    glBindBuffer(GL_ARRAY_BUFFER, astronaut.colorID);
    glVertexAttribPointer(
      1,              // attribute 1.
      3,              // size (in vertices)
      GL_FLOAT,       // type
      GL_FALSE,       // normalized(yes or no?)
      0,              // offset
      (void *)0       // offset of array buffer
    );

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 14);

    glUseProgram(programID2);

    glm::mat4 MVPastronaut = MVP;
    MVPastronaut = glm::scale(MVPastronaut, glm::vec3(32*1.2, 32*1.2, 32*1.2));
    MVPastronaut = glm::translate(MVPastronaut, glm::vec3(0, -2, 0));
    MVPastronaut = glm::rotate(MVPastronaut, (float)M_PI/2, glm::vec3(0, 1, 0));
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glUniformMatrix4fv(MatrixID2, 1, GL_FALSE, &MVPastronaut[0][0]);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, Texture2);
    glUniform1i(Texture2ID, 0);

    // 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer2);
		glVertexAttribPointer(
			0,                  // attribute
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		// 2nd attribute buffer : UVs
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer2);
		glVertexAttribPointer(
			1,                                // attribute
			2,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);

    glDrawArrays(GL_TRIANGLES, 0, vertices.size());

    // 3 indices starting at 0 -> 1 triangle

    int numOfChecks = 32;
    for(int t = 0;t < numOfChecks;++t) {
    bool collision = false;
      for(int i = 0; i < tilesX * tilesZ;i++) {
        if(colliding(astronaut, tileMap[i])) {
          collision = true;
          break;
        }
      }
      astronaut.speed.y -= gravity * (deltaTime/numOfChecks);
      if(collision) {
        if (astronaut.speed.y < 0) astronaut.speed.y = 0;

        if (astronaut.speed.y == 0) astronaut.speed.x = 0;
        if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE &&
           spacePressed) {
          spacePressed = false;

	      	if(pos != astronaut.pos.x && astronaut.speed.y == 0) {
	     		printf("DISTANCE: %fm\n", fabsf(astronaut.pos.x - pos)/tileSize.x);
	      	}
        }
      }
      astronaut.pos.y += astronaut.speed.y * (deltaTime/numOfChecks);
      astronaut.pos.x += astronaut.speed.x * (deltaTime/numOfChecks);
      airMass = randRange(0.f, AIRMASS_MAX);
      astronaut.pos.x *= astronaut.mass / (astronaut.mass + pow(airMass, astronaut.size.z));
      astronaut.pos.y *= astronaut.mass / (astronaut.mass + pow(airMass, astronaut.size.z));
    }

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);

    // Draw 3D object

    // Bind our texture in Texture Unit 0
	// glActiveTexture(GL_TEXTURE0);
	// glBindTexture(GL_TEXTURE_2D, Texture);
	// // Set our "myTextureSampler" sampler to use Texture Unit 0
	// glUniform1i(TextureID, 0);

	// // 1rst attribute buffer : vertices
	// glEnableVertexAttribArray(2);
	// glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer2);
	// glVertexAttribPointer(
	// 	0,                  // attribute
	// 	3,                  // size
	// 	GL_FLOAT,           // type
	// 	GL_FALSE,           // normalized?
	// 	0,                  // stride
	// 	(void*)0            // array buffer offset
	// );

	// // 2nd attribute buffer : UVs
	// glEnableVertexAttribArray(3);
	// glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	// glVertexAttribPointer(
	// 	1,                                // attribute
	// 	2,                                // size
	// 	GL_FLOAT,                         // type
	// 	GL_FALSE,                         // normalized?
	// 	0,                                // stride
	// 	(void*)0                          // array buffer offset
	// );

	// glDrawArrays(GL_TRIANGLES, 0, vertices.size());

	// glDisableVertexAttribArray(2);
 //    glDisableVertexAttribArray(3);

    glfwSwapBuffers(window);
    glfwPollEvents();

  }

  // Cleanup VBO
  for(int i = 0; i < tilesX*tilesZ; i++) {
	 glDeleteBuffers(1, &tileMap[i].blockID);
   glDeleteBuffers(1, &tileMap[i].colorID);
 }
	glDeleteVertexArrays(1, &VertexArrayID);

	glDeleteBuffers(1, &vertexbuffer2);
	glDeleteBuffers(1, &uvbuffer2);
	glDeleteProgram(programID);
	glDeleteProgram(programID2);
	glDeleteTextures(1, &Texture2);
	glDeleteVertexArrays(1, &VertexArrayID2);

  // Close window
  glfwTerminate();
  return 0;
}
