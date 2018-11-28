#pragma once
/**
* @file MainScene.h
*/
#include "Sprite.h"

const int tileId_SmallEnemy = 256;		// 敵とみなすタイルID.
const int tileId_MediumEnemy = 191;		// 敵2とみなすタイルID.
const int tileId_LargeEnemy = 63;		// 敵3とみなすタイルID.
const int tileId_Boss = 75;				// ボスとみなすタイルID.
const int tileId_PowerUpItem = 230;		// パワーアップアイテムのタイルID.

/**
* マップに配置されたオブジェクトのデータ.
*/
struct MapObjectData {
	int id;							// オブジェクトのID.
	Rect imageRect;					// 画像の範囲.
	Rect collisionShape;			// 衝突判定の範囲.
	int health;						// 初期体力.
	int score;						// 得点.
	FrameAnimation::TimelinePtr tl;	// アニメーション・タイムライン.

	using CreateTweenFunc = TweenAnimation::TweenPtr(*)();	// 行動設定関数の型.
	CreateTweenFunc createTween;	// 行動を設定する関数.
};

/**
* メイン画面で使用するの構造体.
*/
struct MainScene {
	int stageNo;		// 現在のステージ番号.
};
bool initialize(MainScene*);

void initializeMapObjectDataList();
const MapObjectData* findMapObjectData(int);