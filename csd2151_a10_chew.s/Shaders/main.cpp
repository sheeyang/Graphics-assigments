/*!*****************************************************************************
\file main.cpp
\author Vadim Surov (vsurov@digipen.edu)
\author YOUR NAME (YOUR_EMAIL)
\date 01/04/2026 (MM/DD/YYYY)
\brief Application entry point: initializes GLFW/Windows, constructs demo windows
       and rendering passes, registers callbacks, and runs the render loop.
*******************************************************************************/
#include <iostream> // input/output

#include "windowmanager.h"
#include "trianglemeshes.h"

const int NOMARGIN = 0;      // To set no margin for a window

// Entry point of the application
int main(int argc, char** argv)
{
    int retcode = 0; // Return code for the application
        
    // Create a window manager instance
    auto& windowManager = cg::WindowManager::getInstance();

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

        GLFWwindow* pMainWindow = nullptr; // Pointer to the main window

        // Add the main window with its properties
        pMainWindow = windowManager.addWindow({
            cg::State::SHOW,        // State (SHOW, HIDE)
            nullptr,                // Parent window (nullptr for top-level window)
            DEFAULT,                // Viewport (DEFAULT, {GAP}, {WIDTH, HEIGHT}, 
                                    //   {GRIDELEMENT, SIZEOFGRID, GAP}, {X, Y, WIDTH, HEIGHT})
            "Shaders Framework",    // Title
            NONE,                   // Background color (NONE, BLACK, GREY, WHITE, RED, GREEN, 
                                    //   BLUE, {R,G,B,A})
            NONE,                   // Textures ({filename})
            NONE,                   // Compute shader
            {                       // Rendering passes
                {                       // Pass 0
                    { NOMARGIN },           // Viewport
                    NONE,                   // Background color
                    DISABLE,                // Depth test
                    { { QUAD } },           // Objects
                    NONE,                   // Cameras
                    NONE,                   // Lights
                    DEFAULT,                // Rendering target
                    {                       // Vertex and fragment shaders
                        #include "Shaders/Shader.vert.glsl"
                        ,
                        #include "Shaders/Shader.frag.glsl"  
                    }
                }
            },

            // FocusCallback - called when the window gains or loses focus
            [](cg::ManagedWindow* pWindow, int focused)
            {
                if (focused)
                {
                    std::cout << std::endl;
                    std::cout << "Shaders Framework (version " 
                        << __DATE__ << " " << __TIME__ << ")";
                    std::cout << std::endl << R"(
   This framework provides a solid foundation for exploring the capabilities of
   modern real-time 2D and 3D graphics for developing interactive simulations
   and games. It harnesses the power of the graphics card and advanced shader
   technology to deliver smooth, visually immersive experiences.
Controls:
   - Resize: Drag the window edges to adjust the application size.
   - Fullscreen: Press Space to toggle between windowed and fullscreen mode.
   - Save Screenshot: Press 'P' to save a screenshot of the active window.
   - Exit: Press Esc to close the application.
)";
                }
            },

            // InitCallback - called for initializing shaders across
            // the entire rendering process or individual passes when required
            [](cg::ManagedWindow* pWindow, int pass)
            {
            },

            // SizeCallback - called when the window is resized
            [](cg::ManagedWindow* pWindow, int width, int height)
            {
            },

            // KeyCallback - called when a key is pressed
            [](cg::ManagedWindow* pWindow, int key)
            {
                return true; // Propagate the event by default
            },

            // MousebuttonCallback - called when a mouse button is pressed
            [](cg::ManagedWindow* pWindow, int button, int xpos, int ypos)
            {
            },

            // ScrollCallback - called when the mouse wheel is scrolled
            [](cg::ManagedWindow* pWindow, double xoffset, double yoffset, int mods)
            {
            }, 

            // CursorposCallback - called when the mouse is moved
            [](cg::ManagedWindow* pWindow, int xpos, int ypos, double xoffset, double yoffset, int mods)
            {
            }
        });

        
        // Show all windows with SHOW state
        windowManager.show();

        // Set focus to the main window if available
        if (pMainWindow)
            glfwFocusWindow(pMainWindow);

        // Start the rendering loop
        windowManager.render();
    }
    catch (const std::exception& e)
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