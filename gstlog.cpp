#include "gstlog.hpp"

#include <string>
#include <iostream>

#include <glib.h>

static const ImVec4 WARNING_COLOR = ImVec4 (1.0f, 0.75f, 0.0f, 1.0f);
static const ImVec4 ERROR_COLOR = ImVec4 (1.0f, 0.0f, 0.25f, 1.0f);
static const ImVec4 TEXT_FG_COLOR = ImVec4 (0.0f, 0.0f, 0.0f, 1.0f);
static const ImVec4 LOG_BG_COLOR = ImVec4 (0.02f, 0.02f, 0.02f, 1.0f);

GstLog::GstLog ()
	: buf ()
	, line_offsets ()
	, track (true)
	, filter ()
	, file ()
{
	g_snprintf (log_path, sizeof (log_path), "%s" G_DIR_SEPARATOR_S "gst-%p.log", g_get_tmp_dir (), this);
	g_setenv ("GST_DEBUG_FILE", log_path, TRUE);

	const gchar* gst_debug_env = g_getenv ("GST_DEBUG");
	g_snprintf (gst_debug, sizeof (gst_debug), "GST_DEBUG=%s", gst_debug_env ? gst_debug_env : "");
}

static void
render_log_line (const gchar* line, const gchar* line_end)
{
	ImDrawList* draw_list = ImGui::GetWindowDrawList ();
	const ImVec2 p = ImGui::GetCursorScreenPos ();
	ImVec4 colf;
	bool draw_bg = false;

	if (g_strstr_len (line, line_end - line, "WARN")) {
		colf = WARNING_COLOR;
		draw_bg = true;
	}
	else if (g_strstr_len (line, line_end - line, "ERROR")) {
		colf = ERROR_COLOR;
		draw_bg = true;
	}

	if (draw_bg) {
		ImVec2 text_size = ImGui::CalcTextSize (line, line_end);
		draw_list->AddRectFilled (ImVec2 (p.x, p.y), ImVec2 (p.x + text_size.x, p.y + text_size.y), ImColor (colf));

		ImGui::PushStyleColor (ImGuiCol_Text, TEXT_FG_COLOR);
		ImGui::TextUnformatted (line, line_end);
		ImGui::PopStyleColor ();
	}
	else
		ImGui::TextUnformatted (line, line_end);
}

void GstLog::render (bool* open)
{
	if (!file.is_open ())
		file.open (log_path, std::fstream::in);
	else {
		file.clear ();

		for (std::string line; std::getline (file, line); ) {
			line_offsets.push_back (buf.size ());
			buf.append (line.c_str (), line.c_str () + line.size ());
		}
	}

	ImGui::SetNextWindowSize (ImVec2 (600, 250), ImGuiCond_FirstUseEver);
	ImGui::Begin ("GStreamer Log", open);

	if (ImGui::Button ("Clear")) {
		buf.clear ();
		line_offsets.clear ();
	}
	ImGui::SameLine ();

	bool copy = ImGui::Button ("Copy");
	ImGui::SameLine ();

	bool set_track = ImGui::Button ("Follow");
	ImGui::SameLine ();

	ImGui::Text (gst_debug);
	ImGui::SameLine ();

	filter.Draw ("filter", -100.0f);
	ImGui::Separator ();

	ImGui::PushStyleColor (ImGuiCol_ChildBg, LOG_BG_COLOR);
	ImGui::BeginChild ("scrolling");
	ImGui::PushStyleVar (ImGuiStyleVar_ItemSpacing, ImVec2 (0, 1));

	if (copy)
		ImGui::LogToClipboard ();

	const char* buf_begin = buf.begin ();
	const char* line = buf_begin;
	for (int line_no = 0; line != NULL; line_no++)
	{
		const char* line_end = (line_no < line_offsets.Size) ? buf_begin + line_offsets[line_no] : NULL;

		if (filter.IsActive ()) {
			if (filter.PassFilter (line, line_end))
				render_log_line (line, line_end);
		}
		else
			render_log_line (line, line_end);

		line = line_end && line_end[1] ? line_end + 1 : NULL;
	}

	if (!set_track && track && (ImGui::GetScrollMaxY () - ImGui::GetScrollY () > 0.1f))
		track = false;

	if (set_track || track) {
		ImGui::SetScrollHere (1.0f);
		track = true;
	}

	ImGui::PopStyleVar ();
	ImGui::EndChild ();

	ImGui::PopStyleColor ();
	ImGui::End ();


}