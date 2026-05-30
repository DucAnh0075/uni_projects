/**
 * @file object_tree.cpp
 * @brief Implementation of hierarchical object tree and CSG operations
 * @see docs/api_documentation.md#object-tree-system
 */

#include "object_tree.h"

/** @brief Local helper wrappers for object tree implementation */
class ObjectTree::TransformedWrapper : public Object {
  std::shared_ptr<Object> base;
  Transform T;

 public:
  TransformedWrapper(std::shared_ptr<Object> b, const Transform& t)
      : base(std::move(b)), T(t) {
    color = base->getColor();
    Eigen::Vector3f p(base->getPos().x, base->getPos().y, base->getPos().z);
    Eigen::Vector3f wp = T.transformPoint(p);
    pos = Position{wp.x(), wp.y(), wp.z()};
  }
  PixelInfo intersectWithLine(const Line& worldLine) override {
    Line objLine = T.transformLine(worldLine);
    PixelInfo hit = base->intersectWithLine(objLine);
    if (hit.color.r == 0 && hit.color.g == 0 && hit.color.b == 0) return hit;
    Eigen::Vector3f oh(hit.pos.x, hit.pos.y, hit.pos.z);
    Eigen::Vector3f wh = T.transformPoint(oh);
    PixelInfo out = hit;
    out.pos = Position{wh.x(), wh.y(), wh.z()};
    Eigen::Vector3f v = wh - worldLine.origin;
    out.distance = v.norm();
    return out;
  }
  bool contains(const Eigen::Vector3f& P) const override {
    Eigen::Vector3f Po = T.transformPointInverse(P);
    return base->contains(Po);
  }
  bool rayInterval(const Line& line, float& tEnter,
                   float& tExit) const override {
    Line obj = T.transformLine(line);
    return base->rayInterval(obj, tEnter, tExit);
  }
};

class ObjectTree::ColorOverrideWrapper : public Object {
  std::shared_ptr<Object> base;

 public:
  explicit ColorOverrideWrapper(std::shared_ptr<Object> b, const Color& c)
      : base(std::move(b)) {
    color = c;
    pos = base->getPos();
  }
  PixelInfo intersectWithLine(const Line& line) override {
    PixelInfo p = base->intersectWithLine(line);
    if (p.distance > 0.0) p.color = color;
    return p;
  }
  bool contains(const Eigen::Vector3f& P) const override {
    return base->contains(P);
  }
  bool rayInterval(const Line& line, float& tEnter,
                   float& tExit) const override {
    return base->rayInterval(line, tEnter, tExit);
  }
};

ObjectTree::ObjectTree(std::shared_ptr<Node> root_, const Eigen::Vector3f& col)
    : root(std::move(root_)) {
  color = Color{(int)col.x(), (int)col.y(), (int)col.z()};
  if (root && root->kind == Node::Kind::Transform) {
    const auto& matrix = root->transform.getMatrix();
    pos = Position{matrix(0, 3), matrix(1, 3), matrix(2, 3)};
  } else {
    pos = Position{0, 0, 0};
  }

  treeMaterial = Material();
}

ObjectTree::ObjectTree(std::shared_ptr<Node> root_, const Eigen::Vector3f& col,
                       const Material& mat)
    : root(std::move(root_)) {
  color = Color{(int)col.x(), (int)col.y(), (int)col.z()};

  if (root && root->kind == Node::Kind::Transform) {
    const auto& matrix = root->transform.getMatrix();
    pos = Position{matrix(0, 3), matrix(1, 3), matrix(2, 3)};
  } else {
    pos = Position{0, 0, 0};
  }

  treeMaterial = mat;
}

std::shared_ptr<Object> ObjectTree::compileNode(
    const std::shared_ptr<Node>& n) const {
  using Kind = Node::Kind;
  if (!n) return nullptr;
  switch (n->kind) {
    case Kind::SphereUnit: {
      return std::make_shared<Sphere>(
          Eigen::Vector3f(0, 0, 0), 1.0f,
          Eigen::Vector3f(color.r, color.g, color.b), treeMaterial);
    }
    case Kind::CubeUnit: {
      return std::make_shared<Cube>(Eigen::Vector3f(0, 0, 0), 1.0,
                                    Eigen::Vector3f(color.r, color.g, color.b),
                                    treeMaterial);
    }
    case Kind::HalfSpaceZ: {
      return std::make_shared<HalfSpace>(
          Eigen::Vector3f(0, 0, 1), 0.0f,
          Eigen::Vector3f(color.r, color.g, color.b));
    }
    case Kind::ExistingObject: {
      return n->existingObject;
    }
    case Kind::Transform: {
      auto childObj = compileNode(n->child);
      if (!childObj) return nullptr;
      return std::make_shared<TransformedWrapper>(childObj, n->transform);
    }
    case Kind::Op: {
      auto A = compileNode(n->left);
      auto B = compileNode(n->right);
      if (!A || !B) return nullptr;
      auto csg = std::make_shared<CSGObject>(
          A, B, n->op, Eigen::Vector3f(color.r, color.g, color.b));
      return csg;
    }
  }
  return nullptr;
}

void ObjectTree::ensureCompiled() const {
  if (!compiled) {
    compiled = compileNode(root);
    if (compiled) {
      compiled = std::make_shared<ColorOverrideWrapper>(compiled, color);
    }
  }
}

PixelInfo ObjectTree::intersectWithLine(const Line& line) {
  ensureCompiled();
  if (!compiled) return PixelInfo{Color{0, 0, 0}, Position{0, 0, 0}, 0.0};
  return compiled->intersectWithLine(line);
}

bool ObjectTree::contains(const Eigen::Vector3f& P) const {
  ensureCompiled();
  if (!compiled) return false;
  return compiled->contains(P);
}

bool ObjectTree::rayInterval(const Line& line, float& tEnter,
                             float& tExit) const {
  ensureCompiled();
  if (!compiled) return false;
  return compiled->rayInterval(line, tEnter, tExit);
}
