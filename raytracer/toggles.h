// Clean, single definition
#pragma once
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <iostream>
#include <string>

struct ToggleSettings {
  bool headless = false;
  bool showPositionMarkers = false;
  bool showRays = false;
  bool showLightRays = false;
  bool showPlaneFlow = false;
  int planeFlowStride = 64;

  static bool parseYesNo(const std::string &in) {
    std::string ans = in;
    auto l = ans.find_first_not_of(" \t\r\n");
    auto r = ans.find_last_not_of(" \t\r\n");
    if (l != std::string::npos && r != std::string::npos)
      ans = ans.substr(l, r - l + 1);
    for (auto &ch : ans)
      ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    return (ans == "y" || ans == "yes" || ans == "true" || ans == "1");
  }

  static bool startsWith(const std::string &s, const std::string &p) {
    return s.rfind(p, 0) == 0;
  }

  void loadFromEnv() {
    if (const char *v = std::getenv("RT_HEADLESS")) headless = parseYesNo(v);
    if (const char *v = std::getenv("RT_SHOW_POSITION_MARKERS"))
      showPositionMarkers = parseYesNo(v);
    if (const char *v = std::getenv("RT_SHOW_RAYS")) showRays = parseYesNo(v);
    if (const char *v = std::getenv("RT_SHOW_LIGHT_RAYS"))
      showLightRays = parseYesNo(v);
    if (const char *v = std::getenv("RT_SHOW_PLANE_FLOW"))
      showPlaneFlow = parseYesNo(v);
    if (const char *v = std::getenv("RT_PLANE_FLOW_STRIDE")) {
      try {
        int n = std::stoi(std::string(v));
        if (n >= 16 && n <= 512) planeFlowStride = n;
      } catch (...) {
        // ignore invalid
      }
    }
  }

  void parseArgs(int argc, char **argv) {
    for (int i = 1; i < argc; ++i) {
      std::string a(argv[i]);
      auto parseBoolArg = [&](const std::string &name, bool &out) {
        if (a == name) {
          out = true;
          return true;
        }
        if (startsWith(a, name + "=")) {
          out = parseYesNo(a.substr(name.size() + 1));
          return true;
        }
        return false;
      };

      if (a == "--headless" || a == "--no-prompt") {
        headless = true;
        continue;
      }
      if (parseBoolArg("--show-position-markers", showPositionMarkers))
        continue;
      if (parseBoolArg("--show-rays", showRays)) continue;
      if (parseBoolArg("--show-light-rays", showLightRays)) continue;
      if (parseBoolArg("--show-plane-flow", showPlaneFlow)) continue;

      if (a == "--plane-flow-stride") {
        if (i + 1 < argc) {
          try {
            int n = std::stoi(argv[++i]);
            if (n >= 16 && n <= 512) planeFlowStride = n;
          } catch (...) {
          }
        }
        continue;
      }
      if (startsWith(a, "--plane-flow-stride=")) {
        try {
          int n =
              std::stoi(a.substr(std::string("--plane-flow-stride=").size()));
          if (n >= 16 && n <= 512) planeFlowStride = n;
        } catch (...) {
        }
        continue;
      }
    }
  }

  void promptFromConsole() {
    using std::cout;
    using std::getline;
    using std::string;

    cout << "[INPUT] show_position_markers? (y/n) [n]: " << std::flush;
    string ans;
    ans.clear();
    if (getline(std::cin, ans)) showPositionMarkers = parseYesNo(ans);
    cout << "[INFO] show_position_markers = "
         << (showPositionMarkers ? "true" : "false") << "\n";

    cout << "[INPUT] show_rays? (y/n) [n]: " << std::flush;
    ans.clear();
    if (getline(std::cin, ans)) showRays = parseYesNo(ans);
    cout << "[INFO] show_rays = " << (showRays ? "true" : "false") << "\n";

    cout << "[INPUT] show_light_rays? (y/n) [n]: " << std::flush;
    ans.clear();
    if (getline(std::cin, ans)) showLightRays = parseYesNo(ans);
    cout << "[INFO] show_light_rays = " << (showLightRays ? "true" : "false")
         << "\n";

    cout << "[INPUT] show_plane_flow? (y/n) [n]: " << std::flush;
    ans.clear();
    if (getline(std::cin, ans)) showPlaneFlow = parseYesNo(ans);
    cout << "[INFO] show_plane_flow = " << (showPlaneFlow ? "true" : "false")
         << "\n";

    if (showPlaneFlow) {
      cout << "[INPUT] plane_flow_stride? [64]: " << std::flush;
      ans.clear();
      if (getline(std::cin, ans)) {
        try {
          int v = std::stoi(ans);
          if (v >= 16 && v <= 512) planeFlowStride = v;
        } catch (...) {
          // keep default
        }
      }
      cout << "[INFO] plane_flow_stride = " << planeFlowStride << "\n";
    }
  }
};
