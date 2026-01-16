/*!*****************************************************************************
\file main.cpp
\author Vadim Surov (vsurov@digipen.edu)
\co-author Chew Shee Yang (chew.s@digipen.edu)
\par Course: CSD2151
\par Assignment: 1 (Tabs)
\date 01/11/2026 (MM/DD/YYYY)
\brief
    Application entry point for the Tabs assignment. Implements a tab-based
    interface with keyboard (Tab key) and mouse click navigation. Features:
    - Tab 1: Animated checkerboard moving down-left continuously
    - Tab 2: Blurred radial gradients with sine-wave color cycling
    - Tab 3: Checkerboard with blurred highlight following mouse cursor

    Interactions implemented:
    1. Tab key cycles through tabs (keyboard navigation)
    2. Click on tab circles to switch tabs (mouse navigation)
    3. Tab 1 has continuous animation (moving checkerboard)
    4. Tab 2 has time-based color variations (sine wave colors)
    5. Tab 3 highlights mouse position in real-time

\par AI Disclosure:
    AI (GitHub Copilot) was used to brainstorm implementation ideas and to
    write documentation comments for this assignment.
*******************************************************************************/
#include <iostream> // input/output

#include "windowmanager.h"
#include "trianglemeshes.h"

const int NOMARGIN = 0; // To set no margin for a window
const int NUM_TABS = 3; // Total number of tabs

// Tab indicator constants (must match shader)
const float TAB_CIRCLE_RADIUS = 15.0f;
const float TAB_CIRCLE_Y_OFFSET = 30.0f;
const float TAB_CIRCLE_SPACING = 40.0f;

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

        // Add the main window with its properties
        pMainWindow = windowManager.addWindow({cg::State::SHOW,               // State (SHOW, HIDE)
                                               nullptr,                       // Parent window (nullptr for top-level window)
                                               DEFAULT,                       // Viewport (DEFAULT, {GAP}, {WIDTH, HEIGHT},
                                                                              //   {GRIDELEMENT, SIZEOFGRID, GAP}, {X, Y, WIDTH, HEIGHT})
                                               "CSD2151 Assignment 1 - Tabs", // Title
                                               NONE,                          // Background color (NONE, BLACK, GREY, WHITE, RED, GREEN,
                                                                              //   BLUE, {R,G,B,A})
                                               NONE,                          // Textures ({filename})
                                               NONE,                          // Compute shader
                                               {                              // Rendering passes
                                                {                             // Pass 0
                                                 {NOMARGIN},                  // Viewport
                                                 NONE,                        // Background color
                                                 DISABLE,                     // Depth test
                                                 {{QUAD}},                    // Objects
                                                 NONE,                        // Cameras
                                                 NONE,                        // Lights
                                                 DEFAULT,                     // Rendering target
                                                 {
        // Vertex and fragment shaders
#include "Shaders/Shader.vert.glsl"
                                                     ,
#include "Shaders/Shader.frag.glsl"
                                                 }}},

                                               // FocusCallback - called when the window gains or loses focus
                                               [](cg::ManagedWindow *pWindow, int focused)
                                               {
                                                   if (focused)
                                                   {
                                                       std::cout << std::endl;
                                                       std::cout << "CSD2151 Assignment 1 - Tabs (version "
                                                                 << __DATE__ << " " << __TIME__ << ")";
                                                       std::cout << std::endl
                                                                 << R"(
   This application demonstrates a tab-based interface with multiple
   interaction methods and visual effects.

Controls:
   - Tab: Press Tab key to cycle through tabs (1 -> 2 -> 3 -> 1)
   - Mouse Click: Click on the tab circles at the bottom to switch tabs
   - Resize: Drag the window edges to adjust the application size
   - Fullscreen: Press Space to toggle between windowed and fullscreen mode
   - Save Screenshot: Press 'P' to save a screenshot of the active window
   - Exit: Press Esc to close the application

Tab Features:
   - Tab 1: Animated checkerboard pattern moving down-left continuously
   - Tab 2: Blurred radial gradients with sine-wave color cycling
   - Tab 3: Checkerboard with mouse-following highlight
)";
                                                       std::cout << std::endl;
                                                       std::cout << "Current Tab: " << (pWindow->context.intparam[0] + 1) << std::endl;
                                                   }
                                               },

                                               // InitCallback - called for initializing shaders across
                                               // the entire rendering process or individual passes when required
                                               [](cg::ManagedWindow *pWindow, int pass)
                                               {
                                                   if (pass == 0)
                                                   {
                                                       // Ensure shader is active before setting uniforms
                                                       pWindow->passes[0].renderShader.use();

                                                       // Set current tab uniform
                                                       pWindow->passes[0].renderShader.setUniform("iCurrentTab", pWindow->context.intparam[0]);

                                                       // Set mouse position uniform
                                                       pWindow->passes[0].renderShader.setUniform("iMousePos", pWindow->context.vec2param[0]);
                                                   }
                                               },

                                               // SizeCallback - called when the window is resized
                                               [](cg::ManagedWindow *pWindow, int width, int height) {},

                                               // KeyCallback - called when a key is pressed
                                               [](cg::ManagedWindow *pWindow, int key)
                                               {
                                                   if (key == GLFW_KEY_TAB)
                                                   {
                                                       // Cycle to next tab
                                                       pWindow->context.intparam[0] = (pWindow->context.intparam[0] + 1) % NUM_TABS;
                                                       std::cout << "Tab key pressed - Switched to Tab " << (pWindow->context.intparam[0] + 1) << std::endl;
                                                       return false; // Don't propagate
                                                   }
                                                   return true; // Propagate the event by default
                                               },

                                               // MousebuttonCallback - called when a mouse button is pressed
                                               [](cg::ManagedWindow *pWindow, int button, int xpos, int ypos)
                                               {
                                                   if (button == GLFW_MOUSE_BUTTON_LEFT)
                                                   {
                                                       // Check if click is on any tab circle
                                                       float centerX = pWindow->width / 2.0f;
                                                       float y = pWindow->height - TAB_CIRCLE_Y_OFFSET; // From bottom in window coordinates

                                                       for (int i = 0; i < NUM_TABS; i++)
                                                       {
                                                           float tabX = centerX + (float(i) - 1.0f) * TAB_CIRCLE_SPACING;
                                                           float tabY = y;

                                                           // Calculate distance from click to tab center
                                                           float dx = xpos - tabX;
                                                           float dy = ypos - tabY;
                                                           float dist = sqrt(dx * dx + dy * dy);

                                                           // Check if click is within the tab circle (with some extra margin)
                                                           if (dist <= TAB_CIRCLE_RADIUS + 5.0f)
                                                           {
                                                               pWindow->context.intparam[0] = i;
                                                               std::cout << "Mouse clicked on Tab " << (i + 1) << " - Switched to Tab " << (i + 1) << std::endl;
                                                               break;
                                                           }
                                                       }
                                                   }
                                               },

                                               // ScrollCallback - called when the mouse wheel is scrolled
                                               [](cg::ManagedWindow *pWindow, double xoffset, double yoffset, int mods) {},

                                               // CursorposCallback - called when the mouse is moved
                                               [](cg::ManagedWindow *pWindow, int xpos, int ypos, double xoffset, double yoffset, int mods)
                                               {
                                                   // Update mouse position for Tab 3's highlight effect
                                                   pWindow->context.vec2param[0] = glm::vec2(static_cast<float>(xpos), static_cast<float>(ypos));
                                               }});

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