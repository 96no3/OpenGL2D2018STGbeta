/**
* @file MainScene.cpp
*/
#include "MainScene.h"

// �v���g�^�C�v�錾.
TweenAnimation::TweenPtr createCurveTween();
TweenAnimation::TweenPtr createUTurnTween();
TweenAnimation::TweenPtr createSwayTween();
TweenAnimation::TweenPtr createBossTween();

// �G�̃A�j���[�V����.
const FrameAnimation::KeyFrame smallEnemyKeyFrames[] = {
{ 0.000f, glm::vec2(480, 0), glm::vec2(32, 32) },
{ 0.125f, glm::vec2(480, 96), glm::vec2(32, 32) },
{ 0.250f, glm::vec2(480, 64), glm::vec2(32, 32) },
{ 0.375f, glm::vec2(480, 32), glm::vec2(32, 32) },
{ 0.500f, glm::vec2(480, 0), glm::vec2(32, 32) },
};
const FrameAnimation::KeyFrame mediumEnemyKeyFrames[] = {
{ 0.000f, glm::vec2(464, 128), glm::vec2(48, 32) },
{ 0.100f, glm::vec2(464, 160), glm::vec2(48, 32) },
{ 0.200f, glm::vec2(464, 192), glm::vec2(48, 32) },
{ 0.300f, glm::vec2(464, 224), glm::vec2(48, 32) },
{ 0.400f, glm::vec2(464, 256), glm::vec2(48, 32) },
{ 0.500f, glm::vec2(464, 288), glm::vec2(48, 32) },
{ 0.600f, glm::vec2(464, 320), glm::vec2(48, 32) },
};
const FrameAnimation::KeyFrame largeEnemyKeyFrames[] = {
{ 0.000f, glm::vec2(448, 352), glm::vec2(64, 64) },
{ 0.125f, glm::vec2(448, 416), glm::vec2(64, 64) },
{ 0.250f, glm::vec2(448, 352), glm::vec2(64, 64) },
};
const FrameAnimation::KeyFrame bossKeyFrames[] = {
{ 0.000f, glm::vec2(320, 128), glm::vec2(128, 256) },
{ 1.000f, glm::vec2(320, 128), glm::vec2(128, 256) },
};

/**
* �}�b�v�ɔz�u����Ă��镨�̂̃f�[�^.
*/
MapObjectData mapObjectDataList[]{
	{ tileId_SmallEnemy, Rect(480, 0, 32, 32), Rect(-16, -16, 32, 32), 1, 100, nullptr, createCurveTween },
	{ tileId_MediumEnemy, Rect(464, 128, 48, 32), Rect(-24, -16, 48, 32), 5, 200, nullptr, createUTurnTween },
	{ tileId_LargeEnemy, Rect(448, 352, 64, 64), Rect(-32, -32, 64, 64), 30, 500, nullptr, createSwayTween },
	{ tileId_Boss, Rect(320, 128, 128, 256), Rect(-48, -112, 96, 224), 200, 10000, nullptr, createBossTween },
};

/**
* ���̂̃f�[�^�z�������������.
*/
void initializeMapObjectDataList() {
	mapObjectDataList[0].tl = FrameAnimation::Timeline::Create(smallEnemyKeyFrames);
	mapObjectDataList[1].tl = FrameAnimation::Timeline::Create(mediumEnemyKeyFrames);
	mapObjectDataList[2].tl = FrameAnimation::Timeline::Create(largeEnemyKeyFrames);
	mapObjectDataList[3].tl = FrameAnimation::Timeline::Create(bossKeyFrames);
}

/**
* �^�C��ID�ɑΉ�����MapObjectData���擾����.
*
* @param tileId			��������^�C��ID.
*
* @retval nullptr�ȊO	�^�C��ID�ɑΉ�����f�[�^.
* @retval nullptr		�Ή�����f�[�^��������Ȃ�����.
*/
const MapObjectData* findMapObjectData(int tileId) {
	for (const MapObjectData* i = std::begin(mapObjectDataList); i != std::end(mapObjectDataList); i++) {
		if (i->id == tileId) {
			return i;
		}
	}
	return nullptr;
}

/**
* �G�̈ړ��p�^�[�����쐬����(�֍s�^�C�v).
*/
TweenAnimation::TweenPtr createCurveTween() {
	namespace TA = TweenAnimation;
	TA::SequencePtr seq = TA::Sequence::Create(4);
	seq->Add(TA::MoveBy::Create(1, glm::vec3(0, 100, 0), TA::EasingType::EaseInOut, TA::Target::Y));
	seq->Add(TA::MoveBy::Create(1, glm::vec3(0, -100, 0), TA::EasingType::EaseInOut, TA::Target::Y));
	TA::ParallelizePtr par = TA::Parallelize::Create(1);
	par->Add(seq);
	par->Add(TA::MoveBy::Create(8, glm::vec3(-1000, 0, 0), TA::EasingType::Linear, TA::Target::X));
	return par;
}

/**
* �G�̈ړ��p�^�[�����쐬����(U�^�[���^�C�v).
*/
TweenAnimation::TweenPtr createUTurnTween() {
	namespace TA = TweenAnimation;
	TA::SequencePtr seq = TA::Sequence::Create(1);
	seq->Add(TA::MoveBy::Create(1, glm::vec3(-100, 0, 0), TA::EasingType::EaseOut));
	seq->Add(TA::Wait::Create(2));
	seq->Add(TA::MoveBy::Create(1, glm::vec3(100, 0, 0), TA::EasingType::EaseIn));	
	return seq;
}

/**
* �G�̈ړ��p�^�[�����쐬����(�s�����藈���肷��^�C�v).
*/
TweenAnimation::TweenPtr createSwayTween() {
	namespace TA = TweenAnimation;
	TA::SequencePtr seq = TA::Sequence::Create(1);
	seq->Add(TA::MoveBy::Create(2, glm::vec3(-200, 0, 0), TA::EasingType::EaseOut));
	seq->Add(TA::MoveBy::Create(1, glm::vec3(0, 100, 0), TA::EasingType::EaseInOut));
	for (int i = 0; i < 2; i++) {
		seq->Add(TA::MoveBy::Create(2, glm::vec3(-200, 0, 0), TA::EasingType::EaseInOut));
		seq->Add(TA::MoveBy::Create(2, glm::vec3(0, 200, 0), TA::EasingType::EaseInOut));
	}
	seq->Add(TA::MoveBy::Create(1, glm::vec3(0, -100, 0), TA::EasingType::EaseInOut));
	seq->Add(TA::MoveBy::Create(4, glm::vec3(-800, 0, 0), TA::EasingType::EaseIn));
	return seq;
}

/**
* �G�̈ړ��p�^�[�����쐬����(�{�X�p).
*/
TweenAnimation::TweenPtr createBossTween() {
	namespace TA = TweenAnimation;
	TA::SequencePtr seq = TA::Sequence::Create(1);
	seq->Add(TA::MoveBy::Create(2, glm::vec3(-200, 0, 0), TA::EasingType::EaseOut));
	seq->Add(TA::MoveBy::Create(1, glm::vec3(0, 100, 0), TA::EasingType::EaseInOut));
	for (int i = 0; i < 8; i++) {
		seq->Add(TA::MoveBy::Create(2, glm::vec3(0, -200, 0), TA::EasingType::EaseInOut));
		seq->Add(TA::MoveBy::Create(2, glm::vec3(0, 200, 0), TA::EasingType::EaseInOut));
	}
	seq->Add(TA::MoveBy::Create(1, glm::vec3(0, -100, 0), TA::EasingType::EaseInOut));
	seq->Add(TA::MoveBy::Create(4, glm::vec3(-800, 0, 0), TA::EasingType::EaseIn));
	return seq;
}

//// �G2�̈ړ�(�g�E�B�[�j���O)
//namespace TA = TweenAnimation;
//TA::SequencePtr seq = TA::Sequence::Create(4);
//seq->Add(TA::MoveBy::Create(1, glm::vec3(0, 100, 0), TA::EasingType::EaseInOut, TA::Target::Y));
//seq->Add(TA::MoveBy::Create(1, glm::vec3(0, -100, 0), TA::EasingType::EaseInOut, TA::Target::Y));
//TA::ParallelizePtr par = TA::Parallelize::Create(1);
//par->Add(seq);
//par->Add(TA::MoveBy::Create(8, glm::vec3(-1000, 0, 0), TA::EasingType::Linear, TA::Target::X));
//enemy2->spr.Tweener(TA::Animate::Create(par));
//
// //�G�̈ړ�(�g�E�B�[�j���O)
//namespace TA = TweenAnimation;
//
// //���꒼���̈ړ�
//enemy->spr.Tweener(TweenAnimation::Animate::Create(
//TweenAnimation::MoveBy::Create(
//5.0f, glm::vec3(-1000, 0, 0), TweenAnimation::EasingType::Linear)));
//
//// 09��@1.1
//TA::SequencePtr seq = TA::Sequence::Create(2);
//seq->Add(TA::MoveBy::Create(1, glm::vec3(-200, 100, 0), TA::EasingType::Linear));
//seq->Add(TA::MoveBy::Create(1, glm::vec3(-200, -100, 0), TA::EasingType::Linear));
//enemy->spr.Tweener(TA::Animate::Create(seq));
//
// //09��@�ۑ�1
//TA::SequencePtr seq = TA::Sequence::Create(4);
//seq->Add(TA::MoveBy::Create(1, glm::vec3(-100, 100, 0), TA::EasingType::Linear));
//seq->Add(TA::MoveBy::Create(1, glm::vec3(-100, -100, 0), TA::EasingType::Linear));
//enemy->spr.Tweener(TA::Animate::Create(seq));
//
// // 09��@�ۑ�2
//TA::SequencePtr seq = TA::Sequence::Create(4);
//seq->Add(TA::MoveBy::Create(1, glm::vec3(-100, 0, 0), TA::EasingType::Linear));
//seq->Add(TA::MoveBy::Create(1, glm::vec3(0, 100, 0), TA::EasingType::Linear));
//seq->Add(TA::MoveBy::Create(1, glm::vec3(-100, 0, 0), TA::EasingType::Linear));
//seq->Add(TA::MoveBy::Create(1, glm::vec3(0, -100, 0), TA::EasingType::Linear));
//enemy->spr.Tweener(TA::Animate::Create(seq));
//
//// 09��@1.2
//TA::SequencePtr seq = TA::Sequence::Create(2);
//seq->Add(TA::MoveBy::Create(1, glm::vec3(0, 100, 0), TA::EasingType::EaseInOut,TA::Target::Y));
//seq->Add(TA::MoveBy::Create(1, glm::vec3(0, -100, 0), TA::EasingType::EaseInOut, TA::Target::Y));
//TA::ParallelizePtr par = TA::Parallelize::Create(1);
//par->Add(seq);
//par->Add(TA::MoveBy::Create(8, glm::vec3(-1000, 0, 0), TA::EasingType::Linear, TA::Target::X));
//enemy->spr.Tweener(TA::Animate::Create(par));
//
//// 09��@�ۑ�3 �W�O�U�O�ړ�
//TA::SequencePtr seq = TA::Sequence::Create(2);
//seq->Add(TA::MoveBy::Create(4, glm::vec3(-400, -150, 0), TA::EasingType::Linear));
//seq->Add(TA::MoveBy::Create(4, glm::vec3(400, -150, 0), TA::EasingType::Linear));
//enemy->spr.Tweener(TA::Animate::Create(seq));
//
////  09��@�ۑ�3 x���W�����_���o��ver
//TA::SequencePtr seq = TA::Sequence::Create(3);
//seq->Add(TA::MoveBy::Create(1, glm::vec3(-400, 0, 0), TA::EasingType::EaseInOut, TA::Target::X));
//seq->Add(TA::MoveBy::Create(1, glm::vec3(400, 0, 0), TA::EasingType::EaseInOut, TA::Target::X));
//TA::ParallelizePtr par = TA::Parallelize::Create(1);
//par->Add(seq);
//par->Add(TA::MoveBy::Create(6, glm::vec3(0, -900, 0), TA::EasingType::Linear, TA::Target::Y));
//enemy->spr.Tweener(TA::Animate::Create(par));