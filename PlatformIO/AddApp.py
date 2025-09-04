#!/usr/bin/env python3
"""
AddApp.py - Scaffold a new ESP32Berry app and wire it into the UI

Usage:
  python AddApp.py "My App" [--index 3] [--title "My App Title"]

What this does:
  - Creates src/Apps/<Folder>/ESP32Berry_App<Folder>.hpp/.cpp with a minimal AppBase-derived skeleton
  - Adds include and pointer to src/ESP32Berry.hpp
  - Adds a launch case to src/ESP32Berry.cpp (APP switch) for the chosen index
  - Shows an icon in ui_main_screen (reuses folder_BW icon) for that index

Notes:
  - Defaults to the first free index in [0..4], skipping those already used in ESP32Berry.cpp
  - Safe re-run: skips creating/adding duplicates if already present
"""

from __future__ import annotations
import argparse
import os
import re
import sys

ROOT = os.path.dirname(os.path.abspath(__file__))
SRC_DIR = os.path.join(ROOT, "src")
APPS_DIR = os.path.join(SRC_DIR, "Apps")

ESP32BERRY_HPP = os.path.join(SRC_DIR, "ESP32Berry.hpp")
ESP32BERRY_CPP = os.path.join(SRC_DIR, "ESP32Berry.cpp")
UI_MAIN = os.path.join(SRC_DIR, "UI", "Screens", "ui_main_screen.cpp")


def to_pascal(s: str) -> str:
	parts = re.findall(r"[A-Za-z0-9]+", s)
	return "".join(p.capitalize() for p in parts if p)


def detect_used_indices() -> set[int]:
	used = set()
	if not os.path.exists(ESP32BERRY_CPP):
		return used
	with open(ESP32BERRY_CPP, "r", encoding="utf-8") as f:
		text = f.read()
	# Look for: case <num>:
	for m in re.finditer(r"\bcase\s+(\d+)\s*:\s*\n", text):
		try:
			used.add(int(m.group(1)))
		except ValueError:
			pass
	return used


def pick_index(preferred: int | None) -> int:
	used = detect_used_indices()
	if preferred is not None:
		if preferred in used:
			print(f"[WARN] Preferred index {preferred} already used: {sorted(used)}")
		return preferred
	for i in range(0, 5):
		if i not in used:
			return i
	# default to 4 if all taken (script won't update UI loop size)
	return 4


def ensure_app_files(folder: str, class_suffix: str, title: str) -> tuple[str, str]:
	app_folder = os.path.join(APPS_DIR, folder)
	os.makedirs(app_folder, exist_ok=True)

	hpp_path = os.path.join(app_folder, f"ESP32Berry_App{folder}.hpp")
	cpp_path = os.path.join(app_folder, f"ESP32Berry_App{folder}.cpp")

	class_name = f"App{class_suffix}"

	if not os.path.exists(hpp_path):
		hpp = f"""#pragma once
#include \"Configurations/ESP32Berry_Config.hpp\"
#include \"Apps/ESP32Berry_AppBase.hpp\"
#include <Arduino.h>

class {class_name} : public AppBase {{
private:
  void draw_ui();

public:
  {class_name}(Display *display, System *system, Network *network, const char *title);
  ~{class_name}();
  void close_app();
}};
"""
		with open(hpp_path, "w", encoding="utf-8") as f:
			f.write(hpp)
		print(f"[ADD] {os.path.relpath(hpp_path, ROOT)}")
	else:
		print(f"[SKIP] Exists: {os.path.relpath(hpp_path, ROOT)}")

	if not os.path.exists(cpp_path):
		cpp = f"""#include \"Apps/{folder}/ESP32Berry_App{folder}.hpp\"\n
static {class_name} *instance = NULL;\n
{class_name}::{class_name}(Display *display, System *system, Network *network, const char *title)
	: AppBase(display, system, network, title) {{
  instance = this;
  this->draw_ui();
}}

{class_name}::~{class_name}() {{}}

void {class_name}::draw_ui() {{
  // Minimal UI: add a label into the app panel
  lv_obj_t *lbl = lv_label_create(ui_AppPanel);
  lv_label_set_text(lbl, \"{title}\");
  lv_obj_align(lbl, LV_ALIGN_TOP_MID, 0, 0);
}}

void {class_name}::close_app() {{
  _display->goback_main_screen();
  lv_obj_del(_bodyScreen);
  delete this;
}}
"""
		with open(cpp_path, "w", encoding="utf-8") as f:
			f.write(cpp)
		print(f"[ADD] {os.path.relpath(cpp_path, ROOT)}")
	else:
		print(f"[SKIP] Exists: {os.path.relpath(cpp_path, ROOT)}")

	return hpp_path, cpp_path


def patch_esp32berry_hpp(folder: str, class_suffix: str) -> None:
	class_name = f"App{class_suffix}"
	include_line = f"#include \"Apps/{folder}/ESP32Berry_App{folder}.hpp\""
	ptr_line = f"  {class_name} *app{class_suffix};\n"

	with open(ESP32BERRY_HPP, "r", encoding="utf-8") as f:
		lines = f.readlines()

	text = "".join(lines)
	changed = False

	if include_line not in text:
		# Insert include after the last existing Apps include
		last_idx = 0
		for i, ln in enumerate(lines):
			if ln.strip().startswith("#include \"Apps/"):
				last_idx = i
		lines.insert(last_idx + 1, include_line + "\n")
		changed = True

	if ptr_line.strip() not in text:
		# Insert before the constructor declaration or after last app ptr
		insert_idx = 0
		last_ptr_idx = 0
		for i, ln in enumerate(lines):
			if re.search(r"\bApp[A-Za-z0-9_]*\s*\*app", ln):
				last_ptr_idx = i
			if "ESP32Berry();" in ln:
				insert_idx = i
				break
		idx = last_ptr_idx + 1 if last_ptr_idx else insert_idx
		lines.insert(idx, ptr_line)
		changed = True

	if changed:
		with open(ESP32BERRY_HPP, "w", encoding="utf-8") as f:
			f.write("".join(lines))
		print(f"[MOD] {os.path.relpath(ESP32BERRY_HPP, ROOT)}")
	else:
		print(f"[SKIP] No changes: {os.path.relpath(ESP32BERRY_HPP, ROOT)}")


def patch_esp32berry_cpp(folder: str, class_suffix: str, title: str, index: int) -> None:
	class_name = f"App{class_suffix}"
	new_case = (
		f"    case {index}:\n"
		f"      instance->app{class_suffix} = new {class_name}(instance->display, instance->system, instance->network, \"{title}\");\n"
		f"      break;\n"
	)

	with open(ESP32BERRY_CPP, "r", encoding="utf-8") as f:
		text = f.read()

	if new_case in text:
		print(f"[SKIP] Launch case already exists for index {index}")
		return

	# Find switch(menuNum) block and insert before its closing brace
	m = re.search(r"switch\s*\(menuNum\)\s*\{", text)
	if not m:
		print("[ERROR] Could not find switch(menuNum) in ESP32Berry.cpp", file=sys.stderr)
		sys.exit(1)

	# Locate the end of this switch block by simple brace matching
	start = m.end()
	depth = 1
	i = start
	while i < len(text) and depth > 0:
		if text[i] == '{':
			depth += 1
		elif text[i] == '}':
			depth -= 1
		i += 1
	if depth != 0:
		print("[ERROR] Unbalanced braces in switch(menuNum)", file=sys.stderr)
		sys.exit(1)

	# Insert just before closing brace of switch
	insert_pos = i - 1
	new_text = text[:insert_pos] + new_case + text[insert_pos:]

	with open(ESP32BERRY_CPP, "w", encoding="utf-8") as f:
		f.write(new_text)
	print(f"[MOD] {os.path.relpath(ESP32BERRY_CPP, ROOT)} (added case {index})")


def patch_ui_main(index: int) -> None:
	# Add a block assigning an icon for the new index (reusing folder_BW)
	with open(UI_MAIN, "r", encoding="utf-8") as f:
		text = f.read()

	block = f"""
		if (i == {index})
		{{
			ui_btn_icon = lv_img_create(obj);
			lv_img_set_src(ui_btn_icon, &folder_BW);
			lv_obj_set_width(ui_btn_icon, LV_SIZE_CONTENT);
			lv_obj_set_height(ui_btn_icon, LV_SIZE_CONTENT);
			lv_obj_set_align(ui_btn_icon, LV_ALIGN_CENTER);
			lv_obj_add_flag(ui_btn_icon, LV_OBJ_FLAG_ADV_HITTEST);
			lv_obj_clear_flag(ui_btn_icon, LV_OBJ_FLAG_SCROLLABLE);
		}}
"""

	# If already present, skip
	if re.search(rf"if \(i == {index}\)\s*\{{", text):
		print(f"[SKIP] UI icon block for index {index} already exists")
		return

	# Heuristically insert after the last existing 'if (i == N)' block inside for loop
	# Find the for-loop header: for (int i = 0; i < 5; i++)
	m_for = re.search(r"for \(int i = 0; i < \d+; i\+\+\)\s*\{", text)
	if not m_for:
		print("[ERROR] Could not find app buttons for-loop in UI main screen", file=sys.stderr)
		sys.exit(1)

	# Insert near the existing blocks; place after last known icon block (i==2) or at the end of loop content
	# We'll place it after the last occurrence of 'if (i == ' within the loop body.
	start = m_for.end()
	# Find end of this for-loop block by brace matching
	depth = 1
	i = start
	last_if_pos = None
	while i < len(text) and depth > 0:
		if text[i] == '{':
			depth += 1
		elif text[i] == '}':
			depth -= 1
		# Track last 'if (i == ' occurrence
		if text.startswith("        if (i == ", i):
			last_if_pos = i
		i += 1
	if depth != 0:
		print("[ERROR] Unbalanced braces in UI main screen for-loop", file=sys.stderr)
		sys.exit(1)

	loop_end = i - 1
	insert_pos = loop_end
	if last_if_pos is not None:
		# find the end of that if-block to insert after it
		j = last_if_pos
		depth2 = 0
		while j < loop_end:
			if text[j] == '{':
				depth2 += 1
			elif text[j] == '}':
				depth2 -= 1
				if depth2 == 0:
					insert_pos = j + 1
					break
			j += 1

	new_text = text[:insert_pos] + block + text[insert_pos:]

	with open(UI_MAIN, "w", encoding="utf-8") as f:
		f.write(new_text)
	print(f"[MOD] {os.path.relpath(UI_MAIN, ROOT)} (added icon for index {index})")


def main():
	parser = argparse.ArgumentParser(description="Scaffold and register a new ESP32Berry app.")
	parser.add_argument("app_name", help="Name of the app (e.g., 'Weather Station')")
	parser.add_argument("--index", type=int, default=None, help="Button index on main screen (0-4). Defaults to first free.")
	parser.add_argument("--title", type=str, default=None, help="Display title (defaults to app name)")
	args = parser.parse_args()

	folder = to_pascal(args.app_name)
	class_suffix = folder
	title = args.title or args.app_name.strip()
	index = pick_index(args.index)

	print("=== AddApp ===")
	print(f"App folder     : {folder}")
	print(f"Class name     : App{class_suffix}")
	print(f"Title          : {title}")
	print(f"Main index     : {index}")

	# 1) Create files
	ensure_app_files(folder, class_suffix, title)

	# 2) Patch headers
	patch_esp32berry_hpp(folder, class_suffix)

	# 3) Patch launcher switch
	patch_esp32berry_cpp(folder, class_suffix, title, index)

	# 4) Patch UI main icon block
	patch_ui_main(index)

	print("\nDone. Build the project to verify.")


if __name__ == "__main__":
	try:
		main()
	except KeyboardInterrupt:
		print("Aborted.")
		sys.exit(130)
