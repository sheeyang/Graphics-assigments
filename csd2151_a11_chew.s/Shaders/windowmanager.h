/*!*****************************************************************************
\file CG/windowmanager.h
\author Vadim Surov (vsurov@digipen.edu)
\date 03/11/2026 (MM/DD/YYYY)
\brief Window management interface: declares context/state, scene objects/passes,
       callbacks, a managed window wrapper, and a singleton window manager.
*******************************************************************************/
#pragma once

#include <iostream>     // For input/output
#include <list>         // For std::list
#include <array>        // For std::array

#include <GL/glew.h>    // OpenGL extensions
#include <GLFW/glfw3.h> // OpenGL and windowing

#include "material.h"
#include "texture.h"
#include "camera.h"
#include "light.h"
#include "fbo.h"
#include "object.h"

namespace cg
{
    enum State { HIDE = 0, SHOW };

    // Color and state definitions
    #define BLACK           std::initializer_list<float>{ 0.0f, 0.0f, 0.0f, 1.0f }
    #define GREY            std::initializer_list<float>{ 0.5f, 0.5f, 0.5f, 1.0f }
    #define WHITE           std::initializer_list<float>{ 1.0f, 1.0f, 1.0f, 1.0f }
    #define RED             std::initializer_list<float>{ 1.0f, 0.0f, 0.0f, 1.0f }
    #define GREEN           std::initializer_list<float>{ 0.0f, 1.0f, 0.0f, 1.0f }
    #define BLUE            std::initializer_list<float>{ 0.0f, 0.0f, 1.0f, 1.0f }

    #define DEFAULT         { }
    #define NONE            { }

    // Default dimensions and transforms
    #define WIDTH   800
    #define HEIGHT  600

    // Method to toggle fullscreen mode
    void toggleFullscreen(GLFWwindow* pWindow);

    /**
    * @brief Base class for application context holding scene and interaction related states.
    *
    * Encapsulates the rendering context, ....
    */
    struct Context
    {
        // Parameters
        int intparam[5] = { 0 };
        float floatparam[5] = { 0.0f };
        glm::vec2 vec2param[5] = { glm::vec2(0.0f) };
        glm::vec4 vec4param[5] = { glm::vec4(0.0f) };
        GLuint gluintparam[5] = { 0 };
        cg::LightFrustum lightfrustumparam = {};

        // Input state - grouped for better cache locality
        struct InputState
        {
            bool lbutton_down;
            int mods;

            double dragx;
            double dragy;

            double pressx;
            double pressy;

            double lastClickTime;
            double lastClickX;
            double lastClickY;

            InputState()
                : lbutton_down{ false }, mods{ 0 },
                dragx(0.0), dragy(0.0), pressx{ 0.0 }, pressy{ 0.0 },
                lastClickTime{ 0.0 }, lastClickX{ 0.0 }, lastClickY{ 0.0 }
            {
            }
        } input;

        // Window state - grouped and with validation
        struct WindowState
        {
            bool isFullscreen;

            // Windowed mode position and size
            int x;
            int y;
            int width;
            int height;

            WindowState()
                : isFullscreen(false),
                x(0), y(0), width(0), height(0)
            {
            }

            void setWindowedBounds(int x, int y, int width, int height) noexcept
            {
                this->x = x;
                this->y = y;
                this->width = std::max(1, width);  // Ensure minimum size
                this->height = std::max(1, height);
            }
        } window;

        // Performance monitoring - improved with moving average
        struct PerformanceMetrics
        {
            double frameLastTime;
            int frameCount;

            // Additional metrics for better performance tracking
            static constexpr int FRAME_HISTORY_SIZE = 60;
            mutable std::array<double, FRAME_HISTORY_SIZE> frameTimes;
            mutable int frameHistoryIndex;
            mutable bool historyFull;

            // Cached average to avoid recalculation
            mutable double cachedAverageFPS;
            mutable bool averageValid;

            PerformanceMetrics()
                : frameLastTime(glfwGetTime()), frameCount(0),
                frameTimes{}, frameHistoryIndex(0), historyFull(false),
                cachedAverageFPS(0.0), averageValid(false)
            {
            }

            // Efficient FPS calculation with caching
            double getAverageFPS() const noexcept
            {
                if (!averageValid)
                    recalculateAverage();

                return cachedAverageFPS;
            }

            void recordFrameTime(double deltaTime) const noexcept
            {
                if (deltaTime > 0.0)
                {
                    frameTimes[frameHistoryIndex] = deltaTime;
                    frameHistoryIndex = (frameHistoryIndex + 1) % FRAME_HISTORY_SIZE;
                    if (frameHistoryIndex == 0) historyFull = true;
                    averageValid = false; // Invalidate cache
                }
            }

        private:
            void recalculateAverage() const noexcept
            {
                if (!historyFull && frameHistoryIndex == 0) {
                    cachedAverageFPS = 0.0;
                    averageValid = true;
                    return;
                }

                int count = historyFull ? FRAME_HISTORY_SIZE : frameHistoryIndex;
                double sum = 0.0;

                for (int i = 0; i < count; ++i) {
                    if (frameTimes[i] > 0.0) {
                        sum += 1.0 / frameTimes[i]; // Convert to FPS
                    }
                }

                cachedAverageFPS = (count > 0) ? sum / count : 0.0;
                averageValid = true;
            }

        } performance;

        /**
         * @brief Construct a new Context object with improved initialization.
         * @param pScene Pointer to the scene.
         */
        Context() noexcept
            : input(), window(), performance()
        {
        }
    };

    // Structure representing a rendering pass
    struct Pass
    {
        std::vector<int> viewport;              // Viewport dimensions
        std::vector<float> backgroundColor;     // Clear color for the pass
        std::vector<bool> enableDepthBuffer;    // Depth buffer enable/disable flags
        std::vector<Object> objects;            // Objects rendered in this pass
        std::vector<Camera> cameras;            // Cameras used in this pass
        std::vector<Light> lights;              // Lights affecting this pass
        FBO fbo;                                // Framebuffer Object for the pass
        Program renderShader;                   // Shader program used for rendering

        int x, y, width, height;  // Actual viewport position and dimensions in the window space (computed at runtime)

        // Constructor for older frameworks (single depth buffer flag)
        Pass(
            const std::initializer_list<int>& viewport = { },
            const std::initializer_list<float>& backgroundColor = { },
            bool enableDepthBuffer = true,
            const std::initializer_list<Object>& objects = { },
            const std::initializer_list<Camera>& cameras = { },
            const std::initializer_list<Light>& lights = { },
            const FBO& fbo = { },
            Program renderShader = {})
            :
            viewport{ viewport },
            backgroundColor{ backgroundColor },
            enableDepthBuffer{ enableDepthBuffer },
            cameras{ cameras },
            lights{ lights },
            fbo{ fbo },
            renderShader{ renderShader },
            x{ 0 }, y{ 0 }, width{ 0 }, height{ 0 }
        {
            for (auto object : objects)
                this->objects.push_back(object);
        }
    };


    // A structure to manage a single window and its associated resources
    struct ManagedWindow
    {
        typedef void (*FocusCallback)(ManagedWindow* window, int focused);
        typedef void (*InitCallback)(ManagedWindow* window, int pass); 
        typedef void (*SizeCallback)(ManagedWindow* window, int width, int height);
        typedef bool (*KeyCallback)(ManagedWindow* window, int key);
        typedef void (*MousebuttonCallback)(ManagedWindow* window, int button, int xpos, int ypos);
        typedef void (*ScrollCallback)(ManagedWindow* window, double xoffset, double yoffset, int mods);
        typedef void (*CursorposCallback)(ManagedWindow* window, int xpos, int ypos, double xoffset, double yoffset, int mods);
        typedef void (*MouseclickCallback)(ManagedWindow* window, int xpos, int ypos);
        typedef void (*MousedoubleclickCallback)(ManagedWindow* window, int xpos, int ypos);

        State state;                            // The window's visibility state
        GLFWwindow* pWindow;                    // The GLFW window handle
        GLFWwindow* pParentWindow;              // The parent window, if any
        std::vector<int> viewport;              // The window's position and size
        const char* title;                      // The window's title
        std::vector<float> backgroundColor;     // The window's background color
        std::vector<Texture> textures;          // Textures used in the window
        std::vector<Pass> passes;               // Rendering passes for the window 
        Program computeShader;                  // Optional compute shader
        FocusCallback focusCallback;            // Callback for focus events
        InitCallback initCallback;              // Callback for initialization events
        SizeCallback sizeCallback;              // Callback for window resize events
        KeyCallback keyCallback;                // Callback for key events
        CursorposCallback cursorposCallback;    // Callback for cursor position events
        MousebuttonCallback mousebuttonCallback;// Callback for mouse button events
        ScrollCallback scrollCallback;          // Callback for scroll events   
        MouseclickCallback mouseclickCallback;  // Callback for mouse button single click events
        MousedoubleclickCallback mousedoubleclickCallback; // Callback for mouse button double click events

        Context context;                        // Each window has its own context
        int x, y, width, height;                // Actual window position and dimensions in the screen or 
                                                //  parent's window space (computed at runtime)
        bool isFocused;                         // Whether the window is currently focused
        bool needUpdate;                        // Whether the window needs to be updated (for child windows)

        ManagedWindow( 
            State state = SHOW,
            GLFWwindow* pParentWindow = nullptr,
            const std::vector<int>& viewport = {},
            const char* title = "",
            const std::vector<float>& backgroundColor = {},
            const std::vector<Texture>& textures = {},
            Program computeShader = {},
            const std::vector<Pass>& passes = {},
            FocusCallback focusCallback = nullptr,
            InitCallback initCallback = nullptr,
            SizeCallback sizeCallback = nullptr,
            KeyCallback keyCallback = nullptr,
            MousebuttonCallback mousebuttonCallback = nullptr,
            ScrollCallback scrollCallback = nullptr,
            CursorposCallback cursorposCallback = nullptr,
            MouseclickCallback mouseclickCallback = nullptr, 
            MousedoubleclickCallback mousedoubleclickCallback = nullptr)
            : 
            state{ state }, pWindow{ nullptr }, pParentWindow{ pParentWindow }, viewport{ viewport }, title{ title },
            backgroundColor{ backgroundColor }, textures{ textures }, computeShader{ computeShader }, passes{ passes },
            focusCallback{ focusCallback }, initCallback{ initCallback }, sizeCallback{ sizeCallback }, keyCallback{ keyCallback },
            mousebuttonCallback{ mousebuttonCallback }, scrollCallback{ scrollCallback }, cursorposCallback{ cursorposCallback },
			mouseclickCallback{ mouseclickCallback }, mousedoubleclickCallback{ mousedoubleclickCallback },
            x{ 0 }, y{ 0 }, width{ 0 }, height{ 0 }, isFocused{ false }, needUpdate{ true }
        {
        }

        // Clean up resources associated with a window
        void cleanup();

        // Updates the window title with the current framerate
        void updateWindowTitle();

        // Resize the window based on the viewport and parent window
        void resize(int width, int height);

        // Change the visible object in a scene
        void changeVisibleObject(int start = 1, int pass = 0);

    };

    // A singleton class to manage multiple windows
    struct WindowManager
    {
        // List of managed windows
        std::list<ManagedWindow> windows;
    private:

        // Private constructor to prevent direct instantiation
        WindowManager() = default;
        // Delete copy constructor and assignment operator
        WindowManager(const WindowManager&) = delete;
        WindowManager& operator=(const WindowManager&) = delete;
        // Delete move constructor and move assignment operator
        WindowManager(WindowManager&&) = delete;
        WindowManager& operator=(WindowManager&&) = delete;

    public:

        // Static method to get the singleton instance
        static WindowManager& getInstance()
        {
            static WindowManager instance;
            return instance;
        }

        // Special method for demo app to show a selected scene as a single scene in a window
        void copyWindow(const ManagedWindow& window);
            
        // Core method to add a new window with full specifications
        GLFWwindow* addWindow(const ManagedWindow& window, GLFWwindow* pShareWindow = nullptr);

        // Show all visible windows
        void show();

        // Render all windows
        void render();

        // Cleanup all windows
        void cleanup();

        // Resize all child windows
        void resizeChildren(GLFWwindow* pWindow, int width, int height);
    };

}