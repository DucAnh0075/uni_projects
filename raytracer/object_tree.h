#pragma once
#include <memory>
#include <utility>

#include "object.h"
#include "transform.h"

class ObjectTree : public Object {
 public:
  struct Node {
    enum class Kind {
      SphereUnit,
      CubeUnit,
      HalfSpaceZ,
      Transform,
      Op,
      ExistingObject
    };
    Kind kind;
    Transform transform;
    std::shared_ptr<Node> child;
    CSGOp op{CSGOp::Union};
    std::shared_ptr<Node> left;
    std::shared_ptr<Node> right;
    std::shared_ptr<Object> existingObject;

    explicit Node(Kind k) : kind(k) {}
  };

  static std::shared_ptr<Node> SphereUnit() {
    return std::make_shared<Node>(Node::Kind::SphereUnit);
  }
  static std::shared_ptr<Node> CubeUnit() {
    return std::make_shared<Node>(Node::Kind::CubeUnit);
  }
  static std::shared_ptr<Node> HalfSpaceZ() {
    return std::make_shared<Node>(Node::Kind::HalfSpaceZ);
  }
  static std::shared_ptr<Node> ExistingObjectNode(std::shared_ptr<Object> obj) {
    auto n = std::make_shared<Node>(Node::Kind::ExistingObject);
    n->existingObject = std::move(obj);
    return n;
  }
  static std::shared_ptr<Node> TransformNode(const Transform& T,
                                             std::shared_ptr<Node> child) {
    auto n = std::make_shared<Node>(Node::Kind::Transform);
    n->transform = T;
    n->child = std::move(child);
    return n;
  }
  static std::shared_ptr<Node> OpNode(CSGOp op, std::shared_ptr<Node> A,
                                      std::shared_ptr<Node> B) {
    auto n = std::make_shared<Node>(Node::Kind::Op);
    n->op = op;
    n->left = std::move(A);
    n->right = std::move(B);
    return n;
  }

  explicit ObjectTree(std::shared_ptr<Node> root_, const Eigen::Vector3f& col);
  explicit ObjectTree(std::shared_ptr<Node> root_, const Eigen::Vector3f& col,
                      const Material& mat);

  PixelInfo intersectWithLine(const Line& line) override;
  bool contains(const Eigen::Vector3f& P) const override;
  bool rayInterval(const Line& line, float& tEnter,
                   float& tExit) const override;

  std::shared_ptr<Node> getRoot() const { return root; }

  const Material& getMaterial() const override { return treeMaterial; }

 private:
  std::shared_ptr<Node> root;
  mutable std::shared_ptr<Object> compiled;
  Material treeMaterial;

  class TransformedWrapper;
  class ColorOverrideWrapper;

  std::shared_ptr<Object> compileNode(const std::shared_ptr<Node>& n) const;
  void ensureCompiled() const;
};
