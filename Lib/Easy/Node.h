/**
* @file Node.h
*/
#ifndef NODE_H_INCLUDED
#define NODE_H_INCLUDED
#include "TweenAnimation.h"
#include <glm/glm.hpp>
#include <deque>
#include <string>

// ��s�錾.
class SpriteRenderer;

/**
* �m�[�h�N���X.
*
* �m�[�h�́u�V�[�����\������؍\��(�V�[���O���t)�v���\�z���邽�߂̕��i�ł�.
* �V�[���O���t���\�z����ɂ́A�܂����{�ɂ�����e�m�[�h(���[�g�m�[�h)���쐬���A
* addChild�ɂ���Ďq�m�[�h��ǉ����Ă����܂�.
* 
* �m�[�h�̓V�[���O���t���\�����邾���ŁA���ɂ͉������܂���.
* �m�[�h�ɓ��ʂȓ��������������ꍇ��Node�N���X��h�������āAUpdate�֐���
* Draw�֐��A���邢�͂��̗������I�[�o�[���C�h���Ă�������.
*
* ��ʂɉ�����\���������ꍇ��Node�N���X�ł͂Ȃ�Sprite�N���X���g�p���Ă�������.
*
* @sa Sprite
*/
class Node
{
public:
  friend class SpriteRenderer;

  Node() = default;
  virtual ~Node();
  Node(const Node&) = default;
  Node& operator=(const Node&) = default;

  void Name(const char* n) { name = n; } 
  const std::string& Name() const { return name; }

  void Position(const glm::vec3& p) { position = p; }
  const glm::vec3& Position() const { return position; }
  const glm::vec3& WorldPosition() const { return worldPosition; }
  void Scale(const glm::vec2& s) { scale = s; }
  const glm::vec2& Scale() const { return scale; }
  void Rotation(float r) { rotation = r; }
  float Rotation() const { return rotation; }
  void Shear(float s) { shear = s; }
  float Shear() const { return shear; }
  const glm::mat4x4& Transform() const { return transform; }

  void AddChild(Node*);
  void RemoveChild(Node*);
  Node* Parent() const { return parent; }
  const std::deque<Node*>& Children() const { return children; }

  void UpdateRecursive(float dt);
  void UpdateTransform();
  virtual void Update(float dt);

  void Tweener(const TweenAnimation::AnimatePtr& p);
  const TweenAnimation::AnimatePtr& Tweener() const { return tweener; }

private:
  virtual void Draw(SpriteRenderer&) const;

  std::string name; ///< �m�[�h��.
  glm::vec3 position; ///< �m�[�h�̍��W.
  glm::vec2 scale = glm::vec2(1, 1); ///< �m�[�h�̊g��k����.
  float rotation = 0; 
  float shear = 0;

  glm::mat4x4 transform; ///< �m�[�h�̍��W�ϊ��s��.
  glm::vec3 worldPosition;

  Node* parent = nullptr; ///< �e�m�[�h.
  std::deque<Node*> children; ///< �q�m�[�h�̃��X�g.

  TweenAnimation::AnimatePtr tweener; ///< �g�E�B�[�j���O����.
};

#endif // NODE_H_INCLUDED