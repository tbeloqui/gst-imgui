#include <assert.h>
#include <stdio.h>

#include <glib.h>
#include <gst/gst.h>
#include <gst/video/video.h>
#include <gst/app/app.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <GL/gl3w.h>
#include <GLFW/glfw3.h>

static void
glfw_error_callback (int error, const char *description)
{
  g_print ("Glfw Error %d: %s\n", error, description);
}

int
main (int argc, char **argv)
{
  glfwSetErrorCallback (glfw_error_callback);
  if (!glfwInit ())
    return 1;

  const char *glsl_version = "#version 130";
  glfwWindowHint (GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint (GLFW_CONTEXT_VERSION_MINOR, 0);

  GLFWwindow *window =
      glfwCreateWindow (1280, 720, "Dear ImGui GLFW++OpenGL3+Gstreamer example",
      NULL,
      NULL);
  if (window == NULL)
    return 1;
  glfwMakeContextCurrent (window);
  glfwSwapInterval (1);         // Enable vsync

  bool err = gl3wInit () != 0;
  if (err) {
    fprintf (stderr, "Failed to initialize OpenGL loader!\n");
    return 1;
  }

  IMGUI_CHECKVERSION ();
  ImGui::CreateContext ();
  ImGui::StyleColorsDark ();

  ImGui_ImplGlfw_InitForOpenGL (window, true);
  ImGui_ImplOpenGL3_Init (glsl_version);

  GstElement *pipeline;
  GstElement *videosrc;
  GstElement *appsink;

  gst_init (&argc, &argv);

  pipeline = gst_pipeline_new ("pipeline");
  videosrc = gst_element_factory_make ("videotestsrc", "videosrc0");
  appsink = gst_element_factory_make ("appsink", "videosink0");

  gst_bin_add_many (GST_BIN (pipeline), videosrc, appsink, NULL);

  GstCaps *caps = gst_caps_new_simple ("video/x-raw",
      "format", G_TYPE_STRING, "RGBA",
      "width", G_TYPE_INT, 1280,
      "height", G_TYPE_INT, 720,
      "framerate", GST_TYPE_FRACTION, 30, 1,
      NULL);

  gboolean link_ok = gst_element_link_filtered (videosrc, appsink, caps);
  g_assert (link_ok);
  gst_caps_unref (caps);

  gst_element_set_state (pipeline, GST_STATE_PLAYING);

  GLuint videotex;
  glGenTextures (1, &videotex);
  glBindTexture (GL_TEXTURE_2D, videotex);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  while (!glfwWindowShouldClose (window)) {

    glfwPollEvents ();

    GstSample *videosample =
        gst_app_sink_try_pull_sample (GST_APP_SINK (appsink), 10 * GST_MSECOND);
    if (videosample) {
      GstBuffer *videobuf = gst_sample_get_buffer (videosample);
      GstMapInfo map;

      gst_buffer_map (videobuf, &map, GST_MAP_READ);

      glBindTexture (GL_TEXTURE_2D, videotex);
      glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, 1280, 720, 0, GL_RGBA,
          GL_UNSIGNED_BYTE, map.data);

      gst_buffer_unmap (videobuf, &map);
      gst_sample_unref (videosample);
    }

    ImGui_ImplOpenGL3_NewFrame ();
    ImGui_ImplGlfw_NewFrame ();
    ImGui::NewFrame ();

    ImGui::GetBackgroundDrawList ()->AddImage ( //
        (void *) (guintptr) videotex,   //
        ImVec2 (0, 0),          //
        ImVec2 (1280, 720),     //
        ImVec2 (0, 0),          //
        ImVec2 (1, 1)
        );

    ImGui::ShowMetricsWindow ();
    ImGui::Render ();

    int display_w, display_h;
    glfwMakeContextCurrent (window);
    glfwGetFramebufferSize (window, &display_w, &display_h);
    glViewport (0, 0, display_w, display_h);

    ImVec4 clear_color = ImVec4 (0.45f, 0.55f, 0.60f, 1.00f);
    glClearColor (clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    glClear (GL_COLOR_BUFFER_BIT);

    ImGui_ImplOpenGL3_RenderDrawData (ImGui::GetDrawData ());

    glfwMakeContextCurrent (window);
    glfwSwapBuffers (window);
  }

  gst_element_set_state (pipeline, GST_STATE_NULL);
  gst_object_unref (pipeline);

  ImGui_ImplOpenGL3_Shutdown ();
  ImGui_ImplGlfw_Shutdown ();
  ImGui::DestroyContext ();

  glfwDestroyWindow (window);
  glfwTerminate ();

  return 0;
}
