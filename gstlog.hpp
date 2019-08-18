#pragma once

#include "imgui.h"

#include <fstream>

// GStreamer Log Widget
// based on https://github.com/ocornut/imgui/issues/300

class GstLog
{
public:
	GstLog ();
	~GstLog () = default;

	void render (bool* open);

private:

	ImGuiTextBuffer  buf;
	ImGuiTextFilter  filter;
	ImVector<int>  line_offsets;

	bool track;

	char log_path[255];
	char gst_debug[255];

	std::fstream file;
};