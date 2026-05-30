/**
 * @file main.cpp
 * @brief Main entry point for the ray tracer application
 * @see docs/api_documentation.md#main-application
 */

#include <Eigen/Dense>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <limits>
#include <opencv2/opencv.hpp>

#include "light.h"
#include "object.h"
#include "render.h"
#include "scene.h"
#include "scene_loader.h"
#include "toggles.h"
#include "transform.h"

using namespace cv;
using namespace std;
using Vec3 = Eigen::Vector3f;
using Vec4 = Eigen::Vector4f;

/** @brief Main application entry point with interactive scene setup */
int main(int argc, char** argv) {
  ToggleSettings settings;
  settings.loadFromEnv();
  settings.parseArgs(argc, argv);

  // Setup lighting from environment variables
  float lightIntensity = 1.0f;
  Vec3 lightPos(0, 0, 0);
  Vec3 sunDir(-1.0f, -1.0f, -0.5f);

  if (const char* envI = std::getenv("RT_LIGHT_INTENSITY")) {
    try {
      lightIntensity = std::stof(envI);
    } catch (...) {
    }
  }
  if (const char* envLP = std::getenv("RT_LIGHT_POS")) {
    string s(envLP);
    float vals[3] = {lightPos.x(), lightPos.y(), lightPos.z()};
    size_t start = 0;
    int idx = 0;
    for (; idx < 3 && start < s.size(); ++idx) {
      size_t comma = s.find(',', start);
      string tok =
          s.substr(start, comma == string::npos ? string::npos : comma - start);
      try {
        vals[idx] = std::stof(tok);
      } catch (...) {
      }
      if (comma == string::npos)
        break;
      else
        start = comma + 1;
    }
    lightPos = Vec3(vals[0], vals[1], vals[2]);
  }
  if (const char* envSunDir = std::getenv("RT_SUN_DIR")) {
    string s(envSunDir);
    float vals[3] = {sunDir.x(), sunDir.y(), sunDir.z()};
    size_t start = 0;
    int idx = 0;
    for (; idx < 3 && start < s.size(); ++idx) {
      size_t comma = s.find(',', start);
      string tok =
          s.substr(start, comma == string::npos ? string::npos : comma - start);
      try {
        vals[idx] = std::stof(tok);
      } catch (...) {
      }
      if (comma == string::npos)
        break;
      else
        start = comma + 1;
    }
    sunDir = Vec3(vals[0], vals[1], vals[2]);
  }

  // Parse CLI arguments (cli_parser.h not available - using env vars only)

  if (!settings.headless) {
    settings.promptFromConsole();
  } else {
    cout << "[INFO] Headless mode: prompts disabled.\n";
    cout << "[INFO] show_rays=" << (settings.showRays ? "true" : "false")
         << ", show_light_rays=" << (settings.showLightRays ? "true" : "false")
         << ", show_plane_flow=" << (settings.showPlaneFlow ? "true" : "false")
         << ", plane_flow_stride=" << settings.planeFlowStride
         << ", light_intensity=" << lightIntensity << "\n";
  }

  // Try to load scene file
  SceneLoader::SceneConfig config;
  Scene scene;

  std::string sceneFilename;
  for (int i = 1; i < argc; ++i) {
    std::string arg(argv[i]);
    if (arg.find("--") == 0 || arg.find("-") == 0) {
      continue;
    }
    sceneFilename = arg;
    break;
  }

  bool sceneLoaded = false;

  if (!sceneFilename.empty()) {
    if (SceneLoader::loadScene(sceneFilename, scene, config)) {
      std::cout << "Successfully loaded scene from command line: "
                << sceneFilename << "\n";
      sceneLoaded = true;
    } else {
      std::cout << "Error loading scene file: " << sceneFilename << "\n";
      std::cerr << "Fatal: Cannot continue without valid scene file"
                << std::endl;
      return 1;
    }
  }

  // Fall back to RT_SCENE_FILE.json
  if (!sceneLoaded &&
      SceneLoader::loadScene("RT_SCENE_FILE.json", scene, config)) {
    std::cout << "Successfully loaded RT_SCENE_FILE.json\n";
    sceneLoaded = true;
  } else if (!sceneLoaded) {
    if (const char* sceneFile = std::getenv("RT_SCENE_FILE")) {
      if (SceneLoader::loadScene(sceneFile, scene, config)) {
        std::cout << "Successfully loaded scene from environment variable\n";
        sceneLoaded = true;
      }
    }
  }

  if (!sceneLoaded) {
    std::cout << "No scene file found. Using default JSON scene.\n";
    SceneLoader::loadScene("RT_SCENE_FILE.json", scene, config);
  }

#ifdef DEBUG_BUILD
  std::cout << "[DEBUG] Loaded " << scene.objects.size()
            << " objects:" << std::endl;
  for (size_t i = 0; i < scene.objects.size(); ++i) {
    auto& obj = scene.objects[i];
    Position pos = obj->getPos();
    std::cout << "[DEBUG] Object " << i << " position: [" << pos.x << ", "
              << pos.y << ", " << pos.z << "]" << std::endl;
  }

  std::cout << "[DEBUG] Observer position: [" << config.observerPos().x()
            << ", " << config.observerPos().y() << ", "
            << config.observerPos().z() << "]" << std::endl;
  std::cout << "[DEBUG] Screen origin: [" << config.screenOrigin().x() << ", "
            << config.screenOrigin().y() << ", " << config.screenOrigin().z()
            << "]" << std::endl;
  std::cout << "[DEBUG] Screen dimensions: [" << config.screenWidth() << ", "
            << config.screenHeight() << "]" << std::endl;
#endif

  // Setup screen and image
  Screen screen(config.screenOrigin(), config.screenWidth(),
                config.screenHeight(), config.screenDpiX(),
                config.screenDpiY());
  int width = screen.width * screen.dpi_x;
  int height = screen.height * screen.dpi_y;

#ifdef DEBUG_BUILD
  std::cout << "[DEBUG] Image size: " << width << "x" << height << " pixels"
            << std::endl;
#endif

  Mat image(height, width, CV_8UC3, Scalar(0, 0, 0));

  Vec3 camera = Vec3(0, 0, 0);
  Vec4 observer = config.observerPos();

  if (scene.lights.empty()) {
    scene.addLight(Light::createPointLight(Vec3(0, 0, 0), Color{255, 255, 255},
                                           config.defaultLightIntensity, true));
  }
  // Update light position from scene
  if (!scene.lights.empty()) {
    for (const auto& light : scene.lights) {
      if (light.getType() == Light::Type::Point) {
        lightPos = light.getPosition();
        break;
      } else if (light.getType() == Light::Type::Directional) {
        sunDir = light.getDirection();
      }
    }
  }

  // Auto-centering logic
  bool doAutoCenter = true;
  Vec3 centerOverride(0, 0, -5);
  bool hasCenterOverride = false;
  if (const char* cenv = std::getenv("RT_CENTER")) {
    string s(cenv);
    if (s == "0" || s == "false") {
      doAutoCenter = false;
    } else if (s == "auto" || s.empty()) {
      doAutoCenter = true;
    } else {
      float vals[3] = {centerOverride.x(), centerOverride.y(),
                       centerOverride.z()};
      size_t start = 0;
      int idx = 0;
      for (; idx < 3 && start < s.size(); ++idx) {
        size_t comma = s.find(',', start);
        string tok = s.substr(
            start, comma == string::npos ? string::npos : comma - start);
        try {
          vals[idx] = std::stof(tok);
        } catch (...) {
        }
        if (comma == string::npos)
          break;
        else
          start = comma + 1;
      }
      centerOverride = Vec3(vals[0], vals[1], vals[2]);
      hasCenterOverride = true;
      doAutoCenter = true;
    }
  }

  // Auto-center screen around objects
  if (doAutoCenter) {
    Vec3 O = RayTracer::vec4ToVec3(observer);
    float z_s = screen.origin.z();
    auto projectToPlane = [&](const Vec3& P, Vec3& S) -> bool {
      float denom = (P.z() - O.z());
      if (std::abs(denom) < 1e-6f) return false;
      float t_proj = (z_s - O.z()) / denom;
      if (t_proj <= 0.0f) return false;
      S = O + t_proj * (P - O);
      return true;
    };

    vector<float> xs, ys;
    auto consider = [&](const Vec3& P) {
      Vec3 S;
      if (projectToPlane(P, S)) {
        xs.push_back(S.x());
        ys.push_back(S.y());
      }
    };

    if (hasCenterOverride) {
      consider(centerOverride);
    } else {
      for (const auto& obj : scene.objects) {
        if (auto* s = dynamic_cast<Sphere*>(obj.get())) {
          Position p = s->getPos();
          consider(Vec3(p.x, p.y, p.z));
        } else if (auto* c = dynamic_cast<Cube*>(obj.get())) {
          Position p = c->getPos();
          consider(Vec3(p.x, p.y, p.z));
        }
      }
    }
    if (xs.empty()) {
      consider(Vec3(0, 0, -5));
    }

    if (!xs.empty()) {
      auto medianOf = [](vector<float>& v) {
        sort(v.begin(), v.end());
        size_t n = v.size();
        if (n % 2 == 1) return v[n / 2];
        return 0.5f * (v[n / 2 - 1] + v[n / 2]);
      };
      float cx = medianOf(xs);
      float cy = medianOf(ys);
      screen.origin.x() = cx - screen.width * 0.5f;
      screen.origin.y() = cy - screen.height * 0.5f;
      if (settings.headless) {
        cout << "[INFO] Centered screen around 2D median (" << cx << ", " << cy
             << ") using " << xs.size() << " centers\n";
      }
    }
  }

  // Render the scene
  Renderer::renderScene(image, scene, screen, observer, camera,
                        settings.showRays, settings.showLightRays,
                        settings.showPlaneFlow, settings.planeFlowStride,
                        settings.showPositionMarkers);

  if (!settings.headless) {
    imshow("Ray Traced Scene with Transforms", image);

    std::cout << "Press ESC, Enter, or Space to close the window..."
              << std::endl;
    int key;
    do {
      key = waitKey(0) & 0xFF;
    } while (key != 27 && key != 13 && key != 32);
  } else {
    const char* savePath = std::getenv("RT_SAVE");
    if (savePath && std::strlen(savePath) > 0) {
      try {
        imwrite(savePath, image);
        cout << "[INFO] Saved image to: " << savePath << "\n";
      } catch (const std::exception& e) {
        cerr << "[WARN] Failed to save image: " << e.what() << "\n";
      }
    }
  }
  return 0;
}