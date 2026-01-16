/*!*****************************************************************************
\file main.cpp
\author Vadim Surov (vsurov@digipen.edu)
\coauthor SIOW YU TENG, ETHAN (s.yutengethan@digipen.edu)
\par Course: CSD2151
\par Assignment: 2 (Ray Casting)
\date 01/16/2026 (MM/DD/YYYY)
\brief Application entry point: Main menu with interactive demo subwindows.

 * This file initializes the GLFW windowing system, configures an OpenGL 4.6
 * core profile rendering context, and creates a managed rendering window
 * using the custom WindowManager framework.
 *
 * Features:
 *  - Main menu with 4x4 grid layout for interactive demo subwindows
 *  - Subwindows are fully interactive (keyboard/mouse input works)
 *  - Double-click on subwindow to open it in a new separate window
 *  - Assignment 1: Animated checkerboard effect
 *  - Assignment 2: Ray Casting FPS-style demo
*******************************************************************************/
#include <iostream>
#include <vector>
#include <cmath>

#include "windowmanager.h"
#include "trianglemeshes.h"

const int NOMARGIN = 0;

// ============ GRID CONFIGURATION ============
const int GRID_SIZE = 4;    // 4x4 grid
const int GRID_GAP = 15;    // Gap between grid cells
const int GRID_MARGIN = 50; // Margin from window edges

// ============ DEMO STATE ============
struct DemoState
{
    const char *name;
    int gridIndex;
    GLFWwindow *thumbnailWindow; // Child window in menu
    GLFWwindow *fullWindow;      // Separate full window (if opened)
    bool isFullWindowOpen;

    // Demo-specific state
    float playerX;
    float playerY;
    float playerAngle;

    // Demo 1 specific state
    int tabIndex; // Current tab for Demo 1
    float mouseX; // Mouse X position
    float mouseY; // Mouse Y position

    // Double-click detection per demo
    double lastClickTime;
};

std::vector<DemoState> demos;

const float moveSpeed = 0.05f;
const float rotSpeed = 0.03f;
const double DOUBLE_CLICK_TIME = 0.4;

// Forward declarations
void createDemo1Thumbnail(cg::WindowManager &wm, GLFWwindow *parent, int x, int y, int w, int h, int demoIdx);
void createDemo2Thumbnail(cg::WindowManager &wm, GLFWwindow *parent, int x, int y, int w, int h, int demoIdx);
void openFullWindow(cg::WindowManager &wm, int demoIndex);

// Helper: Calculate grid cell position
void getGridCellBounds(int index, int windowW, int windowH, int &x, int &y, int &w, int &h)
{
    int availW = windowW - GRID_MARGIN * 2 - (GRID_SIZE - 1) * GRID_GAP;
    int availH = windowH - GRID_MARGIN * 2 - (GRID_SIZE - 1) * GRID_GAP;
    int cellW = availW / GRID_SIZE;
    int cellH = availH / GRID_SIZE;

    int col = index % GRID_SIZE;
    int row = index / GRID_SIZE;

    x = GRID_MARGIN + col * (cellW + GRID_GAP);
    y = GRID_MARGIN + row * (cellH + GRID_GAP);
    w = cellW;
    h = cellH;
}

// Wall collision check for ray casting
bool isWall(float x, float y)
{
    int map[8][8] = {
        {1, 1, 1, 1, 1, 1, 1, 1},
        {1, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 2, 0, 0, 3, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 3, 0, 0, 2, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 1},
        {1, 1, 1, 1, 1, 1, 1, 1}};
    int mx = (int)x;
    int my = (int)y;
    if (mx < 0 || mx >= 8 || my < 0 || my >= 8)
        return true;
    return map[my][mx] != 0;
}

// Entry point
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

        // Initialize demo states
        // Demo 1: tabIndex=0, mouseX=0, mouseY=0
        demos.push_back({"Assignment 1 - Tab Effects", 0, nullptr, nullptr, false, 0, 0, 0, 0, 0, 0, 0});
        // Demo 2: playerX=2.5, playerY=2.5, playerAngle=0
        demos.push_back({"Assignment 2 - Ray Casting", 1, nullptr, nullptr, false, 1.5f, 1.5f, 0.0f, 0, 0, 0, 0});

        GLFWwindow *pMainWindow = nullptr;

        // ========== MAIN MENU WINDOW ==========
        pMainWindow = windowManager.addWindow({cg::State::SHOW,
                                               nullptr,
                                               DEFAULT,
                                               "Shaders Framework - Demo Menu",
                                               NONE,
                                               NONE,
                                               NONE,
                                               {{{NOMARGIN},
                                                 BLACK,
                                                 DISABLE,
                                                 {{QUAD}},
                                                 NONE,
                                                 NONE,
                                                 DEFAULT,
                                                 {
#include "Shaders/Shader.vert.glsl"
                                                     ,
#include "Shaders/Shader.frag.glsl"
                                                 }}},

                                               // FocusCallback
                                               [](cg::ManagedWindow *pWindow, int focused)
                                               {
                                                   if (focused)
                                                   {
                                                       std::cout << std::endl;
                                                       std::cout << "Shaders Framework - Demo Menu (version "
                                                                 << __DATE__ << " " << __TIME__ << ")" << std::endl;
                                                       std::cout << R"(
Controls:
   - Click on a demo subwindow to interact with it
   - Double-click on a demo to open it in a new window
   - Press Esc to close the application

Available Demos:
   - Assignment 1: Animated Checkerboard (click to see animation)
   - Assignment 2: Ray Casting (WASD to move, drag mouse to look)
)" << std::endl;
                                                   }
                                               },

                                               // InitCallback
                                               [](cg::ManagedWindow *pWindow, int pass)
                                               {
                                                   if (pWindow->passes.empty())
                                                       return;
                                                   auto &program = pWindow->passes[0].renderShader;
                                                   program.use();
                                                   program.setUniform("Time", (float)glfwGetTime());
                                                   program.setUniform("ShowButtons", 0.0f); // Disable tab buttons in menu
                                               },

                                               // SizeCallback
                                               [](cg::ManagedWindow *pWindow, int width, int height)
                                               {
                                                   pWindow->width = width;
                                                   pWindow->height = height;
                                               },

                                               // KeyCallback
                                               [](cg::ManagedWindow *pWindow, int key)
                                               { return false; },

                                               // MousebuttonCallback
                                               [](cg::ManagedWindow *pWindow, int button, int xpos, int ypos) {},

                                               // ScrollCallback
                                               [](cg::ManagedWindow *pWindow, double xoffset, double yoffset, int mods) {},

                                               // CursorposCallback
                                               [](cg::ManagedWindow *pWindow, int xpos, int ypos, double xoffset, double yoffset, int mods) {}});

        // Get initial window size for positioning child windows
        int mainW = 800, mainH = 600;
        glfwGetWindowSize(pMainWindow, &mainW, &mainH);

        // Create interactive demo thumbnail subwindows
        for (size_t i = 0; i < demos.size(); i++)
        {
            int x, y, w, h;
            getGridCellBounds(demos[i].gridIndex, mainW, mainH, x, y, w, h);

            if (i == 0)
            {
                createDemo1Thumbnail(windowManager, pMainWindow, x, y, w, h, (int)i);
            }
            else if (i == 1)
            {
                createDemo2Thumbnail(windowManager, pMainWindow, x, y, w, h, (int)i);
            }
        }

        windowManager.show();

        if (pMainWindow)
            glfwFocusWindow(pMainWindow);

        windowManager.render();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        retcode = -1;
    }

    windowManager.cleanup();
    glfwTerminate();

    return retcode;
}

// ============ DEMO 1: ANIMATED CHECKERBOARD THUMBNAIL ============
void createDemo1Thumbnail(cg::WindowManager &wm, GLFWwindow *parent, int x, int y, int w, int h, int demoIdx)
{
    GLFWwindow *thumbWindow = wm.addWindow({cg::State::SHOW,
                                            parent,
                                            {x, y, w, h},
                                            "",
                                            BLACK,
                                            NONE,
                                            NONE,
                                            {{{NOMARGIN},
                                              BLACK,
                                              DISABLE,
                                              {{QUAD}},
                                              NONE,
                                              NONE,
                                              DEFAULT,
                                              {
#include "Shaders/Shader.vert.glsl"
                                                  ,
#include "Shaders/Shader.frag.glsl"
                                              }}},

                                            // FocusCallback
                                            [](cg::ManagedWindow *pWindow, int focused)
                                            {
                                                if (focused)
                                                {
                                                    std::cout << "Demo 1 selected - Animated Checkerboard" << std::endl;
                                                }
                                            },

                                            // InitCallback
                                            [](cg::ManagedWindow *pWindow, int pass)
                                            {
                                                if (pWindow->passes.empty())
                                                    return;
                                                auto &program = pWindow->passes[0].renderShader;
                                                program.use();
                                                program.setUniform("Time", (float)glfwGetTime());
                                                program.setUniform("TabIndex", (float)demos[0].tabIndex);
                                                program.setUniform("ShowButtons", 1.0f); // Enable tab buttons
                                                // Flip Y coordinate: GLFW uses top-left origin, OpenGL uses bottom-left
                                                float mouseY_flipped = (float)pWindow->height - (float)demos[0].mouseY;
                                                program.setUniform("Mouse", glm::vec2((float)demos[0].mouseX, mouseY_flipped));
                                            },

                                            // SizeCallback
                                            [](cg::ManagedWindow *pWindow, int width, int height)
                                            {
                                                pWindow->width = width;
                                                pWindow->height = height;
                                            },

                                            // KeyCallback - Tab key to cycle tabs
                                            [](cg::ManagedWindow *pWindow, int key)
                                            {
                                                if (key == GLFW_KEY_TAB)
                                                {
                                                    demos[0].tabIndex = (demos[0].tabIndex + 1) % 3;
                                                    std::cout << "Demo 1 Tab switched to: " << demos[0].tabIndex << std::endl;
                                                    pWindow->needUpdate = true;
                                                    return true;
                                                }
                                                return false;
                                            },

                                            // MousebuttonCallback - double-click detection and tab button clicks
                                            [](cg::ManagedWindow *pWindow, int button, int xpos, int ypos)
                                            {
                                                if (button == GLFW_MOUSE_BUTTON_LEFT)
                                                {
                                                    double currentTime = glfwGetTime();

                                                    // Check if clicking on tab buttons (3 circles at bottom)
                                                    float w = (float)pWindow->width;
                                                    float h = (float)pWindow->height;
                                                    // Shader draws buttons at 0.1 * h from bottom (OpenGL coords)
                                                    // In GLFW coords (top-left origin), this is 0.9 * h from top
                                                    float buttonY = 0.9f * h;
                                                    float buttonRadius = 18.0f;

                                                    // Button positions: center - 50, center, center + 50
                                                    float centerX = w * 0.5f;
                                                    float buttonPositions[3] = {centerX - 50.0f, centerX, centerX + 50.0f};

                                                    for (int i = 0; i < 3; i++)
                                                    {
                                                        float dx = (float)xpos - buttonPositions[i];
                                                        float dy = (float)ypos - buttonY;
                                                        if (dx * dx + dy * dy <= buttonRadius * buttonRadius)
                                                        {
                                                            demos[0].tabIndex = i;
                                                            std::cout << "Demo 1 Tab button " << i << " clicked" << std::endl;
                                                            pWindow->needUpdate = true;
                                                            return; // Button was clicked, don't process double-click
                                                        }
                                                    }

                                                    // Check for double-click
                                                    if ((currentTime - demos[0].lastClickTime) < DOUBLE_CLICK_TIME)
                                                    {
                                                        // Double-click - open full window
                                                        if (!demos[0].isFullWindowOpen)
                                                        {
                                                            std::cout << "Opening full window: " << demos[0].name << std::endl;
                                                            auto &wm = cg::WindowManager::getInstance();
                                                            openFullWindow(wm, 0);
                                                        }
                                                        demos[0].lastClickTime = 0;
                                                    }
                                                    else
                                                    {
                                                        demos[0].lastClickTime = currentTime;
                                                    }
                                                }
                                            },

                                            // ScrollCallback
                                            [](cg::ManagedWindow *pWindow, double xoffset, double yoffset, int mods) {},

                                            // CursorposCallback - track mouse for flashlight effect
                                            [](cg::ManagedWindow *pWindow, int xpos, int ypos, double xoffset, double yoffset, int mods)
                                            {
                                                demos[0].mouseX = static_cast<float>(xpos);
                                                demos[0].mouseY = static_cast<float>(ypos);
                                                pWindow->needUpdate = true;
                                            }});

    demos[demoIdx].thumbnailWindow = thumbWindow;
}

// ============ DEMO 2: RAY CASTING THUMBNAIL ============
void createDemo2Thumbnail(cg::WindowManager &wm, GLFWwindow *parent, int x, int y, int w, int h, int demoIdx)
{
    GLFWwindow *thumbWindow = wm.addWindow({cg::State::SHOW,
                                            parent,
                                            {x, y, w, h},
                                            "",
                                            BLACK,
                                            NONE,
                                            NONE,
                                            {{{NOMARGIN},
                                              BLACK,
                                              DISABLE,
                                              {{QUAD}},
                                              NONE,
                                              NONE,
                                              DEFAULT,
                                              {
#include "Shaders/RayCast.vert.glsl"
                                                  ,
#include "Shaders/RayCast.frag.glsl"
                                              }}},

                                            // FocusCallback
                                            [](cg::ManagedWindow *pWindow, int focused)
                                            {
                                                if (focused)
                                                {
                                                    std::cout << "Demo 2 selected - Ray Casting (WASD to move, drag to look)" << std::endl;
                                                }
                                            },

                                            // InitCallback
                                            [](cg::ManagedWindow *pWindow, int pass)
                                            {
                                                if (pWindow->passes.empty())
                                                    return;
                                                auto &program = pWindow->passes[0].renderShader;
                                                program.use();
                                                program.setUniform("playerPos", glm::vec2(demos[1].playerX, demos[1].playerY));
                                                program.setUniform("playerAngle", demos[1].playerAngle);
                                            },

                                            // SizeCallback
                                            [](cg::ManagedWindow *pWindow, int width, int height)
                                            {
                                                pWindow->width = width;
                                                pWindow->height = height;
                                            },

                                            // KeyCallback - WASD movement
                                            [](cg::ManagedWindow *pWindow, int key)
                                            {
                                                float &pX = demos[1].playerX;
                                                float &pY = demos[1].playerY;
                                                float &pA = demos[1].playerAngle;

                                                float dirX = cos(pA);
                                                float dirY = sin(pA);
                                                float perpX = -dirY;
                                                float perpY = dirX;

                                                float newX = pX, newY = pY;

                                                switch (key)
                                                {
                                                case GLFW_KEY_W:
                                                    newX = pX + dirX * moveSpeed;
                                                    newY = pY + dirY * moveSpeed;
                                                    break;
                                                case GLFW_KEY_S:
                                                    newX = pX - dirX * moveSpeed;
                                                    newY = pY - dirY * moveSpeed;
                                                    break;
                                                case GLFW_KEY_A:
                                                    newX = pX - perpX * moveSpeed;
                                                    newY = pY - perpY * moveSpeed;
                                                    break;
                                                case GLFW_KEY_D:
                                                    newX = pX + perpX * moveSpeed;
                                                    newY = pY + perpY * moveSpeed;
                                                    break;
                                                default:
                                                    return false;
                                                }

                                                float margin = 0.2f;
                                                if (!isWall(newX + margin, pY) && !isWall(newX - margin, pY))
                                                    pX = newX;
                                                if (!isWall(pX, newY + margin) && !isWall(pX, newY - margin))
                                                    pY = newY;

                                                if (!pWindow->passes.empty())
                                                {
                                                    auto &program = pWindow->passes[0].renderShader;
                                                    program.use();
                                                    program.setUniform("playerPos", glm::vec2(pX, pY));
                                                }
                                                pWindow->needUpdate = true;

                                                // Also update full window if open
                                                if (demos[1].isFullWindowOpen && demos[1].fullWindow)
                                                {
                                                    // Full window will update via shared state in InitCallback
                                                }

                                                return true;
                                            },

                                            // MousebuttonCallback - double-click detection
                                            [](cg::ManagedWindow *pWindow, int button, int xpos, int ypos)
                                            {
                                                if (button == GLFW_MOUSE_BUTTON_LEFT)
                                                {
                                                    double currentTime = glfwGetTime();
                                                    if ((currentTime - demos[1].lastClickTime) < DOUBLE_CLICK_TIME)
                                                    {
                                                        // Double-click - open full window
                                                        if (!demos[1].isFullWindowOpen)
                                                        {
                                                            std::cout << "Opening full window: " << demos[1].name << std::endl;
                                                            auto &wm = cg::WindowManager::getInstance();
                                                            openFullWindow(wm, 1);
                                                        }
                                                        demos[1].lastClickTime = 0;
                                                    }
                                                    else
                                                    {
                                                        demos[1].lastClickTime = currentTime;
                                                    }
                                                }
                                            },

                                            // ScrollCallback
                                            [](cg::ManagedWindow *pWindow, double xoffset, double yoffset, int mods) {},

                                            // CursorposCallback - mouse look
                                            [](cg::ManagedWindow *pWindow, int xpos, int ypos, double xoffset, double yoffset, int mods)
                                            {
                                                if (glfwGetMouseButton(glfwGetCurrentContext(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
                                                {
                                                    demos[1].playerAngle -= (float)xoffset * rotSpeed;

                                                    if (!pWindow->passes.empty())
                                                    {
                                                        auto &program = pWindow->passes[0].renderShader;
                                                        program.use();
                                                        program.setUniform("playerAngle", demos[1].playerAngle);
                                                    }
                                                    pWindow->needUpdate = true;
                                                }
                                            }});

    demos[demoIdx].thumbnailWindow = thumbWindow;
}

// ============ OPEN FULL WINDOW ============
void openFullWindow(cg::WindowManager &wm, int demoIndex)
{
    if (demoIndex < 0 || demoIndex >= (int)demos.size())
        return;
    if (demos[demoIndex].isFullWindowOpen)
        return;

    GLFWwindow *fullWindow = nullptr;

    if (demoIndex == 0)
    {
        // Demo 1: Animated Checkerboard - Full Window
        fullWindow = wm.addWindow({cg::State::SHOW,
                                   nullptr,
                                   {100, 100, 800, 600},
                                   "Assignment 1 - Animated Checkerboard",
                                   BLACK,
                                   NONE,
                                   NONE,
                                   {{{NOMARGIN},
                                     BLACK,
                                     DISABLE,
                                     {{QUAD}},
                                     NONE,
                                     NONE,
                                     DEFAULT,
                                     {
#include "Shaders/Shader.vert.glsl"
                                         ,
#include "Shaders/Shader.frag.glsl"
                                     }}},

                                   [](cg::ManagedWindow *pWindow, int focused)
                                   {
                                       if (focused)
                                           std::cout << "Assignment 1 - Full Window (Tab key or click buttons to switch tabs)" << std::endl;
                                   },

                                   // InitCallback - sync tab and mouse state
                                   [](cg::ManagedWindow *pWindow, int pass)
                                   {
                                       if (pWindow->passes.empty())
                                           return;
                                       auto &program = pWindow->passes[0].renderShader;
                                       program.use();
                                       program.setUniform("Time", (float)glfwGetTime());
                                       program.setUniform("TabIndex", (float)demos[0].tabIndex);
                                       program.setUniform("ShowButtons", 1.0f); // Enable tab buttons
                                       // Flip Y coordinate: GLFW uses top-left origin, OpenGL uses bottom-left
                                       float mouseY_flipped = (float)pWindow->height - (float)demos[0].mouseY;
                                       program.setUniform("Mouse", glm::vec2((float)demos[0].mouseX, mouseY_flipped));
                                   },

                                   [](cg::ManagedWindow *pWindow, int width, int height)
                                   {
                                       pWindow->width = width;
                                       pWindow->height = height;
                                   },

                                   // KeyCallback - Tab key to cycle tabs
                                   [](cg::ManagedWindow *pWindow, int key)
                                   {
                                       if (key == GLFW_KEY_TAB)
                                       {
                                           demos[0].tabIndex = (demos[0].tabIndex + 1) % 3;
                                           std::cout << "Demo 1 Tab switched to: " << demos[0].tabIndex << std::endl;
                                           pWindow->needUpdate = true;
                                           return true;
                                       }
                                       return false;
                                   },

                                   // MousebuttonCallback - check for tab button clicks
                                   [](cg::ManagedWindow *pWindow, int button, int xpos, int ypos)
                                   {
                                       if (button == GLFW_MOUSE_BUTTON_LEFT)
                                       {
                                           // Check if clicking on tab buttons
                                           float w = (float)pWindow->width;
                                           float h = (float)pWindow->height;
                                           // Shader draws buttons at 0.1 * h from bottom (OpenGL coords)
                                           // In GLFW coords (top-left origin), this is 0.9 * h from top
                                           float buttonY = 0.9f * h;
                                           float buttonRadius = 18.0f;

                                           float centerX = w * 0.5f;
                                           float buttonPositions[3] = {centerX - 50.0f, centerX, centerX + 50.0f};

                                           for (int i = 0; i < 3; i++)
                                           {
                                               float dx = (float)xpos - buttonPositions[i];
                                               float dy = (float)ypos - buttonY;
                                               if (dx * dx + dy * dy <= buttonRadius * buttonRadius)
                                               {
                                                   demos[0].tabIndex = i;
                                                   std::cout << "Demo 1 Tab button " << i << " clicked" << std::endl;
                                                   pWindow->needUpdate = true;
                                                   return;
                                               }
                                           }
                                       }
                                   },

                                   [](cg::ManagedWindow *pWindow, double xoffset, double yoffset, int mods) {},

                                   // CursorposCallback - track mouse for flashlight effect
                                   [](cg::ManagedWindow *pWindow, int xpos, int ypos, double xoffset, double yoffset, int mods)
                                   {
                                       demos[0].mouseX = static_cast<float>(xpos);
                                       demos[0].mouseY = static_cast<float>(ypos);
                                       pWindow->needUpdate = true;
                                   }});
    }
    else if (demoIndex == 1)
    {
        // Demo 2: Ray Casting - Full Window
        fullWindow = wm.addWindow({cg::State::SHOW,
                                   nullptr,
                                   {150, 100, 1024, 768},
                                   "Assignment 2 - Ray Casting",
                                   BLACK,
                                   NONE,
                                   NONE,
                                   {{{NOMARGIN},
                                     BLACK,
                                     DISABLE,
                                     {{QUAD}},
                                     NONE,
                                     NONE,
                                     DEFAULT,
                                     {
#include "Shaders/RayCast.vert.glsl"
                                         ,
#include "Shaders/RayCast.frag.glsl"
                                     }}},

                                   [](cg::ManagedWindow *pWindow, int focused)
                                   {
                                       if (focused)
                                           std::cout << "Assignment 2 - Full Window (WASD + Mouse Drag)" << std::endl;
                                   },

                                   // InitCallback - uses shared state
                                   [](cg::ManagedWindow *pWindow, int pass)
                                   {
                                       if (pWindow->passes.empty())
                                           return;
                                       auto &program = pWindow->passes[0].renderShader;
                                       program.use();
                                       program.setUniform("playerPos", glm::vec2(demos[1].playerX, demos[1].playerY));
                                       program.setUniform("playerAngle", demos[1].playerAngle);
                                   },

                                   [](cg::ManagedWindow *pWindow, int width, int height)
                                   {
                                       pWindow->width = width;
                                       pWindow->height = height;
                                   },

                                   // KeyCallback - WASD (same logic)
                                   [](cg::ManagedWindow *pWindow, int key)
                                   {
                                       float &pX = demos[1].playerX;
                                       float &pY = demos[1].playerY;
                                       float &pA = demos[1].playerAngle;

                                       float dirX = cos(pA), dirY = sin(pA);
                                       float perpX = -dirY, perpY = dirX;
                                       float newX = pX, newY = pY;

                                       switch (key)
                                       {
                                       case GLFW_KEY_W:
                                           newX = pX + dirX * moveSpeed;
                                           newY = pY + dirY * moveSpeed;
                                           break;
                                       case GLFW_KEY_S:
                                           newX = pX - dirX * moveSpeed;
                                           newY = pY - dirY * moveSpeed;
                                           break;
                                       case GLFW_KEY_A:
                                           newX = pX - perpX * moveSpeed;
                                           newY = pY - perpY * moveSpeed;
                                           break;
                                       case GLFW_KEY_D:
                                           newX = pX + perpX * moveSpeed;
                                           newY = pY + perpY * moveSpeed;
                                           break;
                                       default:
                                           return false;
                                       }

                                       float margin = 0.2f;
                                       if (!isWall(newX + margin, pY) && !isWall(newX - margin, pY))
                                           pX = newX;
                                       if (!isWall(pX, newY + margin) && !isWall(pX, newY - margin))
                                           pY = newY;

                                       if (!pWindow->passes.empty())
                                       {
                                           auto &program = pWindow->passes[0].renderShader;
                                           program.use();
                                           program.setUniform("playerPos", glm::vec2(pX, pY));
                                       }
                                       pWindow->needUpdate = true;
                                       return true;
                                   },

                                   [](cg::ManagedWindow *pWindow, int button, int xpos, int ypos) {},
                                   [](cg::ManagedWindow *pWindow, double xoffset, double yoffset, int mods) {},

                                   // CursorposCallback - mouse look
                                   [](cg::ManagedWindow *pWindow, int xpos, int ypos, double xoffset, double yoffset, int mods)
                                   {
                                       if (glfwGetMouseButton(glfwGetCurrentContext(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
                                       {
                                           demos[1].playerAngle -= (float)xoffset * rotSpeed;
                                           if (!pWindow->passes.empty())
                                           {
                                               auto &program = pWindow->passes[0].renderShader;
                                               program.use();
                                               program.setUniform("playerAngle", demos[1].playerAngle);
                                           }
                                           pWindow->needUpdate = true;
                                       }
                                   }});
    }

    if (fullWindow)
    {
        demos[demoIndex].fullWindow = fullWindow;
        demos[demoIndex].isFullWindowOpen = true;
    }
}