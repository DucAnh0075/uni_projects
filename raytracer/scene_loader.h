#pragma once
#include <fstream>
#include <functional>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "light.h"
#include "math_utils.h"
#include "object.h"
#include "object_tree.h"
#include "scene.h"
#include "transform.h"

struct JsonValue {
  enum Type { OBJECT, ARRAY, STRING, NUMBER, NULL_VAL };

  Type type;
  std::string str_val;
  double num_val;
  std::map<std::string, std::shared_ptr<JsonValue>> obj_val;
  std::vector<std::shared_ptr<JsonValue>> arr_val;

  JsonValue(Type t = NULL_VAL) : type(t), num_val(0) {}

  bool isObject() const { return type == OBJECT; }
  bool isArray() const { return type == ARRAY; }
  bool isString() const { return type == STRING; }
  bool isNumber() const { return type == NUMBER; }
  bool isNull() const { return type == NULL_VAL; }

  std::shared_ptr<JsonValue> operator[](const std::string& key) {
    if (type == OBJECT && obj_val.find(key) != obj_val.end()) {
      return obj_val[key];
    }
    return std::make_shared<JsonValue>(NULL_VAL);
  }

  std::shared_ptr<JsonValue> operator[](size_t index) {
    if (type == ARRAY && index < arr_val.size() && arr_val[index]) {
      return arr_val[index];
    }
    return std::make_shared<JsonValue>(NULL_VAL);
  }

  size_t size() const {
    if (type == ARRAY) return arr_val.size();
    if (type == OBJECT) return obj_val.size();
    return 0;
  }

  Vec3 asVec3() const {
    if (type == ARRAY && arr_val.size() >= 2) {
      float x = (arr_val.size() > 0 && arr_val[0] && arr_val[0]->isNumber())
                    ? (float)arr_val[0]->num_val
                    : 0.0f;
      float y = (arr_val.size() > 1 && arr_val[1] && arr_val[1]->isNumber())
                    ? (float)arr_val[1]->num_val
                    : 0.0f;
      float z = (arr_val.size() > 2 && arr_val[2] && arr_val[2]->isNumber())
                    ? (float)arr_val[2]->num_val
                    : 0.0f;
      return Vec3(x, y, z);
    }
    return Vec3(0, 0, 0);
  }

  Vec3 asPosition() const {
    if (type == ARRAY && arr_val.size() >= 3) {
      float x = (arr_val.size() > 0 && arr_val[0] && arr_val[0]->isNumber())
                    ? (float)arr_val[0]->num_val
                    : 0.0f;
      float y = (arr_val.size() > 1 && arr_val[1] && arr_val[1]->isNumber())
                    ? (float)arr_val[1]->num_val
                    : 0.0f;
      float z = (arr_val.size() > 2 && arr_val[2] && arr_val[2]->isNumber())
                    ? (float)arr_val[2]->num_val
                    : 0.0f;
      return Vec3(x, y, z);
    }
    return Vec3(0, 0, 0);
  }

  Vec3 asColor() const {
    if (type == ARRAY && arr_val.size() >= 3) {
      float r = (arr_val.size() > 0 && arr_val[0] && arr_val[0]->isNumber())
                    ? std::max(0.0f, std::min(1.0f, (float)arr_val[0]->num_val))
                    : 0.0f;
      float g = (arr_val.size() > 1 && arr_val[1] && arr_val[1]->isNumber())
                    ? std::max(0.0f, std::min(1.0f, (float)arr_val[1]->num_val))
                    : 0.0f;
      float b = (arr_val.size() > 2 && arr_val[2] && arr_val[2]->isNumber())
                    ? std::max(0.0f, std::min(1.0f, (float)arr_val[2]->num_val))
                    : 0.0f;
      return Vec3(r, g, b);
    }
    return Vec3(0, 0, 0);
  }

  float asFloat() const { return isNumber() ? (float)num_val : 0.0f; }

  int asInt() const { return isNumber() ? (int)num_val : 0; }
};

class SceneLoader {
 private:
  static std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, (last - first + 1));
  }

  static std::shared_ptr<JsonValue> parseJsonString(const std::string& json,
                                                    size_t& pos) {
    auto result = std::make_shared<JsonValue>(JsonValue::STRING);
    pos++;
    std::string value;

    while (pos < json.length() && json[pos] != '"') {
      if (json[pos] == '\\' && pos + 1 < json.length()) {
        pos++;
        switch (json[pos]) {
          case 'n':
            value += '\n';
            break;
          case 't':
            value += '\t';
            break;
          case 'r':
            value += '\r';
            break;
          case '\\':
            value += '\\';
            break;
          case '"':
            value += '"';
            break;
          default:
            value += json[pos];
            break;
        }
      } else {
        value += json[pos];
      }
      pos++;
    }

    if (pos < json.length()) pos++;
    result->str_val = value;
    return result;
  }

  static std::shared_ptr<JsonValue> parseJsonNumber(const std::string& json,
                                                    size_t& pos) {
    auto result = std::make_shared<JsonValue>(JsonValue::NUMBER);
    std::string numStr;

    while (pos < json.length() &&
           (std::isdigit(json[pos]) || json[pos] == '.' || json[pos] == '-' ||
            json[pos] == '+' || json[pos] == 'e' || json[pos] == 'E')) {
      numStr += json[pos++];
    }

    result->num_val = std::stod(numStr);
    return result;
  }

  static std::shared_ptr<JsonValue> parseJsonArray(const std::string& json,
                                                   size_t& pos) {
    auto result = std::make_shared<JsonValue>(JsonValue::ARRAY);
    pos++;

    while (pos < json.length()) {
      while (pos < json.length() && std::isspace(json[pos])) pos++;

      if (pos >= json.length()) break;
      if (json[pos] == ']') {
        pos++;
        break;
      }

      auto value = parseJsonValue(json, pos);
      if (value) result->arr_val.push_back(value);
      while (pos < json.length() && std::isspace(json[pos])) pos++;

      if (pos < json.length() && json[pos] == ',') {
        pos++;
      } else if (pos < json.length() && json[pos] == ']') {
        pos++;
        break;
      }
    }

    return result;
  }

  static std::shared_ptr<JsonValue> parseJsonObject(const std::string& json,
                                                    size_t& pos) {
    auto result = std::make_shared<JsonValue>(JsonValue::OBJECT);
    pos++;

    while (pos < json.length()) {
      while (pos < json.length() && std::isspace(json[pos])) pos++;

      if (pos >= json.length()) break;
      if (json[pos] == '}') {
        pos++;
        break;
      }
      if (json[pos] != '"') break;
      auto keyValue = parseJsonString(json, pos);
      std::string key = keyValue->str_val;
      while (pos < json.length() &&
             (std::isspace(json[pos]) || json[pos] == ':'))
        pos++;

      auto value = parseJsonValue(json, pos);
      if (value) result->obj_val[key] = value;

      while (pos < json.length() && std::isspace(json[pos])) pos++;

      if (pos < json.length() && json[pos] == ',') {
        pos++;
      } else if (pos < json.length() && json[pos] == '}') {
        pos++;
        break;
      }
    }

    return result;
  }

  static std::shared_ptr<JsonValue> parseJsonValue(const std::string& json,
                                                   size_t& pos) {
    while (pos < json.length() && std::isspace(json[pos])) pos++;

    if (pos >= json.length()) return nullptr;

    char c = json[pos];

    if (c == '"') {
      return parseJsonString(json, pos);
    } else if (c == '{') {
      return parseJsonObject(json, pos);
    } else if (c == '[') {
      return parseJsonArray(json, pos);
    } else if (std::isdigit(c) || c == '-') {
      return parseJsonNumber(json, pos);
    } else if (json.substr(pos, 4) == "null") {
      pos += 4;
      return std::make_shared<JsonValue>(JsonValue::NULL_VAL);
    }

    return nullptr;
  }

  static std::shared_ptr<JsonValue> parseJson(const std::string& json) {
    size_t pos = 0;
    return parseJsonValue(json, pos);
  }

  /**
   * @brief Create standard object transform from position, rotation, and scale
   */
  static Transform createObjectTransform(const Vec3& position,
                                         const Vec3& rotationDegrees,
                                         const Vec3& scale) {
    Transform rotationTransform =
        Transform::rotationXDegrees(rotationDegrees.x()) *
        Transform::rotationYDegrees(rotationDegrees.y()) *
        Transform::rotationZDegrees(rotationDegrees.z());

    Transform scaleTransform =
        Transform::scaling(scale.x(), scale.y(), scale.z());
    Transform translateTransform =
        Transform::translation(position.x(), position.y(), position.z());

    return translateTransform * rotationTransform * scaleTransform;
  }

  template <typename T>
  static T parseJsonProperty(
      std::shared_ptr<JsonValue> obj, const std::string& key,
      std::function<T(std::shared_ptr<JsonValue>)> converter,
      const T& defaultValue) {
    if (obj && obj->isObject() &&
        obj->obj_val.find(key) != obj->obj_val.end()) {
      return converter(obj->obj_val[key]);
    }
    return defaultValue;
  }

  static Eigen::Vector3f parseJsonColorProperty(
      std::shared_ptr<JsonValue> obj, const std::string& key,
      const Eigen::Vector3f& defaultValue = Eigen::Vector3f(0.0f, 0.0f, 0.0f)) {
    return parseJsonProperty<Eigen::Vector3f>(
        obj, key,
        [](std::shared_ptr<JsonValue> val) -> Eigen::Vector3f {
          return RayTracer::vec3ToVector3f(val->asColor());
        },
        defaultValue);
  }

  static float parseJsonFloatProperty(std::shared_ptr<JsonValue> obj,
                                      const std::string& key,
                                      float defaultValue = 0.0f) {
    return parseJsonProperty<float>(
        obj, key,
        [](std::shared_ptr<JsonValue> val) -> float { return val->asFloat(); },
        defaultValue);
  }

  static Vec3 parseJsonVec3Property(std::shared_ptr<JsonValue> obj,
                                    const std::string& key,
                                    const Vec3& defaultValue = Vec3(0, 0, 0)) {
    return parseJsonProperty<Vec3>(
        obj, key,
        [](std::shared_ptr<JsonValue> val) -> Vec3 { return val->asVec3(); },
        defaultValue);
  }

  static int parseJsonIntProperty(std::shared_ptr<JsonValue> obj,
                                  const std::string& key,
                                  int defaultValue = 0) {
    return parseJsonProperty<int>(
        obj, key,
        [](std::shared_ptr<JsonValue> val) -> int { return val->asInt(); },
        defaultValue);
  }
  static Material parseJsonMaterial(std::shared_ptr<JsonValue> colorJson) {
    Material material;

    if (!colorJson || !colorJson->isObject()) {
      return material;
    }

    if (colorJson->obj_val.find("ambient") != colorJson->obj_val.end()) {
      Vec3 ambient = colorJson->obj_val["ambient"]->asColor();
      material.Ka = Eigen::Vector3f(ambient.x(), ambient.y(), ambient.z());
    } else {
      material.Ka = Eigen::Vector3f(0.0f, 0.0f, 0.0f);
    }

    if (colorJson->obj_val.find("diffuse") != colorJson->obj_val.end()) {
      Vec3 diffuse = colorJson->obj_val["diffuse"]->asColor();
      material.Kd = Eigen::Vector3f(diffuse.x(), diffuse.y(), diffuse.z());
    } else {
      material.Kd = Eigen::Vector3f(0.0f, 0.0f, 0.0f);
    }

    if (colorJson->obj_val.find("specular") != colorJson->obj_val.end()) {
      Vec3 specular = colorJson->obj_val["specular"]->asColor();
      material.Ks = Eigen::Vector3f(specular.x(), specular.y(), specular.z());
    } else {
      material.Ks = Eigen::Vector3f(0.0f, 0.0f, 0.0f);
    }

    if (colorJson->obj_val.find("reflected") != colorJson->obj_val.end()) {
      Vec3 reflected = colorJson->obj_val["reflected"]->asColor();
      material.Kr =
          Eigen::Vector3f(reflected.x(), reflected.y(), reflected.z());
    } else {
      material.Kr = Eigen::Vector3f(0.0f, 0.0f, 0.0f);
    }

    if (colorJson->obj_val.find("refracted") != colorJson->obj_val.end()) {
      Vec3 refracted = colorJson->obj_val["refracted"]->asColor();
      material.Kt =
          Eigen::Vector3f(refracted.x(), refracted.y(), refracted.z());
    } else {
      material.Kt = Eigen::Vector3f(0.0f, 0.0f, 0.0f);
    }

    if (colorJson->obj_val.find("shininess") != colorJson->obj_val.end()) {
      material.shininess = colorJson->obj_val["shininess"]->asFloat();
    } else {
      material.shininess = 1.0f;
    }

    return material;
  }

  static void applyPositionOffset(std::shared_ptr<Object> obj,
                                  const Vec3& offset) {
    if (!obj) return;

    Position currentPos = obj->getPos();
    Position newPos{currentPos.x + offset.x(), currentPos.y + offset.y(),
                    currentPos.z + offset.z()};
    obj->setPosition(newPos);

    std::cerr << "[INFO] Applied offset [" << offset.x() << ", " << offset.y()
              << ", " << offset.z() << "] to object at [" << currentPos.x
              << ", " << currentPos.y << ", " << currentPos.z << "] -> ["
              << newPos.x << ", " << newPos.y << ", " << newPos.z << "]"
              << std::endl;
  }

  static void setMaterialIndex(Material& material,
                               std::shared_ptr<JsonValue> objectData,
                               float defaultIndex) {
    if (objectData && objectData->isObject() &&
        objectData->obj_val.find("index") != objectData->obj_val.end()) {
      material.refractive_index = objectData->obj_val["index"]->asFloat();
    } else {
      material.refractive_index = defaultIndex;
    }
  }

  static std::shared_ptr<Object> parseJsonObject(
      std::shared_ptr<JsonValue> objJson, float defaultIndex) {
    if (!objJson || !objJson->isObject() || objJson->size() == 0) {
      return nullptr;
    }

    auto& obj_map = objJson->obj_val;
    if (obj_map.size() < 1 || obj_map.size() > 2) {
      return nullptr;
    }

    bool hasPosition = obj_map.find("position") != obj_map.end();
    if (obj_map.size() == 2 && !hasPosition) {
      return nullptr;
    }

    std::string objectType;
    std::shared_ptr<JsonValue> objectData;
    for (auto& pair : obj_map) {
      if (pair.first != "position") {
        objectType = pair.first;
        objectData = pair.second;
        break;
      }
    }

    if (objectType.empty() || !objectData) {
      return nullptr;
    }
    if (objectType == "sphere") {
      if (!objectData->isObject()) return nullptr;

      Vec3 position = objectData->obj_val["position"]->asVec3();
      float radius = objectData->obj_val["radius"]->asFloat();

      Material material = parseJsonMaterial(objectData->obj_val["color"]);
      setMaterialIndex(material, objectData, defaultIndex);

      Eigen::Vector3f eigenPos(position.x(), position.y(), position.z());
      Eigen::Vector3f eigenColor(material.Kd.x() * 255.0f,
                                 material.Kd.y() * 255.0f,
                                 material.Kd.z() * 255.0f);

      auto sphere =
          std::make_shared<Sphere>(eigenPos, radius, eigenColor, material);

      if (objectData->obj_val.find("rotation") != objectData->obj_val.end()) {
        Vec3 rotationDegrees = objectData->obj_val["rotation"]->asVec3();

        Transform rotationTransform =
            Transform::rotationXDegrees(rotationDegrees.x()) *
            Transform::rotationYDegrees(rotationDegrees.y()) *
            Transform::rotationZDegrees(rotationDegrees.z());

        Transform scaleTransform = Transform::scaling(radius, radius, radius);
        Transform translateTransform =
            Transform::translation(eigenPos.x(), eigenPos.y(), eigenPos.z());

        Transform finalTransform =
            translateTransform * rotationTransform * scaleTransform;

        auto transformedSphere = std::make_shared<ObjectTree>(
            ObjectTree::TransformNode(finalTransform, ObjectTree::SphereUnit()),
            eigenColor, material);

        return transformedSphere;
      }

      return sphere;
    } else if (objectType == "cube") {
      if (!objectData->isObject()) return nullptr;

      Vec3 position = objectData->obj_val["position"]->asVec3();
      float size = objectData->obj_val["size"]->asFloat();
      Material material = parseJsonMaterial(objectData->obj_val["color"]);
      setMaterialIndex(material, objectData, defaultIndex);

      Eigen::Vector3f eigenPos(position.x(), position.y(), position.z());
      Eigen::Vector3f eigenColor(material.Kd.x() * 255.0f,
                                 material.Kd.y() * 255.0f,
                                 material.Kd.z() * 255.0f);

      auto cube = std::make_shared<Cube>(eigenPos, size, eigenColor, material);

      if (objectData->obj_val.find("rotation") != objectData->obj_val.end()) {
        Vec3 rotationDegrees = objectData->obj_val["rotation"]->asVec3();

        Transform rotationTransform =
            Transform::rotationXDegrees(rotationDegrees.x()) *
            Transform::rotationYDegrees(rotationDegrees.y()) *
            Transform::rotationZDegrees(rotationDegrees.z());

        Transform scaleTransform = Transform::scaling(size, size, size);
        Transform translateTransform =
            Transform::translation(eigenPos.x(), eigenPos.y(), eigenPos.z());

        Transform finalTransform =
            translateTransform * rotationTransform * scaleTransform;
        auto transformedCube = std::make_shared<ObjectTree>(
            ObjectTree::TransformNode(finalTransform, ObjectTree::CubeUnit()),
            eigenColor, material);

        return transformedCube;
      }

      return cube;
    } else if (objectType == "halfSpace") {
      if (!objectData->isObject()) return nullptr;

      Vec3 position = objectData->obj_val["position"]->asVec3();
      Vec3 normal = objectData->obj_val["normal"]->asVec3();

      Material material = parseJsonMaterial(objectData->obj_val["color"]);
      setMaterialIndex(material, objectData, defaultIndex);

      float offset = normal.dot(position);

      Eigen::Vector3f eigenNormal(normal.x(), normal.y(), normal.z());

      Eigen::Vector3f eigenColor(material.Kd.x() * 255.0f,
                                 material.Kd.y() * 255.0f,
                                 material.Kd.z() * 255.0f);

      auto halfspace = std::make_shared<HalfSpace>(eigenNormal, offset,
                                                   eigenColor, material);

      return halfspace;
    }

    else if (objectType == "union" || objectType == "intersection" ||
             objectType == "difference") {
      if (!objectData->isArray() || objectData->size() < 2) return nullptr;

      Vec3 unionOffset(0, 0, 0);
      if (objJson->obj_val.find("position") != objJson->obj_val.end()) {
        unionOffset = objJson->obj_val["position"]->asVec3();
        std::cerr << "[INFO] Union has position offset: [" << unionOffset.x()
                  << ", " << unionOffset.y() << ", " << unionOffset.z() << "]"
                  << std::endl;
      }

      if (objectData->arr_val.size() < 2) {
        std::cerr << "Error: CSG operation requires at least 2 operands"
                  << std::endl;
        return nullptr;
      }
      auto objA = parseJsonObject(objectData->arr_val[0], defaultIndex);
      auto objB = parseJsonObject(objectData->arr_val[1], defaultIndex);

      if (objA && objB) {
        if (unionOffset.x() != 0 || unionOffset.y() != 0 ||
            unionOffset.z() != 0) {
          Transform translateTransform = Transform::translation(
              unionOffset.x(), unionOffset.y(), unionOffset.z());

          auto colorA = Eigen::Vector3f(objA->getColor().r, objA->getColor().g,
                                        objA->getColor().b);
          auto colorB = Eigen::Vector3f(objB->getColor().r, objB->getColor().g,
                                        objB->getColor().b);

          objA = std::make_shared<ObjectTree>(
              ObjectTree::TransformNode(translateTransform,
                                        ObjectTree::ExistingObjectNode(objA)),
              colorA, objA->getMaterial());

          objB = std::make_shared<ObjectTree>(
              ObjectTree::TransformNode(translateTransform,
                                        ObjectTree::ExistingObjectNode(objB)),
              colorB, objB->getMaterial());

          std::cerr << "[INFO] Applied transform-based offset ["
                    << unionOffset.x() << ", " << unionOffset.y() << ", "
                    << unionOffset.z() << "] to union components" << std::endl;
        }

        CSGOp op;
        if (objectType == "union")
          op = CSGOp::Union;
        else if (objectType == "intersection")
          op = CSGOp::Intersection;
        else if (objectType == "difference")
          op = CSGOp::Difference;
        else
          return nullptr;

        Eigen::Vector3f color(128, 128, 128);
        auto csgObj = std::make_shared<CSGObject>(objA, objB, op, color);
        if (objA) {
          csgObj->setMaterial(objA->getMaterial());
        }
        std::cerr << "[INFO] Created CSG " << objectType
                  << " with specification format" << std::endl;
        return csgObj;
      }
    } else if (objectType == "scaling" || objectType == "translation" ||
               objectType == "rotation") {
      if (!objectData->isObject()) return nullptr;
      if (objectData->obj_val.find("subject") == objectData->obj_val.end()) {
        return nullptr;
      }

      auto subjectObj =
          parseJsonObject(objectData->obj_val["subject"], defaultIndex);
      if (!subjectObj) return nullptr;

      Transform transform;

      if (objectType == "scaling") {
        if (objectData->obj_val.find("factors") != objectData->obj_val.end()) {
          Vec3 factors = objectData->obj_val["factors"]->asVec3();
          transform = Transform::scaling(factors.x(), factors.y(), factors.z());
        }
      } else if (objectType == "translation") {
        if (objectData->obj_val.find("factors") != objectData->obj_val.end()) {
          Vec3 factors = objectData->obj_val["factors"]->asVec3();
          transform =
              Transform::translation(factors.x(), factors.y(), factors.z());
        }
      } else if (objectType == "rotation") {
        if (objectData->obj_val.find("angle") != objectData->obj_val.end() &&
            objectData->obj_val.find("direction") !=
                objectData->obj_val.end()) {
          float angleDeg = objectData->obj_val["angle"]->asFloat();
          int direction = objectData->obj_val["direction"]->asInt();

          if (direction == 0) {
            transform = Transform::rotationX(angleDeg);
          } else if (direction == 1) {
            transform = Transform::rotationY(angleDeg);
          } else if (direction == 2) {
            transform = Transform::rotationZ(angleDeg);
          }
        }
      }

      Eigen::Vector3f color(subjectObj->getColor().r, subjectObj->getColor().g,
                            subjectObj->getColor().b);

      std::shared_ptr<ObjectTree::Node> transformNode;

      if (objectType == "scaling") {
        transformNode =
            ObjectTree::TransformNode(transform, ObjectTree::SphereUnit());
      } else {
        transformNode =
            ObjectTree::TransformNode(transform, ObjectTree::SphereUnit());
      }

      auto transformedObj = std::make_shared<ObjectTree>(transformNode, color);
      std::cerr << "[INFO] Created " << objectType << " transformation"
                << std::endl;
      return transformedObj;
    }

    return nullptr;
  }

 public:
  struct SceneConfig {
    int dpi = 128;
    Vec3 screenDimensions{2.0f, 4.0f, 0.0f};
    Vec3 screenPosition{-1.0f, 5.0f, -30.0f};
    Vec3 observerPosition{0.0f, 10.0f, 50.0f};

    Vec3 ambientLight{0.1f, 0.1f, 0.1f};
    float refractiveIndex = 1.0f;
    int recursionDepth = 1;

    float defaultLightIntensity = 1.0f;

    Vec4 screenOrigin() const {
      return Vec4(screenPosition.x(), screenPosition.y(), screenPosition.z(),
                  1.0f);
    }
    float screenWidth() const { return screenDimensions.x(); }
    float screenHeight() const { return screenDimensions.y(); }
    int screenDpiX() const { return dpi; }
    int screenDpiY() const { return dpi; }
    Vec4 observerPos() const {
      return Vec4(observerPosition.x(), observerPosition.y(),
                  observerPosition.z(), 1.0f);
    }
  };

  static bool loadScene(const std::string& filename, Scene& scene,
                        SceneConfig& config) {
    std::ifstream file(filename);
    if (!file.is_open()) {
      std::cerr << "Error: Could not open scene file: " << filename
                << std::endl;
      return false;
    }

    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    file.close();

    if (content.empty()) {
      std::cerr << "Error: Scene file is empty: " << filename << std::endl;
      return false;
    }

    return loadSceneJson(content, scene, config);
  }

 private:
  static bool loadSceneJson(const std::string& jsonContent, Scene& scene,
                            SceneConfig& config) {
    try {
      std::string trimmed = jsonContent;
      trimmed.erase(0, trimmed.find_first_not_of(" \t\n\r"));
      trimmed.erase(trimmed.find_last_not_of(" \t\n\r") + 1);

      if (trimmed.empty()) {
        std::cerr << "Error: JSON content is empty or contains only whitespace"
                  << std::endl;
        return false;
      }

      auto root = parseJson(jsonContent);
      if (!root || !root->isObject()) {
        std::cerr << "Error: Invalid JSON format" << std::endl;
        return false;
      }

      if (root->obj_val.find("screen") != root->obj_val.end()) {
        auto screenJson = root->obj_val["screen"];
        if (screenJson->isObject()) {
          if (screenJson->obj_val.find("dpi") != screenJson->obj_val.end()) {
            config.dpi = screenJson->obj_val["dpi"]->asInt();
          }
          if (screenJson->obj_val.find("dimensions") !=
              screenJson->obj_val.end()) {
            config.screenDimensions =
                screenJson->obj_val["dimensions"]->asVec3();
          }
          if (screenJson->obj_val.find("position") !=
              screenJson->obj_val.end()) {
            config.screenPosition = screenJson->obj_val["position"]->asVec3();
          }
          if (screenJson->obj_val.find("observer") !=
              screenJson->obj_val.end()) {
            config.observerPosition = screenJson->obj_val["observer"]->asVec3();
          }
        }
      }

      if (root->obj_val.find("medium") != root->obj_val.end()) {
        auto mediumJson = root->obj_val["medium"];
        if (mediumJson->isObject()) {
          if (mediumJson->obj_val.find("ambient") !=
              mediumJson->obj_val.end()) {
            config.ambientLight = mediumJson->obj_val["ambient"]->asColor();
          }
          if (mediumJson->obj_val.find("index") != mediumJson->obj_val.end()) {
            config.refractiveIndex = mediumJson->obj_val["index"]->asFloat();
          }
          if (mediumJson->obj_val.find("recursion") !=
              mediumJson->obj_val.end()) {
            config.recursionDepth = mediumJson->obj_val["recursion"]->asInt();
          }
        }
      }

      if (root->obj_val.find("background") != root->obj_val.end()) {
        auto backgroundJson = root->obj_val["background"];
        if (backgroundJson->isArray() && backgroundJson->size() == 3) {
          Vec3 bgColor = backgroundJson->asColor();
          scene.backgroundColor =
              Eigen::Vector3f(bgColor.x(), bgColor.y(), bgColor.z());
        }
      }

      if (root->obj_val.find("sources") != root->obj_val.end()) {
        auto sourcesJson = root->obj_val["sources"];
        if (sourcesJson->isArray()) {
          for (size_t i = 0; i < sourcesJson->size(); ++i) {
            if (i >= sourcesJson->arr_val.size() || !sourcesJson->arr_val[i]) {
              continue;
            }
            auto lightJson = sourcesJson->arr_val[i];
            if (lightJson->isObject()) {
              Vec3 position = lightJson->obj_val["position"]->asVec3();
              Vec3 intensity = lightJson->obj_val["intensity"]->asColor();

              Color lightColor{(int)(intensity.x() * 255),
                               (int)(intensity.y() * 255),
                               (int)(intensity.z() * 255)};
              float lightIntensity =
                  (intensity.x() + intensity.y() + intensity.z()) / 3.0f;

              scene.addLight(Light::createPointLight(position, lightColor,
                                                     lightIntensity, true, 1.0f,
                                                     0.0f, 0.0f));
            }
          }
        }
      }

      if (root->obj_val.find("objects") != root->obj_val.end()) {
        auto objectsJson = root->obj_val["objects"];
        if (objectsJson->isArray()) {
          for (size_t i = 0; i < objectsJson->size(); ++i) {
            if (i >= objectsJson->arr_val.size() || !objectsJson->arr_val[i]) {
              continue;
            }
            auto obj = parseJsonObject(objectsJson->arr_val[i],
                                       config.refractiveIndex);
            if (obj) {
              scene.addObject(obj);
            }
          }
        }
      }

      return true;

    } catch (const std::exception& e) {
      std::cerr << "Error parsing JSON scene: " << e.what() << std::endl;
      return false;
    }
  }
};