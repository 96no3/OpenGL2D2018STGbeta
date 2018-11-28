#pragma once
/**
* @file MainScene.h
*/
#include "Sprite.h"

const int tileId_SmallEnemy = 256;		// �G�Ƃ݂Ȃ��^�C��ID.
const int tileId_MediumEnemy = 191;		// �G2�Ƃ݂Ȃ��^�C��ID.
const int tileId_LargeEnemy = 63;		// �G3�Ƃ݂Ȃ��^�C��ID.
const int tileId_Boss = 75;				// �{�X�Ƃ݂Ȃ��^�C��ID.
const int tileId_PowerUpItem = 230;		// �p���[�A�b�v�A�C�e���̃^�C��ID.

/**
* �}�b�v�ɔz�u���ꂽ�I�u�W�F�N�g�̃f�[�^.
*/
struct MapObjectData {
	int id;							// �I�u�W�F�N�g��ID.
	Rect imageRect;					// �摜�͈̔�.
	Rect collisionShape;			// �Փ˔���͈̔�.
	int health;						// �����̗�.
	int score;						// ���_.
	FrameAnimation::TimelinePtr tl;	// �A�j���[�V�����E�^�C�����C��.

	using CreateTweenFunc = TweenAnimation::TweenPtr(*)();	// �s���ݒ�֐��̌^.
	CreateTweenFunc createTween;	// �s����ݒ肷��֐�.
};

/**
* ���C����ʂŎg�p����̍\����.
*/
struct MainScene {
	int stageNo;		// ���݂̃X�e�[�W�ԍ�.
};
bool initialize(MainScene*);

void initializeMapObjectDataList();
const MapObjectData* findMapObjectData(int);