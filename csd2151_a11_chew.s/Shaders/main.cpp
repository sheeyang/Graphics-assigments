/*!*****************************************************************************
\file main.cpp
\author Vadim Surov (vsurov@digipen.edu)
\co-author Chew Shee Yang (chew.s@digipen.edu)
\par Course: CSD2151
\par Assignment: 11 (Deferred Shading with Normal Mapping)
\date 03/17/2026 (MM/DD/YYYY)
\brief
    Application entry point for the Deferred Shading + Normal Mapping assignment.
    Implements a menu system with 8 previous assignment subwindows, plus a new
    primary A11 window featuring:
    - Deferred shading (2-pass: geometry G-buffer + lighting pass)
    - Normal mapping on the Ogre model
    - Grid instancing of multiple ogres
    - Camera orbit/zoom + light orbit/zoom controls
    - Keys 1-3 to switch rendering modes

\par AI Disclosure:
    AI (GitHub Copilot) was used to brainstorm implementation ideas and to
    write documentation comments for this assignment.
*******************************************************************************/
#include <iostream> // input/output
#include <vector>   // for storing child windows

#include "windowmanager.h"
#include "trianglemeshes.h"

const int NOMARGIN = 0; // To set no margin for a window

// Grid configuration for menu (4 columns x 3 rows = 12 max)
const int GRID_COLS = 4;
const int GRID_ROWS = 3;
const int GRID_SIZE = GRID_COLS * GRID_ROWS; // 12 subwindows max
const int GRID_GAP = 10;

// Number of active subwindows
const int NUM_SUBWINDOWS = 9;

// Tab indicator constants (from Assignment 1)
const int NUM_TABS = 3;
const float TAB_CIRCLE_RADIUS = 15.0f;
const float TAB_CIRCLE_Y_OFFSET = 30.0f;
const float TAB_CIRCLE_SPACING = 40.0f;

// Assignment 4 constants
constexpr float A4_OBJECT_DISTANCE = 3.0f; // Distance of objects from center
constexpr float A4_PLANE_SCALE = 2.0f;     // Scale of the floor plane

// ============================================================================
// Map data for collision detection and shader (passed to GPU)
// ============================================================================
const int MAP_SIZE = 9;
const int MAP[MAP_SIZE * MAP_SIZE] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 2, 2, 0, 3, 3, 0, 1,
    1, 0, 2, 0, 0, 0, 3, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 4, 0, 0, 0, 2, 0, 1,
    1, 0, 4, 4, 0, 2, 2, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1};

// Collision detection margin (distance from wall)
const float COLLISION_MARGIN = 0.2f;

// Check if a position is inside a wall
bool isWall(float x, float y)
{
    int mapX = static_cast<int>(x);
    int mapY = static_cast<int>(y);
    if (mapX < 0 || mapX >= MAP_SIZE || mapY < 0 || mapY >= MAP_SIZE)
        return true;
    return MAP[mapY * MAP_SIZE + mapX] > 0;
}

// Check if a position with margin is valid (not colliding with walls)
bool canMoveTo(float x, float y)
{
    return !isWall(x - COLLISION_MARGIN, y - COLLISION_MARGIN) &&
           !isWall(x + COLLISION_MARGIN, y - COLLISION_MARGIN) &&
           !isWall(x - COLLISION_MARGIN, y + COLLISION_MARGIN) &&
           !isWall(x + COLLISION_MARGIN, y + COLLISION_MARGIN);
}

// Demo objects for Assignment 8 / 9 subwindows
#define DEMOOBJECTS_A8 \
    {\
        {\
            PLANE,\
            MATERIAL_CHECKERBOARD,\
            glm::translate(glm::scale(glm::mat4(1.0f), {3.0f, 1.0f, 3.0f}), {0.0f, -1.1f, 0.0f}),\
            VISIBLE\
        },\
        {\
            SPHERE,\
            MATERIAL_PBR_PHONG,\
            glm::translate(glm::mat4(1.0f), {2.5f, 0.0f, 0.0f}),\
            VISIBLE\
        },\
        {\
            TORUS,\
            MATERIAL_CARTOON,\
            NOTRANSFORM,\
            VISIBLE\
        },\
        {\
            TEAPOT,\
            MATERIAL_DISCARD,\
            glm::translate(glm::mat4(1.0f), {-2.5f, 0.0f, 0.0f}),\
            VISIBLE\
        }\
    }

// Global storage for child windows
std::vector<GLFWwindow *> subwindows;

// ============================================================================
// MAKE_OGRE_OBJECT(meshName, xSize, ySize, spacing)
//   Expands to a cg::Object whose instances form an xSize x ySize grid
//   centered at the origin with the given spacing between each ogre.
// ============================================================================
#define MAKE_OGRE_OBJECT(meshName, xSize, ySize, spacing)            \
    ([&]() {                                                         \
        cg::Object _obj{(meshName), MATERIAL_OBJECT};                \
        _obj.instances.clear();                                      \
        _obj.instances.reserve((xSize) * (ySize));                   \
        float _hx = ((xSize) - 1) * (spacing) * 0.5f;               \
        float _hz = ((ySize) - 1) * (spacing) * 0.5f;               \
        for (int _r = 0; _r < (ySize); ++_r)                        \
            for (int _c = 0; _c < (xSize); ++_c)                    \
                _obj.instances.push_back({MATERIAL_OBJECT,           \
                    glm::translate(glm::mat4(1.0f),                  \
                        glm::vec3(_c*(spacing)-_hx, 0.0f,           \
                                  _r*(spacing)-_hz)), VISIBLE});     \
        return _obj;                                                 \
    }())



// Entry point of the application
int main(int argc, char **argv)
{
    int retcode = 0;

    auto &windowManager = cg::WindowManager::getInstance();

    try
    {
        if (!glfwInit())
            throw std::runtime_error("Failed to initialize GLFW.");

#ifdef _DEBUG
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif

        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        GLFWwindow *pMainWindow = nullptr;

        // ====================================================================
        // MAIN MENU WINDOW
        // ====================================================================
        pMainWindow = windowManager.addWindow({cg::State::SHOW,
                                               nullptr,
                                               {0, 0, 1024, 768},
                                               "CSD2151 Assignment 11 - Menu",
                                               {0.15f, 0.15f, 0.2f, 1.0f},
                                               NONE,
                                               NONE,
                                               {
                                                {
                                                 {NOMARGIN},
                                                 {0.15f, 0.15f, 0.2f, 1.0f},
                                                 DISABLE,
                                                 {{QUAD}},
                                                 NONE,
                                                 NONE,
                                                 DEFAULT,
                                                 {
#include "Shaders/Shader.vert.glsl"
                                                     ,
#include "Shaders/ShaderMenu.frag.glsl"
                                                 }}},

                                               // FocusCallback
                                               [](cg::ManagedWindow *pWindow, int focused)
                                               {
                                                   if (focused)
                                                   {
                                                       std::cout << std::endl;
                                                       std::cout << "CSD2151 Assignment 11 - Menu (version "
                                                                 << __DATE__ << " " << __TIME__ << ")";
                                                       std::cout << std::endl
                                                                 << R"(
   This application demonstrates deferred shading with normal mapping.
   Each subwindow can be opened in a separate window by double-clicking.

Controls:
   - Double-click: Open subwindow in a new window
   - Resize: Drag the window edges to adjust the application size
   - Fullscreen: Press Space to toggle between windowed and fullscreen mode
   - Save Screenshot: Press 'P' to save a screenshot of the active window
   - Exit: Press Esc to close the application

Subwindows:
   - Subwindow 1: Assignment 1 (Tabs) - Animated visual effects
   - Subwindow 2: Assignment 2 (Ray Casting) - First-person environment
   - Subwindow 3: Assignment 3 (Ray Tracing) - Earth, Sun, Moon system
   - Subwindow 4: Assignment 4 (Blinn-Phong) - 3D shaded objects with lighting
   - Subwindow 5: Assignment 5 (Environment Mapping) - Skybox with reflection/refraction
   - Subwindow 6: Assignment 8 (Unified Rendering) - PBR, Cartoon, Discard, Fog
   - Subwindow 7: Assignment 9 (Cel-Shading) - 6-pass cel-shading pipeline
   - Subwindow 8: Assignment 10 (Portal) - Skybox with projected portal texture
   - Subwindow 9: Assignment 11 (Deferred Shading + Normal Mapping) - Ogres grid
)";
                                                   }
                                               },

                                               // InitCallback
                                               [](cg::ManagedWindow *pWindow, int pass)
                                               {
                                                   if (pass == 0)
                                                   {
                                                       pWindow->passes[0].renderShader.use();
                                                       pWindow->passes[0].renderShader.setUniform("iNumSubwindows", NUM_SUBWINDOWS);
                                                       pWindow->passes[0].renderShader.setUniform("iGridSize", GRID_SIZE);
                                                   }
                                               },

                                               // SizeCallback
                                               [](cg::ManagedWindow *pWindow, int width, int height) {},

                                               // KeyCallback
                                               [](cg::ManagedWindow *pWindow, int key)
                                               {
                                                   return true;
                                               },

                                               // MousebuttonCallback
                                               [](cg::ManagedWindow *pWindow, int button, int xpos, int ypos) {},

                                               // ScrollCallback
                                               [](cg::ManagedWindow *pWindow, double xoffset, double yoffset, int mods) {},

                                               // CursorposCallback
                                               [](cg::ManagedWindow *pWindow, int xpos, int ypos, double xoffset, double yoffset, int mods) {},

                                               // MouseclickCallback (A11 new)
                                               nullptr,

                                               // MousedoubleclickCallback (A11 new)
                                               nullptr});

        // ====================================================================
        // SUBWINDOW 1: Assignment 1 (Tabs)
        // ====================================================================
        subwindows.push_back(windowManager.addWindow({cg::State::SHOW,
                                                      pMainWindow,
                                                      {0, GRID_SIZE, GRID_GAP},
                                                      "Assignment 1 - Tabs",
                                                      NONE,
                                                      NONE,
                                                      NONE,
                                                      {
                                                       {
                                                        {NOMARGIN},
                                                        NONE,
                                                        DISABLE,
                                                        {{QUAD}},
                                                        NONE,
                                                        NONE,
                                                        DEFAULT,
                                                        {
#include "Shaders/Shader.vert.glsl"
                                                            ,
#include "Shaders/ShaderA1.frag.glsl"
                                                        }}},

                                                      // FocusCallback
                                                      [](cg::ManagedWindow *pWindow, int focused)
                                                      {
                                                          if (focused)
                                                              std::cout << "Subwindow 1 (Assignment 1 - Tabs) focused. Tab: " << (pWindow->context.intparam[0] + 1) << std::endl;
                                                      },

                                                      // InitCallback
                                                      [](cg::ManagedWindow *pWindow, int pass)
                                                      {
                                                          if (pass == 0)
                                                          {
                                                              pWindow->passes[0].renderShader.use();
                                                              pWindow->passes[0].renderShader.setUniform("iCurrentTab", pWindow->context.intparam[0]);
                                                              pWindow->passes[0].renderShader.setUniform("iMousePos", pWindow->context.vec2param[0]);
                                                          }
                                                      },

                                                      // SizeCallback
                                                      [](cg::ManagedWindow *pWindow, int width, int height) {},

                                                      // KeyCallback
                                                      [](cg::ManagedWindow *pWindow, int key)
                                                      {
                                                          if (key == GLFW_KEY_TAB)
                                                          {
                                                              pWindow->context.intparam[0] = (pWindow->context.intparam[0] + 1) % NUM_TABS;
                                                              return false;
                                                          }
                                                          return true;
                                                      },

                                                      // MousebuttonCallback
                                                      [](cg::ManagedWindow *pWindow, int button, int xpos, int ypos)
                                                      {
                                                          if (button == GLFW_MOUSE_BUTTON_LEFT)
                                                          {
                                                              float centerX = pWindow->width / 2.0f;
                                                              float y = pWindow->height - TAB_CIRCLE_Y_OFFSET;
                                                              for (int i = 0; i < NUM_TABS; i++)
                                                              {
                                                                  float tabX = centerX + (float(i) - 1.0f) * TAB_CIRCLE_SPACING;
                                                                  float dx = xpos - tabX;
                                                                  float dy = ypos - y;
                                                                  if (sqrt(dx * dx + dy * dy) <= TAB_CIRCLE_RADIUS + 5.0f)
                                                                  {
                                                                      pWindow->context.intparam[0] = i;
                                                                      break;
                                                                  }
                                                              }
                                                          }
                                                      },

                                                      // ScrollCallback
                                                      [](cg::ManagedWindow *pWindow, double xoffset, double yoffset, int mods) {},

                                                      // CursorposCallback
                                                      [](cg::ManagedWindow *pWindow, int xpos, int ypos, double xoffset, double yoffset, int mods)
                                                      {
                                                          pWindow->context.vec2param[0] = glm::vec2(static_cast<float>(xpos), static_cast<float>(ypos));
                                                      },

                                                      // MouseclickCallback (A11 new)
                                                      nullptr,

                                                      // MousedoubleclickCallback (A11 new)
                                                      nullptr},
                                                     pMainWindow));

        // ====================================================================
        // SUBWINDOW 2: Assignment 2 (Ray Casting)
        // ====================================================================
        subwindows.push_back(windowManager.addWindow({cg::State::SHOW,
                                                      pMainWindow,
                                                      {1, GRID_SIZE, GRID_GAP},
                                                      "Assignment 2 - Ray Casting",
                                                      NONE,
                                                      NONE,
                                                      NONE,
                                                      {
                                                       {
                                                        {NOMARGIN},
                                                        NONE,
                                                        DISABLE,
                                                        {{QUAD}},
                                                        NONE,
                                                        NONE,
                                                        DEFAULT,
                                                        {
#include "Shaders/Shader.vert.glsl"
                                                            ,
#include "Shaders/ShaderA2.frag.glsl"
                                                        }}},

                                                      // FocusCallback
                                                      [](cg::ManagedWindow *pWindow, int focused)
                                                      {
                                                          if (focused)
                                                              std::cout << "Subwindow 2 (Assignment 2 - Ray Casting) focused" << std::endl;
                                                      },

                                                      // InitCallback
                                                      [](cg::ManagedWindow *pWindow, int pass)
                                                      {
                                                          if (pass == 0)
                                                          {
                                                              pWindow->passes[0].renderShader.use();
                                                              pWindow->passes[0].renderShader.setUniform("iCameraPos", pWindow->context.vec2param[0]);
                                                              pWindow->passes[0].renderShader.setUniform("iCameraDir", pWindow->context.floatparam[0]);
                                                              pWindow->passes[0].renderShader.setUniform("iMapSize", MAP_SIZE);
                                                              for (int i = 0; i < MAP_SIZE * MAP_SIZE; i++)
                                                              {
                                                                  std::string uniformName = "iMap[" + std::to_string(i) + "]";
                                                                  pWindow->passes[0].renderShader.setUniform(uniformName, MAP[i]);
                                                              }
                                                          }
                                                      },

                                                      // SizeCallback
                                                      [](cg::ManagedWindow *pWindow, int width, int height) {},

                                                      // KeyCallback
                                                      [](cg::ManagedWindow *pWindow, int key)
                                                      {
                                                          float moveSpeed = 0.1f;
                                                          float dir = pWindow->context.floatparam[0];
                                                          float currentX = pWindow->context.vec2param[0].x;
                                                          float currentY = pWindow->context.vec2param[0].y;
                                                          float newX = currentX, newY = currentY;

                                                          if (key == GLFW_KEY_W)
                                                          {
                                                              newX = currentX + cos(dir) * moveSpeed;
                                                              newY = currentY + sin(dir) * moveSpeed;
                                                              if (canMoveTo(newX, newY)) { pWindow->context.vec2param[0].x = newX; pWindow->context.vec2param[0].y = newY; }
                                                              else if (canMoveTo(newX, currentY)) { pWindow->context.vec2param[0].x = newX; }
                                                              else if (canMoveTo(currentX, newY)) { pWindow->context.vec2param[0].y = newY; }
                                                              return false;
                                                          }
                                                          if (key == GLFW_KEY_S)
                                                          {
                                                              newX = currentX - cos(dir) * moveSpeed;
                                                              newY = currentY - sin(dir) * moveSpeed;
                                                              if (canMoveTo(newX, newY)) { pWindow->context.vec2param[0].x = newX; pWindow->context.vec2param[0].y = newY; }
                                                              else if (canMoveTo(newX, currentY)) { pWindow->context.vec2param[0].x = newX; }
                                                              else if (canMoveTo(currentX, newY)) { pWindow->context.vec2param[0].y = newY; }
                                                              return false;
                                                          }
                                                          if (key == GLFW_KEY_A)
                                                          {
                                                              newX = currentX + sin(dir) * moveSpeed;
                                                              newY = currentY - cos(dir) * moveSpeed;
                                                              if (canMoveTo(newX, newY)) { pWindow->context.vec2param[0].x = newX; pWindow->context.vec2param[0].y = newY; }
                                                              else if (canMoveTo(newX, currentY)) { pWindow->context.vec2param[0].x = newX; }
                                                              else if (canMoveTo(currentX, newY)) { pWindow->context.vec2param[0].y = newY; }
                                                              return false;
                                                          }
                                                          if (key == GLFW_KEY_D)
                                                          {
                                                              newX = currentX - sin(dir) * moveSpeed;
                                                              newY = currentY + cos(dir) * moveSpeed;
                                                              if (canMoveTo(newX, newY)) { pWindow->context.vec2param[0].x = newX; pWindow->context.vec2param[0].y = newY; }
                                                              else if (canMoveTo(newX, currentY)) { pWindow->context.vec2param[0].x = newX; }
                                                              else if (canMoveTo(currentX, newY)) { pWindow->context.vec2param[0].y = newY; }
                                                              return false;
                                                          }
                                                          return true;
                                                      },

                                                      // MousebuttonCallback
                                                      [](cg::ManagedWindow *pWindow, int button, int xpos, int ypos) {},

                                                      // ScrollCallback
                                                      [](cg::ManagedWindow *pWindow, double xoffset, double yoffset, int mods) {},

                                                      // CursorposCallback
                                                      [](cg::ManagedWindow *pWindow, int xpos, int ypos, double xoffset, double yoffset, int mods)
                                                      {
                                                          if (pWindow->context.input.lbutton_down)
                                                              pWindow->context.floatparam[0] += static_cast<float>(xoffset) * 0.01f;
                                                      },

                                                      // MouseclickCallback (A11 new)
                                                      nullptr,

                                                      // MousedoubleclickCallback (A11 new)
                                                      nullptr},
                                                     pMainWindow));

        // ====================================================================
        // SUBWINDOW 3: Assignment 3 (Ray Tracing)
        // ====================================================================
        subwindows.push_back(windowManager.addWindow({cg::State::SHOW,
                                                      pMainWindow,
                                                      {2, GRID_SIZE, GRID_GAP},
                                                      "Assignment 3 - Ray Tracing",
                                                      NONE,
                                                      NONE,
                                                      NONE,
                                                      {
                                                       {
                                                        {NOMARGIN},
                                                        NONE,
                                                        DISABLE,
                                                        {{QUAD}},
                                                        NONE,
                                                        NONE,
                                                        DEFAULT,
                                                        {
#include "Shaders/Shader.vert.glsl"
                                                            ,
#include "Shaders/ShaderA3.frag.glsl"
                                                        }}},

                                                      // FocusCallback
                                                      [](cg::ManagedWindow *pWindow, int focused)
                                                      {
                                                          if (focused)
                                                              std::cout << "Subwindow 3 (Assignment 3 - Ray Tracing) focused" << std::endl;
                                                      },

                                                      // InitCallback
                                                      [](cg::ManagedWindow *pWindow, int pass) {},

                                                      // SizeCallback
                                                      [](cg::ManagedWindow *pWindow, int width, int height) {},

                                                      // KeyCallback
                                                      [](cg::ManagedWindow *pWindow, int key) { return true; },

                                                      // MousebuttonCallback
                                                      [](cg::ManagedWindow *pWindow, int button, int xpos, int ypos) {},

                                                      // ScrollCallback
                                                      [](cg::ManagedWindow *pWindow, double xoffset, double yoffset, int mods) {},

                                                      // CursorposCallback
                                                      [](cg::ManagedWindow *pWindow, int xpos, int ypos, double xoffset, double yoffset, int mods) {},

                                                      // MouseclickCallback (A11 new)
                                                      nullptr,

                                                      // MousedoubleclickCallback (A11 new)
                                                      nullptr},
                                                     pMainWindow));

        // ====================================================================
        // SUBWINDOW 4: Assignment 4 (Blinn-Phong Shading)
        // ====================================================================
#define A4_PLANE_OBJECTS                                                                                                                                           \
    {PLANE, PHONG_MATERIAL_BLUE, glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -2.0f, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(A4_PLANE_SCALE)), VISIBLE}, \
        {SPHERE, PHONG_MATERIAL_ORANGE, glm::translate(glm::mat4(1.0f), glm::vec3(-A4_OBJECT_DISTANCE, 0.0f, 0.0f)), VISIBLE},                                     \
        {CUBE, PHONG_MATERIAL_ORANGE, glm::translate(glm::mat4(1.0f), glm::vec3(A4_OBJECT_DISTANCE, 0.0f, 0.0f)), VISIBLE},                                        \
        {TORUS, PHONG_MATERIAL_ORANGE, glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -A4_OBJECT_DISTANCE)), VISIBLE},                                      \
        {TEAPOT, PHONG_MATERIAL_ORANGE, glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, A4_OBJECT_DISTANCE)), VISIBLE}

        subwindows.push_back(windowManager.addWindow({cg::State::SHOW,
                                                      pMainWindow,
                                                      {3, GRID_SIZE, GRID_GAP},
                                                      "Assignment 4 - Blinn-Phong",
                                                      BLACK,
                                                      NONE,
                                                      NONE,
                                                      {
                                                       {
                                                        {NOMARGIN},
                                                        BLACK,
                                                        ENABLE,
                                                        {A4_PLANE_OBJECTS},
                                                        {DEFAULT},
                                                        {{{0.0f, 5.0f, 5.0f}}},
                                                        DEFAULT,
                                                        {
#include "Shaders/ShaderA4.vert.glsl"
                                                            ,
#include "Shaders/ShaderA4.frag.glsl"
                                                        }}},

                                                      // FocusCallback
                                                      [](cg::ManagedWindow *pWindow, int focused)
                                                      {
                                                          if (focused)
                                                              std::cout << "Subwindow 4 (Assignment 4 - Blinn-Phong) focused" << std::endl;
                                                      },

                                                      // InitCallback
                                                      [](cg::ManagedWindow *pWindow, int pass)
                                                      {
                                                          if (pass == 0)
                                                          {
                                                              auto &shader = pWindow->passes[0].renderShader;
                                                              shader.use();
                                                              float time = static_cast<float>(glfwGetTime());
                                                              const float rotationSpeed = 1.0f;
                                                              const float scaleSpeed = 1.5f;
                                                              auto &camera = pWindow->passes[0].cameras[0];
                                                              auto &light = pWindow->passes[0].lights[0];
                                                              float aspect = pWindow->width > 0 && pWindow->height > 0
                                                                                 ? static_cast<float>(pWindow->width) / static_cast<float>(pWindow->height)
                                                                                 : 1.0f;
                                                              shader.setUniform("V", camera.getLookAt());
                                                              shader.setUniform("P", camera.getPerspective(aspect));
                                                              light.setUniforms(&shader, 0);

                                                              auto &objects = pWindow->passes[0].objects;
                                                              if (objects.size() > 1 && !objects[1].instances.empty())
                                                              {
                                                                  float scale = 0.5f + 0.3f * sin(time * scaleSpeed);
                                                                  glm::mat4 basePos = glm::translate(glm::mat4(1.0f), glm::vec3(-A4_OBJECT_DISTANCE, 0.0f, 0.0f));
                                                                  objects[1].instances[0].modeltransform = basePos * glm::scale(glm::mat4(1.0f), glm::vec3(scale));
                                                              }
                                                              if (objects.size() > 2 && !objects[2].instances.empty())
                                                              {
                                                                  float angle = time * rotationSpeed * 0.8f;
                                                                  glm::mat4 basePos = glm::translate(glm::mat4(1.0f), glm::vec3(A4_OBJECT_DISTANCE, 0.0f, 0.0f));
                                                                  objects[2].instances[0].modeltransform = basePos * glm::rotate(glm::mat4(1.0f), angle, glm::normalize(glm::vec3(1.0f, 1.0f, 1.0f)));
                                                              }
                                                              if (objects.size() > 3 && !objects[3].instances.empty())
                                                              {
                                                                  float angle = time * rotationSpeed;
                                                                  glm::mat4 basePos = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -A4_OBJECT_DISTANCE));
                                                                  objects[3].instances[0].modeltransform = basePos * glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 1.0f, 0.0f));
                                                              }
                                                              if (objects.size() > 4 && !objects[4].instances.empty())
                                                              {
                                                                  float angle = time * rotationSpeed * 0.6f;
                                                                  glm::mat4 basePos = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, A4_OBJECT_DISTANCE));
                                                                  objects[4].instances[0].modeltransform = basePos * glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 1.0f, 0.0f));
                                                              }
                                                          }
                                                      },

                                                      // SizeCallback
                                                      [](cg::ManagedWindow *pWindow, int width, int height) {},

                                                      // KeyCallback
                                                      [](cg::ManagedWindow *pWindow, int key) { return true; },

                                                      // MousebuttonCallback
                                                      [](cg::ManagedWindow *pWindow, int button, int xpos, int ypos) {},

                                                      // ScrollCallback
                                                      [](cg::ManagedWindow *pWindow, double xoffset, double yoffset, int mods)
                                                      {
                                                          if (mods & GLFW_MOD_SHIFT)
                                                              pWindow->passes[0].lights[0].onScroll(xoffset, yoffset, 0);
                                                          else
                                                              pWindow->passes[0].cameras[0].onScroll(xoffset, yoffset);
                                                      },

                                                      // CursorposCallback
                                                      [](cg::ManagedWindow *pWindow, int xpos, int ypos, double xoffset, double yoffset, int mods)
                                                      {
                                                          if (pWindow->context.input.lbutton_down)
                                                          {
                                                              if (mods & GLFW_MOD_SHIFT)
                                                                  pWindow->passes[0].lights[0].onCursor(xoffset, yoffset, 0);
                                                              else
                                                                  pWindow->passes[0].cameras[0].onCursor(xoffset, yoffset);
                                                          }
                                                      },

                                                      // MouseclickCallback (A11 new)
                                                      nullptr,

                                                      // MousedoubleclickCallback (A11 new)
                                                      nullptr},
                                                     pMainWindow));

        // ====================================================================
        // SUBWINDOW 5: Assignment 5 (Environment Mapping)
        // ====================================================================
        subwindows.push_back(windowManager.addWindow({cg::State::SHOW,
                                                      pMainWindow,
                                                      {4, GRID_SIZE, GRID_GAP},
                                                      "Assignment 5 - Environment Mapping",
                                                      BLACK,
                                                      NONE,
                                                      NONE,
                                                      {
                                                       {
                                                        {NOMARGIN},
                                                        NONE,
                                                        ENABLE,
                                                        {{SKYBOX}},
                                                        {{{3.0f, 2.0f, 0.0f}}},
                                                        NONE,
                                                        DEFAULT,
                                                        {
#include "Shaders/ShaderA5.vert.glsl"
                                                            ,
#include "Shaders/ShaderA5.frag.glsl"
                                                        }},
                                                       {
                                                        {NOMARGIN},
                                                        NONE,
                                                        ENABLE,
                                                        DEMOOBJECTS_ENVIRONMENTMAPPING,
                                                        {{{3.0f, 2.0f, 0.0f}}},
                                                        NONE,
                                                        DEFAULT,
                                                        {
#include "Shaders/ShaderA5Obj.vert.glsl"
                                                            ,
#include "Shaders/ShaderA5Obj.frag.glsl"
                                                        }}},

                                                      // FocusCallback
                                                      [](cg::ManagedWindow *pWindow, int focused)
                                                      {
                                                          if (focused)
                                                              std::cout << "Subwindow 5 (Assignment 5 - Environment Mapping) focused" << std::endl;
                                                      },

                                                      // InitCallback
                                                      [](cg::ManagedWindow *pWindow, int pass)
                                                      {
                                                          if (pass == -1)
                                                          {
                                                              pWindow->context.floatparam[0] = 0.9f;
                                                              pWindow->context.floatparam[1] = 0.67f;
                                                              pWindow->context.intparam[0] = 0;
                                                              for (auto &object : pWindow->passes[1].objects)
                                                              {
                                                                  object.instances[0].material.params["material.reflectionFactor"] = {pWindow->context.floatparam[0]};
                                                                  object.instances[0].material.params["material.eta"] = {pWindow->context.floatparam[1]};
                                                              }
                                                          }
                                                          if (pass == 0 || pass == 1)
                                                          {
                                                              auto &shader = pWindow->passes[pass].renderShader;
                                                              shader.use();
                                                              auto &camera = pWindow->passes[pass].cameras[0];
                                                              float aspect = pWindow->width > 0 && pWindow->height > 0
                                                                                 ? static_cast<float>(pWindow->width) / static_cast<float>(pWindow->height)
                                                                                 : 1.0f;
                                                              shader.setUniform("V", camera.getLookAt());
                                                              shader.setUniform("P", camera.getPerspective(aspect));
                                                              if (pass == 1)
                                                              {
                                                                  glm::mat4 invV = glm::inverse(camera.getLookAt());
                                                                  shader.setUniform("camera.position", glm::vec3(invV[3]));
                                                              }
                                                          }
                                                      },

                                                      // SizeCallback
                                                      [](cg::ManagedWindow *pWindow, int width, int height) {},

                                                      // KeyCallback
                                                      [](cg::ManagedWindow *pWindow, int key)
                                                      {
                                                          int newObject = -1;
                                                          if (key == GLFW_KEY_1) newObject = 0;
                                                          else if (key == GLFW_KEY_2) newObject = 1;
                                                          else if (key == GLFW_KEY_3) newObject = 2;
                                                          else if (key == GLFW_KEY_4) newObject = 3;
                                                          if (newObject >= 0 && (int)pWindow->passes[1].objects.size() > newObject && newObject != pWindow->context.intparam[0])
                                                          {
                                                              pWindow->passes[1].objects[pWindow->context.intparam[0]].instances[0].isVisible = false;
                                                              pWindow->passes[1].objects[newObject].instances[0].isVisible = true;
                                                              pWindow->context.intparam[0] = newObject;
                                                              return false;
                                                          }
                                                          return true;
                                                      },

                                                      // MousebuttonCallback
                                                      [](cg::ManagedWindow *pWindow, int button, int xpos, int ypos) {},

                                                      // ScrollCallback
                                                      [](cg::ManagedWindow *pWindow, double xoffset, double yoffset, int mods)
                                                      {
                                                          pWindow->passes[0].cameras[0].onScroll(xoffset, yoffset);
                                                          pWindow->passes[1].cameras[0].onScroll(xoffset, yoffset);
                                                      },

                                                      // CursorposCallback
                                                      [](cg::ManagedWindow *pWindow, int xpos, int ypos, double xoffset, double yoffset, int mods)
                                                      {
                                                          if (pWindow->context.input.lbutton_down)
                                                          {
                                                              pWindow->passes[0].cameras[0].onCursor(xoffset, yoffset);
                                                              pWindow->passes[1].cameras[0].onCursor(xoffset, yoffset);
                                                          }
                                                      },

                                                      // MouseclickCallback (A11 new)
                                                      nullptr,

                                                      // MousedoubleclickCallback (A11 new)
                                                      nullptr},
                                                     pMainWindow));

        // ====================================================================
        // SUBWINDOW 6: Assignment 8 (Unified Rendering)
        // ====================================================================
#define FOG_COLOR {0.05f, 0.05f, 0.08f, 1.0f}
        subwindows.push_back(windowManager.addWindow({cg::State::SHOW,
                                                      pMainWindow,
                                                      {5, GRID_SIZE, GRID_GAP},
                                                      "Assignment 8 - Unified Rendering",
                                                      {0.05f, 0.05f, 0.08f, 1.0f},
                                                      {{"../NightSky/night_posx.png",
                                                        "../NightSky/night_negx.png",
                                                        "../NightSky/night_posy.png",
                                                        "../NightSky/night_negy.png",
                                                        "../NightSky/night_posz.png",
                                                        "../NightSky/night_negz.png"}},
                                                      NONE,
                                                      {
                                                       {
                                                        {NOMARGIN},
                                                        NONE,
                                                        ENABLE,
                                                        {{SKYBOX}},
                                                        {{{0.0f, 1.0f, 4.0f}}},
                                                        NONE,
                                                        DEFAULT,
                                                        {
#include "Shaders/ShaderA8Sky.vert.glsl"
                                                            ,
#include "Shaders/ShaderA8Sky.frag.glsl"
                                                        }},
                                                       {
                                                        {NOMARGIN},
                                                        NONE,
                                                        ENABLE,
                                                        DEMOOBJECTS_A8,
                                                        {{{0.0f, 1.0f, 4.0f}}},
                                                        {{{0.0f, 3.0f, 5.0f}}},
                                                        DEFAULT,
                                                        {
#include "Shaders/ShaderA8.vert.glsl"
                                                            ,
#include "Shaders/ShaderA8.frag.glsl"
                                                        }}},

                                                      // FocusCallback
                                                      [](cg::ManagedWindow *pWindow, int focused)
                                                      {
                                                          if (focused)
                                                              std::cout << "Subwindow 6 (Assignment 8 - Unified Rendering) focused" << std::endl;
                                                      },

                                                      // InitCallback
                                                      [](cg::ManagedWindow *pWindow, int pass)
                                                      {
                                                          if (pass == -1)
                                                          {
                                                              pWindow->context.intparam[0] = 0;
                                                              pWindow->context.floatparam[0] = 2.0f;
                                                              pWindow->context.floatparam[1] = 15.0f;
                                                              pWindow->context.vec4param[0] = glm::vec4(0.05f, 0.05f, 0.08f, 1.0f);
                                                              pWindow->passes[1].renderShader.use();
                                                              pWindow->passes[1].renderShader.setUniform("fog.minDist", pWindow->context.floatparam[0]);
                                                              pWindow->passes[1].renderShader.setUniform("fog.maxDist", pWindow->context.floatparam[1]);
                                                              pWindow->passes[1].renderShader.setUniform("fog.color", pWindow->context.vec4param[0]);
                                                              pWindow->passes[0].renderShader.use();
                                                              pWindow->passes[0].renderShader.setUniform("fog.minDist", pWindow->context.floatparam[0]);
                                                              pWindow->passes[0].renderShader.setUniform("fog.maxDist", pWindow->context.floatparam[1]);
                                                              pWindow->passes[0].renderShader.setUniform("fog.color", pWindow->context.vec4param[0]);
                                                          }
                                                          if (pass == 0 || pass == 1)
                                                          {
                                                              auto &shader = pWindow->passes[pass].renderShader;
                                                              shader.use();
                                                              auto &camera = pWindow->passes[pass].cameras[0];
                                                              float aspect = pWindow->width > 0 && pWindow->height > 0
                                                                                 ? static_cast<float>(pWindow->width) / static_cast<float>(pWindow->height)
                                                                                 : 1.0f;
                                                              shader.setUniform("V", camera.getLookAt());
                                                              shader.setUniform("P", camera.getPerspective(aspect));
                                                              if (pass == 0)
                                                                  shader.setUniform("CubeMapTex", 0);
                                                          }
                                                      },

                                                      // SizeCallback
                                                      [](cg::ManagedWindow *pWindow, int width, int height) {},

                                                      // KeyCallback
                                                      [](cg::ManagedWindow *pWindow, int key)
                                                      {
                                                          if (key == GLFW_KEY_W)
                                                          {
                                                              pWindow->passes[0].cameras[0].setType(cg::CameraType::WALKING);
                                                              pWindow->passes[1].cameras[0].setType(cg::CameraType::WALKING);
                                                              pWindow->context.intparam[0] = 1;
                                                              return false;
                                                          }
                                                          else if (key == GLFW_KEY_O)
                                                          {
                                                              pWindow->passes[0].cameras[0].setType(cg::CameraType::ORBITING);
                                                              pWindow->passes[1].cameras[0].setType(cg::CameraType::ORBITING);
                                                              pWindow->context.intparam[0] = 0;
                                                              return false;
                                                          }
                                                          return true;
                                                      },

                                                      // MousebuttonCallback
                                                      [](cg::ManagedWindow *pWindow, int button, int xpos, int ypos) {},

                                                      // ScrollCallback
                                                      [](cg::ManagedWindow *pWindow, double xoffset, double yoffset, int mods)
                                                      {
                                                          if (mods & GLFW_MOD_SHIFT)
                                                              pWindow->passes[1].lights[0].onScroll(xoffset, yoffset);
                                                          else if (mods & GLFW_MOD_ALT)
                                                          {
                                                              pWindow->context.floatparam[1] += (yoffset > 0.0 ? 1.0f : -1.0f);
                                                              pWindow->context.floatparam[1] = glm::clamp(pWindow->context.floatparam[1], 5.0f, 30.0f);
                                                              pWindow->passes[1].renderShader.use();
                                                              pWindow->passes[1].renderShader.setUniform("fog.maxDist", pWindow->context.floatparam[1]);
                                                              pWindow->passes[0].renderShader.use();
                                                              pWindow->passes[0].renderShader.setUniform("fog.maxDist", pWindow->context.floatparam[1]);
                                                          }
                                                          else
                                                          {
                                                              pWindow->passes[0].cameras[0].onScroll(xoffset, yoffset);
                                                              pWindow->passes[1].cameras[0].onScroll(xoffset, yoffset);
                                                          }
                                                      },

                                                      // CursorposCallback
                                                      [](cg::ManagedWindow *pWindow, int xpos, int ypos, double xoffset, double yoffset, int mods)
                                                      {
                                                          if (pWindow->context.input.lbutton_down)
                                                          {
                                                              if (mods & GLFW_MOD_SHIFT)
                                                                  pWindow->passes[1].lights[0].onCursor(xoffset, yoffset);
                                                              else
                                                              {
                                                                  pWindow->passes[0].cameras[0].onCursor(xoffset, yoffset);
                                                                  pWindow->passes[1].cameras[0].onCursor(xoffset, yoffset);
                                                              }
                                                          }
                                                      },

                                                      // MouseclickCallback (A11 new)
                                                      nullptr,

                                                      // MousedoubleclickCallback (A11 new)
                                                      nullptr},
                                                     pMainWindow));

        // ====================================================================
        // SUBWINDOW 7: Assignment 9 (Cel-Shading Pipeline)
        // ====================================================================
        subwindows.push_back(windowManager.addWindow({cg::State::SHOW,
                                                      pMainWindow,
                                                      {6, GRID_SIZE, GRID_GAP},
                                                      "Assignment 9 - Cel-Shading",
                                                      {0.05f, 0.05f, 0.08f, 1.0f},
                                                      NONE,
                                                      NONE,
                                                      {
                                                       {  // Pass 0 - Blinn-Phong scene
                                                        {NOMARGIN},
                                                        {0.05f, 0.05f, 0.08f, 1.0f},
                                                        ENABLE,
                                                        DEMOOBJECTS_A8,
                                                        {{{0.0f, 1.0f, 4.0f}}},
                                                        {{{0.0f, 3.0f, 5.0f}}},
                                                        DEFAULT,
                                                        {
#include "Shaders/ShaderA8.vert.glsl"
                                                            ,
#include "Shaders/ShaderA9CelPhong.frag.glsl"
                                                        }},
                                                       {  // Pass 1 - Vertical blur
                                                        {NOMARGIN},
                                                        NONE,
                                                        DISABLE,
                                                        {{QUAD}},
                                                        NONE,
                                                        NONE,
                                                        DEFAULT,
                                                        {
#include "Shaders/Shader.vert.glsl"
                                                            ,
#include "Shaders/ShaderA9CelBlurV.frag.glsl"
                                                        }},
                                                       {  // Pass 2 - Horizontal blur
                                                        {NOMARGIN},
                                                        NONE,
                                                        DISABLE,
                                                        {{QUAD}},
                                                        NONE,
                                                        NONE,
                                                        DEFAULT,
                                                        {
#include "Shaders/Shader.vert.glsl"
                                                            ,
#include "Shaders/ShaderA9CelBlurH.frag.glsl"
                                                        }},
                                                       {  // Pass 3 - Sobel edge detection
                                                        {NOMARGIN},
                                                        NONE,
                                                        DISABLE,
                                                        {{QUAD}},
                                                        NONE,
                                                        NONE,
                                                        DEFAULT,
                                                        {
#include "Shaders/Shader.vert.glsl"
                                                            ,
#include "Shaders/ShaderA9CelSobel.frag.glsl"
                                                        }},
                                                       {  // Pass 4 - Cartoon shading
                                                        {NOMARGIN},
                                                        {0.05f, 0.05f, 0.08f, 1.0f},
                                                        ENABLE,
                                                        DEMOOBJECTS_A8,
                                                        {{{0.0f, 1.0f, 4.0f}}},
                                                        {{{0.0f, 3.0f, 5.0f}}},
                                                        DEFAULT,
                                                        {
#include "Shaders/ShaderA8.vert.glsl"
                                                            ,
#include "Shaders/ShaderA9CelCartoon.frag.glsl"
                                                        }},
                                                       {  // Pass 5 - Final composite
                                                        {NOMARGIN},
                                                        NONE,
                                                        DISABLE,
                                                        {{QUAD}},
                                                        NONE,
                                                        NONE,
                                                        DEFAULT,
                                                        {
#include "Shaders/Shader.vert.glsl"
                                                            ,
#include "Shaders/ShaderA9CelComposite.frag.glsl"
                                                        }}},

                                                      // FocusCallback
                                                      [](cg::ManagedWindow *pWindow, int focused)
                                                      {
                                                          if (focused)
                                                              std::cout << "Subwindow 7 (Assignment 9 - Cel-Shading) focused" << std::endl;
                                                      },

                                                      // InitCallback
                                                      [](cg::ManagedWindow *pWindow, int pass)
                                                      {
                                                          if (pass == -1)
                                                          {
                                                              pWindow->context.intparam[1] = 1;
                                                              pWindow->context.intparam[2] = 1;
                                                              pWindow->context.floatparam[0] = 2.0f;
                                                              pWindow->context.floatparam[1] = 15.0f;
                                                              pWindow->context.floatparam[2] = 0.05f;
                                                              pWindow->context.floatparam[3] = 1.0f;
                                                              pWindow->context.vec4param[0] = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
                                                              pWindow->passes[0].fbo = cg::FBO(cg::FBO::Texture, GL_TEXTURE1, &pWindow->context.gluintparam[0]);
                                                              pWindow->passes[1].fbo = cg::FBO(cg::FBO::Texture, GL_TEXTURE2, &pWindow->context.gluintparam[1]);
                                                              pWindow->passes[2].fbo = cg::FBO(cg::FBO::Texture, GL_TEXTURE3, &pWindow->context.gluintparam[2]);
                                                              pWindow->passes[3].fbo = cg::FBO(cg::FBO::Texture, GL_TEXTURE4, &pWindow->context.gluintparam[3]);
                                                              pWindow->passes[4].fbo = cg::FBO(cg::FBO::Texture, GL_TEXTURE5, &pWindow->context.gluintparam[4]);
                                                              pWindow->passes[0].renderShader.use();
                                                              pWindow->passes[0].renderShader.setUniform("fog.minDist", pWindow->context.floatparam[0]);
                                                              pWindow->passes[0].renderShader.setUniform("fog.maxDist", pWindow->context.floatparam[1]);
                                                              pWindow->passes[0].renderShader.setUniform("fog.color", pWindow->context.vec4param[0]);
                                                              pWindow->passes[4].renderShader.use();
                                                              pWindow->passes[4].renderShader.setUniform("fog.minDist", pWindow->context.floatparam[0]);
                                                              pWindow->passes[4].renderShader.setUniform("fog.maxDist", pWindow->context.floatparam[1]);
                                                              pWindow->passes[4].renderShader.setUniform("fog.color", pWindow->context.vec4param[0]);
                                                          }
                                                          if (pass == 0)
                                                          {
                                                              if (pWindow->context.intparam[2] == 1)
                                                                  pWindow->passes[0].backgroundColor = {1.0f, 1.0f, 1.0f, 1.0f};
                                                              else
                                                                  pWindow->passes[0].backgroundColor = {0.05f, 0.05f, 0.08f, 1.0f};
                                                              auto &shader = pWindow->passes[0].renderShader;
                                                              shader.use();
                                                              auto &camera = pWindow->passes[0].cameras[0];
                                                              float aspect = pWindow->width > 0 && pWindow->height > 0
                                                                                 ? static_cast<float>(pWindow->width) / static_cast<float>(pWindow->height)
                                                                                 : 1.0f;
                                                              shader.setUniform("V", camera.getLookAt());
                                                              shader.setUniform("P", camera.getPerspective(aspect));
                                                          }
                                                          if (pass == 1)
                                                          {
                                                              pWindow->passes[1].renderShader.use();
                                                              glActiveTexture(GL_TEXTURE0);
                                                              glBindTexture(GL_TEXTURE_2D, pWindow->context.gluintparam[0]);
                                                          }
                                                          if (pass == 2)
                                                          {
                                                              pWindow->passes[2].renderShader.use();
                                                              glActiveTexture(GL_TEXTURE0);
                                                              glBindTexture(GL_TEXTURE_2D, pWindow->context.gluintparam[1]);
                                                          }
                                                          if (pass == 3)
                                                          {
                                                              pWindow->passes[3].renderShader.use();
                                                              glActiveTexture(GL_TEXTURE0);
                                                              if (pWindow->context.intparam[1] == 1)
                                                                  glBindTexture(GL_TEXTURE_2D, pWindow->context.gluintparam[2]);
                                                              else
                                                                  glBindTexture(GL_TEXTURE_2D, pWindow->context.gluintparam[0]);
                                                              pWindow->passes[3].renderShader.setUniform("EdgeThreshold", pWindow->context.floatparam[2]);
                                                          }
                                                          if (pass == 4)
                                                          {
                                                              if (pWindow->context.intparam[2] == 1)
                                                                  pWindow->passes[4].backgroundColor = {1.0f, 1.0f, 1.0f, 1.0f};
                                                              else
                                                                  pWindow->passes[4].backgroundColor = {0.05f, 0.05f, 0.08f, 1.0f};
                                                              auto &shader = pWindow->passes[4].renderShader;
                                                              shader.use();
                                                              auto &camera = pWindow->passes[4].cameras[0];
                                                              float aspect = pWindow->width > 0 && pWindow->height > 0
                                                                                 ? static_cast<float>(pWindow->width) / static_cast<float>(pWindow->height)
                                                                                 : 1.0f;
                                                              shader.setUniform("V", camera.getLookAt());
                                                              shader.setUniform("P", camera.getPerspective(aspect));
                                                          }
                                                          if (pass == 5)
                                                          {
                                                              pWindow->passes[5].renderShader.use();
                                                              glActiveTexture(GL_TEXTURE0);
                                                              glBindTexture(GL_TEXTURE_2D, pWindow->context.gluintparam[3]);
                                                              glActiveTexture(GL_TEXTURE1);
                                                              glBindTexture(GL_TEXTURE_2D, pWindow->context.gluintparam[4]);
                                                              pWindow->passes[5].renderShader.setUniform("AlphaFactor", pWindow->context.floatparam[3]);
                                                              pWindow->passes[5].renderShader.setUniform("EdgeEnabled", pWindow->context.intparam[2]);
                                                          }
                                                      },

                                                      // SizeCallback
                                                      [](cg::ManagedWindow *pWindow, int width, int height) {},

                                                      // KeyCallback
                                                      [](cg::ManagedWindow *pWindow, int key)
                                                      {
                                                          if (key == GLFW_KEY_1) { pWindow->context.intparam[1] = 1; return false; }
                                                          else if (key == GLFW_KEY_2) { pWindow->context.intparam[1] = 0; return false; }
                                                          else if (key == GLFW_KEY_3)
                                                          {
                                                              pWindow->context.intparam[2] = 1;
                                                              pWindow->context.vec4param[0] = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
                                                              pWindow->passes[0].renderShader.use(); pWindow->passes[0].renderShader.setUniform("fog.color", pWindow->context.vec4param[0]);
                                                              pWindow->passes[4].renderShader.use(); pWindow->passes[4].renderShader.setUniform("fog.color", pWindow->context.vec4param[0]);
                                                              return false;
                                                          }
                                                          else if (key == GLFW_KEY_4)
                                                          {
                                                              pWindow->context.intparam[2] = 0;
                                                              pWindow->context.vec4param[0] = glm::vec4(0.05f, 0.05f, 0.08f, 1.0f);
                                                              pWindow->passes[0].renderShader.use(); pWindow->passes[0].renderShader.setUniform("fog.color", pWindow->context.vec4param[0]);
                                                              pWindow->passes[4].renderShader.use(); pWindow->passes[4].renderShader.setUniform("fog.color", pWindow->context.vec4param[0]);
                                                              return false;
                                                          }
                                                          return true;
                                                      },

                                                      // MousebuttonCallback
                                                      [](cg::ManagedWindow *pWindow, int button, int xpos, int ypos) {},

                                                      // ScrollCallback
                                                      [](cg::ManagedWindow *pWindow, double xoffset, double yoffset, int mods)
                                                      {
                                                          if (mods & GLFW_MOD_SHIFT)
                                                          {
                                                              pWindow->passes[0].lights[0].onScroll(xoffset, yoffset);
                                                              pWindow->passes[4].lights[0].onScroll(xoffset, yoffset);
                                                          }
                                                          else if (mods & GLFW_MOD_ALT)
                                                          {
                                                              pWindow->context.floatparam[2] += (yoffset > 0.0 ? 0.01f : -0.01f);
                                                              pWindow->context.floatparam[2] = glm::clamp(pWindow->context.floatparam[2], 0.01f, 1.0f);
                                                          }
                                                          else if (mods & GLFW_MOD_CONTROL)
                                                          {
                                                              pWindow->context.floatparam[3] += (yoffset > 0.0 ? 0.05f : -0.05f);
                                                              pWindow->context.floatparam[3] = glm::clamp(pWindow->context.floatparam[3], 0.0f, 1.0f);
                                                          }
                                                          else
                                                          {
                                                              pWindow->passes[0].cameras[0].onScroll(xoffset, yoffset);
                                                              pWindow->passes[4].cameras[0].onScroll(xoffset, yoffset);
                                                          }
                                                      },

                                                      // CursorposCallback
                                                      [](cg::ManagedWindow *pWindow, int xpos, int ypos, double xoffset, double yoffset, int mods)
                                                      {
                                                          if (pWindow->context.input.lbutton_down)
                                                          {
                                                              if (mods & GLFW_MOD_SHIFT)
                                                              {
                                                                  pWindow->passes[0].lights[0].onCursor(xoffset, yoffset);
                                                                  pWindow->passes[4].lights[0].onCursor(xoffset, yoffset);
                                                              }
                                                              else
                                                              {
                                                                  pWindow->passes[0].cameras[0].onCursor(xoffset, yoffset);
                                                                  pWindow->passes[4].cameras[0].onCursor(xoffset, yoffset);
                                                              }
                                                          }
                                                      },

                                                      // MouseclickCallback (A11 new)
                                                      nullptr,

                                                      // MousedoubleclickCallback (A11 new)
                                                      nullptr},
                                                     pMainWindow));

        // ====================================================================
        // SUBWINDOW 8: Assignment 10 (Portal with Skybox)
        // ====================================================================
        {
            subwindows.push_back(windowManager.addWindow({cg::State::SHOW,
                                                          pMainWindow,
                                                          {7, GRID_SIZE, GRID_GAP},
                                                          "Assignment 10 - Portal",
                                                          BLACK,
                                                          {{"../NightSky/night_posx.png",
                                                            "../NightSky/night_negx.png",
                                                            "../NightSky/night_posy.png",
                                                            "../NightSky/night_negy.png",
                                                            "../NightSky/night_posz.png",
                                                            "../NightSky/night_negz.png"}},
                                                          NONE,
                                                          {
                                                           {  // Pass 0 - Portal texture to FBO
                                                            {NOMARGIN},
                                                            BLACK,
                                                            DISABLE,
                                                            {{QUAD}},
                                                            NONE,
                                                            NONE,
                                                            DEFAULT,
                                                            {
#include "Shaders/Shader.vert.glsl"
                                                                ,
#include "Shaders/ShaderA10Portal.frag.glsl"
                                                            }},
                                                           {  // Pass 1 - Skybox
                                                            {NOMARGIN},
                                                            BLACK,
                                                            ENABLE,
                                                            {{SKYBOX}},
                                                            {{{0.0f, 1.0f, 4.0f}}},
                                                            NONE,
                                                            DEFAULT,
                                                            {
#include "Shaders/ShaderA10Sky.vert.glsl"
                                                                ,
#include "Shaders/ShaderA10Sky.frag.glsl"
                                                            }},
                                                           {  // Pass 2 - Portal overlay
                                                            {NOMARGIN},
                                                            NONE,
                                                            DISABLE,
                                                            {{QUAD}},
                                                            NONE,
                                                            NONE,
                                                            DEFAULT,
                                                            {
#include "Shaders/Shader.vert.glsl"
                                                                ,
#include "Shaders/ShaderA10Scene.frag.glsl"
                                                            }}},

                                                          // FocusCallback
                                                          [](cg::ManagedWindow *pWindow, int focused)
                                                          {
                                                              if (focused)
                                                                  std::cout << "Subwindow 8 (Assignment 10 - Portal) focused" << std::endl;
                                                          },

                                                          // InitCallback
                                                          [](cg::ManagedWindow *pWindow, int pass)
                                                          {
                                                              if (pass == -1)
                                                              {
                                                                  pWindow->passes[1].cameras[0].setType(cg::CameraType::WALKING);
                                                                  pWindow->context.vec4param[0] = glm::vec4(0.0f, 0.0f, -1.0f, 1.0f);
                                                                  pWindow->context.floatparam[0] = 0.2f;
                                                                  pWindow->passes[0].fbo = cg::FBO(cg::FBO::Texture, GL_TEXTURE1, &pWindow->context.gluintparam[0]);
                                                              }
                                                              if (pass == 0)
                                                                  glDisable(GL_BLEND);
                                                              if (pass == 1)
                                                              {
                                                                  pWindow->passes[1].renderShader.use();
                                                                  pWindow->passes[1].renderShader.setUniform("CubeMapTex", 0);
                                                              }
                                                              if (pass == 2)
                                                              {
                                                                  glEnable(GL_BLEND);
                                                                  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                                                                  glActiveTexture(GL_TEXTURE1);
                                                                  glBindTexture(GL_TEXTURE_2D, pWindow->context.gluintparam[0]);
                                                                  pWindow->passes[2].renderShader.use();
                                                                  glm::vec3 portalDir = glm::vec3(pWindow->context.vec4param[0]);
                                                                  pWindow->passes[2].renderShader.setUniform("PortalDir", portalDir);
                                                                  pWindow->passes[2].renderShader.setUniform("PortalRadius", pWindow->context.floatparam[0]);
                                                                  auto &cam = pWindow->passes[1].cameras[0];
                                                                  float aspect = pWindow->width > 0 && pWindow->height > 0
                                                                                     ? static_cast<float>(pWindow->width) / static_cast<float>(pWindow->height)
                                                                                     : 1.0f;
                                                                  pWindow->passes[2].renderShader.setUniform("SkyV", cam.getLookAt());
                                                                  pWindow->passes[2].renderShader.setUniform("SkyP", cam.getPerspective(aspect));
                                                              }
                                                          },

                                                          // SizeCallback
                                                          [](cg::ManagedWindow *pWindow, int width, int height) {},

                                                          // KeyCallback
                                                          [](cg::ManagedWindow *pWindow, int key)
                                                          {
                                                              if (key == GLFW_KEY_W) { pWindow->passes[1].cameras[0].setType(cg::CameraType::WALKING); return false; }
                                                              else if (key == GLFW_KEY_O) { pWindow->passes[1].cameras[0].setType(cg::CameraType::ORBITING); return false; }
                                                              return true;
                                                          },

                                                          // MousebuttonCallback
                                                          [](cg::ManagedWindow *pWindow, int button, int xpos, int ypos)
                                                          {
                                                              if (button == GLFW_MOUSE_BUTTON_LEFT)
                                                              {
                                                                  auto &cam = pWindow->passes[1].cameras[0];
                                                                  float aspect = static_cast<float>(pWindow->width) / std::max(static_cast<float>(pWindow->height), 1.0f);
                                                                  glm::mat4 Vinv = glm::inverse(cam.getLookAt());
                                                                  glm::mat4 Pinv = glm::inverse(cam.getPerspective(aspect));
                                                                  float ndcX = (2.0f * xpos / pWindow->width) - 1.0f;
                                                                  float ndcY = 1.0f - (2.0f * ypos / pWindow->height);
                                                                  glm::vec4 clipDir = glm::vec4(ndcX, ndcY, -1.0f, 1.0f);
                                                                  glm::vec4 viewDir = Pinv * clipDir;
                                                                  viewDir = glm::vec4(viewDir.x, viewDir.y, -1.0f, 0.0f);
                                                                  glm::vec3 worldDir = glm::normalize(glm::vec3(Vinv * viewDir));
                                                                  pWindow->context.vec4param[0] = glm::vec4(worldDir, 1.0f);
                                                              }
                                                          },

                                                          // ScrollCallback
                                                          [](cg::ManagedWindow *pWindow, double xoffset, double yoffset, int mods)
                                                          {
                                                              pWindow->passes[1].cameras[0].onScroll(xoffset, yoffset);
                                                          },

                                                          // CursorposCallback
                                                          [](cg::ManagedWindow *pWindow, int xpos, int ypos, double xoffset, double yoffset, int mods)
                                                          {
                                                              if (pWindow->context.input.lbutton_down)
                                                                  pWindow->passes[1].cameras[0].onCursor(xoffset, yoffset);
                                                          },

                                                          // MouseclickCallback (A11 new)
                                                          nullptr,

                                                          // MousedoubleclickCallback (A11 new)
                                                          nullptr},
                                                         pMainWindow));
        }

        // ====================================================================
        // A11 PRIMARY WINDOW: Deferred Shading + Normal Mapping (Ogres)
        // ====================================================================
        // Pass 0: Geometry pass - renders Ogre instances to the G-buffer
        //         FBO type: Deferred (3 color attachments: position, normal, texcoord)
        //         Shader: ViewSpaceTransform.vert + DeferredShading.0.frag
        // Pass 1: Lighting pass - fullscreen quad reads G-buffer, applies Blinn-Phong
        //         with color texture sampled from stored UVs
        //         Shader: PassThrough.vert + DeferredShading.1.frag
        //
        // Window-level textures (loaded by window manager):
        //   texture[0]: ../Ogre/ogre_diffuse.png  -> bound to GL_TEXTURE0 in lighting pass
        //   texture[1]: ../Ogre/ogre_normalmap.png -> bound to GL_TEXTURE1 (not used in deferred lighting directly)
        //
        // Context:
        //   intparam[0]: Option (1=full shading, 2=normals, 3=texture)  
        //   gluintparam[0..2]: G-buffer texture IDs (position, normal, texcoord)
        subwindows.push_back(windowManager.addWindow({cg::State::SHOW,
                                                      pMainWindow,
                                                      {8, GRID_SIZE, GRID_GAP},
                                                      "Assignment 11 - Deferred Shading (Ogres)",
                                     BLACK,
                                     {cg::Texture({"../Ogre/ogre_diffuse.png"},  GL_TEXTURE4), // Tex 0: diffuse color at unit 4
                                      cg::Texture({"../Ogre/ogre_normalmap.png"}, GL_TEXTURE5)}, // Tex 1: normal map at unit 5
                                     NONE,
                                     {
                                      {  // Pass 0 - Geometry: fills G-buffer
                                       {NOMARGIN},
                                       BLACK,
                                       ENABLE,
                                       {{MAKE_OGRE_OBJECT("../Ogre/bs_ears.obj", 11, 21, 3.5f)}},
                                       {{{0.0f, 2.0f, 20.0f}}},
                                       {{{5.0f, 5.0f, 10.0f}}},
                                       {cg::FBO::Deferred,
                                        GL_TEXTURE1, nullptr,
                                        GL_TEXTURE2, nullptr,
                                        GL_TEXTURE3, nullptr},
                                       {
#include "Shaders/ViewSpaceTransform.vert.glsl"
                                           ,
#include "Shaders/DeferredShading.0.frag.glsl"
                                       }},
                                      {  // Pass 1 - Lighting: fullscreen quad reads G-buffer
                                       {NOMARGIN},
                                       NONE,
                                       DISABLE,
                                       {{QUAD}},
                                       NONE,
                                       NONE,
                                       DEFAULT,
                                       {
#include "Shaders/PassThrough.vert.glsl"
                                           ,
#include "Shaders/DeferredShading.1.frag.glsl"
                                       }}},

                                     // FocusCallback
                                     [](cg::ManagedWindow *pWindow, int focused)
                                     {
                                         if (focused)
                                         {
                                             std::cout << std::endl;
                                             std::cout << "========================================" << std::endl;
                                             std::cout << "Assignment 11 - Deferred Shading + Normal Mapping (Ogres)" << std::endl;
                                             std::cout << "========================================" << std::endl;
                                             std::cout << std::endl;
                                             std::cout << "Deferred Shading pipeline:" << std::endl;
                                             std::cout << "  Pass 0: Geometry - fills G-buffer (position, normal, texcoord)" << std::endl;
                                             std::cout << "  Pass 1: Lighting - Blinn-Phong with diffuse texture" << std::endl;
                                             std::cout << std::endl;
                                              std::cout << "Scene: 4x4 grid of Ogre instances" << std::endl;
                                             std::cout << std::endl;
                                             std::cout << "CONTROLS:" << std::endl;
                                             std::cout << "  Mouse drag:        Orbit camera" << std::endl;
                                             std::cout << "  Scroll:            Zoom camera" << std::endl;
                                             std::cout << "  Shift + Drag:      Orbit light" << std::endl;
                                             std::cout << "  Shift + Scroll:    Zoom light" << std::endl;
                                             std::cout << "  Key 1:             Full deferred shading (default)" << std::endl;
                                             std::cout << "  Key 2:             Visualize normals from G-buffer" << std::endl;
                                             std::cout << "  Key 3:             Diffuse texture only" << std::endl;
                                             std::cout << "========================================" << std::endl;
                                         }
                                     },

                                     // InitCallback
                                     [](cg::ManagedWindow *pWindow, int pass)
                                     {
                                         if (pass == -1)
                                         {
                                             pWindow->context.intparam[0] = 1; // Default: full shading

                                             // Set up G-buffer FBO for pass 0 with 3 color attachments
                                             // G-buffer: gPosition (texUnit1), gNormal (texUnit2), gTexCoord (texUnit3)
                                             pWindow->passes[0].fbo = cg::FBO(
                                                 cg::FBO::Deferred,
                                                 GL_TEXTURE1, &pWindow->context.gluintparam[0],
                                                 GL_TEXTURE2, &pWindow->context.gluintparam[1],
                                                 GL_TEXTURE3, &pWindow->context.gluintparam[2]);
                                         }

                                         if (pass == 0)
                                         {
                                             auto &shader = pWindow->passes[0].renderShader;
                                             shader.use();
                                             auto &camera = pWindow->passes[0].cameras[0];
                                             auto &light  = pWindow->passes[0].lights[0];
                                             float aspect = pWindow->width > 0 && pWindow->height > 0
                                                                ? static_cast<float>(pWindow->width) / static_cast<float>(pWindow->height)
                                                                : 1.0f;
                                             glm::mat4 V = camera.getLookAt();
                                             glm::mat4 P = camera.getPerspective(aspect);
                                             shader.setUniform("V", V);
                                             shader.setUniform("P", P);
                                             light.setUniforms(&shader, 0);
                                         }

                                         if (pass == 1)
                                         {
                                             // Unbind the G-buffer FBO before sampling from its textures.
                                             // The render loop calls initCallback BEFORE pass.fbo.bind(), so
                                             // the G-buffer FBO from pass 0 is still bound here. Binding its
                                             // textures as samplers while they're attached to the active FBO
                                             // is an OpenGL feedback loop (undefined behaviour) which causes
                                             // occasional screen flashing on certain drivers.
                                             glBindFramebuffer(GL_FRAMEBUFFER, 0);

                                             auto &shader = pWindow->passes[1].renderShader;
                                             shader.use();

                                             // Bind G-buffer textures to the lighting pass shader
                                             // G-buffer bindings: gPosition=0, gNormal=1, gTexCoord=2
                                             // The window manager binds diffuse at GL_TEXTURE4 automatically.
                                             glActiveTexture(GL_TEXTURE0);
                                             glBindTexture(GL_TEXTURE_2D, pWindow->context.gluintparam[0]); // gPosition
                                             glActiveTexture(GL_TEXTURE1);
                                             glBindTexture(GL_TEXTURE_2D, pWindow->context.gluintparam[1]); // gNormal
                                             glActiveTexture(GL_TEXTURE2);
                                             glBindTexture(GL_TEXTURE_2D, pWindow->context.gluintparam[2]); // gTexCoord

                                             // Pass G-buffer samplers
                                             shader.setUniform("gPosition", 0);
                                             shader.setUniform("gNormal", 1);
                                             shader.setUniform("gTexture", 2);
                                             shader.setUniform("ColorTex", 4); // diffuse bound at GL_TEXTURE4 by window manager

                                             // Pass the view matrix for lighting in view space
                                             auto &camera = pWindow->passes[0].cameras[0];
                                             shader.setUniform("V", camera.getLookAt());

                                             // Pass material (orange Phong - MATERIAL_OBJECT)
                                             shader.setUniform("material.Ka", glm::vec3(0.9f, 0.5f, 0.3f));
                                             shader.setUniform("material.Kd", glm::vec3(0.9f, 0.5f, 0.3f));
                                             shader.setUniform("material.Ks", glm::vec3(0.8f, 0.8f, 0.8f));
                                             shader.setUniform("material.shininess", 100.0f);

                                             // Pass light from pass 0 to the lighting pass
                                             auto &light = pWindow->passes[0].lights[0];
                                             light.setUniforms(&shader, 0);

                                             // Pass rendering option
                                             shader.setUniform("Option", pWindow->context.intparam[0]);
                                         }
                                     },

                                     // SizeCallback
                                     [](cg::ManagedWindow *pWindow, int width, int height) {},

                                     // KeyCallback - 1/2/3 switch rendering option
                                     [](cg::ManagedWindow *pWindow, int key)
                                     {
                                         if (key == GLFW_KEY_1)
                                         {
                                             pWindow->context.intparam[0] = 1;
                                             std::cout << "Option 1: Full deferred shading" << std::endl;
                                             return false;
                                         }
                                         else if (key == GLFW_KEY_2)
                                         {
                                             pWindow->context.intparam[0] = 2;
                                             std::cout << "Option 2: Normals visualization" << std::endl;
                                             return false;
                                         }
                                         else if (key == GLFW_KEY_3)
                                         {
                                             pWindow->context.intparam[0] = 3;
                                             std::cout << "Option 3: Diffuse texture only" << std::endl;
                                             return false;
                                         }
                                         return true;
                                     },

                                     // MousebuttonCallback
                                     [](cg::ManagedWindow *pWindow, int button, int xpos, int ypos) {},

                                     // ScrollCallback
                                     [](cg::ManagedWindow *pWindow, double xoffset, double yoffset, int mods)
                                     {
                                         if (mods & GLFW_MOD_SHIFT)
                                             pWindow->passes[0].lights[0].onScroll(xoffset, yoffset);
                                         else
                                             pWindow->passes[0].cameras[0].onScroll(xoffset, yoffset);
                                     },

                                     // CursorposCallback
                                     [](cg::ManagedWindow *pWindow, int xpos, int ypos, double xoffset, double yoffset, int mods)
                                     {
                                         if (pWindow->context.input.lbutton_down)
                                         {
                                             if (mods & GLFW_MOD_SHIFT)
                                                 pWindow->passes[0].lights[0].onCursor(xoffset, yoffset);
                                             else
                                                 pWindow->passes[0].cameras[0].onCursor(xoffset, yoffset);
                                         }
                                     },

                                     // MouseclickCallback (A11 new)
                                     nullptr,

                                                      // MousedoubleclickCallback (A11 new)
                                                      nullptr},
                                                     pMainWindow));

        // Initialize ray casting camera to center of map
        for (auto &window : windowManager.windows)
        {
            if (window.title && std::string(window.title) == "Assignment 2 - Ray Casting")
            {
                window.context.vec2param[0] = glm::vec2(4.5f, 4.5f);
                window.context.floatparam[0] = 0.0f;
            }
        }

        // Show all windows with SHOW state
        windowManager.show();

        // Set focus to the main window if available
        if (pMainWindow)
            glfwFocusWindow(pMainWindow);

        // Start the rendering loop
        windowManager.render();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        retcode = -1;
    }

    // Cleanup before GLFW termination
    windowManager.cleanup();
    glfwTerminate();

    return retcode;
}
