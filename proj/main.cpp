// Include C++ headers
#include <iostream>
#include <string>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <glfw3.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "glm/gtx/string_cast.hpp"

// Include openmp
#include <omp.h>

// Shader loading utilities and other
#include <common/shader.h>
#include <common/util.h>
#include <common/camera.h>
#include <common/model.h>

// Include project code
#include "Sphere.h"
#include "BoundingBox.h"
#include "Collision.h"
#include "Box.h"
#include "SphereFit.h"
#include "GlobalVariables.h"
#include "Simulation.h"
#include "Billboard.h"
#include "BillboardGenerator.h"

// Mechanics to be included in the executable
#define SPHERES
#define GLOVE
#define DISPERSION

using namespace std;
using namespace glm;

// Function prototypes
void initialize();
void createContext();
void mainLoop();
void free();

#define W_WIDTH 1024
#define W_HEIGHT 768
#define TITLE "Thanos Snap"

// Global game data structures
GLFWwindow* window;
Camera* camera;
GLuint shaderProgram;
GLuint projectionMatrixLocation, viewMatrixLocation, modelMatrixLocation;
float limits[5][6];
Drawable* thanos;
vector<BillboardGenerator*> bboard_generator(N);
vector<Drawable*> models;
vector<vector<BoundingBox*>> bbox(N, vector<BoundingBox*>(5));
vector<vector<Sphere*>> spheres(N);
vector<vector<bool>> billboardMap(5);
vector<float> b_levels;
vector<vector<float>> spheresStartingHeight(N);

// Global variables
bool clicked = false;
bool dispersion[N] = { false };
bool extinct[N] = { false };
float disp_level[N];
float disp_speed = 2.5f;
float bboard_size = 0.02f;
bool sim[N] = { false };
bool wireframe = false;
int b_level_counter[N] = { 0 };
float model_speed = 0.01f;

void createContext() {
    shaderProgram = loadShaders(
        "StandardShading.vertexshader",
        "StandardShading.fragmentshader");

    projectionMatrixLocation = glGetUniformLocation(shaderProgram, "P");
    viewMatrixLocation = glGetUniformLocation(shaderProgram, "V");
    modelMatrixLocation = glGetUniformLocation(shaderProgram, "M");

    // Debug console messages
    if (DEBUG_MESSAGES) {
        cout << "\n************ Runtime debug messages ************" << endl;
        cout << "Available threads: " << omp_get_max_threads() << "\n" << endl;
    }

    // Add human models
    for(int i = 0; i < N; i++)
        models.push_back(new Drawable("models/BodyMesh.obj"));

#ifdef GLOVE
    // add thanos glove
    thanos = new Drawable("models/thanos.obj");
#endif

    /**
     * Create the bounding boxes of the model, between manually selected y-levels.
     * These levels are shown in the createLimitsArray function in SphereFit.cpp
     * Each bbox also holds the vertices that it contains in a class data member
     */
    createLimitsArray();
    if (DEBUG_MESSAGES)
        cout << "Vertices inside each bounding box: "<< endl;
    for (int n = 0; n < N; n++) {
        for (int i = 0; i < 5; i++) {
            bbox[n][i] = new BoundingBox(limits[i]);
            if (n == 0) bbox[n][i]->fillVertices(models[0]->vertices);
            else bbox[n][i]->vertices = bbox[0][i]->vertices;
            if (DEBUG_MESSAGES && n == 0)
                cout << i << ": " << bbox[n][i]->vertices.size() << endl;
        }
    }
#ifdef DISPERSION
    // Create Billboards for the dispersion effect
    createBillboardMap(bboard_size);
    double start1 = omp_get_wtime();
    for(int i = 0; i < N; i++)
        bboard_generator[i] = new BillboardGenerator(bbox[i], billboardMap, bboard_size);
    double end1 = omp_get_wtime();
    if (DEBUG_MESSAGES) {
        cout << "\nBillboard generation took " << end1 - start1 << " seconds" << endl;
    }
#endif

#ifdef SPHERES
    /**
     * Fit and create the spheres inside the human with the parameters given.
     * The parameter cube_side is the step used in the createSpheres function.
     */
    // Parameters
    int cube_side = 15;     // The real cube side will be cube_side/100.0f
    float rad[] = { 0.03f, 0.02f, 0.015f };
    float mass = 0.3f;

    double start2 = omp_get_wtime();
    createSpheres(cube_side, rad, mass);
    double end2 = omp_get_wtime();

    if (DEBUG_MESSAGES) {
        cout << "\nSphere fitting and creation took " << end2 - start2 << " seconds" << endl;
        cout << "Spheres inside the model:\n" << spheres[0].size() << endl;
    }
#endif
}

// Slow model movement after the first kill
void moveModels(vec3* positions, mat4* matrices) {
    bool started = false;
    for (int i = 0; i < N; i++)
        started = started || sim[i];
    if (!started) return;
    for (int i = 0; i < N; i++) {
        if (sim[i]) continue;
        vec3 dir = normalize(camera->position - positions[i]);
        positions[i] += dir * model_speed;
        matrices[i] = translate(mat4(), positions[i]);
        for (int j = 0; j < spheres[i].size(); j++)
            spheres[i][j]->x += dir * model_speed;

        for (int j = 0; j < bbox[i].size(); j++) {
            for (int k = 0; k < bbox[i][j]->BoxVertices.size(); k++) {
                bbox[i][j]->BoxVertices[k] += dir * model_speed;
            }
        }
    }
}

// Checks if the user gets touched by the models
bool killed(vec3 *positions) {
    for (int i = 0; i < N; i++) {
        if (extinct[i]) continue;
        if (distance(positions[i], camera->position) < 0.05f)
            return true;
    }
    return false;
}

void free() {
    for (int n = 0; n < N; n++)
        delete models[n];
    for(int n = 0; n < N; n++)
        for (int i = 0; i < 5; i++)
            delete bbox[n][i];
    for (int i = 0; i < spheres.size(); i++)
        for(int n = 0; n < spheres[i].size(); n++)
            delete spheres[i][n];
    glDeleteProgram(shaderProgram);
    glfwTerminate();
}

void mainLoop() {
    if(DEBUG_MESSAGES)
        cout << "\n---- Render loop initiated ----" <<endl;
    // User starting position
    camera->position = glm::vec3(0, 1.5, 10);
    float t = 0;
    float dt = 0;

    // Models' starting positions
    vec3 modelPositions[] = {
        vec3(-9.0f, -limits[4][2] + 0.01f, -5.0f),
        vec3(-3.0f, -limits[4][2] + 0.01f, 0.0f),
        vec3(3.0f, -limits[4][2] + 0.01f, -5.0f),
        vec3(9.0f, -limits[4][2] + 0.01f, 0.0f)
    };

    // Models' model matrices 
    mat4 maleModelMatrix[N];
    for(int i = 0; i < N; i++)
        maleModelMatrix[i] = translate(mat4(), modelPositions[i]);

#ifdef SPHERES
    // Put spheres into their right starting position
    for (int n = 0; n < N; n++) {
        for (int i = 0; i < spheres[n].size(); i++) {
            spheres[n][i]->x += modelPositions[n];
            spheresStartingHeight[n][i] += modelPositions[n].y;
        } 
    }
#endif

    // Move bounding boxes into their right starting position
    for (int i = 0; i < bbox.size(); i++) {
        for (int j = 0; j < bbox[i].size(); j++) {
            for (int k = 0; k < bbox[i][j]->BoxVertices.size(); k++) {
                bbox[i][j]->BoxVertices[k] += modelPositions[i];
            }
        } 
    }

    // Initialize disp_level helping variable
    for (int i = 0; i < N; i++)
        disp_level[i] = limits[0][3];
    
    do {
        // Initialize simulation time
        if (!clicked) t = glfwGetTime();
        // Apply controlled simulation step (for slow PCs)
        if (clicked) {
            dt = glfwGetTime() - t;
            if (dt > 0.004) dt = 0.004;
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(shaderProgram);

        // camera
        camera->update();
        mat4 projectionMatrix = camera->projectionMatrix;
        mat4 viewMatrix = camera->viewMatrix;
        glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, &viewMatrix[0][0]);
        glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, &projectionMatrix[0][0]);

        // Update models' position
        moveModels(modelPositions, maleModelMatrix);

        // Draw the models if they have not been destroyed, or the
        // part of them that hasn't been destroyed yet
        for (int n = 0; n < N; n++) {
            if (!dispersion[n]) {
                models[n]->bind();
                glUniform1i(glGetUniformLocation(shaderProgram, "balls"), 0);
                glUniform1f(glGetUniformLocation(shaderProgram, "disp_level"), disp_level[n]);
                glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, &maleModelMatrix[n][0][0]);
                if (wireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                models[n]->draw();
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            }
            else if (!extinct[n]) {
                if (disp_level[n] > limits[4][2]) disp_level[n] -= disp_speed * dt;
                else {
                    extinct[n] = true;
                    continue;
                }
#ifdef DISPERSION
                if (disp_level[n] <= b_levels[b_level_counter[n]]) {
                    bboard_generator[n]->newRow(b_level_counter[n]);
                    b_level_counter[n]++;
                    if (b_level_counter[n] == b_levels.size()) b_level_counter[n]--;
                }
#endif
                models[n]->bind();
                glUniform1i(glGetUniformLocation(shaderProgram, "balls"), 0);
                glUniform1f(glGetUniformLocation(shaderProgram, "disp_level"), disp_level[n]);
                glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, &maleModelMatrix[n][0][0]);
                if (wireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                models[n]->draw();
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            }
        }

#ifdef DISPERSION
        // Update and draw the billboards
        for (int n = 0; n < N; n++) {
            bboard_generator[n]->updateBillboards(modelPositions[n], camera->position);
            for (int i = 0; i < b_level_counter[n]; i++) {
                for (int j = 0; j < bboard_generator[n]->billboards[i].size(); j++) {
                    bboard_generator[n]->billboards[i][j]->quad->bind();
                    mat4 modMatrix = maleModelMatrix[n] * bboard_generator[n]->billboards[i][j]->modelMatrix;
                    glUniform1i(glGetUniformLocation(shaderProgram, "balls"), 0);
                    glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, &modMatrix[0][0]);
                    bboard_generator[n]->billboards[i][j]->quad->draw();
                }
                
            }
            bboard_generator[n]->removeBillboards();
        }
#endif

#ifdef SPHERES
        // Draw the spheres if the human is in wireframe mode or the simulation has started
        for (int n = 0; n < N; n++) {
            if (sim[n]) {
                for (int i = 0; i < spheres[n].size(); i++) {
                    if (spheresStartingHeight[n][i] - spheres[n][i]->r < disp_level[n] - limits[4][2]) continue;
                    handleFloorSphereCollision(*spheres[n][i]);
                    for (int j = 0; j < spheres[n].size(); j++) {
                        if (i == j) continue;
                        handleSpheresCollision(*spheres[n][i], *spheres[n][j]);
                    }
                    spheres[n][i]->forcing = [&](float t, const vector<float>& y)->vector<float> {
                        vector<float> f(6, 0.0f);
                        f[1] = -(spheres[n][i]->m * g_earth);
                        return f;
                    };
                    spheres[n][i]->update(t, dt);
                    glUniform1i(glGetUniformLocation(shaderProgram, "balls"), 1);
                    glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, &spheres[n][i]->modelMatrix[0][0]);
                    spheres[n][i]->draw();
                }
            }
            else if (wireframe) {
                for (int i = 0; i < spheres[n].size(); i++) {
                    spheres[n][i]->update();
                    glUniform1i(glGetUniformLocation(shaderProgram, "balls"), 1);
                    glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, &spheres[n][i]->modelMatrix[0][0]);
                    spheres[n][i]->draw();
                }
            }
        }
        removeSpheres();
#endif

#ifdef GLOVE
        // Draw the glove
        mat4 thanos_model = translate(mat4(), vec3(0.0f, 0.32f, 0.56f));
        thanos_model = rotate(mat4(), radians(45.0f), vec3(0.0f, 1.0f, 0.0f)) * thanos_model;
        thanos_model = rotate(mat4(), radians(90.0f), vec3(-1.0f, 0.0f, 0.0f)) * thanos_model;
        // No movement on vertical axis yet //
        thanos_model = rotate(mat4(), camera->horizontalAngle + radians(180.0f), vec3(0.0f, 1.0f, 0.0f)) * thanos_model;
        vec3 glove_position = camera->position + camera->direction * 0.8f - 0.3f * camera->up;
        thanos_model = translate(mat4(), glove_position) * thanos_model;
        thanos->bind();
        glUniform1i(glGetUniformLocation(shaderProgram, "balls"), 1);
        glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, &thanos_model[0][0]);
        thanos->draw();
#endif

        t += dt;
        glfwSwapBuffers(window);
        glfwPollEvents();

    } while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
             glfwWindowShouldClose(window) == 0 && !killed(modelPositions));    // end the game if the models touch the user
}

void pollKeyboard(GLFWwindow* window, int key, int scancode, int action, int mods) {
    
    // T key changes between wireframe and normal mode
    if (key == GLFW_KEY_T && action == GLFW_PRESS) {
        wireframe = !wireframe;
    }
}

void pollMouse(GLFWwindow* window, int button, int action, int mods) {
    // Check if the user targeted a model
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        clicked = true;
        checkSim(camera->position, camera->horizontalAngle, camera->verticalAngle);
    }
}

void initialize() {
    // Initialize GLFW
    if (!glfwInit()) {
        throw runtime_error("Failed to initialize GLFW\n");
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Open a window and create its OpenGL context
    window = glfwCreateWindow(W_WIDTH, W_HEIGHT, TITLE, NULL, NULL);
    if (window == NULL) {
        glfwTerminate();
        throw runtime_error(string(string("Failed to open GLFW window.") +
                            " If you have an Intel GPU, they are not 3.3 compatible." +
                            "Try the 2.1 version.\n"));
    }
    glfwMakeContextCurrent(window);

    // Start GLEW extension handler
    glewExperimental = GL_TRUE;

    // Initialize GLEW
    if (glewInit() != GLEW_OK) {
        glfwTerminate();
        throw runtime_error("Failed to initialize GLEW\n");
    }

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    // Hide the mouse and enable unlimited movement
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Set the mouse at the center of the screen
    glfwPollEvents();
    glfwSetCursorPos(window, W_WIDTH / 2, W_HEIGHT / 2);

    // Gray background color
    glClearColor(0.5f, 0.5f, 0.5f, 0.0f);

    // Poll keyboard
    glfwSetKeyCallback(window, pollKeyboard);

    // Poll mouse
    glfwSetMouseButtonCallback(window, pollMouse);

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);

    // Cull triangles which normal is not towards the camera
    glEnable(GL_CULL_FACE);
    // glFrontFace(GL_CW);
    // glFrontFace(GL_CCW);

    // enable point size when drawing points
    glEnable(GL_PROGRAM_POINT_SIZE);

    // Log
    logGLParameters();

    // Create camera
    camera = new Camera(window);
}

int main(void) {
    try {
        initialize();
        createContext();
        mainLoop();
        free();
    } catch (exception& ex) {
        cout << ex.what() << endl;
        getchar();
        free();
        return -1;
    }

    return 0;
}
