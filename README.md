# VulkanModelViewer

[Japanese (日本語版)](./README.ja-JP.md)

## Overview
This project is a personal attempt at building a 3D model viewer in C++ and Vulkan. Users can view 3D models by dragging and dropping them directly into the window, with an orbit camera for navigation and real-time mesh statistics.
![Cover GIF](./doc/vulkan-model-viewer.gif)

## Features
1. Model Loading & Autoscaling
   - Loads 3D model formats (OBJ, glTF) via drag-and-drop into the application window.
   - Processes mesh hierarchy, vertex positions, indices, and texture coordinate data.
   - Automatically scales loaded models so that they fit comfortably within the camera's field of view.
2. Vulkan Rendering
   - Uses Vulkan as the rendering backend for the application.
3. Texture Mapping & Shading
   - Binds diffuse and emissive textures per submesh with combined image samplers.
   - Automatically falls back to embedded 1x1 default textures when a model mesh lacks diffuse or emissive maps.
4. Orbit Camera Navigation
   - Camera supports orbiting around an adjustable 3D look target point, as well as panning using keyboard controls.

## Controls
| Input | Action |
| :--- | :--- |
| **Right Click + Drag** | Orbit / rotate camera (Yaw / Pitch) |
| **W / S** | Move camera look target forward / backward |
| **A / D** | Strafe camera look target left / right |
| **Drag & Drop** | Drop a 3D model file onto the window to load |

## Tech Stack & Dependencies
- **Language**: C++20
- **Graphics API**: Vulkan (1.2+)
- **Windowing & Input**: [GLFW](https://www.glfw.org/) (3.4)
- **3D Asset Loading**: [Assimp](https://github.com/assimp/assimp) (Open Asset Import Library)
- **Math Library**: [GLM](https://github.com/g-truc/glm) (OpenGL Mathematics)
- **Image Loading**: [stb_image](https://github.com/nothings/stb)
- **User Interface**: [Dear ImGui](https://github.com/ocornut/imgui)
- **Shader Compiler**: `glslangValidator` (included with Vulkan SDK)
- **Build System**: CMake (3.10+)

## Project Structure

```text
|-- include/
|   |-- Graphics/
|   |   |-- Vulkan/
|   |   |   |-- VulkanBuffer.hpp      # GPU buffer wrapper for vertex, index, and uniform memory
|   |   |   |-- VulkanContext.hpp     # Vulkan instance, device, queue, and surface management
|   |   |   |-- VulkanImage.hpp       # Vulkan image and memory allocation wrapper
|   |   |   |-- VulkanImageView.hpp   # Vulkan image view resource abstraction
|   |   |-- Camera.hpp                # Camera view and projection matrix calculations
|   |   |-- Mesh.hpp                  # Mesh vertex, index, and texture path definitions
|   |   |-- Model.hpp                 # Assimp model loader and node hierarchy parser
|   |   |-- OrbitCamera.hpp           # Spherical orbit camera controller
|   |   |-- Renderer.hpp              # Vulkan graphics pipeline, descriptors, and draw batching
|   |   |-- Vertex.hpp                # Vertex layout description (position, color, UV)
|   |-- IO/
|   |   |-- FileIO.hpp                # Binary file reader utility
|   |-- Input/
|   |   |-- Input.hpp                 # GLFW input polling and cursor tracking singleton
|   |-- Application.hpp               # Core application loop, swapchain, and event handlers
|-- src/
|   |-- Graphics/
|   |   |-- Vulkan/                   # Vulkan resource wrapper implementations
|   |   |-- Camera.cpp                # Camera projection and view matrix implementation
|   |   |-- Model.cpp                 # Assimp scene traversal, mesh conversion, and texture paths
|   |   |-- OrbitCamera.cpp           # Orbit camera target tracking and spherical rotation
|   |   |-- Renderer.cpp              # Graphics pipeline creation, buffer staging, and rendering
|   |-- IO/
|   |   |-- FileIO.cpp                # File read helper implementation
|   |-- Input/
|   |   |-- Input.cpp                 # Keyboard and mouse state polling implementation
|   |-- Application.cpp               # Window initialization, render loop, ImGui, and drag-and-drop
|   |-- Main.cpp                      # Program entry point
|-- resources/
|   |-- shaders/                      # GLSL vertex and fragment shaders and compiled SPIR-V binaries
|   |-- textures/                     # Fallback 1x1 diffuse and emissive textures
|-- deps/                             # Vendored third-party libraries (GLM, STB, Dear ImGui)
|-- CMakeLists.txt                    # CMake build configuration with FetchContent (GLFW, Assimp)
|-- build.bat                         # Windows build script using MinGW Makefiles
```

## Building and Running

### Prerequisites
- CMake 3.10 or higher
- C++20 compliant compiler (GCC 10+, Clang 11+, or MSVC 2019+)
- Vulkan SDK (including headers, loader, and `glslangValidator`)
- Git (required by CMake `FetchContent` to download GLFW and Assimp)

#### On Fedora:
```bash
sudo dnf install -y gcc-c++ cmake git vulkan-loader-devel vulkan-tools glslang
```

#### On Ubuntu / Debian:
```bash
sudo apt update && sudo apt install -y build-essential cmake git libvulkan-dev vulkan-tools glslang-tools libx11-xcb-dev libxkbcommon-dev libwayland-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev
```

### Build Instructions

#### On Linux / macOS / WSL:
```bash
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
cd build
./VulkanModelViewer
```

#### On Windows (CMake & Visual Studio / MinGW):
Using `build.bat` (MinGW):
```cmd
build.bat
cd build
VulkanModelViewer.exe
```

Or using CMake directly with Visual Studio:
```cmd
cmake -B build -S .
cmake --build build --config Release
cd build/Release
VulkanModelViewer.exe
```
