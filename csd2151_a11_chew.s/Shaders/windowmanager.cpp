/*!*****************************************************************************
\file CG/windowmanager.cpp
\author Vadim Surov (vsurov@digipen.edu)
\date 03/1123/2026 (MM/DD/YYYY)
\brief Window management implementation: creates child/main windows, handles
       callbacks and fullscreen, sets up meshes/FBOs, and drives multi-pass rendering.
*******************************************************************************/
#include <algorithm> // any_of
#include <fstream>  // file output
#include <vector>   // dynamic buffer

#include "windowmanager.h"
#include "trianglemeshes.h"

#define GLFW_EXPOSE_NATIVE_WGL 
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <windows.h>

// Local helper functions
namespace
{
    static bool saveWindowScreenshot(cg::ManagedWindow* pWindow, const std::string& filename)
    {
        if (!pWindow || !pWindow->pWindow)
            return false;

        // Ensure the window's OpenGL context is current.
        glfwMakeContextCurrent(pWindow->pWindow);

        // Use framebuffer size to match actual backbuffer resolution.
        int fbWidth = 0;
        int fbHeight = 0;
        glfwGetFramebufferSize(pWindow->pWindow, &fbWidth, &fbHeight);
        if (fbWidth <= 0 || fbHeight <= 0)
            return false;

        // Read RGB8 pixels from the backbuffer.
        const int channels = 3; // RGB
        const int srcStride = fbWidth * channels;
        std::vector<unsigned char> pixels(static_cast<size_t>(fbWidth) * static_cast<size_t>(fbHeight) * channels);

        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glReadBuffer(GL_BACK);
        glReadPixels(0, 0, fbWidth, fbHeight, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

        // BMP requires each row padded to a multiple of 4 bytes and stored bottom-to-top.
        const int rowSizeNoPad = fbWidth * channels;             // 3 bytes per pixel
        const int rowSizePad = (rowSizeNoPad + 3) & ~3;          // align to 4 bytes
        const unsigned int imageSize = static_cast<unsigned int>(rowSizePad) * static_cast<unsigned int>(fbHeight);
        const unsigned int pixelDataOffset = 14u + 40u;          // BITMAPFILEHEADER(14) + BITMAPINFOHEADER(40)
        const unsigned int fileSize = pixelDataOffset + imageSize;

        // Open output file
        std::ofstream ofs(filename, std::ios::binary);
        if (!ofs)
            return false;

        auto write_u16 = [&](unsigned short v) {
            unsigned char b[2] = {
                static_cast<unsigned char>(v & 0xFF),
                static_cast<unsigned char>((v >> 8) & 0xFF)
            };
            ofs.write(reinterpret_cast<const char*>(b), 2);
            };
        auto write_u32 = [&](unsigned int v) {
            unsigned char b[4] = {
                static_cast<unsigned char>(v & 0xFF),
                static_cast<unsigned char>((v >> 8) & 0xFF),
                static_cast<unsigned char>((v >> 16) & 0xFF),
                static_cast<unsigned char>((v >> 24) & 0xFF)
            };
            ofs.write(reinterpret_cast<const char*>(b), 4);
            };
        auto write_i32 = [&](int v) {
            write_u32(static_cast<unsigned int>(v));
            };

        // Write BITMAPFILEHEADER (14 bytes)
        ofs.put('B'); ofs.put('M');                // bfType
        write_u32(fileSize);                       // bfSize
        write_u16(0);                              // bfReserved1
        write_u16(0);                              // bfReserved2
        write_u32(pixelDataOffset);                // bfOffBits

        // Write BITMAPINFOHEADER (40 bytes)
        write_u32(40u);                            // biSize
        write_i32(fbWidth);                        // biWidth
        write_i32(fbHeight);                       // biHeight (positive => bottom-up)
        write_u16(1);                              // biPlanes
        write_u16(24);                             // biBitCount (24-bit)
        write_u32(0u);                             // biCompression (BI_RGB)
        write_u32(imageSize);                      // biSizeImage
        write_i32(0);                              // biXPelsPerMeter
        write_i32(0);                              // biYPelsPerMeter
        write_u32(0u);                             // biClrUsed
        write_u32(0u);                             // biClrImportant

        if (!ofs.good())
            return false;

        // Write pixel data: bottom-to-top, convert RGB to BGR, add per-row padding.
        std::vector<unsigned char> row(static_cast<size_t>(rowSizePad), 0);
        for (int y = 0; y < fbHeight; ++y)
        {
            // OpenGL origin is bottom-left; row 0 from glReadPixels is the bottom row.
            const int srcY = y; // already bottom-to-top order compatible with BMP
            const unsigned char* src = pixels.data() + static_cast<size_t>(srcY) * srcStride;
            unsigned char* dst = row.data();

            // Convert RGB => BGR
            for (int x = 0; x < fbWidth; ++x)
            {
                const int si = x * channels;
                const int di = x * channels;
                dst[di + 0] = src[si + 2]; // B
                dst[di + 1] = src[si + 1]; // G
                dst[di + 2] = src[si + 0]; // R
            }

            ofs.write(reinterpret_cast<const char*>(row.data()), rowSizePad);
            if (!ofs.good())
                return false;
        }

        return ofs.good();
    }


    // Get the real viewport x, y, w, h for a child window relatively 
    // the parent window with width W and height H.
    // Returns true if viewport is a full screen.
    bool xywh(const std::vector<int>& viewport, int W, int H, int& x, int& y, int& w, int& h)
    {
        if (viewport.size() == 0)
        {
            x = 0;
            y = 0;
            w = W;
            h = H;
        }
        else if (viewport.size() == 1)
        {
            x = viewport[0];
            y = viewport[0];
            w = W - viewport[0] * 2;
            h = H - viewport[0] * 2;
            return viewport[0] == 0;
        }
        else if (viewport.size() == 2)
        {
            x = 0;
            y = 0;
            w = viewport[0];
            h = viewport[1];
            return false;
        }
        else if (viewport.size() == 3)
        {
            const int viewportIndex = viewport[0];
            const int sizeOfGrid = viewport[1];

            int COL = 1;
            int ROW = 1;
            int GAP = viewport[2];

            if (sizeOfGrid == 2) { COL = 2; ROW = 1; }
            else if (sizeOfGrid == 3 || sizeOfGrid == 4) { COL = 2; ROW = 2; }
            else if (sizeOfGrid == 5 || sizeOfGrid == 6) { COL = 3; ROW = 2; }
            else if (sizeOfGrid >= 7 && sizeOfGrid <= 9) { COL = 3; ROW = 3; }
            else if (sizeOfGrid >= 10 && sizeOfGrid <= 12) { COL = 4; ROW = 3; }
            else if (sizeOfGrid >= 13 && sizeOfGrid <= 16) { COL = 4; ROW = 4; }
            else if (sizeOfGrid >= 17 && sizeOfGrid <= 20) { COL = 5; ROW = 4; }
            else if (sizeOfGrid >= 21 && sizeOfGrid <= 25) { COL = 5; ROW = 5; }
            else if (sizeOfGrid >= 26) { COL = 10; ROW = 10; }

            const int r = viewportIndex / COL;
            const int c = viewportIndex % COL;

            const int widthPerCol = (W - (COL + 1) * GAP) / COL;
            const int heightPerRow = (H - 40 - (ROW + 1) * GAP) / ROW;

            x = (c + 1) * GAP + c * widthPerCol;
            y = (r + 1) * GAP + r * heightPerRow;
            w = widthPerCol;
            h = heightPerRow;

            return false;
        }
        else if (viewport.size() == 4)
        {
            x = viewport[0];
            y = viewport[1];
            w = viewport[2];
            h = viewport[3];
            return false;
        }

        return true;
    }

}

namespace cg
{
    /**
    * \brief OpenGL debug callback function. Logs detailed debug information.
    *
    * \param source The origin of the message (e.g., API, Shader Compiler, etc.).
    * \param type The type of error (e.g., error, deprecated behavior, etc.).
    * \param id The ID of the error message.
    * \param severity The severity of the error (e.g., high, medium, etc.).
    * \param length Length of the message string.
    * \param msg The error/debug message.
    * \param param Additional user-defined parameters (not used here).
    */
    void debugCallback(GLenum source, GLenum type, GLuint id,
        GLenum severity, GLsizei length, const GLchar* msg, const void* param)
    {
        // Skip low-severity informational messages
        if (severity == GL_DEBUG_SEVERITY_NOTIFICATION)
            return;

        // Determine the source of the debug message.
        std::string sourceStr;
        switch (source) {
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
            sourceStr = "WindowSys";
            break;
        case GL_DEBUG_SOURCE_APPLICATION:
            sourceStr = "App";
            break;
        case GL_DEBUG_SOURCE_API:
            sourceStr = "OpenGL";
            break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER:
            sourceStr = "ShaderCompiler";
            break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:
            sourceStr = "3rdParty";
            break;
        case GL_DEBUG_SOURCE_OTHER:
            sourceStr = "Other";
            break;
        default:
            sourceStr = "Unknown";
        }

        // Determine the type of the debug message.
        std::string typeStr;
        switch (type) {
        case GL_DEBUG_TYPE_ERROR:
            typeStr = "Error";
            break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
            typeStr = "Deprecated";
            break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
            typeStr = "Undefined";
            break;
        case GL_DEBUG_TYPE_PORTABILITY:
            typeStr = "Portability";
            break;
        case GL_DEBUG_TYPE_PERFORMANCE:
            typeStr = "Performance";
            break;
        case GL_DEBUG_TYPE_MARKER:
            typeStr = "Marker";
            break;
        case GL_DEBUG_TYPE_PUSH_GROUP:
            typeStr = "PushGrp";
            break;
        case GL_DEBUG_TYPE_POP_GROUP:
            typeStr = "PopGrp";
            break;
        case GL_DEBUG_TYPE_OTHER:
            typeStr = "Other";
            break;
        default:
            typeStr = "Unknown";
        }

        // Determine the severity of the debug message.
        std::string sevStr;
        switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH:
            sevStr = "HIGH";
            break;
        case GL_DEBUG_SEVERITY_MEDIUM:
            sevStr = "MED";
            break;
        case GL_DEBUG_SEVERITY_LOW:
            sevStr = "LOW";
            break;
        case GL_DEBUG_SEVERITY_NOTIFICATION:
            sevStr = "NOTIFY";
            break;
        default:
            sevStr = "UNK";
        }

        // Log the formatted debug message.
        std::cerr << sourceStr << ":" << typeStr << "[" << sevStr << "]"
            << "(" << id << "): " << msg << std::endl;
    }

    // Start OpenGL debug output (if supported by the driver).
    void startDebug()
    {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(debugCallback, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DEBUG_TYPE_OTHER, GL_DEBUG_SEVERITY_LOW, 0, nullptr, GL_FALSE);
        // Filter out performance warning about fragment shader recompiles
        glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_PERFORMANCE, GL_DONT_CARE, 0, nullptr, GL_FALSE);

        // Retrieve and print OpenGL context information.
        const GLubyte* renderer = glGetString(GL_RENDERER);
        const GLubyte* vendor = glGetString(GL_VENDOR);
        const GLubyte* version = glGetString(GL_VERSION);
        const GLubyte* glslVersion = glGetString(GL_SHADING_LANGUAGE_VERSION);

        GLint major, minor, samples, sampleBuffers;
        glGetIntegerv(GL_MAJOR_VERSION, &major);
        glGetIntegerv(GL_MINOR_VERSION, &minor);
        glGetIntegerv(GL_SAMPLES, &samples);
        glGetIntegerv(GL_SAMPLE_BUFFERS, &sampleBuffers);

        // Print general OpenGL information.
        std::cout << "-------------------------------------------------------------\n"
            << "GL Vendor    : " << vendor << "\n"
            << "GL Renderer  : " << renderer << "\n"
            << "GL Version   : " << version << "\n"
            << "GL Version   : " << major << "." << minor << "\n"
            << "GLSL Version : " << glslVersion << "\n"
            << "MSAA samples : " << samples << "\n"
            << "MSAA buffers : " << sampleBuffers << "\n"
            << "-------------------------------------------------------------\n";
    }

    /**
     * @brief Helper function to get the monitor that contains the window
     * @param pWindow Pointer to the GLFW window.
     * @return Pointer to the monitor containing the window, or primary monitor if not found.
     */
    GLFWmonitor* getCurrentMonitor(GLFWwindow* pWindow)
    {
        int windowX, windowY, windowWidth, windowHeight;
        glfwGetWindowPos(pWindow, &windowX, &windowY);
        glfwGetWindowSize(pWindow, &windowWidth, &windowHeight);

        // Calculate window center
        int windowCenterX = windowX + windowWidth / 2;
        int windowCenterY = windowY + windowHeight / 2;

        int monitorCount;
        GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);

        // Check which monitor contains the window center
        for (int i = 0; i < monitorCount; ++i)
        {
            int monitorX, monitorY;
            glfwGetMonitorPos(monitors[i], &monitorX, &monitorY);

            const GLFWvidmode* mode = glfwGetVideoMode(monitors[i]);
            if (windowCenterX >= monitorX && windowCenterX < monitorX + mode->width &&
                windowCenterY >= monitorY && windowCenterY < monitorY + mode->height)
            {
                return monitors[i];
            }
        }

        // If window is not found on any monitor, return primary monitor
        return glfwGetPrimaryMonitor();
    }

    /**
     * @brief Toggles fullscreen mode for the specified window.
     *        Remembers windowed position and size for restoration.
     *        Now uses the current monitor where the window is located.
     * @param pWindow Pointer to the GLFW window.
     */
    void toggleFullscreen(GLFWwindow* pWindow)
    {
        if (auto* pManagedWindow = static_cast<cg::ManagedWindow*>(glfwGetWindowUserPointer(pWindow)))
        {
            if (!pManagedWindow->context.window.isFullscreen)
            {
                // Save windowed position and size
                int x, y, width, height;
                glfwGetWindowPos(pWindow, &x, &y);
                glfwGetWindowSize(pWindow, &width, &height);
                pManagedWindow->context.window.setWindowedBounds(x, y, width, height);

                // Get current monitor where the window is located and its video mode
                GLFWmonitor* monitor = getCurrentMonitor(pWindow);
                const GLFWvidmode* mode = glfwGetVideoMode(monitor);
                // Set window to fullscreen on current monitor
                glfwSetWindowMonitor(pWindow, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
                pManagedWindow->context.window.isFullscreen = true;
            }
            else
            {
                // Restore windowed mode
                glfwSetWindowMonitor(pWindow, nullptr,
                    pManagedWindow->context.window.x, pManagedWindow->context.window.y,
                    pManagedWindow->context.window.width, pManagedWindow->context.window.height, 0);
                pManagedWindow->context.window.isFullscreen = false;
            }
        }
    }

    // Clean up resources associated with a window
    void ManagedWindow::cleanup()
    {
        computeShader.clean();

        // Clean up passes
        for (auto& pass : passes)
        {
            // Clean up objects
            for (auto& object : pass.objects)
            {
                // Delete VAO
                if (object.vao != 0)
                {
                    glDeleteVertexArrays(1, &object.vao);
                    object.vao = 0;
                }

                // Delete VBOs
                if (!object.buffers.empty())
                {
                    glDeleteBuffers(static_cast<GLsizei>(object.buffers.size()), object.buffers.data());
                    object.buffers.clear();
                }
            }

            // Clean up FBO
            if (pass.fbo.handle != 0)
            {
                glDeleteFramebuffers(1, &pass.fbo.handle);
                pass.fbo.handle = 0;
            }

            pass.renderShader.clean();

            for (auto& texture : textures)
                texture.bind();
        }
    }

    // Updates the window title with the current framerate.
    void ManagedWindow::updateWindowTitle()
    {
        if (pParentWindow)
            return; // Only update title for main windows

        auto& perf = context.performance;
        perf.frameCount++;
        double currentTime = glfwGetTime();
        double elapsed = currentTime - perf.frameLastTime;

        if (elapsed >= 1.0)
        {
            // Record the frame time for averaging
            double frameTime = elapsed / perf.frameCount;
            perf.recordFrameTime(frameTime);

            // Get smoothed FPS
            double fps = perf.getAverageFPS();

            char buf[128];
            snprintf(buf, sizeof(buf), "%s - %.1f fps", title ? title : "", fps);
            glfwSetWindowTitle(pWindow, buf);

            perf.frameCount = 0;
            perf.frameLastTime = currentTime;
        }
    }

    // Resize the window based on the viewport and parent window
    void ManagedWindow::resize(int width, int height)
    {
        xywh(viewport, width, height, this->x, this->y, this->width, this->height);

        // Size the passes
        for (auto& pass : passes)
            xywh(pass.viewport, this->width, this->height, pass.x, pass.y, pass.width, pass.height);

        glfwSetWindowPos(pWindow, this->x, this->y);
        glfwSetWindowSize(pWindow, this->width, this->height);

        needUpdate = true;
    }

    // Change the visible object in a scene
    void ManagedWindow::changeVisibleObject(int start /*= 1*/, int pass /*= 0*/)
    {
        const size_t size = passes[pass].objects.size();
        size_t n = start;
        for (; n < size; ++n)
            if (passes[pass].objects[n].instances[0].isVisible)
                break;
        passes[pass].objects[n].instances[0].isVisible = false;
        passes[pass].objects[n + 1 == size ? start : n + 1].instances[0].isVisible = true;
    }
    
    // Special method for demo app to show a selected child window as a stand alone window
    void WindowManager::copyWindow(const ManagedWindow& window)
    {
        ManagedWindow newindow(window);
        newindow.pWindow = nullptr;
        newindow.pParentWindow = nullptr;
        newindow.viewport = DEFAULT;
        newindow.x = 0;
        newindow.y = 0;
        newindow.width = 0;
        newindow.height = 0;

        for (auto& pass : newindow.passes)
            pass.viewport = {};

        glfwShowWindow(addWindow(newindow, window.pWindow));
    }

    // Core method to add a new window with full specifications
    GLFWwindow* WindowManager::addWindow(const ManagedWindow& _window, GLFWwindow* pShareWindow /*= nullptr*/)
    {
        windows.push_back(_window);

        ManagedWindow& window = windows.back();

        // Size the window
        if (window.pParentWindow)
        {
            auto pMW = static_cast<cg::ManagedWindow*>(glfwGetWindowUserPointer(window.pParentWindow));
            xywh(window.viewport, pMW->width, pMW->height, window.x, window.y, window.width, window.height);
        }
        else
        {
            if (window.viewport.size() == 0)
            {
                window.x = 0;
                window.y = 0;
                window.width = WIDTH;
                window.height = HEIGHT;
            }
            else  if (window.viewport.size() == 3)
            {
                GLFWmonitor* monitor = glfwGetPrimaryMonitor();
                const GLFWvidmode* mode = glfwGetVideoMode(monitor);
                xywh(window.viewport, mode->width, mode->height, window.x, window.y, window.width, window.height);
            }
            else if (window.viewport.size() == 4)
            {
                window.x = window.viewport[0];
                window.y = window.viewport[1];
                window.width = window.viewport[2];
                window.height = window.viewport[3];
            }
        }

        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        glfwWindowHint(GLFW_DECORATED, window.pParentWindow ? GLFW_FALSE : GLFW_TRUE);

        window.pWindow = glfwCreateWindow(window.width, window.height, window.title, nullptr, pShareWindow);
        if (!window.pWindow)
            throw std::runtime_error("Unable to create a window and its associated OpenGL context.");

        // Size the passes

        for (auto& pass : window.passes)
            xywh(pass.viewport, window.width, window.height, pass.x, pass.y, pass.width, pass.height);

        if (window.pParentWindow)
        {
            HWND hwNative = glfwGetWin32Window(window.pWindow);
            HWND hwParentWindow = glfwGetWin32Window(window.pParentWindow);
            SetParent(hwNative, hwParentWindow);
            long style = GetWindowLong(hwNative, GWL_STYLE);
            style &= ~WS_POPUP; // remove popup style
            style |= WS_CHILDWINDOW; // add childwindow style
            SetWindowLong(hwNative, GWL_STYLE, style);
            ShowWindow(hwNative, SW_SHOWNA);

            // Set the window position relatively to the parent
            glfwSetWindowPos(window.pWindow, window.x, window.y);
        }

        // Set GLFW callback functions
        glfwSetWindowFocusCallback(window.pWindow, [](GLFWwindow* pWindow, int focused)
        {
            if (auto pManagedWindow = static_cast<cg::ManagedWindow*>(glfwGetWindowUserPointer(pWindow)))
            {
                if (pManagedWindow->pParentWindow)
                    pManagedWindow->backgroundColor = focused ? WHITE : BLACK;

                pManagedWindow->needUpdate = true;
                pManagedWindow->isFocused = focused;

                if (pManagedWindow->focusCallback)
                {
                    glfwMakeContextCurrent(pManagedWindow->pWindow);
                    pManagedWindow->focusCallback(pManagedWindow, focused);
                }
            }
        });

        // Size callback to handle window resizing
        glfwSetWindowSizeCallback(window.pWindow, [](GLFWwindow* pWindow, int width, int height)
        {
            if (auto pManagedWindow = static_cast<cg::ManagedWindow*>(glfwGetWindowUserPointer(pWindow)))
            {
                pManagedWindow->x = 0;
                pManagedWindow->y = 0;
                pManagedWindow->width = width;
                pManagedWindow->height = height;

                pManagedWindow->needUpdate = true;

                // Size the passes
                for (auto& pass : pManagedWindow->passes)
                    xywh(pass.viewport, width, height, pass.x, pass.y, pass.width, pass.height);

                if (pManagedWindow->sizeCallback)
                {
                    glfwMakeContextCurrent(pManagedWindow->pWindow);
                    pManagedWindow->sizeCallback(pManagedWindow, width, height);
                }
            }
        });

        // Key callback to handle keyboard input
        glfwSetKeyCallback(window.pWindow, [](GLFWwindow* pWindow, int key, int scancode, int action, int mods)
        {
            if (auto* pManagedWindow = static_cast<cg::ManagedWindow*>(glfwGetWindowUserPointer(pWindow)))
            {
                if (action == GLFW_PRESS || action == GLFW_REPEAT)
                {
                    if (key == GLFW_KEY_ESCAPE)
                    {
                        if (pManagedWindow->pParentWindow)
                            glfwSetWindowShouldClose(pManagedWindow->pParentWindow, GL_TRUE);
                        glfwSetWindowShouldClose(pManagedWindow->pWindow, GL_TRUE);
                    }
                    else if (key == GLFW_KEY_SPACE)
                    {
                        toggleFullscreen(pManagedWindow->pParentWindow ? pManagedWindow->pParentWindow : pManagedWindow->pWindow);
                    }
                    else if (key == GLFW_KEY_ENTER)
                    {
                        if (pManagedWindow->pParentWindow)
                            WindowManager::getInstance().copyWindow(*pManagedWindow);
                    }
                    else if (key == 'P')
                    {
                        std::string title = pManagedWindow->title ? pManagedWindow->title : "";
                        std::replace(title.begin(), title.end(), ' ', '_'); 

                        const std::string filename = "Screenshot_" + title + "_" +
                            std::to_string(pManagedWindow->width) + "x" +
                            std::to_string(pManagedWindow->height) + ".bmp";
                        if (saveWindowScreenshot(pManagedWindow, filename))
                            std::cout << "Saved screenshot to " << filename << std::endl;
                    }
                    else
                    {
                        bool propagate = false;
                        if (pManagedWindow->keyCallback)
                        {
                            glfwMakeContextCurrent(pManagedWindow->pWindow);
                            if (pManagedWindow->keyCallback(pManagedWindow, key))
                                propagate = true;
                        }
                        else
                            propagate = true;

                        if (propagate)
                        {
                            // Propagate the event to the parent window if handled
                            if (pManagedWindow->pParentWindow)
                            {
                                auto* pManagedParentWindow = static_cast<cg::ManagedWindow*>(glfwGetWindowUserPointer(pManagedWindow->pParentWindow));
                                if (pManagedParentWindow->keyCallback)
                                {
                                    glfwMakeContextCurrent(pManagedParentWindow->pWindow);
                                    pManagedParentWindow->keyCallback(pManagedParentWindow, key);
                                }
                            }
                        }
                    }
                }

                pManagedWindow->context.input.mods = mods;
            }
        });

        // Mouse button callback to handle mouse clicks
        glfwSetMouseButtonCallback(window.pWindow, [](GLFWwindow* pWindow, int button, int action, int mods)
        {
            // Handle left mouse button
            if (button == GLFW_MOUSE_BUTTON_LEFT)
            {
                if (auto* pManagedWindow = static_cast<cg::ManagedWindow*>(glfwGetWindowUserPointer(pWindow)))
                {
                    // Thresholds
                    const double clickThreshold = 5.0;      // pixels
                    const double doubleClickTime = 0.30;    // seconds

                    double xpos, ypos;
                    glfwGetCursorPos(pWindow, &xpos, &ypos);

                    if (action == GLFW_PRESS)
                    {
                        glfwFocusWindow(pManagedWindow->pWindow);

                        // Store the action and position
                        pManagedWindow->context.input.lbutton_down = true;
                        pManagedWindow->context.input.pressx = xpos;
                        pManagedWindow->context.input.pressy = ypos;
                    }
                    else if (action == GLFW_RELEASE)
                    {
                        pManagedWindow->context.input.lbutton_down = false;

                        // Calculate distance moved during drag
                        const double deltaX = xpos - pManagedWindow->context.input.pressx;
                        const double deltaY = ypos - pManagedWindow->context.input.pressy;

                        // Determine if this was a click (not a drag)
                        bool isClick = (deltaX * deltaX + deltaY * deltaY < clickThreshold * clickThreshold);

                        // Double-click detection
                        double now = glfwGetTime();
                        double dt = now - pManagedWindow->context.input.lastClickTime;

                        const double dxLast = xpos - pManagedWindow->context.input.lastClickX;
                        const double dyLast = ypos - pManagedWindow->context.input.lastClickY;
                        const double lastDistSq = dxLast * dxLast + dyLast * dyLast;

                        if (dt > 0.0 && dt <= doubleClickTime && lastDistSq < clickThreshold * clickThreshold && isClick)
                        {
                            if (pManagedWindow->pParentWindow)
                                WindowManager::getInstance().copyWindow(*pManagedWindow);
                            else if (pManagedWindow->mousedoubleclickCallback)
                            {
                                glfwMakeContextCurrent(pManagedWindow->pWindow);
                                pManagedWindow->mousedoubleclickCallback(pManagedWindow, static_cast<int>(xpos), static_cast<int>(ypos));
                            }
                        }
                        else if (pManagedWindow->mouseclickCallback && isClick)
                        {
                            glfwMakeContextCurrent(pManagedWindow->pWindow);
                            pManagedWindow->mouseclickCallback(pManagedWindow, static_cast<int>(xpos), static_cast<int>(ypos));
                        }
                        else if (pManagedWindow->mousebuttonCallback)
                        {
                            glfwMakeContextCurrent(pManagedWindow->pWindow);
                            pManagedWindow->mousebuttonCallback(pManagedWindow, button, static_cast<int>(xpos), static_cast<int>(ypos));
                        }

                        // Update last click info
                        pManagedWindow->context.input.lastClickTime = now;
                        pManagedWindow->context.input.lastClickX = xpos;
                        pManagedWindow->context.input.lastClickY = ypos;
                    }
                }
            }
        });

        // Cursor position callback to handle mouse movement
        glfwSetCursorPosCallback(window.pWindow, [](GLFWwindow* pWindow, double xpos, double ypos)
        {

            // Throttle updates to 120 fps to reduce CPU usage
            const int FPS = 120;

            static double lastThrottleTime = glfwGetTime();
            double currentTime = glfwGetTime();

            if (currentTime - lastThrottleTime < 1.0 / FPS)
                return;

            lastThrottleTime = currentTime;


            if (auto pManagedWindow = static_cast<cg::ManagedWindow*>(glfwGetWindowUserPointer(pWindow)))
            {

                if (pManagedWindow->cursorposCallback)
                {
                    glfwMakeContextCurrent(pManagedWindow->pWindow);
                    pManagedWindow->cursorposCallback(pManagedWindow,
                        static_cast<int>(xpos), static_cast<int>(ypos),
                        0.0f, 0.0f,
                        pManagedWindow->context.input.mods);
                }

                if (pManagedWindow->context.input.lbutton_down)
                {
                    if (pManagedWindow->context.input.dragx == 0.0) pManagedWindow->context.input.dragx = xpos;
                    if (pManagedWindow->context.input.dragy == 0.0) pManagedWindow->context.input.dragy = ypos;

                    if (pManagedWindow->cursorposCallback)
                    {
                        glfwMakeContextCurrent(pManagedWindow->pWindow);
                        pManagedWindow->cursorposCallback(pManagedWindow, 
                            static_cast<int>(xpos), static_cast<int>(ypos),
                            xpos - pManagedWindow->context.input.dragx, ypos - pManagedWindow->context.input.dragy,
                            pManagedWindow->context.input.mods);
                    }

                    pManagedWindow->context.input.dragx = xpos;
                    pManagedWindow->context.input.dragy = ypos;
                }
            }
        });

        // Cursor enter/leave callback to handle mouse entering/leaving window
        glfwSetCursorEnterCallback(window.pWindow, [](GLFWwindow* pWindow, int entered)
        {
        });

        // Scroll callback to handle mouse wheel scrolling
        glfwSetScrollCallback(window.pWindow, [](GLFWwindow* pWindow, double xoffset, double yoffset)
        {
            if (auto pManagedWindow = static_cast<cg::ManagedWindow*>(glfwGetWindowUserPointer(pWindow)))
                if (pManagedWindow->scrollCallback)
                {
                    glfwMakeContextCurrent(pManagedWindow->pWindow);
                    pManagedWindow->scrollCallback(pManagedWindow, xoffset, yoffset, pManagedWindow->context.input.mods);
                }
        });

        glfwMakeContextCurrent(window.pWindow);

        if (windows.size()==1)
        {
            // Initialize OpenGL extensions (GLEW)
            glewExperimental = GL_TRUE; // Enable experimental features for core profile
            GLenum err = glewInit();
            if (GLEW_OK != err)
                throw std::runtime_error(reinterpret_cast<const char*>(glewGetErrorString(err)));

            // Set up OpenGL debug context and start the debugging output
            #if defined( _DEBUG )
            startDebug();
            #endif
        }

        for (auto& texture : window.textures)
            texture.bind();
         
        for (auto& pass : window.passes)
            for (auto& object : pass.objects)
                object.bind();

        window.computeShader.build();

        for (auto& pass : window.passes)
            pass.renderShader.build();

        glfwSetWindowUserPointer(window.pWindow, &window);

        if (window.initCallback)
            window.initCallback(&window, -1);

        return window.pWindow;
    }

    void WindowManager::show()
    {
        for (auto& window : windows)
        {
            if (!window.pWindow)
                continue;

            if (window.state == State::SHOW)
            {
                window.needUpdate = true;
                glfwShowWindow(window.pWindow);
            }
            else
                glfwHideWindow(window.pWindow);
        }
    }

    // The function orchestrates the rendering process for a scene by iterating through a series 
    // of rendering passes. This function handles various stages of the rendering pipeline, 
    // including framebuffer setup, shader configuration, texture binding, lighting, 
    // and object rendering, making it versatile for multi-pass rendering in OpenGL.
    void WindowManager::render()
    {
        while (std::any_of(windows.begin(), windows.end(), 
            [](const ManagedWindow& window) { return !window.pParentWindow && window.pWindow; }))
        {
            for (auto& window : windows)
            {
                if (!window.pWindow)
                    continue;

                // Check if window should close
                if (glfwWindowShouldClose(window.pWindow))
                {
                    // Clean up associated resources
                    glfwDestroyWindow(window.pWindow);
                    window.pWindow = nullptr;
                    continue;
                }

                if (!glfwGetWindowAttrib(window.pWindow, GLFW_VISIBLE))
                    continue;

                if (window.pParentWindow)
                    if (window.needUpdate)
                        window.needUpdate = false;
                    else if (!window.isFocused)
                        continue;

                glfwMakeContextCurrent(window.pWindow);

                // Clear the window
                if (window.backgroundColor.size() == 3)
                {
                    glClearColor(window.backgroundColor[0], window.backgroundColor[1], window.backgroundColor[2], 1.0f);
                    glClear(GL_COLOR_BUFFER_BIT);
                }
                else if (window.backgroundColor.size() == 4)
                {
                    glClearColor(window.backgroundColor[0], window.backgroundColor[1], window.backgroundColor[2], window.backgroundColor[3]);
                    glClear(GL_COLOR_BUFFER_BIT);
                }

                // Iterate through all rendering passes
                int i = 0;
                for (; i < window.passes.size(); ++i)
                {
                    Pass& pass = window.passes[i];

                    // Skip when minimized
                    if (pass.width == 0 || pass.height == 0)
                        continue;

                    if (window.initCallback)
                        window.initCallback(&window, i);

                    glViewport(pass.x, pass.y, pass.width, pass.height);

                    for (auto& texture : window.textures)
                        texture.bind();

                    pass.fbo.bind(pass.width, pass.height);

                    // Clear the color buffer if a background color is specified
                    if (pass.backgroundColor.size() == 4)
                    {
                        if (pass.x == 0 && pass.y == 0 && pass.width == window.width && pass.height == window.height)
                        {
                            // Clear the whole color buffer 
                            glClearColor(pass.backgroundColor[0], pass.backgroundColor[1], pass.backgroundColor[2], pass.backgroundColor[3]);
                            glClear(GL_COLOR_BUFFER_BIT);
                        }
                        else
                        {
                            // Enable scissor test to limit clearing to viewport area
                            glEnable(GL_SCISSOR_TEST);

                            // Set scissor rectangle to match the viewport
                            glScissor(pass.x, pass.y, pass.width, pass.height);

                            glClearColor(pass.backgroundColor[0], pass.backgroundColor[1], pass.backgroundColor[2], pass.backgroundColor[3]);
                            glClear(GL_COLOR_BUFFER_BIT);

                            // Disable scissor test after clearing
                            glDisable(GL_SCISSOR_TEST);
                        }
                    }

                    // Configure depth buffer for this pass
                    if (!pass.enableDepthBuffer.empty())
                    {
                        if (pass.enableDepthBuffer[0])
                        {
                            glEnable(GL_DEPTH_TEST);
                            glDepthFunc(GL_LESS); // Explicitly set depth function
                            glClear(GL_DEPTH_BUFFER_BIT);
                        }
                        else
                        {
                            glDisable(GL_DEPTH_TEST);
                        }
                    }

                    pass.renderShader.use();

                    // Set the pass number and other parameters for rendering
                    pass.renderShader.setUniform("Pass", i);
                    pass.renderShader.setUniform("iTime", static_cast<float>(glfwGetTime()));
                    pass.renderShader.setUniform("iResolution", glm::vec2{ pass.width, pass.height });

                    // Configure lighting for the pass
                    for (int idx = 0; idx < pass.lights.size(); ++idx)
                        pass.lights[idx].setUniforms(&pass.renderShader, idx);

                    glm::mat4 V = glm::mat4(1.0f); // View matrix
                    glm::mat4 P = glm::mat4(1.0f); // Projection matrix

                    // If a single camera is used, set its uniforms and update matrices
                    if (pass.cameras.size() == 1)
                    {
                        pass.cameras[0].setUniforms(&pass.renderShader);

                        V = pass.cameras[0].getLookAt();
                        P = pass.cameras[0].getPerspective(static_cast<float>(pass.width) / pass.height);
                    }

                    // Set the view and projection matrices in the shader
                    pass.renderShader.setUniform("V", V);
                    pass.renderShader.setUniform("P", P);

                    // Render objects for the pass
                    for (const Object& object : pass.objects)
                        for (const Object::Instance& instance : object.instances)
                            if (instance.isVisible)
                            {
                                // Set material properties for the instance
                                instance.material.setUniforms(&pass.renderShader);

                                // Set the model transformation matrix and render the instance
                                pass.renderShader.setUniform("M", instance.modeltransform);

                                glBindVertexArray(object.vao);
                                glDrawElements(GL_TRIANGLES, (GLsizei)object.size, GL_UNSIGNED_INT, 0);
                                glBindVertexArray(0);
                            }

                    pass.renderShader.disuse();
                }
                // Swap front and back buffers
                glfwSwapBuffers(window.pWindow);

                // Update FPS and window title using the Context's performance metrics if available
                window.updateWindowTitle();

                // Poll for and process events
                glfwPollEvents();
            }

        }
    }

    // Clean up all managed windows
    void WindowManager::cleanup() 
    {
        // Clean up windows before GLFW termination
        for (auto& window : windows)
            window.cleanup();
        windows.clear();
    }

    // Resize all child windows of a given parent window
    void WindowManager::resizeChildren(GLFWwindow* pWindow, int width, int height)
    { 
        for (auto& window : windows)
            if (window.pParentWindow == pWindow)
                window.resize(width, height);
    }

}