当前项目用
```
cmake -S . -B build -G "Visual Studio 17 2022" -A win32 -T v143 -DUSE_COCOS_ENGINE=ON -DBUILD_APP=ON -DBUILD_TESTS=OFF
cmake --build build --config Debug -j12
```
编译时不再报错,运行时cocos弹窗为黑屏,且有以下报错信息
```
Ready for GLSL
Ready for OpenGL 2.0

{
	cocos2d.x.version: cocos2d-x-4.0
	cocos2d.x.compiled_with_profiler: false
	supports_vertex_array_object: true
	cocos2d.x.build_type: DEBUG
	cocos2d.x.compiled_with_gl_state_cache: true
	vendor: NVIDIA Corporation
	renderer: NVIDIA GeForce RTX 4060 Laptop GPU/PCIe/SSE2
	max_texture_size: 32768
	version: 4.6.0 NVIDIA 581.29
	max_vertex_attributes: 16
	max_texture_units: 192
	max_samples_allowed: 0
	supports_NPOT: true
	supports_ETC1: false
	supports_S3TC: true
	supports_ATITC: false
	supports_PVRTC: false
	supports_BGRA8888: false
	supports_discard_framebuffer: false
	supports_OES_packed_depth_stencil: false
	supports_OES_map_buffer: false
	supports_OES_depth24: false
}


引发了异常: 读取访问权限冲突。
**this** 是 0xCDCDCDCD。

程序“[16104] Clash_of_Clans.exe”已退出，返回值为 0 (0x0)。
```

应该只是资源问题
