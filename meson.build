project('gst-imgui', 'c', 'cpp', version : '0.1', license : 'BSD')

cc = meson.get_compiler('c')
gst_req = '>= 1.0.0'
host_system = host_machine.system()

gst_dep = dependency('gstreamer-1.0')
gst_app_dep = dependency('gstreamer-app-1.0')
gl_dep = dependency('gl')

gst_video_dep = dependency('gstreamer-video-1.0', version : gst_req,
    fallback : ['gst-plugins-base', 'video_dep'])

deps = [gst_dep, gst_app_dep, gst_video_dep,  gl_dep]

flags = []

if host_system == 'windows'
deps += [
cc.find_library('glfw3', dirs : meson.current_source_dir() + '/external/glfw3/lib-vc2017'),
cc.find_library('gdi32'),
cc.find_library('shell32'),
cc.find_library('imm32'),
]

#  gdi32.lib shell32.lib
# cl /nologo /Zi /MD /I .. /I ..\.. /I ..\libs\glfw\include /I ..\libs\gl3w *.cpp ..\imgui_impl_glfw.cpp ..\imgui_impl_opengl3.cpp ..\..\imgui*.cpp ..\libs\gl3w\GL\gl3w.c /FeDebug/example_glfw_opengl3.exe /FoDebug/ /link /LIBPATH:..\libs\glfw\lib-vc2010-32 glfw3.lib opengl32.lib gdi32.lib shell32.lib
endif

if host_system == 'linux'
deps += [
     cc.find_library('dl', required: true),
     dependency('glfw3')
]
endif

inc_dirs = [
     include_directories('external/imgui'),
     include_directories('external/imgui/examples'),
     include_directories('external/imgui/examples/libs/gl3w'),
]

if host_system == 'windows'
inc_dirs += [include_directories('external/glfw3/include')]
endif

sources = [
     'external/imgui/imgui_demo.cpp',
     'external/imgui/imgui_draw.cpp',
     'external/imgui/imgui_widgets.cpp',
     'external/imgui/imgui.cpp',
     'external/imgui/examples/imgui_impl_glfw.cpp',
     'external/imgui/examples/imgui_impl_opengl3.cpp',
     'external/imgui/examples/libs/gl3w/GL/gl3w.c',
     'main.cpp'
]

executable('gst-imgui',
    sources,
    include_directories: inc_dirs,
    dependencies : deps)

