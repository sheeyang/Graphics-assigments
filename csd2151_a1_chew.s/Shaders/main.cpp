/*!*****************************************************************************
\file main.cpp
\author Vadim Surov (vsurov@digipen.edu)
\coauthor SIOW YU TENG, ETHAN (s.yutengethan@digipen.edu)
\par Course: CSD2150
\par Assignment: 1 (Tabs)
\date 01/10/2026 (MM/DD/YYYY)
\brief Application entry point: initializes GLFW/Windows, constructs demo windows
	   and rendering passes, registers callbacks, and runs the render loop.

 * This file initializes the GLFW windowing system, configures an OpenGL 4.6
 * core profile rendering context, and creates a managed rendering window
 * using the custom WindowManager framework.
 *
 * The application demonstrates real-time shader rendering with interactive
 * input handling including keyboard, mouse, and window callbacks.
*******************************************************************************/
#include <iostream> // input/output

#include "windowmanager.h"
#include "trianglemeshes.h"

/// No margin constant for viewport layout
const int NOMARGIN = 0;

/// Current active shader tab index
int tabIndex{ 0 };

/**
 * @brief Application entry point.
 *
 * Initializes GLFW, configures the OpenGL rendering context, creates the
 * main application window via the WindowManager, and registers all required
 * input and window callbacks.
 *
 * The main render loop is handled internally by the WindowManager.
 *
 * @param argc Number of command-line arguments.
 * @param argv Array of command-line arguments.
 * @return Application exit code. Returns 0 on success, -1 on failure.
 */
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
/**
 * @brief Window focus callback.
 *
 * Triggered when the window gains or loses focus.
 * Displays application information and controls when focused.
 *
 * @param pWindow Pointer to the managed window.
 * @param focused Non-zero if focused, zero if unfocused.
 */
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
   - Switch Tabs: Press Tab or click on the on-screen buttons to cycle through
	 different shader effects.
   - Exit: Press Esc to close the application.

Tabs:
   - Tab 0: Animation
   - Tab 1: Dynamic Color Changes
   - Tab 2: Mouse Position Highlight
)";
				}
			},

			// InitCallback - called for initializing shaders across
			// the entire rendering process or individual passes when required
			/**
			 * @brief Initialization callback for rendering passes.
			 *
			 * Called once during initialization and whenever a render pass
			 * requires re-initialization. Used to initialize shader uniforms.
			 *
			 * @param pWindow Pointer to the managed window.
			 * @param pass Index of the rendering pass being initialized.
			 */
			[](cg::ManagedWindow* pWindow, int pass)
			{
				if (pWindow->passes.empty()) return;
				auto& program = pWindow->passes[0].renderShader;
				program.use();
				program.setUniform("Time", (float)glfwGetTime());
				program.setUniform("TabIndex", (float)tabIndex);
			},

			// SizeCallback - called when the window is resized
			/**
			* @brief Window resize callback.
			*
			* Updates the internal width and height of the managed window
			* whenever the user resizes the application window.
			*
			* @param pWindow Pointer to the managed window.
			* @param width New window width.
			* @param height New window height.
			*/

			[](cg::ManagedWindow* pWindow, int width, int height)
			{
#ifdef _DEBUG
				std::cout << "SizeCallback getting called" << std::endl;
#endif
				pWindow->width = width;
				pWindow->height = height;
			},

			// KeyCallback - called when a key is pressed
			/**
			* @brief Keyboard input callback.
			*
			* Handles keyboard input events. Currently supports:
			* - TAB key to cycle through shader tabs.
			*
			* @param pWindow Pointer to the managed window.
			* @param key GLFW key code.
			* @return True if the event was handled, false otherwise.
			*/

			[](cg::ManagedWindow* pWindow, int key)
			{
				#ifdef _DEBUG
				std::cout << "Key Pressed: ";
				#endif
				switch (key)
				{
				case GLFW_KEY_TAB:
					// Check logic to filter out Key Release events
					if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_TAB) == GLFW_PRESS)
					{
#ifdef _DEBUG
						std::cout << "Tab Key\n";
#endif
						tabIndex = (tabIndex + 1) % 3;

						if (!pWindow->passes.empty()) {
							auto& program = pWindow->passes[0].renderShader;
							program.use();
							program.setUniform("TabIndex", (float)tabIndex);
						}
						pWindow->needUpdate = true;
						return true;
					}
					return false;
				default:
					return false;
				}
			},

			// MousebuttonCallback - called when a mouse button is pressed
			/**
			* @brief Mouse button input callback.
			*
			* Handles mouse button press events. Left-click selects
			* one of three on-screen shader tabs based on cursor position.
			*
			* @param pWindow Pointer to the managed window.
			* @param button GLFW mouse button code.
			* @param xpos Mouse cursor X position.
			* @param ypos Mouse cursor Y position.
			*/

			[](cg::ManagedWindow* pWindow, int button, int xpos, int ypos) {
			switch (button)
			{
			case GLFW_MOUSE_BUTTON_LEFT:
			{
#ifdef _DEBUG
				std::cout << "Left ";
#endif
				const int w = pWindow->width;
				const int h = pWindow->height;
				const float rSq = 18.0f * 18.0f; // Radius Squared

				float btnY = h - (0.1f * h);
				float btnX[3] = { 0.50f * w - 50.0f, 0.50f * w, 0.50f * w + 50.0f };

				// Check all 3 buttons
				for (int i = 0; i < 3; ++i) {
					float dx = (float)xpos - btnX[i];
					float dy = (float)ypos - btnY;

					if (dx * dx + dy * dy <= rSq) {
						tabIndex = i;

						// Update Shader immediately
						if (!pWindow->passes.empty()) {
							auto& program = pWindow->passes[0].renderShader;
							program.use();
							program.setUniform("TabIndex", (float)tabIndex);
						}
						pWindow->needUpdate = true;
						break; // Stop checking other buttons
					}
				}
				break; // Proper break for the switch case
			}
			default:
				break;
			}
#ifdef _DEBUG
			std::cout << "Mouse Button Pressed\n";
#endif
			},

			// ScrollCallback - called when the mouse wheel is scrolled
			/**
			* @brief Mouse scroll wheel callback.
			*
			* Triggered when the mouse wheel is scrolled.
			*
			* @param pWindow Pointer to the managed window.
			* @param xoffset Horizontal scroll offset.
			* @param yoffset Vertical scroll offset.
			* @param mods Modifier keys.
			*/

			[](cg::ManagedWindow* pWindow, double xoffset, double yoffset, int mods)
			{
				#ifdef _DEBUG
				std::cout << "Mouse scroll wheel offset: (" << xoffset << ", " << yoffset << ")" << std::endl;
				#endif
			},

			// CursorposCallback - called when the mouse is moved
			/**
			* @brief Mouse movement callback.
			*
			* Triggered when the mouse cursor moves.
			* Updates the shader uniform with the current mouse position.
			*
			* @param pWindow Pointer to the managed window.
			* @param xpos Mouse cursor X position.
			* @param ypos Mouse cursor Y position.
			* @param xoffset Horizontal offset since last movement.
			* @param yoffset Vertical offset since last movement.
			* @param mods Modifier keys.
			*/

			[](cg::ManagedWindow* pWindow, int xpos, int ypos, double xoffset, double yoffset, int mods)
			{
				#ifdef _DEBUG
				std::cout << "Mouse cursor position: (" << xpos << ", " << ypos << ")" << std::endl;
				#endif
				if (!pWindow->passes.empty()) {
					auto& program = pWindow->passes[0].renderShader;
					program.use();

					float glY = (float)(pWindow->height - ypos);

					program.setUniform("Mouse", glm::vec2((float)xpos, glY));
				}
			}
			});
		windowManager.show();

		if (pMainWindow)
			glfwFocusWindow(pMainWindow);

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