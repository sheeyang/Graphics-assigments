/*!*****************************************************************************
\file main.cpp
\author Vadim Surov (vsurov@digipen.edu)
\co-author Chew Shee Yang (chew.s@digipen.edu)
\par Course: CSD2151
\par Assignment: 2 (Ray Casting)
\date 01/17/2026 (MM/DD/YYYY)
\brief
    Application entry point for the Ray Casting assignment. Implements a menu
    system with subwindows that can be opened in separate windows on double-click.

    Features:
    - Main window with 4x4 grid layout for subwindows
    - Subwindow 1: Assignment 1 (Tabs) demo
    - Subwindow 2: Assignment 2 (Ray Casting) demo
    - Double-click on subwindow to open in new actual window
    - Interactable subwindows with full keyboard/mouse support

\par AI Disclosure:
    AI (GitHub Copilot) was used to brainstorm implementation ideas and to
    write documentation comments for this assignment.
*******************************************************************************/
#include <iostream> // input/output
#include <vector>   // for storing child windows

#include "windowmanager.h"
#include "trianglemeshes.h"

const int NOMARGIN = 0; // To set no margin for a window

// Grid configuration for menu (4x4 max)
const int GRID_SIZE = 4;
const int GRID_GAP = 10;

// Number of active subwindows
const int NUM_SUBWINDOWS = 2;

// Tab indicator constants (from Assignment 1)
const int NUM_TABS = 3;
const float TAB_CIRCLE_RADIUS = 15.0f;
const float TAB_CIRCLE_Y_OFFSET = 30.0f;
const float TAB_CIRCLE_SPACING = 40.0f;

// Double-click detection threshold (in seconds)
const double DOUBLE_CLICK_TIME = 0.3;

// ============================================================================
// Map data for collision detection (must match shader)
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
        return true; // Out of bounds is treated as wall

    return MAP[mapY * MAP_SIZE + mapX] > 0;
}

// Check if a position with margin is valid (not colliding with walls)
bool canMoveTo(float x, float y)
{
    // Check all corners of a small box around the position
    return !isWall(x - COLLISION_MARGIN, y - COLLISION_MARGIN) &&
           !isWall(x + COLLISION_MARGIN, y - COLLISION_MARGIN) &&
           !isWall(x - COLLISION_MARGIN, y + COLLISION_MARGIN) &&
           !isWall(x + COLLISION_MARGIN, y + COLLISION_MARGIN);
}

// Global storage for child windows and their detached states
std::vector<GLFWwindow *> subwindows;
std::vector<GLFWwindow *> detachedWindows;

// Forward declaration for creating detached window
void createDetachedWindow(cg::WindowManager &windowManager, int subwindowIndex, GLFWwindow *pMainWindow);

// Entry point of the application
int main(int argc, char **argv)
{
    int retcode = 0; // Return code for the application

    // Create a window manager instance
    auto &windowManager = cg::WindowManager::getInstance();

    try
    {
        // Initialize GLFW
        if (!glfwInit())
            throw std::runtime_error("Failed to initialize GLFW.");

// Debug context setup for OpenGL if in debug mode
#ifdef _DEBUG
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif

        // Configure OpenGL context properties
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        GLFWwindow *pMainWindow = nullptr; // Pointer to the main window

        // ====================================================================
        // MAIN MENU WINDOW
        // ====================================================================
        pMainWindow = windowManager.addWindow({cg::State::SHOW,               // State (SHOW, HIDE)
                                               nullptr,                       // Parent window (nullptr for top-level window)
                                               {0, 0, 1024, 768},             // Viewport - x, y, width, height
                                               "CSD2151 Assignment 2 - Menu", // Title
                                               {0.15f, 0.15f, 0.2f, 1.0f},    // Background color (dark blue-grey)
                                               NONE,                          // Textures
                                               NONE,                          // Compute shader
                                               {                              // Rendering passes
                                                {                             // Pass 0 - Background
                                                 {NOMARGIN},                  // Viewport
                                                 {0.15f, 0.15f, 0.2f, 1.0f},  // Background color
                                                 DISABLE,                     // Depth test
                                                 {{QUAD}},                    // Objects
                                                 NONE,                        // Cameras
                                                 NONE,                        // Lights
                                                 DEFAULT,                     // Rendering target
                                                 {
        // Vertex and fragment shaders (menu background)
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
                                                       std::cout << "CSD2151 Assignment 2 - Menu (version "
                                                                 << __DATE__ << " " << __TIME__ << ")";
                                                       std::cout << std::endl
                                                                 << R"(
   This application demonstrates a menu system with interactive subwindows.
   Each subwindow can be opened in a separate window by double-clicking.

Controls:
   - Double-click: Open subwindow in a new window
   - Click on subwindow: Select and interact with it
   - Resize: Drag the window edges to adjust the application size
   - Fullscreen: Press Space to toggle between windowed and fullscreen mode
   - Save Screenshot: Press 'P' to save a screenshot of the active window
   - Exit: Press Esc to close the application

Subwindows:
   - Subwindow 1: Assignment 1 (Tabs) - Animated visual effects
   - Subwindow 2: Assignment 2 (Ray Casting) - First-person environment
)";
                                                   }
                                               },

                                               // InitCallback
                                               [](cg::ManagedWindow *pWindow, int pass)
                                               {
                                                   if (pass == 0)
                                                   {
                                                       pWindow->passes[0].renderShader.use();
                                                       // Pass the number of subwindows to the shader
                                                       pWindow->passes[0].renderShader.setUniform("iNumSubwindows", NUM_SUBWINDOWS);
                                                       pWindow->passes[0].renderShader.setUniform("iGridSize", GRID_SIZE);
                                                   }
                                               },

                                               // SizeCallback
                                               [](cg::ManagedWindow *pWindow, int width, int height) {},

                                               // KeyCallback
                                               [](cg::ManagedWindow *pWindow, int key)
                                               {
                                                   return true; // Propagate the event by default
                                               },

                                               // MousebuttonCallback
                                               [](cg::ManagedWindow *pWindow, int button, int xpos, int ypos) {},

                                               // ScrollCallback
                                               [](cg::ManagedWindow *pWindow, double xoffset, double yoffset, int mods) {},

                                               // CursorposCallback
                                               [](cg::ManagedWindow *pWindow, int xpos, int ypos, double xoffset, double yoffset, int mods) {}});

        // ====================================================================
        // SUBWINDOW 1: Assignment 1 (Tabs)
        // ====================================================================
        subwindows.push_back(windowManager.addWindow({cg::State::SHOW,          // State
                                                      pMainWindow,              // Parent window
                                                      {0, GRID_SIZE, GRID_GAP}, // Grid position: element 0 in 4x4 grid
                                                      "Assignment 1 - Tabs",    // Title
                                                      NONE,                     // Background color
                                                      NONE,                     // Textures
                                                      NONE,                     // Compute shader
                                                      {                         // Rendering passes
                                                       {                        // Pass 0
                                                        {NOMARGIN},             // Viewport
                                                        NONE,                   // Background color
                                                        DISABLE,                // Depth test
                                                        {{QUAD}},               // Objects
                                                        NONE,                   // Cameras
                                                        NONE,                   // Lights
                                                        DEFAULT,                // Rendering target
                                                        {
        // Shaders - Assignment 1 tabs
#include "Shaders/Shader.vert.glsl"
                                                            ,
#include "Shaders/ShaderA1.frag.glsl"
                                                        }}},

                                                      // FocusCallback
                                                      [](cg::ManagedWindow *pWindow, int focused)
                                                      {
                                                          if (focused)
                                                          {
                                                              std::cout << "Subwindow 1 (Assignment 1 - Tabs) focused" << std::endl;
                                                              std::cout << "Current Tab: " << (pWindow->context.intparam[0] + 1) << std::endl;
                                                          }
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
                                                              std::cout << "Tab key pressed - Switched to Tab " << (pWindow->context.intparam[0] + 1) << std::endl;
                                                              return false;
                                                          }
                                                          return true;
                                                      },

                                                      // MousebuttonCallback - handles tab clicks (double-click to detach is handled by framework)
                                                      [](cg::ManagedWindow *pWindow, int button, int xpos, int ypos)
                                                      {
                                                          if (button == GLFW_MOUSE_BUTTON_LEFT)
                                                          {
                                                              // Single click - check for tab indicator clicks
                                                              float centerX = pWindow->width / 2.0f;
                                                              float y = pWindow->height - TAB_CIRCLE_Y_OFFSET;

                                                              for (int i = 0; i < NUM_TABS; i++)
                                                              {
                                                                  float tabX = centerX + (float(i) - 1.0f) * TAB_CIRCLE_SPACING;
                                                                  float tabY = y;

                                                                  float dx = xpos - tabX;
                                                                  float dy = ypos - tabY;
                                                                  float dist = sqrt(dx * dx + dy * dy);

                                                                  if (dist <= TAB_CIRCLE_RADIUS + 5.0f)
                                                                  {
                                                                      pWindow->context.intparam[0] = i;
                                                                      std::cout << "Mouse clicked on Tab " << (i + 1) << std::endl;
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
                                                      }},
                                                     pMainWindow));

        // ====================================================================
        // SUBWINDOW 2: Assignment 2 (Ray Casting)
        // ====================================================================
        subwindows.push_back(windowManager.addWindow({cg::State::SHOW,              // State
                                                      pMainWindow,                  // Parent window
                                                      {1, GRID_SIZE, GRID_GAP},     // Grid position: element 1 in 4x4 grid
                                                      "Assignment 2 - Ray Casting", // Title
                                                      NONE,                         // Background color
                                                      NONE,                         // Textures
                                                      NONE,                         // Compute shader
                                                      {                             // Rendering passes
                                                       {                            // Pass 0
                                                        {NOMARGIN},                 // Viewport
                                                        NONE,                       // Background color
                                                        DISABLE,                    // Depth test
                                                        {{QUAD}},                   // Objects
                                                        NONE,                       // Cameras
                                                        NONE,                       // Lights
                                                        DEFAULT,                    // Rendering target
                                                        {
        // Shaders - Assignment 2 ray casting
#include "Shaders/Shader.vert.glsl"
                                                            ,
#include "Shaders/ShaderA2.frag.glsl"
                                                        }}},

                                                      // FocusCallback
                                                      [](cg::ManagedWindow *pWindow, int focused)
                                                      {
                                                          if (focused)
                                                          {
                                                              std::cout << "Subwindow 2 (Assignment 2 - Ray Casting) focused" << std::endl;
                                                          }
                                                      },

                                                      // InitCallback
                                                      [](cg::ManagedWindow *pWindow, int pass)
                                                      {
                                                          if (pass == 0)
                                                          {
                                                              pWindow->passes[0].renderShader.use();
                                                              // Pass camera position and direction
                                                              pWindow->passes[0].renderShader.setUniform("iCameraPos", pWindow->context.vec2param[0]);
                                                              pWindow->passes[0].renderShader.setUniform("iCameraDir", pWindow->context.floatparam[0]);
                                                          }
                                                      },

                                                      // SizeCallback
                                                      [](cg::ManagedWindow *pWindow, int width, int height) {},

                                                      // KeyCallback - WASD movement with collision detection
                                                      [](cg::ManagedWindow *pWindow, int key)
                                                      {
                                                          float moveSpeed = 0.1f;
                                                          float dir = pWindow->context.floatparam[0];
                                                          float currentX = pWindow->context.vec2param[0].x;
                                                          float currentY = pWindow->context.vec2param[0].y;
                                                          float newX = currentX;
                                                          float newY = currentY;

                                                          if (key == GLFW_KEY_W)
                                                          {
                                                              newX = currentX + cos(dir) * moveSpeed;
                                                              newY = currentY + sin(dir) * moveSpeed;
                                                              // Try to move, with wall sliding
                                                              if (canMoveTo(newX, newY))
                                                              {
                                                                  pWindow->context.vec2param[0].x = newX;
                                                                  pWindow->context.vec2param[0].y = newY;
                                                              }
                                                              else if (canMoveTo(newX, currentY))
                                                              {
                                                                  pWindow->context.vec2param[0].x = newX;
                                                              }
                                                              else if (canMoveTo(currentX, newY))
                                                              {
                                                                  pWindow->context.vec2param[0].y = newY;
                                                              }
                                                              return false;
                                                          }
                                                          if (key == GLFW_KEY_S)
                                                          {
                                                              newX = currentX - cos(dir) * moveSpeed;
                                                              newY = currentY - sin(dir) * moveSpeed;
                                                              if (canMoveTo(newX, newY))
                                                              {
                                                                  pWindow->context.vec2param[0].x = newX;
                                                                  pWindow->context.vec2param[0].y = newY;
                                                              }
                                                              else if (canMoveTo(newX, currentY))
                                                              {
                                                                  pWindow->context.vec2param[0].x = newX;
                                                              }
                                                              else if (canMoveTo(currentX, newY))
                                                              {
                                                                  pWindow->context.vec2param[0].y = newY;
                                                              }
                                                              return false;
                                                          }
                                                          if (key == GLFW_KEY_A)
                                                          {
                                                              newX = currentX + sin(dir) * moveSpeed;
                                                              newY = currentY - cos(dir) * moveSpeed;
                                                              if (canMoveTo(newX, newY))
                                                              {
                                                                  pWindow->context.vec2param[0].x = newX;
                                                                  pWindow->context.vec2param[0].y = newY;
                                                              }
                                                              else if (canMoveTo(newX, currentY))
                                                              {
                                                                  pWindow->context.vec2param[0].x = newX;
                                                              }
                                                              else if (canMoveTo(currentX, newY))
                                                              {
                                                                  pWindow->context.vec2param[0].y = newY;
                                                              }
                                                              return false;
                                                          }
                                                          if (key == GLFW_KEY_D)
                                                          {
                                                              newX = currentX - sin(dir) * moveSpeed;
                                                              newY = currentY + cos(dir) * moveSpeed;
                                                              if (canMoveTo(newX, newY))
                                                              {
                                                                  pWindow->context.vec2param[0].x = newX;
                                                                  pWindow->context.vec2param[0].y = newY;
                                                              }
                                                              else if (canMoveTo(newX, currentY))
                                                              {
                                                                  pWindow->context.vec2param[0].x = newX;
                                                              }
                                                              else if (canMoveTo(currentX, newY))
                                                              {
                                                                  pWindow->context.vec2param[0].y = newY;
                                                              }
                                                              return false;
                                                          }
                                                          return true;
                                                      },

                                                      // MousebuttonCallback - double-click to detach is handled by framework
                                                      [](cg::ManagedWindow *pWindow, int button, int xpos, int ypos)
                                                      {
                                                          // Single clicks can be handled here if needed
                                                      },

                                                      // ScrollCallback
                                                      [](cg::ManagedWindow *pWindow, double xoffset, double yoffset, int mods) {},

                                                      // CursorposCallback - mouse rotation
                                                      [](cg::ManagedWindow *pWindow, int xpos, int ypos, double xoffset, double yoffset, int mods)
                                                      {
                                                          if (pWindow->context.input.lbutton_down)
                                                          {
                                                              float rotSpeed = 0.01f;
                                                              pWindow->context.floatparam[0] += static_cast<float>(xoffset) * rotSpeed;
                                                          }
                                                      }},
                                                     pMainWindow));

        // Initialize camera positions for ray casting
        // Set initial camera position to center of map
        for (auto &window : windowManager.windows)
        {
            if (window.title && std::string(window.title) == "Assignment 2 - Ray Casting")
            {
                window.context.vec2param[0] = glm::vec2(4.5f, 4.5f); // Center of 9x9 map
                window.context.floatparam[0] = 0.0f;                 // Initial direction
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

    // Explicit cleanup before GLFW termination
    windowManager.cleanup();

    // Clean up and terminate GLFW
    glfwTerminate();

    return retcode;
}

// ============================================================================
// Function to create a detached (standalone) window from a subwindow
// ============================================================================
void createDetachedWindow(cg::WindowManager &windowManager, int subwindowIndex, GLFWwindow *pMainWindow)
{
    if (subwindowIndex == 0)
    {
        // Create detached Assignment 1 window
        GLFWwindow *pDetached = windowManager.addWindow({cg::State::SHOW,
                                                         nullptr,                          // No parent - standalone window
                                                         {0, 0, 800, 600},                 // x, y, width, height
                                                         "Assignment 1 - Tabs (Detached)", // Title
                                                         NONE,
                                                         NONE,
                                                         NONE,
                                                         {{{NOMARGIN},
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
                                                             {
                                                                 std::cout << "Detached Assignment 1 window focused" << std::endl;
                                                                 std::cout << "Current Tab: " << (pWindow->context.intparam[0] + 1) << std::endl;
                                                             }
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
                                                                 std::cout << "Tab key pressed - Switched to Tab " << (pWindow->context.intparam[0] + 1) << std::endl;
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
                                                                     float tabY = y;
                                                                     float dx = xpos - tabX;
                                                                     float dy = ypos - tabY;
                                                                     float dist = sqrt(dx * dx + dy * dy);

                                                                     if (dist <= TAB_CIRCLE_RADIUS + 5.0f)
                                                                     {
                                                                         pWindow->context.intparam[0] = i;
                                                                         std::cout << "Mouse clicked on Tab " << (i + 1) << std::endl;
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
                                                         }},
                                                        pMainWindow);

        if (pDetached)
        {
            detachedWindows.push_back(pDetached);
            glfwShowWindow(pDetached);
            glfwFocusWindow(pDetached);
        }
    }
    else if (subwindowIndex == 1)
    {
        // Create detached Assignment 2 window
        GLFWwindow *pDetached = windowManager.addWindow({cg::State::SHOW,
                                                         nullptr,                                 // No parent - standalone window
                                                         {0, 0, 800, 600},                        // x, y, width, height
                                                         "Assignment 2 - Ray Casting (Detached)", // Title
                                                         NONE,
                                                         NONE,
                                                         NONE,
                                                         {{{NOMARGIN},
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
                                                             {
                                                                 std::cout << "Detached Assignment 2 window focused" << std::endl;
                                                             }
                                                         },

                                                         // InitCallback
                                                         [](cg::ManagedWindow *pWindow, int pass)
                                                         {
                                                             if (pass == 0)
                                                             {
                                                                 pWindow->passes[0].renderShader.use();
                                                                 pWindow->passes[0].renderShader.setUniform("iCameraPos", pWindow->context.vec2param[0]);
                                                                 pWindow->passes[0].renderShader.setUniform("iCameraDir", pWindow->context.floatparam[0]);
                                                             }
                                                         },

                                                         // SizeCallback
                                                         [](cg::ManagedWindow *pWindow, int width, int height) {},

                                                         // KeyCallback - WASD movement with collision detection
                                                         [](cg::ManagedWindow *pWindow, int key)
                                                         {
                                                             float moveSpeed = 0.1f;
                                                             float dir = pWindow->context.floatparam[0];
                                                             float currentX = pWindow->context.vec2param[0].x;
                                                             float currentY = pWindow->context.vec2param[0].y;
                                                             float newX = currentX;
                                                             float newY = currentY;

                                                             if (key == GLFW_KEY_W)
                                                             {
                                                                 newX = currentX + cos(dir) * moveSpeed;
                                                                 newY = currentY + sin(dir) * moveSpeed;
                                                                 if (canMoveTo(newX, newY))
                                                                 {
                                                                     pWindow->context.vec2param[0].x = newX;
                                                                     pWindow->context.vec2param[0].y = newY;
                                                                 }
                                                                 else if (canMoveTo(newX, currentY))
                                                                 {
                                                                     pWindow->context.vec2param[0].x = newX;
                                                                 }
                                                                 else if (canMoveTo(currentX, newY))
                                                                 {
                                                                     pWindow->context.vec2param[0].y = newY;
                                                                 }
                                                                 return false;
                                                             }
                                                             if (key == GLFW_KEY_S)
                                                             {
                                                                 newX = currentX - cos(dir) * moveSpeed;
                                                                 newY = currentY - sin(dir) * moveSpeed;
                                                                 if (canMoveTo(newX, newY))
                                                                 {
                                                                     pWindow->context.vec2param[0].x = newX;
                                                                     pWindow->context.vec2param[0].y = newY;
                                                                 }
                                                                 else if (canMoveTo(newX, currentY))
                                                                 {
                                                                     pWindow->context.vec2param[0].x = newX;
                                                                 }
                                                                 else if (canMoveTo(currentX, newY))
                                                                 {
                                                                     pWindow->context.vec2param[0].y = newY;
                                                                 }
                                                                 return false;
                                                             }
                                                             if (key == GLFW_KEY_A)
                                                             {
                                                                 newX = currentX + sin(dir) * moveSpeed;
                                                                 newY = currentY - cos(dir) * moveSpeed;
                                                                 if (canMoveTo(newX, newY))
                                                                 {
                                                                     pWindow->context.vec2param[0].x = newX;
                                                                     pWindow->context.vec2param[0].y = newY;
                                                                 }
                                                                 else if (canMoveTo(newX, currentY))
                                                                 {
                                                                     pWindow->context.vec2param[0].x = newX;
                                                                 }
                                                                 else if (canMoveTo(currentX, newY))
                                                                 {
                                                                     pWindow->context.vec2param[0].y = newY;
                                                                 }
                                                                 return false;
                                                             }
                                                             if (key == GLFW_KEY_D)
                                                             {
                                                                 newX = currentX - sin(dir) * moveSpeed;
                                                                 newY = currentY + cos(dir) * moveSpeed;
                                                                 if (canMoveTo(newX, newY))
                                                                 {
                                                                     pWindow->context.vec2param[0].x = newX;
                                                                     pWindow->context.vec2param[0].y = newY;
                                                                 }
                                                                 else if (canMoveTo(newX, currentY))
                                                                 {
                                                                     pWindow->context.vec2param[0].x = newX;
                                                                 }
                                                                 else if (canMoveTo(currentX, newY))
                                                                 {
                                                                     pWindow->context.vec2param[0].y = newY;
                                                                 }
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
                                                             {
                                                                 float rotSpeed = 0.01f;
                                                                 pWindow->context.floatparam[0] += static_cast<float>(xoffset) * rotSpeed;
                                                             }
                                                         }},
                                                        pMainWindow);

        if (pDetached)
        {
            // Initialize camera position
            for (auto &window : windowManager.windows)
            {
                if (window.pWindow == pDetached)
                {
                    window.context.vec2param[0] = glm::vec2(4.5f, 4.5f);
                    window.context.floatparam[0] = 0.0f;
                    break;
                }
            }
            detachedWindows.push_back(pDetached);
            glfwShowWindow(pDetached);
            glfwFocusWindow(pDetached);
        }
    }
}