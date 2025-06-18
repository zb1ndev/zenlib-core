C:/VulkanSDK/1.4.313.0/Bin/glslc.exe examples/vulkan/shaders/shader.vert -o examples/vulkan/shaders/compiled/vert.spv
C:/VulkanSDK/1.4.313.0/Bin/glslc.exe examples/vulkan/shaders/shader.frag -o examples/vulkan/shaders/compiled/frag.spv

set INCLUDE=-I"C:\VulkanSDK\1.4.313.0\Include"
set LIBS=-L"C:\VulkanSDK\1.4.313.0\Lib" -lvulkan-1 -luser32 -lkernel32 -lgdi32 -ldwmapi
set ZLIBS=build\zenlib-core.a
set FLAGS=-std=c99 -Wall -Wextra

set "VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_validation"
set "VK_LOADER_DEBUG=all"

echo [GAME] Compiling...
gcc -c examples\vulkan\main.c %INCLUDE% %FLAGS% -o examples\vulkan\build\int\main.o

echo [GAME] Linking...
gcc examples\vulkan\build\int\main.o %ZLIBS% %INCLUDE% %FLAGS% %LIBS% -o examples\vulkan\build\test.exe
echo [GAME] Finished

echo [GAME] Running...
.\examples\vulkan\build\test.exe