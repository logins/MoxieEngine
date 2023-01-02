
# Moxie Engine

Direct3D 12 multithreaded render engine.

[![License: MIT](https://img.shields.io/badge/License-MIT-orange.svg)](https://opensource.org/licenses/MIT)

[Example application built with Moxie](Examples/MoxieLogoScene/Source/MoxieLogoScene.cpp): showing usage of cubemaps, 2d textures, dynamic buffers and world entities.  

![Moxie Logo App](MoxieLogoApp.gif)

# Features

## Direct3D 12 Renderer

[D3D12CommandQueue](lib/Moxie/Source/Graphics/D3D12/D3D12CommandQueue.h) class is used to execute [D3D12CommandList](lib/Moxie/Source/Graphics/D3D12/D3D12CommandList.h) objects.

The [D3D12GraphicsAllocator](lib/Moxie/Source/Graphics/D3D12/D3D12GraphicsAllocator.h) class handles allocation for for 
- [Buffers](lib/Moxie/Source/Graphics/D3D12/D3D12StaticBufferAllocator.h)
- [Dynamic buffers](lib/Moxie/Source/Graphics/D3D12/D3D12DynamicBufferAllocator.h)
- [Textures](lib/Moxie/Source/Graphics/D3D12/D3D12TextureAllocator.h) 
- [Resource descriptors](lib/Moxie/Source/Graphics/D3D12/D3D12DescHeapFactory.h)

The engine takes care of operating all the required resource transitions and an optional debug layer ensures that rendering is setup as intended.

## Multithreaded On CPU

The [Application](lib/Moxie/Source/Public/Application.h) class synchronises updates between the [Simulation](lib/Moxie/Source/Public/Simulator.h) thread and the [Render](lib/Moxie/Source/Graphics/Public/Renderer.h) thread.

Visible entities are each represented by a [RenderProxy](lib/Moxie/Source/Graphics/Public/MoxRenderProxy.h) object owned by the render thread.

## Abstraction Layer

The whole engine code is abstracted so that it can support multiple rendering APIs and platforms underneath.
Examples are the [GraphicsAllocator](lib/Moxie/Source/Graphics/Public/GraphicsAllocator.h) class, the [Window](lib/Moxie/Source/Graphics/Public/Window.h) and all the [GraphicsTypes](lib/Moxie/Source/Graphics/Public/GraphicsTypes.h).

The engine currently runs with Direct3D 12 on Windows, but it can potentially be extended to operate in environments such as Vulkan on Linux.

## Render Passes

Rendering is organised in self-registering render passes, deriving from [RenderPass](lib/Moxie/Source/Graphics/Features/Public/RenderPass.h).

Concrete example is the [BasePass](lib/Moxie/Source/Graphics/Features/Public/BasePass.h) which is currently rendering all the geometry.

Each render pass answers to different shader parameters read from [Drawable](lib/Moxie/Source/Graphics/Public/MoxDrawable.h) objects.

## CMake Project Structure

The entire project is configured with [CMake files](CMakeLists.txt) and it is composed of:

- [Moxie library](lib/Moxie/CMakeLists.txt)
- [Third party dependencies](lib/ThirdParty/CMakeLists.txt)
  - D3DX12 interface library
  - entt library (not currently used)
  - DirectXTex library
  - Eigen library
- Examples
  - [DynamicBuffer](Examples/DynamicBuffer/CMakeLists.txt) executable
  - [Textures](Examples/Textures/CMakeLists.txt) executable
  - [MoxieLogoScene](Examples/MoxieLogoScene/CMakeLists.txt) executable

## Misc

Applications built with Moxie can use [delegate objects](lib/Moxie/Source/Public/Delegate.h) to answer events, for example mouse and keyboard events from the window class, like done in the [textures example](Examples/Textures/Source/TexturesExample.cpp).

DDS file format for cubemap and 2D textures loading is supported through [D3D12ResourceLoader](lib/Moxie/Source/Graphics/D3D12/D3D12ResourceLoader.h) which internally relies on the dependency from DirectXTex library.  

# Disclaimer

All the code is given without guarantees and under MIT license.

Textures by Solar System Scope: [https://www.solarsystemscope.com/textures/](https://www.solarsystemscope.com/textures/).
